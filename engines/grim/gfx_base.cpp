/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/gfx_base.h"
#include "engines/grim/savegame.h"

namespace Grim {

GfxBase::GfxBase() :
	_renderBitmaps(true),
	_renderZBitmaps(true),
	_shadowModeActive(false),
	_currentPos(0,0,0),
	_currentQuat(0,0,0,1) {

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
	state->writeByte(r),
	state->writeByte(g),
	state->writeByte(b),
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
	setShadowColor(r, g ,b);
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

#ifndef USE_OPENGL
// Allow CreateGfxOpenGL to be called even if OpenGL isn't included
GfxBase *CreateGfxOpenGL() {
	return CreateGfxTinyGL();
}
#endif // USE_OPENGL

void SpecialtyMaterial::select() const {
	if (_texture) {
		g_driver->selectMaterial(_texture);
	}
}

void SpecialtyMaterial::create(const char *data, int width, int height) {
	delete _texture;
	_texture = new Texture();
	_texture->_width = width;
	_texture->_height = height;
	_texture->_bpp = 4;
	_texture->_colorFormat = BM_RGBA;
	g_driver->createMaterial(_texture, data, NULL);
}

}
