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

#ifndef TWINE_PARSER_BODYTYPES_H
#define TWINE_PARSER_BODYTYPES_H

#include "common/array.h"
#include "twine/shared.h"
#include "twine/parser/anim.h"

namespace TwinE {

struct BodyVertex {
	int16 x;
	int16 y;
	int16 z;
	uint16 bone;
};

struct BodyLine {
	// fill byte here
	uint8 color;
	// 2 fill bytes here
	uint16 vertex1;
	uint16 vertex2;
};

struct BodySphere {
	uint8 fillType;
	uint16 color; // start and end color index
	// fill byte here
	uint16 radius;
	uint16 vertex;
};

struct BodyBone {
	uint16 parent;
	uint16 vertex;
	int16 firstVertex;
	int16 numVertices;
	int32 numOfShades;
	BoneFrame initalBoneState;

	inline bool isRoot() const {
		return parent == 0xffff;
	}
};

struct BodyShade {
	int16 col1;
	int16 col2;
	int16 col3;
	uint16 unk4;
};

struct BodyPolygon {
	Common::Array<uint16> indices;
	Common::Array<uint16> intensities;
	int8 materialType = 0;
	int16 color = 0;
};

}

#endif
