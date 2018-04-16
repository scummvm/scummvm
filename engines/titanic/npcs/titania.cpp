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

#include "titanic/npcs/titania.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTitania, CCharacter)
	ON_MESSAGE(AddHeadPieceMsg)
	ON_MESSAGE(TakeHeadPieceMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CTitania::CTitania() : CCharacter() {
	_speechCentre = false;
	_olfactoryCentre = false;
	_centralCore = false;
	_visionCentre = false;
	_eye1 = false;
	_eye2 = false;
	_ear1 = false;
	_ear2 = false;
	_nose = false;
	_mouth = false;
	_showSpeech = true;
}

void CTitania::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_speechCentre, indent);
	file->writeNumberLine(_olfactoryCentre, indent);
	file->writeNumberLine(_auditoryCentre, indent);
	file->writeNumberLine(_centralCore, indent);
	file->writeNumberLine(_visionCentre, indent);
	file->writeNumberLine(_eye1, indent);
	file->writeNumberLine(_eye2, indent);
	file->writeNumberLine(_ear1, indent);
	file->writeNumberLine(_ear2, indent);
	file->writeNumberLine(_nose, indent);
	file->writeNumberLine(_mouth, indent);
	file->writeNumberLine(_showSpeech, indent);

	CCharacter::save(file, indent);
}

void CTitania::load(SimpleFile *file) {
	file->readNumber();
	_speechCentre = file->readNumber();
	_olfactoryCentre = file->readNumber();
	_auditoryCentre = file->readNumber();
	_centralCore = file->readNumber();
	_visionCentre = file->readNumber();
	_eye1 = file->readNumber();
	_eye2 = file->readNumber();
	_ear1 = file->readNumber();
	_ear2 = file->readNumber();
	_nose = file->readNumber();
	_mouth = file->readNumber();
	_showSpeech = file->readNumber();

	CCharacter::load(file);
}

bool CTitania::AddHeadPieceMsg(CAddHeadPieceMsg *msg) {
	if (msg->_value == "VisionCentre") {
		_visionCentre = true;
	} else if (msg->_value == "AuditoryCentre") {
		_auditoryCentre = true;
	} else if (msg->_value == "OlfactoryCentre") {
		_olfactoryCentre = true;
	} else if (msg->_value == "SpeechCentre") {
		_speechCentre = true;
	} else if (msg->_value == "CentralCore") {
		_centralCore = true;
	} else if (msg->_value == "Eye1") {
		_eye1 = true;
	} else if (msg->_value == "Eye2") {
		_eye2 = true;
	} else if (msg->_value == "Ear1") {
		_ear1 = true;
	} else if (msg->_value == "Ear 2") {
		_ear2 = true;
	} else if (msg->_value == "Mouth") {
		_mouth = true;
	} else if (msg->_value == "Nose") {
		_nose = true;
	}

	CActMsg actMsg("CheckHead");
	actMsg.execute(this);
	return true;
}

bool CTitania::TakeHeadPieceMsg(CTakeHeadPieceMsg *msg) {
	if (msg->_value == "VisionCentre") {
		_visionCentre = false;
	} else if (msg->_value == "AuditoryCentre") {
		_auditoryCentre = false;
	} else if (msg->_value == "OlfactoryCentre") {
		_olfactoryCentre = false;
	} else if (msg->_value == "SpeechCentre") {
		_speechCentre = false;
	} else if (msg->_value == "CentralCore") {
		_centralCore = false;
	} else if (msg->_value == "Eye1") {
		_eye1 = false;
	} else if (msg->_value == "Eye2") {
		_eye2 = false;
	} else if (msg->_value == "Ear1") {
		_ear1 = false;
	} else if (msg->_value == "Ear 2") {
		_ear2 = false;
	} else if (msg->_value == "Mouth") {
		_mouth = false;
	} else if (msg->_value == "Nose") {
		_nose = false;
	}

	CActMsg actMsg("CheckHead");
	actMsg.execute(this);
	return true;
}

bool CTitania::ActMsg(CActMsg *msg) {
	if (msg->_action == "SleepTitania") {
		setVisible(true);
		playCutscene(52, 104);
		playSound(TRANSLATE("z#47.wav", "z#578.wav"), 100);
		changeView("Titania.Node 7.S", "");

		// Re-enable control, and reset bomb's volume back to normal 60%
		petShow();
		enableMouse();
		CSetFrameMsg frameMsg(60);
		frameMsg.execute("Bomb");

	} else if (msg->_action == "CheckHead") {
		CSenseWorkingMsg workingMsg1("Not Working");
		CSenseWorkingMsg workingMsg2("Not Working");
		CSenseWorkingMsg workingMsg3("Not Working");
		CSenseWorkingMsg workingMsg4("Not Working");

		if (_eye1 && _eye2) {
			workingMsg1._value = _visionCentre ? "Working" : "Random";
		}
		if (_ear1 && _ear2) {
			workingMsg2._value = _auditoryCentre ? "Working" : "Random";
		}
		if (_nose) {
			workingMsg4._value = _olfactoryCentre ? "Working" : "Random";
		}
		if (_mouth) {
			workingMsg3._value = _speechCentre ? "Working" : "Random";
		}

		if (_centralCore && _eye1 && _eye2 && _ear1 && _ear2 && _nose
				&& _mouth && _visionCentre && _speechCentre
				&& _olfactoryCentre && _auditoryCentre) {
			CProximity prox(Audio::Mixer::kSpeechSoundType);
			playSound(TRANSLATE("z#47.wav", "z#578.wav"), prox);

			CActMsg actMsg("Woken");
			actMsg.execute("MouthSlot");
			actMsg.execute("VisionCentreSlot");
			setPassengerClass(UNCHECKED);

			addTimer(1000);
		} else {
			workingMsg1.execute("Eye1Slot");
			workingMsg1.execute("Eye2Slot");
			workingMsg2.execute("Ear1Slot");
			workingMsg2.execute("Ear2Slot");
			workingMsg3.execute("MouthSlot");
			workingMsg4.execute("NoseSlot");
		}
	}

	return true;
}

bool CTitania::EnterViewMsg(CEnterViewMsg *msg) {
	if (_showSpeech) {
		_showSpeech = false;
		disableMouse();
		petHide();

		// The Bomb uses the CSetFrameMsg as a hack for setting the volume.
		// In case it's currently active, set it to a quieter 25% so that
		// it won't obscure Titania's speech.
		CSetFrameMsg frameMsg;
		frameMsg._frameNumber = 25;
		frameMsg.execute("Bomb");

		playCutscene(0, 52);
		setVisible(false);
		CActMsg actMsg("TitaniaSpeech");
		actMsg.execute("TitaniaSpeech");
	}

	return true;
}

bool CTitania::TimerMsg(CTimerMsg *msg) {
	// WORKAROUND: The original uses the disc change dialog as a pause
	// to allow the parrot speech to finish. I've rewritten it to instead
	// use the standard TrueTalkNotifySpeechEndedMsg message instead
	startTalking("PerchedParrot", 80022);
	lockMouse();

	return true;
}

} // End of namespace Titanic
