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

// Matrix calculations taken from the glm library
// Which is covered by the MIT license
// And has this additional copyright note:
/* Copyright (c) 2005 - 2012 G-Truc Creation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#include "engines/grim/gfx_base.h"
#include "engines/grim/savegame.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/grim.h"

#include "engines/grim/model.h"

namespace Grim {

GfxBase::GfxBase() :
		_renderBitmaps(true), _renderZBitmaps(true), _shadowModeActive(false),
		_currentPos(0, 0, 0), _dimLevel(0.0f),
		_screenWidth(0), _screenHeight(0),
		_scaleW(1.0f), _scaleH(1.0f), _currentShadowArray(nullptr),
		_shadowColorR(255), _shadowColorG(255), _shadowColorB(255) {
			for (unsigned int i = 0; i < _numSpecialtyTextures; i++) {
				_specialtyTextures[i]._isShared = true;
			}
}

void GfxBase::setShadowMode() {
	_shadowModeActive = true;
}

void GfxBase::clearShadowMode() {
	_shadowModeActive = false;
}

bool GfxBase::isShadowModeActive() {
	return _shadowModeActive;
}

void GfxBase::saveState(SaveGame *state) {
	state->beginSection('DRVR');

	byte r, g, b;
	getShadowColor(&r, &g, &b);
	state->writeByte(r);
	state->writeByte(g);
	state->writeByte(b);
	state->writeBool(_renderBitmaps);
	state->writeBool(_renderZBitmaps);

	state->endSection();
}

void GfxBase::restoreState(SaveGame *state) {
	state->beginSection('DRVR');

	byte r, g, b;
	r = state->readByte();
	g = state->readByte();
	b = state->readByte();
	setShadowColor(r, g , b);
	_renderBitmaps = state->readBool();
	_renderZBitmaps = state->readBool();

	state->endSection();
}

void GfxBase::renderBitmaps(bool render) {
	_renderBitmaps = render;
}

void GfxBase::renderZBitmaps(bool render) {
	_renderZBitmaps = render;
}

void GfxBase::drawMesh(const Mesh *mesh) {
	for (int i = 0; i < mesh->_numFaces; i++)
		mesh->_faces[i].draw(mesh);
}

#ifndef USE_OPENGL_GAME
// Allow CreateGfxOpenGL to be called even if OpenGL isn't included
GfxBase *CreateGfxOpenGL() {
	return CreateGfxTinyGL();
}
#endif // USE_OPENGL_GAME

Math::Matrix4 GfxBase::makeLookMatrix(const Math::Vector3d& pos, const Math::Vector3d& interest, const Math::Vector3d& up) {
	Math::Vector3d f = (interest - pos).getNormalized();
	Math::Vector3d u = up.getNormalized();
	Math::Vector3d s = Math::Vector3d::crossProduct(f, u).getNormalized();
	u = Math::Vector3d::crossProduct(s, f);

	Math::Matrix4 look;
	look(0,0) = s.x();
	look(1,0) = s.y();
	look(2,0) = s.z();
	look(0,1) = u.x();
	look(1,1) = u.y();
	look(2,1) = u.z();
	look(0,2) = -f.x();
	look(1,2) = -f.y();
	look(2,2) = -f.z();
	look(3,0) = -Math::Vector3d::dotProduct(s, pos);
	look(3,1) = -Math::Vector3d::dotProduct(u, pos);
	look(3,2) =  Math::Vector3d::dotProduct(f, pos);

	look.transpose();

	return look;
}

Math::Matrix4 GfxBase::makeProjMatrix(float fov, float nclip, float fclip) {
	float right = nclip * tan(fov / 2 * ((float)M_PI / 180));
	float left = -right;
	float top = right * 0.75;
	float bottom = -right * 0.75;

	Math::Matrix4 proj;
	proj(0,0) = (2.0f * nclip) / (right - left);
	proj(1,1) = (2.0f * nclip) / (top - bottom);
	proj(2,0) = (right + left) / (right - left);
	proj(2,1) = (top + bottom) / (top - bottom);
	proj(2,2) = -(fclip + nclip) / (fclip - nclip);
	proj(2,3) = -1.0f;
	proj(3,2) = -(2.0f * fclip * nclip) / (fclip - nclip);
	proj(3,3) = 0.0f;

	return proj;
}


void GfxBase::createSpecialtyTexture(uint id, const uint8 *data, int width, int height) {
	if (id >= _numSpecialtyTextures)
		return;
	if (_specialtyTextures[id]._texture) {
		destroyTexture(&_specialtyTextures[id]);
	}
	delete[] _specialtyTextures[id]._data;
	_specialtyTextures[id]._width = width;
	_specialtyTextures[id]._height = height;
	_specialtyTextures[id]._bpp = 4;
	_specialtyTextures[id]._colorFormat = BM_RGBA;
	createTexture(&_specialtyTextures[id], data, nullptr, true);
}

Bitmap *GfxBase::createScreenshotBitmap(const Graphics::PixelBuffer src, int w, int h, bool flipOrientation) {
        Graphics::PixelBuffer buffer = Graphics::PixelBuffer::createBuffer<565>(w * h, DisposeAfterUse::YES);

        int i1 = (_screenWidth * w - 1) / _screenWidth + 1;
        int j1 = (_screenHeight * h - 1) / _screenHeight + 1;

        for (int j = 0; j < j1; j++) {
                for (int i = 0; i < i1; i++) {
                        int x0 = i * _screenWidth / w;
                        int x1 = ((i + 1) * _screenWidth - 1) / w + 1;
                        int y0 = j * _screenHeight / h;
                        int y1 = ((j + 1) * _screenHeight - 1) / h + 1;
                        uint16 sr = 0, sg = 0, sb = 0;
                        for (int y = y0; y < y1; y++) {
                                for (int x = x0; x < x1; x++) {
                                        uint8 r, g, b;
                                        src.getRGBAt(y * _screenWidth + x, r, g, b);
                                        sr += r;
                                        sg += g;
                                        sb += b;
                                }
                        }
                        sr /= (x1 - x0) * (y1 - y0);
                        sg /= (x1 - x0) * (y1 - y0);
                        sb /= (x1 - x0) * (y1 - y0);
                        if (g_grim->getGameType() == GType_MONKEY4) {
                                buffer.setPixelAt( (flipOrientation ? j : (h - j - 1) ) * w + i, sr, sg, sb);
                        } else {
                                uint32 color = (sr + sg + sb) / 3;
                                buffer.setPixelAt( (flipOrientation ? j : (h - j - 1) ) * w + i, color, color, color);
                        }
                }
        }

        Bitmap *screenshot = new Bitmap(buffer, w, h, "screenshot");
        return screenshot;
}

void GfxBase::makeScreenTextures() {
	//make a buffer big enough to hold any of the textures
	uint8 *buffer = new uint8[256 * 256 * 4];

	// TODO: Handle screen resolutions other than 640 x 480
	createSpecialtyTextureFromScreen(0, buffer, 0, 0, 256, 256);
	createSpecialtyTextureFromScreen(1, buffer, 256, 0, 256, 256);
	createSpecialtyTextureFromScreen(2, buffer, 512, 0, 128, 128);
	createSpecialtyTextureFromScreen(3, buffer, 512, 128, 128, 128);
	createSpecialtyTextureFromScreen(4, buffer, 0, 256, 256, 256);
	createSpecialtyTextureFromScreen(5, buffer, 256, 256, 256, 256);
	createSpecialtyTextureFromScreen(6, buffer, 512, 256, 128, 128);
	createSpecialtyTextureFromScreen(7, buffer, 512, 384, 128, 128);

	delete[] buffer;
}

Texture *GfxBase::getSpecialtyTexturePtr(Common::String name) {
	assert(name.hasPrefix("specialty"));
	name.erase(0, 9);
	unsigned int id;
	sscanf(name.c_str(), "%u", &id);
	if (id >= _numSpecialtyTextures) {
		return nullptr;
	}
	return &_specialtyTextures[id];
}

}
