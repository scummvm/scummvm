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

enum NancyFlag : byte { kFalse = 1, kTrue = 2 };
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
	kCheat,
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
	bool doNotStartSound = false;

	void readData(Common::SeekableReadStream &stream);
};

// Describes a single event flag change or comparison
struct EventFlagDescription {
	int16 label;
	NancyFlag flag;
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

	void readData(Common::SeekableReadStream &stream);
};

// Describes 10 event flag changes to be executed when an action is triggered
struct MultiEventFlagDescription {
	EventFlagDescription descs[10];

	void readData(Common::SeekableReadStream &stream);
	void execute();
};

struct SecondaryVideoDescription {
	int16 frameID = -1;
	Common::Rect srcRect;
	Common::Rect destRect;
	// 2 unknown/empty rects

	void readData(Common::SeekableReadStream &stream);
};

// Descrbes a single sound. Combines four different structs found in the data in one
struct SoundDescription {
	enum Type { kNormal, kMenu, kDIGI, kScene };

	Common::String name;
	uint16 channelID = 0;
	uint16 numLoops = 0;
	uint16 volume = 0;
	uint16 panAnchorFrame = 0;

	void read(Common::SeekableReadStream &stream, Type type);
};

// Structs inside nancy.dat, which contains all the data that was
// originally stored inside the executable

struct ConditionalDialogue {
    byte textID;
    uint16 sceneID;
    Common::String soundID;
	Common::Array<EventFlagDescription> flagConditions;
	Common::Array<EventFlagDescription> inventoryConditions;

	void readData(Common::SeekableReadStream &stream);
};

struct GoodbyeSceneChange {
	Common::Array<uint16> sceneIDs;
	Common::Array<EventFlagDescription> flagConditions;
	EventFlagDescription flagToSet;

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
    Common::Array<EventFlagDescription> flagConditions;
	Common::Array<EventFlagDescription> inventoryConditions;

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

	// In-game strings and related logic
	Common::Array<Common::Array<ConditionalDialogue>> conditionalDialogue;
	Common::Array<Goodbye> goodbyes;
	Common::Array<Common::Array<Hint>> hints;

	Common::Array<Common::String> conditionalDialogueTexts;
	Common::Array<Common::String> goodbyeTexts;
	Common::Array<Common::String> hintTexts;
	Common::String ringingText;

	// Debug strings
	Common::Array<Common::String> itemNames;
	Common::Array<Common::String> eventFlagNames;

	void readData(Common::SeekableReadStream &stream, Common::Language language);
};

} // End of namespace Nancy

#endif // NANCY_COMMONYPES_H
