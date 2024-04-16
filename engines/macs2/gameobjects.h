#pragma once
class gameobjects {
};
#pragma once

#include "common/array.h"
#include "common/singleton.h"

namespace Macs2 {

	class Scene {
		public:
		class Common::MemoryReadStream *Script;

	};

	class Scenes : public Common::Singleton<Scenes> {
		// Data for scenes can be accessed:
		// Script data: Load from objects data [0752]: ID * 0xC, offset at [di-6] and [di-8h]


		public:
		Common::Array<Scene> Scenes;

		// Global [077Ch]
		int CurrentSceneIndex;
	};

class GameObject {
public:
	// These are the values read by fn0037_07F8
	Common::Array<uint16> Values;

	// Each object can have up to 15h blocks of data that are loaded, which can
	// include the animations, the dialogue images, the inventory icons etc.
	Common::Array<Common::Array<uint8> > Blobs;
};

class GameObjects : public Common::Singleton<GameObjects> {

public:
	// Maximum of 200h objects
	// How to address them in the original code:
	// mov	di,[bp+6h]
	//	shl di, 2h;
	// les di, [di + 77Ch]
	Common::Array<GameObject> Objects;
};

} // namespace Macs2
