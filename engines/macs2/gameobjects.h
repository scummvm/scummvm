#pragma once
class gameobjects {
};
#pragma once

#include "common/array.h"
#include "common/rect.h"
#include "common/singleton.h"

namespace Common {
class MemoryReadStream;
class MemoryReadStreamEndian;
} // namespace Common

namespace Macs2 {

class Scene {
public:
	Common::MemoryReadStream *_script;
};

// Data for scenes can be accessed:
// Script data: Load from objects data [0752]: ID * 0xC, offset at [di-6] and [di-8h]
class Scenes : public Common::Singleton<Scenes> {
public:
	Common::Array<Scene> _scenes;

	// Global [077Ch]
	int _currentSceneIndex;

	// Global [077Eh]
	// TODO: Check what the initial value of this is
	int _lastSceneIndex;

	// Global [0776h]
	int _currentActorIndex;

	// TODO: Handle properly as a field of the scene
	class Common::MemoryReadStream *_currentSceneScript;

	class Common::MemoryReadStream *_currentSceneStrings;
	Common::Array<uint32> _currentSceneSpecialAnimOffsets;

	class Common::MemoryReadStream *readSceneScript(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
	Common::Array<uint32> readSpecialAnimsOffsets(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
	class Common::MemoryReadStream *readSceneStrings(uint16 sceneIndex, Common::MemoryReadStream *fileStream);
	Common::Array<uint8> readSpecialAnimBlob(uint16 index, Common::MemoryReadStream *fileStream);
};

class AnimationReader {
public:
	Common::MemoryReadStreamEndian *_readStream;

	// TODO: Can the init list also go into the cpp file?
	AnimationReader(const Common::Array<uint8> &blob);

	uint16 readNumAnimations();

	void seekToAnimation(uint16 index);

	// Expects us to be pointed at the header of an animation frame,
	// will seek to the start of the next header
	void skipCurrentAnimationFrame();
};

class GameObject {
public:
	// Index of the object, starting at 1
	uint16 _index;
	uint32 _dataOffset = 0;

	Common::Array<uint8> overloadAnimation;
	uint16 overloadAnimationSourceKey = 0;
	bool overloadAnimationMirrored = false;
	bool useOverloadAnimation = false;
	// Runtime field +0x22D: when the character's orientation matches this value,
	// the renderer uses animation slot 0x15 (overload) instead of the normal slot.
	// Initialized to 0x7FFF (never match). Set by opcode 0x27.
	uint16 overloadAnimTriggerDirection = 0x7FFF;

	// These are the values read by the code around l0037_082D:
	Common::Point Position;
	uint16 SceneIndex;
	// 8-directional movement system from walkAlongPath (1008:1b8f).
	// Direction codes 1-8 are walking directions, 9-16 are standing (idle) variants.
	// The direction is chosen based on the angle between current and target position:
	//   1 = North (up)         - deltaY dominates, target above
	//   2 = NorthEast          - diagonal (deltaX/4 < deltaY < deltaX*2)
	//   3 = East (right)       - deltaX dominates, target to the right
	//   4 = SouthEast          - diagonal
	//   5 = South (down)       - deltaY dominates, target below
	//   6 = SouthWest          - diagonal
	//   7 = West (left)        - deltaX dominates, target to the left
	//   8 = NorthWest          - diagonal
	//   9-16 = Standing idle variants (walking direction + 8)
	//   17 (0x11) = Pickup animation
	// Each direction has a validity flag at runtime offset +0x43 + (dir-1)*0x20
	// that indicates whether the object has animation data for that direction.
	uint16 Orientation;
	uint16 Unknown;
	// Runtime +0x217: frame index during pickup animation at which the item is grabbed
	uint16 _pickupFrameStart = 0;
	// Runtime +0x219: frame index at which pickup animation completes
	uint16 _pickupFrameEnd = 0;
	uint16 RuntimeSlotValues[0x15] = {0};
	// Runtime +0x22F: when set, snap character position to exact target on walk arrival
	bool _snapToTarget = false;
	bool IsClickable = true;
	bool IsVisible = true;
	// Runtime +0x185: per-object flag loaded from file. When set, character sprites
	// are drawn through the shading table using the shadow map intensity.
	bool HasShading = false;
	// Runtime +0x186: per-object flag loaded from file. When set, character sprites
	// are scaled based on Y position (perspective depth scaling).
	bool HasScaling = false;
	// Runtime field +0x231: "frozen/attached" flag. Set by scriptSetObjectBounds (opcode 0x35).
	// When set, the object cannot be walked (opcode 0x11 returns error 0x1F)
	// and walkAlongPath skips movement for this object.
	// Cleared when objectA == objectB in scriptSetObjectBounds.
	bool HasBoundsAttachment = false;
	uint16 BoundsAttachmentObjectID = 0; // +0x232
	uint16 BoundsAttachmentValue1 = 0;   // +0x234
	uint16 BoundsAttachmentValue2 = 0;   // +0x236
	uint16 BoundsAttachmentValue3 = 0;   // +0x238

	// Each object can have up to 15h blocks of data that are loaded, which can
	// include the animations, the dialogue images, the inventory icons etc.
	Common::Array<Common::Array<uint8>> Blobs;
	Common::Array<uint16> BlobSourceKeys;
	Common::Array<bool> BlobMirrorFlags;
	Common::Array<uint16> BlobSpeeds; // Per-animation walk speed (runtime+slot*16+0x30)

	// The object-specific script
	Common::Array<uint8> Script;

	// Per-object resource offset table (runtime +0x18D, 128 bytes = 32 dword file offsets).
	// Loaded from file during loadSceneObjects. Used by scriptLoadObjectAnim/scriptLoadSpecialAnim
	// to look up animation resource file addresses for this object.
	uint32 _resourceOffsets[32] = {0};

	Common::MemoryReadStream *getScriptStream();
};

class GameObjects : public Common::Singleton<GameObjects> {
public:
	// Maximum of 200h objects
	// How to address them in the original code:
	// mov	di,[bp+6h]
	//	shl di, 2h;
	// les di, [di + 77Ch]
	Common::Array<GameObject *> _objects;

	Common::Array<Common::String> _objectNames;

	void init();

	static GameObject *getProtagonistObject();

	static GameObject *getObjectByIndex(uint16 index);

	static class Common::MemoryReadStream *readGameObjectStrings(uint16 index, Common::MemoryReadStream *fileStream);
};

} // namespace Macs2
