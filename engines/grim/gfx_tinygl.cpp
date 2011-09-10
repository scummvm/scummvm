/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/endian.h"
#include "common/system.h"

#include "engines/grim/actor.h"
#include "engines/grim/colormap.h"
#include "engines/grim/material.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_tinygl.h"
#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/primitives.h"
#include "engines/grim/model.h"
#include "engines/grim/scene.h"

namespace Grim {

GfxBase *CreateGfxTinyGL() {
	return new GfxTinyGL();
}

// below funcs lookAt, transformPoint and tgluProject are from Mesa glu sources
static void lookAt(TGLfloat eyex, TGLfloat eyey, TGLfloat eyez, TGLfloat centerx,
		TGLfloat centery, TGLfloat centerz, TGLfloat upx, TGLfloat upy, TGLfloat upz) {
	TGLfloat m[16];
	TGLfloat x[3], y[3], z[3];
	TGLfloat mag;

	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) {
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];

	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];

	mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

#define M(row,col)  m[col * 4 + row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0f;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0f;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0f;
	M(3, 0) = 0.0f;
	M(3, 1) = 0.0f;
	M(3, 2) = 0.0f;
	M(3, 3) = 1.0f;
#undef M
	tglMultMatrixf(m);

	tglTranslatef(-eyex, -eyey, -eyez);
}

static void transformPoint(TGLfloat out[4], const TGLfloat m[16], const TGLfloat in[4]) {
#define M(row,col)  m[col * 4 + row]
	out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
	out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
	out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
	out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

TGLint tgluProject(TGLfloat objx, TGLfloat objy, TGLfloat objz, const TGLfloat model[16], const TGLfloat proj[16],
		const TGLint viewport[4], TGLfloat *winx, TGLfloat *winy, TGLfloat *winz) {
	TGLfloat in[4], out[4];

	in[0] = objx;
	in[1] = objy;
	in[2] = objz;
	in[3] = 1.0f;
	transformPoint(out, model, in);
	transformPoint(in, proj, out);

	if (in[3] == 0.0)
		return TGL_FALSE;

	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];

	*winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
	*winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;
	*winz = (1 + in[2]) / 2;

	return TGL_TRUE;
}

GfxTinyGL::GfxTinyGL() {
	g_driver = this;
	_zb = NULL;
	_storedDisplay = NULL;
}

GfxTinyGL::~GfxTinyGL() {
	delete[] _storedDisplay;
	if (_zb) {
		TinyGL::glClose();
		ZB_close(_zb);
	}
}

byte *GfxTinyGL::setupScreen(int screenW, int screenH, bool fullscreen) {
	byte *buffer = g_system->setupScreen(screenW, screenH, fullscreen, false);

	_screenWidth = screenW;
	_screenHeight = screenH;
	_screenBPP = 15;
	_isFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);

	g_system->showMouse(!fullscreen);

	g_system->setWindowCaption("Residual: Software 3D Renderer");

	_zb = TinyGL::ZB_open(screenW, screenH, ZB_MODE_5R6G5B, buffer);
	TinyGL::glInit(_zb);

	_storedDisplay = new byte[640 * 480 * 2];
	memset(_storedDisplay, 0, 640 * 480 * 2);

	_currentShadowArray = NULL;

	TGLfloat ambientSource[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	tglLightModelfv(TGL_LIGHT_MODEL_AMBIENT, ambientSource);

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

	tglRotatef(roll, 0, 0, -1);
}

void GfxTinyGL::positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest) {
	Graphics::Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Graphics::Vector3d(0, 1, 0);

	lookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
}

void GfxTinyGL::clearScreen() {
	memset(_zb->pbuf, 0, 640 * 480 * 2);
	memset(_zb->zbuf, 0, 640 * 480 * 2);
	memset(_zb->zbuf2, 0, 640 * 480 * 4);
}

void GfxTinyGL::flipBuffer() {
	g_system->updateScreen();
}

bool GfxTinyGL::isHardwareAccelerated() {
	return false;
}

static void tglShadowProjection(Graphics::Vector3d light, Graphics::Vector3d plane, Graphics::Vector3d normal, bool dontNegate) {
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
	TGLfloat winX, winY, winZ;

	for (int i = 0; i < model->_numFaces; i++) {
		Graphics::Vector3d v;
		float* pVertices;

		for (int j = 0; j < model->_faces[i]._numVertices; j++) {
			TGLfloat modelView[16], projection[16];
			TGLint viewPort[4];

			tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView);
			tglGetFloatv(TGL_PROJECTION_MATRIX, projection);
			tglGetIntegerv(TGL_VIEWPORT, viewPort);

			pVertices = model->_vertices + 3 * model->_faces[i]._vertices[j];

			v.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

			tgluProject(v.x(), v.y(), v.z(), modelView, projection, viewPort, &winX, &winY, &winZ);

			if (winX > right)
				right = winX;
			if (winX < left)
				left = winX;
			if (winY < top)
				top = winY;
			if (winY > bottom)
				bottom = winY;
		}
	}

	float t = bottom;
	bottom = 480 - top;
	top = 480 - t;

	if (left < 0)
		left = 0;
	if (right > 639)
		right = 639;
	if (top < 0)
		top = 0;
	if (bottom > 479)
		bottom = 479;

	if (top > 479 || left > 639 || bottom < 0 || right < 0) {
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
/*
	uint16 *dst = (uint16 *)_zb->pbuf;
	uint16 c = 0xffff;
	for (int x = left; x <= right; x++) {
		WRITE_LE_UINT16(dst + 640 * (int)top + x, c);
	}
	for (int x = left; x <= right; x++) {
		WRITE_LE_UINT16(dst + 640 * (int)bottom + x, c);
	}
	for (int y = top; y <= bottom; y++) {
		WRITE_LE_UINT16(dst + 640 * y + (int)left, c);
	}
	for (int y = top; y <= bottom; y++) {
		WRITE_LE_UINT16(dst + 640 * y + (int)right, c);
	}*/
}

void GfxTinyGL::startActorDraw(Graphics::Vector3d pos, float scale, float yaw, float pitch, float roll) {
	tglEnable(TGL_TEXTURE_2D);
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	if (_currentShadowArray) {
		// TODO find out why shadowMask at device in woods is null
		if (!_currentShadowArray->shadowMask) {
			_currentShadowArray->shadowMask = new byte[_screenWidth * _screenHeight];
			_currentShadowArray->shadowMaskSize = _screenWidth * _screenHeight;
		}
		assert(_currentShadowArray->shadowMask);
		//tglSetShadowColor(255, 255, 255);
		tglSetShadowColor(_shadowColorR, _shadowColorG, _shadowColorB);
		tglSetShadowMaskBuf(_currentShadowArray->shadowMask);
		SectorListType::iterator i = _currentShadowArray->planeList.begin();
		Sector *shadowSector = i->sector;
		tglShadowProjection(_currentShadowArray->pos, shadowSector->getVertices()[0], shadowSector->getNormal(), _currentShadowArray->dontNegate);
	}

	tglTranslatef(pos.x(), pos.y(), pos.z());
	tglScalef(scale, scale, scale);
	tglRotatef(yaw, 0, 0, 1);
	tglRotatef(pitch, 1, 0, 0);
	tglRotatef(roll, 0, 1, 0);
}

void GfxTinyGL::finishActorDraw() {
	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
	tglDisable(TGL_TEXTURE_2D);

	if (_currentShadowArray) {
		tglSetShadowMaskBuf(NULL);
	}/* else {
		uint16 *dst = (uint16 *)_zb->pbuf;
		uint16 c = 0xffff;
		for (int x = g_winX1; x <= g_winX2; x++) {
			WRITE_LE_UINT16(dst + 640 * g_winY1 + x, c);
		}
		for (int x = g_winX1; x <= g_winX2; x++) {
			WRITE_LE_UINT16(dst + 640 * g_winY2 + x, c);
		}
		for (int y = g_winY1; y <= g_winY2; y++) {
			WRITE_LE_UINT16(dst + 640 * y + g_winX1, c);
		}
		for (int y = g_winY1; y <= g_winY2; y++) {
			WRITE_LE_UINT16(dst + 640 * y + g_winX2, c);
		}
	}*/
}

void GfxTinyGL::drawShadowPlanes() {
	tglEnable(TGL_SHADOW_MASK_MODE);
	if (!_currentShadowArray->shadowMask) {
		_currentShadowArray->shadowMask = new byte[_screenWidth * _screenHeight];
		_currentShadowArray->shadowMaskSize = _screenWidth * _screenHeight;
	}
	memset(_currentShadowArray->shadowMask, 0, _screenWidth * _screenHeight);

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
	tglSetShadowMaskBuf(NULL);
	tglDisable(TGL_SHADOW_MASK_MODE);
}

void GfxTinyGL::setShadowMode() {
	tglEnable(TGL_SHADOW_MODE);
}

void GfxTinyGL::clearShadowMode() {
	tglDisable(TGL_SHADOW_MODE);
}

void GfxTinyGL::set3DMode() {
	tglMatrixMode(TGL_MODELVIEW);
	tglEnable(TGL_DEPTH_TEST);
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

void GfxTinyGL::drawModelFace(const MeshFace *face, float *vertices, float *vertNormals, float *textureVerts) {
	tglNormal3fv(const_cast<float *>(face->_normal._coords));
	tglBegin(TGL_POLYGON);
	for (int i = 0; i < face->_numVertices; i++) {
		tglNormal3fv(vertNormals + 3 * face->_vertices[i]);

		if (face->_texVertices)
			tglTexCoord2fv(textureVerts + 2 * face->_texVertices[i]);

		tglVertex3fv(vertices + 3 * face->_vertices[i]);
	}
	tglEnd();
}

void GfxTinyGL::drawSprite(const Sprite *sprite) {
	tglMatrixMode(TGL_TEXTURE);
	tglLoadIdentity();
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
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

	tglDisable(TGL_LIGHTING);

	tglBegin(TGL_POLYGON);
	tglTexCoord2f(0.0f, 0.0f);
	tglVertex3f(sprite->_width / 2, sprite->_height, 0.0f);
	tglTexCoord2f(0.0f, 1.0f);
	tglVertex3f(sprite->_width / 2, 0.0f, 0.0f);
	tglTexCoord2f(1.0f, 1.0f);
	tglVertex3f(-sprite->_width / 2, 0.0f, 0.0f);
	tglTexCoord2f(1.0f, 0.0f);
	tglVertex3f(-sprite->_width / 2, sprite->_height, 0.0f);
	tglEnd();

	tglEnable(TGL_LIGHTING);

	tglPopMatrix();
}

void GfxTinyGL::translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll) {
	tglPushMatrix();

	tglTranslatef(pos.x(), pos.y(), pos.z());
	tglRotatef(yaw, 0, 0, 1);
	tglRotatef(pitch, 1, 0, 0);
	tglRotatef(roll, 0, 1, 0);
}

void GfxTinyGL::translateViewpointFinish() {
	tglPopMatrix();
}

void GfxTinyGL::drawHierachyNode(const ModelNode *node, int *x1, int *y1, int *x2, int *y2) {
	Graphics::Vector3d animPos = node->_pos + node->_animPos;
	float animPitch = node->_pitch + node->_animPitch;
	float animYaw = node->_yaw + node->_animYaw;
	float animRoll = node->_roll + node->_animRoll;
	translateViewpointStart(animPos, animPitch, animYaw, animRoll);
	if (node->_hierVisible) {
		tglPushMatrix();
		tglTranslatef(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());

		if (!_currentShadowArray) {
			Sprite* sprite = node->_sprite;
			while (sprite) {
				sprite->draw();
				sprite = sprite->_next;
			}
		}

		if (node->_mesh && node->_meshVisible) {
			node->_mesh->draw(x1, y1, x2, y2);
		}

		tglMatrixMode(TGL_MODELVIEW);
		tglPopMatrix();

		if (node->_child) {
			node->_child->draw(x1, y1, x2, y2);
			tglMatrixMode(TGL_MODELVIEW);
		}
	}
	translateViewpointFinish();

	if (node->_sibling)
		node->_sibling->draw(x1, y1, x2, y2);
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

	if (light->_type == "omni") {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
	} else if (light->_type == "direct") {
		lightPos[0] = -light->_dir.x();
		lightPos[1] = -light->_dir.y();
		lightPos[2] = -light->_dir.z();
		lightPos[3] = 0;
	} else if (light->_type == "spot") {
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
	} else {
		error("Scene::setupLights() Unknown type of light: %s", light->_type.c_str());
		return;
	}
	tglDisable(TGL_LIGHT0 + lightId);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_DIFFUSE, lightColor);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_POSITION, lightPos);
	tglLightfv(TGL_LIGHT0 + lightId, TGL_SPOT_DIRECTION, lightDir);
	tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_CUTOFF, cutoff);
	tglEnable(TGL_LIGHT0 + lightId);
}

void GfxTinyGL::createBitmap(BitmapData *bitmap) {
	// We want an RGB565-bitmap in TinyGL.
	if (bitmap->_colorFormat != BM_RGB565) {
		bitmap->convertToColorFormat(0, BM_RGB565);
	}
	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *bufPtr = reinterpret_cast<uint16 *>(bitmap->getImageData(pic));
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bitmap->getImageData(pic) + 2 * i);
				bufPtr[i] = ((uint32) val) * 0x10000 / 100 / (0x10000 - val);
			}
		}
	}
}

void TinyGLBlit(byte *dst, byte *src, int x, int y, int width, int height, bool trans) {
	int srcPitch = width * 2;
	int dstPitch = 640 * 2;
	int srcX, srcY;
	int l, r;

	if (x > 639 || y > 479)
		return;

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

	if (x + width > 640)
		width -= (x + width) - 640;

	if (y + height > 480)
		height -= (y + height) - 480;

	dst += (x + (y * 640)) * 2;
	src += (srcX + (srcY * width)) * 2;

	int copyWidth = width * 2;

	if (!trans) {
		for (l = 0; l < height; l++) {
			memcpy(dst, src, copyWidth);
			dst += dstPitch;
			src += srcPitch;
		}
	} else {
		for (l = 0; l < height; l++) {
			for (r = 0; r < copyWidth; r += 2) {
				uint16 pixel = READ_UINT16(src + r);
				if (pixel != 0xf81f)
					WRITE_UINT16(dst + r, pixel);
			}
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

void GfxTinyGL::drawBitmap(const Bitmap *bitmap) {
	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	assert(bitmap->getCurrentImage() > 0);
	if (bitmap->getFormat() == 1)
		TinyGLBlit((byte *)_zb->pbuf, (byte *)bitmap->getData(bitmap->getCurrentImage() - 1),
			bitmap->getX(), bitmap->getY(), bitmap->getWidth(), bitmap->getHeight(), true);
	else
		TinyGLBlit((byte *)_zb->zbuf, (byte *)bitmap->getData(bitmap->getCurrentImage() - 1),
			bitmap->getX(), bitmap->getY(), bitmap->getWidth(), bitmap->getHeight(), false);
}

void GfxTinyGL::destroyBitmap(BitmapData *) { }

void GfxTinyGL::createFont(Font *font) {
}

void GfxTinyGL::destroyFont(Font *font) {
}

struct TextObjectData {
	byte *data;
	int width, height, x, y;
};

void GfxTinyGL::createTextObject(TextObject *text) {
	int numLines = text->getNumLines();
	const Common::String *lines = text->getLines();
	const Font *font = text->getFont();
	const Color *fgColor = text->getFGColor();
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

		uint16 *texData = new uint16[width * height];
		uint16 *texDataPtr = texData;
		uint8 *bitmapData = _textBitmap;
		uint8 r = fgColor->getRed();
		uint8 g = fgColor->getGreen();
		uint8 b = fgColor->getBlue();
		uint16 color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		if (color == 0xf81f)
			color = 0xf81e;

		for (int i = 0; i < width * height; i++, texDataPtr++, bitmapData++) {
			byte pixel = *bitmapData;
			if (pixel == 0x00) {
				WRITE_UINT16(texDataPtr, 0xf81f);
			} else if (pixel == 0x80) {
				*texDataPtr = 0;
			} else if (pixel == 0xFF) {
				WRITE_UINT16(texDataPtr, color);
			}
		}


		userData[j].width = width;
		userData[j].height = height;
		userData[j].data = (byte *)texData;
		userData[j].x = text->getLineX(j);
		userData[j].y = text->getLineY(j);


		delete[] _textBitmap;
	}
}

void GfxTinyGL::drawTextObject(TextObject *text) {
	TextObjectData *userData = (TextObjectData *)text->getUserData();
	if (userData) {
		int numLines = text->getNumLines();
		for (int i = 0; i < numLines; ++i) {
			TinyGLBlit((byte *)_zb->pbuf, userData[i].data, userData[i].x, userData[i].y, userData[i].width, userData[i].height, true);
		}
	}

}

void GfxTinyGL::destroyTextObject(TextObject *text) {
	TextObjectData *userData = (TextObjectData *)text->getUserData();
	if (userData) {
		int numLines = text->getNumLines();
		for (int i = 0; i < numLines; ++i) {
			delete[] userData[i].data;
		}
		delete[] userData;
	}
}

void GfxTinyGL::createMaterial(Texture *material, const char *data, const CMap *cmap) {
	material->_texture = new TGLuint[1];
	tglGenTextures(1, (TGLuint *)material->_texture);
	char *texdata = new char[material->_width * material->_height * 4];
	char *texdatapos = texdata;
	for (int y = 0; y < material->_height; y++) {
		for (int x = 0; x < material->_width; x++) {
			uint8 col = *(uint8 *)(data);
			if (col == 0) {
				memset(texdatapos, 0, 4); // transparent
				if (!material->_hasAlpha) {
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
	TGLuint *textures = (TGLuint *)material->_texture;
	tglBindTexture(TGL_TEXTURE_2D, textures[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_REPEAT);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	tglTexImage2D(TGL_TEXTURE_2D, 0, 3, material->_width, material->_height, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, texdata);
	delete[] texdata;
}

void GfxTinyGL::selectMaterial(const Texture *material) {
	TGLuint *textures = (TGLuint *)material->_texture;
	tglBindTexture(TGL_TEXTURE_2D, textures[0]);
	tglPushMatrix();
	tglMatrixMode(TGL_TEXTURE);
	tglLoadIdentity();
	tglScalef(1.0f / material->_width, 1.0f / material->_height, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
}

void GfxTinyGL::destroyMaterial(Texture *material) {
	tglDeleteTextures(1, (TGLuint *)material->_texture);
	delete[] (TGLuint *)material->_texture;
}

void GfxTinyGL::prepareMovieFrame(int width, int height, byte *bitmap) {
	_smushWidth = width;
	_smushHeight = height;
	_smushBitmap = bitmap;
}

void GfxTinyGL::drawMovieFrame(int offsetX, int offsetY) {
	if (_smushWidth == 640 && _smushHeight == 480) {
		memcpy(_zb->pbuf, _smushBitmap, 640 * 480 * 2);
	} else {
		TinyGLBlit((byte *)_zb->pbuf, _smushBitmap, offsetX, offsetY, _smushWidth, _smushHeight, false);
	}
}

void GfxTinyGL::releaseMovieFrame() {
}

void GfxTinyGL::loadEmergFont() {
}

void GfxTinyGL::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	uint16 color = ((fgColor.getRed() & 0xF8) << 8) | ((fgColor.getGreen() & 0xFC) << 3) | (fgColor.getBlue() >> 3);

	for (int l = 0; l < (int)strlen(text); l++) {
		int c = text[l];
		assert(c >= 32 && c <= 127);
		const uint8 *ptr = Font::emerFont[c - 32];
		for (int py = 0; py < 13; py++) {
			if ((py + y) < 480 && (py + y) >= 0) {
				int line = ptr[12 - py];
				for (int px = 0; px < 8; px++) {
					if ((px + x) < 640 && (px + x) >= 0) {
						int pixel = line & 0x80;
						line <<= 1;
						if (pixel)
							WRITE_UINT16(_zb->pbuf + ((py + y) * 640) + (px + x), color);
					}
				}
			}
		}
		x += 10;
	}
}

Bitmap *GfxTinyGL::getScreenshot(int w, int h) {
	uint16 *buffer = new uint16[w * h];
	uint16 *src = (uint16 *)_storedDisplay;
	assert(buffer);

	int step = 0;
	for (int y = 0; y <= 479; y++) {
		for (int x = 0; x <= 639; x++) {
			uint16 pixel = *(src + y * 640 + x);
			uint8 r = (pixel & 0xF800) >> 8;
			uint8 g = (pixel & 0x07E0) >> 3;
			uint8 b = (pixel & 0x001F) << 3;
			uint32 color = (r + g + b) / 3;
			src[step++] = ((color & 0xF8) << 8) | ((color & 0xFC) << 3) | (color >> 3);
		}
	}

	float step_x = 640.0f / w;
	float step_y = 480.0f / h;
	step = 0;
	for (float y = 0; y < 479; y += step_y) {
		for (float x = 0; x < 639; x += step_x) {
			uint16 pixel = *(src + (int)y * 640 + (int)x);
			buffer[step++] = pixel;
		}
	}

	Bitmap *screenshot = new Bitmap((char *)buffer, w, h, 16, "screenshot");
	delete[] buffer;
	return screenshot;
}

void GfxTinyGL::storeDisplay() {
	memcpy(_storedDisplay, _zb->pbuf, 640 * 480 * 2);
}

void GfxTinyGL::copyStoredToDisplay() {
	memcpy(_zb->pbuf, _storedDisplay, 640 * 480 * 2);
}

void GfxTinyGL::dimScreen() {
	uint16 *data = (uint16 *)_storedDisplay;
	for (int l = 0; l < 640 * 480; l++) {
		uint16 pixel = data[l];
		uint8 r = (pixel & 0xF800) >> 8;
		uint8 g = (pixel & 0x07E0) >> 3;
		uint8 b = (pixel & 0x001F) << 3;
		uint32 color = (r + g + b) / 10;
		data[l] = ((color & 0xF8) << 8) | ((color & 0xFC) << 3) | (color >> 3);
	}
}

void GfxTinyGL::dimRegion(int x, int y, int w, int h, float level) {
	uint16 *data = (uint16 *)_zb->pbuf;
	for (int ly = y; ly < y + h; ly++) {
		for (int lx = x; lx < x + w; lx++) {
			uint16 pixel = data[ly * 640 + lx];
			uint8 r = (pixel & 0xF800) >> 8;
			uint8 g = (pixel & 0x07E0) >> 3;
			uint8 b = (pixel & 0x001F) << 3;
			uint16 color = (uint16)(((r + g + b) / 3) * level);
			data[ly * 640 + lx] = ((color & 0xF8) << 8) | ((color & 0xFC) << 3) | (color >> 3);
		}
	}
}

void GfxTinyGL::irisAroundRegion(int x1, int y1, int x2, int y2) {
	uint16 *data = (uint16 *)_zb->pbuf;
	for (int ly = 0; ly < _screenHeight; ly++) {
		for (int lx = 0; lx < _screenWidth; lx++) {
			// Don't do anything with the data in the region we draw Around
			if(lx > x1 && lx < x2 && ly > y1 && ly < y2)
				continue;
			// But set everything around it to black.
			data[ly * 640 + lx] = (uint16)0.0f;
		}
	}
}

void GfxTinyGL::drawRectangle(PrimitiveObject *primitive) {
	uint16 *dst = (uint16 *)_zb->pbuf;
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = *primitive->getColor();
	uint16 c = ((color.getRed() & 0xF8) << 8) | ((color.getGreen() & 0xFC) << 3) | (color.getBlue() >> 3);

	if (primitive->isFilled()) {
		for (; y1 <= y2; y1++)
			if (y1 >= 0 && y1 < 480)
				for (int x = x1; x <= x2; x++)
					if (x >= 0 && x < 640)
						WRITE_UINT16(dst + 640 * y1 + x, c);
	} else {
		if (y1 >= 0 && y1 < 480)
			for (int x = x1; x <= x2; x++)
				if (x >= 0 && x < 640)
					WRITE_UINT16(dst + 640 * y1 + x, c);
		if (y2 >= 0 && y2 < 480)
			for (int x = x1; x <= x2; x++)
				if (x >= 0 && x < 640)
					WRITE_UINT16(dst + 640 * y2 + x, c);
		if (x1 >= 0 && x1 < 640)
			for (int y = y1; y <= y2; y++)
				if (y >= 0 && y < 480)
					WRITE_UINT16(dst + 640 * y + x1, c);
		if (x2 >= 0 && x2 < 640)
			for (int y = y1; y <= y2; y++)
				if (y >= 0 && y < 480)
					WRITE_UINT16(dst + 640 * y + x2, c);
	}
}

void GfxTinyGL::drawLine(PrimitiveObject *primitive) {
	uint16 *dst = (uint16 *)_zb->pbuf;
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = *primitive->getColor();
	uint16 c = ((color.getRed() & 0xF8) << 8) | ((color.getGreen() & 0xFC) << 3) | (color.getBlue() >> 3);

	if (x2 == x1) {
		for (int y = y1; y <= y2; y++) {
			if (x1 >= 0 && x1 < 640 && y >= 0 && y < 480)
				WRITE_UINT16(dst + 640 * y + x1, c);
		}
	} else {
		float m = (y2 - y1) / (x2 - x1);
		int b = (int)(-m * x1 + y1);
		for (int x = x1; x <= x2; x++) {
			int y = (int)(m * x) + b;
			if (x >= 0 && x < 640 && y >= 0 && y < 480)
				WRITE_UINT16(dst + 640 * y + x, c);
		}
	}
}

void GfxTinyGL::drawPolygon(PrimitiveObject *primitive) {
	uint16 *dst = (uint16 *)_zb->pbuf;
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

	const Color &color = *primitive->getColor();
	uint16 c = ((color.getRed() & 0xF8) << 8) | ((color.getGreen() & 0xFC) << 3) | (color.getBlue() >> 3);

	m = (y2 - y1) / (x2 - x1);
	b = (int)(-m * x1 + y1);
	for (int x = x1; x <= x2; x++) {
		int y = (int)(m * x) + b;
		if (x >= 0 && x < 640 && y >= 0 && y < 480)
			WRITE_UINT16(dst + 640 * y + x, c);
	}
	m = (y4 - y3) / (x4 - x3);
	b = (int)(-m * x3 + y3);
	for (int x = x3; x <= x4; x++) {
		int y = (int)(m * x) + b;
		if (x >= 0 && x < 640 && y >= 0 && y < 480)
			WRITE_UINT16(dst + 640 * y + x, c);
	}
}

} // end of namespace Grim
