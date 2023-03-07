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

#ifndef NANCY_ACTION_PRIMARYVIDEO_H
#define NANCY_ACTION_PRIMARYVIDEO_H

#include "engines/nancy/video.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

class NancyEngine;

namespace Action {

// ActionRecord subclass that handles all NPC dialog and nancy1's intro video
class PlayPrimaryVideoChan0 : public ActionRecord, public RenderObject {

struct ConditionFlag {
	byte type;
	FlagDescription flag;
	byte orFlag;

	void read(Common::SeekableReadStream &stream);
	bool isSatisfied() const;
	void set() const;
};

struct ConditionFlags {
	Common::Array<ConditionFlag> conditionFlags;

	void read(Common::SeekableReadStream &stream);
	bool isSatisfied() const;
};

struct ResponseStruct {
	ConditionFlags conditionFlags; // 0x01
	Common::String text; // 0x06
	Common::String soundName; // 0x196
	SceneChangeDescription sceneChange; // 0x1A0
	FlagDescription flagDesc; // 0x1A8
    
	bool isOnScreen = false;
};

struct FlagsStruct {
	ConditionFlags conditions;
	ConditionFlag flagToSet;
};

public:	
	static const byte kDefaultNextSceneEnabled	= 1;
	static const byte kDefaultNextSceneDisabled	= 2;

	static const byte kPopNextScene				= 1;
	static const byte kNoPopNextScene			= 2;

	PlayPrimaryVideoChan0() : RenderObject(8) {}
	virtual ~PlayPrimaryVideoChan0();

	void init() override;
	void updateGraphics() override;
	void onPause(bool pause) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	// Functions for handling the built-in dialogue responses found in the executable
	void addConditionalDialogue();
	void addGoodbye();

	Common::String _videoName;
	Common::String _paletteName;
	uint _videoFormat = 2;
	Common::String _text;

	SoundDescription _sound;
	SoundDescription _responseGenericSound;

	byte _conditionalResponseCharacterID = 0;
	byte _goodbyeResponseCharacterID = 0;
	byte _defaultNextScene = kDefaultNextSceneEnabled;
	byte _popNextScene = kNoPopNextScene;
	SceneChangeDescription _sceneChange;

	Common::Array<ResponseStruct> _responses;
	Common::Array<FlagsStruct> _flagsStructs;

	AVFDecoder _decoder;

	bool _hasDrawnTextbox = false;
	int16 _pickedResponse = -1;

protected:
	Common::String getRecordTypeName() const override { return "PlayPrimaryVideoChan0"; }
	bool isViewportRelative() const override { return true; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PRIMARYVIDEO_H
