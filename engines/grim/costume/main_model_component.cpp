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

#include "engines/grim/model.h"
#include "engines/grim/costume/model_component.h"
#include "engines/grim/costume/main_model_component.h"

namespace Grim {

MainModelComponent::MainModelComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) :
		ModelComponent(p, parentID, filename, prevComponent, t), _hierShared(false), _parentModel(NULL) {
	if (parentID == -2 && prevComponent && prevComponent->isComponentType('M','M','D','L')) {
		MainModelComponent *mmc = static_cast<MainModelComponent *>(prevComponent);

		if (mmc->_filename == filename) {
			_animation = mmc->_animation;
			_obj = mmc->_obj;
			_hier = mmc->_hier;
			_hierShared = true;
			mmc->_children.push_back(this);
			_parentModel = mmc;
		}
	}
}

MainModelComponent::~MainModelComponent() {
	if (_hierShared) {
		_obj = NULL; // Keep ~ModelComp from deleting it
		_animation = NULL;
	}

	foreach (MainModelComponent *child, _children) {
		child->_obj = NULL;
		child->_hier = NULL;
		child->_parentModel = NULL;
	}

	if (_parentModel) {
		_parentModel->_children.remove(this);
	}
}

void MainModelComponent::init() {
	ModelComponent::init();
	_visible = true;
	_hier->_hierVisible = _visible;
}

int MainModelComponent::update(uint time) {
	if (!_hierShared)
		// Otherwise, it was already initialized
		// and reinitializing it will destroy work
		// from previous costumes
		ModelComponent::update(time);

	return 0;
}

void MainModelComponent::setColormap(CMap *cmap) {
	Component::setColormap(cmap);
	if (_parentModel) {
		_parentModel->setColormap(cmap);
	}
}

void MainModelComponent::reset() {
	_visible = true;
	// Can be NULL if this was attached to another costume which
	// was deleted.
	if (_hier) {
		_hier->_hierVisible = _visible;
	}
}

} // end of namespace Grim
