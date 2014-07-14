/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"

#include "math/glmath.h"

#include "engines/grim/actor.h"
#include "engines/grim/colormap.h"
#include "engines/grim/material.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_tinygl.h"
#include "engines/grim/grim.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/primitives.h"
#include "engines/grim/model.h"
#include "engines/grim/sprite.h"
#include "engines/grim/set.h"
#include "engines/grim/emi/modelemi.h"

namespace Grim {

GfxBase *CreateGfxTinyGL() {
	return new GfxTinyGL();
}

GfxTinyGL::GfxTinyGL() :
		_zb(nullptr), _alpha(1.f),
		_bufferId(0), _currentActor(nullptr) {
	g_driver = this;
	_storedDisplay = nullptr;
	// TGL_LEQUAL as tglDepthFunc ensures that subsequent drawing attempts for
	// the same triangles are not ignored by the depth test.
	// That's necessary for EMI where some models have multiple faces which
	// refer to the same vertices. The first face is usually using the
	// color map and the following are using textures.
	_depthFunc = (g_grim->getGameType() == GType_MONKEY4) ? TGL_LEQUAL : TGL_LESS;
}

GfxTinyGL::~GfxTinyGL() {
	if (_zb) {
		delBuffer(1);
		TinyGL::glClose();
		delete _zb;
	}
}

byte *GfxTinyGL::setupScreen(int screenW, int screenH, bool fullscreen) {
	Graphics::PixelBuffer buf = g_system->setupScreen(screenW, screenH, fullscreen, false);
	byte *buffer = buf.getRawBuffer();

	_screenWidth = screenW;
	_screenHeight = screenH;
	_scaleW = _screenWidth / (float)_gameWidth;
	_scaleH = _screenHeight / (float)_gameHeight;

	_isFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	g_system->showMouse(!fullscreen);

	g_system->setWindowCaption("ResidualVM: Software 3D Renderer");

	_pixelFormat = buf.getFormat();
	_zb = new TinyGL::FrameBuffer(screenW, screenH, buf);
	TinyGL::glInit(_zb, 256);

	_storedDisplay.create(_pixelFormat, _gameWidth * _gameHeight, DisposeAfterUse::YES);
	_storedDisplay.clear(_gameWidth * _gameHeight);

	_currentShadowArray = nullptr;

	TGLfloat ambientSource[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	tglLightModelfv(TGL_LIGHT_MODEL_AMBIENT, ambientSource);

	// we now generate a buffer (id 1), which we will use as a backing buffer, where the actors' clean buffers
	// will blit to. everu frame this will be blitted to screen, but the actors' buffers will be blitted to
	// this only when they change.
	genBuffer();

	return buffer;
}

const char *GfxTinyGL::getVideoDeviceName() {
	return "TinyGL Software Renderer";
}

void GfxTinyGL::setupCamera(float fov, float nclip, float fclip, float roll) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float right = nclip * tan(fov / 2 * (LOCAL_PI / 180));
	tglFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void GfxTinyGL::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) {
	if (g_grim->getGameType() == GType_MONKEY4) {
		tglScalef(1.0, 1.0, -1.0);

		_currentPos = pos;
		_currentQuat = Math::Quaternion(interest.x(), interest.y(), interest.z(), roll);
	} else {
		Math::Vector3d up_vec(0, 0, 1);

		tglRotatef(roll, 0, 0, -1);

		if (pos.x() == interest.x() && pos.y() == interest.y())
			up_vec = Math::Vector3d(0, 1, 0);

		Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
		tglMultMatrixf(lookMatrix.getData());
		tglTranslatef(-pos.x(), -pos.y(), -pos.z());
	}
}

Math::Matrix4 GfxTinyGL::getModelView() {
	Math::Matrix4 modelView;

	if (g_grim->getGameType() == GType_MONKEY4) {
		tglMatrixMode(TGL_MODELVIEW);
		tglPushMatrix();

		Math::Matrix4 worldRot = _currentQuat.toMatrix();
		tglMultMatrixf(worldRot.getData());
		tglTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());

		tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView.getData());

		tglPopMatrix();
	} else {
		tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView.getData());
	}

	modelView.transpose();
	return modelView;
}

Math::Matrix4 GfxTinyGL::getProjection() {
	Math::Matrix4 projection;
	tglGetFloatv(TGL_PROJECTION_MATRIX, projection.getData());
	projection.transpose();
	return projection;
}

void GfxTinyGL::clearScreen() {
	_zb->clear(true, 0, true, 0, 0, 0);
}

void GfxTinyGL::clearDepthBuffer() {
	memset(_zb->zbuf, 0, _gameWidth * _gameHeight * sizeof(uint32));
}

void GfxTinyGL::flipBuffer() {
	g_system->updateScreen();
}

int GfxTinyGL::genBuffer() {
	TinyGL::Buffer *buf = _zb->genOffscreenBuffer();
	_buffers[++_bufferId] = buf;

	return _bufferId;
}

void GfxTinyGL::delBuffer(int id) {
	_zb->delOffscreenBuffer(_buffers[id]);
	_buffers.erase(id);
}

void GfxTinyGL::selectBuffer(int id) {
	if (id == 0) {
		_zb->selectOffscreenBuffer(NULL);
	} else {
		_zb->selectOffscreenBuffer(_buffers[id]);
	}
}

void GfxTinyGL::clearBuffer(int id) {
	TinyGL::Buffer *buf = _buffers[id];
	_zb->clearOffscreenBuffer(buf);
}

void GfxTinyGL::drawBuffers() {
	selectBuffer(1);
	Common::HashMap<int, TinyGL::Buffer *>::iterator i = _buffers.begin();
	for (++i; i != _buffers.end(); ++i) {
		TinyGL::Buffer *buf = i->_value;
		_zb->blitOffscreenBuffer(buf);
		//this is not necessary, but it prevents the buffers to be blitted every frame, if it is not needed
		buf->used = false;
	}

	selectBuffer(0);
	_zb->blitOffscreenBuffer(_buffers[1]);
}

void GfxTinyGL::refreshBuffers() {
	clearBuffer(1);
	Common::HashMap<int, TinyGL::Buffer *>::iterator i = _buffers.begin();
	for (++i; i != _buffers.end(); ++i) {
		TinyGL::Buffer *buf = i->_value;
		buf->used = true;
	}
}

bool GfxTinyGL::isHardwareAccelerated() {
	return false;
}

bool GfxTinyGL::supportsShaders() {
	return false;
}

static void tglShadowProjection(const Math::Vector3d &light, const Math::Vector3d &plane, const Math::Vector3d &normal, bool dontNegate) {
	// Based on GPL shadow projection example by
	// (c) 2002-2003 Phaetos <phaetos@gaffga.de>
	float d, c;
	float mat[16];
	float nx, ny, nz, lx, ly, lz, px, py, pz;

	nx = normal.x();
	ny = normal.y();
	nz = normal.z();
	// for some unknown for me reason normal need negation
	if (!dontNegate) {
		nx = -nx;
		ny = -ny;
		nz = -nz;
	}
	lx = light.x();
	ly = light.y();
	lz = light.z();
	px = plane.x();
	py = plane.y();
	pz = plane.z();

	d = nx * lx + ny * ly + nz * lz;
	c = px * nx + py * ny + pz * nz - d;

	mat[0] = lx * nx + c;
	mat[4] = ny * lx;
	mat[8] = nz * lx;
	mat[12] = -lx * c - lx * d;

	mat[1] = nx * ly;
	mat[5] = ly * ny + c;
	mat[9] = nz * ly;
	mat[13] = -ly * c - ly * d;

	mat[2] = nx * lz;
	mat[6] = ny * lz;
	mat[10] = lz * nz + c;
	mat[14] = -lz * c - lz * d;

	mat[3] = nx;
	mat[7] = ny;
	mat[11] = nz;
	mat[15] = -d;

	tglMultMatrixf(mat);
}

void GfxTinyGL::getBoundingBoxPos(const Mesh *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	TGLfloat top = 1000;
	TGLfloat right = -1000;
	TGLfloat left = 1000;
	TGLfloat bottom = -1000;

	for (int i = 0; i < model->_numFaces; i++) {
		Math::Vector3d obj;
		float *pVertices;

		for (int j = 0; j < model->_faces[i].getNumVertices(); j++) {
			TGLfloat modelView[16], projection[16];
			TGLint viewPort[4];

			tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView);
			tglGetFloatv(TGL_PROJECTION_MATRIX, projection);
			tglGetIntegerv(TGL_VIEWPORT, viewPort);

			pVertices = model->_vertices + 3 * model->_faces[i].getVertex(j);

			obj.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

			Math::Vector3d win;
			Math::gluMathProject<TGLfloat>(obj, modelView, projection, viewPort, win);

			if (win.x() > right)
				right = win.x();
			if (win.x() < left)
				left = win.x();
			if (win.y() < top)
				top = win.y();
			if (win.y() > bottom)
				bottom = win.y();
		}
	}

	float t = bottom;
	bottom = _gameHeight - top;
	top = _gameHeight - t;

	if (left < 0)
		left = 0;
	if (right >= _gameWidth)
		right = _gameWidth - 1;
	if (top < 0)
		top = 0;
	if (bottom >= _gameHeight)
		bottom = _gameHeight - 1;

	if (top >= _gameHeight || left >= _gameWidth || bottom < 0 || right < 0) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	*x1 = (int)left;
	*y1 = (int)top;
	*x2 = (int)right;
	*y2 = (int)bottom;
}

void GfxTinyGL::getBoundingBoxPos(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	TGLfloat top = 1000;
	TGLfloat right = -1000;
	TGLfloat left = 1000;
	TGLfloat bottom = -1000;

	TGLfloat modelView[16], projection[16];
	TGLint viewPort[4];

	tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView);
	tglGetFloatv(TGL_PROJECTION_MATRIX, projection);
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	for (uint i = 0; i < model->_numFaces; i++) {
		int *indices = (int *)model->_faces[i]._indexes;
		
		for (uint j = 0; j < model->_faces[i]._faceLength * 3; j++) {
			int index = indices[j];

			Math::Vector3d obj = model->_drawVertices[index];
			Math::Vector3d win;
			Math::gluMathProject<TGLfloat>(obj, modelView, projection, viewPort, win);

			if (win.x() > right)
				right = win.x();
			if (win.x() < left)
				left = win.x();
			if (win.y() < top)
				top = win.y();
			if (win.y() > bottom)
				bottom = win.y();
		}
	}

	float t = bottom;
	bottom = _gameHeight - top;
	top = _gameHeight - t;

	if (left < 0)
		left = 0;
	if (right >= _gameWidth)
		right = _gameWidth - 1;
	if (top < 0)
		top = 0;
	if (bottom >= _gameHeight)
		bottom = _gameHeight - 1;

	if (top >= _gameHeight || left >= _gameWidth || bottom < 0 || right < 0) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	*x1 = (int)left;
	*y1 = (int)(_gameHeight - bottom);
	*x2 = (int)right;
	*y2 = (int)(_gameHeight - top);
}

void GfxTinyGL::startActorDraw(const Actor *actor) {
	_currentActor = actor;
	tglEnable(TGL_TEXTURE_2D);
	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	if (_currentShadowArray) {
		// TODO find out why shadowMask at device in woods is null
		if (!_currentShadowArray->shadowMask) {
			_currentShadowArray->shadowMask = new byte[_gameWidth * _gameHeight];
			_currentShadowArray->shadowMaskSize = _gameWidth * _gameHeight;
		}
		assert(_currentShadowArray->shadowMask);
		//tglSetShadowColor(255, 255, 255);
		tglSetShadowColor(_shadowColorR, _shadowColorG, _shadowColorB);
		tglSetShadowMaskBuf(_currentShadowArray->shadowMask);
		SectorListType::iterator i = _currentShadowArray->planeList.begin();
		Sector *shadowSector = i->sector;
		tglShadowProjection(_currentShadowArray->pos, shadowSector->getVertices()[0], shadowSector->getNormal(), _currentShadowArray->dontNegate);
	}

	const float alpha = actor->getEffectiveAlpha();
	if (alpha < 1.f) {
		_alpha = alpha;
		tglEnable(TGL_BLEND);
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		tglEnable(TGL_CULL_FACE);
		tglFrontFace(TGL_CW);

		if (actor->isInOverworld()) {
			const Math::Vector3d &pos = actor->getWorldPos();
			const Math::Quaternion &quat = actor->getRotationQuat();
			// At distance 3.2, a 6.4x4.8 actor fills the screen.
			tglMatrixMode(TGL_PROJECTION);
			tglLoadIdentity();
			float right = 1;
			float top = right * 0.75;
			float div = 6.0f;
			tglFrustum(-right / div, right / div, -top / div, top / div, 1.0f / div, 3276.8f);
			tglMatrixMode(TGL_MODELVIEW);
			tglLoadIdentity();
			tglScalef(1.0, 1.0, -1.0);
			tglTranslatef(pos.x(), pos.y(), pos.z());
			tglMultMatrixf(quat.toMatrix().getData());
		} else {
			Math::Matrix4 worldRot = _currentQuat.toMatrix();
			tglMultMatrixf(worldRot.getData());
			tglTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());

			Math::Matrix4 m = actor->getFinalMatrix();
			m.transpose();
			tglMultMatrixf(m.getData());
		}
	} else {
		// Grim
		Math::Vector3d pos = actor->getWorldPos();
		const Math::Quaternion &quat = actor->getRotationQuat();
		const float &scale = actor->getScale();

		tglTranslatef(pos.x(), pos.y(), pos.z());
		tglScalef(scale, scale, scale);
		tglMultMatrixf(quat.toMatrix().getData());
	}

	if (actor->getSortOrder() >= 100) {
		tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);
		tglDepthMask(TGL_TRUE);
	}
}

void GfxTinyGL::finishActorDraw() {
	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();
	tglMatrixMode(TGL_MODELVIEW);

	tglDisable(TGL_TEXTURE_2D);
	if (_alpha < 1.f) {
		tglDisable(TGL_BLEND);
		_alpha = 1.f;
	}

	if (_currentShadowArray) {
		tglSetShadowMaskBuf(nullptr);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		tglDisable(TGL_CULL_FACE);
	}

	tglColorMask(TGL_TRUE, TGL_TRUE, TGL_TRUE, TGL_TRUE);
	_currentActor = nullptr;
}

void GfxTinyGL::drawShadowPlanes() {
	tglEnable(TGL_SHADOW_MASK_MODE);
	if (!_currentShadowArray->shadowMask) {
		_currentShadowArray->shadowMask = new byte[_gameWidth * _gameHeight];
		_currentShadowArray->shadowMaskSize = _gameWidth * _gameHeight;
	}
	memset(_currentShadowArray->shadowMask, 0, _gameWidth * _gameHeight);

	tglSetShadowMaskBuf(_currentShadowArray->shadowMask);
	_currentShadowArray->planeList.begin();
	for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); ++i) {
		Sector *shadowSector = i->sector;
		tglBegin(TGL_POLYGON);
		for (int k = 0; k < shadowSector->getNumVertices(); k++) {
			tglVertex3f(shadowSector->getVertices()[k].x(), shadowSector->getVertices()[k].y(), shadowSector->getVertices()[k].z());
		}
		tglEnd();
	}
	tglSetShadowMaskBuf(nullptr);
	tglDisable(TGL_SHADOW_MASK_MODE);
}

void GfxTinyGL::setShadowMode() {
	GfxBase::setShadowMode();
	tglEnable(TGL_SHADOW_MODE);
}

void GfxTinyGL::clearShadowMode() {
	GfxBase::clearShadowMode();
	tglDisable(TGL_SHADOW_MODE);
}

void GfxTinyGL::set3DMode() {
	tglMatrixMode(TGL_MODELVIEW);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthFunc(_depthFunc);
}

void GfxTinyGL::setShadow(Shadow *shadow) {
	_currentShadowArray = shadow;
	if (shadow)
		tglDisable(TGL_LIGHTING);
	else
		tglEnable(TGL_LIGHTING);
}

void GfxTinyGL::setShadowColor(byte r, byte g, byte b) {
	_shadowColorR = r;
	_shadowColorG = g;
	_shadowColorB = b;
}

void GfxTinyGL::getShadowColor(byte *r, byte *g, byte *b) {
	*r = _shadowColorR;
	*g = _shadowColorG;
	*b = _shadowColorB;
}

void GfxTinyGL::drawEMIModelFace(const EMIModel *model, const EMIMeshFace *face) {
	int *indices = (int *)face->_indexes;

	tglEnable(TGL_DEPTH_TEST);
	tglDisable(TGL_ALPHA_TEST);
	//tglDisable(TGL_LIGHTING); // not apply here in TinyGL
	if (face->_hasTexture)
		tglEnable(TGL_TEXTURE_2D);
	else
		tglDisable(TGL_TEXTURE_2D);

	tglBegin(TGL_TRIANGLES);
	float dim = 1.0f - _dimLevel;
	for (uint j = 0; j < face->_faceLength * 3; j++) {
		int index = indices[j];
		if (face->_hasTexture) {
			tglTexCoord2f(model->_texVerts[index].getX(), model->_texVerts[index].getY());
		}

		Math::Vector3d lighting = model->_lighting[index];
		byte r = (byte)(model->_colorMap[index].r * lighting.x() * dim);
		byte g = (byte)(model->_colorMap[index].g * lighting.y() * dim);
		byte b = (byte)(model->_colorMap[index].b * lighting.z() * dim);
		byte a = (int)(model->_colorMap[index].a * _alpha);
		tglColor4ub(r, g, b, a);

		Math::Vector3d normal = model->_normals[index];
		Math::Vector3d vertex = model->_drawVertices[index];

		tglNormal3fv(normal.getData());
		tglVertex3fv(vertex.getData());
	}
	tglEnd();

	tglEnable(TGL_TEXTURE_2D);
	tglEnable(TGL_DEPTH_TEST);
	tglEnable(TGL_ALPHA_TEST);
	//tglEnable(TGL_LIGHTING); // not apply here in TinyGL
	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
	tglColor3f(1.0f, 1.0f, 1.0f);
}

void GfxTinyGL::drawModelFace(const Mesh *mesh, const MeshFace *face) {
	float *vertices = mesh->_vertices;
	float *vertNormals = mesh->_vertNormals;
	float *textureVerts = mesh->_textureVerts;
	tglNormal3fv(const_cast<float *>(face->getNormal().getData()));
	tglBegin(TGL_POLYGON);
	for (int i = 0; i < face->getNumVertices(); i++) {
		tglNormal3fv(vertNormals + 3 * face->getVertex(i));

		if (face->hasTexture())
			tglTexCoord2fv(textureVerts + 2 * face->getTextureVertex(i));

		tglColor4f(1.0f, 1.0f, 1.0f, _alpha);
		tglVertex3fv(vertices + 3 * face->getVertex(i));
	}
	tglEnd();
}

void GfxTinyGL::drawSprite(const Sprite *sprite) {
	tglMatrixMode(TGL_TEXTURE);
	tglLoadIdentity();
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();

	if (g_grim->getGameType() == GType_MONKEY4) {
		TGLfloat modelview[16];
		tglGetFloatv(TGL_MODELVIEW_MATRIX, modelview);
		Math::Matrix4 act;
		act.buildAroundZ(_currentActor->getYaw());
		act.transpose();
		act(3, 0) = modelview[12];
		act(3, 1) = modelview[13];
		act(3, 2) = modelview[14];
		tglLoadMatrixf(act.getData());
		tglTranslatef(sprite->_pos.x(), sprite->_pos.y(), sprite->_pos.z());
	} else {
		tglTranslatef(sprite->_pos.x(), sprite->_pos.y(), sprite->_pos.z());
		TGLfloat modelview[16];
		tglGetFloatv(TGL_MODELVIEW_MATRIX, modelview);

		// We want screen-aligned sprites so reset the rotation part of the matrix.
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (i == j) {
					modelview[i * 4 + j] = 1.0f;
				} else {
					modelview[i * 4 + j] = 0.0f;
				}
			}
		}
		tglLoadMatrixf(modelview);
	}

	if (sprite->_blendMode == Sprite::BlendAdditive) {
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE);
	} else {
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	}

	tglDisable(TGL_LIGHTING);

	if (sprite->_alphaTest) {
		tglEnable(TGL_ALPHA_TEST);
		tglAlphaFunc(TGL_GEQUAL, g_grim->getGameType() == GType_MONKEY4 ? 0.1f : 0.5f);
	} else {
		tglDisable(TGL_ALPHA_TEST);
	}

	if (sprite->_writeDepth) {
		tglEnable(TGL_DEPTH_TEST);
	} else {
		tglDisable(TGL_DEPTH_TEST);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		if (_currentActor->isInOverworld()) {
			// The Overworld actors don't have a proper sort order
			// so we rely on the z coordinates
			tglDepthMask(TGL_TRUE);
		} else {
			tglDepthMask(TGL_FALSE);
		}

		float halfWidth = sprite->_width / 2;
		float halfHeight = sprite->_height / 2;
		float dim = 1.0f - _dimLevel;
		float texCoordsX[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		float texCoordsY[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		float vertexX[] = { -1.0f, -1.0f, 1.0f, 1.0f };
		float vertexY[] = { -1.0f, 1.0f, 1.0f, -1.0f };

		tglBegin(TGL_POLYGON);
		for (int i = 0; i < 4; ++i) {
			float r = sprite->_red[i] * dim / 255.0f;
			float g = sprite->_green[i] * dim / 255.0f;
			float b = sprite->_blue[i] * dim / 255.0f;
			float a = sprite->_alpha[i] * dim * _alpha / 255.0f;

			tglColor4f(r, g, b, a);
			tglTexCoord2f(texCoordsX[i], texCoordsY[i]);
			tglVertex3f(vertexX[i] * halfWidth, vertexY[i] * halfHeight, 0.0f);
		}
		tglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		tglEnd();
	} else {
		// In Grim, the bottom edge of the sprite is at y=0 and
		// the texture is flipped along the X-axis.
		float halfWidth = sprite->_width / 2;
		float height = sprite->_height;

		tglBegin(TGL_POLYGON);
		tglTexCoord2f(0.0f, 1.0f);
		tglVertex3f(+halfWidth, 0.0f, 0.0f);
		tglTexCoord2f(0.0f, 0.0f);
		tglVertex3f(+halfWidth, +height, 0.0f);
		tglTexCoord2f(1.0f, 0.0f);
		tglVertex3f(-halfWidth, +height, 0.0f);
		tglTexCoord2f(1.0f, 1.0f);
		tglVertex3f(-halfWidth, 0.0f, 0.0f);
		tglEnd();
	}

	tglEnable(TGL_LIGHTING);
	tglDisable(TGL_ALPHA_TEST);
	tglDepthMask(TGL_TRUE);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglDisable(TGL_BLEND);
	tglEnable(TGL_DEPTH_TEST);

	tglPopMatrix();
}

void GfxTinyGL::translateViewpointStart() {
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
}

void GfxTinyGL::translateViewpoint(const Math::Vector3d &vec) {
	tglTranslatef(vec.x(), vec.y(), vec.z());
}

void GfxTinyGL::rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis) {
	tglRotatef(angle.getDegrees(), axis.x(), axis.y(), axis.z());
}

void GfxTinyGL::rotateViewpoint(const Math::Matrix4 &rot) {
	tglMultMatrixf(rot.getData());
}

void GfxTinyGL::translateViewpointFinish() {
	//glMatrixMode(GL_MODELVIEW); // exist in opengl but doesn't work properly here
	tglPopMatrix();
}

void GfxTinyGL::enableLights() {
	tglEnable(TGL_LIGHTING);
}

void GfxTinyGL::disableLights() {
	tglDisable(TGL_LIGHTING);
}

void GfxTinyGL::setupLight(Light *light, int lightId) {
	assert(lightId < T_MAX_LIGHTS);
	tglEnable(TGL_LIGHTING);
	float lightColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightDir[] = { 0.0f, 0.0f, -1.0f };
	float cutoff = 180.0f;

	float intensity = light->_intensity / 1.3f;
	lightColor[0] = ((float)light->_color.getRed() / 15.0f) * intensity;
	lightColor[1] = ((float)light->_color.getGreen() / 15.0f) * intensity;
	lightColor[2] = ((float)light->_color.getBlue() / 15.0f) * intensity;

	if (light->_type == Light::Omni) {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
	} else if (light->_type ==  Light::Direct) {
		lightPos[0] = -light->_dir.x();
		lightPos[1] = -light->_dir.y();
		lightPos[2] = -light->_dir.z();
		lightPos[3] = 0;
	} else if (light->_type ==  Light::Spot) {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
		lightDir[0] = light->_dir.x();
		lightDir[1] = light->_dir.y();
		lightDir[2] = light->_dir.z();
		/* FIXME: TGL_SPOT_CUTOFF should be light->_penumbraangle, but there
		   seems to be a bug in tinygl as it renders differently from OpenGL.
		   Reproducing: turn off all lights (comment out), go to scene "al",
		   and walk along left wall under the lamp. */
		cutoff = 90.0f;
	}

	tglDisable(TGL_LIGHT0 + lightId);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_DIFFUSE, lightColor);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_POSITION, lightPos);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_SPOT_DIRECTION, lightDir);
	tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_CUTOFF, cutoff);
	tglEnable(TGL_LIGHT0 + lightId);
}

void GfxTinyGL::turnOffLight(int lightId) {
	tglDisable(TGL_LIGHT0 + lightId);
}

void GfxTinyGL::createBitmap(BitmapData *bitmap) {
	if (bitmap->_format == 1) {
		bitmap->convertToColorFormat(_pixelFormat);
	}	

	Graphics::BlitImage **imgs = new Graphics::BlitImage*[bitmap->_numImages];
	bitmap->_texIds = (void *)imgs;

	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint32 *buf = new uint32[bitmap->_width * bitmap->_height];
			uint16 *bufPtr = reinterpret_cast<uint16 *>(bitmap->getImageData(pic).getRawBuffer());
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bufPtr + i);
				// fix the value if it is incorrectly set to the bitmap transparency color
				if (val == 0xf81f) {
					val = 0;
				}
				buf[i] = ((uint32)val) * 0x10000 / 100 / (0x10000 - val) << 14;
			}
			delete[] bufPtr;
			imgs[pic] = Graphics::tglGenBlitImage();
			Graphics::Surface imgSurface;
			const Graphics::PixelBuffer imageBuffer(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), (byte *)buf);
			imgSurface.create(bitmap->_width, bitmap->_height, imageBuffer.getFormat());
			for (int y = 0; y < bitmap->_height; y++) {
				memcpy(imgSurface.getBasePtr(0, y), imageBuffer.getRawBuffer(y * bitmap->_width), imageBuffer.getFormat().bytesPerPixel * bitmap->_width);
				for(int x = 0; x < bitmap->_width; x++) {
					uint32 *pixelPtr = (uint32 *)imgSurface.getBasePtr(x, y);
					if (*pixelPtr != 0) {
						*pixelPtr |= (0xFF << 24); // Adding alpha channel value here.
					}
				}
			}
			Graphics::tglUploadBlitImage(imgs[pic], imgSurface, 0, false);
			imgSurface.free();
			bitmap->_data[pic] = Graphics::PixelBuffer(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), (byte *)buf);
		}
	} else {
		for (int i = 0; i < bitmap->_numImages; ++i) {
			imgs[i] = Graphics::tglGenBlitImage();
			Graphics::Surface imgSurface;
			const Graphics::PixelBuffer &imageBuffer = bitmap->getImageData(i);
			imgSurface.create(bitmap->_width, bitmap->_height, imageBuffer.getFormat());
			for (int y = 0; y < bitmap->_height; y++) {
				memcpy(imgSurface.getBasePtr(0, y), imageBuffer.getRawBuffer(y * bitmap->_width), imageBuffer.getFormat().bytesPerPixel * bitmap->_width);
			}
			Graphics::tglUploadBlitImage(imgs[i], imgSurface, 0xFFF800F8, true);
			imgSurface.free();
		}
	}
}

void GfxTinyGL::blit(const Graphics::PixelFormat &format, BlitImage *image, byte *dst, byte *src, int x, int y, int width, int height, bool trans) {
	int srcX, srcY;

	if (x < 0) {
		srcX = -x;
		x = 0;
	} else {
		srcX = 0;
	}

	if (y < 0) {
		srcY = -y;
		y = 0;
	} else {
		srcY = 0;
	}

	blit(format, image, dst, src, x, y, srcX, srcY, width, height, width, height, trans);
}

void GfxTinyGL::blit(const Graphics::PixelFormat &format, BlitImage *image, byte *dst, byte *src, int dstX, int dstY, int srcX, int srcY, int width, int height, int srcWidth, int srcHeight, bool trans) {
	if (_dimLevel > 0.0f && _dimLevel < 1.0f) {
		warning("TinyGL doesn't implement partial screen-dimming yet");
	}

	if (dstX >= _gameWidth || dstY >= _gameHeight)
		return;

	int clampWidth, clampHeight;

	if (dstX + width > _gameWidth)
		clampWidth = _gameWidth - dstX;
	else
		clampWidth = width;

	if (dstY + height > _gameHeight)
		clampHeight = _gameHeight - dstY;
	else
		clampHeight = height;

	dst += (dstX + (dstY * _gameWidth)) * format.bytesPerPixel;
	src += (srcX + (srcY * srcWidth)) * format.bytesPerPixel;

	Graphics::PixelBuffer srcBuf(format, src);
	Graphics::PixelBuffer dstBuf(format, dst);

	if (!trans) {
		for (int l = 0; l < clampHeight; l++) {
			dstBuf.copyBuffer(0, clampWidth, srcBuf);
			dstBuf.shiftBy(_gameWidth);
			srcBuf.shiftBy(srcWidth);
		}
	} else {
		for (int l = 0; l < clampHeight; l++) {
			for (int r = 0; r < clampWidth; ++r) {
				if (srcBuf.getValueAt(r) != 0xf81f) {
					dstBuf.setPixelAt(r, srcBuf);
				}
			}
			dstBuf.shiftBy(_gameWidth);
			srcBuf.shiftBy(srcWidth);
		}
	}
}

void GfxTinyGL::drawBitmap(const Bitmap *bitmap, int x, int y, uint32 layer) {

	// PS2 EMI uses a TGA for it's splash-screen, avoid using the following
	// code for drawing that (as it has no tiles).
	if (g_grim->getGameType() == GType_MONKEY4 && bitmap->_data->_numImages > 1) {
		tglColor3f(1.0f - _dimLevel, 1.0f - _dimLevel, 1.0f  - _dimLevel);

		BitmapData *data = bitmap->_data;
		float *texc = data->_texc;

		Graphics::BlitImage **b = (Graphics::BlitImage **)bitmap->getTexIds();

		uint32 offset = data->_layers[layer]._offset;
		for (uint32 i = offset; i < offset + data->_layers[layer]._numImages; ++i) {
			const BitmapData::Vert &v = data->_verts[i];
			uint32 texId = v._texid;
			uint32 ntex = data->_verts[i]._pos * 4;
			uint32 numRects = data->_verts[i]._verts / 4;
			while (numRects-- > 0) {
				// TODO: better way to fix this:
				// adding '+ 1' fixing broken lines at edges of bitmaps
				// example: EMI ship scene
				int dx1 = (((texc[ntex + 0] + 1) * _screenWidth) / 2) + 1;
				int dy1 = (((1 - texc[ntex + 1]) * _screenHeight) / 2) + 1;
				int dx2 = (((texc[ntex + 8] + 1) * _screenWidth) / 2) + 1;
				int dy2 = (((1 - texc[ntex + 9]) * _screenHeight) / 2) + 1;
				int srcX = texc[ntex + 2] * bitmap->getWidth();
				int srcY = texc[ntex + 3] * bitmap->getHeight();

				Graphics::BlitTransform transform(x + dx1, y + dy1);
				transform.sourceRectangle(srcX, srcY, dx2 - dx1, dy2 - dy1);
				transform.tint(_dimLevel);
				Graphics::tglBlit(b[texId], transform);
				ntex += 16;
			}
		}

		return;
	}

	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	assert(bitmap->getActiveImage() > 0);
	const int num = bitmap->getActiveImage() - 1;

	Graphics::BlitImage **b = (Graphics::BlitImage **)bitmap->getTexIds();

	if (bitmap->getFormat() == 1) {
		Graphics::BlitTransform transform(x, y);
		Graphics::tglBlit(b[num], transform);
	} else {
		blit(bitmap->getPixelFormat(num), nullptr, (byte *)_zb->zbuf, (byte *)bitmap->getData(num).getRawBuffer(),
			 x, y, bitmap->getWidth(), bitmap->getHeight(), false);
	}
}

void GfxTinyGL::destroyBitmap(BitmapData *bitmap) {
	for (int pic = 0; pic < bitmap->_numImages; pic++) {
		if (bitmap->_data)
			bitmap->_data[pic].free();
	}
	delete[] (BlitImage*)bitmap->_texIds;
}

void GfxTinyGL::createFont(Font *font) {
}

void GfxTinyGL::destroyFont(Font *font) {
}

struct TextObjectData {
	Graphics::BlitImage *image;
	int width, height, x, y;
};

void GfxTinyGL::createTextObject(TextObject *text) {
	int numLines = text->getNumLines();
	const Common::String *lines = text->getLines();
	const Font *font = text->getFont();
	const Color &fgColor = text->getFGColor();
	TextObjectData *userData = new TextObjectData[numLines];
	text->setUserData(userData);
	for (int j = 0; j < numLines; j++) {
		const Common::String &currentLine = lines[j];

		int width = font->getStringLength(currentLine) + 1;
		int height = font->getHeight();

		uint8 *_textBitmap = new uint8[height * width];
		memset(_textBitmap, 0, height * width);

		// Fill bitmap
		int startOffset = 0;
		for (unsigned int d = 0; d < currentLine.size(); d++) {
			int ch = currentLine[d];
			int8 startingLine = font->getCharStartingLine(ch) + font->getBaseOffsetY();
			int32 charDataWidth = font->getCharDataWidth(ch);
			int32 charWidth = font->getCharWidth(ch);
			int8 startingCol = font->getCharStartingCol(ch);
			for (int line = 0; line < font->getCharDataHeight(ch); line++) {
				int offset = startOffset + (width * (line + startingLine));
				for (int r = 0; r < charDataWidth; r++) {
					const byte pixel = *(font->getCharData(ch) + r + (charDataWidth * line));
					byte *dst = _textBitmap + offset + startingCol + r;
					if (*dst == 0 && pixel != 0)
						_textBitmap[offset + startingCol + r] = pixel;
				}
				if (line + startingLine >= font->getHeight())
					break;
			}
			startOffset += charWidth;
		}

		Graphics::PixelBuffer buf(_pixelFormat, width * height, DisposeAfterUse::YES);

		uint8 *bitmapData = _textBitmap;
		uint8 r = fgColor.getRed();
		uint8 g = fgColor.getGreen();
		uint8 b = fgColor.getBlue();
		uint32 color = _zb->cmode.RGBToColor(r, g, b);

		if (color == 0xf81f)
			color = 0xf81e;

		int txData = 0;
		for (int i = 0; i < width * height; i++, txData++, bitmapData++) {
			byte pixel = *bitmapData;
			if (pixel == 0x00) {
				buf.setPixelAt(txData, 0xf81f);
			} else if (pixel == 0x80) {
				buf.setPixelAt(txData, 0);
			} else if (pixel == 0xFF) {
				buf.setPixelAt(txData, color);
			}
		}

		userData[j].width = width;
		userData[j].height = height;

		Graphics::Surface sourceSurface;
		sourceSurface.setPixels(buf.getRawBuffer());
		sourceSurface.format = buf.getFormat();
		sourceSurface.w = width;
		sourceSurface.h = height;
		sourceSurface.pitch = sourceSurface.w * buf.getFormat().bytesPerPixel;
		userData[j].image = Graphics::tglGenBlitImage();
		Graphics::tglUploadBlitImage(userData[j].image, sourceSurface, 0xFFF800F8, true);
		userData[j].x = text->getLineX(j);
		userData[j].y = text->getLineY(j);

		if (g_grim->getGameType() == GType_MONKEY4) {
			userData[j].y -= font->getBaseOffsetY();
			if (userData[j].y < 0)
				userData[j].y = 0;
		}

		delete[] _textBitmap;
	}
}

void GfxTinyGL::drawTextObject(const TextObject *text) {
	const TextObjectData *userData = (const TextObjectData *)text->getUserData();
	if (userData) {
		int numLines = text->getNumLines();
		for (int i = 0; i < numLines; ++i) {
			Graphics::BlitTransform transform(userData[i].x, userData[i].y);
			Graphics::tglBlit(userData[i].image, transform);
		}
	}
}

void GfxTinyGL::destroyTextObject(TextObject *text) {
	const TextObjectData *userData = (const TextObjectData *)text->getUserData();
	if (userData) {
		int numLines = text->getNumLines();
		for (int i = 0; i < numLines; ++i) {
			Graphics::tglDeleteBlitImage(userData[i].image);
		}
		delete[] userData;
	}
}

void GfxTinyGL::createTexture(Texture *texture, const char *data, const CMap *cmap, bool clamp) {
	texture->_texture = new TGLuint[1];
	tglGenTextures(1, (TGLuint *)texture->_texture);
	uint8 *texdata = new uint8[texture->_width * texture->_height * 4];
	uint8 *texdatapos = texdata;

	if (cmap != nullptr) { // EMI doesn't have colour-maps
		for (int y = 0; y < texture->_height; y++) {
			for (int x = 0; x < texture->_width; x++) {
				uint8 col = *data;
				if (col == 0) {
					memset(texdatapos, 0, 4); // transparent
					if (!texture->_hasAlpha) {
						texdatapos[3] = '\xff'; // fully opaque
					}
				} else {
					memcpy(texdatapos, cmap->_colors + 3 * (col), 3);
					texdatapos[3] = '\xff'; // fully opaque
				}
				texdatapos += 4;
				data++;
			}
		}
	} else {
		memcpy(texdata, data, texture->_width * texture->_height * texture->_bpp);
	}

	TGLuint format = 0;
//	TGLuint internalFormat = 0;
	if (texture->_colorFormat == BM_RGBA) {
		format = TGL_RGBA;
//		internalFormat = TGL_RGBA;
	} else if (texture->_colorFormat == BM_BGRA) {
		format = TGL_BGRA;
	} else {    // The only other colorFormat we load right now is BGR
		format = TGL_BGR;
//		internalFormat = TGL_RGB;
	}

	TGLuint *textures = (TGLuint *)texture->_texture;
	tglBindTexture(TGL_TEXTURE_2D, textures[0]);

	// TinyGL doesn't have issues with dark lines in EMI intro so doesn't need TGL_CLAMP_TO_EDGE
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_REPEAT);

	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	tglTexImage2D(TGL_TEXTURE_2D, 0, 3, texture->_width, texture->_height, 0, format, TGL_UNSIGNED_BYTE, texdata);
	delete[] texdata;
}

void GfxTinyGL::selectTexture(const Texture *texture) {
	TGLuint *textures = (TGLuint *)texture->_texture;
	tglBindTexture(TGL_TEXTURE_2D, textures[0]);
	
	if (texture->_hasAlpha && g_grim->getGameType() == GType_MONKEY4) {
		tglEnable(TGL_BLEND);
	}	

	// Grim has inverted tex-coords, EMI doesn't
	if (g_grim->getGameType() != GType_MONKEY4) {
		tglPushMatrix(); // removed in opengl but here doesn't work properly after remove
		tglMatrixMode(TGL_TEXTURE);
		tglLoadIdentity();
		tglScalef(1.0f / texture->_width, 1.0f / texture->_height, 1);
		tglMatrixMode(TGL_MODELVIEW); // removed in opengl but here doesn't work properly after remove
		tglPopMatrix(); // removed in opengl but here doesn't work properly after remove
	}
}

void GfxTinyGL::destroyTexture(Texture *texture) {
	tglDeleteTextures(1, (TGLuint *)texture->_texture);
	delete[] (TGLuint *)texture->_texture;
}

void GfxTinyGL::prepareMovieFrame(Graphics::Surface *frame) {
	_smushImage = Graphics::tglGenBlitImage();
	Graphics::tglUploadBlitImage(_smushImage, *frame, 0, false);
}

void GfxTinyGL::drawMovieFrame(int offsetX, int offsetY) {
	Graphics::tglBlitFast(_smushImage, offsetX, offsetY);
}

void GfxTinyGL::releaseMovieFrame() {
	Graphics::tglDeleteBlitImage(_smushImage);
}

void GfxTinyGL::loadEmergFont() {
}

void GfxTinyGL::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	uint32 color = _pixelFormat.RGBToColor(fgColor.getRed(), fgColor.getGreen(), fgColor.getBlue());

	int length = strlen(text);

	for (int l = 0; l < length; l++) {
		int c = text[l];
		assert(c >= 32 && c <= 127);
		const uint8 *ptr = Font::emerFont[c - 32];
		for (int py = 0; py < 13; py++) {
			if ((py + y) < _gameHeight && (py + y) >= 0) {
				int line = ptr[12 - py];
				for (int px = 0; px < 8; px++) {
					if ((px + x) < _gameWidth && (px + x) >= 0) {
						int pixel = line & 0x80;
						line <<= 1;
						if (pixel) {
							_zb->writePixel(((py + y) * _gameWidth) + (px + x), color);
						}
					}
				}
			}
		}
		x += 10;
	}
}

Bitmap *GfxTinyGL::getScreenshot(int w, int h) {
	Graphics::PixelBuffer buffer = Graphics::PixelBuffer::createBuffer<565>(w * h, DisposeAfterUse::YES);

	int i1 = (_gameWidth * w - 1) / _gameWidth + 1;
	int j1 = (_gameHeight * h - 1) / _gameHeight + 1;

	for (int j = 0; j < j1; j++) {
		for (int i = 0; i < i1; i++) {
			int x0 = i * _gameWidth / w;
			int x1 = ((i + 1) * _gameWidth - 1) / w + 1;
			int y0 = j * _gameHeight / h;
			int y1 = ((j + 1) * _gameHeight - 1) / h + 1;
			uint32 color = 0;
			for (int y = y0; y < y1; y++) {
				for (int x = x0; x < x1; x++) {
					uint8 lr, lg, lb;
					_zb->readPixelRGB(y * _gameWidth + x, lr, lg, lb);
					color += (lr + lg + lb) / 3;
				}
			}
			color /= (x1 - x0) * (y1 - y0);
			buffer.setPixelAt(j * w + i, color, color, color);
		}
	}

	Bitmap *screenshot = new Bitmap(buffer, w, h, "screenshot");
	return screenshot;
}

void GfxTinyGL::storeDisplay() {
	_zb->copyToBuffer(_storedDisplay);
}

void GfxTinyGL::copyStoredToDisplay() {
	_zb->copyFromBuffer(_storedDisplay);
}

void GfxTinyGL::dimScreen() {
	for (int l = 0; l < _gameWidth * _gameHeight; l++) {
		uint8 r, g, b;
		_storedDisplay.getRGBAt(l, r, g, b);
		uint32 color = (r + g + b) / 10;
		_storedDisplay.setPixelAt(l, color, color, color);
	}
}

void GfxTinyGL::dimRegion(int x, int y, int w, int h, float level) {
	for (int ly = y; ly < y + h; ly++) {
		for (int lx = x; lx < x + w; lx++) {
			uint8 r, g, b;
			_zb->readPixelRGB(ly * _gameWidth + lx, r, g, b);
			uint32 color = (uint32)(((r + g + b) / 3) * level);
			_zb->writePixel(ly * _gameWidth + lx, color, color, color);
		}
	}
}

void GfxTinyGL::irisAroundRegion(int x1, int y1, int x2, int y2) {
	for (int ly = 0; ly < _gameHeight; ly++) {
		for (int lx = 0; lx < _gameWidth; lx++) {
			// Don't do anything with the data in the region we draw Around
			if (lx > x1 && lx < x2 && ly > y1 && ly < y2)
				continue;
			// But set everything around it to black.
			_zb->writePixel(ly * _gameWidth + lx, 0);
		}
	}
}

void GfxTinyGL::drawRectangle(const PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = primitive->getColor();
	uint32 c = _pixelFormat.RGBToColor(color.getRed(), color.getGreen(), color.getBlue());

	if (primitive->isFilled()) {
		for (; y1 <= y2; y1++)
			if (y1 >= 0 && y1 < _gameHeight)
				for (int x = x1; x <= x2; x++)
					if (x >= 0 && x < _gameWidth)
						_zb->writePixel(_gameWidth * y1 + x, c);
	} else {
		if (y1 >= 0 && y1 < _gameHeight)
			for (int x = x1; x <= x2; x++)
				if (x >= 0 && x < _gameWidth)
					_zb->writePixel(_gameWidth * y1 + x, c);
		if (y2 >= 0 && y2 < _gameHeight)
			for (int x = x1; x <= x2; x++)
				if (x >= 0 && x < _gameWidth)
					_zb->writePixel(_gameWidth * y2 + x, c);
		if (x1 >= 0 && x1 < _gameWidth)
			for (int y = y1; y <= y2; y++)
				if (y >= 0 && y < _gameHeight)
					_zb->writePixel(_gameWidth * y + x1, c);
		if (x2 >= 0 && x2 < _gameWidth)
			for (int y = y1; y <= y2; y++)
				if (y >= 0 && y < _gameHeight)
					_zb->writePixel(_gameWidth * y + x2, c);
	}
}

void GfxTinyGL::drawLine(const PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = primitive->getColor();

	if (x2 == x1) {
		for (int y = y1; y <= y2; y++) {
			if (x1 >= 0 && x1 < _gameWidth && y >= 0 && y < _gameHeight)
				_zb->writePixel(_gameWidth * y + x1, color.getRed(), color.getGreen(), color.getBlue());
		}
	} else {
		float m = (y2 - y1) / (x2 - x1);
		int b = (int)(-m * x1 + y1);
		for (int x = x1; x <= x2; x++) {
			int y = (int)(m * x) + b;
			if (x >= 0 && x < _gameWidth && y >= 0 && y < _gameHeight)
				_zb->writePixel(_gameWidth * y + x, color.getRed(), color.getGreen(), color.getBlue());
		}
	}
}

void GfxTinyGL::drawPolygon(const PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;
	int x3 = primitive->getP3().x;
	int y3 = primitive->getP3().y;
	int x4 = primitive->getP4().x;
	int y4 = primitive->getP4().y;
	float m;
	int b;

	const Color &color = primitive->getColor();
	uint32 c = _pixelFormat.RGBToColor(color.getRed(), color.getGreen(), color.getBlue());

	m = (y2 - y1) / (x2 - x1);
	b = (int)(-m * x1 + y1);
	for (int x = x1; x <= x2; x++) {
		int y = (int)(m * x) + b;
		if (x >= 0 && x < _gameWidth && y >= 0 && y < _gameHeight)
			_zb->writePixel(_gameWidth * y + x, c);
	}
	m = (y4 - y3) / (x4 - x3);
	b = (int)(-m * x3 + y3);
	for (int x = x3; x <= x4; x++) {
		int y = (int)(m * x) + b;
		if (x >= 0 && x < _gameWidth && y >= 0 && y < _gameHeight)
			_zb->writePixel(_gameWidth * y + x, c);
	}
}

void GfxTinyGL::readPixels(int x, int y, int width, int height, uint8 *buffer) {
	uint8 r, g, b;
	int pos = x + y * 640;
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			_zb->readPixelRGB(pos + j, r, g, b);
			buffer[0] = r;
			buffer[1] = g;
			buffer[2] = b;
			buffer[3] = 255;
			buffer += 4;
		}
		pos += 640;
	}
}

void GfxTinyGL::createSpecialtyTextures() {
	//make a buffer big enough to hold any of the textures
	uint8 *buffer = new uint8[256 * 256 * 4];

	readPixels(0, 0, 256, 256, buffer);
	_specialty[0].create((const char *)buffer, 256, 256);

	readPixels(256, 0, 256, 256, buffer);
	_specialty[1].create((const char *)buffer, 256, 256);

	readPixels(512, 0, 128, 128, buffer);
	_specialty[2].create((const char *)buffer, 128, 128);

	readPixels(512, 128, 128, 128, buffer);
	_specialty[3].create((const char *)buffer, 128, 128);

	readPixels(0, 256, 256, 224, buffer);
	_specialty[4].create((const char *)buffer, 256, 256);

	readPixels(256, 256, 256, 224, buffer);
	_specialty[5].create((const char *)buffer, 256, 256);

	readPixels(512, 256, 128, 128, buffer);
	_specialty[6].create((const char *)buffer, 128, 128);

	readPixels(512, 384, 128, 96, buffer);
	_specialty[7].create((const char *)buffer, 128, 128);

	delete[] buffer;
}

} // end of namespace Grim
