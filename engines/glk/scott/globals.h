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

#ifndef GLK_SCOTT_GLOBALS_H
#define GLK_SCOTT_GLOBALS_H

#include "glk/glk_types.h"
#include "glk/windows.h"
#include "glk/scott/definitions.h"
#include "glk/scott/types.h"
#include "common/array.h"
#include "common/str-array.h"

namespace Glk {
namespace Scott {

struct Command;
enum ExtraCommand : int;
struct LineImage;
enum VectorStateType : int;

struct PixelToDraw;
struct Image;

typedef uint8_t RGB[3];
typedef RGB PALETTE[16];

struct Header;
struct Item;
struct Room;
struct Action;

struct SavedState;

class Globals {
public:
	// scott
	Header *_gameHeader;
	Common::Array<Item> _items;
	Common::Array<Room> _rooms;
	Common::StringArray _verbs;
	Common::StringArray _nouns;
	Common::StringArray _messages;
	Common::Array<Action> _actions;
	Common::StringArray _sys;
	Common::StringArray _systemMessages;
	winid_t _graphics = nullptr;
	uint8_t *_entireFile = nullptr;
	size_t _fileLength = 0;
	strid_t _roomDescriptionStream = nullptr;
	int _fileBaselineOffset = 0;
	int _header[24];
	int _lightRefill = 0;
	winid_t _bottomWindow = nullptr, _topWindow = nullptr;
	Command *_currentCommand = nullptr;
	int _justStarted = 1;
	int _shouldRestart = 0;
	int _stopTime = 0;
	strid_t _transcript = nullptr;
	int _counters[16]; ///< Range unknown
	int _currentCounter = 0;
	int _savedRoom = 0;
	int _roomSaved[16]; ///< Range unknown
	uint32 _bitFlags = 0; ///< Might be >32 flags - I haven't seen >32 yet
	int _autoInventory = 0;

	// sagadraw
	int _drawToBuffer = 0;
	uint8_t _sprite[256][8];
	uint8_t _screenchars[768][8];
	uint8_t _buffer[384][9];
	Common::Array<Image> _images;
	int _pixelSize = 0;
	int _xOffset = 0;
	PALETTE _pal;
	int _whiteColour = 15;
	int _blueColour = 9;
	glui32 _diceColour = 0xFF0000;
	int32_t _errorCount = 0;
	PaletteType _palChosen = NO_PALETTE;
	size_t _hulkCoordinates = 0x26DB;
	size_t _hulkItemImageOffsets = 0x2798;
	size_t _hulkLookImageOffsets = 0x27BC;
	size_t _hulkSpecialImageOffsets = 0x276E;
	size_t _hulkImageOffset = 0x441B;

	// line_drawing
	Common::Array<LineImage> _lineImages;
	VectorStateType _vectorState;
	PixelToDraw **_pixelsToDraw = nullptr;
	int _totalDrawInstructions = 0;
	int _currentDrawInstruction = 0;
	int _vectorImageShown = -1;
	uint8_t *_pictureBitmap = nullptr;
	int _lineColour = 15;
	int _bgColour = 0;
	int _scottGraphicsWidth = 255;
	int _scottGraphicsHeight = 94;

	// connect
	int _gliSlowDraw = 0;

	// parser
	int _lastNoun = 0;
	glui32 *_firstErrorMessage = nullptr;
	glui32 **_unicodeWords = nullptr;
	char **_charWords = nullptr;
	int _wordsInInput = 0;
	Common::StringArray _directions;
	Common::StringArray _englishDirections;
	Common::StringArray _skipList;
	Common::StringArray _englishSkipList;
	Common::StringArray _delimiterList;
	Common::StringArray _englishDelimiterList;
	Common::StringArray _extraCommands;
	Common::StringArray _extraNouns;
	Common::StringArray _englishExtraNouns;
	Common::Array<ExtraCommand> _extraNounsKey;
	Common::Array<ExtraCommand> _extraCommandsKey;
	Common::StringArray _abbreviations;
	Common::StringArray _abbreviationsKey;

	//restore state
	int _justUndid = 0;
	SavedState *_initialState = nullptr;
	SavedState *_ramSave = nullptr;
	SavedState *_lastUndo = nullptr;
	SavedState *_oldestUndo = nullptr;
	int _numberOfUndos = 0;

	const GameInfo *_game;
	GameInfo _fallbackGame;
	// Include game list
	#include "glk/scott/games.h"
public:
	Globals();
	~Globals();
};

extern Globals *g_globals;

#define _G(FIELD) (g_globals->FIELD)

} // End of namespace Scott
} // End of namespace Glk

#endif
