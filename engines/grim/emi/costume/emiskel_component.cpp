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

#include "common/foreach.h"

#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/resource.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/costume.h"

namespace Grim {

EMISkelComponent::EMISkelComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, filename, t), _obj(nullptr), _parentModel(nullptr), _hierShared(false) {
}

EMISkelComponent::~EMISkelComponent() {
	delete _obj;
}

void EMISkelComponent::init() {
	_visible = true;
	_obj = g_resourceloader->loadSkeleton(_name);
}

void EMISkelComponent::animate() {
	if (_obj)
		_obj->animate();
}

int EMISkelComponent::update(uint time) {
	return 0;
}

void EMISkelComponent::reset() {
	_visible = true;
}

void EMISkelComponent::draw() {
}

} // end of namespace Grim
