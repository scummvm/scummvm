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

#include "neverhood/background.h"

namespace Neverhood {

// Background

Background::Background(NeverhoodEngine *vm, int objectPriority)
	: Entity(vm, objectPriority), _surface(NULL), _spriteResource(vm) {
	// Empty
}

Background::Background(NeverhoodEngine *vm, uint32 fileHash, int objectPriority, int surfacePriority)
	: Entity(vm, objectPriority), _surface(NULL), _spriteResource(vm) {

	_spriteResource.load(fileHash);
	createSurface(surfacePriority, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_surface->drawSpriteResource(_spriteResource);

}

Background::~Background() {
	delete _surface;
}

void Background::createSurface(int surfacePriority, int16 width, int16 height) {
	_surface = new BaseSurface(_vm, surfacePriority, width, height, "background");
	_surface->setTransparent(false);
	_spriteResource.getPosition().x = width;
	_spriteResource.getPosition().y = height;
}

void Background::load(uint32 fileHash) {
	_spriteResource.load(fileHash);
	if (_surface)
		_surface->drawSpriteResource(_spriteResource);
}

} // End of namespace Neverhood
