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

#define NOFLAG { -1, kFalse }

enum NancyFlag : byte { kFalse = 1, kTrue = 2 };

struct GameConstants {
	uint16 numItems;
	uint16 numEventFlags;
	Common::Array<uint16> mapAccessSceneIDs;
	Common::Array<uint16> genericEventFlags;
	uint16 numNonItemCursors;
	uint16 numCurtainAnimationFrames;
	uint32 logoEndAfter;
};

struct EventFlagDescription {
	int16 label;
	NancyFlag flag;
};

struct SceneChangeDescription {
	uint16 sceneID;
	uint16 frameID;
	uint16 verticalOffset;
	bool doNotStartSound;
};

struct ConditionalDialogue {
    byte textID;
    uint16 sceneID;
    const char *soundID;
	Common::Array<EventFlagDescription> flagConditions;
	Common::Array<EventFlagDescription> inventoryConditions;
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
    SceneChangeDescription sceneChange;
    const char *soundIDs[3];
    Common::Array<EventFlagDescription> flagConditions;
	Common::Array<EventFlagDescription> inventoryConditions;
};

#endif // CREATE_NANCY_TYPES_H
