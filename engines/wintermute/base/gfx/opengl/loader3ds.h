#ifndef WINTERMUTE_LOADER3DS_H
#define WINTERMUTE_LOADER3DS_H

#include "../../../coll_templ.h"

namespace Wintermute {
	class Light3D;
	class Camera3D;
	class Mesh;

	bool load_3ds_file(const char* filename, BaseArray<Mesh*>& meshes, BaseArray<Common::String>& meshNames,
					   BaseArray<Light3D*>& lights, BaseArray<Camera3D*>& cameras);
}
#endif
