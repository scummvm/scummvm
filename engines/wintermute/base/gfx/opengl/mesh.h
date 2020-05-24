#ifndef WINTERMUTE_MESH_H
#define WINTERMUTE_MESH_H

#include "common/scummsys.h"

namespace Wintermute {

class Mesh
{
public:
	void computeNormals();
	void fillVertexBuffer(uint32 color);
};

}

#endif
