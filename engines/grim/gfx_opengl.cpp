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

#if defined(WIN32) && !defined(__SYMBIAN32__)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "common/endian.h"
#include "common/system.h"

#include "engines/grim/actor.h"
#include "engines/grim/colormap.h"
#include "engines/grim/font.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_opengl.h"
#include "engines/grim/grim.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/primitives.h"

#ifdef USE_OPENGL

#if defined (SDL_BACKEND) && defined(GL_ARB_fragment_program)

// We need SDL.h for SDL_GL_GetProcAddress.
#include "backends/platform/sdl/sdl-sys.h"

// Extension functions needed for fragment programs.
PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
PFNGLBINDPROGRAMARBPROC glBindProgramARB;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;

#endif

namespace Grim {

GfxBase *CreateGfxOpenGL() {
	return new GfxOpenGL();
}

// Simple ARB fragment program that writes the value from a texture to the Z-buffer.
static char fragSrc[] =
	"!!ARBfp1.0\n\
	TEX result.depth, fragment.texcoord[0], texture[0], 2D;\n\
	END\n";

GfxOpenGL::GfxOpenGL() {
	g_driver = this;
	_storedDisplay = NULL;
	_emergFont = 0;
}

GfxOpenGL::~GfxOpenGL() {
	delete[] _storedDisplay;
	if (_emergFont && glIsList(_emergFont))
		glDeleteLists(_emergFont, 128);

#ifdef GL_ARB_fragment_program
	if (_useDepthShader)
		glDeleteProgramsARB(1, &_fragmentProgram);
#endif
}

byte *GfxOpenGL::setupScreen(int screenW, int screenH, bool fullscreen) {
	g_system->setupScreen(screenW, screenH, fullscreen, true);

	_screenWidth = screenW;
	_screenHeight = screenH;
	_screenBPP = 24;
	_isFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	_useDepthShader = false;

	char GLDriver[1024];
	sprintf(GLDriver, "Residual: %s/%s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	g_system->setWindowCaption(GLDriver);

	// Load emergency built-in font
	loadEmergFont();

	_storedDisplay = new byte[_screenWidth * _screenHeight * 4];
	memset(_storedDisplay, 0, _screenWidth * _screenHeight * 4);
	_smushNumTex = 0;

	_currentShadowArray = NULL;

	GLfloat ambientSource[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientSource);

	glPolygonOffset(-6.0, -6.0);

	initExtensions();

	return NULL;
}

void GfxOpenGL::initExtensions()
{
#if defined (SDL_BACKEND) && defined(GL_ARB_fragment_program)
	union {
		void* obj_ptr;
		void (APIENTRY *func_ptr)();
	} u;
	// We're casting from an object pointer to a function pointer, the
	// sizes need to be the same for this to work.
	assert(sizeof(u.obj_ptr) == sizeof(u.func_ptr));
	u.obj_ptr = SDL_GL_GetProcAddress("glGenProgramsARB");
	glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glBindProgramARB");
	glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glProgramStringARB");
	glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glDeleteProgramsARB");
	glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)u.func_ptr;

	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	if (strstr(extensions, "ARB_fragment_program")) {
		_useDepthShader = true;
	}

	if (_useDepthShader) {
		glGenProgramsARB(1, &_fragmentProgram);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _fragmentProgram);

		GLint errorPos;
		glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(fragSrc), fragSrc);
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (errorPos != -1) {
			warning("Error compiling fragment program:\n%s", glGetString(GL_PROGRAM_ERROR_STRING_ARB));
		}
	}
#endif
}

const char *GfxOpenGL::getVideoDeviceName() {
	return "OpenGL Renderer";
}

void GfxOpenGL::setupCamera(float fov, float nclip, float fclip, float roll) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float right = nclip * tan(fov / 2 * (LOCAL_PI / 180));
	glFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(roll, 0, 0, -1);
}

void GfxOpenGL::positionCamera(Graphics::Vector3d pos, Graphics::Vector3d interest) {
	Graphics::Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Graphics::Vector3d(0, 1, 0);

	gluLookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
}

void GfxOpenGL::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GfxOpenGL::flipBuffer() {
	g_system->updateScreen();
}

bool GfxOpenGL::isHardwareAccelerated() {
	return true;
}

static void glShadowProjection(Graphics::Vector3d light, Graphics::Vector3d plane, Graphics::Vector3d normal, bool dontNegate) {
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

void GfxOpenGL::getBoundingBoxPos(const Model::Mesh *model, int *x1, int *y1, int *x2, int *y2) {
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
	GLdouble winX, winY, winZ;

	for (int i = 0; i < model->_numFaces; i++) {
		Graphics::Vector3d v;
		float* pVertices;

		for (int j = 0; j < model->_faces[i]._numVertices; j++) {
			GLdouble modelView[16], projection[16];
			GLint viewPort[4];

			glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			glGetIntegerv(GL_VIEWPORT, viewPort);

			pVertices = model->_vertices + 3 * model->_faces[i]._vertices[j];

			v.set(*(pVertices), *(pVertices + 1), *(pVertices + 2));

			gluProject(v.x(), v.y(), v.z(), modelView, projection, viewPort, &winX, &winY, &winZ);

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

	double t = bottom;
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
}

void GfxOpenGL::startActorDraw(Graphics::Vector3d pos, float scale, float yaw, float pitch, float roll) {
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	if (_currentShadowArray) {
		// TODO find out why shadowMask at device in woods is null
		if (!_currentShadowArray->shadowMask) {
			_currentShadowArray->shadowMask = new byte[_screenWidth * _screenHeight];
			_currentShadowArray->shadowMaskSize = _screenWidth * _screenHeight;
		}
		Sector *shadowSector = _currentShadowArray->planeList.front();
		glEnable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		//glColor3f(0.0f, 1.0f, 0.0f);
		glColor3f(_shadowColorR / 255.0f, _shadowColorG / 255.0f, _shadowColorB / 255.0f);
		glShadowProjection(_currentShadowArray->pos, shadowSector->getVertices()[0], shadowSector->getNormal(), _currentShadowArray->dontNegate);
	}
	glTranslatef(pos.x(), pos.y(), pos.z());
	glScalef(scale, scale, scale);
	glRotatef(yaw, 0, 0, 1);
	glRotatef(pitch, 1, 0, 0);
	glRotatef(roll, 0, 1, 0);
}

void GfxOpenGL::finishActorDraw() {
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	if (_currentShadowArray) {
		glEnable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void GfxOpenGL::setShadow(Shadow *shadow) {
	_currentShadowArray = shadow;
}

void GfxOpenGL::drawShadowPlanes() {
/*	glColor3f(1.0f, 1.0f, 1.0f);
	_currentShadowArray->planeList.begin();
	for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); i++) {
		Sector *shadowSector = *i;
		glBegin(GL_POLYGON);
		for (int k = 0; k < shadowSector->getNumVertices(); k++) {
			glVertex3f(shadowSector->getVertices()[k].x(), shadowSector->getVertices()[k].y(), shadowSector->getVertices()[k].z());
		}
		glEnd();
	}
*/
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClearStencil(~0);
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, (GLuint)~0);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE);
	_currentShadowArray->planeList.begin();
	for (SectorListType::iterator i = _currentShadowArray->planeList.begin(); i != _currentShadowArray->planeList.end(); ++i) {
		Sector *shadowSector = *i;
		glBegin(GL_POLYGON);
		for (int k = 0; k < shadowSector->getNumVertices(); k++) {
			glVertex3f(shadowSector->getVertices()[k].x(), shadowSector->getVertices()[k].y(), shadowSector->getVertices()[k].z());
		}
		glEnd();
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, (GLuint)~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void GfxOpenGL::setShadowMode() {
}

void GfxOpenGL::clearShadowMode() {
	glDisable(GL_STENCIL_TEST);
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
	glDepthFunc(GL_LESS);
}

void GfxOpenGL::drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) {
	// Support transparency in actor objects, such as the message tube
	// in Manny's Office
	glAlphaFunc(GL_GREATER, 0.5);
	glEnable(GL_ALPHA_TEST);
	glNormal3fv(face->_normal._coords);
	glBegin(GL_POLYGON);
	for (int i = 0; i < face->_numVertices; i++) {
		glNormal3fv(vertNormals + 3 * face->_vertices[i]);

		if (face->_texVertices)
			glTexCoord2fv(textureVerts + 2 * face->_texVertices[i]);

		glVertex3fv(vertices + 3 * face->_vertices[i]);
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
	glTranslatef(sprite->_pos.x(), sprite->_pos.y(), sprite->_pos.z());

	GLdouble modelview[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	// We want screen-aligned sprites so reset the rotation part of the matrix.
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				modelview[i * 4 + j] = 1.0;
			} else {
				modelview[i * 4 + j] = 0.0;
			}
		}
	}
	glLoadMatrixd(modelview);

	glAlphaFunc(GL_GREATER, 0.5);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);

	glBegin(GL_POLYGON);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(sprite->_width / 2, sprite->_height, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(sprite->_width / 2, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-sprite->_width / 2, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sprite->_width / 2, sprite->_height, 0.0f);
	glEnd();

	glEnable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);

	glPopMatrix();
}

void GfxOpenGL::translateViewpointStart(Graphics::Vector3d pos, float pitch, float yaw, float roll) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(yaw, 0, 0, 1);
	glRotatef(pitch, 1, 0, 0);
	glRotatef(roll, 0, 1, 0);
}

void GfxOpenGL::translateViewpointFinish() {
	glPopMatrix();
}

void GfxOpenGL::drawHierachyNode(const Model::HierNode *node) {
	if (node->_totalWeight > 0) {
		Graphics::Vector3d animPos = node->_pos + node->_animPos / node->_totalWeight;
		float animPitch = node->_pitch + node->_animPitch / node->_totalWeight;
		float animYaw = node->_yaw + node->_animYaw / node->_totalWeight;
		float animRoll = node->_roll + node->_animRoll / node->_totalWeight;
		translateViewpointStart(animPos, animPitch, animYaw, animRoll);
	} else {
		translateViewpointStart(node->_pos, node->_pitch, node->_yaw, node->_roll);
	}
	if (node->_hierVisible) {
		glPushMatrix();
		glTranslatef(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());

		if (!_currentShadowArray) {
			Sprite* sprite = node->_sprite;
			while (sprite) {
				sprite->draw();
				sprite = sprite->_next;
			}
		}

		if (node->_mesh && node->_meshVisible) {
			node->_mesh->draw();
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		if (node->_child) {
			node->_child->draw();
			glMatrixMode(GL_MODELVIEW);
		}
	}
	translateViewpointFinish();

	if (node->_sibling)
		node->_sibling->draw();
}

void GfxOpenGL::disableLights() {
	glDisable(GL_LIGHTING);
}

void GfxOpenGL::setupLight(Scene::Light *light, int lightId) {
	glEnable(GL_LIGHTING);
	float lightColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightDir[] = { 0.0f, 0.0f, -1.0f };

	float intensity = light->_intensity / 1.3f;
	lightColor[0] = ((float)light->_color.getRed() / 15.0f) * intensity;
	lightColor[1] = ((float)light->_color.getGreen() / 15.0f) * intensity;
	lightColor[2] = ((float)light->_color.getBlue() / 15.0f) * intensity;

	if (light->_type == "omni") {
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
		glDisable(GL_LIGHT0 + lightId);
		glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, lightColor);
		glLightfv(GL_LIGHT0 + lightId, GL_POSITION, lightPos);
		glEnable(GL_LIGHT0 + lightId);
	} else if (light->_type == "direct") {
		glDisable(GL_LIGHT0 + lightId);
		lightPos[0] = -light->_dir.x();
		lightPos[1] = -light->_dir.y();
		lightPos[2] = -light->_dir.z();
		lightPos[3] = 0;
		glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, lightColor);
		glLightfv(GL_LIGHT0 + lightId, GL_POSITION, lightPos);
		glEnable(GL_LIGHT0 + lightId);
	} else if (light->_type == "spot") {
		glDisable(GL_LIGHT0 + lightId);
		lightPos[0] = light->_pos.x();
		lightPos[1] = light->_pos.y();
		lightPos[2] = light->_pos.z();
		lightDir[0] = light->_dir.x();
		lightDir[1] = light->_dir.y();
		lightDir[2] = light->_dir.z();
		glLightfv(GL_LIGHT0 + lightId, GL_DIFFUSE, lightColor);
		glLightfv(GL_LIGHT0 + lightId, GL_POSITION, lightPos);
		glLightfv(GL_LIGHT0 + lightId, GL_SPOT_DIRECTION, lightDir);
		glLightf(GL_LIGHT0 + lightId, GL_SPOT_CUTOFF, light->_penumbraangle);
		glEnable(GL_LIGHT0 + lightId);
	} else {
		error("Scene::setupLights() Unknown type of light: %s", light->_type.c_str());
	}
}

#define BITMAP_TEXTURE_SIZE 256

void GfxOpenGL::createBitmap(BitmapData *bitmap) {
	GLuint *textures;

	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(bitmap->getImageData(pic));
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bitmap->getImageData(pic) + 2 * i);
				zbufPtr[i] = 0xffff - ((uint32) val) * 0x10000 / 100 / (0x10000 - val);
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

		byte *texData = 0;
		byte *texOut = 0;

		GLint format = GL_RGBA;
		GLint type = GL_UNSIGNED_BYTE;
		int bytes = 4;
		if (bitmap->_format != 1) {
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_SHORT;
			bytes = 2;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, bytes);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, bitmap->_width);

		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			if (bitmap->_format == 1 && bitmap->_bpp == 16 && bitmap->_colorFormat != BM_RGB1555) {
				if (texData == 0)
					texData = new byte[4 * bitmap->_width * bitmap->_height];
				// Convert data to 32-bit RGBA format
				byte *texDataPtr = texData;
				uint16 *bitmapData = reinterpret_cast<uint16 *>(bitmap->getImageData(pic));
				for (int i = 0; i < bitmap->_width * bitmap->_height; i++, texDataPtr += 4, bitmapData++) {
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
				bitmap->convertToColorFormat(pic, BM_RGBA);
				texOut = (byte *)bitmap->getImageData(pic);
			} else {
				texOut = (byte *)bitmap->getImageData(pic);
			}

			for (int i = 0; i < bitmap->_numTex; i++) {
				glBindTexture(GL_TEXTURE_2D, textures[bitmap->_numTex * pic + i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexImage2D(GL_TEXTURE_2D, 0, format, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, format, type, NULL);
			}

			int cur_tex_idx = bitmap->_numTex * pic;

			for (int y = 0; y < bitmap->_height; y += BITMAP_TEXTURE_SIZE) {
				for (int x = 0; x < bitmap->_width; x += BITMAP_TEXTURE_SIZE) {
					int width  = (x + BITMAP_TEXTURE_SIZE >= bitmap->_width)  ? (bitmap->_width  - x) : BITMAP_TEXTURE_SIZE;
					int height = (y + BITMAP_TEXTURE_SIZE >= bitmap->_height) ? (bitmap->_height - y) : BITMAP_TEXTURE_SIZE;
					glBindTexture(GL_TEXTURE_2D, textures[cur_tex_idx]);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type,
						texOut + (y * bytes * bitmap->_width) + (bytes * x));
					cur_tex_idx++;
				}
			}
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		delete[] texData;
	}
}

void GfxOpenGL::drawBitmap(const Bitmap *bitmap) {
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
	} else
		glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

	// If drawing a Z-buffer image, but no shaders are available, fall back to the glDrawPixels method.
	if (bitmap->getFormat() == 5 && !_useDepthShader) {
		// Only draw the manual zbuffer when enabled
		if (bitmap->getCurrentImage() - 1 < bitmap->getNumImages()) {
			drawDepthBitmap(bitmap->getX(), bitmap->getY(), bitmap->getWidth(), bitmap->getHeight(), bitmap->getData(bitmap->getCurrentImage() - 1));
		} else {
			warning("zbuffer image has index out of bounds! %d/%d", bitmap->getCurrentImage(), bitmap->getNumImages());
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
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
#endif
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor(bitmap->getX(), _screenHeight - (bitmap->getY() + bitmap->getHeight()), bitmap->getWidth(), bitmap->getHeight());
	int cur_tex_idx = bitmap->getNumTex() * (bitmap->getCurrentImage() - 1);
	for (int y = bitmap->getY(); y < (bitmap->getY() + bitmap->getHeight()); y += BITMAP_TEXTURE_SIZE) {
		for (int x = bitmap->getX(); x < (bitmap->getX() + bitmap->getWidth()); x += BITMAP_TEXTURE_SIZE) {
			textures = (GLuint *)bitmap->getTexIds();
			glBindTexture(GL_TEXTURE_2D, textures[cur_tex_idx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2i(x, y);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
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
		glDepthFunc(GL_LESS);
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
	}
}

void GfxOpenGL::createMaterial(Material *material, const char *data, const CMap *cmap) {
	material->_textures = new GLuint[material->_numImages];
	glGenTextures(material->_numImages, (GLuint *)material->_textures);
	char *texdata = new char[material->_width * material->_height * 4];
	for (int i = 0; i < material->_numImages; i++) {
		char *texdatapos = texdata;
		for (int y = 0; y < material->_height; y++) {
			for (int x = 0; x < material->_width; x++) {
				uint8 col = *(uint8 *)(data);
				if (col == 0) {
					memset(texdatapos, 0, 4); // transparent
				} else {
					memcpy(texdatapos, cmap->_colors + 3 * (col), 3);
					texdatapos[3] = '\xff'; // fully opaque
				}
				texdatapos += 4;
				data++;
			}
		}

		GLuint *textures = (GLuint *)material->_textures;
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, material->_width, material->_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
		data += 24;
	}
	delete[] texdata;
}

void GfxOpenGL::selectMaterial(const Material *material) {
	GLuint *textures = (GLuint *)material->_textures;
	glBindTexture(GL_TEXTURE_2D, textures[material->_currImage]);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(1.0f / material->_width, 1.0f / material->_height, 1);
}

void GfxOpenGL::destroyMaterial(Material *material) {
	GLuint *textures = (GLuint *)material->_textures;
	if (textures) {
		glDeleteTextures(material->_numImages, textures);
		delete[] textures;
	}
}

void GfxOpenGL::drawDepthBitmap(int x, int y, int w, int h, char *data) {
	//	if (num != 0) {
	//		warning("Animation not handled yet in GL texture path");
	//	}

	if (y + h == 480) {
		glRasterPos2i(x, _screenHeight - 1);
		glBitmap(0, 0, 0, 0, 0, -1, NULL);
	} else
		glRasterPos2i(x, y + h);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

	glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LESS);
}

void GfxOpenGL::prepareSmushFrame(int width, int height, byte *bitmap) {
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	int curTexIdx = 0;
	for (int y = 0; y < height; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < width; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= width) ? (width - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= height) ? (height - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, bitmap + (y * 2 * width) + (2 * x));
			curTexIdx++;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	_smushWidth = width;
	_smushHeight = height;
}

void GfxOpenGL::drawSmushFrame(int offsetX, int offsetY) {
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

	glScissor(offsetX, _screenHeight - (offsetY + _smushHeight), _smushWidth, _smushHeight);

	int curTexIdx = 0;
	for (int y = 0; y < _smushHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _smushWidth; x += BITMAP_TEXTURE_SIZE) {
			glBindTexture(GL_TEXTURE_2D, _smushTexIds[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(x + offsetX, y + offsetY);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2i(x + offsetX + BITMAP_TEXTURE_SIZE, y + offsetY + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2i(x + offsetX, y + offsetY + BITMAP_TEXTURE_SIZE);
			glEnd();
			curTexIdx++;
		}
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::releaseSmushFrame() {
	if (_smushNumTex > 0) {
		glDeleteTextures(_smushNumTex, _smushTexIds);
		delete[] _smushTexIds;
		_smushNumTex = 0;
	}
}

void GfxOpenGL::loadEmergFont() {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	_emergFont = glGenLists(128);
	for (int i = 32; i < 127; i++) {
		glNewList(_emergFont + i, GL_COMPILE);
		glBitmap(8, 13, 0, 2, 10, 0, Font::emerFont[i - 32]);
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
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *)text);

	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

GfxBase::TextObjectHandle *GfxOpenGL::createTextBitmap(uint8 *data, int width, int height, const Color &fgColor) {
	TextObjectHandle *handle = new TextObjectHandle;
	handle->width = width;
	handle->height = height;
	handle->bitmapData = NULL;
	handle->surface = NULL;

	// create texture
	handle->numTex = ((width + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
		((height + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
	handle->texIds = (GLuint *)new GLuint[handle->numTex];
	glGenTextures(handle->numTex, (GLuint *)handle->texIds);

	// Convert data to 32-bit RGBA format
	byte *texData = new byte[4 * width * height];
	byte *texDataPtr = texData;
	uint8 *bitmapData = data;
	uint8 r = fgColor.getRed();
	uint8 g = fgColor.getGreen();
	uint8 b = fgColor.getBlue();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	for (int i = 0; i < width * height; i++, texDataPtr += 4, bitmapData++) {
		byte pixel = *bitmapData;
		if (pixel == 0x00) {
			texDataPtr[0] = 0;
			texDataPtr[1] = 0;
			texDataPtr[2] = 0;
			texDataPtr[3] = 0;
		} else if (pixel == 0x80) {
			texDataPtr[0] = 0;
			texDataPtr[1] = 0;
			texDataPtr[2] = 0;
			texDataPtr[3] = 255;
		} else if (pixel == 0xFF) {
			texDataPtr[0] = r;
			texDataPtr[1] = g;
			texDataPtr[2] = b;
			texDataPtr[3] = 255;
		}
	}

	for (int i = 0; i < handle->numTex; i++) {
		glBindTexture(GL_TEXTURE_2D, ((GLuint *)handle->texIds)[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	int curTexIdx = 0;
	for (int y = 0; y < height; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < width; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= width) ? (width - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= height) ? (height - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, ((GLuint *)handle->texIds)[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGBA, GL_UNSIGNED_BYTE, texData + (y * 4 * width) + (4 * x));
			curTexIdx++;
		}
	}

	delete[] texData;
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	return handle;
}

void GfxOpenGL::drawTextBitmap(int x, int y, TextObjectHandle *handle) {
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

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 480 - (y + handle->height), handle->width, handle->height);
	int curTexIdx = 0;
	for (int t_y = 0; t_y < handle->height; t_y += BITMAP_TEXTURE_SIZE) {
		for (int t_x = 0; t_x < handle->width; t_x += BITMAP_TEXTURE_SIZE) {
			GLuint *textures = (GLuint *)handle->texIds;
			glBindTexture(GL_TEXTURE_2D, textures[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2i(t_x + x, t_y + y);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2i(t_x + x + BITMAP_TEXTURE_SIZE, y + t_y);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2i(t_x + x + BITMAP_TEXTURE_SIZE, y + t_y + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2i(t_x + x, t_y + y + BITMAP_TEXTURE_SIZE);
			glEnd();
			curTexIdx++;
		}
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::destroyTextBitmap(TextObjectHandle *handle) {
	GLuint *textures = (GLuint *)handle->texIds;
	if (textures) {
		glDeleteTextures(handle->numTex, textures);
		delete[] textures;
	}
}

Bitmap *GfxOpenGL::getScreenshot(int w, int h) {
	uint16 *buffer = new uint16[w * h];
	uint32 *src = (uint32 *)_storedDisplay;

	int step = 0;
	for (int y = 0; y <= 479; y++) {
		for (int x = 0; x <= 639; x++) {
			uint32 pixel = *(src + y * 640 + x);
			uint8 r = (pixel & 0xFF0000);
			uint8 g = (pixel & 0x00FF00);
			uint8 b = (pixel & 0x0000FF);
			uint32 color = (r + g + b) / 3;
			src[step++] = ((color << 24) | (color << 16) | (color << 8) | color);
		}
	}

	float step_x = _screenWidth * 1.0f / w;
	float step_y = _screenHeight * 1.0f / h;
	step = 0;
	for (float y = 0; y < 479; y += step_y) {
		for (float x = 0; x < 639; x += step_x) {
			uint32 pixel = *(src + (int)y * _screenWidth + (int)x);
			uint8 r = (pixel & 0xFF0000) >> 16;
			uint8 g = (pixel & 0x00FF00) >> 8;
			uint8 b = (pixel & 0x0000FF);
			int pos = step / w;
			int wpos = step - pos * w;
			// source is upside down, flip appropriately while storing
			buffer[h * w - (pos * w + w - wpos)] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			step++;
		}
	}

	Bitmap *screenshot = g_grim->registerBitmap((char *)buffer, w, h, "screenshot");
	delete[] buffer;
	return screenshot;
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
	glBitmap(0, 0, 0, 0, 0, -1, NULL);
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
	uint32 *data = new uint32[w * h];
	int y = _screenHeight - yReal;

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

void GfxOpenGL::drawRectangle(PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = *primitive->getColor();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3f(color.getRed() / 255.0f, color.getGreen() / 255.0f, color.getBlue() / 255.0f);

	if (primitive->isFilled()) {
		glBegin(GL_QUADS);
	} else {
		glBegin(GL_LINE_LOOP);
	}

	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::drawLine(PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;

	const Color &color = *primitive->getColor();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3f(color.getRed() / 255.0f, color.getGreen() / 255.0f, color.getBlue() / 255.0f);

	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void GfxOpenGL::drawPolygon(PrimitiveObject *primitive) {
	int x1 = primitive->getP1().x;
	int y1 = primitive->getP1().y;
	int x2 = primitive->getP2().x;
	int y2 = primitive->getP2().y;
	int x3 = primitive->getP3().x;
	int y3 = primitive->getP3().y;
	int x4 = primitive->getP4().x;
	int y4 = primitive->getP4().y;

	const Color &color = *primitive->getColor();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenWidth, _screenHeight, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3f(color.getRed() / 255.0f, color.getGreen() / 255.0f, color.getBlue() / 255.0f);

	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();

	glBegin(GL_LINES);
	glVertex2i(x3, y3);
	glVertex2i(x4, y4);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

} // end of namespace Grim

#endif
