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

#include "neverhood/resource.h"
#include "neverhood/resourceman.h"

namespace Neverhood {

SpriteResource::SpriteResource(NeverhoodEngine *vm)
	 : _vm(vm), _resourceHandle(-1), _pixels(NULL) {
}

SpriteResource::~SpriteResource() {
	unload();
}

void SpriteResource::draw(byte *dest, int destPitch, bool flipX, bool flipY) {
	if (_pixels) {
		if (_rle) {
			unpackSpriteRle(_pixels, _dimensions.width, _dimensions.height, dest, destPitch, flipX, flipY);
		} else {
			unpackSpriteNormal(_pixels, _dimensions.width, _dimensions.height, dest, destPitch, flipX, flipY);
		}
	}
}

bool SpriteResource::load(uint32 fileHash) {
	// TODO: Later merge with load2 and make the mode a parameter
	unload();
	_resourceHandle = _vm->_res->useResource(fileHash);
	debug("SpriteResource::load(0x%08X) _resourceHandle = %d", fileHash, _resourceHandle);
	if (_resourceHandle != -1) {
		if (_vm->_res->getResourceType(_resourceHandle) == 2) {
			byte *spriteData = _vm->_res->loadResource(_resourceHandle, true);
			parseBitmapResource(spriteData, &_rle, &_dimensions, NULL, NULL, &_pixels);
		} else {
			_vm->_res->unuseResource(_resourceHandle);
			_resourceHandle = -1;
		}
	} 
	return _pixels != NULL;
}

bool SpriteResource::load2(uint32 fileHash) {
	unload();
	_resourceHandle = _vm->_res->useResource(fileHash);
	if (_resourceHandle != -1) {
		if (_vm->_res->getResourceType(_resourceHandle) == 2) {
			byte *spriteData = _vm->_res->loadResource(_resourceHandle, true);
			parseBitmapResource(spriteData, &_rle, &_dimensions, &_position, NULL, &_pixels);
		} else {
			_vm->_res->unuseResource(_resourceHandle);
			_resourceHandle = -1;
		}
	} 
	return _pixels != NULL;
}

void SpriteResource::unload() {
	if (_resourceHandle != -1) {
		_vm->_res->unloadResource(_resourceHandle);
		_vm->_res->unuseResource(_resourceHandle);
		_resourceHandle = -1;
	} else {
		delete[] _pixels;
	}
	_pixels = NULL;
	_rle = false;
}

PaletteResource::PaletteResource(NeverhoodEngine *vm)
	: _vm(vm), _resourceHandle(-1), _palette(NULL) {
}

PaletteResource::~PaletteResource() {
	unload();
}

bool PaletteResource::load(uint32 fileHash) {
	unload();
	_resourceHandle = _vm->_res->useResource(fileHash);
	if (_resourceHandle != -1) {
		_palette = _vm->_res->loadResource(_resourceHandle, true);
		switch (_vm->_res->getResourceType(_resourceHandle)) {
		case 2:
			// Palette is stored in a bitmap
			parseBitmapResource(_palette, NULL, NULL, NULL, &_palette, NULL);
			break;
		case 3:
			// _palette already points to the correct data
			break;
		default:						
			_vm->_res->unuseResource(_resourceHandle);
			_resourceHandle = -1;
			break;
		}
	} 
	return _palette != NULL;
}

void PaletteResource::unload() {
	if (_resourceHandle != -1) {
		_vm->_res->unloadResource(_resourceHandle);
		_vm->_res->unuseResource(_resourceHandle);
		_resourceHandle = -1;
		_palette = NULL;
	}
}

void PaletteResource::copyPalette(byte *destPalette) {
	if (_palette) {
		memcpy(destPalette, _palette, 1024);
	} else {
		// TODO?: buildDefaultPalette(palette);
	}
}

} // End of namespace Neverhood
