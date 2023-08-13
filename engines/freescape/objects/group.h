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

#ifndef FREESCAPE_GROUP_H
#define FREESCAPE_GROUP_H

#include "freescape/gfx.h"
#include "freescape/objects/object.h"

namespace Freescape {

class Group : public Object {
public:
	Group(uint16 objectID_, uint16 flags_,
		const Common::Array<uint16> objectIds_,
		const Common::Array<uint16> objectOperations_,
		const Common::Array<Math::Vector3d> objectPositions_);
	void linkObject(Object *obj);
	void assemble(int index);
	void step();
	void run();
	void run(int index);

	Common::Array<Object *> _objects;
	Common::Array<Math::Vector3d> _origins;
	Common::Array<Math::Vector3d> _objectPositions;
	Common::Array<uint16> _objectOperations;
	Common::Array<uint16> _objectIds;
	int _scale;
	int _step;
	bool _active;
	bool _finished;

	ObjectType getType() override { return ObjectType::kGroupType; };
	bool isDrawable() override { return true; }
	void draw(Renderer *gfx) override;
	void scale(int scale_) override { _scale = scale_; };
	Object *duplicate() override { error("cannot duplicate Group"); };
};

} // End of namespace Freescape

#endif // FREESCAPE_GLOBAL_H
