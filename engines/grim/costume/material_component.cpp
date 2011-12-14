/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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
#include "engines/grim/model.h"
#include "engines/grim/material.h"
#include "engines/grim/savegame.h"

#include "engines/grim/costume/material_component.h"
#include "engines/grim/costume/model_component.h"

namespace Grim {


MaterialComponent::MaterialComponent(Component *p, int parentID, const char *filename, tag32 t) :
		Component(p, parentID, t), _filename(filename) {

	Debug::debug(Debug::Costumes, "Constructing MaterialComponent %s", filename);
}

void MaterialComponent::init() {
	_mat = NULL;
	if (FROM_BE_32(_parent->getTag()) == MKTAG('M','M','D','L') ||
		FROM_BE_32(_parent->getTag()) == MKTAG('M','O','D','L')) {
		ModelComponent *p = static_cast<ModelComponent *>(_parent);
		Model *model = p->getModel();
		if (model) {
			for (int i = 0; i < model->_numMaterials; ++i) {
				if (_filename.compareToIgnoreCase(model->_materials[i]->getFilename()) == 0) {
					_mat = model->_materials[i];
					return;
				}
			}
		}
	} else {
		warning("Parent of a MaterialComponent not a ModelComponent. %s %s", _filename.c_str(), _cost->getFilename().c_str());
	}
}

void MaterialComponent::setKey(int val) {
	_mat->setActiveTexture(val);
}

void MaterialComponent::reset() {
	_mat->setActiveTexture(0);
}

void MaterialComponent::resetColormap() {
	init();
}

void MaterialComponent::saveState(SaveGame *state) {
	state->writeLESint32(_mat->getActiveTexture());
}

void MaterialComponent::restoreState(SaveGame *state) {
	_mat->setActiveTexture(state->readLESint32());
}

} // end of namespace Grim
