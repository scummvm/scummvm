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

#ifndef TITANIC_MESSAGES_H
#define TITANIC_MESSAGES_H

#include "common/keyboard.h"
#include "titanic/core/saveable_object.h"
#include "titanic/core/tree_item.h"
#include "titanic/support/strings.h"

namespace Titanic {

enum MessageFlag {
	MSGFLAG_SCAN = 1,
	MSGFLAG_BREAK_IF_HANDLED = 2,
	MSGFLAG_CLASS_DEF = 4
};

#define MESSAGE0(NAME) \
	class NAME: public CMessage { \
	public: NAME() : CMessage() {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); } \
}
#define MESSAGE1(NAME, F1, N1, V1) \
	class NAME: public CMessage { \
	public: F1 _##N1; \
	NAME() : CMessage(), _##N1(V1) {} \
	NAME(F1 N1) : CMessage(), _##N1(N1) {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); } \
}
#define MESSAGE2(NAME, F1, N1, V1, F2, N2, V2) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2) {} \
	NAME(F1 N1, F2 N2) : CMessage(), _##N1(N1), _##N2(N2) {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); } \
}
#define MESSAGE3(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2), _##N3(V3) {} \
	NAME(F1 N1, F2 N2, F3 N3) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3) {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); } \
}
#define MESSAGE4(NAME, F1, N1, V1, F2, N2, V2, F3, N3, V3, F4, N4, V4) \
	class NAME: public CMessage { \
	public: F1 _##N1; F2 _##N2; F3 _##N3; F4 _##N4; \
	NAME() : CMessage(), _##N1(V1), _##N2(V2), _##N3(V3), _##N4(V4) {} \
	NAME(F1 N1, F2 N2, F3 N3, F4 N4) : CMessage(), _##N1(N1), _##N2(N2), _##N3(N3), _##N4(N4) {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); } \
}

class CCarry;
class CCharacter;
class CGameObject;
class CRoomItem;
class CNodeItem;
class CViewItem;
class CMusicPlayer;
class CMovePlayerTo;

class CMessage : public CSaveableObject {
private:
	/**
	 * Find a map entry that supports the given class
	 */
	static const MSGMAP_ENTRY *findMapEntry(const CTreeItem *treeItem, const ClassDef *classDef);
public:
	CLASSDEF;
	CMessage();

	/**
	 * Executes the message, passing it on to the designated target,
	 * and optionally it's children
	 */
	bool execute(CTreeItem *target, const ClassDef *classDef = nullptr,
		int flags = MSGFLAG_SCAN | MSGFLAG_BREAK_IF_HANDLED);

	/**
	 * Executes the message, passing it on to the designated target,
	 * and optionally it's children
	 */
	bool execute(const CString &target, const ClassDef *classDef = nullptr,
		int flags = MSGFLAG_SCAN | MSGFLAG_BREAK_IF_HANDLED);

	/**
	 * Makes the passed item execute the message
	 */
	virtual bool perform(CTreeItem *treeItem);

	/**
	 * Returns true if the passed item supports the specified message class
	 */
	static bool supports(const CTreeItem *treeItem, ClassDef *classDef);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	virtual bool isMouseMsg() const;
	virtual bool isButtonDownMsg() const;
	virtual bool isButtonUpMsg() const;
	virtual bool isMouseMoveMsg() const;
	virtual bool isDoubleClickMsg() const;
	virtual bool isEnterRoomMsg() const;
	virtual bool isPreEnterRoomMsg() const;
	virtual bool isleaveRoomMsg() const;
	virtual bool isEnterNodeMsg() const;
	virtual bool isPreEnterNodeMsg() const;
	virtual bool isLeaveNodeMsg() const;
	virtual bool isEnterViewMsg() const;
	virtual bool isPreEnterViewMsg() const;
	virtual bool isLeaveViewMsg() const;
};

enum EditControlAction {
	EDIT_INIT = 0,
	EDIT_CLEAR = 1,
	EDIT_SET_TEXT = 2,
	EDIT_GET_TEXT = 3,
	EDIT_LENGTH = 4,
	EDIT_MAX_LENGTH = 5,
	EDIT_KEYPRESS = 6,
	EDIT_SET_FONT = 7,
	EDIT_SHOW_CURSOR = 8,
	EDIT_HIDE_CURSOR = 9,
	EDIT_BORDERS = 10,
	EDIT_SET_COLOR = 11,
	EDIT_SHOW = 12,
	EDIT_HIDE = 13,
	EDIT_RENDER = 14
};

class CEditControlMsg : public CMessage {
public:
	EditControlAction _mode;
	int _param;
	CString _text;
	byte _textR;
	byte _textG;
	byte _textB;
public:
	CLASSDEF;
	CEditControlMsg() : _mode(EDIT_INIT), _param(0), _textR(0), _textG(0), _textB(0) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return CMessage::supports(item, _type);
	}
};

MESSAGE1(CTimeMsg, uint, _ticks, 0);

class CTimerMsg : public CTimeMsg {
public:
	uint _timerCtr;
	int _actionVal;
	CString _action;
public:
	CLASSDEF;
	CTimerMsg() : CTimeMsg(), _timerCtr(0), _actionVal(0) {}
	CTimerMsg(uint ticks, uint timerCtr, int actionVal, const CString &action) :
		CTimeMsg(ticks), _timerCtr(timerCtr), _actionVal(actionVal), _action(action) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

class CShowTextMsg : public CMessage {
public:
	CString _message;
public:
	CLASSDEF;
	CShowTextMsg();
	CShowTextMsg(const CString &msg);
	CShowTextMsg(StringId stringId);

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}
};

enum MissiveOMatAction {
	MESSAGE_NONE = 1, MESSAGE_SHOW = 2, NEXT_MESSAGE = 3, PRIOR_MESSAGE = 4,
	MESSAGE_5 = 5, MESSAGE_DOWN = 6, MESSAGE_UP = 7, REDRAW_MESSAGE = 8,
	MESSAGE_STARTUP = 9
};

enum Movement {
	MOVE_NONE = 0, MOVE_FORWARDS, MOVE_BACKWARDS, TURN_LEFT, TURN_RIGHT
};

enum ChangeMusicAction {
	MUSIC_NONE = 0, MUSIC_STOP = 1, MUSIC_START = 2
};

class CMovementMsg : public CMessage {
public:
	Movement _movement;
	Point _posToUse;
public:
	CLASSDEF;
	CMovementMsg() : _movement(MOVE_NONE) {}
	CMovementMsg(Movement move) : _movement(move) {}
	CMovementMsg(Common::KeyCode key) :
		_movement(getMovement(key)) {}

	static bool isSupportedBy(const CTreeItem *item) {
		return supports(item, _type);
	}

	/**
	 * Returns the movement associated with a given key, if any
	 */
	static Movement getMovement(Common::KeyCode keycode);
};


MESSAGE1(CActMsg, CString, action, "");
MESSAGE1(CActivationmsg, CString, value, "");
MESSAGE1(CAddHeadPieceMsg, CString, value, "NULL");
MESSAGE1(CAnimateMaitreDMsg, int, value, 0);
MESSAGE1(CArboretumGateMsg, int, value, 0);
MESSAGE0(CArmPickedUpFromTableMsg);
MESSAGE0(CBodyInBilgeRoomMsg);
MESSAGE1(CBowlStateChangeMsg, int, state, 0);
MESSAGE2(CCarryObjectArrivedMsg, CString, strValue, "", int, numValue, 0);
MESSAGE2(CChangeMusicMsg, CString, filename, "", ChangeMusicAction, action, MUSIC_NONE);
MESSAGE1(CChangeSeasonMsg, CString, season, "Summer");
MESSAGE0(CCheckAllPossibleCodes);
MESSAGE2(CCheckChevCode, int, classNum, 0, uint, chevCode, 0);
MESSAGE1(CChildDragEndMsg, int, value, 0);
MESSAGE0(CClearChevPanelBits);
MESSAGE0(CCorrectMusicPlayedMsg);
MESSAGE0(CCreateMusicPlayerMsg);
MESSAGE0(CCylinderHolderReadyMsg);
MESSAGE0(CDeactivationMsg);
MESSAGE1(CDeliverCCarryMsg, CString, value, "");
MESSAGE0(CDisableMaitreDProdReceptor);
MESSAGE0(CDismissBotMsg);
MESSAGE0(CDoffNavHelmet);
MESSAGE0(CDonNavHelmet);
MESSAGE1(CDoorbotNeededInElevatorMsg, int, value, 0);
MESSAGE0(CDoorbotNeededInHomeMsg);
MESSAGE1(CDropObjectMsg, CCarry *, item, nullptr);
MESSAGE1(CDropZoneGotObjectMsg, CGameObject *, object, nullptr);
MESSAGE1(CDropZoneLostObjectMsg, CGameObject *, object, nullptr);
MESSAGE1(CEjectCylinderMsg, int, value, 0);
MESSAGE2(CPreEnterNodeMsg, CNodeItem *, oldNode, nullptr, CNodeItem *, newNode, nullptr);
MESSAGE2(CPreEnterRoomMsg, CRoomItem *, oldRoom, nullptr, CRoomItem *, newRoom, nullptr);
MESSAGE2(CPreEnterViewMsg, CViewItem *, oldView, nullptr, CViewItem *, newView, nullptr);
MESSAGE2(CEnterNodeMsg, CNodeItem *, oldNode, nullptr, CNodeItem *, newNode, nullptr);
MESSAGE2(CEnterRoomMsg, CRoomItem *, oldRoom, nullptr, CRoomItem *, newRoom, nullptr);
MESSAGE2(CEnterViewMsg, CViewItem *, oldView, nullptr, CViewItem *, newView, nullptr);
MESSAGE0(CErasePhonographCylinderMsg);
MESSAGE1(CFrameMsg, uint, ticks, 0);
MESSAGE2(CFreshenCookieMsg, int, value1, 0, int, value2, 1);
MESSAGE1(CGetChevClassBits, int, classBits, 0);
MESSAGE1(CGetChevClassNum, int, classNum, 0);
MESSAGE2(CGetChevCodeFromRoomNameMsg, CString, roomName, "", uint, chevCode, 0);
MESSAGE1(CGetChevFloorBits, int, floorBits, 0);
MESSAGE1(CGetChevFloorNum, int, floorNum, 0);
MESSAGE1(CGetChevLiftBits, int, liftBits, 0);
MESSAGE1(CGetChevLiftNum, int, liftNum, 0);
MESSAGE1(CGetChevRoomBits, int, roomNum, 0);
MESSAGE1(CGetChevRoomNum, int, roomNum, 0);
MESSAGE2(CHoseConnectedMsg, bool, connected, true, CGameObject *, object, nullptr);
MESSAGE0(CInitializeAnimMsg);
MESSAGE1(CIsEarBowlPuzzleDone, int, value, 0);
MESSAGE3(CIsHookedOnMsg, Rect, rect, Rect(), bool, isHooked, false, CString, armName, "");
MESSAGE1(CIsParrotPresentMsg, bool, isPresent, false);
MESSAGE1(CKeyCharMsg, int, key, 32);
MESSAGE2(CLeaveNodeMsg, CNodeItem *, oldNode, nullptr, CNodeItem *, newNode, nullptr);
MESSAGE2(CLeaveRoomMsg, CRoomItem *, oldRoom, nullptr, CRoomItem *, newRoom, nullptr);
MESSAGE2(CLeaveViewMsg, CViewItem *, oldView, nullptr, CViewItem *, newView, nullptr);
MESSAGE1(CLemonFallsFromTreeMsg, Point, pt, Point());
MESSAGE4(CLightsMsg, bool, topRight, false, bool, topLeft, false, bool, bottomLeft, false, bool, bottomRight, false);
MESSAGE1(CLoadSuccessMsg, int, ticks, 0);
MESSAGE1(CLockPhonographMsg, int, value, 0);
MESSAGE0(CMaitreDDefeatedMsg);
MESSAGE0(CMaitreDHappyMsg);
MESSAGE1(CMissiveOMatActionMsg, MissiveOMatAction, action, MESSAGE_NONE);
MESSAGE0(CMoveToStartPosMsg);
MESSAGE2(CMovieEndMsg, int, startFrame, 0, int, endFrame, 0);
MESSAGE2(CMovieFrameMsg, int, frameNumber, 0, int, value2, 0);
MESSAGE0(CMusicHasStartedMsg);
MESSAGE0(CMusicHasStoppedMsg);
MESSAGE0(CMusicSettingChangedMsg);
MESSAGE2(CNPCPlayAnimationMsg, const char *const *, names, nullptr, int, maxDuration, 0);
MESSAGE1(CNPCPlayIdleAnimationMsg, const char *const *, names, 0);
MESSAGE3(CNPCPlayTalkingAnimationMsg, uint, speechDuration, 0, int, value2, 0, const char *const *, names, nullptr);
MESSAGE0(CNPCQueueIdleAnimMsg);
MESSAGE1(CNutPuzzleMsg, CString, action, "");
MESSAGE1(COnSummonBotMsg, int, value, 0);
MESSAGE0(COpeningCreditsMsg);
MESSAGE1(CPanningAwayFromParrotMsg, CMovePlayerTo *, target, nullptr);
MESSAGE2(CParrotSpeakMsg, CString, target, "", CString, action, "");
MESSAGE2(CParrotTriesChickenMsg, bool, isHot, false, int, condiment, 0);
MESSAGE1(CPhonographPlayMsg, int, value, 0);
MESSAGE0(CPhonographReadyToPlayMsg);
MESSAGE1(CPhonographRecordMsg, bool, canRecord, false);
MESSAGE3(CPhonographStopMsg, bool, leavingRoom, false, bool, cylinderPresent, false, bool, dontStop, false);
MESSAGE2(CPlayRangeMsg, int, value1, 0, int, value2, 0);
MESSAGE2(CPlayerTriesRestaurantTableMsg, int, tableId, 0, bool, result, false);
MESSAGE1(CPreSaveMsg, int, value, 0);
MESSAGE1(CProdMaitreDMsg, int, value, 0);
MESSAGE2(CPumpingMsg, int, value, 0, CGameObject *, object, nullptr);
MESSAGE1(CPutBotBackInHisBoxMsg, int, value, 0);
MESSAGE1(CPutParrotBackMsg, int, value, 0);
MESSAGE0(CPuzzleSolvedMsg);
MESSAGE3(CQueryCylinderHolderMsg, bool, isOpen, false, bool, isPresent, false, CTreeItem *, target, (CTreeItem *)nullptr);
MESSAGE1(CQueryCylinderMsg, CString, name, "");
MESSAGE1(CQueryCylinderNameMsg, CString, name, "");
MESSAGE3(CQueryCylinderTypeMsg, int, value1, 0, int, value2, 0, int, value3, 0);
MESSAGE1(CQueryMusicControlSettingMsg, int, value, 0);
MESSAGE1(CQueryPhonographState, int, value, 0);
MESSAGE0(CRecordOntoCylinderMsg);
MESSAGE0(CRemoveFromGameMsg);
MESSAGE0(CReplaceBowlAndNutsMsg);
MESSAGE1(CRestaurantMusicChanged, CString, value, "");
MESSAGE2(CSendCCarryMsg, CString, strValue, "", int, numValue, 0);
MESSAGE1(CSenseWorkingMsg, CString, value, "Not Working");
MESSAGE2(CServiceElevatorFloorChangeMsg, int, startFloor, 0, int, endFloor, 0);
MESSAGE0(CServiceElevatorFloorRequestMsg);
MESSAGE1(CServiceElevatorMsg, int, value, 4);
MESSAGE2(CSetChevButtonImageMsg, int, value1, 0, int, value2, 0);
MESSAGE1(CSetChevClassBits, int, classNum, 0);
MESSAGE1(CSetChevFloorBits, int, floorNum, 0);
MESSAGE1(CSetChevLiftBits, int, liftNum, 0);
MESSAGE2(CSetChevPanelBitMsg, int, value1, 0, int, value2, 0);
MESSAGE1(CSetChevPanelButtonsMsg, int, chevCode, 0);
MESSAGE1(CSetChevRoomBits, int, roomFlags, 0);
MESSAGE1(CSetFrameMsg, int, frameNumber, 0);
MESSAGE0(CSetMusicControlsMsg);
MESSAGE2(CSetVarMsg, CString, varName, "", int, value, 0);
MESSAGE2(CSetVolumeMsg, int, volume, 70, int, secondsTransition, 0);
MESSAGE2(CShipSettingMsg, int, value, 0, CString, name, "");
MESSAGE2(CSignalObject, CString, strValue, "", int, numValue, 0);
MESSAGE1(CSpeechFallsFromTreeMsg, Point, pos, Point());
MESSAGE1(CStartMusicMsg, CMusicPlayer *, musicPlayer, (CMusicPlayer *)nullptr);
MESSAGE3(CStatusChangeMsg, int, oldStatus, 0, int, newStatus, 0, bool, success, false);
MESSAGE1(CStopMusicMsg, CMusicPlayer *, musicPlayer, (CMusicPlayer *)nullptr);
MESSAGE4(CSubAcceptCCarryMsg, CString, string1, "", int, value1, 0, int, value2, 0, CCarry *, item, nullptr);
MESSAGE0(CSubDeliverCCarryMsg);
MESSAGE0(CSubSendCCarryMsg);
MESSAGE0(CSUBTransition);
MESSAGE0(CSubTurnOffMsg);
MESSAGE0(CSubTurnOnMsg);
MESSAGE2(CSummonBotMsg, CString, npcName, "", int, value, 0);
MESSAGE1(CSummonBotQueryMsg, CString, npcName, "");
MESSAGE1(CTakeHeadPieceMsg, CString, value, "NULL");
MESSAGE2(CTextInputMsg, CString, input, "", CString, response, "");
MESSAGE1(CTimeDilationMsg, int, value, 0);
MESSAGE0(CTitleSequenceEndedMsg);
MESSAGE0(CTransitMsg);
MESSAGE1(CTranslateObjectMsg, Point, delta, Point());
MESSAGE3(CTransportMsg, CString, roomName, "", int, value1, 0, int, value2, 0);
MESSAGE1(CTriggerAutoMusicPlayerMsg, int, value, 0);
MESSAGE1(CTriggerNPCEvent, int, value, 0);
MESSAGE4(CTrueTalkGetAnimSetMsg, int, value1, 0, uint, index, 0, uint, startFrame, 0, uint, endFrame, 0);
MESSAGE2(CTrueTalkGetAssetDetailsMsg, CString, filename, "", int, numValue, 0);
MESSAGE2(CTrueTalkGetStateValueMsg, int, stateNum, 0, int, stateVal, -1000);
MESSAGE2(CTrueTalkNotifySpeechEndedMsg, int, endState, 0, int, dialogueId, 0);
MESSAGE3(CTrueTalkNotifySpeechStartedMsg, uint, speechDuration, 0, uint, dialogueId, 0, int, value, 0);
MESSAGE1(CTrueTalkQueueUpAnimSetMsg, int, value, 0);
MESSAGE0(CTrueTalkSelfQueueAnimSetMsg);
MESSAGE3(CTrueTalkTriggerActionMsg, int, action, 0, int, param1, 0, int, param2, 0);
MESSAGE0(CTurnOff);
MESSAGE0(CTurnOn);
MESSAGE1(CUse, CGameObject *, item, nullptr);
MESSAGE1(CUseWithCharMsg, CCharacter *, character, nullptr);
MESSAGE1(CUseWithOtherMsg, CGameObject *, other, 0);
MESSAGE1(CVirtualKeyCharMsg, Common::KeyState, keyState, Common::KeyState());
MESSAGE1(CVisibleMsg, bool, visible, true);
MESSAGE1(CCheckCodeWheelsMsg, bool, isCorrect, true);

} // End of namespace Titanic

#endif /* TITANIC_MESSAGE_H */
