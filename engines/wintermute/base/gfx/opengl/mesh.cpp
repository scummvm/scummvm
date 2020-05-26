#include "mesh.h"
#include "loader3ds.h"

Wintermute::Mesh::Mesh()
	: vertexData(nullptr), vertexCount(0),
	  indexData(nullptr), indexCount(0),
	  vertexBuffer(0), indexBuffer(0) {

}

Wintermute::Mesh::~Mesh() {

	GLuint bufferNames[2] = {vertexBuffer, indexBuffer};
	glDeleteBuffers(2, bufferNames);

	delete[] vertexData;
	delete[] indexData;
}

void Wintermute::Mesh::computeNormals() {

}

void Wintermute::Mesh::fillVertexBuffer(uint32 color) {
	if (vertexBuffer == 0) {
		GLuint bufferNames[2];
		glGenBuffers(2, bufferNames);
		vertexBuffer = *bufferNames;
		indexBuffer = *(bufferNames + 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * 4, 0, GL_STATIC_DRAW);
	uint32* bufferData = reinterpret_cast<uint32*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

	for (int i = 0; i < vertexCount; ++i) {
		*reinterpret_cast<uint32*>(bufferData + 4 * i) = color;
		*reinterpret_cast<float*>(bufferData + 4 * i + 1) = *reinterpret_cast<float*>(vertexData + 4 * 3 * i);
		*reinterpret_cast<float*>(bufferData + 4 * i + 2) = *reinterpret_cast<float*>(vertexData + 4 * 3 * i + 4);
		*reinterpret_cast<float*>(bufferData + 4 * i + 3) = *reinterpret_cast<float*>(vertexData + 4 * 3 * i + 8);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * 2, indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Wintermute::Mesh::loadFrom3DS(byte** buffer) {
	uint32 whole_chunk_size = *reinterpret_cast<uint32*>(*buffer);
	byte* end = *buffer + whole_chunk_size - 2;
	*buffer += 4;

	while (*buffer < end) {
		uint16 chunk_id = *reinterpret_cast<uint16*>(*buffer);

		switch (chunk_id) {
		case VERTICES:
			*buffer += 6;
			vertexCount = *reinterpret_cast<uint16*>(*buffer);
			*buffer += 2;

			vertexData = new byte[4 * 3 * vertexCount];

			for (int i = 0; i < vertexCount; ++i) {
				*reinterpret_cast<float*>(vertexData + 4 * 3 * i) = *reinterpret_cast<float*>(*buffer);
				*buffer += 4;
				*reinterpret_cast<float*>(vertexData + 4 * 3 * i + 8) = *reinterpret_cast<float*>(*buffer);
				*buffer += 4;
				*reinterpret_cast<float*>(vertexData + 4 * 3 * i + 4) = *reinterpret_cast<float*>(*buffer);
				*buffer += 4;
			}
			break;

		case FACES: {
			*buffer += 6;
			uint16 faceCount = *reinterpret_cast<uint16*>(*buffer);
			indexCount = 3 * faceCount;
			*buffer += 2;

			indexData = new uint16[indexCount];

			for (int i = 0; i < faceCount; ++i) {
				indexData[i * 3] = *reinterpret_cast<uint16*>(*buffer);
				*buffer += 2;
				indexData[i * 3 + 2] = *reinterpret_cast<uint16*>(*buffer);
				*buffer += 2;
				indexData[i * 3 + 1] = *reinterpret_cast<uint16*>(*buffer);
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
			*buffer += *reinterpret_cast<uint32*>(*buffer + 2);
		default:
			break;
		}
	}

	return true;
}

void Wintermute::Mesh::render()
{
	if (vertexBuffer == 0) {
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glInterleavedArrays(GL_C4UB_V3F, 0, 0);

	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
