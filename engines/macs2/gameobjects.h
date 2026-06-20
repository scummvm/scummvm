/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MACS2_GAMEOBJECTS_H
#define MACS2_GAMEOBJECTS_H

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

	Common::Array<uint8> _overloadAnimation;
	uint16 _overloadAnimationSourceKey = 0;
	bool _overloadAnimationMirrored = false;
	bool _useOverloadAnimation = false;
	// Runtime field +0x22D: when the character's orientation matches this value,
	// the renderer uses animation slot 0x15 (overload) instead of the normal slot.
	// Initialized to 0x7FFF (never match). Set by opcode 0x27.
	uint16 _overloadAnimTriggerDirection = 0x7FFF;

	// These are the values read by the code around l0037_082D:
	Common::Point _position;
	uint16 _sceneIndex;
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
	uint16 _orientation;
	// Per-object percentage multiplier for ground-elevation vertical offset.
	// Walkability map values < 0xC8 represent ground height at each pixel;
	// this factor scales how much that height displaces the object upward
	// when drawn. 0 = no vertical offset. 100 = full elevation offset.
	uint16 _verticalOffsetScale;
	// Runtime +0x217: frame index during pickup animation at which the item is grabbed
	uint16 _pickupFrameStart = 0;
	// Runtime +0x219: frame index at which pickup animation completes
	uint16 _pickupFrameEnd = 0;
	// Runtime +0x22F: when set, snap character position to exact target on walk arrival
	bool _snapToTarget = false;
	// Runtime +0x185 (bHasShading): per-object flag loaded from file. When set, character sprites
	// are drawn through the shading table using the shadow map intensity.
	bool _hasShading = false;
	// Runtime +0x186: per-object flag loaded from file. When set, character sprites
	// are scaled based on Y position (perspective depth scaling).
	bool _hasScaling = false;
	// Runtime field +0x231: "frozen/attached" flag. Set by scriptSetObjectBounds (opcode 0x35).
	// When set, the object cannot be walked (opcode 0x11 returns error 0x1F)
	// and walkAlongPath skips movement for this object.
	// Cleared when objectA == objectB in scriptSetObjectBounds.
	bool _hasBoundsAttachment = false;
	uint16 _boundsAttachmentObjectID = 0; // +0x232
	uint16 _boundsAttachmentValue1 = 0;   // +0x234
	uint16 _boundsAttachmentValue2 = 0;   // +0x236
	uint16 _boundsAttachmentValue3 = 0;   // +0x238

	// Runtime[+0x20D..+0x213] dirty rect; [+0x225..+0x22B] last sprite draw bounds.
	int16 _dirtyLeft = 0;
	int16 _dirtyTop = 0;
	int16 _dirtyRight = 0;
	int16 _dirtyBottom = 0;
	int16 _lastDrawX = 0;
	int16 _lastDrawY = 0;
	uint16 _lastDrawWidth = 0;
	uint16 _lastDrawHeight = 0;

	void resetDrawBounds() {
		_lastDrawX = 0;
		_lastDrawY = 0;
		_lastDrawWidth = 0;
		_lastDrawHeight = 0;
		_dirtyLeft = 0;
		_dirtyTop = 0;
		_dirtyRight = 0;
		_dirtyBottom = 0;
	}

	// Each object can have up to 15h blocks of data that are loaded, which can
	// include the animations, the dialogue images, the inventory icons etc.
	Common::Array<Common::Array<uint8>> _blobs;
	Common::Array<uint16> _blobSourceKeys; // Per-slot wSourceKey2 from resource file (slot+0x02, editor metadata)
	Common::Array<bool> _blobMirrorFlags;
	Common::Array<uint16> _blobWalkSpeeds; // Per-slot wAnimSpeed (slot+0x0C, walk speed 2-8 px/frame, used by walkAlongPath)

	// The object-specific script
	Common::Array<uint8> _script;

	// Per-object resource offset table (runtime +0x18D, 128 bytes = 32 dword file offsets).
	// Loaded from file during loadObjectData. Used by scriptLoadObjectAnim/scriptLoadSpecialAnim
	// to look up animation resource file addresses for this object.
	uint32 _resourceOffsets[32] = {0};

	// Stashed walk/motion runtime when no Character exists (binary heap runtime
	// survives scene change / off-scene script opcodes). Restored on Character create.
	struct StoredWalkRuntime {
		bool valid = false;
		Common::Point targetPosition;
		Common::Point pathFinalDestination;
		int16 stepDeltaX = 0;
		int16 stepDeltaY = 0;
		int16 stepError = 0;
		bool stepDirectionSet = false;
		int16 currentPathIndex = 0;
		Common::Array<uint16> path;
		uint16 motionTargetVerticalOffset = 0;
		uint16 motionVerticalOffsetDelta = 0;
		uint16 motionDistanceUnits = 0;
		uint16 motionProgress = 0;
	};
	StoredWalkRuntime _storedWalkRuntime;

	Common::MemoryReadStream *getScriptStream();

	// Binary pAnimSlots[1..0x15] at runtime+slot*0x10. Slot 0x15 may be in _blobs[20]
	// (loadObjectData) or overloadAnimation (script load / savegame).
	Common::Array<uint8> *getAnimSlotBlob(uint16 slot);
	const Common::Array<uint8> *getAnimSlotBlob(uint16 slot) const;

	// Binary bSlotLoaded (runtime+orient*0x10+0x33): blob present or walk-speed high byte set.
	bool isAnimSlotLoaded(uint16 orient) const;
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

#endif // MACS2_GAMEOBJECTS_H
