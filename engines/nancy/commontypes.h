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

static const int8 kFlagNoLabel			= -1;
static const int8 kEvNoEvent			= -1;
static const int8 kFrNoFrame			= -1;

// Inventory items use types
static const byte kInvItemUseThenLose	= 0;
static const byte kInvItemKeepAlways	= 1;
static const byte kInvItemReturn		= 2;

// Dependency types
static const byte kFlagEvent			= 1;
static const byte kFlagInventory		= 2;
static const byte kFlagCursor			= 3;

// Scene sound flags
static const byte kContinueSceneSound	= 1;
static const byte kLoadSceneSound		= 0;

// Clock bump types
static const byte kAbsoluteClockBump 	= 1;
static const byte kRelativeClockBump 	= 2;

// Time of day
static const byte kPlayerDay			= 0;
static const byte kPlayerNight			= 1;
static const byte kPlayerDuskDawn		= 2;

// Video
static const byte kSmallVideoFormat		= 1;
static const byte kLargeVideoFormat		= 2;

enum MovementDirection : byte { kUp = 1, kDown = 2, kLeft = 4, kRight = 8, kMoveFast = 16 };

// Separate namespace to remove possible clashes
namespace NancyState {
enum NancyState {
	kBoot,
	kPartnerLogo,
	kLogo,
	kCredits,
	kMap,
	kMainMenu,
	kLoadSave,
	kSetup,
	// unknown/invalid
	kHelp,
	kScene,
	// CD change
	// Cheat,
	kQuit,
	// regain focus
	kNone,
	kPause, // only used when the GMM is on screen
	kReloadSave
};
}

// Describes a scene transition
struct SceneChangeDescription {
	uint16 sceneID = 0;
	uint16 frameID = 0;
	uint16 verticalOffset = 0;
	uint16 continueSceneSound = kLoadSceneSound;

	int8 paletteID = -1; // TVD only

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

	void readData(Common::SeekableReadStream &stream, bool longFormat = false);
	void execute();
};

// Describes a hotspot
struct HotspotDescription {
	uint16 frameID = 0;
	Common::Rect coords;

	void readData(Common::SeekableReadStream &stream);
};

// Describes a single bitmap draw
struct BitmapDescription {
	uint16 frameID = 0;
	Common::Rect src;
	Common::Rect dest;

	void readData(Common::SeekableReadStream &stream, bool frameIsLong = false);
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

// Descrbes a single sound. Combines four different structs found in the data in one
struct SoundDescription {
	Common::String name;
	uint16 channelID = 0;
	uint16 playCommands = 1;
	uint16 numLoops = 0;
	uint16 volume = 0;
	uint16 panAnchorFrame = 0;
	uint32 samplesPerSec = 0;

	void readNormal(Common::SeekableReadStream &stream);
	void readDIGI(Common::SeekableReadStream &stream);
	void readMenu(Common::SeekableReadStream &stream);
	void readScene(Common::SeekableReadStream &stream);
};

// Structs inside nancy.dat, which contains all the data that was
// originally stored inside the executable

enum class StaticDataConditionType : byte { kEvent = 0, kInventory = 1, kDifficulty = 2 };
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
	uint16 numNonItemCursors = 12;
	uint16 numCurtainAnimationFrames = 7;
	uint32 logoEndAfter = 7000;

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

	// Debug strings
	Common::Array<Common::String> eventFlagNames;

	void readData(Common::SeekableReadStream &stream, Common::Language language, uint32 endPos);
};

} // End of namespace Nancy

#endif // NANCY_COMMONYPES_H
