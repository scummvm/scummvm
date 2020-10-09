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

#include "engines/grim/debug.h"
#include "engines/grim/savegame.h"
#include "engines/grim/model.h"
#include "engines/grim/sprite.h"
#include "engines/grim/material.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume/mesh_component.h"
#include "engines/grim/costume/model_component.h"
#include "engines/grim/costume/sprite_component.h"

namespace Grim {

SpriteComponent::SpriteComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, filename, t), _sprite(nullptr) {

}

SpriteComponent::~SpriteComponent() {
	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			if (mc) {
				if (mc->getParent()->isComponentType('M','M','D','L') ||
					mc->getParent()->isComponentType('M','O','D','L')) {
					ModelComponent *mdlc = static_cast<ModelComponent *>(mc->getParent());
					if (mdlc->getHierarchy())
						mc->getNode()->removeSprite(_sprite);
				}
			}
		}
		delete _sprite->_material;
		delete _sprite;
	}
}

void SpriteComponent::init() {
	const char *comma = strchr(_name.c_str(), ',');

	Common::String name(_name.c_str(), comma);

	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			mc->getNode()->removeSprite(_sprite);
		}
		delete _sprite;
		_sprite = nullptr;
	}

	if (comma) {
		_sprite = new Sprite();
		_sprite->loadGrim(name, comma, getCMap());

		if (_parent) {
			if (_parent->isComponentType('M','E','S','H')) {
				MeshComponent *mc = static_cast<MeshComponent *>(_parent);
				mc->getNode()->addSprite(_sprite);
			} else
				Debug::warning(Debug::Costumes, "Parent of sprite %s wasn't a mesh", _name.c_str());
		}
	}
}

void SpriteComponent::setKey(int val) {
	if (!_sprite)
		return;

	if (val == 0) {
		_sprite->_visible = false;
	} else {
		_sprite->_visible = true;
		_sprite->_material->setActiveTexture(val - 1);
	}
}

void SpriteComponent::reset() {
	if (_sprite)
		_sprite->_visible = false;
}

void SpriteComponent::saveState(SaveGame *state) {
	state->writeBool(_sprite->_visible);
	state->writeLESint32(_sprite->_material->getActiveTexture());
}

void SpriteComponent::restoreState(SaveGame *state) {
	_sprite->_visible = state->readBool();
	_sprite->_material->setActiveTexture(state->readLESint32());
}

} // end of namespace Grim
