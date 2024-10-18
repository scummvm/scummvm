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

#include "twine/parser/body.h"
#include "twine/renderer/renderer.h"
#include "common/memstream.h"

namespace TwinE {

void BodyData::reset() {
	_vertices.clear();
	_bones.clear();
	_normals.clear();
	_polygons.clear();
	_spheres.clear();
	_lines.clear();
}

void BodyData::loadVertices(Common::SeekableReadStream &stream) {
	const uint16 numVertices = stream.readUint16LE();
	if (stream.eos())
		return;

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
	if (stream.eos())
		return;

	_bones.reserve(numBones);
	for (uint16 i = 0; i < numBones; ++i) {
		const int16 firstPoint = stream.readSint16LE() / 6;
		const int16 numPoints = stream.readSint16LE();
		const int16 basePoint = stream.readSint16LE() / 6;
		const int16 baseElementOffset = stream.readSint16LE();
		BoneFrame boneframe;
		boneframe.type = (BoneType)stream.readSint16LE();
		boneframe.x = stream.readSint16LE();
		boneframe.y = stream.readSint16LE();
		boneframe.z = stream.readSint16LE();
		/*int16 unk1 =*/ stream.readSint16LE();
		const int16 numNormals = stream.readSint16LE();
		/*int16 unk2 =*/ stream.readSint16LE();
		/*int32 field_18 =*/ stream.readSint32LE();
		/*int32 y =*/ stream.readSint32LE();
		/*int32 field_20 =*/ stream.readSint32LE();
		/*int32 field_24 =*/ stream.readSint32LE();

		// PatchObjet in original sources
		BodyBone bone;
		bone.parent = baseElementOffset == -1 ? 0xffff : baseElementOffset / 38;
		bone.vertex = basePoint;
		bone.firstVertex = firstPoint;
		bone.numVertices = numPoints;
		bone.initalBoneState = boneframe;
		bone.numNormals = numNormals;

		// assign the bone index to the vertices
		for (int j = 0; j < numPoints; ++j) {
			_vertices[firstPoint + j].bone = i;
		}

		_bones.push_back(bone);
		_boneStates[i] = bone.initalBoneState;
	}
}

void BodyData::loadNormals(Common::SeekableReadStream &stream) {
	const uint16 numNormals = stream.readUint16LE();
	if (stream.eos())
		return;

	_normals.reserve(numNormals);
	for (uint16 i = 0; i < numNormals; ++i) {
		BodyNormal shape;
		shape.x = stream.readSint16LE();
		shape.y = stream.readSint16LE();
		shape.z = stream.readSint16LE();
		shape.prenormalizedRange = stream.readUint16LE();
		_normals.push_back(shape);
	}
}

void BodyData::loadPolygons(Common::SeekableReadStream &stream) {
	const uint16 numPolygons = stream.readUint16LE();
	if (stream.eos())
		return;

	_polygons.reserve(numPolygons);
	for (uint16 i = 0; i < numPolygons; ++i) {
		BodyPolygon poly;
		poly.materialType = stream.readByte();
		const uint8 numVertices = stream.readByte();

		poly.intensity = stream.readSint16LE();
		int16 normal = -1;
		if (poly.materialType == MAT_FLAT || poly.materialType == MAT_GRANIT) {
			// only one shade value is used
			normal = stream.readSint16LE();
		}

		poly.indices.reserve(numVertices);
		poly.normals.reserve(numVertices);
		for (int k = 0; k < numVertices; ++k) {
			if (poly.materialType >= MAT_GOURAUD) {
				normal = stream.readSint16LE();
			}
			// numPoint is point index precomupted * 6
			const uint16 vertexIndex = stream.readUint16LE() / 6;
			poly.indices.push_back(vertexIndex);
			poly.normals.push_back(normal);
		}

		_polygons.push_back(poly);
	}
}

void BodyData::loadLines(Common::SeekableReadStream &stream) {
	const uint16 numLines = stream.readUint16LE();
	if (stream.eos())
		return;

	_lines.reserve(numLines);
	for (uint16 i = 0; i < numLines; ++i) {
		BodyLine line;
		stream.skip(1);
		line.color = stream.readByte();
		stream.skip(2);
		// indexPoint is point index precomupted * 6
		line.vertex1 = stream.readUint16LE() / 6;
		line.vertex2 = stream.readUint16LE() / 6;
		_lines.push_back(line);
	}
}

void BodyData::loadSpheres(Common::SeekableReadStream &stream) {
	const uint16 numSpheres = stream.readUint16LE();
	if (stream.eos())
		return;

	_spheres.reserve(numSpheres);
	for (uint16 i = 0; i < numSpheres; ++i) {
		BodySphere sphere;
		sphere.fillType = stream.readByte();
		sphere.color = stream.readUint16LE();
		stream.readByte();
		sphere.radius = stream.readUint16LE();
		sphere.vertex = stream.readUint16LE() / 6;
		_spheres.push_back(sphere);
	}
}

bool BodyData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	reset();
	if (lba1) {
		const uint16 flags = stream.readUint16LE();
		animated = (flags & 2) != 0;
		bbox.mins.x = stream.readSint16LE();
		bbox.maxs.x = stream.readSint16LE();
		bbox.mins.y = stream.readSint16LE();
		bbox.maxs.y = stream.readSint16LE();
		bbox.mins.z = stream.readSint16LE();
		bbox.maxs.z = stream.readSint16LE();
		offsetToData = stream.readSint16LE();

		// using this value as the offset crashes the demo of lba1 - see https://bugs.scummvm.org/ticket/14294
		// stream.seek(offsetToData);
		stream.seek(0x1A);

		loadVertices(stream);
		loadBones(stream);
		loadNormals(stream);
		loadPolygons(stream);
		loadLines(stream);
		loadSpheres(stream);
	} else {
		// T_BODY_HEADER (lba2)
		const uint32 flags = stream.readUint32LE();
		animated = (flags & 2) != 0;
		stream.skip(4); // int16 size of header and int16 dummy
		bbox.mins.x = stream.readSint32LE();
		bbox.maxs.x = stream.readSint32LE();
		bbox.mins.y = stream.readSint32LE();
		bbox.maxs.y = stream.readSint32LE();
		bbox.mins.z = stream.readSint32LE();
		bbox.maxs.z = stream.readSint32LE();
		stream.seek(0x20);
#if 0
		const uint32 bonesSize = stream.readUint32LE();
		const uint32 bonesOffset = stream.readUint32LE();
		const uint32 verticesSize = stream.readUint32LE();
		const uint32 verticesOffset = stream.readUint32LE();
		const uint32 normalsSize = stream.readUint32LE();
		const uint32 normalsOffset = stream.readUint32LE();
		const uint32 unk1Size = stream.readUint32LE();
		const uint32 unk1Offset = stream.readUint32LE();
		const uint32 polygonsSize = stream.readUint32LE();
		const uint32 polygonsOffset = stream.readUint32LE();
		const uint32 linesSize = stream.readUint32LE();
		const uint32 linesOffset = stream.readUint32LE();
		const uint32 spheresSize = stream.readUint32LE();
		const uint32 spheresOffset = stream.readUint32LE();
		const uint32 uvGroupsSize = stream.readUint32LE();
		const uint32 uvGroupsOffset = stream.readUint32LE();
#endif
	}

	return !stream.err();
}

} // namespace TwinE
