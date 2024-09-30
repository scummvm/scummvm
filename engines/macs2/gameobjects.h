#pragma once
class gameobjects {
};
#pragma once

#include "common/array.h"
#include "common/singleton.h"
#include "common/rect.h"


namespace Common {
class MemoryReadStream;
class MemoryReadStreamEndian;
}

namespace Macs2 {

	class Scene {
		public:
		Common::MemoryReadStream *Script;

	};

	class Scenes : public Common::Singleton<Scenes> {
		// Data for scenes can be accessed:
		// Script data: Load from objects data [0752]: ID * 0xC, offset at [di-6] and [di-8h]


		public:
		Common::Array<Scene> Scenes;

		// Global [077Ch]
		int CurrentSceneIndex;

		// Global [077Eh]
		// TODO: Check what the initial value of this is
		int LastSceneIndex;

		// TODO: Handle properly as a field of the scene 
		class Common::MemoryReadStream* CurrentSceneScript;

		class Common::MemoryReadStream *CurrentSceneStrings;
		Common::Array<uint32> CurrentSceneSpecialAnimOffsets;

		class Common::MemoryReadStream *ReadSceneScript(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
		Common::Array<uint32> ReadSpecialAnimsOffsets(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
		class Common::MemoryReadStream *ReadSceneStrings(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
		Common::Array<uint8> ReadSpecialAnimBlob(uint16 index, Common::MemoryReadStream *fileStream);
	};

class AnimationReader {

	private:
	public:

		Common::MemoryReadStreamEndian *readStream;

		// TODO: Can the init list also go into the cpp file?
		AnimationReader(const Common::Array<uint8> &blob);

		uint16 readNumAnimations();

		void SeekToAnimation(uint16 index);

		// Expects us to be pointed at the header of an animation frame,
		// will seek to the start of the next header
		void SkipCurrentAnimationFrame();

};

class GameObject {
public:

	// Index of the object, starting at 1
	uint16 Index;

	int testOverloadAnimation = -1;

	// These are the values read by the code around l0037_082D:
	Common::Point Position;
	uint16 SceneIndex;
	uint16 Orientation;
	uint16 Unknown;

	// Each object can have up to 15h blocks of data that are loaded, which can
	// include the animations, the dialogue images, the inventory icons etc.
	Common::Array<Common::Array<uint8> > Blobs;

	// The object-specific script
	// TODO: Random thought - do objects have their own space for script variables?
	Common::Array<uint8> Script;

	Common::MemoryReadStream *GetScriptStream(); 
};

class GameObjects : public Common::Singleton<GameObjects> {

public:
	// Maximum of 200h objects
	// How to address them in the original code:
	// mov	di,[bp+6h]
	//	shl di, 2h;
	// les di, [di + 77Ch]
	Common::Array<GameObject*> Objects;

	static GameObject *GetProtagonistObject();

	static GameObject *GetObjectByIndex(uint16 index);

	static class Common::MemoryReadStream *ReadGameObjectStrings(uint16 index, Common::MemoryReadStream *fileStream);
};

} // namespace Macs2
