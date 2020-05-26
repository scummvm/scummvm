#ifndef WINTERMUTE_MESH_H
#define WINTERMUTE_MESH_H

#include "common/scummsys.h"

namespace Wintermute {

class Mesh
{
public:
	Mesh();
	~Mesh();
	void computeNormals();
	void fillVertexBuffer(uint32 color);
	bool loadFrom3DS(byte** buffer);

private:
	byte* vertexData;
	uint16 vertexCount;
	uint16* indexData;
	uint16 indexCount;
};

}

#endif
