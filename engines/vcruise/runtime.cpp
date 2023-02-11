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

#include "common/formats/winexe.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/stream.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "graphics/managed_surface.h"

#include "vcruise/runtime.h"
#include "vcruise/script.h"
#include "vcruise/textparser.h"


namespace VCruise {

AnimationDef::AnimationDef() : animNum(0), firstFrame(0), lastFrame(0) {
}


Runtime::Runtime(OSystem *system, const Common::FSNode &rootFSNode, VCruiseGameID gameID)
	: _system(system), _roomNumber(1), _screenNumber(0), _loadedRoomNumber(0), _activeScreenNumber(0), _gameState(kGameStateBoot), _gameID(gameID), _rootFSNode(rootFSNode), _havePendingScreenChange(false) {

	_logDir = _rootFSNode.getChild("Log");
	if (!_logDir.exists() || !_logDir.isDirectory())
		error("Couldn't resolve Log directory");
}

Runtime::~Runtime() {
}

void Runtime::loadCursors(const char *exeName) {
	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(exeName));
	if (!winRes)
		error("Couldn't open executable file %s", exeName);

	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			continue;
		}

		Common::String nameStr = id.getString();
		if (nameStr.size() == 8 && nameStr.substr(0, 7) == "CURSOR_") {
			char c = nameStr[7];
			if (c >= '0' && c <= '9') {
				uint shortID = c - '0';
				if (shortID >= _cursorsShort.size())
					_cursorsShort.resize(shortID + 1);
				_cursorsShort[shortID] = cursorGroup;
			}
		} else if (nameStr.size() == 13 && nameStr.substr(0, 11) == "CURSOR_CUR_") {
			char c1 = nameStr[11];
			char c2 = nameStr[12];
			if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
				uint longID = (c1 - '0') * 10 + (c2 - '0');
				if (longID >= _cursors.size())
					_cursors.resize(longID + 1);
				_cursors[longID] = cursorGroup;
			}
		}
	}
}

bool Runtime::runFrame() {
	bool moreActions = true;
	while (moreActions) {
		moreActions = false;
		switch (_gameState) {
		case kGameStateBoot:
			moreActions = bootGame();
			break;
		case kGameStateQuit:
			return false;
		case kGameStateRunning:
			moreActions = runGame();
			break;
		default:
			error("Unknown game state");
			return false;
		}
	}

	return true;
}

bool Runtime::bootGame() {
	debug(1, "Booting V-Cruise game...");
	loadIndex();
	debug(1, "Index loaded OK");

	_gameState = kGameStateRunning;

	if (_gameID == GID_REAH) {
		// TODO: Change to the logo instead (0xb1) instead when menus are implemented
		_roomNumber = 1;
		_screenNumber = 0xb0;
		_havePendingScreenChange = true;
	} else
		error("Couldn't figure out what screen to start on");

	return true;
}

bool Runtime::runGame() {

	if (_havePendingScreenChange) {
		_havePendingScreenChange = false;

		changeToScreen(_roomNumber, _screenNumber);
		return true;
	}

	return false;
}

void Runtime::loadIndex() {
	Common::FSNode indexFSNode = _logDir.getChild("Index.txt");

	Common::ReadStream *stream = indexFSNode.createReadStream();
	if (!stream)
		error("Failed to open main index");

	Common::String blamePath = indexFSNode.getPath();

	TextParser parser(stream);

	Common::String token;
	TextParserState state;

	static const IndexPrefixTypePair parsePrefixes[] = {
		{"Room", kIndexParseTypeRoom},
		{"RRoom", kIndexParseTypeRRoom},
		{"YRoom", kIndexParseTypeYRoom},
		{"VRoom", kIndexParseTypeVRoom},
		{"TRoom", kIndexParseTypeTRoom},
		{"CRoom", kIndexParseTypeCRoom},
		{"SRoom", kIndexParseTypeSRoom},
	};

	IndexParseType indexParseType = kIndexParseTypeNone;
	uint currentRoomNumber = 0;

	for (;;) {
		char firstCh = 0;
		if (!parser.skipWhitespaceAndComments(firstCh, state))
			break;

		if (firstCh == '[') {
			if (!parser.parseToken(token, state))
				error("Index open bracket wasn't terminated");

			if (token == "NameRoom") {
				indexParseType = kIndexParseTypeNameRoom;
			} else {
				bool foundType = false;
				uint prefixLen = 0;
				for (const IndexPrefixTypePair &prefixTypePair : parsePrefixes) {
					uint len = strlen(prefixTypePair.prefix);
					if (token.size() > len && !memcmp(token.c_str(), prefixTypePair.prefix, len)) {
						indexParseType = prefixTypePair.parseType;
						foundType = true;
						prefixLen = len;
						break;
					}
				}

				if (!foundType)
					error("Unknown index heading type %s", token.c_str());

				currentRoomNumber = 0;
				for (uint i = prefixLen; i < token.size(); i++) {
					char digit = token[i];
					if (digit < '0' || digit > '9')
						error("Malformed room def");
					currentRoomNumber = currentRoomNumber * 10 + (token[i] - '0');
				}
			}

			parser.expect("]", blamePath);

			allocateRoomsUpTo(currentRoomNumber);
		} else {
			parser.requeue(&firstCh, 1, state);

			if (!parseIndexDef(parser, indexParseType, currentRoomNumber, blamePath))
				break;
		}
	}
}

void Runtime::changeToScreen(uint roomNumber, uint screenNumber) {
	bool changedRoom = (roomNumber != _loadedRoomNumber);
	bool changedScreen = (screenNumber != _activeScreenNumber) || changedRoom;

	if (changedRoom) {
		Common::String logFileName = Common::String::format("Room%02i.log", static_cast<int>(roomNumber));

		Common::FSNode logFileNode = _logDir.getChild(logFileName);
		if (Common::SeekableReadStream *logicFile = logFileNode.createReadStream())
			_scriptSet = compileLogicFile(*logicFile, static_cast<uint>(logicFile->size()), logFileNode.getPath());
		else
			_scriptSet.reset();
	}
}

bool Runtime::parseIndexDef(TextParser &parser, IndexParseType parseType, uint roomNumber, const Common::String &blamePath) {
	Common::String lineText;
	parser.expectLine(lineText, blamePath, true);

	Common::MemoryReadStream lineStream(reinterpret_cast<const byte *>(lineText.c_str()), lineText.size(), DisposeAfterUse::NO);
	TextParser strParser(&lineStream);

	switch (parseType) {
	case kIndexParseTypeNameRoom: {
			uint nameRoomNumber = 0;
			Common::String name;
			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectUInt(nameRoomNumber, blamePath);

			allocateRoomsUpTo(nameRoomNumber);
			_roomDefs[nameRoomNumber]->name = name;
		} break;
	case kIndexParseTypeRoom: {
			Common::String name;

			AnimationDef animDef;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(animDef.animNum, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectUInt(animDef.firstFrame, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectUInt(animDef.lastFrame, blamePath);
			_roomDefs[roomNumber]->animations[name] = animDef;
		} break;
	case kIndexParseTypeRRoom: {
			Common::String name;

			Common::Rect rect;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectShort(rect.left, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectShort(rect.top, blamePath);
			strParser.expect(",", blamePath);
			strParser.expectShort(rect.right, blamePath);

			// Line 4210 in Reah contains an animation def instead of a rect def, detect this and discard
			if (!strParser.checkEOL()) {
				strParser.expect(",", blamePath);
				strParser.expectShort(rect.bottom, blamePath);

				_roomDefs[roomNumber]->rects[name] = rect;
			}

		} break;
	case kIndexParseTypeYRoom: {
			Common::String name;

			uint varSlot = 0;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectUInt(varSlot, blamePath);

			_roomDefs[roomNumber]->vars[name] = varSlot;
		} break;
	case kIndexParseTypeVRoom: {
			Common::String name;

			int value = 0;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->values[name] = value;
		} break;
	case kIndexParseTypeTRoom: {
			Common::String name;
			Common::String value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectLine(value, blamePath, false);

			_roomDefs[roomNumber]->texts[name] = value;
		} break;
	case kIndexParseTypeCRoom: {
			Common::String name;
			int value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->consts[name] = value;
		} break;
	case kIndexParseTypeSRoom: {
			Common::String name;
			int value;

			strParser.expectToken(name, blamePath);
			strParser.expect("=", blamePath);
			strParser.expectInt(value, blamePath);

			_roomDefs[roomNumber]->sounds[name] = value;
		} break;
	default:
		assert(false);
		return false;
	}

	return true;
}

void Runtime::allocateRoomsUpTo(uint roomNumber) {
	while (_roomDefs.size() <= roomNumber) {
		_roomDefs.push_back(Common::SharedPtr<RoomDef>(new RoomDef()));
	}
}

void Runtime::drawFrame() {

	_system->updateScreen();
}

} // End of namespace VCruise
