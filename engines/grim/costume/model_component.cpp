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
#include "engines/grim/model.h"
#include "engines/grim/resource.h"
#include "engines/grim/grim.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/colormap.h"
#include "engines/grim/animation.h"

#include "engines/grim/costume/model_component.h"
#include "engines/grim/costume/main_model_component.h"
#include "engines/grim/costume/mesh_component.h"

namespace Grim {

#define DEFAULT_COLORMAP "item.cmp"

ModelComponent::ModelComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) :
		Component(p, parentID, filename, t),
		_obj(nullptr), _hier(nullptr), _animation(nullptr), _animated(false) {
	const char *comma = strchr(filename, ',');

	// Can be called with a comma and a numeric parameter afterward, but
	// the use for this parameter is currently unknown
	// Example: At the "scrimshaw parlor" in Rubacava the object
	// "manny_cafe.3do,1" is requested
	if (comma) {
		_name = Common::String(filename, comma);
		warning("Comma in model components not supported: %s", filename);
	}
	_prevComp = prevComponent;
}

ModelComponent::~ModelComponent() {
	if (_hier && _hier->_parent) {
		_hier->_parent->removeChild(_hier);
	}

	delete _obj;
	delete _animation;
}

void ModelComponent::init() {
	if (_prevComp && _prevComp->isComponentType('M','M','D','L')) {
		_previousCmap = _prevComp->getCMap();
	}
	// Skip loading if it was initialized
	// by the sharing MainModelComponent
	// constructor before
	if (!_obj) {
		CMapPtr cm = getCMap();

		// Get the default colormap if we haven't found
		// a valid colormap
		if (!cm && g_grim->getCurrSet())
			cm = g_grim->getCurrSet()->getCMap();
		if (!cm) {
			Debug::warning(Debug::Costumes, "No colormap specified for %s, using %s", _name.c_str(), DEFAULT_COLORMAP);

			cm = g_resourceloader->getColormap(DEFAULT_COLORMAP);
		}

		// If we're the child of a mesh component, put our nodes in the
		// parent object's tree.
		if (_parent) {
			MeshComponent *mc = static_cast<MeshComponent *>(_parent);
			_obj = g_resourceloader->loadModel(_name, cm, mc->getModel());
			_hier = _obj->getHierarchy();
			mc->getNode()->addChild(_hier);
		} else {
			_obj = g_resourceloader->loadModel(_name, cm);
			_hier = _obj->getHierarchy();
			Debug::warning(Debug::Costumes, "Parent of model %s wasn't a mesh", _name.c_str());
		}

		// Use parent availablity to decide whether to default the
		// component to being visible
		if (_parent)
			setKey(0);
		else
			setKey(1);
	}

	if (!_animation) {
		_animation = new AnimManager();
	}
}

void ModelComponent::setKey(int val) {
	_visible = (val != 0);
	_hier->_hierVisible = _visible;
}

void ModelComponent::reset() {
	_visible = false;
	_hier->_hierVisible = _visible;
}

AnimManager *ModelComponent::getAnimManager() const {
	return _animation;
}

int ModelComponent::update(uint time) {
	// First reset the current animation.
	for (int i = 0; i < getNumNodes(); i++) {
		_hier[i]._animPos = _hier[i]._pos;
		_hier[i]._animRot = _hier[i]._rot;
	}

	_animated = false;
	return 0;
}

void ModelComponent::animate() {
	if (_animated) {
		return;
	}

	_animation->animate(_hier, getNumNodes());
	_animated = true;
}

void ModelComponent::resetColormap() {
	CMap *cm;

	cm = getCMap();
	if (_obj && cm)
		_obj->reload(cm);
}

void ModelComponent::restoreState(SaveGame *state) {
	_hier->_hierVisible = _visible;
}

int ModelComponent::getNumNodes() {
	return _obj->getNumNodes();
}

void ModelComponent::translateObject(ModelNode *node, bool reset) {
	if (node->_parent)
		translateObject(node->_parent, reset);

	if (reset) {
		node->translateViewpointFinish();
	} else {
		node->translateViewpointStart();
		node->translateViewpoint();
	}
}

void ModelComponent::translateObject(bool res) {
	ModelNode *node = _hier->_parent;
	if (node) {
		translateObject(node, res);
	}
}

void ModelComponent::draw() {
	// If the object was drawn by being a component
	// of it's parent then don't draw it

	if (_parent && _parent->isVisible())
		return;
	// Need to translate object to be in accordance
	// with the setup of the parent
	translateObject(false);

	_hier->draw();

	// Need to un-translate when done
	translateObject(true);
}

void ModelComponent::getBoundingBox(int *x1, int *y1, int *x2, int *y2) {
	// If the object was drawn by being a component
	// of it's parent then don't draw it

	if (_parent && _parent->isVisible())
		return;
	// Need to translate object to be in accordance
	// with the setup of the parent
	translateObject(false);

	_hier->getBoundingBox(x1, y1, x2, y2);

	// Need to un-translate when done
	translateObject(true);
}

} // end of namespace Grim
