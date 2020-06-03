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
#include "engines/wintermute/base/gfx/opengl/loader3ds.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds.h"

namespace Wintermute {

Mesh3DS::Mesh3DS()
    : _vertexData(nullptr), _vertexCount(0),
      _indexData(nullptr), _indexCount(0),
      _vertexBuffer(0), _indexBuffer(0) {
}

Mesh3DS::~Mesh3DS() {

	GLuint bufferNames[2] = {_vertexBuffer, _indexBuffer};
	glDeleteBuffers(2, bufferNames);

	delete[] _vertexData;
	delete[] _indexData;
}

void Mesh3DS::computeNormals() {
}

void Mesh3DS::fillVertexBuffer(uint32 color) {
	if (_vertexBuffer == 0) {
		GLuint bufferNames[2];
		glGenBuffers(2, bufferNames);
		_vertexBuffer = *bufferNames;
		_indexBuffer = *(bufferNames + 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, _vertexCount * kVertexSize, 0, GL_STATIC_DRAW);
	uint32 *bufferData = reinterpret_cast<uint32 *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

	for (int i = 0; i < _vertexCount; ++i) {
		*reinterpret_cast<uint32 *>(bufferData + 4 * i) = color;
		*reinterpret_cast<float *>(bufferData + 4 * i + 1) = *reinterpret_cast<float *>(_vertexData + 4 * 3 * i);
		*reinterpret_cast<float *>(bufferData + 4 * i + 2) = *reinterpret_cast<float *>(_vertexData + 4 * 3 * i + 4);
		*reinterpret_cast<float *>(bufferData + 4 * i + 3) = *reinterpret_cast<float *>(_vertexData + 4 * 3 * i + 8);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexCount * 2, _indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Wintermute::Mesh3DS::loadFrom3DS(byte **buffer) {
	uint32 whole_chunk_size = *reinterpret_cast<uint32 *>(*buffer);
	byte *end = *buffer + whole_chunk_size - 2;
	*buffer += 4;

	while (*buffer < end) {
		uint16 chunk_id = *reinterpret_cast<uint16 *>(*buffer);

		switch (chunk_id) {
		case VERTICES:
			*buffer += 6;
			_vertexCount = *reinterpret_cast<uint16 *>(*buffer);
			*buffer += 2;

			_vertexData = new byte[4 * 3 * _vertexCount];

			for (int i = 0; i < _vertexCount; ++i) {
				*reinterpret_cast<float *>(_vertexData + 4 * 3 * i) = *reinterpret_cast<float *>(*buffer);
				*buffer += 4;
				*reinterpret_cast<float *>(_vertexData + 4 * 3 * i + 8) = -*reinterpret_cast<float *>(*buffer);
				*buffer += 4;
				*reinterpret_cast<float *>(_vertexData + 4 * 3 * i + 4) = *reinterpret_cast<float *>(*buffer);
				*buffer += 4;
			}
			break;

		case FACES: {
			*buffer += 6;
			uint16 faceCount = *reinterpret_cast<uint16 *>(*buffer);
			_indexCount = 3 * faceCount;
			*buffer += 2;

			_indexData = new uint16[_indexCount];

			for (int i = 0; i < faceCount; ++i) {
				_indexData[i * 3] = *reinterpret_cast<uint16 *>(*buffer);
				*buffer += 2;
				_indexData[i * 3 + 2] = *reinterpret_cast<uint16 *>(*buffer);
				*buffer += 2;
				_indexData[i * 3 + 1] = *reinterpret_cast<uint16 *>(*buffer);
				*buffer += 2;
				// not used appearently
				*buffer += 2;
			}
			break;
		}
		case FACES_MATERIAL:
		case MAPPING_COORDS:
		case LOCAL_COORDS:
		case SMOOTHING_GROUPS:
			*buffer += *reinterpret_cast<uint32 *>(*buffer + 2);
		default:
			break;
		}
	}

	return true;
}

void Mesh3DS::render() {
	if (_vertexBuffer == 0) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	glInterleavedArrays(GL_C4UB_V3F, 0, 0);

	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh3DS::dumpVertexCoordinates(const char *filename) {
	Common::DumpFile dump;
	dump.open(filename);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	uint32 *vertices = reinterpret_cast<uint32 *>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
	uint16 *indices = reinterpret_cast<uint16 *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));

	for (uint16 *index = indices; index < indices + _indexCount; ++index) {
		float x = *reinterpret_cast<float *>(vertices + 4 * *index + 1);
		float y = *reinterpret_cast<float *>(vertices + 4 * *index + 2);
		float z = *reinterpret_cast<float *>(vertices + 4 * *index + 3);

		dump.writeString(Common::String::format("%u ", *index));
		dump.writeString(Common::String::format("%g ", x));
		dump.writeString(Common::String::format("%g ", y));
		dump.writeString(Common::String::format("%g\n", z));
	}

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Wintermute
