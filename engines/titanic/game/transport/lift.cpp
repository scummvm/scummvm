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

#include "titanic/game/transport/lift.h"
#include "titanic/debugger.h"
#include "titanic/moves/multi_move.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLift, CTransport)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

bool CLift::_hasHead;
bool CLift::_hasCorrectHead;
int CLift::_elevator1Floor;
int CLift::_elevator2Floor;
int CLift::_elevator3Floor;
int CLift::_elevator4Floor;

void CLift::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_hasHead, indent);
	file->writeNumberLine(_elevator1Floor, indent);
	file->writeNumberLine(_elevator2Floor, indent);
	file->writeNumberLine(_elevator3Floor, indent);
	file->writeNumberLine(_elevator4Floor, indent);
	file->writeNumberLine(_liftNum, indent);
	file->writeNumberLine(_hasCorrectHead, indent);

	CTransport::save(file, indent);
}

void CLift::load(SimpleFile *file) {
	file->readNumber();
	_hasHead = file->readNumber();
	_elevator1Floor = file->readNumber();
	_elevator2Floor = file->readNumber();
	_elevator3Floor = file->readNumber();
	_elevator4Floor = file->readNumber();
	_liftNum = file->readNumber();
	_hasCorrectHead = file->readNumber();

	CTransport::load(file);
}

bool CLift::StatusChangeMsg(CStatusChangeMsg *msg) {
	CPetControl *pet = getPetControl();
	if ((!_hasHead && pet->getRoomsElevatorNum() == 4) ||
			(!_hasCorrectHead && pet->getRoomsElevatorNum() == 4))
		return true;

	int oldFloorNum = msg->_oldStatus;
	int floorNum = msg->_newStatus;
	int oldClass = 1, newClass = 1;
	if (oldFloorNum > 27)
		oldClass = 3;
	else if (oldFloorNum > 19)
		oldClass = 2;
	if (floorNum > 27)
		newClass = 3;
	else if (floorNum > 19)
		newClass = 2;

	static const int UP_FRAME_NUMBERS[40] = {
		0, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58,
		63, 68, 73, 78, 83, 88, 93, 118, 123, 128, 133,
		138, 143, 148, 153, 228, 233, 238, 243, 248, 253,
		258, 263, 268, 273, 278, 298, 299
	};
	static const int DOWN_FRAME_NUMBERS[39] = {
		598, 589, 584, 579, 574, 569, 564, 559, 554, 549,
		544, 539, 534, 529, 524, 519, 514, 509, 504, 479,
		474, 469, 464, 459, 454, 449, 444, 369, 364, 359,
		354, 349, 344, 339, 334, 329, 324, 319, 299
	};

	pet->setRoomsFloorNum(floorNum);
	if (pet->getRoomsElevatorNum() == 2 || pet->getRoomsElevatorNum() == 4) {
		if (floorNum > 27)
			floorNum = 27;
		if (oldFloorNum > 27)
			oldFloorNum = 27;
	}

	changeView("Lift.Node 1.N");
	CTurnOn onMsg;
	onMsg.execute("LiftHood");

	CString debugStr;
	if (floorNum > oldFloorNum) {
		// Animate lift going up
		_startFrame = UP_FRAME_NUMBERS[oldFloorNum - 1];
		_endFrame = UP_FRAME_NUMBERS[floorNum - 1];

		if (oldClass == newClass) {
			debugStr = CString::format("Same (%d-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else if (oldClass == 1 && newClass == 2) {
			debugStr = CString::format("1 to 2 (%d-108, 108-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 108, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(108, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else if (oldClass == 1 && newClass == 3) {
			debugStr = CString::format("1 to 3 (%d-108, 108-190, 190-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 108, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(108, 190, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(190, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else {
			debugStr = CString::format("2 to 3 (%d-190, 190-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 190, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(190, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	}

	if (floorNum < oldFloorNum) {
		// Animate lift going down
		_startFrame = DOWN_FRAME_NUMBERS[oldFloorNum - 1];
		_endFrame = DOWN_FRAME_NUMBERS[floorNum - 1];

		if (oldClass == newClass) {
			debugStr = CString::format("Same (%d-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else if (oldClass == 3 && newClass == 2) {
			debugStr = CString::format("3 to 2 (%d-407, 407-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 407, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(407, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else if (oldClass == 3 && newClass == 1) {
			debugStr = CString::format("3 to 1 (%d-407, 407-489, 489-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 407, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(407, 489, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(489, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		} else {
			debugStr = CString::format("2 to 1 (%d-489, 489-%d)", _startFrame, _endFrame);
			playMovie(_startFrame, 489, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playMovie(489, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	}

	CShipSettingMsg settingMsg;
	switch (pet->getRoomsElevatorNum()) {
	case 1:
		_elevator1Floor = floorNum;
		break;
	case 2:
		_elevator2Floor = floorNum;
		_elevator4Floor = oldFloorNum;
		settingMsg._value = oldFloorNum;
		settingMsg.execute("SGTStateroomTV");
		break;
	case 3:
		_elevator3Floor = floorNum;
		break;
	case 4:
		_elevator4Floor = floorNum;
		break;
	default:
		break;
	}

	debugC(1, kDebugScripts, "%s", debugStr.c_str());
	return true;
}

bool CLift::MovieEndMsg(CMovieEndMsg *msg) {
	switch (msg->_endFrame) {
	case 108:
		setAmbientSoundVolume(VOL_MUTE, 1, 0);
		setAmbientSoundVolume(VOL_QUIET, 1, 1);
		break;

	case 190:
		setAmbientSoundVolume(VOL_MUTE, 1, 1);
		setAmbientSoundVolume(VOL_QUIET, 1, 2);
		break;

	case 407:
		setAmbientSoundVolume(VOL_MUTE, 1, 2);
		setAmbientSoundVolume(VOL_QUIET, 1, 1);
		break;

	case 489:
		setAmbientSoundVolume(VOL_MUTE, 1, 1);
		setAmbientSoundVolume(VOL_QUIET, 1, 0);
		break;

	default: {
		CActMsg actMsg("LiftArrive");
		actMsg.execute("Liftbot");
		sleep(500);
		playSound("352 gp button 1.wav");

		CTurnOff offMsg;
		offMsg.execute("LiftHood");
		changeView("Lift.Node 1.W");
		break;
	}
	}

	return true;
}

bool CLift::EnterViewMsg(CEnterViewMsg *msg) {
	static const int FRAME_NUMBERS[40] = {
		0, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58, 63, 68, 73,
		78, 83, 88, 93, 118, 123, 128, 133, 138, 143, 148, 153,
		228, 233, 238, 243, 248, 253, 258, 263, 268, 273, 278, 298
	};

	CPetControl *pet = getPetControl();
	loadFrame(FRAME_NUMBERS[pet->getRoomsFloorNum() - 1]);
	return true;
}

bool CLift::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (isEquals("Well")) {
		CPetControl *pet = getPetControl();
		int floorNum = pet->getRoomsFloorNum();
		int elevNum = pet->getRoomsElevatorNum();
		loadSound(TRANSLATE("z#520.wav", "z#259.wav"));
		loadSound(TRANSLATE("z#519.wav", "z#258.wav"));
		loadSound(TRANSLATE("z#518.wav", "z#257.wav"));

		if (elevNum == 4 && _hasHead && !_hasCorrectHead) {
			CVisibleMsg visibleMsg;
			visibleMsg.execute("GetLiftEye");
		}

		if (floorNum < 20) {
			playAmbientSound(TRANSLATE("z#520.wav", "z#259.wav"), VOL_QUIET, true, true, 0);
			playAmbientSound(TRANSLATE("z#519.wav", "z#258.wav"), VOL_MUTE, false, true, 1);
			playAmbientSound(TRANSLATE("z#518.wav", "z#257.wav"), VOL_MUTE, false, true, 2);
		} else if (floorNum < 28) {
			playAmbientSound(TRANSLATE("z#520.wav", "z#259.wav"), VOL_MUTE, false, true, 0);
			playAmbientSound(TRANSLATE("z#519.wav", "z#258.wav"), VOL_QUIET, true, true, 1);
			playAmbientSound(TRANSLATE("z#518.wav", "z#257.wav"), VOL_MUTE, false, true, 2);
		} else {
			playAmbientSound(TRANSLATE("z#520.wav", "z#259.wav"), VOL_MUTE, false, true, 0);
			playAmbientSound(TRANSLATE("z#519.wav", "z#258.wav"), VOL_MUTE, false, true, 1);
			playAmbientSound(TRANSLATE("z#518.wav", "z#257.wav"), VOL_QUIET, true, true, 2);
		}
	}

	return true;
}

bool CLift::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	stopAmbientSound(true, -1);

	CPetControl *pet = getPetControl();
	if (pet->getRoomsElevatorNum() == 4 && _hasHead && !_hasCorrectHead) {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("Eye2");
	}

	return true;
}

bool CLift::ActMsg(CActMsg *msg) {
	if (msg->_action == "LoseHead") {
		_hasHead = false;
		_hasCorrectHead = false;

		CActMsg actMsg1("Lift.Node 2.N");
		actMsg1.execute("RPanInLiftW");
		CActMsg actMsg2("Lift.Node 2.S");
		actMsg2.execute("LPanInLiftW");

		// WORKAROUND: In the original, when Lift 4's head is removed, the other
		// view directions use Node 2. These "removed" views have links, but their
		// movement cursors weren't correctly set. This fixes them
		CNamedItem *node2 = findRoom()->findByName("Node 2");
		static_cast<CMultiMove *>(node2->findByName("LMultiLiftPan"))->_cursorId = CURSOR_MOVE_LEFT;
		static_cast<CLinkItem *>(node2->findByName("_PANR,2,N,E"))->_cursorId = CURSOR_MOVE_RIGHT;
		static_cast<CLinkItem *>(node2->findByName("_PANL,2,E,N"))->_cursorId = CURSOR_MOVE_LEFT;
		static_cast<CLinkItem *>(node2->findByName("_PANR,2,E,S"))->_cursorId = CURSOR_MOVE_RIGHT;
		static_cast<CLinkItem *>(node2->findByName("_PANL,2,S,E"))->_cursorId = CURSOR_MOVE_LEFT;
		static_cast<CMultiMove *>(node2->findByName("RMultiLiftPan"))->_cursorId = CURSOR_MOVE_RIGHT;
	} else if (msg->_action == "AddWrongHead") {
		_hasHead = true;
		_hasCorrectHead = false;

		CActMsg actMsg1("Lift.Node 1.N");
		actMsg1.execute("RPanInLiftW");
		CActMsg actMsg2("Lift.Node 1.S");
		actMsg2.execute("LPanInLiftW");
	} else if (msg->_action == "AddRightHead") {
		_hasHead = true;
		_hasCorrectHead = true;
		petSetRoomsElevatorBroken(false);

		CActMsg actMsg1("Lift.Node 1.N");
		actMsg1.execute("RPanInLiftW");
		CActMsg actMsg2("Lift.Node 1.S");
		actMsg2.execute("LPanInLiftW");
		CActMsg actMsg3("ActivateLift");
		actMsg3.execute("Liftbot");
	}

	CVisibleMsg visibleMsg;
	visibleMsg.execute("LiftbotWithoutHead");
	return true;
}

} // End of namespace Titanic
