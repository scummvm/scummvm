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

#include "engines/grim/debug.h"
#include "engines/grim/emi/costume/emianim_component.h"
#include "engines/grim/emi/costume/emiskel_component.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/animationemi.h"

namespace Grim {

EMIAnimComponent::EMIAnimComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, filename, t), _obj(NULL), _parentModel(NULL), _hierShared(false), _looping(false), _active(false) {
}

EMIAnimComponent::~EMIAnimComponent() {
	delete _obj;
}

void EMIAnimComponent::init() {
	_visible = true;
	_obj = g_resourceloader->loadAnimationEmi(_name);
}

int EMIAnimComponent::update(uint time) {
	if (_obj && _active) {
		EMISkelComponent *skel = ((EMICostume *) _cost)->_emiSkel;
		_active = _obj->animate(skel->_obj, time, _looping);
	}
	return 0;
}

void EMIAnimComponent::setKey(int f) {
	switch (f) {
	case 1: // Play?
		// We had it so that if f == 1 it would always reset the animation, but that caused
		// issues with idle animations resetting too early. We now reset the animation
		// only if this is not a looping animation. Similar behavior exists in Grim.
		if (!_looping)
			_obj->reset();
		_active = true;
		break;
	case 3: // Looping?
		_looping = true;
		break;
	default:
		Debug::warning(Debug::Costumes, "Unknown key %d for component %s", f, _name.c_str());
		break;
	}
}

void EMIAnimComponent::reset() {
	_visible = true;
	_looping = false;
	_active = false;
	if (_obj)
		_obj->reset();
}

void EMIAnimComponent::draw() {
}

} // end of namespace Grim
