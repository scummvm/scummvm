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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "engines/wintermute/base/gfx/3ds/loader3ds.h"
#include "engines/wintermute/base/gfx/3ds/mesh3ds.h"

namespace Wintermute {

Mesh3DS::Mesh3DS() : _vertexData(nullptr), _vertexCount(0), _indexData(nullptr), _indexCount(0) {

}

Mesh3DS::~Mesh3DS() {
	delete[] _vertexData;
	delete[] _indexData;
}

bool Wintermute::Mesh3DS::loadFrom3DS(Common::MemoryReadStream &fileStream) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	while (fileStream.pos() < end) {
		uint16 chunkId = fileStream.readUint16LE();
		uint32 chunkSize = fileStream.readUint32LE();

		switch (chunkId) {
		case VERTICES:
			_vertexCount = fileStream.readUint16LE();
			_vertexData = new GeometryVertex[_vertexCount]();

			for (int i = 0; i < _vertexCount; ++i) {
				// note that .3ds has a right handed coordinate system
				// with the z axis pointing upwards
				_vertexData[i].x = fileStream.readFloatLE();
				_vertexData[i].z = -fileStream.readFloatLE();
				_vertexData[i].y = fileStream.readFloatLE();
			}
			break;

		case FACES: {
			uint16 faceCount = fileStream.readUint16LE();
			_indexCount = 3 * faceCount;
			_indexData = new uint16[_indexCount];

			for (int i = 0; i < faceCount; ++i) {
				_indexData[i * 3 + 0] = fileStream.readUint16LE();
				_indexData[i * 3 + 1] = fileStream.readUint16LE();
				_indexData[i * 3 + 2] = fileStream.readUint16LE();
				// not used appearently
				fileStream.readUint16LE();
			}
			break;
		}
		case FACES_MATERIAL:
		case MAPPING_COORDS:
		case LOCAL_COORDS:
		case SMOOTHING_GROUPS:
		default:
			fileStream.seek(chunkSize - 6, SEEK_CUR);
			break;
		}
	}

	return true;
}

void Mesh3DS::dumpVertexCoordinates(const char *filename) {
	Common::DumpFile dump;
	dump.open(filename);

	for (uint16 *index = _indexData; index < _indexData + _indexCount; ++index) {
		float x = _vertexData[*index].x;
		float y = _vertexData[*index].y;
		float z = _vertexData[*index].z;

		dump.writeString(Common::String::format("%u ", *index));
		dump.writeString(Common::String::format("%g ", x));
		dump.writeString(Common::String::format("%g ", y));
		dump.writeString(Common::String::format("%g\n", z));
	}
}

int Mesh3DS::faceCount() {
	// .3ds files have only triangles anyways
	return _indexCount / 3;
}

uint16 *Mesh3DS::getFace(int index) {
	return _indexData + 3 * index;
}

float *Mesh3DS::getVertexPosition(int index) {
	return reinterpret_cast<float *>(&((_vertexData + index)->x));
}

int Wintermute::Mesh3DS::vertexCount() {
	return _vertexCount;
}

} // namespace Wintermute
