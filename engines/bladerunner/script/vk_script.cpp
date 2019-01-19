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

#include "bladerunner/script/vk_script.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/mouse.h"

namespace BladeRunner {

VKScript::VKScript(BladeRunnerEngine *vm) : ScriptBase(vm) {
	_inScriptCounter = 0;
	_calibrationQuestionCounter = 0;
	_questionCounter = 0;
}

void VKScript::initialize(int actorId) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_Initialize(actorId);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

void VKScript::calibrate(int actorId) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_Calibrate(actorId);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

void VKScript::beginTest(int actorId) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_Begin_Test(actorId);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

void VKScript::mcCoyAsksQuestion(int actorId, int questionId) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_McCoy_Asks_Question(actorId, questionId);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

void VKScript::questionAsked(int actorId, int questionId) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_Question_Asked(actorId, questionId);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

void VKScript::shutdown(int actorId, int humanPercentage, int replicantPercentage, int anxiety) {
	++_inScriptCounter;
	_vm->_mouse->disable();
	SCRIPT_VK_DLL_Shutdown(actorId, humanPercentage, replicantPercentage, anxiety);
	_vm->_mouse->enable();
	--_inScriptCounter;
}

bool VKScript::isInsideScript() const {
	return _inScriptCounter > 0;
}

bool VKScript::SCRIPT_VK_DLL_Initialize(int actorId) {
	VK_Add_Question(0, 7400, -1);
	VK_Add_Question(0, 7405, -1);
	VK_Add_Question(0, 7410, -1);
	VK_Add_Question(0, 7415, -1);
	VK_Add_Question(0, 7420, -1);
	VK_Add_Question(0, 7425, -1);
	if (actorId == kActorLucy || actorId == kActorDektora) {
		VK_Add_Question(0, 7430, -1);
	}
	VK_Add_Question(0, 7435, -1);
	VK_Add_Question(0, 7440, -1);
	VK_Add_Question(0, 7445, -1);
	VK_Add_Question(0, 7450, -1);
	VK_Add_Question(0, 7455, -1);
	VK_Add_Question(0, 7460, -1);
	VK_Add_Question(0, 7465, -1);
	VK_Add_Question(0, 7470, -1);
	VK_Add_Question(1, 7475, -1);
	VK_Add_Question(1, 7480, -1);
	VK_Add_Question(1, 7485, -1);
	VK_Add_Question(1, 7490, -1);
	VK_Add_Question(1, 7495, -1);
	VK_Add_Question(1, 7515, -1);
	VK_Add_Question(1, 7525, -1);
	VK_Add_Question(1, 7535, -1);
	VK_Add_Question(1, 7540, -1);
	VK_Add_Question(1, 7550, -1);
	VK_Add_Question(1, 7565, -1);
	VK_Add_Question(1, 7580, -1);
	VK_Add_Question(1, 7585, -1);
	VK_Add_Question(1, 7595, -1);
	VK_Add_Question(1, 7600, -1);
	VK_Add_Question(2, 7605, -1);
	VK_Add_Question(2, 7620, -1);
	VK_Add_Question(2, 7635, -1);
	VK_Add_Question(2, 7670, -1);
	VK_Add_Question(2, 7680, -1);
	VK_Add_Question(2, 7690, -1);
	VK_Add_Question(2, 7705, -1);
	VK_Add_Question(2, 7740, -1);
	VK_Add_Question(2, 7750, -1);
	VK_Add_Question(2, 7770, -1);

	switch (actorId) {
	case kActorDektora:
	case kActorLucy:
	case kActorGrigorian:
	case kActorBulletBob:
	case kActorRunciter:
		return true;
	default:
		return false;
	}
}

void VKScript::SCRIPT_VK_DLL_Calibrate(int actorId) {
	if (_calibrationQuestionCounter == 0) {
		VK_Play_Speech_Line(kActorMcCoy, 7370, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7385, 0.5f);
		askCalibrationQuestion1(actorId, 7385);
	} else if (_calibrationQuestionCounter == 1) {
		VK_Play_Speech_Line(kActorMcCoy, 7390, 0.5f);
		askCalibrationQuestion2(actorId, 7390);
	} else if (_calibrationQuestionCounter == 2) {
		VK_Play_Speech_Line(kActorMcCoy, 7395, 0.5f);
		askCalibrationQuestion3(actorId, 7395);
	}
	if (++_calibrationQuestionCounter > 3) {
		_calibrationQuestionCounter = 0;
	}
}

bool VKScript::SCRIPT_VK_DLL_Begin_Test(int actorId) {
	_questionCounter = 0;
	return false;
}

void VKScript::SCRIPT_VK_DLL_McCoy_Asks_Question(int actorId, int questionId) {
	switch (questionId) {
	case 7400:
		VK_Play_Speech_Line(kActorMcCoy, 7400, 0.5f);
		break;
	case 7405:
		VK_Play_Speech_Line(kActorMcCoy, 7405, 0.5f);
		break;
	case 7410:
		VK_Play_Speech_Line(kActorMcCoy, 7410, 0.5f);
		break;
	case 7415:
		VK_Play_Speech_Line(kActorMcCoy, 7415, 0.5f);
		break;
	case 7420:
		VK_Play_Speech_Line(kActorMcCoy, 7420, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(kActorMcCoy, 7425, 0.5f);
		break;
	case 7430:
		VK_Play_Speech_Line(kActorMcCoy, 7430, 0.5f);
		break;
	case 7435:
		VK_Play_Speech_Line(kActorMcCoy, 7435, 0.5f);
		break;
	case 7440:
		VK_Play_Speech_Line(kActorMcCoy, 7440, 0.5f);
		break;
	case 7445:
		VK_Play_Speech_Line(kActorMcCoy, 7445, 0.5f);
		break;
	case 7450:
		VK_Play_Speech_Line(kActorMcCoy, 7450, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(kActorMcCoy, 7455, 0.5f);
		break;
	case 7460:
		VK_Play_Speech_Line(kActorMcCoy, 7460, 0.5f);
		break;
	case 7465:
		VK_Play_Speech_Line(kActorMcCoy, 7465, 0.5f);
		break;
	case 7470:
		VK_Play_Speech_Line(kActorMcCoy, 7470, 0.5f);
		break;
	case 7475:
		VK_Play_Speech_Line(kActorMcCoy, 7475, 0.5f);
		break;
	case 7480:
		VK_Play_Speech_Line(kActorMcCoy, 7480, 0.5f);
		break;
	case 7485:
		VK_Play_Speech_Line(kActorMcCoy, 7485, 0.5f);
		break;
	case 7490:
		VK_Play_Speech_Line(kActorMcCoy, 7490, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(kActorMcCoy, 7495, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7500, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7505, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7510, 0.5f);
		break;
	case 7515:
		VK_Play_Speech_Line(kActorMcCoy, 7515, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7520, 0.5f);
		break;
	case 7525:
		VK_Play_Speech_Line(kActorMcCoy, 7525, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7530, 0.5f);
		break;
	case 7535:
		VK_Play_Speech_Line(kActorMcCoy, 7535, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(kActorMcCoy, 7540, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7545, 0.5f);
		break;
	case 7550:
		VK_Play_Speech_Line(kActorMcCoy, 7550, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7555, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7560, 0.5f);
		break;
	case 7565:
		VK_Play_Speech_Line(kActorMcCoy, 7565, 0.5f);
		if (actorId != kActorBulletBob) {
			VK_Play_Speech_Line(kActorMcCoy, 7570, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7575, 0.5f);
		}
		break;
	case 7580:
		VK_Play_Speech_Line(kActorMcCoy, 7580, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(kActorMcCoy, 7585, 0.5f);
		if (actorId != kActorLucy && actorId != kActorBulletBob) {
			VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
		}
		break;
	case 7595:
		VK_Play_Speech_Line(kActorMcCoy, 7595, 0.5f);
		break;
	case 7600:
		VK_Play_Speech_Line(kActorMcCoy, 7600, 0.5f);
		break;
	case 7605:
		VK_Play_Speech_Line(kActorMcCoy, 7605, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7610, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7615, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(kActorMcCoy, 7620, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7625, 0.5f);
		if (actorId != kActorBulletBob) {
			if (actorId == kActorDektora && Game_Flag_Query(kFlagDektoraIsReplicant)) {
				VK_Play_Speech_Line(kActorDektora, 2330, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7880, 0.5f);
			}
			VK_Play_Speech_Line(kActorMcCoy, 7630, 0.5f);
		}
		break;
	case 7635:
		VK_Play_Speech_Line(kActorMcCoy, 7635, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7640, 0.5f);
		if (actorId != kActorGrigorian && actorId != kActorBulletBob) {
			VK_Play_Speech_Line(kActorMcCoy, 7645, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7650, 0.5f);
			if (actorId != kActorLucy) {
				VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
			}
		}
		break;
	case 7670:
		VK_Play_Speech_Line(kActorMcCoy, 7670, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7675, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(kActorMcCoy, 7680, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7685, 0.5f);
		break;
	case 7690:
		VK_Play_Speech_Line(kActorMcCoy, 7690, 0.5f);
		if (actorId != kActorGrigorian) {
			VK_Play_Speech_Line(kActorMcCoy, 7695, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7700, 0.5f);
		}
		break;
	case 7705:
		VK_Play_Speech_Line(kActorMcCoy, 7705, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7710, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7715, 0.5f);
		if (actorId != kActorGrigorian && actorId != kActorBulletBob) {
			VK_Play_Speech_Line(kActorMcCoy, 7720, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7725, 0.5f);
			if (actorId != kActorLucy) {
				if (actorId == kActorDektora) {
					VK_Play_Speech_Line(kActorDektora, 2490, 0.5f);
				}
				VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
			}
		}
		break;
	case 7740:
		VK_Play_Speech_Line(kActorMcCoy, 7740, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7745, 0.5f);
		break;
	case 7750:
		VK_Play_Speech_Line(kActorMcCoy, 7750, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7755, 0.5f);
		if (actorId == kActorDektora) {
			VK_Play_Speech_Line(kActorDektora, 2570, 0.5f);
		}
		VK_Play_Speech_Line(kActorMcCoy, 7760, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7765, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(kActorMcCoy, 7770, 0.5f);
		if (actorId == kActorDektora) {
			VK_Play_Speech_Line(kActorDektora, 2620, 0.5f);
		}
		VK_Play_Speech_Line(kActorMcCoy, 7775, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7780, 0.5f);
		break;
	}
	if (++_questionCounter >= 10) {
		VK_Subject_Reacts(5, 0, 0, 100);
	}
}

void VKScript::SCRIPT_VK_DLL_Question_Asked(int actorId, int questionId) {
	switch (actorId) {
	case kActorRunciter:
		askRunciter(questionId);
		break;
	case kActorBulletBob:
		askBulletBob(questionId);
		break;
	case kActorGrigorian:
		askGrigorian(questionId);
		break;
	case kActorLucy:
		askLucy(questionId);
		break;
	case kActorDektora:
		askDektora(questionId);
		break;
	}
}

void VKScript::SCRIPT_VK_DLL_Shutdown(int actorId, int humanPercentage, int replicantPercentage, int anxiety) {
	if (humanPercentage > 79 && replicantPercentage > 79) {
		VK_Play_Speech_Line(kActorAnsweringMachine, 450, 0.5f);
	} else if (replicantPercentage > 79) {
		VK_Play_Speech_Line(kActorAnsweringMachine, 420, 0.5f);
		VK_Play_Speech_Line(kActorAnsweringMachine, 430, 0.5f);
		switch (actorId) {
		case kActorRunciter:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKRunciterReplicant, true, -1);
			break;
		case kActorBulletBob:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKBobGorskyReplicant, true, -1);
			break;
		case kActorGrigorian:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKGrigorianReplicant, true, -1);
			break;
		case kActorLucy:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKLucyReplicant, true, -1);
			break;
		case kActorDektora:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKDektoraReplicant, true, -1);
			break;
		}
	} else if (humanPercentage > 79) {
		VK_Play_Speech_Line(kActorAnsweringMachine, 420, 0.5f);
		VK_Play_Speech_Line(kActorAnsweringMachine, 440, 0.5f);
		switch (actorId) {
		case kActorRunciter:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKRunciterHuman, true, -1);
			break;
		case kActorBulletBob:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKBobGorskyHuman, true, -1);
			break;
		case kActorGrigorian:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKGrigorianHuman, true, -1);
			break;
		case kActorLucy:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKLucyHuman, true, -1);
			break;
		case kActorDektora:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKDektoraHuman, true, -1);
			break;
		}
	}
	VK_Play_Speech_Line(kActorAnsweringMachine, 460, 0.5f);
}

void VKScript::askLucy(int questionId) {
	switch (questionId) {
	case 7385:
		VK_Subject_Reacts(40, 0, 0, 0);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 1240, 0.5f);
		break;
	case 7390:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1250, 0.5f);
			VK_Subject_Reacts(40, 0, 2, 5);
			VK_Play_Speech_Line(kActorLucy, 1260, 0.5f);
			VK_Eye_Animates(3);
		} else {
			VK_Play_Speech_Line(kActorLucy, 1270, 0.5f);
			VK_Subject_Reacts(40, 3, 0, 2);
		}
		break;
	case 7395:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1280, 0.5f);
			VK_Subject_Reacts(40, 0, 0, 0);
		} else {
			VK_Subject_Reacts(40, 0, 0, 0);
			VK_Play_Speech_Line(kActorLucy, 1280, 0.5f);
		}
		break;
	case 7400:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 0, 8, -5);
		} else {
			VK_Subject_Reacts(30, 9, 0, -10);
		}
		VK_Play_Speech_Line(kActorLucy, 1300, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7405:
		VK_Play_Speech_Line(kActorLucy, 1310, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(50, 1, 11, 5);
		} else {
			VK_Subject_Reacts(60, 11, 1, 5);
		}
		VK_Play_Speech_Line(kActorLucy, 1320, 0.5f);
		break;
	case 7410:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 1, 10, -5);
			VK_Eye_Animates(2);
		} else {
			VK_Subject_Reacts(40, 10, -2, -5);
		}
		VK_Play_Speech_Line(kActorLucy, 1330, 0.5f);
		break;
	case 7415:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1340, 0.5f);
			VK_Subject_Reacts(50, 1, 11, -5);
			VK_Play_Speech_Line(kActorMcCoy, 7935, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1350, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorLucy, 1360, 0.5f);
			VK_Subject_Reacts(20, 9, -2, -5);
		}
		break;
	case 7420:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 1370, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8000, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(50, 1, 11, -8);
		} else {
			VK_Subject_Reacts(60, 11, -2, -8);
		}
		VK_Play_Speech_Line(kActorLucy, 1390, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(kActorLucy, 1400, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -2, 9, -2);
		} else {
			VK_Subject_Reacts(20, 9, -2, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1410, 0.5f);
		break;
	case 7430:
		VK_Play_Speech_Line(kActorLucy, 1420, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -1, 9, -3);
		} else {
			VK_Subject_Reacts(30, 9, -1, -3);
		}
		VK_Play_Speech_Line(kActorLucy, 1430, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7940, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7435:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1440, 0.5f);
			VK_Subject_Reacts(30, 5, 10, 2);
		} else {
			VK_Subject_Reacts(30, 9, 3, 2);
			VK_Play_Speech_Line(kActorLucy, 1450, 0.5f);
		}
		break;
	case 7440:
		VK_Play_Speech_Line(kActorLucy, 1460, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 5, 10, 2);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(20, 9, -3, 2);
		}
		VK_Play_Speech_Line(kActorLucy, 1470, 0.5f);
		break;
	case 7445:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1480, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7940, 0.5f);
			VK_Subject_Reacts(50, 4, 11, 10);
			VK_Play_Speech_Line(kActorLucy, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(30, 9, -1, -2);
			VK_Play_Speech_Line(kActorLucy, 1510, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7945, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1520, 0.5f);
		}
		break;
	case 7450:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 3, 9, -6);
		} else {
			VK_Subject_Reacts(30, 9, 4, -6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1530, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(kActorLucy, 1540, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7950, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(10, 1, 8, -5);
		} else {
			VK_Subject_Reacts(10, 9, -1, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1550, 0.5f);
		break;
	case 7460:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(30, 1, 10, -5);
		} else {
			VK_Subject_Reacts(30, 9, 2, -5);
		}
		VK_Play_Speech_Line(kActorLucy, 1560, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7955, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 1570, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1580, 0.5f);
		break;
	case 7465:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, -1, 11, 2);
			VK_Play_Speech_Line(kActorLucy, 1590, 0.5f);
		} else {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(kActorLucy, 1590, 0.5f);
			VK_Subject_Reacts(20, 9, 2, -8);
		}
		break;
	case 7470:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1610, 0.5f);
			VK_Subject_Reacts(20, 3, 9, -5);
			VK_Play_Speech_Line(kActorLucy, 1620, 0.5f);
		} else {
			VK_Subject_Reacts(30, 9, 0, -5);
			VK_Play_Speech_Line(kActorLucy, 1630, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1640, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7960, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1650, 0.5f);
		}
		break;
	case 7475:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorLucy, 1660, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7965, 0.5f);
			VK_Subject_Reacts(70, -3, 13, 10);
			VK_Play_Speech_Line(kActorLucy, 1670, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorLucy, 1680, 0.5f);
			VK_Subject_Reacts(60, 13, -4, 5);
			VK_Play_Speech_Line(kActorLucy, 1690, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7970, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorLucy, 1700, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1710, 0.5f);
		}
		break;
	case 7480:
		VK_Play_Speech_Line(kActorLucy, 1720, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7975, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 2, 10, 7);
		} else {
			VK_Subject_Reacts(50, 12, 3, 7);
		}
		VK_Play_Speech_Line(kActorLucy, 1730, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7980, 0.5f);
		break;
	case 7485:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -2, 10, 6);
		} else {
			VK_Subject_Reacts(30, 10, -2, 6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1740, 0.5f);
		break;
	case 7490:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(30, 3, 11, 9);
		} else {
			VK_Subject_Reacts(40, 11, 1, 8);
		}
		VK_Play_Speech_Line(kActorLucy, 1750, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7985, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7990, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1760, 0.5f);
		break;
	case 7495:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(10, 2, 8, 5);
		} else {
			VK_Subject_Reacts(30, 10, -1, 5);
		}
		VK_Play_Speech_Line(kActorLucy, 1770, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 1780, 0.5f);
		break;
	case 7515:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(50, -1, 12, 5);
			VK_Play_Speech_Line(kActorLucy, 1790, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7995, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 1800, 0.5f);
		} else {
			VK_Subject_Reacts(30, 10, 0, 3);
			VK_Play_Speech_Line(kActorLucy, 1810, 0.5f);
		}
		break;
	case 7525:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -4, 9, 5);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(40, 11, -3, 7);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1820, 0.5f);
		break;
	case 7535:
		VK_Play_Speech_Line(kActorLucy, 1830, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8000, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(10, 1, 8, 0);
		} else {
			VK_Subject_Reacts(20, 9, -1, 0);
		}
		VK_Play_Speech_Line(kActorLucy, 1840, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1850, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8005, 0.5f);
		break;
	case 7540:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorLucy, 1860, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8010, 0.5f);
			VK_Subject_Reacts(50, -2, 12, 5);
			VK_Play_Speech_Line(kActorLucy, 1870, 0.5f);
		} else {
			VK_Subject_Reacts(60, 12, -2, 5);
			VK_Play_Speech_Line(kActorLucy, 1880, 0.5f);
		}
		break;
	case 7550:
		VK_Play_Speech_Line(kActorLucy, 1890, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 2, 9, -1);
		} else {
			VK_Subject_Reacts(10, 8, -1, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1900, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8015, 0.5f);
		break;
	case 7565:
		VK_Play_Speech_Line(kActorLucy, 1910, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(30, -2, 10, 8);
		} else {
			VK_Subject_Reacts(20, 9, -3, 6);
		}
		VK_Play_Speech_Line(kActorLucy, 1920, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8020, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7580:
		VK_Play_Speech_Line(kActorLucy, 1930, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 5, 10, -1);
		} else {
			VK_Subject_Reacts(30, 10, 3, 0);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8025, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1940, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(kActorLucy, 1950, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8030, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1960, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8035, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 1970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1980, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 1, 11, 5);
		} else {
			VK_Subject_Reacts(50, 12, -3, 5);
		}
		VK_Play_Speech_Line(kActorLucy, 1990, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8040, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 2000, 0.5f);
		break;
	case 7595:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 2010, 0.5f);
			VK_Subject_Reacts(30, -2, 10, 5);
			VK_Play_Speech_Line(kActorLucy, 2020, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8045, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 2030, 0.5f);
		} else {
			VK_Subject_Reacts(60, 12, -3, 7);
			VK_Play_Speech_Line(kActorLucy, 2040, 0.5f);
		}
		break;
	case 7600:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(75, 4, 13, 15);
		} else {
			VK_Subject_Reacts(60, 12, -2, 10);
		}
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 2050, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8050, 0.5f);
		VK_Subject_Reacts(90, 0, 0, 0);
		VK_Play_Speech_Line(kActorLucy, 2060, 0.5f);
		break;
	case 7605:
		VK_Play_Speech_Line(kActorLucy, 2070, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(30, 0, 11, 12);
		} else {
			VK_Subject_Reacts(50, 10, -3, 15);
		}
		VK_Play_Speech_Line(kActorLucy, 2080, 0.5f);
		break;
	case 7620:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(60, -2, 14, 12);
		} else {
			VK_Subject_Reacts(70, 9, -1, 10);
		}
		VK_Play_Speech_Line(kActorLucy, 2090, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8055, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 2100, 0.5f);
		break;
	case 7635:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 2110, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(80, 5, 14, 25);
		} else {
			VK_Subject_Reacts(70, 9, -2, 20);
		}
		VK_Play_Speech_Line(kActorLucy, 2120, 0.5f);
		break;
	case 7670:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 2130, 0.5f);
			VK_Subject_Reacts(30, -3, 11, 8);
			VK_Play_Speech_Line(kActorLucy, 2140, 0.5f);
		} else {
			VK_Subject_Reacts(20, 10, 0, 5);
			VK_Play_Speech_Line(kActorLucy, 2150, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 2160, 0.5f);
		}
		break;
	case 7680:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 2170, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(65, 1, 6, 5);
		} else {
			VK_Subject_Reacts(50, 10, 3, 4);
		}
		VK_Play_Speech_Line(kActorLucy, 2180, 0.5f);
		break;
	case 7690:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 0, 12, 0);
		} else {
			VK_Subject_Reacts(50, 13, 0, 0);
		}
		VK_Play_Speech_Line(kActorLucy, 2190, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8060, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 2200, 0.5f);
		VK_Subject_Reacts(30, -4, -4, -5);
		VK_Play_Speech_Line(kActorLucy, 2210, 0.5f);
		break;
	case 7705:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Eye_Animates(3);
			VK_Subject_Reacts(30, 0, 0, 0);
			VK_Play_Speech_Line(kActorLucy, 2220, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
			VK_Subject_Reacts(50, 2, 10, 12);
			VK_Play_Speech_Line(kActorLucy, 2230, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8065, 0.5f);
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(kActorLucy, 2240, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8070, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8075, 0.5f);
		} else {
			VK_Eye_Animates(2);
			VK_Subject_Reacts(50, 0, 0, 0);
			VK_Play_Speech_Line(kActorLucy, 2250, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
			VK_Eye_Animates(3);
			VK_Subject_Reacts(60, 12, 2, 12);
			VK_Play_Speech_Line(kActorLucy, 2230, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 2270, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8080, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 2280, 0.5f);
		}
		break;
	case 7740:
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, -3, -1, 3);
		} else {
			VK_Subject_Reacts(50, -1, -3, 3);
		}
		VK_Play_Speech_Line(kActorLucy, 2290, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8085, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 2300, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8090, 0.5f);
		VK_Subject_Reacts(20, 2, 2, 0);
		VK_Play_Speech_Line(kActorLucy, 2310, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 2320, 0.5f);
		break;
	case 7750:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(60, 4, 10, 15);
		} else {
			VK_Subject_Reacts(80, 12, -3, 18);
		}
		VK_Play_Speech_Line(kActorLucy, 2330, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8095, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 2340, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(kActorLucy, 2350, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(90, -3, -1, 12);
		} else {
			VK_Eye_Animates(2);
			VK_Subject_Reacts(90, 13, -4, 12);
		}
		VK_Play_Speech_Line(kActorLucy, 2360, 0.5f);
		break;
	}
}

void VKScript::askGrigorian(int questionId) {
	switch (questionId) {
	case 7385:
		VK_Subject_Reacts(20, 0, 0, 5);
		VK_Play_Speech_Line(kActorGrigorian, 440, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8100, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 450, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8105, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(20, 0, 0, 3);
		VK_Play_Speech_Line(kActorGrigorian, 460, 0.5f);
		break;
	case 7395:
		VK_Subject_Reacts(20, 0, 0, 7);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 470, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 480, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8105, 0.5f);
		break;
	case 7400:
		VK_Subject_Reacts(10, 8, 0, -5);
		VK_Play_Speech_Line(kActorGrigorian, 490, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 500, 0.5f);
		break;
	case 7405:
		VK_Play_Speech_Line(kActorGrigorian, 510, 0.5f);
		VK_Subject_Reacts(30, 10, 7, 0);
		VK_Play_Speech_Line(kActorGrigorian, 520, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8115, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(20, 12, -3, -3);
		VK_Play_Speech_Line(kActorGrigorian, 530, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 540, 0.5f);
		break;
	case 7415:
		VK_Play_Speech_Line(kActorGrigorian, 550, 0.5f);
		VK_Subject_Reacts(25, 7, 8, 8);
		VK_Play_Speech_Line(kActorGrigorian, 560, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8120, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8125, 0.5f);
		VK_Subject_Reacts(75, 8, 0, 0);
		VK_Play_Speech_Line(kActorGrigorian, 570, 0.5f);
		break;
	case 7420:
		VK_Subject_Reacts(10, 6, 8, -5);
		VK_Play_Speech_Line(kActorGrigorian, 580, 0.5f);
		break;
	case 7425:
		VK_Subject_Reacts(30, 8, -2, 5);
		VK_Play_Speech_Line(kActorGrigorian, 590, 0.5f);
		break;
	case 7430:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(45, 8, 7, 15);
		VK_Play_Speech_Line(kActorGrigorian, 600, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8130, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 610, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8135, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8140, 0.5f);
		break;
	case 7435:
		VK_Play_Speech_Line(kActorGrigorian, 620, 0.5f);
		VK_Subject_Reacts(15, 8, -3, -5);
		VK_Play_Speech_Line(kActorGrigorian, 630, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(25, 4, 7, -3);
		VK_Play_Speech_Line(kActorGrigorian, 640, 0.5f);
		break;
	case 7445:
		VK_Subject_Reacts(30, 8, 3, 5);
		VK_Play_Speech_Line(kActorGrigorian, 650, 0.5f);
		break;
	case 7450:
		VK_Play_Speech_Line(kActorGrigorian, 660, 0.5f);
		VK_Subject_Reacts(35, 4, 8, 3);
		VK_Play_Speech_Line(kActorMcCoy, 8145, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 670, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(kActorGrigorian, 680, 0.5f);
		VK_Subject_Reacts(20, 7, -3, -5);
		VK_Play_Speech_Line(kActorGrigorian, 690, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(15, 4, 8, 0);
		VK_Play_Speech_Line(kActorGrigorian, 700, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(5, 7, -4, -7);
		VK_Play_Speech_Line(kActorGrigorian, 710, 0.5f);
		break;
	case 7470:
		VK_Subject_Reacts(30, 4, 8, -2);
		VK_Play_Speech_Line(kActorGrigorian, 720, 0.5f);
		break;
	case 7475:
		VK_Subject_Reacts(15, 8, 2, 5);
		VK_Play_Speech_Line(kActorGrigorian, 730, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(20, 9, 4, 8);
		VK_Play_Speech_Line(kActorGrigorian, 740, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 750, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(15, 8, 8, 5);
		VK_Play_Speech_Line(kActorGrigorian, 760, 0.5f);
		break;
	case 7490:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(30, 7, -4, 12);
		VK_Play_Speech_Line(kActorGrigorian, 770, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(kActorGrigorian, 780, 0.5f);
		VK_Subject_Reacts(30, 9, 6, 5);
		VK_Play_Speech_Line(kActorGrigorian, 790, 0.5f);
		break;
	case 7515:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 800, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8150, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8155, 0.5f);
		VK_Subject_Reacts(30, 9, -5, 12);
		VK_Play_Speech_Line(kActorGrigorian, 810, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8160, 0.5f);
		break;
	case 7525:
		VK_Subject_Reacts(30, 8, 5, 8);
		VK_Play_Speech_Line(kActorGrigorian, 820, 0.5f);
		break;
	case 7535:
		VK_Subject_Reacts(20, 9, 2, 4);
		VK_Play_Speech_Line(kActorGrigorian, 830, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(kActorGrigorian, 840, 0.5f);
		VK_Subject_Reacts(20, 5, 1, 8);
		break;
	case 7550:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 850, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8165, 0.5f);
		VK_Subject_Reacts(60, 6, 2, 15);
		VK_Play_Speech_Line(kActorGrigorian, 860, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8170, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7565:
		VK_Subject_Reacts(40, 8, 8, 12);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 870, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8175, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 880, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 890, 0.5f);
		break;
	case 7580:
		VK_Play_Speech_Line(kActorGrigorian, 900, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8180, 0.5f);
		VK_Subject_Reacts(20, 8, 3, 6);
		VK_Play_Speech_Line(kActorGrigorian, 910, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(50, 9, 3, 8);
		VK_Play_Speech_Line(kActorGrigorian, 1250, 0.5f);
		break;
	case 7595:
		VK_Subject_Reacts(25, 8, -3, 5);
		VK_Play_Speech_Line(kActorGrigorian, 920, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8185, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 930, 0.5f);
		break;
	case 7600:
		VK_Subject_Reacts(20, 5, 9, 2);
		VK_Play_Speech_Line(kActorGrigorian, 940, 0.5f);
		break;
	case 7605:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(40, 10, -3, 15);
		VK_Play_Speech_Line(kActorGrigorian, 950, 0.5f);
		break;
	case 7620:
		VK_Subject_Reacts(30, 9, 10, 10);
		VK_Play_Speech_Line(kActorGrigorian, 960, 0.5f);
		break;
	case 7635:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7645, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7650, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 980, 0.5f);
		VK_Subject_Reacts(20, 10, 5, 12);
		break;
	case 7670:
		VK_Subject_Reacts(30, 4, 1, 10);
		VK_Play_Speech_Line(kActorGrigorian, 990, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(kActorGrigorian, 1000, 0.5f);
		VK_Subject_Reacts(30, 9, 3, 10);
		VK_Play_Speech_Line(kActorGrigorian, 1010, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8190, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1020, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1030, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1040, 0.5f);
		break;
	case 7690:
		VK_Play_Speech_Line(kActorGrigorian, 1050, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7695, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7700, 0.5f);
		VK_Subject_Reacts(60, 11, 9, 100);
		VK_Play_Speech_Line(kActorGrigorian, 1060, 0.5f);
		break;
	case 7705:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 1070, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7720, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7725, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
		VK_Subject_Reacts(60, 14, 3, 20);
		VK_Play_Speech_Line(kActorGrigorian, 1080, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8195, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 1090, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8200, 0.5f);
		break;
	case 7740:
		VK_Subject_Reacts(40, 10, 1, 15);
		VK_Play_Speech_Line(kActorGrigorian, 1100, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8205, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 1110, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8210, 0.5f);
		break;
	case 7750:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 9, -4, 20);
		VK_Play_Speech_Line(kActorGrigorian, 1120, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1130, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1140, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8220, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1150, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(kActorGrigorian, 1160, 0.5f);
		VK_Subject_Reacts(5, -8, 7, 10);
		break;
	}
}

void VKScript::askDektora(int questionId) {
	switch (questionId) {
	case 7385:
		VK_Subject_Reacts(36, 0, 0, 0);
		VK_Play_Speech_Line(kActorDektora, 1440, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(40, 2, 2, 0);
		VK_Play_Speech_Line(kActorDektora, 1450, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7785, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1460, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7790, 0.5f);
		break;
	case 7395:
		VK_Play_Speech_Line(kActorDektora, 1470, 0.5f);
		VK_Subject_Reacts(40, 4, 4, 0);
		VK_Play_Speech_Line(kActorMcCoy, 7795, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1480, 0.5f);
		break;
	case 7400:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1490, 0.5f);
			VK_Subject_Reacts(15, -1, 9, 0);
			VK_Play_Speech_Line(kActorDektora, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(15, 13, -1, 0);
			VK_Play_Speech_Line(kActorDektora, 1510, 0.5f);
		}
		break;
	case 7405:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1520, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7840, 0.5f);
			VK_Subject_Reacts(20, -1, 9, 10);
			VK_Play_Speech_Line(kActorDektora, 1540, 0.8f);
			VK_Play_Speech_Line(kActorDektora, 1550, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 1560, 0.5f);
			VK_Subject_Reacts(25, 13, -3, 0);
			VK_Play_Speech_Line(kActorDektora, 1570, 0.8f);
			VK_Play_Speech_Line(kActorDektora, 1580, 0.5f);
		}
		break;
	case 7410:
		VK_Play_Speech_Line(kActorDektora, 1590, 0.5f);
		VK_Subject_Reacts(50, 10, 10, 10);
		VK_Play_Speech_Line(kActorDektora, 1600, 0.5f);
		break;
	case 7415:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(80, 6, 4, 10);
		VK_Play_Speech_Line(kActorDektora, 1610, 0.5f);
		break;
	case 7420:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1620, 0.5f);
			VK_Subject_Reacts(25, -1, 9, 0);
		} else {
			VK_Subject_Reacts(25, 14, -2, 0);
			VK_Play_Speech_Line(kActorDektora, 1630, 0.9f);
			VK_Play_Speech_Line(kActorDektora, 1640, 0.5f);
		}
		break;
	case 7425:
		VK_Subject_Reacts(40, -1, -1, 0);
		VK_Play_Speech_Line(kActorDektora, 1650, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(65, 4, 6, 10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorDektora, 1660, 0.5f);
		break;
	case 7435:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1670, 0.5f);
			VK_Subject_Reacts(60, -2, 9, 0);
			VK_Play_Speech_Line(kActorDektora, 1680, 0.5f);
		} else {
			VK_Subject_Reacts(60, 14, 2, 0);
			VK_Play_Speech_Line(kActorDektora, 1690, 0.5f);
		}
		break;
	case 7440:
		VK_Subject_Reacts(30, 3, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 1700, 0.5f);
		break;
	case 7445:
		VK_Play_Speech_Line(kActorDektora, 1710, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7800, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1720, 0.5f);
		VK_Subject_Reacts(45, 4, 6, 0);
		VK_Play_Speech_Line(kActorDektora, 1730, 0.5f);
		break;
	case 7450:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(60, 7, 7, 20);
		VK_Play_Speech_Line(kActorDektora, 1740, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7805, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorDektora, 1750, 0.9f);
		VK_Play_Speech_Line(kActorDektora, 1760, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(kActorDektora, 1780, 0.5f);
		VK_Subject_Reacts(35, 3, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 1790, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7810, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1800, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7815, 0.5f);
		break;
	case 7460:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(40, -2, 10, 10);
			VK_Play_Speech_Line(kActorDektora, 1810, 0.5f);
		} else {
			VK_Subject_Reacts(35, 14, 3, 0);
			VK_Play_Speech_Line(kActorDektora, 1820, 0.5f);
		}
		break;
	case 7465:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(60, -3, 10, 5);
			VK_Play_Speech_Line(kActorDektora, 1830, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 1840, 0.5f);
			VK_Subject_Reacts(60, 13, 2, 5);
		}
		break;
	case 7470:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1850, 0.5f);
			VK_Subject_Reacts(50, -2, 11, 0);
			VK_Play_Speech_Line(kActorDektora, 1860, 0.5f);
		} else {
			VK_Subject_Reacts(90, 15, -5, 10);
			VK_Play_Speech_Line(kActorDektora, 1870, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8532, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 1890, 0.5f);
		}
		break;
	case 7475:
		VK_Subject_Reacts(28, 0, 0, 0);
		VK_Play_Speech_Line(kActorDektora, 1900, 0.5f);
		break;
	case 7480:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(55, -3, 12, 5);
			VK_Play_Speech_Line(kActorDektora, 1910, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 1920, 0.5f);
			VK_Subject_Reacts(55, 17, -3, 5);
			VK_Play_Speech_Line(kActorDektora, 1930, 0.5f);
		}
		break;
	case 7485:
		VK_Play_Speech_Line(kActorDektora, 1940, 0.5f);
		VK_Subject_Reacts(38, 4, 9, 0);
		break;
	case 7490:
		VK_Subject_Reacts(43, 8, 8, 5);
		VK_Play_Speech_Line(kActorDektora, 1950, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7820, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1960, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7825, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(kActorDektora, 1970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7830, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1980, 0.5f);
		VK_Subject_Reacts(65, 4, 4, 5);
		VK_Play_Speech_Line(kActorDektora, 1990, 0.5f);
		break;
	case 7515:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2000, 0.5f);
			VK_Subject_Reacts(72, -3, 12, 2);
			VK_Play_Speech_Line(kActorDektora, 2010, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7835, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2020, 0.5f);
		} else {
			VK_Subject_Reacts(60, 16, -1, 2);
			VK_Play_Speech_Line(kActorDektora, 2030, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7840, 0.5f);
		}
		break;
	case 7525:
		VK_Subject_Reacts(40, 6, 6, 0);
		VK_Play_Speech_Line(kActorDektora, 2040, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8533, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2060, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2070, 0.5f);
		break;
	case 7535:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2080, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7845, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2090, 0.5f);
			VK_Subject_Reacts(60, -6, 11, 0);
			VK_Play_Speech_Line(kActorDektora, 2100, 0.5f);
		} else {
			VK_Subject_Reacts(60, 17, -7, 0);
			VK_Play_Speech_Line(kActorDektora, 2110, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7850, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2120, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7855, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2130, 0.5f);
		}
		break;
	case 7540:
		VK_Eye_Animates(2);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(70, -5, 12, 80);
			VK_Play_Speech_Line(kActorDektora, 2140, 0.5f);
		} else {
			VK_Subject_Reacts(80, 17, -1, 80);
			VK_Play_Speech_Line(kActorDektora, 2150, 1.0f);
			VK_Play_Speech_Line(kActorDektora, 2160, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7860, 0.5f);
		}
		break;
	case 7550:
		VK_Play_Speech_Line(kActorDektora, 2170, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7865, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2180, 0.5f);
		VK_Subject_Reacts(55, 6, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 2190, 0.5f);
		break;
	case 7565:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorDektora, 2210, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7870, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2220, 0.5f);
		VK_Subject_Reacts(70, 8, 7, 10);
		VK_Play_Speech_Line(kActorDektora, 2230, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7875, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorDektora, 2240, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2250, 0.5f);
		break;
	case 7580:
		VK_Play_Speech_Line(kActorDektora, 2260, 0.5f);
		VK_Subject_Reacts(40, 5, 5, 10);
		VK_Play_Speech_Line(kActorDektora, 2270, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(50, 8, 8, 7);
		VK_Play_Speech_Line(kActorDektora, 2280, 0.5f);
		break;
	case 7595:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorDektora, 2290, 0.5f);
		VK_Subject_Reacts(30, 5, 5, 8);
		break;
	case 7600:
		VK_Play_Speech_Line(kActorDektora, 2300, 0.5f);
		VK_Subject_Reacts(30, 4, 4, 5);
		if (!Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2310, 0.5f);
		}
		break;
	case 7605:
		VK_Subject_Reacts(60, -1, -1, 5);
		VK_Play_Speech_Line(kActorDektora, 2320, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(kActorDektora, 2340, 0.5f);
		VK_Subject_Reacts(72, 9, 9, 5);
		VK_Play_Speech_Line(kActorDektora, 2350, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7885, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2360, 0.5f);
		break;
	case 7635:
		VK_Subject_Reacts(60, 6, 7, 0);
		VK_Play_Speech_Line(kActorDektora, 2370, 0.5f);
		break;
	case 7670:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2380, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7890, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2390, 0.5f);
			VK_Subject_Reacts(90, -3, 14, 50);
			VK_Play_Speech_Line(kActorMcCoy, 7895, 0.5f);
		} else {
			VK_Subject_Reacts(80, 18, -3, 10);
			VK_Play_Speech_Line(kActorDektora, 2410, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8534, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2430, 0.5f);
		}
		break;
	case 7680:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(70, -4, 14, 15);
			VK_Play_Speech_Line(kActorDektora, 2440, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 2450, 0.5f);
			VK_Subject_Reacts(70, 18, -4, 15);
			VK_Play_Speech_Line(kActorDektora, 2460, 0.5f);
		}
		break;
	case 7690:
		VK_Play_Speech_Line(kActorDektora, 2470, 0.5f);
		VK_Subject_Reacts(20, 9, 8, 5);
		VK_Play_Speech_Line(kActorDektora, 2480, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7900, 0.5f);
		break;
	case 7705:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2500, 0.5f);
			VK_Subject_Reacts(85, 7, 14, 20);
			VK_Play_Speech_Line(kActorDektora, 2510, 0.5f);
		} else {
			VK_Subject_Reacts(99, 18, 7, 20);
			VK_Play_Speech_Line(kActorDektora, 2530, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7910, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2550, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorMcCoy, 7915, 0.5f);
		}
		break;
	case 7740:
		VK_Subject_Reacts(60, 5, 6, 0);
		VK_Play_Speech_Line(kActorDektora, 2560, 0.5f);
		break;
	case 7750:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2580, 0.5f);
			VK_Subject_Reacts(90, -5, 14, 20);
			VK_Play_Speech_Line(kActorDektora, 2590, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7920, 0.5f);
		} else {
			VK_Subject_Reacts(90, 17, 3, 20);
			VK_Play_Speech_Line(kActorDektora, 2600, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7925, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorDektora, 2610, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7930, 0.5f);
		}
		break;
	case 7770:
		VK_Eye_Animates(2);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2630, 0.5f);
			VK_Subject_Reacts(99, 6, 15, 30);
		} else {
			VK_Play_Speech_Line(kActorDektora, 2640, 0.5f);
			VK_Subject_Reacts(99, 15, -4, 30);
		}
		break;
	}
}

void VKScript::askRunciter(int questionId) {
	switch (questionId) {
	case 7385:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7390:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(60, 15, -30, 2);
		VK_Play_Speech_Line(kActorRunciter, 1260, 0.5f);
		break;
	case 7395:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(90, -40, -10, 6);
		VK_Play_Speech_Line(kActorRunciter, 1280, 0.5f);
		break;
	case 7400:
		VK_Subject_Reacts(70, -10, -10, 0);
		VK_Play_Speech_Line(kActorRunciter, 1300, 0.5f);
		break;
	case 7405:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(100, 10, 10, 0);
		VK_Play_Speech_Line(kActorRunciter, 1330, 0.5f);
		break;
	case 7415:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7420:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7425:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7435:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7445:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7450:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7455:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7470:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7475:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7490:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7495:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7515:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7525:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7535:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7540:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7550:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7565:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7580:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7595:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7600:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7605:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7620:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7635:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7670:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7680:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7690:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7705:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7740:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7750:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	case 7770:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
		break;
	}
}

void VKScript::askBulletBob(int questionId) {
	switch (questionId) {
	case 7385:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(54, 0, 0, 0);
		VK_Play_Speech_Line(kActorBulletBob, 900, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 910, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8225, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(48, 0, 0, 3);
		VK_Play_Speech_Line(kActorBulletBob, 920, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8230, 0.5f);
		break;
	case 7395:
		VK_Play_Speech_Line(kActorBulletBob, 930, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(50, 0, 0, 8);
		VK_Play_Speech_Line(kActorBulletBob, 940, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8235, 0.5f);
		break;
	case 7400:
		VK_Play_Speech_Line(kActorBulletBob, 950, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, -5, -6, 2);
		VK_Play_Speech_Line(kActorBulletBob, 960, 0.5f);
		break;
	case 7405:
		VK_Subject_Reacts(25, 8, 7, 4);
		VK_Play_Speech_Line(kActorBulletBob, 970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8240, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 980, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(40, -6, -5, 5);
		VK_Play_Speech_Line(kActorBulletBob, 990, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8245, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1000, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8250, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(70, 8, 6, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1010, 0.5f);
		break;
	case 7415:
		VK_Subject_Reacts(25, 9, 6, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1020, 0.5f);
		break;
	case 7420:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1030, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8255, 0.5f);
		VK_Subject_Reacts(30, 7, 5, 3);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1040, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(kActorBulletBob, 1050, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1060, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8260, 0.5f);
		VK_Subject_Reacts(5, 5, 6, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1070, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(15, 7, 6, -6);
		VK_Play_Speech_Line(kActorBulletBob, 1080, 0.5f);
		break;
	case 7435:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1100, 0.5f);
		VK_Subject_Reacts(25, 8, 5, -7);
		VK_Play_Speech_Line(kActorMcCoy, 8265, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1110, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1120, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(30, 8, 6, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1130, 0.5f);
		break;
	case 7445:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1140, 0.5f);
		VK_Subject_Reacts(80, 8, 8, -10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1150, 0.5f);
		break;
	case 7450:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1160, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(60, 8, 8, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1170, 0.5f);
		break;
	case 7455:
		VK_Subject_Reacts(30, 7, 6, 3);
		VK_Play_Speech_Line(kActorBulletBob, 1180, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(10, 4, 4, 2);
		VK_Play_Speech_Line(kActorBulletBob, 1190, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(15, 5, 3, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1200, 0.5f);
		if (Actor_Query_Friendliness_To_Other(14, 0) <= 40) {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(kActorBulletBob, 1210, 0.5f);
			VK_Eye_Animates(1);
		}
		break;
	case 7470:
		VK_Subject_Reacts(50, -4, 0, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1240, 0.5f);
		break;
	case 7475:
		VK_Play_Speech_Line(kActorBulletBob, 1250, 0.5f);
		VK_Subject_Reacts(30, 9, 7, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1260, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(60, 10, 8, -6);
		VK_Play_Speech_Line(kActorBulletBob, 1270, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(70, 8, 9, 10);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1280, 0.5f);
		break;
	case 7490:
		VK_Play_Speech_Line(kActorBulletBob, 1290, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1300, 0.5f);
		VK_Subject_Reacts(10, 11, 10, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1310, 0.5f);
		break;
	case 7495:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1320, 0.5f);
		VK_Subject_Reacts(10, 8, 7, 7);
		break;
	case 7515:
		VK_Subject_Reacts(25, 7, 7, 0);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1330, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1340, 0.5f);
		break;
	case 7525:
		VK_Play_Speech_Line(kActorBulletBob, 1350, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 7, 6, 6);
		break;
	case 7535:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1360, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8275, 0.5f);
		VK_Subject_Reacts(10, 9, 7, -4);
		VK_Play_Speech_Line(kActorMcCoy, 8280, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1380, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1390, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8285, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(kActorBulletBob, 1400, 0.5f);
		VK_Subject_Reacts(30, 10, 9, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1410, 0.5f);
		break;
	case 7550:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1420, 0.5f);
		VK_Subject_Reacts(20, 7, 7, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1430, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		break;
	case 7565:
		VK_Play_Speech_Line(kActorBulletBob, 1440, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8295, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1450, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7570, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7575, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1460, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8300, 0.5f);
		VK_Subject_Reacts(90, 8, 9, 18);
		VK_Play_Speech_Line(kActorBulletBob, 1470, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7580:
		VK_Subject_Reacts(20, 9, 7, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1480, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8305, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1490, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(kActorBulletBob, 1500, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 8, 7, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1510, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1520, 0.5f);
		break;
	case 7595:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 10, 9, 15);
		VK_Play_Speech_Line(kActorBulletBob, 1530, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7600:
		VK_Subject_Reacts(20, 8, 8, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1540, 0.5f);
		break;
	case 7605:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 9, 8, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1550, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(kActorBulletBob, 1560, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 7630, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1570, 0.5f);
		VK_Subject_Reacts(10, 10, 9, 10);
		break;
	case 7635:
		VK_Play_Speech_Line(kActorBulletBob, 1580, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 8310, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7645, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7650, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
		VK_Eye_Animates(2);
		VK_Subject_Reacts(60, 8, 8, 40);
		VK_Play_Speech_Line(kActorBulletBob, 1590, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8315, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(85, 10, 11, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1600, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7670:
		VK_Subject_Reacts(50, 12, 7, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1620, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1630, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(kActorBulletBob, 1640, 0.5f);
		VK_Subject_Reacts(15, 5, 6, 5);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8320, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1650, 0.5f);
		break;
	case 7690:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 14, 13, 15);
		VK_Play_Speech_Line(kActorBulletBob, 1660, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 8325, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1670, 0.5f);
		break;
	case 7705:
		VK_Play_Speech_Line(kActorBulletBob, 1680, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 7720, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 12, 10, 0);
		VK_Play_Speech_Line(kActorMcCoy, 7725, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
		VK_Subject_Reacts(55, 6, 6, 0);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
		VK_Eye_Animates(2);
		VK_Subject_Reacts(70, 11, 9, 100);
		VK_Play_Speech_Line(kActorBulletBob, 1690, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7740:
		VK_Subject_Reacts(30, 4, 3, 3);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1700, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorBulletBob, 1710, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1720, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorBulletBob, 1730, 0.5f);
		break;
	case 7750:
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Subject_Reacts(10, 8, 5, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1740, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8330, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorBulletBob, 1750, 0.5f);
		VK_Subject_Reacts(25, 7, 5, 8);
		VK_Play_Speech_Line(kActorBulletBob, 1760, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1770, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(kActorBulletBob, 1780, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8335, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1790, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Subject_Reacts(30, 7, 7, 10);
		break;
	}
}

void VKScript::askCalibrationQuestion1(int actorId, int notUsed) {
	switch (actorId) {
	case kActorRunciter:
		askRunciter(7385);
		break;
	case kActorBulletBob:
		askBulletBob(7385);
		break;
	case kActorGrigorian:
		askGrigorian(7385);
		break;
	case kActorLucy:
		askLucy(7385);
		break;
	case kActorDektora:
		askCalibrationDektora1(7385);
		break;
	}
}

void VKScript::askCalibrationQuestion2(int actorId, int notUsed) {
	switch (actorId) {
	case kActorRunciter:
		askRunciter(7390);
		break;
	case kActorBulletBob:
		askBulletBob(7390);
		break;
	case kActorGrigorian:
		askGrigorian(7390);
		break;
	case kActorLucy:
		askLucy(7390);
		break;
	case kActorDektora:
		askCalibrationDektora2(7390);
		break;
	}
}

void VKScript::askCalibrationQuestion3(int actorId, int notUsed) {
	switch (actorId) {
	case kActorRunciter:
		askRunciter(7395);
		break;
	case kActorBulletBob:
		askBulletBob(7395);
		break;
	case kActorGrigorian:
		askGrigorian(7395);
		break;
	case kActorLucy:
		askLucy(7395);
		break;
	case kActorDektora:
		askCalibrationDektora3(7395);
		break;
	}
}

void VKScript::askCalibrationDektora3(int question1) {
	VK_Play_Speech_Line(kActorDektora, 1470, 0.5f);
	VK_Subject_Reacts(40, 4, 4, 0);
	VK_Play_Speech_Line(kActorMcCoy, 7795, 0.5f);
	VK_Play_Speech_Line(kActorDektora, 1480, 0.5f);
}

void VKScript::askCalibrationDektora2(int question1) {
	VK_Subject_Reacts(40, 2, 2, 0);
	VK_Play_Speech_Line(kActorDektora, 1450, 0.5f);
	VK_Play_Speech_Line(kActorMcCoy, 7785, 0.5f);
	VK_Play_Speech_Line(kActorDektora, 1460, 0.5f);
	VK_Play_Speech_Line(kActorMcCoy, 7790, 0.5f);
}

void VKScript::askCalibrationDektora1(int question1) {
	VK_Subject_Reacts(36, 0, 0, 0);
	VK_Play_Speech_Line(kActorDektora, 1440, 0.5f);
}

} // End of namespace BladeRunner
