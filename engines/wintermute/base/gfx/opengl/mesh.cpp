#include "mesh.h"
#include "loader3ds.h"

Wintermute::Mesh::Mesh()
	: vertexData(nullptr), vertexCount(0), indexData(nullptr), indexCount(0) {

}

Wintermute::Mesh::~Mesh() {
	delete[] vertexData;
	delete[] indexData;
}

void Wintermute::Mesh::computeNormals() {

}

void Wintermute::Mesh::fillVertexBuffer(uint32 color) {

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
