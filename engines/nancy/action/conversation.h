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

#ifndef NANCY_ACTION_CONVERSATION_H
#define NANCY_ACTION_CONVERSATION_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/video.h"

namespace Nancy {
namespace Action {

// The base class for conversations, with no video data
class ConversationSound : public ActionRecord, public RenderObject {
public:
	ConversationSound() : RenderObject(8) {}
	virtual ~ConversationSound();

	void init() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	virtual bool isVideoDonePlaying() { return true; }

protected:
	struct ConversationFlag {
		byte type;
		FlagDescription flag;
		byte orFlag;

		void read(Common::SeekableReadStream &stream);
		bool isSatisfied() const;
		void set() const;
	};

	struct ConversationFlags {
		Common::Array<ConversationFlag> conditionFlags;

		void read(Common::SeekableReadStream &stream);
		bool isSatisfied() const;
	};

	struct ResponseStruct {
		ConversationFlags conditionFlags;
		Common::String text;
		Common::String soundName;
		SceneChangeDescription sceneChange;
		FlagDescription flagDesc;

		bool isOnScreen = false;
	};

	struct FlagsStruct {
		ConversationFlags conditions;
		ConversationFlag flagToSet;
	};

	struct SceneBranchStruct {
		ConversationFlags conditions;
		SceneChangeDescription sceneChange;
	};

	static const byte kDefaultNextSceneEnabled	= 1;
	static const byte kDefaultNextSceneDisabled	= 2;

	static const byte kPopNextScene				= 1;
	static const byte kNoPopNextScene			= 2;

	Common::String getRecordTypeName() const override { return "ConversationSound"; }
	bool isViewportRelative() const override { return true; }

	// Functions for handling the built-in dialogue responses found in the executable
	void addConditionalDialogue();
	void addGoodbye();

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
	Common::Array<SceneBranchStruct> _sceneBranchStructs;

	bool _hasDrawnTextbox = false;
	int16 _pickedResponse = -1;
};

// Conversation with an AVF video. Originally called PlayPrimaryVideoChan0
class ConversationVideo : public ConversationSound {
public:
	void init() override;
	void updateGraphics() override;
	void onPause(bool pause) override;

	void readData(Common::SeekableReadStream &stream) override;

	bool isVideoDonePlaying() override;

protected:
	Common::String getRecordTypeName() const override;

	Common::String _videoName;
	Common::String _paletteName;
	uint _videoFormat = kLargeVideoFormat;
	uint16 _firstFrame = 0;
	int16 _lastFrame = 0;
	AVFDecoder _decoder;
};

// Conversation with separate cels for the body and head of the character.
// Cels are separate images bundled inside a .cal file
class ConversationCel : public ConversationSound {
public:
	struct Cel {
		Graphics::ManagedSurface bodySurf;
		Common::Rect bodySrc;
		Common::Rect bodyDest;
		Graphics::ManagedSurface headSurf;
		Common::Rect headSrc;
		Common::Rect headDest;
	};

	class HeadCel : public RenderObject {
	public:
		HeadCel() : RenderObject(9) {}
		bool isViewportRelative() const override { return true; }
	};

	ConversationCel() {}

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;

	bool isVideoDonePlaying() override;

	Common::Array<Cel> _cels;
	uint16 _frameTime = 0;
	uint _videoFormat = kLargeVideoFormat;
	uint16 _firstFrame = 0;
	uint16 _lastFrame = 0;

	uint _curFrame = 0; 
	uint32 _nextFrameTime = 0;

	// We use the built-in RenderObject for the body
	HeadCel _headRObj;

protected:
	Common::String getRecordTypeName() const override { return "ConversationCel"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CONVERSATION_H
