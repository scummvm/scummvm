/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "prince/graphics.h"

#include "prince/prince.h"

#include "graphics/palette.h"

#include "common/memstream.h"

namespace Prince {

GraphicsMan::GraphicsMan(PrinceEngine *vm) 
	: _vm(vm), _changed(false) {
	initGraphics(640, 480, true);
	_frontScreen = new Graphics::Surface();
	_frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_shadowTable70 = new byte[256 * 3];
	_shadowTable50 = new byte[256 * 3];
}

GraphicsMan::~GraphicsMan() {
	_frontScreen->free();
	delete _frontScreen;
	delete[] _shadowTable70;
	delete[] _shadowTable50;
}

void GraphicsMan::update() {
	if (_changed) {
		_vm->_system->copyRectToScreen((byte*)_frontScreen->getBasePtr(0,0), 640, 0, 0, 640, 480);

		_vm->_system->updateScreen();
		_changed = false;
	}
}

void GraphicsMan::setPalette(const byte *palette) {
	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void GraphicsMan::change() {
	_changed = true;
}

void GraphicsMan::draw(uint16 posX, uint16 posY, const Graphics::Surface *s)
{
	uint16 w = MIN(_frontScreen->w, s->w);
	for (uint y = 0; y < s->h; y++) {
		if (y < _frontScreen->h) {
		   memcpy((byte*)_frontScreen->getBasePtr(0, y), (byte*)s->getBasePtr(0, y), w);
		}
	}
	change();
}

void GraphicsMan::drawTransparent(uint16 posX, uint16 posY, const Graphics::Surface *s)
{
	for (uint y = 0; y < s->h; ++y) {
		for (uint x = 0; x < s->w; ++x) {
			byte pixel = *((byte*)s->getBasePtr(x, y));
			if (pixel != 255) {
				//*((byte*)_frontScreen->getBasePtr(x, y)) = pixel;
				*((byte*)_frontScreen->getBasePtr(x + posX, y + posY)) = pixel;
			}
		}
	}
   change();
}

void GraphicsMan::makeShadowTable(int brightness, byte *shadowPallete) {
	int shadow =  brightness * 256 / 100;
	byte *orginalPallete = new byte[256 * 3];
	_vm->_system->getPaletteManager()->grabPalette(orginalPallete, 0, 256);
	Common::MemoryReadStream readS(orginalPallete, 256 * 3);
	Common::MemoryWriteStream writeS(shadowPallete, 256 * 3);
	for(int i = 0; i < 256 * 3; i++) {
		writeS.writeByte(readS.readByte() * shadow / 256);
	}
	delete[] orginalPallete;
}

}

/* vim: set tabstop=4 noexpandtab: */
