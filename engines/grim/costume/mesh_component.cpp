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

#include "common/textconsole.h"

#include "engines/grim/debug.h"
#include "engines/grim/model.h"
#include "engines/grim/savegame.h"
#include "engines/grim/costume/mesh_component.h"
#include "engines/grim/costume/model_component.h"

namespace Grim {

MeshComponent::MeshComponent(Component *p, int parentID, const char *name, tag32 t) :
		Component(p, parentID, name, t), _node(nullptr) {
	if (sscanf(name, "mesh %d", &_num) < 1)
		error("Couldn't parse mesh name %s", name);

}

void MeshComponent::init() {
	if (_parent->isComponentType('M','M','D','L') ||
		_parent->isComponentType('M','O','D','L')) {
		ModelComponent *mc = static_cast<ModelComponent *>(_parent);
		_node = mc->getHierarchy() + _num;
		_model = mc->getModel();
	} else {
		Debug::warning(Debug::Costumes, "Parent of mesh %d was not a model", _num);
		_node = nullptr;
		_model = nullptr;
	}
}

CMap *MeshComponent::cmap() {
	if (_parent->isComponentType('M','M','D','L') ||
		_parent->isComponentType('M','O','D','L')) {
		ModelComponent *mc = static_cast<ModelComponent *>(_parent);
		return mc->getCMap();
	}
	return nullptr;
}

void MeshComponent::setKey(int val) {
	_node->_meshVisible = (val != 0);
}

void MeshComponent::reset() {
	// NOTE: Setting the visibility to true here causes a bug with the thunderboy costume:
	// closing the inventory causes the hat to appear, while it shouldn't.
	// This could however introduce regressions somewhere else, so if there is something
	// disappearing or not behaving properly in a costume the cause might be here.
// 	_node->_meshVisible = true;
}

int MeshComponent::update(uint /*time*/) {
	_node->setMatrix(_matrix);
	return 0;
}

void MeshComponent::saveState(SaveGame *state) {
	state->writeBool(_node->_meshVisible);
	state->writeVector3d(_matrix.getPosition());
}

void MeshComponent::restoreState(SaveGame *state) {
	_node->_meshVisible = state->readBool();
	if (state->saveMinorVersion() >= 14) {
		_matrix.setPosition(state->readVector3d());
		_node->setMatrix(_matrix);
	}
}

} // end of namespace Grim
