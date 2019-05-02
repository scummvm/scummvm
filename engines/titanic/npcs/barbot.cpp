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

#include "titanic/npcs/barbot.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

int CBarbot::_timesCalled;

BEGIN_MESSAGE_MAP(CBarbot, CTrueTalkNPC)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TrueTalkSelfQueueAnimSetMsg)
	ON_MESSAGE(TrueTalkQueueUpAnimSetMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(LoadSuccessMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CBarbot::FrameRanges::FrameRanges() : Common::Array<FrameRange>() {
	resize(60);
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("FRAMES/BARBOT");
	for (int idx = 0; idx < 60; ++idx) {
		(*this)[idx]._startFrame = stream->readUint32LE();
		(*this)[idx]._endFrame = stream->readUint32LE();
	}

	delete stream;
}

/*------------------------------------------------------------------------*/

CBarbot::CBarbot() : CTrueTalkNPC() {
	_field108 = 0;
	_field10C = 0;
	_field110 = 0;
	_addedLemon = false;
	_addedTV = false;
	_addedPuret = false;
	_field120 = 0;
	_field124 = 0;
	_visCenterOnCounter = false;
	_addedVodka = false;
	_gottenDrunk = false;
	_field134 = 0;
	_field138 = 0;
	_field13C = -1;
	_volume = 30;
	_frameNum = -1;
	_field148 = -1;
	_field14C = 0;
	_field150 = 0;
	_field154 = 0;
	_glassContent = GG_DEFAULT;
	_drunkFlag = false;
	_field160 = 0;
}

void CBarbot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_addedLemon, indent);
	file->writeNumberLine(_addedTV, indent);
	file->writeNumberLine(_addedPuret, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_visCenterOnCounter, indent);

	file->writeNumberLine(_timesCalled, indent);
	file->writeNumberLine(_addedVodka, indent);
	file->writeNumberLine(_gottenDrunk, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_volume, indent);
	file->writeNumberLine(_frameNum, indent);
	file->writeNumberLine(_field148, indent);
	file->writeNumberLine(_field14C, indent);
	file->writeNumberLine(_field150, indent);
	file->writeNumberLine(_field154, indent);
	file->writeNumberLine(_glassContent, indent);
	file->writeNumberLine(_drunkFlag, indent);
	file->writeNumberLine(_field160, indent);

	CTrueTalkNPC::save(file, indent);
}

void CBarbot::load(SimpleFile *file) {
	file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_addedLemon = file->readNumber();
	_addedTV = file->readNumber();
	_addedPuret = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_visCenterOnCounter = file->readNumber();

	_timesCalled = file->readNumber();
	_addedVodka = file->readNumber();
	_gottenDrunk = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_volume = file->readNumber();
	_frameNum = file->readNumber();
	_field148 = file->readNumber();
	_field14C = file->readNumber();
	_field150 = file->readNumber();
	_field154 = file->readNumber();
	_glassContent = (GlassGiven)file->readNumber();
	_drunkFlag = file->readNumber();
	_field160 = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CBarbot::ActMsg(CActMsg *msg) {
	if (msg->_action == "Vodka") {
		if (!_addedVodka) {
			playRange(_frames[47], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[46], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[40]);
			playRange(_frames[7]);
			playRange(_frames[13]);
			playRange(_frames[8]);
			playRange(_frames[40]);
			playRange(_frames[7]);
			playRange(_frames[13]);
			playRange(_frames[8]);
			playRange(_frames[7]);
			playRange(_frames[40]);
			playRange(_frames[13]);
			playRange(_frames[40]);
			playRange(_frames[7]);
			playRange(_frames[8]);
			playRange(_frames[13]);
			playRange(_frames[40], MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_frameNum = _frames[40]._endFrame;
		}
	} else if (msg->_action == "GiveBackVisCentre") {
		if (_field134) {
			playRange(_frames[27]);
			_frameNum = _frames[27]._endFrame;
		}
	} else if (msg->_action == "Bird") {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 2;
		statusMsg.execute("PickUpGlass");
		_glassContent = GG_BIRD;

		playRange(_frames[32], MOVIE_NOTIFY_OBJECT);
		movieEvent();
		playRange(_frames[30], MOVIE_NOTIFY_OBJECT);
		_frameNum = _frames[30]._endFrame;

		if (areIngredientsMissing()) {
			playRange(_frames[42], MOVIE_NOTIFY_OBJECT);
			_frameNum = _frames[42]._endFrame;
		}

		CActMsg actMsg("InTitilator");
		actMsg.execute("BeerGlass");
	} else if (msg->_action == "None") {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 2;
		statusMsg.execute("PickUpGlass");
		_glassContent = GG_EMPTY;

		playRange(_frames[55], MOVIE_NOTIFY_OBJECT);
		movieEvent();
		playRange(_frames[54], MOVIE_NOTIFY_OBJECT);
		_frameNum = _frames[54]._endFrame;
	} else if (msg->_action == "Mustard" || msg->_action == "Tomato") {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 2;
		statusMsg.execute("PickUpGlass");
		_glassContent = GG_CONDIMENT;

		playRange(_frames[55], MOVIE_NOTIFY_OBJECT);
		movieEvent();
		playRange(_frames[54], MOVIE_NOTIFY_OBJECT);
		_frameNum = _frames[54]._endFrame;

		CActMsg actMsg("InTitilator");
		actMsg.execute("BeerGlass");
	} else if (msg->_action == "Fruit") {
		if (!_addedLemon) {
			CActMsg visibleMsg;
			visibleMsg.execute("LemonOnBar");
			startTalking(this, 250576);
			_addedLemon = true;

			playRange(_frames[36], MOVIE_NOTIFY_OBJECT);
			movieEvent();
			_frameNum = _frames[36]._endFrame;

			if (areIngredientsMissing()) {
				playRange(_frames[43], MOVIE_NOTIFY_OBJECT);
				_frameNum = _frames[43]._endFrame;
			}

			CRemoveFromGameMsg removeMsg;
			removeMsg.execute("Lemon");
		}
	} else if (msg->_action == "CrushedTV") {
		if (!_addedTV) {
			CVisibleMsg visibleMsg;
			visibleMsg.execute("TVOnBar");
			startTalking(this, 250584);
			_field160 = 1;
			_addedTV = true;

			playSound(TRANSLATE("c#5.wav", "c#65.wav"), _volume);
			playRange(_frames[35], MOVIE_NOTIFY_OBJECT);
			movieEvent();
			playRange(_frames[34]);
			playRange(_frames[33], MOVIE_NOTIFY_OBJECT);
			_frameNum = _frames[33]._endFrame;

			if (areIngredientsMissing()) {
				playRange(_frames[41], MOVIE_NOTIFY_OBJECT);
				_frameNum = _frames[41]._endFrame;
			}

			CRemoveFromGameMsg removeMsg;
			removeMsg.execute("CrushedTV");
		}
	} else if (msg->_action == "PlayerTakesGlass") {
		playRange(_frames[53]);
		_field124 = 0;

		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpGlass");
	} else if (msg->_action == "PlayerTakesVisCentre") {
		_visCenterOnCounter = false;
		loadFrame(0);
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpVisCentre");
	} else if (msg->_action == "BellRing1") {
		startTalking(this, 251105);
	} else if (msg->_action == "BellRing2") {
		startTalking(this, 251107);
	} else if (msg->_action == "BellRing3") {
		startTalking(this, 250285);
	} else if (msg->_action == "GoRingBell") {
		startTalking(this, 250285);
	} else if (msg->_action == "ClickOnVision") {
		startTalking(this, 251858);
	}

	return true;
}

bool CBarbot::EnterViewMsg(CEnterViewMsg *msg) {
	// I think this is a remnant of early debugging code
	if (getName() != "Barbot")
		playMovie(MOVIE_REPEAT);

	return true;
}

bool CBarbot::EnterRoomMsg(CEnterRoomMsg *msg) {
	// I think this is a remnant of early debugging code
	if (getName() != "Barbot")
		addTimer(g_vm->getRandomNumber(20000));

	return true;
}

bool CBarbot::TurnOn(CTurnOn *msg) {
	if (!_fieldC4) {
		_field13C = -1;
		setVisible(true);

		CGameObject *glass = findInRoom("BeerGlass");
		if (!_gottenDrunk) {
			CVisibleMsg visibleMsg(false);
			visibleMsg.execute("BarShelfVisCentre");
		}

		if (glass && !_addedPuret) {
			playRange(_frames[38], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[58], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[57], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[56], MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_frameNum = _frames[56]._endFrame;
		} else {
			playRange(_frames[38]);
			playRange(_frames[23], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[21], MOVIE_NOTIFY_OBJECT);
			_frameNum = _frames[21]._endFrame;

			switch (g_vm->getRandomNumber(2)) {
			case 0:
				playRange(_frames[10], MOVIE_NOTIFY_OBJECT);
				_frameNum = _frames[10]._endFrame;
				break;
			case 1:
				playRange(_frames[12], MOVIE_NOTIFY_OBJECT);
				_frameNum = _frames[12]._endFrame;
				break;
			default:
				break;
			}
			_field124 = 0;
		}

		_fieldC4 = 1;
		++_timesCalled;
		petSetArea(PET_CONVERSATION);
		setTalking(this, true);
	}

	return true;
}

bool CBarbot::TurnOff(CTurnOff *msg) {
	if (_fieldC4) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpGlass");
		statusMsg.execute("PickUpVisCentre");

		if (_field124) {
			playRange(_frames[17], MOVIE_NOTIFY_OBJECT);
			_frameNum = _frames[17]._endFrame;
			_field124 = 0;
		}

		if (_visCenterOnCounter) {
			// Barbot will put away the vision center
			playRange(_frames[28], MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_frameNum = _frames[28]._endFrame;
			_visCenterOnCounter = false;
			_field134 = 1;
		}

		playRange(_frames[29], MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		movieEvent(_frames[29]._startFrame);
		_frameNum = _frames[29]._endFrame;
		_fieldC4 = 0;
	}

	return true;
}

bool CBarbot::LeaveViewMsg(CLeaveViewMsg *msg) {
	CTurnOff offMsg;
	offMsg.execute(this);
	return true;
}

bool CBarbot::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == _frameNum) {
		_frameNum = -1;
		_field14C = getTicksCount();
	}

	if (msg->_endFrame == _field148) {
		_field148 = -1;
		_field150 = getTicksCount();
	}

	if (msg->_endFrame == _field13C) {
		if (_field124)
			playMovie(_frames[53]._startFrame, _frames[53]._startFrame, 0);
		else if (_visCenterOnCounter)
			playMovie(_frames[27]._endFrame, _frames[27]._endFrame, 0);

		_field13C = -1;
		return true;
	}

	if (msg->_endFrame == _frames[58]._endFrame || msg->_endFrame == _frames[21]._endFrame) {
		if (!_gottenDrunk) {
			CVisibleMsg visibleMsg(true);
			visibleMsg.execute("BarShelfVisCentre");
		}
	}

	if (msg->_endFrame == _frames[57]._endFrame) {
		startTalking(this, 250575);
		playSound(TRANSLATE("c#10.wav", "c#70.wav"), _volume);
		return true;
	}

	if (msg->_endFrame == _frames[55]._endFrame) {
		playSound(TRANSLATE("c#10.wav", "c#70.wav"), _volume);
		return true;
	}

	if (msg->_endFrame == _frames[56]._endFrame
			|| msg->_endFrame == _frames[54]._endFrame) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("PickUpGlass");
		CMoveToStartPosMsg moveMsg;
		moveMsg.execute("BeerGlass");
		return true;
	}

	if (msg->_endFrame == _frames[30]._endFrame) {
		_field124 = 0;
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpGlass");
	}

	if (msg->_endFrame == _frames[45]._endFrame) {
		if (!_gottenDrunk) {
			CVisibleMsg visibleMsg(false);
			visibleMsg.execute("BarShelfVisCentre");
		}

		return true;
	}

	if (msg->_endFrame == _frames[44]._endFrame) {
		_visCenterOnCounter = true;
		_gottenDrunk = true;
		startTalking(this, 250586);
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("PickUpVisCentre");
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("VisionCentre");
	}

	if (msg->_endFrame == _frames[46]._endFrame) {
		if (!_gottenDrunk && !areIngredientsMissing())
			startTalking(this, 250571);
		return true;
	}

	if (msg->_endFrame == _frames[43]._endFrame
			|| msg->_endFrame == _frames[42]._endFrame
			|| msg->_endFrame == _frames[41]._endFrame) {
		if (_field124)
			playMovie(_frames[53]._startFrame, _frames[53]._startFrame, 0);
		return true;
	}

	if (msg->_endFrame == _frames[38]._endFrame || msg->_endFrame == _frames[23]._endFrame) {
		playSound(TRANSLATE("c#3.wav", "c#63.wav"), _volume);
	} else if (msg->_endFrame == _frames[36]._endFrame) {
		playSound(TRANSLATE("c#6.wav", "c#66.wav"), _volume);
	} else if (msg->_endFrame == _frames[35]._endFrame) {
		playSound(TRANSLATE("c#8.wav", "c#68.wav"), _volume);
	} else if (msg->_endFrame == _frames[33]._endFrame) {
		playSound(TRANSLATE("c#4.wav", "c#64.wav"), _volume);
	} else if (msg->_endFrame == _frames[32]._endFrame) {
		startTalking(this, 145);
		playSound(TRANSLATE("c#9.wav", "c#69.wav"), _volume);
	} else if (msg->_endFrame == _frames[47]._endFrame) {
		playSound(TRANSLATE("c#9.wav", "c#69.wav"), _volume);
		_addedVodka = true;
		_drunkFlag = true;
	} else if (msg->_endFrame == _frames[30]._endFrame) {
		playSound(TRANSLATE("c#4.wav", "c#64.wav"), 60);
	} else if (msg->_endFrame == _frames[29]._endFrame) {
		if (!_fieldC4) {
			performAction(true, nullptr);
			setVisible(false);
			CActMsg actMsg("ResetCount");
			actMsg.execute("BarBell");
		}
	} else if (msg->_endFrame == _frames[27]._endFrame) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("PickUpVisCentre");
		_visCenterOnCounter = true;
		_field134 = 0;
		startTalking(this, 250586);
	}

	return true;
}

bool CBarbot::TrueTalkSelfQueueAnimSetMsg(CTrueTalkSelfQueueAnimSetMsg *msg) {
	return true;
}

bool CBarbot::TrueTalkQueueUpAnimSetMsg(CTrueTalkQueueUpAnimSetMsg *msg) {
	return true;
}

bool CBarbot::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	switch (msg->_stateNum) {
	case 2:
		if (!_gottenDrunk) {
			if (_drunkFlag) {
				msg->_stateVal = _field134 | 1;
				return true;
			}
		}

		msg->_stateVal = _field134;
		break;

	case 3:
		msg->_stateVal = 0;
		if (_addedLemon)
			msg->_stateVal = 1;
		if (_addedVodka)
			msg->_stateVal |= 2;
		if (_addedPuret)
			msg->_stateVal |= 4;
		if (_addedTV)
			msg->_stateVal |= 8;
		break;

	case 9:
		msg->_stateVal = _drunkFlag ? 1 : 0;
		break;

	default:
		break;
	}

	return true;
}

bool CBarbot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	switch (msg->_action) {
	case 6:
		if (_field134) {
			playRange(_frames[27], MOVIE_NOTIFY_OBJECT);
			_frameNum = _frames[27]._endFrame;
		} else if (!_gottenDrunk && _drunkFlag) {
			playRange(_frames[45], MOVIE_NOTIFY_OBJECT);
			playRange(_frames[44], MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_frameNum = _frames[44]._endFrame;
		}
		break;

	case 7: {
		// "add vodka"
		CActMsg actMsg("Vodka");
		actMsg.execute(this);
		break;
	}

	case 30:
		// "starling puret"
		_addedPuret = true;
		break;

	default:
		break;
	}

	return true;
}

bool CBarbot::FrameMsg(CFrameMsg *msg) {
	if (!_fieldC4 || _frameNum != -1 || _field148 != -1
			|| (msg->_ticks - _field14C) <= 5000
			|| (msg->_ticks - _field150) <= 1000)
		return true;

	if (!_drunkFlag) {
		if (++_field154 > 2) {
			playRange(_frames[0]);
			playRange(_frames[1], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[1]._endFrame;
			_field154 = 0;

			return true;
		}

		switch (g_vm->getRandomNumber(5)) {
		case 0:
			playRange(_frames[4], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[4]._endFrame;
			break;

		case 1:
			playRange(_frames[10], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[10]._endFrame;
			break;

		case 2:
			playRange(_frames[7], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[7]._endFrame;
			break;

		case 3:
			playRange(_frames[0]);
			playRange(_frames[1], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[1]._endFrame;
			break;

		case 4:
			playRange(_frames[3], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[3]._endFrame;
			break;

		case 5:
			if (!_field160 && !_visCenterOnCounter) {
				playRange(_frames[15], MOVIE_NOTIFY_OBJECT);
				_field148 = _frames[15]._endFrame;
			}
			break;

		default:
			break;
		}
	} else {
		static const int CASES[23] = {
			0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7
		};
		switch (CASES[g_vm->getRandomNumber(22)]) {
		case 0:
			playRange(_frames[13], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[13]._endFrame;
			break;

		case 1:
			playRange(_frames[4], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[4]._endFrame;
			break;

		case 2:
			playRange(_frames[8], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[8]._endFrame;
			break;

		case 3:
			playRange(_frames[7], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[7]._endFrame;
			break;

		case 4:
			playRange(_frames[10], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[10]._endFrame;
			break;

		case 5:
			playRange(_frames[2], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[2]._endFrame;
			break;

		case 6:
			playRange(_frames[6], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[6]._endFrame;
			break;

		default:
			playRange(_frames[3], MOVIE_NOTIFY_OBJECT);
			_field148 = _frames[3]._endFrame;
			break;
		}
	}

	return true;
}

bool CBarbot::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	_field14C = _field150 = getTicksCount();
	_frameNum = -1;
	_field148 = -1;

	return true;
}

bool CBarbot::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (msg->_frameNumber == _frames[29]._startFrame) {
		playSound(TRANSLATE("c#2.wav", "c#62.wav"), _volume);

	} else if (msg->_frameNumber == _frames[55]._startFrame
			|| msg->_frameNumber == _frames[32]._startFrame) {
		// Finished giving the Barbot a glass
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpGlass");

		if (_glassContent == GG_EMPTY) {
			// I'd rather see that full of Starling Puret
			startTalking(this, 250574);
		} else if (_glassContent > GG_EMPTY) {
			// What's this?
			startTalking(this, 250580);
			petSetArea(PET_CONVERSATION);
		}

		_glassContent = GG_DEFAULT;

	} else if (msg->_frameNumber == _frames[36]._startFrame) {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("LemonOnBar");

	} else if (msg->_frameNumber == _frames[35]._startFrame) {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("TVOnBar");
	}

	return true;
}

bool CBarbot::TimerMsg(CTimerMsg *msg) {
	if (!_fieldC4 && compareRoomNameTo("Bar")) {
		CParrotSpeakMsg speakMsg("Barbot", "AskForDrink");
		speakMsg.execute("PerchedParrot");
		addTimer(10000 + getRandomNumber(20000));
	}

	return true;
}

void CBarbot::playRange(const FrameRange &range, uint flags) {
	playMovie(range._startFrame, range._endFrame, flags);
}

} // End of namespace Titanic
