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

DriverTinyGL::DriverTinyGL(int screenW, int screenH, int screenBPP, bool fullscreen) {
	uint32 flags = SDL_HWSURFACE;

	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	_screen = SDL_SetVideoMode(screenW, screenH, screenBPP, flags);
	if (_screen == NULL)
		error("Could not initialize video");

	_screenWidth = screenW;
	_screenHeight = screenH;
	_screenBPP = screenBPP;
	_isFullscreen = fullscreen;

	SDL_WM_SetCaption("Residual: Software 3D Renderer", "Residual");

	_zb = ZB_open(screenW, screenH, ZB_MODE_5R6G5B, 0, NULL, NULL, _screen->pixels);
	tglInit(_zb);

	_storedDisplay = new byte[640 * 480 * 2];
	memset(_storedDisplay, 0, 640 * 480 * 2);
}

DriverTinyGL::~DriverTinyGL() {
	delete []_storedDisplay;
	tglClose();
	ZB_close(_zb);
}

void DriverTinyGL::toggleFullscreenMode() {
	uint32 flags = SDL_HWSURFACE;

	if (!_isFullscreen)
		flags |= SDL_FULLSCREEN;
	if (SDL_SetVideoMode(_screenWidth, _screenHeight, _screenBPP, flags) == 0)
		warning("Could not change fullscreen mode");
	else
		_isFullscreen = !_isFullscreen;
}

void DriverTinyGL::setupCamera(float fov, float nclip, float fclip, float roll) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float right = nclip * std::tan(fov / 2 * (M_PI / 180));
	tglFrustum(-right, right, -right * 0.75, right * 0.75, nclip, fclip);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglRotatef(roll, 0, 0, -1);
}

void DriverTinyGL::positionCamera(Vector3d pos, Vector3d interest) {
	Vector3d up_vec(0, 0, 1);

	if (pos.x() == interest.x() && pos.y() == interest.y())
		up_vec = Vector3d(0, 1, 0);

	lookAt(pos.x(), pos.y(), pos.z(), interest.x(), interest.y(), interest.z(), up_vec.x(), up_vec.y(), up_vec.z());
}

void DriverTinyGL::clearScreen() {
	memset(_zb->pbuf, 0, 640 * 480 * 2);
	memset(_zb->zbuf, 0, 640 * 480 * 2);
}

void DriverTinyGL::flipBuffer() {
	SDL_Flip(_screen);
}

bool DriverTinyGL::isHardwareAccelerated() {
	return false;
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

void DriverTinyGL::disableLights() {
	tglDisable(TGL_LIGHTING);
}

void DriverTinyGL::setupLight(Scene::Light *light, int lightId) {
	tglEnable(TGL_LIGHTING);
	float ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float diffuseLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float specularLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightPos[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float lightDir[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	lightPos[0] = light->_pos.x();
	lightPos[1] = light->_pos.y();
	lightPos[2] = light->_pos.z();
	ambientLight[0] = (float)light->_color.red() / 256.0f;
	ambientLight[1] = (float)light->_color.blue() / 256.0f;
	ambientLight[2] = (float)light->_color.green() / 256.0f;
//	diffuseLight[0] = (float)light->_intensity;
//	diffuseLight[1] = (float)light->_intensity;
//	diffuseLight[2] = (float)light->_intensity;

	if (strcmp(light->_type.c_str(), "omni") == 0) {
//		tglLightfv(TGL_LIGHT0 + lightId, TGL_AMBIENT, ambientLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_DIFFUSE, diffuseLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_SPECULAR, specularLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_POSITION, lightPos);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_CUTOFF, 1.8f);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_LINEAR_ATTENUATION, light->_intensity);
		tglEnable(TGL_LIGHT0 + lightId);
	} else if (strcmp(light->_type.c_str(), "direct") == 0) {
		lightDir[0] = light->_dir.x();
		lightDir[1] = light->_dir.y();
		lightDir[2] = light->_dir.z();
		lightDir[3] = 0.0f;
//		tglLightfv(TGL_LIGHT0 + lightId, TGL_AMBIENT, ambientLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_DIFFUSE, diffuseLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_SPECULAR, specularLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_POSITION, lightPos);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_SPOT_DIRECTION, lightDir);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_CUTOFF, 1.8f);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_EXPONENT, 2.0f);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_LINEAR_ATTENUATION, light->_intensity);
		tglEnable(TGL_LIGHT0 + lightId);
	} else if (strcmp(light->_type.c_str(), "spot") == 0) {
		lightDir[0] = light->_dir.x();
		lightDir[1] = light->_dir.y();
		lightDir[2] = light->_dir.z();
		lightDir[3] = 0.0f;
//		tglLightfv(TGL_LIGHT0 + lightId, TGL_AMBIENT, ambientLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_DIFFUSE, diffuseLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_SPECULAR, specularLight);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_POSITION, lightPos);
		tglLightfv(TGL_LIGHT0 + lightId, TGL_SPOT_DIRECTION, lightDir);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_CUTOFF, 1.8f);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_SPOT_EXPONENT, 2.0f);
//		tglLightf(TGL_LIGHT0 + lightId, TGL_LINEAR_ATTENUATION, light->_intensity);
		tglEnable(TGL_LIGHT0 + lightId);
	} else {
		error("Scene::setupLights() Unknown type of light: %s", light->_type.c_str());
	}
}

void DriverTinyGL::createBitmap(Bitmap *bitmap) {
	if (bitmap->_format != 1) {
		for (int pic = 0; pic < bitmap->_numImages; pic++) {
			uint16 *bufPtr = reinterpret_cast<uint16 *>(bitmap->_data[pic]);
			for (int i = 0; i < (bitmap->_width * bitmap->_height); i++) {
				uint16 val = READ_LE_UINT16(bitmap->_data[pic] + 2 * i);
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
		x = 0;
		srcX = -x;
	} else {
		srcX = 0;
	}
	if (y < 0) {
		y = 0;
		srcY = -y;
	} else {
		srcY = 0;
	}

	if (x + width > 639)
		width -= (x + width) - 639;

	if (y + height > 479)
		height -= (y + height) - 479;

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
				uint16 pixel = READ_LE_UINT16(src + r);
				if (pixel != 0xf81f)
					WRITE_LE_UINT16(dst + r, pixel);
			}
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

void DriverTinyGL::drawBitmap(const Bitmap *bitmap) {
	assert(bitmap->_currImage > 0);
	if (bitmap->_format == 1)
		TinyGLBlit((byte *)_zb->pbuf, (byte *)bitmap->_data[bitmap->_currImage - 1],
			bitmap->x(), bitmap->y(), bitmap->width(), bitmap->height(), true);
	else
		TinyGLBlit((byte *)_zb->zbuf, (byte *)bitmap->_data[bitmap->_currImage - 1],
			bitmap->x(), bitmap->y(), bitmap->width(), bitmap->height(), false);
}

void DriverTinyGL::destroyBitmap(Bitmap *) { }

void DriverTinyGL::drawDepthBitmap(int, int, int, int, char *) { }

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
	delete[] (TGLuint *)material->_textures;
}

void DriverTinyGL::prepareSmushFrame(int width, int height, byte *bitmap) {
	_smushWidth = width;
	_smushHeight = height;
	_smushBitmap = bitmap;
}

void DriverTinyGL::drawSmushFrame(int offsetX, int offsetY) {
	if (_smushWidth == 640 && _smushHeight == 480) {
		memcpy(_zb->pbuf, _smushBitmap, 640 * 480 * 2);
	} else {
		TinyGLBlit((byte *)_zb->pbuf, _smushBitmap, offsetX, offsetY, _smushWidth, _smushHeight, false);
	}
}

void DriverTinyGL::loadEmergFont() {
}

void DriverTinyGL::drawEmergString(int /*x*/, int /*y*/, const char * /*text*/, const Color &/*fgColor*/) {
}

Driver::TextObjectHandle *DriverTinyGL::createTextBitmap(uint8 *data, int width, int height, const Color &fgColor) {
	TextObjectHandle *handle = new TextObjectHandle;
	handle->width = width;
	handle->height = height;
	handle->numTex = 0;
	handle->texIds = NULL;

	// Convert data to 16-bit RGB 565 format
	uint16 *texData = new uint16[width * height];
	uint16 *texDataPtr = texData;
	handle->bitmapData = texData;
	uint8 *bitmapData = data;
	uint8 r = fgColor.red();
	uint8 g = fgColor.green();
	uint8 b = fgColor.blue();

	for (int i = 0; i < width * height; i++, texDataPtr++, bitmapData++) {
		byte pixel = *bitmapData;
		if (pixel == 0x00) {
			WRITE_LE_UINT16(texDataPtr, 0xf81f);
		} else if (pixel == 0x80) {
			*texDataPtr = 0;
		} else if (pixel == 0xFF) {
			WRITE_LE_UINT16(texDataPtr, ((r & 0xF8) << 8) |
				((g & 0xFC) << 3) | (b >> 3));
		}
	}

	return handle;
}

void DriverTinyGL::drawTextBitmap(int x, int y, TextObjectHandle *handle) {
	TinyGLBlit((byte *)_zb->pbuf, (byte *)handle->bitmapData, x, y, handle->width, handle->height, true);
}

void DriverTinyGL::destroyTextBitmap(TextObjectHandle *handle) {
	delete[] handle->bitmapData;
}

Bitmap *DriverTinyGL::getScreenshot(int w, int h) {
	uint16 *buffer = new uint16[w * h];
	uint16 *src = (uint16 *)_zb->pbuf;
	assert(buffer);

	float step_x = 640.0 / w;
	float step_y = 480.0 / h;
	int step = 0;
	for (float y = 0; y < 479; y += step_y) {
		for (float x = 0; x < 639; x += step_x) {
			buffer[step++] = *(src + (int)y * 640 + (int)x);
		}
	}

	Bitmap *screenshot = new Bitmap((char *)buffer, w, h, "screenshot");
	delete []buffer;
	return screenshot;
}

void DriverTinyGL::storeDisplay() {
	memcpy(_storedDisplay, _zb->pbuf, 640 * 480 * 2);
}

void DriverTinyGL::copyStoredToDisplay() {
	memcpy(_zb->pbuf, _storedDisplay, 640 * 480 * 2);
}

void DriverTinyGL::drawDim() {
}

void DriverTinyGL::drawRectangle(PrimitiveObject *primitive) {
	uint16 *dst = (uint16 *)_zb->pbuf;
	int x1 = primitive->getX1();
	int x2 = primitive->getX2();
	int y1 = primitive->getY1();
	int y2 = primitive->getY2();

	Color color = primitive->getColor();
	uint16 c = ((color.red() & 0xF8) << 8) | ((color.green() & 0xFC) << 3) | (color.blue() >> 3);

	if (primitive->isFilled()) {
		for (; y1 <= y2; y1++) {
			for (int x = x1; x <= x2; x++) {
				WRITE_LE_UINT16(dst + 640 * y1 + x, c);
			}
		}
	} else {
		for (int x = x1; x <= x2; x++) {
			WRITE_LE_UINT16(dst + 640 * y1 + x, c);
		}
		for (int x = x1; x <= x2; x++) {
			WRITE_LE_UINT16(dst + 640 * y2 + x, c);
		}
		for (int y = y1; y <= y2; y++) {
			WRITE_LE_UINT16(dst + 640 * y + x1, c);
		}
		for (int y = y1; y <= y2; y++) {
			WRITE_LE_UINT16(dst + 640 * y + x2, c);
		}
	}
}

void DriverTinyGL::drawLine(PrimitiveObject *primitive) {
	uint16 *dst = (uint16 *)_zb->pbuf;
	int x1 = primitive->getX1();
	int x2 = primitive->getX2();
	int y1 = primitive->getY1();
	int y2 = primitive->getY2();
	float m = (y2 - y1) / (x2 - x1);
	int b = (int)(-m * x1 + y1);

	Color color = primitive->getColor();
	uint16 c = ((color.red() & 0xF8) << 8) | ((color.green() & 0xFC) << 3) | (color.blue() >> 3);

	for (int x = x1; x <= x2; x++) {
		int y = (int) (m * x) + b;
		WRITE_LE_UINT16(dst + 640 * y + x, c);
	}
}
