#ifndef WINTERMUTE_LOADER3DS_H
#define WINTERMUTE_LOADER3DS_H

#include "../../../coll_templ.h"

namespace Wintermute {
	class Light3D;
	class Camera3D;
	class Mesh;

	bool load3DSFile(const char* filename, BaseArray<Mesh*>& meshes, BaseArray<Common::String>& meshNames,
					   BaseArray<Light3D*>& lights, BaseArray<Camera3D*>& cameras);

	enum Chunks3DS {
		RGB_FLOAT = 0x0010,
		RGB_BYTE = 0x0011,
		EDITOR = 0x3D3D,
		OBJECT = 0x4000,
		MESH = 0x4100,
		VERTICES = 0x4110,
		FACES = 0x4120,
		FACES_MATERIAL = 0x4130,
		MAPPING_COORDS = 0x4140,
		SMOOTHING_GROUPS = 0x4150,
		LOCAL_COORDS = 0x4160,
		LIGHT = 0x4600,
		SPOTLIGHT = 0x4610,
		LIGHT_IS_OFF = 0x4620,
		SPOT_RAY_TRACE = 0x4627,
		SPOT_SHADOW_MAP = 0x4641,
		ROLL = 0x4656,
		SPOT_RAY_TRACE_BIAS = 0x4658,
		RANGE_END = 0x465A,
		MULTIPLIER = 0x465B,
		CAMERA = 0x4700,
		MAIN = 0x4D4D,
		KEYFRAMER = 0xB000,
		AMBIENT_INFO = 0xB001,
		MESH_INFO = 0xB002,
		CAMERA_INFO = 0xB003,
		CAMERA_TARGET_INFO = 0xB004,
		OMNI_LIGHT_INFO = 0xB005,
		SPOTLIGHT_TARGET_INFO = 0xB006,
		SPOTLIGHT_INFO = 0xB007,
		NODE_HEADER = 0xB010,
		ROLL_TRACK = 0xB024
	};
}
#endif
