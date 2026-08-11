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

#ifndef CREATE_NANCY_TYPES_H
#define CREATE_NANCY_TYPES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/language.h"

#define NOFLAG { kEv, -1, kFalse }

enum NancyFlag : byte { kFalse = 1, kTrue = 2 };
enum ConditionType : byte { kEv = 0, kIn = 1, kDi = 2 };
enum GameLanguage : byte { kEnglish = 0, kRussian = 1, kGerman = 2, kFrench = 3 };

struct GameConstants {
	uint16 numItems;
	uint16 numEventFlags;
	Common::Array<uint16> genericEventFlags;
	uint16 numCursorTypes;
	uint32 logoEndAfter;
	uint16 wonGameFlagID;
};

struct EventFlagDescription {
	byte type;
	int16 label;
	byte flag; // NancyFlag up to nancy2, bool from nancy3 up
};

struct SceneChangeDescription {
	uint16 sceneID;
	uint16 frameID;
	uint16 verticalOffset;
	bool doNotStartSound;
};

// Note: in nancy6 and above, the textID field is ignored since all dialogue strings are bundled
// inside the CONVO file's CVTX chunk (thus nancy.dat doesn't include any).
// Instead, the soundID doubles as the key for the HashMap containing the CONVO data.
struct ConditionalDialogue {
	byte textID;
	uint16 sceneID;
	const char *soundID;
	Common::Array<EventFlagDescription> conditions;
};

struct GoodbyeSceneChange {
	Common::Array<uint16> sceneIDs;
	Common::Array<EventFlagDescription> flagConditions;
	EventFlagDescription flagToSet;
};

struct Goodbye {
	const char *soundID;
	Common::Array<GoodbyeSceneChange> sceneChanges;
};

struct Hint {
	byte textID;
	int16 hintWeight;
	const char *soundIDs[3];
	Common::Array<EventFlagDescription> conditions;
};

struct SoundChannelInfo {
	byte numChannels;
	byte numSceneSpecificChannels;
	Common::Array<byte> speechChannels; // 0 in the original engine
	Common::Array<byte> musicChannels; // 1
	Common::Array<byte> sfxChannels; // 2
};

struct PatchAssociation {
	Common::Array<const char *> confManProps;
	Common::Array<const char *> fileIDs;
};

#endif // CREATE_NANCY_TYPES_H
