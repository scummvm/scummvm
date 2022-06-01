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

#ifndef GLK_SCOTT_DEFINITIONS_H
#define GLK_SCOTT_DEFINITIONS_H

#include "common/str.h"

namespace Glk {
namespace Scott {

const int FOLLOWS = 0xFFFF;

const int GO = 1;
const int TAKE = 10;
const int DROP = 18;

const int LASTALL = 128;

enum GameIDType {
	UNKNOWN_GAME,
	SCOTTFREE,
	TI994A,
	PIRATE,
	VOODOO,
	STRANGE,
	BANZAI,
	BATON,
	BATON_C64,
	TIME_MACHINE,
	TIME_MACHINE_C64,
	ARROW1,
	ARROW1_C64,
	ARROW2,
	ARROW2_C64,
	PULSAR7,
	PULSAR7_C64,
	CIRCUS,
	CIRCUS_C64,
	FEASIBILITY,
	FEASIBILITY_C64,
	AKYRZ,
	AKYRZ_C64,
	PERSEUS,
	PERSEUS_C64,
	PERSEUS_ITALIAN,
	INDIANS,
	INDIANS_C64,
	WAXWORKS,
	WAXWORKS_C64,
	HULK,
	HULK_C64,
	ADVENTURELAND,
	ADVENTURELAND_C64,
	SECRET_MISSION,
	SECRET_MISSION_C64,
	CLAYMORGUE,
	CLAYMORGUE_C64,
	SPIDERMAN,
	SPIDERMAN_C64,
	SAVAGE_ISLAND,
	SAVAGE_ISLAND_C64,
	SAVAGE_ISLAND2,
	SAVAGE_ISLAND2_C64,
	GREMLINS,
	GREMLINS_ALT,
	GREMLINS_C64,
	GREMLINS_GERMAN,
	GREMLINS_GERMAN_C64,
	GREMLINS_SPANISH,
	SUPERGRAN,
	SUPERGRAN_C64,
	ROBIN_OF_SHERWOOD,
	ROBIN_OF_SHERWOOD_C64,
	SEAS_OF_BLOOD,
	SEAS_OF_BLOOD_C64,
	NUMGAMES
};

enum ExplicitResultType {
	ER_NO_RESULT,
	ER_SUCCESS = 0,
	ER_RAN_ALL_LINES_NO_MATCH = -1,
	ER_RAN_ALL_LINES = -2
};

enum ActionResultType {
	ACT_SUCCESS = 0,
	ACT_FAILURE = 1,
	ACT_CONTINUE,
	ACT_GAMEOVER
};

enum SysMessageType {
	NORTH,
	SOUTH,
	EAST,
	WEST,
	UP,
	DOWN,
	PLAY_AGAIN,
	IVE_STORED,
	TREASURES,
	ON_A_SCALE_THAT_RATES,
	DROPPED,
	TAKEN,
	OK,
	YOUVE_SOLVED_IT,
	I_DONT_UNDERSTAND,
	YOU_CANT_DO_THAT_YET,
	HUH,
	DIRECTION,
	YOU_HAVENT_GOT_IT,
	YOU_HAVE_IT,
	YOU_DONT_SEE_IT,
	THATS_BEYOND_MY_POWER,
	DANGEROUS_TO_MOVE_IN_DARK,
	YOU_FELL_AND_BROKE_YOUR_NECK,
	YOU_CANT_GO_THAT_WAY,
	I_DONT_KNOW_HOW_TO,
	SOMETHING,
	I_DONT_KNOW_WHAT_A,
	IS,
	TOO_DARK_TO_SEE,
	YOU_ARE,
	YOU_SEE,
	EXITS,
	INVENTORY,
	NOTHING,
	WHAT_NOW,
	HIT_ENTER,
	LIGHT_HAS_RUN_OUT,
	LIGHT_RUNS_OUT_IN,
	TURNS,
	YOURE_CARRYING_TOO_MUCH,
	IM_DEAD,
	RESUME_A_SAVED_GAME,
	NONE,
	NOTHING_HERE_TO_TAKE,
	YOU_HAVE_NOTHING,
	LIGHT_GROWING_DIM,
	EXITS_DELIMITER,
	MESSAGE_DELIMITER,
	ITEM_DELIMITER,
	WHAT,
	YES,
	NO,
	ANSWER_YES_OR_NO,
	ARE_YOU_SURE,
	MOVE_UNDONE,
	CANT_UNDO_ON_FIRST_TURN,
	NO_UNDO_STATES,
	SAVED,
	CANT_USE_ALL,
	TRANSCRIPT_OFF,
	TRANSCRIPT_ON,
	NO_TRANSCRIPT,
	TRANSCRIPT_ALREADY,
	FAILED_TRANSCRIPT,
	TRANSCRIPT_START,
	TRANSCRIPT_END,
	BAD_DATA,
	STATE_SAVED,
	STATE_RESTORED,
	NO_SAVED_STATE,
	LAST_SYSTEM_MESSAGE
};

const SysMessageType MAX_SYSMESS = LAST_SYSTEM_MESSAGE;

enum DictionaryType {
	NOT_A_GAME,
	FOUR_LETTER_UNCOMPRESSED,
	THREE_LETTER_UNCOMPRESSED,
	FIVE_LETTER_UNCOMPRESSED,
	FOUR_LETTER_COMPRESSED,
	FIVE_LETTER_COMPRESSED,
	GERMAN,
	SPANISH,
	ITALIAN
};

enum GameType {
	NO_TYPE,
	GREMLINS_VARIANT,
	SHERWOOD_VARIANT,
	SAVAGE_ISLAND_VARIANT,
	SECRET_MISSION_VARIANT,
	SEAS_OF_BLOOD_VARIANT,
	OLD_STYLE,
};

enum Subtype {
	ENGLISH = 0x1,
	MYSTERIOUS = 0x2,
	LOCALIZED = 0x4,
	C64 = 0x8
};

enum PaletteType {
	NO_PALETTE,
	ZX,
	ZXOPT,
	C64A,
	C64B,
	VGA
};

enum HeaderType {
	NO_HEADER,
	EARLY,
	LATE,
	HULK_HEADER,
	GREMLINS_C64_HEADER,
	ROBIN_C64_HEADER,
	SUPERGRAN_C64_HEADER,
	SEAS_OF_BLOOD_C64_HEADER,
	MYSTERIOUS_C64_HEADER,
	ARROW_OF_DEATH_PT_2_C64_HEADER,
	INDIANS_C64_HEADER
};

enum ActionTableType {
	UNKNOWN_ACTIONS_TYPE,
	COMPRESSED,
	UNCOMPRESSED,
	HULK_ACTIONS
};

struct GameInfo {
	GameInfo();
	GameInfo(const char *title, GameIDType gameID, GameType type, Subtype subType, DictionaryType dictionary,
			 int numberOfItems, int numberOfActions, int numberOfWords, int numberOfRooms, int maxCarried,
			 int wordLength, int numberOfMessages, int numberOfVerbs, int numberOfNouns, int startOfHeader,
			 HeaderType headerStyle, int startOfRoomImageList, int startOfItemFlags, int startOfItemImageList,
			 int startOfActions, ActionTableType actionsStyle, int startOfDictionary, int startOfRoomDescriptions,
			 int startOfRoomConnections, int startOfMessages, int startOfItemDescriptions, int startOfItemLocations,
			 int startOfSystemMessages, int startOfDirections, int startOfCharacters, int startOfImageData,
			 int imageAddressOffset, int numberOfPictures, PaletteType palette, int pictureFormatVersion,
			 int startOfIntroText);
	const char *_title;

	GameIDType _gameID = UNKNOWN_GAME;
	GameType _type = NO_TYPE;
	Subtype _subType = ENGLISH;
	DictionaryType _dictionary = NOT_A_GAME;

	int _numberOfItems = 0;
	int _numberOfActions = 0;
	int _numberOfWords = 0;
	int _numberOfRooms = 0;
	int _maxCarried = 0;
	int _wordLength = 0;
	int _numberOfMessages = 0;

	int _numberOfVerbs = 0;
	int _numberOfNouns = 0;

	int _startOfHeader = 0;
	HeaderType _headerStyle = NO_HEADER;

	int _startOfRoomImageList = 0;
	int _startOfItemFlags = 0;
	int _startOfItemImageList = 0;

	int _startOfActions = 0;
	ActionTableType _actionsStyle = UNKNOWN_ACTIONS_TYPE;
	int _startOfDictionary = 0;
	int _startOfRoomDescriptions = 0;
	int _startOfRoomConnections = 0;
	int _startOfMessages = 0;
	int _startOfItemDescriptions = 0;
	int _startOfItemLocations = 0;

	int _startOfSystemMessages = 0;
	int _startOfDirections = 0;

	int _startOfCharacters = 0;
	int _startOfImageData = 0;
	int _imageAddressOffset = 0; /* This is the difference between the value given by
							   the image data lookup table and a usable file
							   offset */
	int _numberOfPictures = 0;
	PaletteType _palette = NO_PALETTE;
	int _pictureFormatVersion = 0;
	int _startOfIntroText = 0;
};

} // End of namespace Scott
} // End of namespace Glk
#endif /* definitions_h */
