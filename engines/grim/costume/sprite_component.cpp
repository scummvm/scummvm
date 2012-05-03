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

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/debug.h"
#include "engines/grim/savegame.h"
#include "engines/grim/model.h"
#include "engines/grim/material.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume/mesh_component.h"
#include "engines/grim/costume/model_component.h"
#include "engines/grim/costume/sprite_component.h"

namespace Grim {

SpriteComponent::SpriteComponent(Component *p, int parentID, const char *filename, tag32 t) :
	Component(p, parentID, t), _filename(filename), _sprite(NULL) {

}

SpriteComponent::~SpriteComponent() {
	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			if (mc) {
				if (mc->getParent()->getTag() == MKTAG('M','M','D','L') ||
					mc->getParent()->getTag() == MKTAG('M','O','D','L')) {
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
	const char *comma = strchr(_filename.c_str(), ',');

	Common::String name(_filename.c_str(), comma);

	if (_sprite) {
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			mc->getNode()->removeSprite(_sprite);
		}
		delete _sprite;
		_sprite = NULL;
	}

	if (comma) {
		int width, height, x, y, z;
		sscanf(comma, ",%d,%d,%d,%d,%d", &width, &height, &x, &y, &z);

		_sprite = new Sprite;
		_sprite->_material = g_resourceloader->loadMaterial(name, getCMap());
		_sprite->_width = (float)width / 100.0f;
		_sprite->_height = (float)height / 100.0f;
		_sprite->_pos.set((float)x / 100.0f, (float)y / 100.0f, (float)z / 100.0f);
		_sprite->_visible = false;
		_sprite->_next = NULL;

		if (_parent) {
			if (_parent->getTag() == MKTAG('M','M','D','L') ||
				_parent->getTag() == MKTAG('M','O','D','L')) {
				MeshComponent *mc = static_cast<MeshComponent *>(_parent);
				mc->getNode()->addSprite(_sprite);
			} else
				Debug::warning(Debug::Costumes, "Parent of sprite %s wasn't a mesh", _filename.c_str());
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
