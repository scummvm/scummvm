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

#include "titanic/game/bomb.h"
#include "titanic/game/code_wheel.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBomb, CBackground)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(TrueTalkGetStateValueMsg)
	ON_MESSAGE(SetFrameMsg)
END_MESSAGE_MAP()

const int CORRECT_WHEELS = 23;

static const char *const HUNDREDS_WAVS[] = {
	"", "z#353.wav", "z#339.wav", "z#325.wav", "z#311.wav", "z#297.wav",
	"z#283.wav", "z#269.wav", "z#255.wav", "z#241.wav"
};

static const char *const HUNDREDS_AND_WAVS[] = {
	"", "z#352.wav", "z#338.wav", "z#324.wav", "z#310.wav", "z#296.wav",
	"z#281.wav", "z#268.wav", "z#254.wav", "z#240.wav"
};

static const char *const COUNTDOWN_WAVS[100] = {
	"bombcountdown_c0.wav", "z#355.wav", "z#341.wav",  "z#327.wav", "z#313.wav",
	"z#299.wav", "z#285.wav", "z#271.wav", "z#257.wav", "z#243.wav",
	"z#354.wav", "z#350.wav", "z#349.wav", "z#348.wav", "z#347.wav",
	"z#346.wav", "z#345.wav", "z#344.wav", "z#343.wav", "z#342.wav",
	"z#340.wav", "z#336.wav", "z#335.wav", "z#334.wav", "z#333.wav",
	"z#332.wav", "z#331.wav", "z#330.wav", "z#329.wav", "z#328.wav",
	"z#326.wav", "z#322.wav", "z#321.wav", "z#320.wav", "z#319.wav",
	"z#318.wav", "z#317.wav", "z#316.wav", "z#315.wav", "z#314.wav",
	"z#312.wav", "z#308.wav", "z#307.wav", "z#306.wav", "z#305.wav",
	"z#304.wav", "z#303.wav", "z#302.wav", "z#301.wav", "z#300.wav",
	"z#298.wav", "z#294.wav", "z#293.wav", "z#292.wav", "z#291.wav",
	"z#290.wav", "z#289.wav", "z#288.wav", "z#287.wav", "z#286.wav",
	"z#284.wav", "z#280.wav", "z#279.wav", "z#278.wav", "z#277.wav",
	"z#276.wav", "z#275.wav", "z#274.wav", "z#273.wav", "z#272.wav",
	"z#270.wav", "z#266.wav", "z#265.wav", "z#264.wav", "z#263.wav",
	"z#262.wav", "z#261.wav", "z#260.wav", "z#259.wav", "z#258.wav",
	"z#256.wav", "z#252.wav", "z#251.wav", "z#250.wav", "z#249.wav",
	"z#248.wav", "z#247.wav", "z#246.wav", "z#245.wav", "z#244.wav",
	"z#242.wav", "z#238.wav", "z#237.wav", "z#236.wav", "z#235.wav",
	"z#234.wav", "z#233.wav", "z#232.wav", "z#231.wav", "z#230.wav",
};

CBomb::CBomb() : CBackground() {
	_active = false;
	_numCorrectWheels = 0;
	_tappedCtr = 17;
	_hammerCtr = 9;
	_commentCtr = 0;
	_countdown = 999;
	_soundHandle = 0;
	_unusedHandle = 0;
	_startingTicks = 0;
	_volume = 60;
}

void CBomb::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_active, indent);
	file->writeNumberLine(_numCorrectWheels, indent);
	file->writeNumberLine(_tappedCtr, indent);
	file->writeNumberLine(_hammerCtr, indent);
	file->writeNumberLine(_commentCtr, indent);
	file->writeNumberLine(_countdown, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_unusedHandle, indent);
	file->writeNumberLine(_startingTicks, indent);
	file->writeNumberLine(_volume, indent);

	CBackground::save(file, indent);
}

void CBomb::load(SimpleFile *file) {
	file->readNumber();
	_active = file->readNumber();
	_numCorrectWheels = file->readNumber();
	_tappedCtr = file->readNumber();
	_hammerCtr = file->readNumber();
	_commentCtr = file->readNumber();
	_countdown = file->readNumber();
	_soundHandle = file->readNumber();
	_unusedHandle = file->readNumber();
	_startingTicks = file->readNumber();
	_volume = file->readNumber();

	CBackground::load(file);
}

bool CBomb::StatusChangeMsg(CStatusChangeMsg *msg) {
	// Check whether the wheels are corect
	CCheckCodeWheelsMsg checkMsg;
	checkMsg.execute(findRoom(), nullptr, MSGFLAG_SCAN);

	_numCorrectWheels = checkMsg._isCorrect ? CORRECT_WHEELS : 0;

	if (_numCorrectWheels == CORRECT_WHEELS) {
		// Nobody likes a smartass
		startAnimTimer("Disarmed", 2000);
		lockMouse();
	}

	_commentCtr = (_commentCtr % 1000) + 1;
	if (!(_commentCtr % 20) && _countdown < 995) {
		int val = getRandomNumber(5) + 25;
		if (_commentCtr < 20 || _commentCtr > 80)
			val = 28;

		CString name;
		switch (val) {
		case 25:
			name = "z#372.wav";
			break;
		case 26:
			name = "z#371.wav";
			break;
		case 27:
			name = "z#370.wav";
			break;
		case 28:
			name = "z#369.wav";
			break;
		case 29:
			name = "z#368.wav";
			break;
		default:
			name = "z#366.wav";
			break;
		}

		_soundHandle = queueSound(name, _soundHandle, _volume);
	}

	return true;
}

bool CBomb::EnterViewMsg(CEnterViewMsg *msg) {
	// WORKAROUND: Don't keep resetting wheels
	return true;
}

bool CBomb::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	playSound("z#62.wav");

	if (_active) {
		stopSound(_soundHandle);
		//stopSound(_unusedHandle);

		if (_numCorrectWheels < CORRECT_WHEELS) {
			_tappedCtr = MIN(_tappedCtr + 1, 23);

			CString name;
			switch (_tappedCtr) {
			case 18:
				name = "z#380.wav";
				break;
			case 19:
				name = "z#379.wav";
				break;
			case 20:
				name = "z#377.wav";
				break;
			case 21:
				name = "z#376.wav";
				break;
			case 22:
				name = "z#375.wav";
				break;
			default:
				name = "z#374.wav";
				break;
			}

			_soundHandle = queueSound(name, _soundHandle, _volume);
			_countdown = 999;
		}
	} else {
		_soundHandle = playSound("z#389.wav", _volume);
		_active = true;
		CActMsg actMsg("Arm Bomb");
		actMsg.execute("EndExplodeShip");
	}

	return true;
}

bool CBomb::EnterRoomMsg(CEnterRoomMsg *msg) {
	_tappedCtr = 17;
	_hammerCtr = 9;
	_commentCtr = 0;
	_startingTicks = getTicksCount();
	return true;
}

bool CBomb::ActMsg(CActMsg *msg) {
	if (msg->_action == "Hit") {
		playSound("z#63.wav");
		stopSound(_soundHandle);

		if (_hammerCtr < 17)
			++_hammerCtr;

		CString name;
		switch (_hammerCtr) {
		case 10:
			name = "z#388.wav";
			break;
		case 11:
			name = "z#387.wav";
			break;
		case 12:
			name = "z#386.wav";
			break;
		case 13:
			name = "z#385.wav";
			break;
		case 14:
			name = "z#384.wav";
			break;
		case 15:
			name = "z#383.wav";
			break;
		case 16:
			name = "z#382.wav";
			break;
		default:
			name = "z#381.wav";
			break;
		}

		_soundHandle = queueSound(name, _soundHandle, _volume);
		_countdown = 999;
	}

	return true;
}

bool CBomb::TurnOn(CTurnOn *msg) {
	if (!_active) {
		_soundHandle = playSound("z#389.wav", _volume);
		_active = true;

		// WORKAROUND: Only reset the code wheels back to 'O' value
		// when first arming the bomb, not whenever the bomb view is entered
		_numCorrectWheels = 2;
		CRoomItem *room = findRoom();
		for (CTreeItem *treeItem = room; treeItem; treeItem = treeItem->scan(room)) {
			CodeWheel *codeWheel = dynamic_cast<CodeWheel *>(treeItem);
			if (codeWheel)
				codeWheel->reset();
		}

		CActMsg actMsg("Arm Bomb");
		actMsg.execute("EndExplodeShip");
		addTimer(0);
	}

	changeView("Titania.Node 8.W", "");
	CActMsg actMsg("Titania.Node 8.N");
	actMsg.execute("BombNav");
	actMsg.execute("EnterBombRoom");

	return true;
}

bool CBomb::TimerMsg(CTimerMsg *msg) {
	if (msg->_action == "Disarmed") {
		stopSound(_soundHandle);
		playSound("z#364.wav", _volume);

		CActMsg actMsg1("Disarm Bomb");
		actMsg1.execute("EndExplodeShip");
		_active = false;
		CActMsg actMsg2("Titania.Node 5.N");
		actMsg2.execute("BombNav");
		actMsg2.execute("EnterBombNav");

		changeView("Titania.Node 8.W", "");
		changeView("Titania.Node 13.N", "");
		unlockMouse();
	}

	if (compareRoomNameTo("Titania")) {
		if (msg->_actionVal == 1 && getRandomNumber(9) == 0) {
			if (!_active)
				return true;

			CParrotSpeakMsg speakMsg("Bomb", "BombCountdown");
			speakMsg.execute("PerchedParrot");
		}

		if (_active) {
			if (!isSoundActive(_soundHandle)) {
				if (msg->_actionVal == 0) {
					addTimer(1, 1000, 0);
				} else {
					_soundHandle = 0;
					int hundreds = _countdown / 100;
					int remainder = _countdown % 100;

					if (_countdown >= 100) {
						// Play "x hundred and" or just "x hundred"
						CString hName = remainder ? HUNDREDS_AND_WAVS[hundreds] : HUNDREDS_WAVS[hundreds];
						_soundHandle = playSound(hName, _volume);
					}

					CString ctrName = COUNTDOWN_WAVS[remainder];
					if (_countdown == 10) {
						ctrName = "z#229.wav";
						_countdown = 998;
					}

					// Play the sub-hundred portion of the countdown amount
					if (_soundHandle > 0) {
						_soundHandle = queueSound(ctrName, _soundHandle, _volume);
					} else {
						_soundHandle = playSound(ctrName, _volume);
					}

					// Reduce countdown and schedule another timer
					--_countdown;
					addTimer(0, 1000, 0);
				}
			} else {
				// Bomb speech currently active, so schedule the method'
				// to re-trigger after 100ms to check if speech is finished
				addTimer(0, 100, 0);
			}
		}
	} else {
		// In rooms other than the bomb room
		if (_active) {
			--_countdown;
			addTimer(6000);

			if (_countdown < 11)
				_countdown = getRandomNumber(900) + 50;
		}
	}

	return true;
}

bool CBomb::TrueTalkGetStateValueMsg(CTrueTalkGetStateValueMsg *msg) {
	if (msg->_stateNum == 10)
		msg->_stateVal = _active ? 1 : 0;

	return true;
}

bool CBomb::SetFrameMsg(CSetFrameMsg *msg) {
	_volume = msg->_frameNumber;
	return true;
}

} // End of namespace Titanic
