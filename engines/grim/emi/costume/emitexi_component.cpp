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

#include "engines/grim/costume.h"
#include "engines/grim/debug.h"
#include "engines/grim/material.h"
#include "engines/grim/savegame.h"

#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/costume/emitexi_component.h"

namespace Grim {

	EMITexiComponent::EMITexiComponent(Component *parent, int parentID, const char *filename, Component *prevComponent, tag32 tag) : Component(parent, parentID, filename, tag) {
	}

	EMITexiComponent::~EMITexiComponent() {
		if (_mat)
			_mat->dereference();
	}

	void EMITexiComponent::init() {
		EMICostume *c = static_cast<EMICostume *>(_cost);
		_mat = c->findSharedMaterial(_name);
	}

	int EMITexiComponent::update(uint time) {
		return 0;
	}

	void EMITexiComponent::setKey(int k) {
		if (_mat)
			_mat->setActiveTexture(k);
	}

	void EMITexiComponent::reset() {
	}

	void EMITexiComponent::draw() {
	}

} // end of namespace Grim
