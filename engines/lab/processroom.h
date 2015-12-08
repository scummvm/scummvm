/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_PROCESSROOM_H
#define LAB_PROCESSROOM_H

namespace Lab {

//------------------------------- Action types ------------------------------
#define PLAYSOUND     1
#define PLAYSOUNDCONT 2
#define SHOWDIFF      3
#define SHOWDIFFCONT  4
#define LOADDIFF      5
#define LOADBM        6
#define SHOWBM        7
#define TRANSITION    8
#define NOUPDATE      9
#define FORCEUPDATE  10
#define SHOWCURPICT  11
#define SETELEMENT   12
#define UNSETELEMENT 13
#define SHOWMESSAGE  14
#define SHOWMESSAGES 15
#define SETPOSITION  16
#define SETCLOSEUP   17
#define MAINVIEW     18
#define SUBINV       19
#define ADDINV       20
#define SHOWDIR      21
#define WAITSECS     22
#define STOPMUSIC    23
#define STARTMUSIC   24
#define CHANGEMUSIC  25
#define RESETMUSIC   26
#define FILLMUSIC    27
#define WAITSOUND    28
#define CLEARSOUND   29
#define WINMUSIC     30
#define WINGAME      31
#define LOSTGAME     32
#define RESETBUFFER  33
#define SPECIALCMD   34
#define CSHOWMESSAGE 35
#define PLAYSOUNDB   36

// Rule Types
#define ACTION      1
#define OPERATE     2
#define GOFORWARD   3
#define CONDITIONS  4
#define TURN        5
#define GOMAINVIEW  6
#define TURNFROMTO  7

//----------------------------- Rule Type Action ----------------------------
#define TAKE        1
#define MOVE        2
#define OPENDOOR    3
#define CLOSEDOOR   4
#define TAKEDEF     5

#if defined(WIN32)
#pragma pack(push, 1)
#endif

typedef struct CloseData {
	uint16 x1, y1, x2, y2;
	int16 _closeUpType;          // if > 0, an object.  If < 0, an item
	uint16 _depth;               // Level of the closeup.
	char *_graphicName;
	char *_message;
	CloseData *_nextCloseUp;
	CloseData *_subCloseUps;
} CloseData;

typedef CloseData *CloseDataPtr;

struct ViewData {
	int16 *_condition;
	char *_graphicName;
	struct ViewData *_nextCondition;
	CloseDataPtr _closeUps;
};

struct Action {
	int16 _actionType;
	int16 _param1;
	int16 _param2;
	int16 _param3;
	byte *_data;            // Message, or a pointer to array of messages.
	Action *_nextAction;
};

struct Rule {
	int16 _ruleType;
	int16 _param1;
	int16 _param2;
	int16 *_condition;
	Action *_actionList;
	Rule *_nextRule;
};

typedef Common::List<Rule *> RuleList;

struct RoomData {
	uint16 _northDoor;
	uint16 _southDoor;
	uint16 _eastDoor;
	uint16 _westDoor;
	byte _transitionType;
	ViewData *_view[4];
	RuleList *_rules;
	char *_roomMsg;
};

struct InventoryData {
	uint16 _many;
	char *_name;
	char *_bitmapName;
};

// Map Flags

// Where the doors are; in a corridor, assumed to be left doors
#define     NORTHDOOR        1L
#define     EASTDOOR         2L
#define     SOUTHDOOR        4L
#define     WESTDOOR         8L

// Where the doors are in corridors; M means middle, R means right, B means bottom
#define     NORTHMDOOR      16L
#define     NORTHRDOOR      32L
#define     SOUTHMDOOR      64L
#define     SOUTHRDOOR     128L

#define     EASTMDOOR       16L
#define     EASTBDOOR       32L
#define     WESTMDOOR       64L
#define     WESTBDOOR      128L

// Special Map ID's
#define     NORMAL           0
#define     UPARROWROOM      1
#define     DOWNARROWROOM    2
#define     BRIDGEROOM       3
#define     VCORRIDOR        4
#define     HCORRIDOR        5
#define     MEDMAZE          6
#define     HEDGEMAZE        7
#define     SURMAZE          8
#define     MULTIMAZEF1      9
#define     MULTIMAZEF2     10
#define     MULTIMAZEF3     11

struct MapData {
	uint16 x, y, PageNumber, SpecialID;
	uint32 MapFlags;
};

#if defined(WIN32)
#pragma pack(pop)
#endif

bool parse(const char *inputFile);
ViewData *getViewData(uint16 roomNum, uint16 direction);
void drawDirection(CloseDataPtr lcptr);
uint16 processArrow(uint16 curDirection, uint16 arrow);
void setCurClose(Common::Point pos, CloseDataPtr *cptr, bool useAbsoluteCoords = false);
bool takeItem(uint16 x, uint16 y, CloseDataPtr *cptr);
bool doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *lcptr);
bool doOperateRule(int16 x, int16 y, int16 itemNum, CloseDataPtr *lcptr);
bool doGoForward(CloseDataPtr *lcptr);
bool doTurn(uint16 from, uint16 to, CloseDataPtr *lcptr);
bool doMainView(CloseDataPtr *lcptr);

} // End of namespace Lab

#endif // LAB_PROCESSROOM_H
