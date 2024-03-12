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
#include "engines/nancy/video.h"

namespace Nancy {
namespace Action {

// The base class for conversations, with no video data. Contains the following:
// - a base sound for the NPC's speech and its caption (mandatory)
// - a list of possible player responses, also with sounds and captions (optional)
// Captions are displayed in the Textbox, and player responses are also selectable there.
// Captions are hypertext; meaning, they contain extra data related to the text (see misc/hypertext.h)
// A conversation will auto-advance to a next scene when no responses are available; the next scene
// can either be described within the Conversation data, or can be whatever's pushed onto the scene "stack".
// Also supports branching scenes depending on a condition, though that is only used in older games.
// Player responses can also be conditional; the original engine had special-purpose "infocheck"
// functions, two per character ID, which were used to evaluate those conditions. We replace that with
// the data bundled inside nancy.dat (see devtools/create_nancy).
class ConversationSound : public RenderActionRecord {
public:
	ConversationSound();
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
		enum AddRule { kAddIfNotFound, kRemoveAndAddToEnd, kRemove };

		ConversationFlags conditionFlags;
		Common::String text;
		Common::String soundName;
		byte addRule = kAddIfNotFound;
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

	// Functions for reading captions are virtual to allow easier support for the terse Conversation variants
	virtual void readCaptionText(Common::SeekableReadStream &stream);
	virtual void readResponseText(Common::SeekableReadStream &stream, ResponseStruct &response);

	// Used in subclasses
	void readTerseData(Common::SeekableReadStream &stream);
	void readTerseCaptionText(Common::SeekableReadStream &stream);
	void readTerseResponseText(Common::SeekableReadStream &stream, ResponseStruct &response);

	// Functions for handling the built-in dialogue responses found in the executable
	void addConditionalDialogue();
	void addGoodbye();

	Common::String _text;

	SoundDescription _sound;
	SoundDescription _responseGenericSound;

	byte _conditionalResponseCharacterID;
	byte _goodbyeResponseCharacterID;
	byte _defaultNextScene = kDefaultNextSceneEnabled;
	byte _popNextScene = kNoPopNextScene;
	SceneChangeDescription _sceneChange;

	Common::Array<ResponseStruct> _responses;
	Common::Array<FlagsStruct> _flagsStructs;
	Common::Array<SceneBranchStruct> _sceneBranchStructs;

	bool _hasDrawnTextbox;
	int16 _pickedResponse;

	const byte _noResponse;
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
	Common::Path _paletteName;
	uint _videoFormat = kLargeVideoFormat;
	uint16 _firstFrame = 0;
	int16 _lastFrame = 0;
	AVFDecoder _decoder;
};

class ConversationCelLoader;

// Conversation with separate cels for the body and head of the character.
// Cels are separate images bundled inside a .cal file
class ConversationCel : public ConversationSound {
	friend class ConversationCelLoader;
public:
	ConversationCel() {}
	virtual ~ConversationCel();

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "ConversationCel"; }

	struct Cel {
		Graphics::ManagedSurface surf;
		Common::Rect src;
		Common::Rect dest;
	};

	class RenderedCel : public RenderObject {
		friend class ConversationCel;
	public:
		RenderedCel() : RenderObject(9) {}
		bool isViewportRelative() const override { return true; }
	};

	static const byte kCelOverrideTreeRectsOff	= 1;
	static const byte kCelOverrideTreeRectsOn	= 2;

	bool isVideoDonePlaying() override;
	Cel &loadCel(const Common::Path &name, const Common::String &treeName);

	void readXSheet(Common::SeekableReadStream &stream, const Common::String &xsheetName);

	Common::Array<Common::Array<Common::Path>> _celNames;
	Common::Array<Common::String> _treeNames;

	uint16 _frameTime = 0;
	uint _videoFormat = kLargeVideoFormat;
	uint16 _firstFrame = 0;
	uint16 _lastFrame = 0;

	Common::Array<byte> _drawingOrder;
	Common::Array<byte> _overrideTreeRects;

	Common::Array<Common::Rect> _overrideRectSrcs;
	Common::Array<Common::Rect> _overrideRectDests;

	uint _curFrame = 0;
	uint32 _nextFrameTime = 0;

	Common::Array<RenderedCel> _celRObjects;

	Common::HashMap<Common::Path, Cel, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> _celCache;
	Common::SharedPtr<ConversationCelLoader> _loaderPtr;
};

// A ConversationSound without embedded text; uses the CONVO chunk instead
class ConversationSoundT : public ConversationSound {
protected:
	Common::String getRecordTypeName() const override { return "ConversationSoundT"; }

	void readCaptionText(Common::SeekableReadStream &stream) override { readTerseCaptionText(stream); }
	void readResponseText(Common::SeekableReadStream &stream, ResponseStruct &response) override { readTerseResponseText(stream, response); }
};

// A ConversationCel without embedded text; uses the CONVO chunk instead
class ConversationCelT : public ConversationCel {
protected:
	Common::String getRecordTypeName() const override { return "ConversationCelT"; }

	void readCaptionText(Common::SeekableReadStream &stream) override { readTerseCaptionText(stream); }
	void readResponseText(Common::SeekableReadStream &stream, ResponseStruct &response) override { readTerseResponseText(stream, response); }
};

// A ConversationSound with a much smaller data footprint
class ConversationSoundTerse : public ConversationSound {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "ConversationSoundTerse"; }
};

// A ConversationCel with a much smaller data footprint
class ConversationCelTerse : public ConversationCel {
public:
	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "ConversationCelTerse"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CONVERSATION_H
