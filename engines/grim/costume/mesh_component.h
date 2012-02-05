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

#ifndef GRIM_MESH_COMPONENT_H
#define GRIM_MESH_COMPONENT_H

#include "engines/grim/costume/component.h"

namespace Grim {

class CMap;
class Model;
class ModelNode;

class MeshComponent : public Component {
public:
	MeshComponent(Component *parent, int parentID, const char *name, tag32 tag);
	void init();
	CMap *cmap();
	void setKey(int val);
	int update(uint time);
	void reset();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);

	void setMatrix(const Math::Matrix4 &matrix) { _matrix = matrix; };

	ModelNode *getNode() { return _node; }
	Model *getModel() { return _model; }

private:
	Common::String _name;
	int _num;
	Model *_model;
	ModelNode *_node;
	Math::Matrix4 _matrix;
};

} // end of namespace Grim

#endif
