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
#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl_shader.h"
#include "engines/wintermute/wintypes.h"
#include "math/vector3d.h"

namespace Wintermute {

Mesh3DSOpenGLShader::Mesh3DSOpenGLShader() : _vertexData(nullptr), _vertexCount(0), _indexData(nullptr), _indexCount(0) {
}

Mesh3DSOpenGLShader::~Mesh3DSOpenGLShader() {
	delete[] _vertexData;
	delete[] _indexData;
}

void Mesh3DSOpenGLShader::computeNormals() {
	for (int i = 0; i < _vertexCount; ++i) {
		_vertexData[i].n1 = 0.0f;
		_vertexData[i].n2 = 0.0f;
		_vertexData[i].n3 = 0.0f;
	}

	for (int i = 0; i < faceCount(); ++i) {
		uint16 index1 = _indexData[3 * i + 0];
		uint16 index2 = _indexData[3 * i + 1];
		uint16 index3 = _indexData[3 * i + 2];

		Math::Vector3d v1(getVertexPosition(index1));
		Math::Vector3d v2(getVertexPosition(index2));
		Math::Vector3d v3(getVertexPosition(index3));

		Math::Vector3d edge1 = v2 - v1;
		Math::Vector3d edge2 = v3 - v2;
		Math::Vector3d normal = Math::Vector3d::crossProduct(edge1, edge2);

		_vertexData[index1].n1 += normal.x();
		_vertexData[index1].n2 += normal.y();
		_vertexData[index1].n3 += normal.z();

		_vertexData[index2].n1 += normal.x();
		_vertexData[index2].n2 += normal.y();
		_vertexData[index2].n3 += normal.z();

		_vertexData[index3].n1 += normal.x();
		_vertexData[index3].n2 += normal.y();
		_vertexData[index3].n3 += normal.z();
	}

	for (int i = 0; i < _vertexCount; ++i) {
		Math::Vector3d normal;
		normal.x() = _vertexData[i].n1;
		normal.y() = _vertexData[i].n2;
		normal.z() = _vertexData[i].n3;
		normal.normalize();

		_vertexData[i].n1 = normal.x();
		_vertexData[i].n2 = normal.y();
		_vertexData[i].n3 = normal.z();
	}
}

void Mesh3DSOpenGLShader::fillVertexBuffer(uint32 color) {
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);
	byte a = RGBCOLGetA(color);

	for (int i = 0; i < _vertexCount; ++i) {
		_vertexData[i].r = r;
		_vertexData[i].g = g;
		_vertexData[i].b = b;
		_vertexData[i].a = a;
	}
}

bool Wintermute::Mesh3DSOpenGLShader::loadFrom3DS(Common::MemoryReadStream &fileStream) {
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

void Mesh3DSOpenGLShader::render() {
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glColorPointer(4, GL_UNSIGNED_BYTE, kVertexSize, _vertexData);
	glNormalPointer(3, kVertexSize, reinterpret_cast<byte *>(_vertexData) + 4);
	glVertexPointer(3, GL_FLOAT, kVertexSize, reinterpret_cast<byte *>(_vertexData) + 16);

	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, _indexData);
}

void Mesh3DSOpenGLShader::dumpVertexCoordinates(const char *filename) {
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

int Mesh3DSOpenGLShader::faceCount() {
	// .3ds files have only triangles anyways
	return _indexCount / 3;
}

uint16 *Mesh3DSOpenGLShader::getFace(int index) {
	return _indexData + 3 * index;
}

float *Mesh3DSOpenGLShader::getVertexPosition(int index) {
	return reinterpret_cast<float *>(&((_vertexData + index)->x));
}

int Wintermute::Mesh3DSOpenGLShader::vertexCount() {
	return _vertexCount;
}

} // namespace Wintermute
