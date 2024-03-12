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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_COMMONYPES_H
#define NANCY_COMMONYPES_H

#include "common/rect.h"
#include "common/array.h"
#include "common/str.h"
#include "math/vector3d.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;

// The original engine used a large amount of #defines for numerical constants,
// which can be found listed inside the gameflow.h file shipping with many of
// the titles (the Russian variant of nancy1 has it as a separate file, while
// nancy2 and above embed it within the ciftree).
//
// Other, more specific constants are declared within their related classes,
// so as not to litter the namespace

static const int8 kFlagNoLabel						= -1;
static const int8 kEvNoEvent						= -1;
static const int8 kFrNoFrame						= -1;
static const uint16 kNoScene						= 9999;

// Inventory items use types
static const byte kInvItemUseThenLose				= 0;
static const byte kInvItemKeepAlways				= 1;
static const byte kInvItemReturn					= 2;
static const byte kInvItemNewSceneView				= 3;

// Inventory item sound override commands
static const byte kInvSoundOverrideCommandNoSound	= 0;
static const byte kInvSoundOverrideCommandTurnOff	= 1;
static const byte kInvSoundOverrideCommandNewSound	= 2;
static const byte kInvSoundOverrideCommandICant		= 3;

// Dependency types
static const byte kFlagEvent						= 1;
static const byte kFlagInventory					= 2;
static const byte kFlagCursor						= 3;

// Scene panning
static const byte kPanNone							= 0;
static const byte kPan360							= 1;
static const byte kPanLeftRight						= 2;

// Scene sound flags
static const byte kContinueSceneSound				= 1;
static const byte kLoadSceneSound					= 0;

// Scene rotation special values
static const uint16 kInvertedNode					= 77;
static const uint16 kNoAutoScroll					= 333;

// Clock bump types
static const byte kAbsoluteClockBump 				= 1;
static const byte kRelativeClockBump 				= 2;

// Time of day
static const byte kPlayerDay						= 0;
static const byte kPlayerNight						= 1;
static const byte kPlayerDuskDawn					= 2;

// Video
static const byte kSmallVideoFormat					= 1;
static const byte kLargeVideoFormat					= 2;

static const byte kVideoPlaytypeAVF					= 0;
static const byte kVideoPlaytypeBink				= 1;

// Overlay
static const byte kPlayOverlayPlain					= 1;
static const byte kPlayOverlayTransparent			= 2;

static const byte kPlayOverlaySceneChange			= 1;
static const byte kPlayOverlayNoSceneChange			= 2;

static const byte kPlayOverlayStatic				= 1;
static const byte kPlayOverlayAnimated				= 2;

static const byte kPlayOverlayOnce					= 1;
static const byte kPlayOverlayLoop					= 2;

static const byte kPlayOverlayForward				= 1;
static const byte kPlayOverlayReverse				= 2;

static const byte kPlayOverlayWithHotspot			= 1;
static const byte kPlayOverlayNoHotspot				= 2;

// Table access
static const byte kNoChangeTableValue				= 0;
static const byte kIncrementTableValue				= 1;
static const byte kDecrementTableValue				= 2;
static const uint16 kNoTableIndex					= 99;
static const int16 kNoTableValue					= 9999;

// Autotext ordering info
static const uint16 kListLIFO						= 0;
static const uint16 kListFIFO						= 1;

// 3D sound rotation
static const byte kRotateAroundX					= 0;
static const byte kRotateAroundY					= 1;
static const byte kRotateAroundZ					= 2;

enum MovementDirection : byte { kUp = 1, kDown = 2, kLeft = 4, kRight = 8, kMoveFast = 16 };

// Separate namespace to remove possible clashes
namespace NancyState {
enum NancyState {
	// Original engine states
	kBoot, kLogo, kCredits, kMap,
	kMainMenu, kLoadSave, kSetup,
	kHelp, kScene, kSaveDialog,

	// Not real states
	kNone,
	kQuit,
	kPause, // only used when the GMM is on screen
};
}

// Describes a scene transition
struct SceneChangeDescription {
	uint16 sceneID = kNoScene;
	uint16 frameID = 0;
	uint16 verticalOffset = 0;
	uint16 continueSceneSound = kLoadSceneSound;

	int8 paletteID = -1; // TVD only

	Math::Vector3d listenerFrontVector = Math::Vector3d(0, 0, 1);
	uint16 frontVectorFrameID = 0;

	void readData(Common::SeekableReadStream &stream, bool longFormat = false);
};

// Describes a single flag change or comparison
struct FlagDescription {
	int16 label = -1;
	byte flag = 0;
};

struct SceneChangeWithFlag {
	SceneChangeDescription _sceneChange;
	FlagDescription _flag;

	void readData(Common::SeekableReadStream &stream, bool reverseFormat = false);
	void execute();
};

// Describes a hotspot
struct HotspotDescription {
	uint16 frameID = 0;
	Common::Rect coords;

	void readData(Common::SeekableReadStream &stream);
};

// Describes a blit operation, dependent on a background frame
struct FrameBlitDescription {
	uint16 frameID = 0; // Frame ID of the Scene background
	uint16 staticRectID = 0; // Used in Overlay
	uint hasHotspot = kPlayOverlayNoHotspot;
	Common::Rect src;
	Common::Rect dest;

	void readData(Common::SeekableReadStream &stream, bool longFormat = false);
};

// Describes 10 event flag changes to be executed when an action is triggered
struct MultiEventFlagDescription {
	FlagDescription descs[10];

	void readData(Common::SeekableReadStream &stream);
	void execute();
};

struct SecondaryVideoDescription {
	int16 frameID = kFrNoFrame;
	Common::Rect srcRect;
	Common::Rect destRect;
	// 2 unknown/empty rects

	void readData(Common::SeekableReadStream &stream);
};

// Describes set of effects that can be applied to sounds.
// Defaults are set according to the values used by PlaySoundTerse
struct SoundEffectDescription {
	uint32 minTimeDelay = 500;
	uint32 maxTimeDelay = 2000;

	int32 randomMoveMinX = 0;
	int32 randomMoveMaxX = 0;
	int32 randomMoveMinY = 0;
	int32 randomMoveMaxY = 0;
	int32 randomMoveMinZ = 0;
	int32 randomMoveMaxZ = 0;

	int32 fixedPosX = 0;
	int32 fixedPosY = 0;
	int32 fixedPosZ = 0;

	uint32 moveStepTime = 1000;
	int32 numMoveSteps = 10;

	int32 linearMoveStartX = 0;
	int32 linearMoveEndX = 0;
	int32 linearMoveStartY = 0;
	int32 linearMoveEndY = 0;
	int32 linearMoveStartZ = 0;
	int32 linearMoveEndZ = 0;

	int32 rotateMoveStartX = 0;
	int32 rotateMoveStartY = 0;
	int32 rotateMoveStartZ = 0;
	byte rotateMoveAxis = kRotateAroundY;

	uint32 minDistance = 0;
	uint32 maxDistance = 0;

	void readData(Common::SeekableReadStream &stream);
};

// Descrbes a single sound. Combines four different structs found in the data in one
struct SoundDescription {
	Common::String name = "NO SOUND";
	uint16 channelID = 0;
	uint16 playCommands = 1;
	uint16 numLoops = 1;
	uint16 volume = 50;
	uint16 panAnchorFrame = 0;
	uint32 samplesPerSec = 0;
	bool isPanning = false;

	void readNormal(Common::SeekableReadStream &stream);
	void readDIGI(Common::SeekableReadStream &stream);
	void readMenu(Common::SeekableReadStream &stream);
	void readScene(Common::SeekableReadStream &stream);
	void readTerse(Common::SeekableReadStream &stream);
};

// Structs inside nancy.dat, which contains all the data that was
// originally stored inside the executable

enum class StaticDataConditionType { kEvent = 0, kInventory = 1, kDifficulty = 2 };
struct StaticDataFlag { byte type; int16 label; byte flag; };

struct ConditionalDialogue {
	byte textID;
	uint16 sceneID;
	Common::String soundID;
	Common::Array<StaticDataFlag> conditions;

	void readData(Common::SeekableReadStream &stream);
};

struct GoodbyeSceneChange {
	Common::Array<uint16> sceneIDs;
	Common::Array<StaticDataFlag> conditions;
	StaticDataFlag flagToSet;

	void readData(Common::SeekableReadStream &stream);
};

struct Goodbye {
	Common::String soundID;
	Common::Array<GoodbyeSceneChange> sceneChanges;

	void readData(Common::SeekableReadStream &stream);
};

struct Hint {
	byte textID;
	int16 hintWeight;
	SceneChangeDescription sceneChange;
	Common::String soundIDs[3];
	Common::Array<StaticDataFlag> conditions;

	void readData(Common::SeekableReadStream &stream);
};

struct SoundChannelInfo {
	byte numChannels;
	byte numSceneSpecificChannels;
	Common::Array<byte> speechChannels;
	Common::Array<byte> musicChannels;
	Common::Array<byte> sfxChannels;

	void readData(Common::SeekableReadStream &stream);
};

struct StaticData {
	// Default values are for nancy1, provided for debugging purposes
	uint16 numItems = 11;
	uint16 numEventFlags = 168;
	Common::Array<uint16> mapAccessSceneIDs;
	Common::Array<uint16> genericEventFlags;
	uint16 numCursorTypes = 4;
	uint32 logoEndAfter = 7000;
	int16 wonGameFlagID = -1;

	// Data for sound channels
	SoundChannelInfo soundChannelInfo;

	// In-game strings and related logic
	Common::Array<Common::Array<ConditionalDialogue>> conditionalDialogue;
	Common::Array<Goodbye> goodbyes;
	Common::Array<Common::Array<Hint>> hints;

	Common::Array<Common::String> conditionalDialogueTexts;
	Common::Array<Common::String> goodbyeTexts;
	Common::Array<Common::String> hintTexts;
	Common::String ringingText;
	Common::String emptySaveText;

	// Debug strings
	Common::Array<Common::String> eventFlagNames;

	void readData(Common::SeekableReadStream &stream, Common::Language language, uint32 endPos, int8 majorVersion, int8 minorVersion);
};

} // End of namespace Nancy

#endif // NANCY_COMMONYPES_H
