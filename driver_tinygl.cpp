// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "debug.h"
#include "colormap.h"
#include "material.h"
#include "driver_tinygl.h"

#include "tinygl/gl.h"
#include "tinygl/zgl.h"


// func below is from Mesa glu sources
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
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
	tglMultMatrixf(m);

	tglTranslatef(-eyex, -eyey, -eyez);
}

DriverTinyGL::DriverTinyGL(int screenW, int screenH, int screenBPP) {
	_screen = SDL_SetVideoMode(screenW, screenH, screenBPP, SDL_HWSURFACE);
	if (_screen == NULL)
		error("Could not initialize video");

	SDL_WM_SetCaption("Residual: Modified TinyGL - Software Renderer", "Residual");

	byte *frameBuffer = (byte *)_screen->pixels;
	_zb = ZB_open(screenW, screenH, ZB_MODE_5R6G5B, 0, NULL, NULL, frameBuffer);
	tglInit(_zb);

	_zbufferSurface = SDL_CreateRGBSurfaceFrom(_zb->zbuf, screenW, screenH, 16, screenW * 2, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
	_fullScreenBitmapData = NULL;
	_fullScreenZBitmapData = NULL;
	_smushSurface = NULL;
}

DriverTinyGL::~DriverTinyGL() {
	SDL_FreeSurface(_zbufferSurface);
	tglClose();
	ZB_close(_zb);
}

void DriverTinyGL::setupCamera(float fov, float nclip, float fclip, float roll) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float right = nclip * std::tan(fov / 2 * (M_PI / 180));
	tglFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	Vector3d up_vec(0, 0, 1);
	tglRotatef(roll, 0, 0, -1);
}

void DriverTinyGL::positionCamera(Vector3d pos, Vector3d interest) {
	Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Vector3d(0, 1, 0);

	lookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
  }

void DriverTinyGL::clearScreen() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
}

void DriverTinyGL::flipBuffer() {
	SDL_Flip(_screen);
}

void DriverTinyGL::startActorDraw(Vector3d pos, float yaw, float pitch, float roll) {
	tglEnable(TGL_TEXTURE_2D);
	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglTranslatef(pos.x(), pos.y(), pos.z());
	tglRotatef(yaw, 0, 0, 1);
	tglRotatef(pitch, 1, 0, 0);
	tglRotatef(roll, 0, 1, 0);
}

void DriverTinyGL::finishActorDraw() {
	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
	tglDisable(TGL_TEXTURE_2D);
}

void DriverTinyGL::set3DMode() {
	tglMatrixMode(TGL_MODELVIEW);
	tglEnable(TGL_DEPTH_TEST);
}

void DriverTinyGL::drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) {
	tglNormal3fv((float *)face->_normal._coords);
	tglBegin(TGL_POLYGON);
	for (int i = 0; i < face->_numVertices; i++) {
		tglNormal3fv(vertNormals + 3 * face->_vertices[i]);

		if (face->_texVertices != NULL)
			tglTexCoord2fv(textureVerts + 2 * face->_texVertices[i]);

		tglVertex3fv(vertices + 3 * face->_vertices[i]);
	}
	tglEnd();
}

void DriverTinyGL::drawHierachyNode(const Model::HierNode *node) {
	if (node->_hierVisible) {
		tglPushMatrix();

		tglTranslatef(node->_animPos.x() / node->_totalWeight, node->_animPos.y() / node->_totalWeight, node->_animPos.z() / node->_totalWeight);
		tglRotatef(node->_animYaw / node->_totalWeight, 0, 0, 1);
		tglRotatef(node->_animPitch / node->_totalWeight, 1, 0, 0);
		tglRotatef(node->_animRoll / node->_totalWeight, 0, 1, 0);

		if (node->_mesh != NULL && node->_meshVisible) {
			tglPushMatrix();
			tglTranslatef(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());
			node->_mesh->draw();
			tglMatrixMode(TGL_MODELVIEW);
			tglPopMatrix();
		}

		if (node->_child != NULL) {
			node->_child->draw();
			tglMatrixMode(TGL_MODELVIEW);
		}
		tglPopMatrix();
	}

	if (node->_sibling != NULL)
		node->_sibling->draw();
}

void DriverTinyGL::createBitmap(Bitmap *bitmap) {
	if (bitmap->_format == 1) {
	} else {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16 *>(bitmap->_data[pic]);
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bitmap->_data[pic] + 2 * i);
				zbufPtr[i] = ((uint32) val) * 0x10000 / 100 / (0x10000 - val);
			}
		}
	}
}

void DriverTinyGL::drawBitmap(const Bitmap *bitmap) {
	SDL_Surface *tmpSurface = NULL;
	SDL_Rect srcRect, dstRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = bitmap->width();
	srcRect.h = bitmap->height();
	dstRect.x = bitmap->x();
	dstRect.y = bitmap->y();
	dstRect.w = bitmap->width();
	dstRect.h = bitmap->height();

	if (bitmap->_format == 1) {
		char *tmp = bitmap->_data[bitmap->_currImage - 1];
		tmpSurface = SDL_CreateRGBSurfaceFrom(tmp, bitmap->width(), bitmap->height(),
			16, bitmap->width() * 2, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
		SDL_SetColorKey(tmpSurface, SDL_SRCCOLORKEY, 0xf81f);
		SDL_BlitSurface(tmpSurface, &srcRect, _screen, &dstRect);
		SDL_FreeSurface(tmpSurface);
	} else {
		char *tmp = bitmap->_data[bitmap->_currImage - 1];
		tmpSurface = SDL_CreateRGBSurfaceFrom(tmp, bitmap->width(), bitmap->height(),
			16, bitmap->width() * 2, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
		SDL_BlitSurface(tmpSurface, &srcRect, _zbufferSurface, &dstRect);
		SDL_FreeSurface(tmpSurface);
	}
}

void DriverTinyGL::destroyBitmap(Bitmap *bitmap) {
}

void DriverTinyGL::createMaterial(Material *material, const char *data, const CMap *cmap) {
	material->_textures = new TGLuint[material->_numImages];
	tglGenTextures(material->_numImages, (TGLuint *)material->_textures);
	char *texdata = new char[material->_width * material->_height * 4];
	for (int i = 0; i < material->_numImages; i++) {
		char *texdatapos = texdata;
		for (int y = 0; y < material->_height; y++) {
			for (int x = 0; x < material->_width; x++) {
				int col = *(uint8 *)(data);
				if (col == 0)
					memset(texdatapos, 0, 3); // transparent
				else {
					memcpy(texdatapos, cmap->_colors + 3 * (*(uint8 *)(data)), 3);
				}
				texdatapos += 3;
				data++;
			}
		}
		TGLuint *textures = (TGLuint *)material->_textures;
		tglBindTexture(TGL_TEXTURE_2D, textures[i]);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
		tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, /*TGL_REPLACE*/ TGL_DECAL);
		tglTexImage2D(TGL_TEXTURE_2D, 0, 3, material->_width, material->_height, 0, TGL_RGB, TGL_UNSIGNED_BYTE, texdata);
		data += 24;
	}
	delete[] texdata;
}

void DriverTinyGL::selectMaterial(const Material *material) {
	TGLuint *textures = (TGLuint *)material->_textures;
	tglBindTexture(TGL_TEXTURE_2D, textures[material->_currImage]);
	tglPushMatrix();
	tglMatrixMode(TGL_TEXTURE);
	tglLoadIdentity();
	tglScalef(1.0f / material->_width, 1.0f / material->_height, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
}

void DriverTinyGL::destroyMaterial(Material *material) {
	tglDeleteTextures(material->_numImages, (TGLuint *)material->_textures);
	delete[] material->_textures;
}

void DriverTinyGL::drawDepthBitmap(int x, int y, int w, int h, char *data) {
}

void DriverTinyGL::prepareSmushFrame(int width, int height, byte *bitmap) {
	_smushWidth = width;
	_smushHeight = height;
	if (_smushSurface) {
		SDL_FreeSurface(_smushSurface);
		_smushSurface = NULL;
	}
	_smushSurface = SDL_CreateRGBSurfaceFrom(bitmap, _smushWidth, _smushHeight, 16, _smushWidth * 2, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);
}

void DriverTinyGL::drawSmushFrame(int offsetX, int offsetY) {
	SDL_Rect srcRect, dstRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = _smushWidth;
	srcRect.h = _smushHeight;
	dstRect.x = offsetX;
	dstRect.y = offsetY;
	dstRect.w = _smushWidth;
	dstRect.h = _smushHeight;

	SDL_BlitSurface(_smushSurface, &srcRect, _screen, &dstRect);
}

void DriverTinyGL::loadEmergFont() {
}

void DriverTinyGL::drawEmergString(int x, int y, const char *text, const Color &fgColor) {
}
