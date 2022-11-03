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

#include "engines/grim/emi/costume/emisprite_component.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"
#include "engines/grim/sprite.h"

namespace Grim {

EMISpriteComponent::EMISpriteComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, filename, t), _sprite(nullptr) {
}

EMISpriteComponent::~EMISpriteComponent() {
	delete _sprite;
}

void EMISpriteComponent::init() {
	EMICostume *c = static_cast<EMICostume *>(_cost);
	_sprite = g_resourceloader->loadSprite(_name, c);
}

int EMISpriteComponent::update(uint time) {
	return 0;
}

void EMISpriteComponent::reset() {
}

void EMISpriteComponent::draw() {
	if (_sprite) {
		_sprite->draw();
	}
}

} // end of namespace Grim
