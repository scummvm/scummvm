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

#include "engines/grim/colormap.h"
#include "engines/grim/model.h"
#include "engines/grim/costume/model_component.h"
#include "engines/grim/costume/main_model_component.h"

namespace Grim {

MainModelComponent::MainModelComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) :
		ModelComponent(p, parentID, filename, prevComponent, t), _hierShared(false), _parentModel(nullptr) {
	if (parentID == -2 && prevComponent && prevComponent->isComponentType('M','M','D','L')) {
		MainModelComponent *mmc = static_cast<MainModelComponent *>(prevComponent);

		if (mmc->_name == _name) {
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
		_obj = nullptr; // Keep ~ModelComp from deleting it
		_animation = nullptr;
	}

	for (MainModelComponent *child : _children) {
		child->_obj = nullptr;
		child->_hier = nullptr;
		child->_parentModel = nullptr;
	}

	if (_parentModel) {
		_parentModel->_children.remove(this);

		// The model goes back to the colormap of the costumes below
		if (_cmap)
			_parentModel->refreshColormap();
	}
}

void MainModelComponent::init() {
	ModelComponent::init();
	_visible = true;
	_hier->_hierVisible = _visible;
}

void MainModelComponent::refreshColormap() {
	Component::refreshColormap();
	if (_parentModel) {
		_parentModel->refreshColormap();
	}
}

CMap *MainModelComponent::getStackCMap() const {
	if (_cmap)
		return _cmap;
	for (MainModelComponent *child : _children) {
		CMap *cmap = child->getStackCMap();
		if (cmap)
			return cmap;
	}
	return nullptr;
}

void MainModelComponent::resetColormap() {
	const MainModelComponent *root = this;
	while (root->_parentModel)
		root = root->_parentModel;
	CMap *cmap = root->getStackCMap();
	if (!cmap)
		cmap = getCMap();
	if (_obj && cmap)
		_obj->reload(cmap);
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
