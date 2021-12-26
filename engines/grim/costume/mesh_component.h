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
	void init() override;
	CMap *cmap();
	void setKey(int val) override;
	int update(uint time) override;
	void reset() override;
	void saveState(SaveGame *state) override;
	void restoreState(SaveGame *state) override;

	void setMatrix(const Math::Matrix4 &matrix) override { _matrix = matrix; };

	ModelNode *getNode() { return _node; }
	Model *getModel() { return _model; }

private:
	int _num;
	Model *_model;
	ModelNode *_node;
	Math::Matrix4 _matrix;
};

} // end of namespace Grim

#endif
