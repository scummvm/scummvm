#ifndef WINTERMUTE_MESH_H
#define WINTERMUTE_MESH_H

#include "common/scummsys.h"
#include "graphics/opengl/system_headers.h"

namespace Wintermute {

class Mesh
{
public:
	Mesh();
	~Mesh();
	void computeNormals();
	void fillVertexBuffer(uint32 color);
	bool loadFrom3DS(byte** buffer);
	void render();

private:
	byte* vertexData;
	uint16 vertexCount;
	uint16* indexData;
	uint16 indexCount;
	GLuint vertexBuffer;
	GLuint indexBuffer;
};

}

#endif
