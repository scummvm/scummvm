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

#ifndef VCRUISE_RUNTIME_H
#define VCRUISE_RUNTIME_H

#include "common/hashmap.h"

#include "vcruise/detection.h"

class OSystem;

namespace Graphics {

struct WinCursorGroup;

} // End of namespace Graphics

namespace VCruise {

class TextParser;
struct ScriptSet;

enum GameState {
	kGameStateBoot,
	kGameStateCinematic,
	kGameStateQuit,
	kGameStateRunning,
};

struct AnimationDef {
	AnimationDef();

	int animNum;	// May be negative if reversed
	uint firstFrame;
	uint lastFrame;	// Inclusive
};

struct RoomDef {
	Common::HashMap<Common::String, AnimationDef> animations;
	Common::HashMap<Common::String, Common::Rect> rects;
	Common::HashMap<Common::String, uint> vars;
	Common::HashMap<Common::String, int> values;
	Common::HashMap<Common::String, Common::String> texts;
	Common::HashMap<Common::String, int> consts;
	Common::HashMap<Common::String, int> sounds;

	Common::String name;
};

class Runtime {
public:
	Runtime(OSystem *system, const Common::FSNode &rootFSNode, VCruiseGameID gameID);
	virtual ~Runtime();

	void loadCursors(const char *exeName);

	bool runFrame();
	void drawFrame();

private:
	bool bootGame();
	bool runGame();

	void loadIndex();
	void changeToScreen(uint roomNumber, uint screenNumber);

	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursors;		// Cursors indexed as CURSOR_CUR_##
	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursorsShort;	// Cursors indexed as CURSOR_#

	OSystem *_system;
	uint _roomNumber;	// Room number can be changed independently of the loaded room, the screen doesn't change until a command changes it
	uint _screenNumber;

	uint _loadedRoomNumber;
	uint _activeScreenNumber;
	bool _havePendingScreenChange;
	GameState _gameState;
	VCruiseGameID _gameID;

	Common::FSNode _rootFSNode;
	Common::FSNode _logDir;

	Common::Array<Common::SharedPtr<RoomDef> > _roomDefs;
	Common::SharedPtr<ScriptSet> _scriptSet;

	enum IndexParseType {
		kIndexParseTypeNone,
		kIndexParseTypeRoom,
		kIndexParseTypeRRoom,	// Rectangle room (constrains animation areas)
		kIndexParseTypeYRoom,	// Yes room (variable/ID mappings)
		kIndexParseTypeVRoom,	// Value room (value/ID mappings?)
		kIndexParseTypeTRoom,	// Text
		kIndexParseTypeCRoom,	// Const
		kIndexParseTypeSRoom,	// Sound
		kIndexParseTypeNameRoom,
	};

	struct IndexPrefixTypePair {
		const char *prefix;
		IndexParseType parseType;
	};

	bool parseIndexDef(TextParser &parser, IndexParseType parseType, uint roomNumber, const Common::String &blamePath);
	void allocateRoomsUpTo(uint roomNumber);
};

} // End of namespace VCruise

#endif
