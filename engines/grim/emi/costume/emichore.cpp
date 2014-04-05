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

#include "engines/grim/emi/costume/emichore.h"
#include "engines/grim/emi/modelemi.h"

namespace Grim {

EMIChore::EMIChore(char name[32], int id, Costume *owner, int length, int numTracks) :
		Chore(name, id, owner, length, numTracks), _mesh(NULL), _skeleton(NULL) {
}

void EMIChore::addComponent(Component *component) {
	if (component->isComponentType('m', 'e', 's', 'h')) {
		_mesh = static_cast<EMIMeshComponent *>(component);
	} else if (component->isComponentType('s', 'k', 'e', 'l')) {
		_skeleton = static_cast<EMISkelComponent *>(component);
	}
	if (_mesh && _mesh->_obj && _skeleton) {
		_mesh->_obj->setSkeleton(_skeleton->_obj);
	}
}

} // end of namespace Grim
