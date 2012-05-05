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

#include "common/foreach.h"

#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/animationemi.h"

namespace Grim {

EMIAnimComponent::EMIAnimComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, filename, t), _obj(NULL), _parentModel(NULL) {
}

EMIAnimComponent::~EMIAnimComponent() {
	delete _obj;
}

void EMIAnimComponent::init() {
	_visible = true;
	_obj = g_resourceloader->loadAnimationEmi(_name);
}

int EMIAnimComponent::update(uint time) {
	return 0;
}

void EMIAnimComponent::reset() {
	_visible = true;
}
	
void EMIAnimComponent::draw() {
	EMISkelComponent *skel = ((EMICostume*) _cost)->_emiSkel;
	if (skel && skel->_obj) {
		skel->_obj->setAnim(_obj);
	}
}

} // end of namespace Grim
