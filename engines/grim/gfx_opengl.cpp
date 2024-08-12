/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"
#include "common/system.h"
#include "common/config-manager.h"

#if defined(USE_OPENGL_GAME)

#include "graphics/surface.h"

#include "math/glmath.h"

#include "engines/grim/actor.h"
#include "engines/grim/colormap.h"
#include "engines/grim/font.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_opengl.h"
#include "engines/grim/grim.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/primitives.h"
#include "engines/grim/sprite.h"
#include "engines/grim/model.h"
#include "engines/grim/set.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/remastered/overlay.h"
#include "engines/grim/registry.h"

namespace Grim {

GfxBase *CreateGfxOpenGL() {
	return new GfxOpenGL();
}

#ifdef GL_ARB_fragment_program
// Simple ARB fragment program that writes the value from a texture to the Z-buffer.
static char fragSrc[] =
	"!!ARBfp1.0\n\
	TEMP d;\n\
	TEX d, fragment.texcoord[0], texture[0], 2D;\n\
	MOV result.depth, d.r;\n\
	END\n";

static char dimFragSrc[] =
	"!!ARBfp1.0\n\
	PARAM level = program.local[0];\n\
	TEMP color;\n\
	TEMP d;\n\
	TEX d, fragment.texcoord[0], texture[0], 2D;\n\
	TEMP sum;\n\
	MOV sum, d.r;\n\
	ADD sum, sum, d.g;\n\
	ADD sum, sum, d.b;\n\
	MUL sum, sum, 0.33;\n\
	MUL sum, sum, level.x;\n\
	MOV result.color.r, sum;\n\
	MOV result.color.g, sum;\n\
	MOV result.color.b, sum;\n\
	END\n";
#endif

GfxOpenGL::GfxOpenGL() : _smushNumTex(0),
		_smushTexIds(nullptr), _smushWidth(0), _smushHeight(0),
		_useDepthShader(false), _useDimShader(0),
		_maxLights(0), _storedDisplay(nullptr),
#ifdef GL_ARB_fragment_program
		_fragmentProgram(0), _dimFragProgram(0),
#endif
		_emergFont(0), _alpha(1.f) {
	type = Graphics::RendererType::kRendererTypeOpenGL;
	// GL_LEQUAL as glDepthFunc ensures that subsequent drawing attempts for
	// the same triangles are not ignored by the depth test.
	// That's necessary for EMI where some models have multiple faces which
	// refer to the same vertices. The first face is usually using the
	// color map and the following are using textures.
	_depthFunc = (g_grim->getGameType() == GType_MONKEY4) ? GL_LEQUAL : GL_LESS;
}

GfxOpenGL::~GfxOpenGL() {
	releaseMovieFrame();
	delete[] _storedDisplay;

	if (_emergFont && glIsList(_emergFont))
		glDeleteLists(_emergFont, 128);

#ifdef GL_ARB_fragment_program
	if (_useDepthShader)
		glDeleteProgramsARB(1, &_fragmentProgram);

	if (_useDimShader)
		glDeleteProgramsARB(1, &_dimFragProgram);
#endif
	for (unsigned int i = 0; i < _numSpecialtyTextures; i++) {
		destroyTexture(&_specialtyTextures[i]);
	}
}

void GfxOpenGL::setupScreen(int screenW, int screenH) {
	_screenWidth = screenW;
	_screenHeight = screenH;
	_scaleW = _screenWidth / (float)_gameWidth;
	_scaleH = _screenHeight / (float)_gameHeight;

	_globalScaleW = _screenWidth / (float)_globalWidth;
	_globalScaleH = _screenHeight / (float)_globalHeight;

	_useDepthShader = false;
	_useDimShader = false;

	g_system->showMouse(false);

	int screenSize = _screenWidth * _screenHeight * 4;
	_storedDisplay = new byte[screenSize]();
	_smushNumTex = 0;

	_currentShadowArray = nullptr;
	glViewport(0, 0, _screenWidth, _screenHeight);

	GLfloat ambientSource[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientSource);
	GLfloat diffuseReflectance[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseReflectance);

	glClearStencil(~0);

	if (g_grim->getGameType() == GType_GRIM) {
		glPolygonOffset(-6.0, -6.0);
	}

	initExtensions();
	glGetIntegerv(GL_MAX_LIGHTS, &_maxLights);
}

void GfxOpenGL::initExtensions() {
	if (!ConfMan.getBool("use_arb_shaders")) {
		return;
	}

#ifdef GL_ARB_fragment_program
	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	if (extensions && strstr(extensions, "ARB_fragment_program")) {
		_useDepthShader = true;
		_useDimShader = true;
	}

	if (_useDepthShader) {
		glGenProgramsARB(1, &_fragmentProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _fragmentProgram);

		GLint errorPos;
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(fragSrc), fragSrc);
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1) {
			warning("Error compiling depth fragment program:\n%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			_useDepthShader = false;
		}
	}

	if (_useDimShader) {
		glGenProgramsARB(1, &_dimFragProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _dimFragProgram);

		GLint errorPos;
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(dimFragSrc), dimFragSrc);
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1) {
			warning("Error compiling dim fragment program:\n%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			_useDimShader = false;
		}
	}
#endif
}

const char *GfxOpenGL::getVideoDeviceName() {
	return "OpenGL Renderer";
}

void GfxOpenGL::setupCameraFrustum(float fov, float nclip, float fclip) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float right = nclip * tan(fov / 2 * ((float)M_PI / 180));
	glFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GfxOpenGL::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float roll) {
	Math::Vector3d up_vec(0, 0, 1);

	glRotatef(roll, 0, 0, -1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Math::Vector3d(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	glMultMatrixf(lookMatrix.getData());
	glTranslated(-pos.x(), -pos.y(), -pos.z());
}

void GfxOpenGL::positionCamera(const Math::Vector3d &pos, const Math::Matrix4 &rot) {
	glScaled(1.0f, 1.0f, -1.0f);
	_currentPos = pos;
	_currentRot = rot;
}

Math::Matrix4 GfxOpenGL::getModelView() {
	Math::Matrix4 modelView;

	if (g_grim->getGameType() == GType_MONKEY4) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glMultMatrixf(_currentRot.getData());
		glTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());

		glGetFloatv(GL_MODELVIEW_MATRIX, modelView.getData());

		glPopMatrix();
	} else {
		glGetFloatv(GL_MODELVIEW_MATRIX, modelView.getData());
	}

	modelView.transpose();
	return modelView;
}

Math::Matrix4 GfxOpenGL::getProjection() {
	Math::Matrix4 projection;
	glGetFloatv(GL_PROJECTION_MATRIX, projection.getData());
	projection.transpose();
	return projection;
}

void GfxOpenGL::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GfxOpenGL::clearDepthBuffer() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GfxOpenGL::flipBuffer(bool opportunistic) {
	if (opportunistic) {
		GLint fbo = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
		if (fbo == 0) {
			// Don't flip if we are not rendering on FBO
			// Flipping without any draw is undefined
			// When using an FBO, the older texture will be used
			return;
		}
	}

	g_system->updateScreen();
}

bool GfxOpenGL::isHardwareAccelerated() {
	return true;
}

bool GfxOpenGL::supportsShaders() {
	return false;
}

static void shadowProjection(const Math::Vector3d &light, const Math::Vector3d &plane, const Math::Vector3d &normal, bool dontNegate) {
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

	glMultMatrixf((GLfloat *)mat);
}

void GfxOpenGL::getScreenBoundingBox(const Mesh *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	GLdouble top = 1000;
	GLdouble right = -1000;
	GLdouble left = 1000;
	GLdouble bottom = -1000;

	for (int i = 0; i < model->_numFaces; i++) {
		Math::Vector3d obj;
		float *pVertices;

		for (int j = 0; j < model->_faces[i].getNumVertices(); j++) {
			GLdouble modelView[16], projection[16];
			GLint viewPort[4];

			glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewPort);

			pVertices = model->_vertices + 3 * model->_faces[i].getVertex(j);

			obj.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

			Math::Vector3d win;
			Math::gluMathProject<GLdouble, GLint>(obj, modelView, projection, viewPort, win);

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

	double t = bottom;
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

void GfxOpenGL::getScreenBoundingBox(const EMIModel *model, int *x1, int *y1, int *x2, int *y2) {
	if (_currentShadowArray) {
		*x1 = -1;
		*y1 = -1;
		*x2 = -1;
		*y2 = -1;
		return;
	}

	GLdouble top = 1000;
	GLdouble right = -1000;
	GLdouble left = 1000;
	GLdouble bottom = -1000;

	GLdouble modelView[16], projection[16];
	GLint viewPort[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	for (uint i = 0; i < model->_numFaces; i++) {
		uint16 *indices = (uint16 *)model->_faces[i]._indexes;

		for (uint j = 0; j < model->_faces[i]._faceLength * 3; j++) {
			uint16 index = indices[j];
			Math::Vector3d obj = model->_drawVertices[index];
			Math::Vector3d win;
			Math::gluMathProject<GLdouble, GLint>(obj, modelView, projection, viewPort, win);

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

	double t = bottom;
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

void GfxOpenGL::getActorScreenBBox(const Actor *actor, Common::Point &p1, Common::Point &p2) {
	// Get the actor's bounding box information (describes a 3D box)
	Math::Vector3d bboxPos, bboxSize;
	actor->getBBoxInfo(bboxPos, bboxSize);

	// Translate the bounding box to the actor's position
	Math::Matrix4 m = actor->getFinalMatrix();
	bboxPos = bboxPos + actor->getWorldPos();

	// Set up the camera coordinate system
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Apply the view transform.
	Math::Matrix4 worldRot = _currentRot;
	glMultMatrixf(worldRot.getData());
	glTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());

	// Get the current OpenGL state
	GLdouble modelView[16], projection[16];
	GLint viewPort[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewPort);

	// Set values outside of the screen range
	p1.x = 1000;
	p1.y = 1000;
	p2.x = -1000;
	p2.y = -1000;

	// Project all of the points in the 3D bounding box
	Math::Vector3d p, projected;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				Math::Vector3d added(bboxSize.x() * 0.5f * (x * 2 - 1), bboxSize.y() * 0.5f * (y * 2 - 1), bboxSize.z() * 0.5f * (z * 2 - 1));
				m.transform(&added, false);
				p = bboxPos + added;
				Math::gluMathProject<GLdouble, GLint>(p, modelView, projection, viewPort, projected);

				// Find the points
				if (projected.x() < p1.x)
					p1.x = projected.x();
				if (projected.y() < p1.y)
					p1.y = projected.y();
				if (projected.x() > p2.x)
					p2.x = projected.x();
				if (projected.y() > p2.y)
					p2.y = projected.y();
			}
		}
	}

	// Swap the p1/p2 y coorindates
	int16 tmp = p1.y;
	p1.y = 480 - p2.y;
	p2.y = 480 - tmp;

	// Restore the state
	glPopMatrix();
}

void GfxOpenGL::startActorDraw(const Actor *actor) {
	_currentActor = actor;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (g_grim->getGameType() == GType_MONKEY4 && !actor->isInOverworld()) {
		// Apply the view transform.
		glMultMatrixf(_currentRot.getData());
		glTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());
	}

	if (_currentShadowArray) {
		Sector *shadowSector = _currentShadowArray->planeList.front().sector;
		glDepthMask(GL_FALSE);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		if (g_grim->getGameType() == GType_GRIM) {
			glColor3ub(_shadowColorR, _shadowColorG, _shadowColorB);
		} else {
			glColor3ub(_currentShadowArray->color.getRed(), _currentShadowArray->color.getGreen(), _currentShadowArray->color.getBlue());
		}
		//glColor3f(0.0f, 1.0f, 0.0f); // debug draw color
		shadowProjection(_currentShadowArray->pos, shadowSector->getVertices()[0], shadowSector->getNormal(), _currentShadowArray->dontNegate);
	}

	const float alpha = actor->getEffectiveAlpha();
	if (alpha < 1.f) {
		_alpha = alpha;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);

		if (actor->isInOverworld()) {
			const Math::Vector3d &pos = actor->getWorldPos();
			const Math::Quaternion &quat = actor->getRotationQuat();
			// At distance 3.2, a 6.4x4.8 actor fills the screen.
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			float right = 1;
			float top = right * 0.75;
			float div = 6.0f;
			glFrustum(-right / div, right / div, -top / div, top / div, 1.0f / div, 3276.8f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glScalef(1.0, 1.0, -1.0);
			glTranslatef(pos.x(), pos.y(), pos.z());
			glMultMatrixf(quat.toMatrix().getData());
		} else {
			Math::Matrix4 m = actor->getFinalMatrix();
			m.transpose();
			glMultMatrixf(m.getData());
		}
	} else {
		// Grim
		Math::Vector3d pos = actor->getWorldPos();
		const Math::Quaternion &quat = actor->getRotationQuat();
		const float &scale = actor->getScale();

		glTranslatef(pos.x(), pos.y(), pos.z());
		glScalef(scale, scale, scale);
		glMultMatrixf(quat.toMatrix().getData());
	}
}

void GfxOpenGL::finishActorDraw() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_2D);
	if (_alpha < 1.f) {
		glDisable(GL_BLEND);
		_alpha = 1.f;
	}

	if (_currentShadowArray) {
		glEnable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		glDisable(GL_CULL_FACE);
	}

	_currentActor = nullptr;
}

void GfxOpenGL::drawShadowPlanes() {
/*	glColor3f(1.0f, 1.0f, 1.0f);
	_currentShadowArray->planeList.begin();
	for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); i++) {
		Sector *shadowSector = i->sector;
		glBegin(GL_POLYGON);
		for (int k = 0; k < shadowSector->getNumVertices(); k++) {
			glVertex3f(shadowSector->getVertices()[k].x(), shadowSector->getVertices()[k].y(), shadowSector->getVertices()[k].z());
		}
		glEnd();
	}*/

	glPushMatrix();

	if (g_grim->getGameType() == GType_MONKEY4) {
		// Apply the view transform.
		glMultMatrixf(_currentRot.getData());
		glTranslatef(-_currentPos.x(), -_currentPos.y(), -_currentPos.z());
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, (GLuint)~0);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); ++i) {
		Sector *shadowSector = i->sector;
		glBegin(GL_POLYGON);
		for (int k = 0; k < shadowSector->getNumVertices(); k++) {
			glVertex3f(shadowSector->getVertices()[k].x(), shadowSector->getVertices()[k].y(), shadowSector->getVertices()[k].z());
		}
		glEnd();
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, (GLuint)~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glPopMatrix();
}

void GfxOpenGL::setShadow(Shadow *shadow) {
	_currentShadowArray = shadow;
}

void GfxOpenGL::setShadowMode() {
	GfxBase::setShadowMode();
}

void GfxOpenGL::clearShadowMode() {
	GfxBase::clearShadowMode();

	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);
}

void GfxOpenGL::setShadowColor(byte r, byte g, byte b) {
	_shadowColorR = r;
	_shadowColorG = g;
	_shadowColorB = b;
}

void GfxOpenGL::getShadowColor(byte *r, byte *g, byte *b) {
	*r = _shadowColorR;
	*g = _shadowColorG;
	*b = _shadowColorB;
}

void GfxOpenGL::set3DMode() {
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(_depthFunc);
}

void GfxOpenGL::drawEMIModelFace(const EMIModel *model, const EMIMeshFace *face) {
	uint16 *indices = (uint16 *)face->_indexes;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	if (!_currentShadowArray && face->_hasTexture)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
	if (face->_flags & EMIMeshFace::kAlphaBlend || face->_flags & EMIMeshFace::kUnknownBlend || _currentActor->hasLocalAlpha() || _alpha < 1.0f)
		glEnable(GL_BLEND);

	glBegin(GL_TRIANGLES);
	float alpha = _alpha;
	if (model->_meshAlphaMode == Actor::AlphaReplace) {
		alpha *= model->_meshAlpha;
	}
	Math::Vector3d noLighting(1.f, 1.f, 1.f);
	for (uint j = 0; j < face->_faceLength * 3; j++) {
		uint16 index = indices[j];

		if (!_currentShadowArray) {
			if (face->_hasTexture) {
				glTexCoord2f(model->_texVerts[index].getX(), model->_texVerts[index].getY());
			}
			Math::Vector3d lighting = (face->_flags & EMIMeshFace::kNoLighting) ? noLighting : model->_lighting[index];
			byte r = (byte)(model->_colorMap[index].r * lighting.x());
			byte g = (byte)(model->_colorMap[index].g * lighting.y());
			byte b = (byte)(model->_colorMap[index].b * lighting.z());
			byte a = (int)(alpha * (model->_meshAlphaMode == Actor::AlphaReplace ? model->_colorMap[index].a * _currentActor->getLocalAlpha(index) : 255.f));
			glColor4ub(r, g, b, a);
		}

		Math::Vector3d normal = model->_normals[index];
		Math::Vector3d vertex = model->_drawVertices[index];

		glNormal3fv(normal.getData());
		glVertex3fv(vertex.getData());
	}
	glEnd();

	if (!_currentShadowArray) {
		glColor3f(1.0f, 1.0f, 1.0f);
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);

	if (!_currentShadowArray)
		glDepthMask(GL_TRUE);
}

void GfxOpenGL::drawModelFace(const Mesh *mesh, const MeshFace *face) {
	// Support transparency in actor objects, such as the message tube
	// in Manny's Office
	float *vertices = mesh->_vertices;
	float *vertNormals = mesh->_vertNormals;
	float *textureVerts = mesh->_textureVerts;
	glAlphaFunc(GL_GREATER, 0.5);
	glEnable(GL_ALPHA_TEST);
	glNormal3fv(face->getNormal().getData());
	glBegin(GL_POLYGON);
	for (int i = 0; i < face->getNumVertices(); i++) {
		glNormal3fv(vertNormals + 3 * face->getVertex(i));

		if (face->hasTexture())
			glTexCoord2fv(textureVerts + 2 * face->getTextureVertex(i));

		glVertex3fv(vertices + 3 * face->getVertex(i));
	}
	glEnd();
	// Done with transparency-capable objects
	glDisable(GL_ALPHA_TEST);
}

void GfxOpenGL::drawSprite(const Sprite *sprite) {
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (g_grim->getGameType() == GType_MONKEY4) {
		GLdouble modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		Math::Matrix4 act;
		act.buildAroundZ(_currentActor->getYaw());
		act.transpose();
		act(3, 0) = modelview[12];
		act(3, 1) = modelview[13];
		act(3, 2) = modelview[14];
		glLoadMatrixf(act.getData());
		glTranslatef(sprite->_pos.x(), sprite->_pos.y(), -sprite->_pos.z());
	} else {
		glTranslatef(sprite->_pos.x(), sprite->_pos.y(), sprite->_pos.z());
		GLdouble modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

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
		glLoadMatrixd(modelview);
	}

	if (sprite->_flags1 & Sprite::BlendAdditive) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDisable(GL_LIGHTING);

	if (g_grim->getGameType() == GType_GRIM) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.5f);
	} else if (sprite->_flags2 & Sprite::AlphaTest) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.1f);
	} else {
		glDisable(GL_ALPHA_TEST);
	}

	if (sprite->_flags2 & Sprite::DepthTest) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (g_grim->getGameType() == GType_MONKEY4) {
		glDepthMask(GL_TRUE);

		float halfWidth = sprite->_width / 2;
		float halfHeight = sprite->_height / 2;
		float vertexX[] = { -1.0f, 1.0f, 1.0f, -1.0f };
		float vertexY[] = { 1.0f, 1.0f, -1.0f, -1.0f };

		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; ++i) {
			float r = sprite->_red[i] / 255.0f;
			float g = sprite->_green[i] / 255.0f;
			float b = sprite->_blue[i] / 255.0f;
			float a = sprite->_alpha[i] * _alpha / 255.0f;

			glColor4f(r, g, b, a);
			glTexCoord2f(sprite->_texCoordX[i], sprite->_texCoordY[i]);
			glVertex3f(vertexX[i] * halfWidth, vertexY[i] * halfHeight, 0.0f);
		}
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	} else {
		// In Grim, the bottom edge of the sprite is at y=0 and
		// the texture is flipped along the X-axis.
		float halfWidth = sprite->_width / 2;
		float height = sprite->_height;

		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(+halfWidth, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(+halfWidth, +height, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-halfWidth, +height, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-halfWidth, 0.0f, 0.0f);
		glEnd();
	}

	glEnable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

void GfxOpenGL::drawOverlay(const Overlay *overlay) {
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glDisable(GL_LIGHTING);


	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.5f);



	glDisable(GL_DEPTH_TEST);


	float height = overlay->getHeight() * _globalScaleH;
	float width = overlay->getWidth() * _globalScaleW;
	float x = overlay->_x * _globalScaleW;
	float y = overlay->_y * _globalScaleH;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(x, y);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f((x + width), y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f((x + width), (y + height));
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(x, (y + height));
	glEnd();



	glEnable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

void GfxOpenGL::translateViewpointStart() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void GfxOpenGL::translateViewpoint(const Math::Vector3d &vec) {
	glTranslatef(vec.x(), vec.y(), vec.z());
}

void GfxOpenGL::rotateViewpoint(const Math::Angle &angle, const Math::Vector3d &axis) {
	glRotatef(angle.getDegrees(), axis.x(), axis.y(), axis.z());
}

void GfxOpenGL::rotateViewpoint(const Math::Matrix4 &rot) {
	glMultMatrixf(rot.getData());
}

void GfxOpenGL::translateViewpointFinish() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GfxOpenGL::enableLights() {
	if (!isShadowModeActive()) {
		glEnable(GL_LIGHTING);
	}
}

void GfxOpenGL::disableLights() {
	glDisable(GL_LIGHTING);
}

void GfxOpenGL::setupLight(Light *light, int lightId) {
	if (lightId >= _maxLights) {
		return;
	}

	glEnable(GL_LIGHTING);
	GLfloat lightColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat lightDir[] = { 0.0f, 0.0f, -1.0f };
	GLfloat cutoff = 180.0f;
	GLfloat spot_exp = 0.0f;
	GLfloat q_attenuation = 1.0f;

	GLfloat intensity = light->_scaledintensity;
	lightColor[0] = (GLfloat)light->_color.getRed() * intensity;
	lightColor[1] = (GLfloat)light->_color.getGreen() * intensity;
	lightColor[2] = (GLfloat)light->_color.getBlue() * intensity;

	if (light->_type == Light::Omni) {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
	} else if (light->_type == Light::Direct) {
		lightPos[0] = -light->_dir.x();
		lightPos[1] = -light->_dir.y();
		lightPos[2] = -light->_dir.z();
		lightPos[3] = 0;
	} else if (light->_type == Light::Spot) {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
		lightDir[0] = light->_dir.x();
		lightDir[1] = light->_dir.y();
		lightDir[2] = light->_dir.z();
		spot_exp = 2.0f;
		cutoff = light->_penumbraangle;
		q_attenuation = 0.0f;
	}

	glDisable(GL_LIGHT0 + lightId);
	glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0 + lightId, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, lightDir);
	glLightf(GL_LIGHT0 + lightId, GL_SPOT_EXPONENT, spot_exp);
	glLightf(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, cutoff);
	glLightf(GL_LIGHT0 + lightId, GL_QUADRATIC_ATTENUATION, q_attenuation);
	glEnable(GL_LIGHT0 + lightId);
}

void GfxOpenGL::turnOffLight(int lightId) {
	glDisable(GL_LIGHT0 + lightId);
}

#define BITMAP_TEXTURE_SIZE 256

void GfxOpenGL::createBitmap(BitmapData *bitmap) {
	GLuint *textures;

	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(const_cast<void  *>(bitmap->getImageData(pic).getPixels()));
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(zbufPtr + i);
				// fix the value if it is incorrectly set to the bitmap transparency color
				if (val == 0xf81f) {
					val = 0;
				}
				zbufPtr[i] = 0xffff - ((uint32)val) * 0x10000 / 100 / (0x10000 - val);
			}

			// Flip the zbuffer image to match what GL expects
			if (!_useDepthShader) {
				for (int y = 0; y < bitmap->_height / 2; y++) {
					uint16 *ptr1 = zbufPtr + y * bitmap->_width;
					uint16 *ptr2 = zbufPtr + (bitmap->_height - 1 - y) * bitmap->_width;
					for (int x = 0; x < bitmap->_width; x++, ptr1++, ptr2++) {
						uint16 tmp = *ptr1;
						*ptr1 = *ptr2;
						*ptr2 = tmp;
					}
				}
			}
		}
	}
	if (bitmap->_format == 1 || _useDepthShader) {
		bitmap->_hasTransparency = false;
		bitmap->_numTex = ((bitmap->_width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
		                  ((bitmap->_height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
		bitmap->_texIds = new GLuint[bitmap->_numTex * bitmap->_numImages];
		textures = (GLuint *)bitmap->_texIds;
		glGenTextures(bitmap->_numTex * bitmap->_numImages, textures);

		byte *texData = nullptr;
		byte *texOut = nullptr;

		GLint format = GL_RGBA;
		GLint btype = GL_UNSIGNED_BYTE;
		int bytes = 4;
		if (bitmap->_format != 1) {
			format = GL_DEPTH_COMPONENT;
			btype = GL_UNSIGNED_SHORT;
			bytes = 2;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, bytes);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, bitmap->_width);

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			if (bitmap->_format == 1 && bitmap->_bpp == 16 && bitmap->_colorFormat != BM_RGB1555) {
				if (texData == nullptr)
					texData = new byte[bytes * bitmap->_width * bitmap->_height];
				// Convert data to 32-bit RGBA format
				byte *texDataPtr = texData;
				uint16 *bitmapData = (uint16 *)const_cast<void *>(bitmap->getImageData(pic).getPixels());
				for (int i = 0; i < bitmap->_width * bitmap->_height; i++, texDataPtr += bytes, bitmapData++) {
					uint16 pixel = *bitmapData;
					int r = pixel >> 11;
					texDataPtr[0] = (r << 3) | (r >> 2);
					int g = (pixel >> 5) & 0x3f;
					texDataPtr[1] = (g << 2) | (g >> 4);
					int b = pixel & 0x1f;
					texDataPtr[2] = (b << 3) | (b >> 2);
					if (pixel == 0xf81f) { // transparent
						texDataPtr[3] = 0;
						bitmap->_hasTransparency = true;
					} else {
						texDataPtr[3] = 255;
					}
				}
				texOut = texData;
			} else if (bitmap->_format == 1 && bitmap->_colorFormat == BM_RGB1555) {
				bitmap->convertToColorFormat(pic, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
				texOut = (byte *)const_cast<void *>(bitmap->getImageData(pic).getPixels());
			} else {
				texOut = (byte *)const_cast<void *>(bitmap->getImageData(pic).getPixels());
			}

			for (int i = 0; i < bitmap->_numTex; i++) {
				glBindTexture(GL_TEXTURE_2D, textures[bitmap->_numTex * pic + i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexImage2D(GL_TEXTURE_2D, 0, format, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, format, btype, nullptr);
			}

			int cur_tex_idx = bitmap->_numTex * pic;

			for (int y = 0; y < bitmap->_height; y += BITMAP_TEXTURE_SIZE) {
				for (int x = 0; x < bitmap->_width; x += BITMAP_TEXTURE_SIZE) {
					int width  = (x + BITMAP_TEXTURE_SIZE >= bitmap->_width) ? (bitmap->_width - x) : BITMAP_TEXTURE_SIZE;
					int height = (y + BITMAP_TEXTURE_SIZE >= bitmap->_height) ? (bitmap->_height - y) : BITMAP_TEXTURE_SIZE;
					glBindTexture(GL_TEXTURE_2D, textures[cur_tex_idx]);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, btype,
					                texOut + (y * bytes * bitmap->_width) + (bytes * x));
					cur_tex_idx++;
				}
			}
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		delete[] texData;
		bitmap->freeData();
	}
}

void GfxOpenGL::drawBitmap(const Bitmap *bitmap, int dx, int dy, uint32 layer) {
	// The PS2 version of EMI uses a TGA for it's splash-screen
	// avoid using the TIL-code below for that, by checking
	// for texture coordinates set by BitmapData::loadTile
	if (g_grim->getGameType() == GType_MONKEY4 && bitmap->_data && bitmap->_data->_texc) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, 0, 1);

		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glColor3f(1.0f, 1.0f, 1.0f);

		BitmapData *data = bitmap->_data;
		GLuint *textures = (GLuint *)bitmap->getTexIds();
		float *texc = data->_texc;

		assert(layer < data->_numLayers);
		uint32 offset = data->_layers[layer]._offset;
		for (uint32 i = offset; i < offset + data->_layers[layer]._numImages; ++i) {
			glBindTexture(GL_TEXTURE_2D, textures[data->_verts[i]._texid]);
			glBegin(GL_QUADS);
			uint32 ntex = data->_verts[i]._pos * 4;
			for (uint32 x = 0; x < data->_verts[i]._verts; ++x) {
				glTexCoord2f(texc[ntex + 2], texc[ntex + 3]);
				glVertex2f(texc[ntex + 0], texc[ntex + 1]);
				ntex += 4;
			}
			glEnd();
		}

		glColor3f(1.0f, 1.0f, 1.0f);

		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);

		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		return;
	}

	int format = bitmap->getFormat();
	if ((format == 1 && !_renderBitmaps) || (format == 5 && !_renderZBitmaps)) {
		return;
	}

	GLuint *textures;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)
	if (bitmap->getFormat() == 1 && bitmap->getHasTransparency()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	// If drawing a Z-buffer image, but no shaders are available, fall back to the glDrawPixels method.
	if (bitmap->getFormat() == 5 && !_useDepthShader) {
		// Only draw the manual zbuffer when enabled
		if (bitmap->getActiveImage() - 1 < bitmap->getNumImages()) {
			drawDepthBitmap(dx, dy, bitmap->getWidth(), bitmap->getHeight(), (const char *)bitmap->getData(bitmap->getActiveImage() - 1).getPixels());
		} else {
			warning("zbuffer image has index out of bounds! %d/%d", bitmap->getActiveImage(), bitmap->getNumImages());
		}
		glEnable(GL_LIGHTING);
		return;
	}

	if (bitmap->getFormat() == 1) { // Normal image
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	} else { // ZBuffer image
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_TRUE);
#ifdef GL_ARB_fragment_program
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _fragmentProgram);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
#endif
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor((int)(dx * _scaleW), _screenHeight - (int)(((dy + bitmap->getHeight())) * _scaleH), (int)(bitmap->getWidth() * _scaleW), (int)(bitmap->getHeight() * _scaleH));
	int cur_tex_idx = bitmap->getNumTex() * (bitmap->getActiveImage() - 1);
	for (int y = dy; y < (dy + bitmap->getHeight()); y += BITMAP_TEXTURE_SIZE) {
		for (int x = dx; x < (dx + bitmap->getWidth()); x += BITMAP_TEXTURE_SIZE) {
			textures = (GLuint *)bitmap->getTexIds();
			glBindTexture(GL_TEXTURE_2D, textures[cur_tex_idx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(x * _scaleW, y * _scaleH);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f((x + BITMAP_TEXTURE_SIZE) * _scaleW, y * _scaleH);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f((x + BITMAP_TEXTURE_SIZE) * _scaleW, (y + BITMAP_TEXTURE_SIZE)  * _scaleH);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(x * _scaleW, (y + BITMAP_TEXTURE_SIZE) * _scaleH);
			glEnd();
			cur_tex_idx++;
		}
	}
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (bitmap->getFormat() == 1) {
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	} else {
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthFunc(_depthFunc);
#ifdef GL_ARB_fragment_program
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif
	}
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::destroyBitmap(BitmapData *bitmap) {
	GLuint *textures = (GLuint *)bitmap->_texIds;
	if (textures) {
		glDeleteTextures(bitmap->_numTex * bitmap->_numImages, textures);
		delete[] textures;
		bitmap->_texIds = nullptr;
	}
}

struct FontUserData {
	int size;
	GLuint texture;
};

void GfxOpenGL::createFont(Font *f) {
	if (!f->is8Bit())
		return;
	BitmapFont *font = static_cast<BitmapFont *>(f);
	const byte *bitmapData = font->getFontData();
	uint dataSize = font->getDataSize();

	uint8 bpp = 4;
	uint8 charsWide = 16;
	uint8 charsHigh = 16;

	byte *texDataPtr = new byte[dataSize * bpp];
	byte *data = texDataPtr;

	for (uint i = 0; i < dataSize; i++, texDataPtr += bpp, bitmapData++) {
		byte pixel = *bitmapData;
		if (pixel == 0x00) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = texDataPtr[3] = 0;
		} else if (pixel == 0x80) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = 0;
			texDataPtr[3] = 255;
		} else if (pixel == 0xFF) {
			texDataPtr[0] = texDataPtr[1] = texDataPtr[2] = texDataPtr[3] = 255;
		}
	}
	int size = 0;
	for (int i = 0; i < 256; ++i) {
		int width = font->getCharBitmapWidth(i), height = font->getCharBitmapHeight(i);
		int m = MAX(width, height);
		if (m > size)
			size = m;
	}
	assert(size < 64);
	if (size < 8)
		size = 8;
	if (size < 16)
		size = 16;
	else if (size < 32)
		size = 32;
	else if (size < 64)
		size = 64;

	uint arraySize = size * size * bpp * charsWide * charsHigh;
	byte *temp = new byte[arraySize]();
	if (!temp)
		error("Could not allocate %d bytes", arraySize);

	FontUserData *userData = new FontUserData;
	font->setUserData(userData);
	userData->texture = 0;
	userData->size = size;

	GLuint *texture = &(userData->texture);
	glGenTextures(1, texture);

	for (int i = 0, row = 0; i < 256; ++i) {
		int width = font->getCharBitmapWidth(i), height = font->getCharBitmapHeight(i);
		int32 d = font->getCharOffset(i);
		if (d + height * width > (int)dataSize) {
			debug("Font overflow: %d: d + height * width <= dataSize: %d + %d * %d <= %d", i, d, height, width, dataSize);
			continue;
		}
		for (int x = 0; x < height; ++x) {
			// a is the offset to get to the correct row.
			// b is the offset to get to the correct line in the character.
			// c is the offset of the character from the start of the row.
			uint a = row * size * size * bpp * charsHigh;
			uint b = x * size * charsWide * bpp;
			uint c = 0;
			if (i != 0)
				c = ((i - 1) % 16) * size * bpp;

			uint pos = a + b + c;
			uint pos2 = d * bpp + x * width * bpp;
			assert(pos + width * bpp <= arraySize);
			assert(pos2 + width * bpp <= dataSize * bpp);
			memcpy(temp + pos, data + pos2, width * bpp);
		}
		if (i != 0 && i % charsWide == 0)
			++row;
	}
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size * charsWide, size * charsHigh, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp);

	delete[] data;
	delete[] temp;
}

void GfxOpenGL::destroyFont(Font *font) {
	if (font->is8Bit()) {
		const FontUserData *data = static_cast<const FontUserData *>(static_cast<const BitmapFont *>(font)->getUserData());
		if (data) {
			glDeleteTextures(1, &(data->texture));
			delete data;
		}
	}
}

struct TextObjectUserData {
	GLuint *_texids;
};

void GfxOpenGL::createTextObject(TextObject *text) {
	//error("Could not get font userdata");
	const Font *font = text->getFont();

	if (font->is8Bit())
		return;

	int numLines = text->getNumLines();
	GLuint *texids = new GLuint[numLines];
	glGenTextures(numLines, texids);
	// Not at all correct for line-wrapping, but atleast we get all the lines now.
	for (int i = 0; i < numLines; i++) {
		Graphics::Surface surface;

		font->render(surface, text->getLines()[i], Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24),
			     0xFF000000, 0xFFFFFFFF, 0x00000000);

		byte *bitmap = (byte *)surface.getPixels();

		glBindTexture(GL_TEXTURE_2D, texids[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.w, surface.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

		surface.free();
	}
	TextObjectUserData *ud = new TextObjectUserData;

	ud->_texids = texids;
	text->setUserData(ud);
}

void GfxOpenGL::drawTextObject(const TextObject *text) {
	if (!text)
		return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	const Color &color = text->getFGColor();
	const Font *f = text->getFont();

	glColor3ub(color.getRed(), color.getGreen(), color.getBlue());
	if (!f->is8Bit()) {
		const TextObjectUserData *ud = (const TextObjectUserData *)text->getUserData();

		int numLines = text->getNumLines();
		for (int i = 0; i < numLines; ++i) {
			float width = f->getKernedStringLength(text->getLines()[i]);
			float height = f->getKernedHeight();
			float x = text->getLineX(i);

			float y = text->getLineY(i);

			if (text->getCoords() == 2 || text->getCoords() == 1) {
				x *= _globalScaleW;
				y *= _globalScaleH;

				width  *= _globalScaleW;
				height *= _globalScaleH;
			} else if (text->getCoords() == 0) {
				x *= _scaleW;
				y *= _scaleH;

				width  *= _scaleW;
				height *= _scaleH;
			}

			glBindTexture(GL_TEXTURE_2D, ud->_texids[i]);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(x, y);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f((x + width), y);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f((x + width), (y + height));
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(x, (y + height));
			glEnd();
		}

		glColor3f(1, 1, 1);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glDepthMask(GL_TRUE);
		return;
	}

	const BitmapFont *font = static_cast<const BitmapFont *>(f);
	const FontUserData *userData = (const FontUserData *)font->getUserData();
	if (!userData)
		error("Could not get font userdata");
	float sizeW = userData->size * _scaleW;
	float sizeH = userData->size * _scaleH;
	GLuint texture = userData->texture;
	const Common::String *lines = text->getLines();
	int numLines = text->getNumLines();
	for (int j = 0; j < numLines; ++j) {
		const Common::String &line = lines[j];
		int x = text->getLineX(j);
		int y = text->getLineY(j);
		for (uint i = 0; i < line.size(); ++i) {
			uint8 character = line[i];
			float w = y + font->getCharStartingLine(character);
			if (g_grim->getGameType() == GType_GRIM)
				w += font->getBaseOffsetY();
			float z = x + font->getCharStartingCol(character);
			z *= _scaleW;
			w *= _scaleH;
			glBindTexture(GL_TEXTURE_2D, texture);
			float width = 1 / 16.f;
			float cx = ((character - 1) % 16) / 16.0f;
			float cy = ((character - 1) / 16) / 16.0f;
			glBegin(GL_QUADS);
			glTexCoord2f(cx, cy);
			glVertex2f(z, w);
			glTexCoord2f(cx + width, cy);
			glVertex2f(z + sizeW, w);
			glTexCoord2f(cx + width, cy + width);
			glVertex2f(z + sizeW, w + sizeH);
			glTexCoord2f(cx, cy + width);
			glVertex2f(z, w + sizeH);
			glEnd();
			x += font->getCharKernedWidth(character);
		}
	}

	glColor3f(1, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
}

void GfxOpenGL::destroyTextObject(TextObject *text) {
	if (g_grim->getGameType() != GType_GRIM || !g_grim->isRemastered())
		return;

	TextObjectUserData *ud = (TextObjectUserData *)const_cast<void *>(text->getUserData());
	glDeleteTextures(text->getNumLines(), ud->_texids);
	delete ud;
}

void GfxOpenGL::createTexture(Texture *texture, const uint8 *data, const CMap *cmap, bool clamp) {
	texture->_texture = new GLuint[1];
	glGenTextures(1, (GLuint *)texture->_texture);
	uint8 *texdata = new uint8[texture->_width * texture->_height * 4];
	uint8 *texdatapos = texdata;

	if (cmap != nullptr) { // EMI doesn't have colour-maps
		int bytes = 4;
		for (int y = 0; y < texture->_height; y++) {
			for (int x = 0; x < texture->_width; x++) {
				uint8 col = *data;
				if (col == 0) {
					memset(texdatapos, 0, bytes); // transparent
					if (!texture->_hasAlpha) {
						texdatapos[3] = 0xff; // fully opaque
					}
				} else {
					memcpy(texdatapos, cmap->_colors + 3 * (col), 3);
					texdatapos[3] = 0xff; // fully opaque
				}
				texdatapos += bytes;
				data++;
			}
		}
	} else {
		memcpy(texdata, data, texture->_width * texture->_height * texture->_bpp);
	}

	GLuint *textures = (GLuint *)texture->_texture;
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	// Remove darkened lines in EMI intro
	if (g_grim->getGameType() == GType_MONKEY4 && clamp) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->_width, texture->_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
	delete[] texdata;
}

void GfxOpenGL::selectTexture(const Texture *texture) {
	GLuint *textures = (GLuint *)texture->_texture;
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	if (texture->_hasAlpha && g_grim->getGameType() == GType_MONKEY4) {
		glEnable(GL_BLEND);
	}

	// Grim has inverted tex-coords, EMI doesn't
	if (g_grim->getGameType() != GType_MONKEY4) {
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(1.0f / texture->_width, 1.0f / texture->_height, 1);
	}
}

void GfxOpenGL::destroyTexture(Texture *texture) {
	GLuint *textures = (GLuint *)texture->_texture;
	if (textures) {
		glDeleteTextures(1, textures);
		delete[] textures;
	}
}

void GfxOpenGL::drawDepthBitmap(int x, int y, int w, int h, const char *data) {
	//if (num != 0) {
	//  warning("Animation not handled yet in GL texture path");
	//}

	if (y + h == 480) {
		glRasterPos2i(x, _screenHeight - 1);
		glBitmap(0, 0, 0, 0, 0, -1, nullptr);
	} else
		glRasterPos2i(x, y + h);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2); // 16 bit Z depth bitmap

	glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(_depthFunc);
}

void GfxOpenGL::prepareMovieFrame(Graphics::Surface *frame) {
	int height = frame->h;
	int width = frame->w;
	byte *bitmap = (byte *)frame->getPixels();

	double scaleW = _scaleW;
	double scaleH = _scaleH;
	// Remastered hack, don't scale full-screen videos for now.
	if (height == 1080) {
		_scaleW = 1.0f;
		_scaleH = 1.0f;
	}

	GLenum format;
	GLenum dataType;
	int bytesPerPixel = frame->format.bytesPerPixel;

	// Aspyr Logo format
	if (frame->format == Graphics::PixelFormat(4, 8, 8, 8, 0, 8, 16, 24, 0)) {
#if !defined(__amigaos4__)
		format = GL_BGRA;
		dataType = GL_UNSIGNED_INT_8_8_8_8;
#else
		// MiniGL on AmigaOS4 doesn't understand GL_UNSIGNED_INT_8_8_8_8 yet.
		format = GL_BGRA;
		dataType = GL_UNSIGNED_BYTE;
#endif
	// Used by Grim Fandango Remastered
	} else if (frame->format == Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0)) {
#if !defined(__amigaos4__)
		format = GL_BGRA;
		dataType = GL_UNSIGNED_INT_8_8_8_8;
#else
		// MiniGL on AmigaOS4 doesn't understand GL_UNSIGNED_INT_8_8_8_8 yet.
		format = GL_BGRA;
		dataType = GL_UNSIGNED_BYTE;
#endif
	} else if (frame->format == Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 0)) {
		format = GL_BGRA;
		dataType = GL_UNSIGNED_INT_8_8_8_8_REV;
	} else if (frame->format == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) {
		format = GL_RGB;
		dataType = GL_UNSIGNED_SHORT_5_6_5;
	} else {
		error("Unknown pixelformat: Bpp: %d RBits: %d GBits: %d BBits: %d ABits: %d RShift: %d GShift: %d BShift: %d AShift: %d",
			frame->format.bytesPerPixel,
			-(frame->format.rLoss - 8),
			-(frame->format.gLoss - 8),
			-(frame->format.bLoss - 8),
			-(frame->format.aLoss - 8),
			frame->format.rShift,
			frame->format.gShift,
			frame->format.bShift,
			frame->format.aShift);
	}

	// remove if already exist
	if (_smushNumTex > 0) {
		glDeleteTextures(_smushNumTex, _smushTexIds);
		delete[] _smushTexIds;
		_smushNumTex = 0;
	}

	// create texture
	_smushNumTex = ((width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
				   ((height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
	_smushTexIds = new GLuint[_smushNumTex];
	glGenTextures(_smushNumTex, _smushTexIds);
	for (int i = 0; i < _smushNumTex; i++) {
		glBindTexture(GL_TEXTURE_2D, _smushTexIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, format, dataType, nullptr);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, bytesPerPixel); // 16 bit RGB 565 bitmap/32 bit BGR
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	int curTexIdx = 0;
	for (int y = 0; y < height; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < width; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= width) ? (width - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= height) ? (height - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, format, dataType, bitmap + (y * bytesPerPixel * width) + (bytesPerPixel * x));
			curTexIdx++;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	_smushWidth = (int)(width * _scaleW);
	_smushHeight = (int)(height * _scaleH);
	_scaleW = scaleW;
	_scaleH = scaleH;
}

void GfxOpenGL::drawMovieFrame(int offsetX, int offsetY) {
	double scaleW = _scaleW;
	double scaleH = _scaleH;
	// Remastered hack, don't scale full-screen videos for now.
	if (_smushHeight == 1080) {
		_scaleW = 1.0f;
		_scaleH = 1.0f;
	}
	// prepare view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	// draw
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);

	offsetX = (int)(offsetX * _scaleW);
	offsetY = (int)(offsetY * _scaleH);

	glScissor(offsetX, _screenHeight - (offsetY + _smushHeight), _smushWidth, _smushHeight);

	int curTexIdx = 0;
	for (int y = 0; y < _smushHeight; y += (int)(BITMAP_TEXTURE_SIZE * _scaleH)) {
		for (int x = 0; x < _smushWidth; x += (int)(BITMAP_TEXTURE_SIZE * _scaleW)) {
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2f(x + offsetX, y + offsetY);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(x + offsetX + BITMAP_TEXTURE_SIZE * _scaleW, y + offsetY);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(x + offsetX + BITMAP_TEXTURE_SIZE * _scaleW, y + offsetY + BITMAP_TEXTURE_SIZE * _scaleH);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(x + offsetX, y + offsetY + BITMAP_TEXTURE_SIZE * _scaleH);
			glEnd();
			curTexIdx++;
		}
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	_scaleW = scaleW;
	_scaleH = scaleH;
}

void GfxOpenGL::releaseMovieFrame() {
	if (_smushNumTex > 0) {
		glDeleteTextures(_smushNumTex, _smushTexIds);
		delete[] _smushTexIds;
		_smushNumTex = 0;
	}
}

void GfxOpenGL::loadEmergFont() {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 8 bit font bitmaps

	_emergFont = glGenLists(128);
	for (int i = 32; i < 128; i++) {
		glNewList(_emergFont + i, GL_COMPILE);
		glBitmap(8, 13, 0, 2, 10, 0, BitmapFont::emerFont[i - 32]);
		glEndList();
	}
}

void GfxOpenGL::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glRasterPos2i(x, y);
	glColor3f(1.0f, 1.0f, 1.0f);

	glListBase(_emergFont);

	char *list = const_cast<char *>(text);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (void *)list);

	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

Bitmap *GfxOpenGL::getScreenshot(int w, int h, bool useStored) {
	Graphics::Surface src;
	src.create(_screenWidth, _screenHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	if (useStored) {
		memcpy(src.getPixels(), _storedDisplay, _screenWidth * _screenHeight * 4);
	} else {
		glReadPixels(0, 0, _screenWidth, _screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, src.getPixels());
	}
	Bitmap *bmp = createScreenshotBitmap(&src, w, h, false);
	src.free();
	return bmp;
}

void GfxOpenGL::storeDisplay() {
	glReadPixels(0, 0, _screenWidth, _screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, _storedDisplay);
}

void GfxOpenGL::copyStoredToDisplay() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glRasterPos2i(0, _screenHeight - 1);
	glBitmap(0, 0, 0, 0, 0, -1, nullptr);
	glDrawPixels(_screenWidth, _screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, _storedDisplay);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::dimScreen() {
	uint32 *data = (uint32 *)_storedDisplay;
	for (int l = 0; l < _screenWidth * _screenHeight; l++) {
		uint32 pixel = data[l];
		uint8 r = (pixel & 0xFF0000) >> 16;
		uint8 g = (pixel & 0x00FF00) >> 8;
		uint8 b = (pixel & 0x0000FF);
		uint32 color = (r + g + b) / 10;
		data[l] = ((color & 0xFF) << 16) | ((color & 0xFF) << 8) | (color & 0xFF);
	}
}

void GfxOpenGL::dimRegion(int x, int yReal, int w, int h, float level) {
	x = (int)(x * _scaleW);
	yReal = (int)(yReal * _scaleH);
	w = (int)(w * _scaleW);
	h = (int)(h * _scaleH);
	int y = _screenHeight - yReal - h;

#ifdef GL_ARB_fragment_program
	if (_useDimShader) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glViewport(0, 0, _screenWidth, _screenHeight);

		// copy the data over to the texture
		glBindTexture(GL_TEXTURE_2D, texture);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, w, h, 0);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, _screenWidth, 0, _screenHeight, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_SCISSOR_TEST);

		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _dimFragProgram);
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, level, 0, 0, 0);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(x + w, y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(x + w, y + h);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(x, y + h);
		glEnd();

		glDisable(GL_FRAGMENT_PROGRAM_ARB);

		glDeleteTextures(1, &texture);

		return;
	}
#endif

	uint32 *data = new uint32[w * h];
	y = _screenHeight - yReal;

	// collect the requested area and generate the dimmed version
	glReadPixels(x, y - h, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
	for (int ly = 0; ly < h; ly++) {
		for (int lx = 0; lx < w; lx++) {
			uint32 pixel = data[ly * w + lx];
			uint8 r = (pixel & 0xFF0000) >> 16;
			uint8 g = (pixel & 0x00FF00) >> 8;
			uint8 b = (pixel & 0x0000FF);
			uint32 color = (uint32)(((r + g + b) / 3) * level);
			data[ly * w + lx] = ((color & 0xFF) << 16) | ((color & 0xFF) << 8) | (color & 0xFF);
		}
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Set the raster position and draw the bitmap
	glRasterPos2i(x, yReal + h);
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	delete[] data;
}

void GfxOpenGL::irisAroundRegion(int x1, int y1, int x2, int y2) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, _screenWidth, _screenHeight, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	glColor3f(0.0f, 0.0f, 0.0f);

	// Explicitly cast to avoid problems with C++11
	float fx1 = x1;
	float fx2 = x2;
	float fy1 = y1;
	float fy2 = y2;
	float width = _screenWidth;
	float height = _screenHeight;
	float points[20] = {
		0.0f, 0.0f,
		0.0f, fy1,
		width, 0.0f,
		fx2, fy1,
		width, height,
		fx2, fy2,
		0.0f, height,
		fx1, fy2,
		0.0f, fy1,
		fx1, fy1
	};

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0 ;i < 10; ++i) {
		glVertex2fv(points+2*i);
	}
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
}

void GfxOpenGL::drawRectangle(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;
	const Color color(primitive->getColor());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(color.getRed(), color.getGreen(), color.getBlue());

	if (primitive->isFilled()) {
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x2 + 1, y1);
		glVertex2f(x2 + 1, y2 + 1);
		glVertex2f(x1, y2 + 1);
		glEnd();
	} else {
		glLineWidth(_scaleW);
		glBegin(GL_LINE_LOOP);
		glVertex2f(x1, y1);
		glVertex2f(x2 + 1, y1);
		glVertex2f(x2 + 1, y2 + 1);
		glVertex2f(x1, y2 + 1);
		glEnd();
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::drawLine(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;

	const Color &color = primitive->getColor();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(color.getRed(), color.getGreen(), color.getBlue());

	glLineWidth(_scaleW);

	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::drawDimPlane() {
	if (_dimLevel == 0.0f)
		return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1.0, 1.0, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0f, 0.0f, 0.0f, _dimLevel);

	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(1.0, 0);
	glVertex2f(1.0, 1.0);
	glVertex2f(0, 1.0);
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::drawPolygon(const PrimitiveObject *primitive) {
	float x1 = primitive->getP1().x * _scaleW;
	float y1 = primitive->getP1().y * _scaleH;
	float x2 = primitive->getP2().x * _scaleW;
	float y2 = primitive->getP2().y * _scaleH;
	float x3 = primitive->getP3().x * _scaleW;
	float y3 = primitive->getP3().y * _scaleH;
	float x4 = primitive->getP4().x * _scaleW;
	float y4 = primitive->getP4().y * _scaleH;

	const Color &color = primitive->getColor();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#if !defined(__amigaos4__)
	glDisable(GL_MULTISAMPLE);
#endif
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(color.getRed(), color.getGreen(), color.getBlue());

	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2 + 1, y2 + 1);
	glVertex2f(x3, y3 + 1);
	glVertex2f(x4 + 1, y4);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
#if !defined(__amigaos4__)
	glEnable(GL_MULTISAMPLE);
#endif
}

void GfxOpenGL::readPixels(int x, int y, int width, int height, uint8 *buffer) {
	uint8 *p = buffer;
	for (int i = y; i < y + height; i++) {
		glReadPixels(x, _screenHeight - 1 - i, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
		p += width * 4;
	}
}

void GfxOpenGL::createSpecialtyTextureFromScreen(uint id, uint8 *data, int x, int y, int width, int height) {
	readPixels(x, y, width, height, data);
	createSpecialtyTexture(id, data, width, height);
}

void GfxOpenGL::setBlendMode(bool additive) {
	if (additive) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

} // end of namespace Grim

#endif
