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

#include "engines/grim/emi/costume/emimesh_component.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"

namespace Grim {

EMIMeshComponent::EMIMeshComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, t), _filename(filename), _obj(NULL), _parentModel(NULL), _hasComplained(false) {
	_hierShared = false;
}

EMIMeshComponent::~EMIMeshComponent() {
	if (_hierShared) {
		_obj = NULL; // Keep ~ModelComp from deleting it
		 //_animation = NULL;
	} else {
		delete _obj;
	}

	foreach (EMIMeshComponent *child, _children) {
		child->_obj = NULL;
		//child->_hier = NULL;
		child->_parentModel = NULL;
	}

	if (_parentModel) {
		_parentModel->_children.remove(this);
	}
}

void EMIMeshComponent::init() {
	_visible = true;
	_obj = g_resourceloader->loadModelEMI(_filename);
}

int EMIMeshComponent::update(uint time) {
	return 0;
}

void EMIMeshComponent::reset() {
	_visible = true;
}
	
void EMIMeshComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it
	if (_parent && _parent->isVisible())
		return;
	if (!_obj) {
		if (!_hasComplained) {
			warning("Tried to draw component we have no file for %s", _filename.c_str());
			_hasComplained = true;
		}
		return;
	}
	// Need to translate object to be in accordance
	// with the setup of the parent
	//translateObject(false);
	
	_obj->draw();
	
	// Need to un-translate when done
	//translateObject(true);
}


} // end of namespace Grim
