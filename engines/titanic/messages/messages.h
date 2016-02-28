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

#include "titanic/core/saveable_object.h"
#include "titanic/core/game_object.h"

namespace Titanic {

class CMessage : public CSaveableObject {
public:
	CMessage();

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CMessage"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

class CAutoSoundEvent : public CGameObject {
protected:
	int _fieldBC;
	int _fieldC0;
public:
	CAutoSoundEvent();

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CAutoSoundEvent"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};


class CDoorAutoSoundEvent : public CAutoSoundEvent {
protected:
	CString _string1;
	CString _string2;
	int _fieldDC;
	int _fieldE0;
public:
	CDoorAutoSoundEvent();

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CDoorAutoSoundEvent"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};


class CEditControlMsg : public CMessage {
private:
	int _field4;
	int _field8;
	CString _string1;
	int _field18;
	int _field1C;
	int _field20;
public:
	CEditControlMsg() : _field4(0), _field8(0), _field18(0),
		_field1C(0), _field20(0) {}

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CEditControlMsg"; }
};

class CLightsMsg : public CMessage {
public:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
public:
	CLightsMsg() : CMessage(), _field4(0), _field8(0),
		_fieldC(0), _field10(0) {}

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CLightsMsg"; }
};

class CIsHookedOnMsg : public CMessage {
private:
	int _field4;
	int _field8;
	CString _string1;
	int _field18;
	int _field1C;
	int _field20;
public:
	CIsHookedOnMsg() : CMessage(), _field4(0), _field8(0),
		_field18(0), _field1C(0), _field20(0) {}

	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CIsHookedOnMsg"; }
};


class CSubAcceptCCarryMsg : public CMessage {
public:
	CString _string1;
	int _value1, _value2, _value3;
public:
	CSubAcceptCCarryMsg() : _value1(0), _value2(0), _value3(0) {}

	/**
	* Return the class name
	*/
	virtual const char *getClassName() const { return "CSubAcceptCCarryMsg"; }
};

class CTransportMsg : public CMessage {
public:
	CString _string;
	int _value1, _value2;
public:
	CTransportMsg() : _value1(0), _value2(0) {}

	/**
	* Return the class name
	*/
	virtual const char *getClassName() const { return "CTransportMsg"; }
};

#define RAW_MESSAGE(NAME) class NAME: public CMessage { \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define STR_MESSAGE(NAME, FIELD) class NAME: public CMessage { \
	public: CString FIELD; \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define STR2_MESSAGE(NAME, FIELD1, FIELD2) class NAME: public CMessage { \
	public: CString FIELD1, FIELD2; \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define STR_MESSAGE_VAL(NAME, FIELD, VAL) class NAME: public CMessage { \
	public: CString FIELD; \
	NAME(): FIELD(VAL) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM_MESSAGE(NAME, FIELD) class NAME: public CMessage { \
	public: int FIELD; \
	NAME(): FIELD(0) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM_MESSAGE_VAL(NAME, FIELD, VAL) class NAME: public CMessage { \
	public: int FIELD; \
	NAME(): CMessage(), FIELD(VAL) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM2_MESSAGE(NAME, FIELD1, FIELD2) class NAME: public CMessage { \
	public: int FIELD1, FIELD2; \
	NAME(): CMessage(), FIELD1(0), FIELD2(0) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM3_MESSAGE(NAME, FIELD1, FIELD2, FIELD3) class NAME: public CMessage { \
	public: int FIELD1, FIELD2, FIELD3; \
	NAME(): CMessage(), FIELD1(0), FIELD2(0), FIELD3(0) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM4_MESSAGE(NAME, FIELD1, FIELD2, FIELD3, FIELD4) \
	class NAME: public CMessage { \
	public: int FIELD1, FIELD2, FIELD3, FIELD4; \
	NAME(): CMessage(), FIELD1(0), FIELD2(0), FIELD3(0), FIELD4(0) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define NUM2_MESSAGE_VAL(NAME, FIELD1, FIELD2, VAL1, VAL2) \
	class NAME: public CMessage { \
	public: int FIELD1, FIELD2; \
	NAME(): CMessage(), FIELD1(VAL1), FIELD2(VAL2) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define SNUM_MESSAGE(NAME, SFIELD, NFIELD) class NAME: public CMessage { \
	public: CString SFIELD; CString NFIELD; \
	virtual const char *getClassName() const { return #NAME; } \
	}
#define SNUM_MESSAGE_VAL(NAME, SFIELD, NFIELD, SVAL, NVAL) \
	class NAME: public CMessage { \
	public: CString SFIELD; CString NFIELD; \
	NAME(): CMessage(), SFIELD(SVAL), NFIELD(NVAL) {} \
	virtual const char *getClassName() const { return #NAME; } \
	}

STR_MESSAGE(CActMsg, _value);
STR_MESSAGE(CActivationmsg, _value);
STR_MESSAGE_VAL(CAddHeadPieceMsg, _value, "NULL");
NUM_MESSAGE(CAnimateMaitreDMsg, _value);
NUM_MESSAGE(CArboretumGateMsg, _value);
RAW_MESSAGE(CArmPickedUpFromTableMsg);
RAW_MESSAGE(CBodyInBilgeRoomMsg);
NUM_MESSAGE(CBowlStateChange, _value);
SNUM_MESSAGE(CCarryObjectArrivedMsg, _strValue, _numValue);
STR_MESSAGE_VAL(CChangeSeasonMsg, _season, "Summer");
RAW_MESSAGE(CCheckAllPossibleCodes);
NUM2_MESSAGE(CCheckChevCode, _value1, _value2);
NUM_MESSAGE(CChildDragEndMsg, _value);
NUM2_MESSAGE(CChildDragMoveMsg, _value1, _value2);
NUM2_MESSAGE(CChildDragStartMsg, _value1, _value2);
RAW_MESSAGE(CClearChevPanelBits);
RAW_MESSAGE(CCorrectMusicPlayedMsg);
RAW_MESSAGE(CCreateMusicPlayerMsg);
RAW_MESSAGE(CCylinderHolderReadyMsg);
RAW_MESSAGE(CDeactivationMsg);
STR_MESSAGE(CDeliverCCarryMsg, _value);
RAW_MESSAGE(CDisableMaitreDProdReceptor);
RAW_MESSAGE(CDismissBotMsg);
RAW_MESSAGE(CDoffNavHelmet);
RAW_MESSAGE(CDonNavHelmet);
NUM_MESSAGE(CDoorbotNeededInElevatorMsg, _value);
RAW_MESSAGE(CDoorbotNeededInHomeMsg);
NUM_MESSAGE(CDropobjectMsg, _value);
NUM_MESSAGE(CDropZoneGotObjectMsg, _value);
NUM_MESSAGE(CDropZoneLostObjectMsg, _value);
NUM_MESSAGE(CEjectCylinderMsg, _value);
RAW_MESSAGE(CErasePhonographCylinderMsg);
NUM2_MESSAGE(CFreshenCookieMsg, _value1, _value2);
NUM_MESSAGE(CGetChevClassBits, _value);
NUM_MESSAGE(CGetChevClassNum, _value);
SNUM_MESSAGE(CGetChevCodeFromRoomNameMsg, _strValue, _numValue);
NUM_MESSAGE(CGetChevFloorBits, _value);
NUM_MESSAGE(CGetChevFloorNum, _value);
NUM_MESSAGE(CGetChevLiftBits, _value);
NUM_MESSAGE(CGetChevLiftNum, _value);
NUM_MESSAGE(CGetChevRoomBits, _value);
NUM_MESSAGE(CGetChevRoomNum, _value);
NUM2_MESSAGE_VAL(CHoseConnectedMsg, _value1, _value2, 1, 0);
RAW_MESSAGE(CInitializeAnimMsg);
NUM_MESSAGE(CIsEarBowlPuzzleDone, _value);
NUM_MESSAGE(CIsParrotPresentMsg, _value);
NUM_MESSAGE_VAL(CKeyCharMsg, _value, 32);
NUM2_MESSAGE(CLemonFallsFromTreeMsg, _value1, _value2);
NUM_MESSAGE(CLockPhonographMsg, _value);
RAW_MESSAGE(CMaitreDDefeatedMsg);
RAW_MESSAGE(CMaitreDHappyMsg);
NUM_MESSAGE(CMissiveOMatActionMsg, _value);
RAW_MESSAGE(CMoveToStartPosMsg);
NUM2_MESSAGE(CMovieEndMsg, _value1, _value2);
NUM2_MESSAGE(CMovieFrameMsg, _value1, _value2);
RAW_MESSAGE(CMusicHasStartedMsg);
RAW_MESSAGE(CMusicHasStoppedMsg);
RAW_MESSAGE(CMusicSettingChangedMsg);
NUM2_MESSAGE(CNPCPlayAnimationMsg, _value1, _value2);
NUM_MESSAGE(CNPCPlayIdleAnimationMsg, _value);
NUM3_MESSAGE(CNPCPlayTalkingAnimationMsg, _value1, _value2, _value3);
RAW_MESSAGE(CNPCQueueIdleAnimMsg);
STR_MESSAGE(CNutPuzzleMsg, _value);
NUM_MESSAGE(COnSummonBotMsg, _value);
RAW_MESSAGE(COpeningCreditsMsg);
NUM_MESSAGE(CPanningAwayFromParrotMsg, _value);
STR2_MESSAGE(CParrotSpeakMsg, _value1, _value2);
NUM2_MESSAGE(CParrotTriesChickenMsg, _value1, _value2);
NUM4_MESSAGE(CPassOnDragStartMsg, _value1, _value2, _value3, _value4);
NUM_MESSAGE(CPhonographPlayMsg, _value);
RAW_MESSAGE(CPhonographReadyToPlayMsg);
NUM_MESSAGE(CPhonographRecordMsg, _value);
NUM3_MESSAGE(CPhonographStopMsg, _value1, _value2, _value3);
NUM2_MESSAGE(CPlayRangeMsg, _value1, _value2);
NUM2_MESSAGE(CPlayerTriesRestaurantTableMsg, _value1, _value2);
NUM_MESSAGE(CPreSaveMsg, _value);
NUM_MESSAGE(CProdMaitreDMsg, _value);
NUM2_MESSAGE(CPumpingMsg, _value1, _value2);
NUM_MESSAGE(CPutBotBackInHisBoxMsg, _value);
NUM_MESSAGE(CPutParrotBackMsg, _value);
RAW_MESSAGE(CPuzzleSolvedMsg);
NUM3_MESSAGE(CQueryCylinderHolderMsg, _value1, _value2, _value3);
NUM3_MESSAGE(CQueryCylinderMsg, _value1, _value2, _value3);
NUM3_MESSAGE(CQueryCylinderNameMsg, _value1, _value2, _value3);
NUM3_MESSAGE(CQueryCylinderTypeMsg, _value1, _value2, _value3);
NUM_MESSAGE(CQueryMusicControlSettingMsg, _value);
NUM_MESSAGE(CQueryPhonographState, _value);
RAW_MESSAGE(CRecordOntoCylinderMsg);
RAW_MESSAGE(CRemoveFromGameMsg);
RAW_MESSAGE(CReplaceBowlAndNutsMsg);
STR_MESSAGE(CRestaurantMusicChanged, _value);
SNUM_MESSAGE(CSendCCarryMsg, _strValue, _numValue);
STR_MESSAGE_VAL(CSenseWorkingMsg, _value, "Not Working");
NUM2_MESSAGE(CServiceElevatorFloorChangeMsg, _value1, _value2);
RAW_MESSAGE(CServiceElevatorFloorRequestMsg);
NUM_MESSAGE_VAL(CServiceElevatorMsg, _value, 4);
NUM2_MESSAGE(CSetChevButtonImageMsg, _value1, _value2);
NUM_MESSAGE(CSetChevClassBits, _value);
NUM_MESSAGE(CSetChevFloorBits, _value);
NUM_MESSAGE(CSetChevLiftBits, _value);
NUM2_MESSAGE(CSetChevPanelBitMsg, _value1, _value2);
NUM_MESSAGE(CSetChevPanelButtonsMsg, _value);
NUM_MESSAGE(CSetChevRoomBits, _value);
RAW_MESSAGE(CSetMusicControlsMsg);
SNUM_MESSAGE(CSetVarMsg, _varName, _value);
NUM2_MESSAGE_VAL(CSetVolumeMsg, _value1, _value2, 70, 0);
SNUM_MESSAGE(CShipSettingMsg, _strValue, _numValue);
STR_MESSAGE_VAL(CShowTextMsg, _value, "NO TEXT INCLUDED!!!");
SNUM_MESSAGE(CSignalObject, _strValue, _numValue);
NUM2_MESSAGE(CSpeechFallsFromTreeMsg, _value1, _value2);
NUM_MESSAGE(CStartMusicMsg, _value);
NUM3_MESSAGE(CStatusChangeMsg, _value1, _value2, _value3);
NUM_MESSAGE(CStopMusicMsg, _value);
RAW_MESSAGE(CSubDeliverCCarryMsg);
RAW_MESSAGE(CSubSendCCarryMsg);
RAW_MESSAGE(CSUBTransition);
RAW_MESSAGE(CSubTurnOffMsg);
RAW_MESSAGE(CSubTurnOnMsg);
SNUM_MESSAGE(CSummonBotMsg, _strValue, _numValue);
STR_MESSAGE(CSummonBotQuerryMsg, _value);
STR_MESSAGE(CTakeHeadPieceMsg, _value);
STR2_MESSAGE(CTextInputMsg, _value1, _value2);
NUM_MESSAGE(CTimeDilationMsg, _value);
NUM_MESSAGE(CTimeMsg, _value);
RAW_MESSAGE(CTitleSequenceEndedMsg);
RAW_MESSAGE(CTransitMsg);
NUM_MESSAGE(CTriggerAutoMusicPlayerMsg, _value);
NUM_MESSAGE(CTriggerNPCEvent, _value);
NUM4_MESSAGE(CTrueTalkGetAnimSetMsg, _value1, _value2, _value3, _value4);
SNUM_MESSAGE(CTrueTalkGetAssetDetailsMsg, _strValue, _numValue);
NUM2_MESSAGE_VAL(CTrueTalkGetStateValueMsg, _value1, _value2, 0, -1000);
NUM2_MESSAGE(CTrueTalkNotifySpeechEndedMsg, _value1, _value2);
NUM3_MESSAGE(CTrueTalkNotifySpeechStartedMsg, _value1, _value2, _value);
NUM_MESSAGE(CTrueTalkQueueUpAnimSetMsg, _value);
RAW_MESSAGE(CTrueTalkSelfQueueAnimSetMsg);
NUM3_MESSAGE(CTrueTalkTriggerActionMsg, _value1, _value2, _value3);
RAW_MESSAGE(CTurnOff);
RAW_MESSAGE(CTurnOn);
NUM_MESSAGE(CUse, _value);
NUM_MESSAGE(CUseWithCharMsg, _value);
NUM_MESSAGE(CUseWithOtherMsg, _value);
NUM_MESSAGE(CVirtualKeyCharMsg, _value);
NUM2_MESSAGE_VAL(CVisibleMsg, _value1, _value2, 1, 0);

} // End of namespace Titanic

#endif /* TITANIC_MESSAGE_H */
