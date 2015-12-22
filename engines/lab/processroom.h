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

enum ActionType {
	kActionPlaySound = 1,
	kActionPlaySoundLooping = 2,
	kActionShowDiff = 3,
	kActionShowDiffLooping = 4,
	kActionLoadDiff = 5,
	kActionLoadBitmap = 6,	// unused?
	kActionShowBitmap = 7,	// unused?
	kActionTransition = 8,
	kActionNoUpdate = 9,
	kActionForceUpdate = 10,
	kActionShowCurPict = 11,
	kActionSetElement = 12,
	kActionUnsetElement = 13,
	kActionShowMessage = 14,
	kActionShowMessages = 15,
	kActionChangeRoom = 16,
	kActionSetCloseup = 17,
	kActionMainView = 18,
	kActionSubInv = 19,
	kActionAddInv = 20,
	kActionShowDir = 21,
	kActionWaitSecs = 22,
	kActionStopMusic = 23,
	kActionStartMusic = 24,
	kActionChangeMusic = 25,
	kActionResetMusic = 26,
	kActionFillMusic = 27,
	kActionWaitSound = 28,
	kActionClearSound = 29,
	kActionWinMusic = 30,
	kActionWinGame = 31,
	kActionLostGame = 32,
	kActionResetBuffer = 33,
	kActionSpecialCmd = 34,
	kActionCShowMessage = 35,
	kActionPlaySoundNoWait = 36
};

enum RuleType {
	kRuleTypeNone = 0,
	kRuleTypeAction = 1,
	kRuleTypeOperate = 2,
	kRuleTypeGoForward = 3,
	kRuleTypeConditions = 4,	// unused?
	kRuleTypeTurn = 5,
	kRuleTypeGoMainView = 6,
	kRuleTypeTurnFromTo = 7
};

enum RuleAction {
	kRuleActionTake = 0,
	kRuleActionMove = 1,	// unused?
	kRuleActionOpenDoor = 2,	// unused?
	kRuleActionCloseDoor = 3,	// unused?
	kRuleActionTakeDef = 4
};

#if defined(WIN32)
#pragma pack(push, 1)
#endif

struct CloseData {
	uint16 _x1, _y1, _x2, _y2;
	int16 _closeUpType;          // if > 0, an object.  If < 0, an item
	uint16 _depth;               // Level of the closeup.
	Common::String _graphicName;
	Common::String _message;
	CloseData *_nextCloseUp;
	CloseData *_subCloseUps;
};

struct ViewData {
	int16 *_condition;
	Common::String _graphicName;
	ViewData *_nextCondition;
	CloseDataPtr _closeUps;
};

struct Action {
	ActionType _actionType;
	int16 _param1;
	int16 _param2;
	int16 _param3;
	Common::String *_messages;
	Action *_nextAction;
};

struct Rule {
	RuleType _ruleType;
	int16 _param1;
	int16 _param2;
	int16 *_condition;
	Action *_actionList;
};

struct RoomData {
	uint16 _doors[4];
	byte _transitionType;
	ViewData *_view[4];
	RuleList *_rules;
	Common::String _roomMsg;
};

struct InventoryData {
	uint16 _quantity;
	Common::String _name;
	Common::String _bitmapName;
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
	uint16 _x, _y, _pageNumber, _specialID;
	uint32 _mapFlags;
};

#if defined(WIN32)
#pragma pack(pop)
#endif

} // End of namespace Lab

#endif // LAB_PROCESSROOM_H
