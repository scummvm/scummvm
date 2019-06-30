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
#include "titanic/translation.h"

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

static const char *const HUNDREDS_WAVS_EN[] = {
	"", "z#353.wav", "z#339.wav", "z#325.wav", "z#311.wav", "z#297.wav",
	"z#283.wav", "z#269.wav", "z#255.wav", "z#241.wav"
};

static const char *const HUNDREDS_AND_WAVS_EN[] = {
	"", "z#352.wav", "z#338.wav", "z#324.wav", "z#310.wav", "z#296.wav",
	"z#281.wav", "z#268.wav", "z#254.wav", "z#240.wav"
};

static const char *const COUNTDOWN_WAVS_EN[100] = {
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

const char *const HUNDREDS_WAVS_DE[10] = {
	"z#56.wav", "z#54.wav", "z#53.wav", "z#52.wav", "z#51.wav",
	"z#50.wav", "z#49.wav", "z#48.wav", "z#47.wav", "z#55.wav"
};

const char *const ONE_TO_NINETEEN_WAVS_DE[19] = {
	"z#15.wav", "z#97.wav", "z#95.wav", "z#86.wav", "z#84.wav",
	"z#82.wav", "z#80.wav", "z#78.wav", "z#76.wav", "z#74.wav",
	"z#73.wav", "z#72.wav", "z#71.wav", "z#70.wav", "z#69.wav",
	"z#68.wav", "z#67.wav", "z#66.wav", "z#65.wav"
};

const char *const TENS_WAVS_DE[9] = {
	"z#98.wav", "z#96.wav", "z#92.wav", "z#85.wav", "z#83.wav",
	"z#81.wav", "z#79.wav", "z#77.wav", "z#75.wav"
};

const char *const DIGITS_WAVS_DE[9] = {
	"z#74.wav", "z#64.wav", "z#63.wav", "z#62.wav", "z#61.wav",
	"z#60.wav", "z#59.wav", "z#58.wav", "z#57.wav"
};

const char *const WAVES_970_DE[30] = {
	"z#46.wav", "z#45.wav", "z#44.wav", "z#43.wav", "z#42.wav",
	"z#41.wav", "z#40.wav", "z#39.wav", "z#38.wav", "z#37.wav",
	"z#36.wav", "z#35.wav", "z#34.wav", "z#33.wav", "z#32.wav",
	"z#31.wav", "z#30.wav", "z#29.wav", "z#28.wav", "z#27.wav",
	"z#26.wav", "z#25.wav", "z#24.wav", "z#23.wav", "z#22.wav",
	"z#21.wav", "z#20.wav", "z#19.wav", "z#18.wav", "z#17.wav"
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
			name = TRANSLATE("z#372.wav", "z#115.wav");
			break;
		case 26:
			name = TRANSLATE("z#371.wav", "z#114.wav");
			break;
		case 27:
			name = TRANSLATE("z#370.wav", "z#113.wav");
			break;
		case 28:
			name = TRANSLATE("z#369.wav", "z#112.wav");
			break;
		case 29:
			name = TRANSLATE("z#368.wav", "z#111.wav");
			break;
		default:
			name = TRANSLATE("z#366.wav", "z#109.wav");
			break;
		}

		_soundHandle = queueSound(name, _soundHandle, _volume, 0, false, Audio::Mixer::kSpeechSoundType);
	}

	return true;
}

bool CBomb::EnterViewMsg(CEnterViewMsg *msg) {
	// WORKAROUND: Don't keep resetting wheels
	return true;
}

bool CBomb::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	playSound(TRANSLATE("z#62.wav", "z#593.wav"));

	if (_active) {
		stopSound(_soundHandle);

		if (_numCorrectWheels < CORRECT_WHEELS) {
			_tappedCtr = MIN(_tappedCtr + 1, 23);

			CString name;
			switch (_tappedCtr) {
			case 18:
				name = TRANSLATE("z#380.wav", "z#122.wav");
				break;
			case 19:
				name = TRANSLATE("z#379.wav", "z#121.wav");
				break;
			case 20:
				name = TRANSLATE("z#377.wav", "z#119.wav");
				break;
			case 21:
				name = TRANSLATE("z#376.wav", "z#118.wav");
				break;
			case 22:
				name = TRANSLATE("z#375.wav", "z#117.wav");
				break;
			default:
				name = TRANSLATE("z#374.wav", "z#116.wav");
				break;
			}

			_soundHandle = queueSound(name, _soundHandle, _volume, 0, false, Audio::Mixer::kSpeechSoundType);
			_countdown = 999;
		}
	} else {
		_soundHandle = playSound(TRANSLATE("z#389.wav", "z#131.wav"), _volume);
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
		playSound(TRANSLATE("z#63.wav", "z#594.wav"));
		stopSound(_soundHandle);

		if (_hammerCtr < 17)
			++_hammerCtr;

		CString name;
		switch (_hammerCtr) {
		case 10:
			name = TRANSLATE("z#388.wav", "z#130.wav");
			break;
		case 11:
			name = TRANSLATE("z#387.wav", "z#129.wav");
			break;
		case 12:
			name = TRANSLATE("z#386.wav", "z#128.wav");
			break;
		case 13:
			name = TRANSLATE("z#385.wav", "z#127.wav");
			break;
		case 14:
			name = TRANSLATE("z#384.wav", "z#126.wav");
			break;
		case 15:
			name = TRANSLATE("z#383.wav", "z#125.wav");
			break;
		case 16:
			name = TRANSLATE("z#382.wav", "z#124.wav");
			break;
		default:
			name = TRANSLATE("z#381.wav", "z#123.wav");
			break;
		}

		_soundHandle = queueSound(name, _soundHandle, _volume, 0, false, Audio::Mixer::kSpeechSoundType);
		_countdown = 999;
	}

	return true;
}

bool CBomb::TurnOn(CTurnOn *msg) {
	if (!_active) {
		CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
		_soundHandle = playSound(TRANSLATE("z#389.wav", "z#131.wav"), prox);
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
		CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
		stopSound(_soundHandle);
		playSound(TRANSLATE("z#364.wav", "z#107.wav"), prox);

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

	if (!compareRoomNameTo("Titania")) {
		// In rooms other than the bomb room
		if (_active) {
			--_countdown;
			addTimer(6000);

			if (_countdown < 11)
				_countdown = getRandomNumber(900) + 50;
		}

		return true;
	}

	if (msg->_actionVal == 1 && getRandomNumber(9) == 0) {
		if (!_active)
			return true;

		CParrotSpeakMsg speakMsg("Bomb", "BombCountdown");
		speakMsg.execute("PerchedParrot");
	}

	// Don't execute if the bomb isn't actually active
	if (!_active)
		return true;

	if (isSoundActive(_soundHandle)) {
		// Bomb speech currently active, so schedule the method
		// to re-trigger after 100ms to check if speech is finished
		addTimer(0, 100, 0);
		return true;
	}

	if (msg->_actionVal == 0) {
		addTimer(1, 1000, 0);
	} else {
		_soundHandle = 0;
		int hundreds = _countdown / 100;
		int remainder = _countdown % 100;

		if (g_language == Common::DE_DEU) {
			if (_countdown <= 10) {
				// Reset countdown back to 1000
				CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
				_soundHandle = playSound("z#14.wav", prox);
				_countdown = 999;
			} else {
				if (_countdown >= 970) {
					// Sounds for numbers 970 to 999
					CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
					_soundHandle = playSound(WAVES_970_DE[_countdown - 970], prox);
				} else {
					if (hundreds >= 1) {
						CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
						_soundHandle = playSound(HUNDREDS_WAVS_DE[hundreds - 1], prox);
					}

					if (remainder >= 20) {
						int tens = remainder / 10;
						int digit = remainder % 10;

						// Tens
						const char *tensStr = TENS_WAVS_DE[tens - 1];
						if (_soundHandle) {
							_soundHandle = queueSound(tensStr, _soundHandle,
								_volume, 0, false, Audio::Mixer::kSpeechSoundType);
						} else {
							CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
							_soundHandle = playSound(tensStr, prox);
						}

						// Digit
						if (digit != 0) {
							const char *digitStr = DIGITS_WAVS_DE[digit - 1];
							_soundHandle = queueSound(digitStr, _soundHandle,
								_volume, 0, false, Audio::Mixer::kSpeechSoundType);
						}
					} else if (remainder != 0) {
						// One to nineteen
						const char *name = ONE_TO_NINETEEN_WAVS_DE[remainder - 1];
						if (_soundHandle) {
							_soundHandle = queueSound(name, _soundHandle,
								_volume, 0, false, Audio::Mixer::kSpeechSoundType);
						} else {
							CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
							_soundHandle = playSound(name, prox);
						}
					}
				}
			}
		} else {
			if (_countdown >= 100) {
				// Play "x hundred and" or just "x hundred"
				CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
				CString hName = remainder ? HUNDREDS_AND_WAVS_EN[hundreds] : HUNDREDS_WAVS_EN[hundreds];
				_soundHandle = playSound(hName, prox);
			}

			CString ctrName = COUNTDOWN_WAVS_EN[remainder];
			if (_countdown == 10) {
				ctrName = "z#229.wav";
				_countdown = 998;
			}

			// Play the sub-hundred portion of the countdown amount
			if (_soundHandle > 0) {
				_soundHandle = queueSound(ctrName, _soundHandle, _volume, 0, false, Audio::Mixer::kSpeechSoundType);
			} else {
				CProximity prox(Audio::Mixer::kSpeechSoundType, _volume);
				_soundHandle = playSound(ctrName, prox);
			}
		}

		// Reduce countdown and schedule another timer
		--_countdown;
		addTimer(0, 1000, 0);
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
