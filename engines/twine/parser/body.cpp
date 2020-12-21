/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "twine/parser/body.h"
#include "twine/renderer/renderer.h"
#include "common/memstream.h"

namespace TwinE {

void BodyData::loadVertices(Common::SeekableReadStream &stream) {
	const uint16 numVertices = stream.readUint16LE();
	_vertices.clear();
	_vertices.reserve(numVertices);
	for (uint16 i = 0U; i < numVertices; ++i) {
		const int16 x = stream.readSint16LE();
		const int16 y = stream.readSint16LE();
		const int16 z = stream.readSint16LE();
		const uint16 bone = 0;
		_vertices.push_back({x, y, z, bone});
	}
}

void BodyData::loadBones(Common::SeekableReadStream &stream) {
	const uint16 numBones = stream.readUint16LE();
	_bones.clear();
	_bones.reserve(numBones);
	for (uint16 i = 0; i < numBones; ++i) {
		const int16 firstPoint = stream.readSint16LE() / 6;
		const int16 numPoints = stream.readSint16LE();
		const int16 basePoint = stream.readSint16LE() / 6;
		const int16 baseElementOffset = stream.readSint16LE();
		BoneFrame boneframe;
		boneframe.type = stream.readSint16LE();
		boneframe.x = stream.readSint16LE();
		boneframe.y = stream.readSint16LE();
		boneframe.z = stream.readSint16LE();
		const int32 numOfShades = stream.readSint32LE();
		/*int32 field_14 =*/ stream.readSint32LE();
		/*int32 field_18 =*/ stream.readSint32LE();
		/*int32 y =*/ stream.readSint32LE();
		/*int32 field_20 =*/ stream.readSint32LE();
		/*int16 field_24 =*/ stream.readSint16LE();

		BodyBone bone;
		bone.parent = baseElementOffset == -1 ? 0xffff : baseElementOffset / 38;
		bone.vertex = basePoint;
		bone.initalBoneState = boneframe;
		bone.numOfShades = numOfShades;

		// assign the bone index to the vertices
		for (int j = 0; j < numPoints; ++j) {
			_vertices[firstPoint + j].bone = i;
		}

		_bones.push_back(bone);
	}
}

void BodyData::loadShades(Common::SeekableReadStream &stream) {
	const uint16 numShades = stream.readUint16LE();
	_shades.clear();
	_shades.reserve(numShades);
	for (uint16 i = 0; i < numShades; ++i) {
		BodyShade shape;
		shape.col1 = stream.readSint16LE();
		shape.col2 = stream.readSint16LE();
		shape.col3 = stream.readSint16LE();
		shape.unk4 = stream.readSint16LE();
		_shades.push_back(shape);
	}
}

void BodyData::loadPolygons(Common::SeekableReadStream &stream) {
	const uint16 numPolygons = stream.readUint16LE();
	_polygons.clear();
	_polygons.reserve(numPolygons);
	for (uint16 i = 0; i < numPolygons; ++i) {
		BodyPolygon poly;
		poly.renderType = stream.readSByte();
		const int8 numVertex = stream.readSByte();

		poly.color = stream.readUint16LE();
		int16 intensity = -1;
		if (poly.renderType >= 7 && poly.renderType < 9) {
			intensity = stream.readSint16LE();
		}

		poly.indices.reserve(numVertex);
		poly.intensities.reserve(numVertex);
		for (int k = 0; k < numVertex; ++k) {
			if (poly.renderType >= 9) {
				intensity = stream.readSint16LE();
			}
			const uint16 vertexIndex = stream.readUint16LE() / 6;
			poly.indices.push_back(vertexIndex);
			poly.intensities.push_back(intensity);
		}

		_polygons.push_back(poly);
	}
}

void BodyData::loadLines(Common::SeekableReadStream &stream) {
	const uint16 numLines = stream.readUint16LE();
	_lines.clear();
	_lines.reserve(numLines);
	for (uint16 i = 0; i < numLines; ++i) {
		BodyLine line;
		line.unk1 = stream.readUint16LE();
		line.color = stream.readUint16LE();
		line.vertex1 = stream.readUint16LE() / 6;
		line.vertex2 = stream.readUint16LE() / 6;
		_lines.push_back(line);
	}
}

void BodyData::loadSpheres(Common::SeekableReadStream &stream) {
	const uint16 numSpheres = stream.readUint16LE();
	_spheres.clear();
	_spheres.reserve(numSpheres);
	for (uint16 i = 0; i < numSpheres; ++i) {
		BodySphere sphere;
		sphere.radius = stream.readUint16LE();
		sphere.color = stream.readUint16LE();
		sphere.size = stream.readUint16LE();
		sphere.vertex = stream.readUint16LE() / 6;
		_spheres.push_back(sphere);
	}
}

bool BodyData::loadFromStream(Common::SeekableReadStream &stream) {
	*(uint16 *)&bodyFlag = stream.readUint16LE();
	minsx = stream.readSint16LE();
	maxsx = stream.readSint16LE();
	minsy = stream.readSint16LE();
	maxsy = stream.readSint16LE();
	minsz = stream.readSint16LE();
	maxsz = stream.readSint16LE();

	stream.seek(0x1A);
	loadVertices(stream);
	loadBones(stream);
	loadShades(stream);
	loadPolygons(stream);
	loadLines(stream);
	loadSpheres(stream);
	return !stream.err();
}

} // namespace TwinE
