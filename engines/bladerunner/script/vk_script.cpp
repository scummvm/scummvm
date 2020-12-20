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
#include "bladerunner/debugger.h"

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
	VK_Add_Question(0, 7400, -1);       // Low 01
	VK_Add_Question(0, 7405, -1);       // Low 02
	VK_Add_Question(0, 7410, -1);       // Low 03
	VK_Add_Question(0, 7415, -1);       // Low 04
	VK_Add_Question(0, 7420, -1);       // Low 05
	VK_Add_Question(0, 7425, -1);       // Low 06
	if (_vm->_cutContent) {
		// Add the question for male subjects too ("when did you stop beating your spouse?")
		if (actorId == kActorLucy) {
			VK_Add_Question(0, 7430, 7770); // Low 07 -> High 10 (A game/ It's not, really, just part of the test)
		} else if (actorId == kActorGrigorian) {
			VK_Add_Question(0, 7430, 7415); // Low 07 -> Low 04  (How the test works)
		} else {
			VK_Add_Question(0, 7430, -1);   // Low 07
		}
	} else {
		if (actorId == kActorLucy || actorId == kActorDektora) {
			VK_Add_Question(0, 7430, -1);   // Low 07
		}
	}
	VK_Add_Question(0, 7435, -1);       // Low 08
	VK_Add_Question(0, 7440, -1);       // Low 09
	VK_Add_Question(0, 7445, -1);       // Low 10
	VK_Add_Question(0, 7450, -1);       // Low 11
	VK_Add_Question(0, 7455, -1);       // Low 12
	VK_Add_Question(0, 7460, -1);       // Low 13
	VK_Add_Question(0, 7465, -1);       // Low 14
	VK_Add_Question(0, 7470, -1);       // Low 15
	VK_Add_Question(1, 7475, -1);       // Medium 01
	VK_Add_Question(1, 7480, -1);       // Medium 02
	VK_Add_Question(1, 7485, -1);       // Medium 03
	VK_Add_Question(1, 7490, -1);       // Medium 04
	VK_Add_Question(1, 7495, -1);       // Medium 05
	VK_Add_Question(1, 7515, -1);       // Medium 06
	VK_Add_Question(1, 7525, -1);       // Medium 07
	VK_Add_Question(1, 7535, -1);       // Medium 08
	VK_Add_Question(1, 7540, -1);       // Medium 09
	VK_Add_Question(1, 7550, -1);       // Medium 10
	VK_Add_Question(1, 7565, -1);       // Medium 11
	VK_Add_Question(1, 7580, -1);       // Medium 12
	VK_Add_Question(1, 7585, -1);       // Medium 13
	if (_vm->_cutContent) {
		if (actorId == kActorBulletBob) {
			// Bullet Bob's Medium 14 is related to Low 05 (Low 05 is supposed to be asked first)
			VK_Add_Question(1, 7595, 7420); // Medium 14 -> Low 05 (Hamster)
		} else {
			VK_Add_Question(1, 7595, -1);   // Medium 14
		}
	} else {
		VK_Add_Question(1, 7595, -1);       // Medium 14
	}
	if (_vm->_cutContent) {
		if (actorId == kActorLucy) {
			VK_Add_Question(1, 7600, 7475); // Medium 15 -> Medium 01 (Can't have children not ever)
		} else {
			VK_Add_Question(1, 7600, -1);   // Medium 15
		}
	} else {
		VK_Add_Question(1, 7600, -1);       // Medium 15
	}
	VK_Add_Question(2, 7605, -1);       // High 01
	VK_Add_Question(2, 7620, -1);       // High 02
	if (_vm->_cutContent) {
		if (actorId == kActorGrigorian) {
			VK_Add_Question(2, 7635, 7585); // High 03 -> Medium 13   // book/ magazine with dead animals
		} else {
			VK_Add_Question(2, 7635, -1);   // High 03
		}
	} else {
		VK_Add_Question(2, 7635, -1);       // High 03
	}
	VK_Add_Question(2, 7670, -1);       // High 04
	VK_Add_Question(2, 7680, -1);       // High 05
	if (_vm->_cutContent) {
		if (actorId == kActorBulletBob) {
			VK_Add_Question(2, 7690, 7565); // High 06 -> Medium 11 (when people laughing at him)
		} else if (actorId == kActorDektora) {
			VK_Add_Question(2, 7690, 7750); // High 06 -> High 09  (You don't write them / I Just read 'em)
		} else {
			VK_Add_Question(2, 7690, -1);   // High 06
		}
	} else {
		VK_Add_Question(2, 7690, -1);       // High 06
	}
	if (_vm->_cutContent) {
		if (actorId == kActorBulletBob) {
			VK_Add_Question(2, 7705, 7635); // High 07 -> High 03 (I've had enough)
		} else if (actorId == kActorGrigorian) {
			VK_Add_Question(2, 7705, 7605); // High 07 -> High 01 (They're just questions x2)
		} else {
			VK_Add_Question(2, 7705, -1);   // High 07
		}
	} else {
		VK_Add_Question(2, 7705, -1);       // High 07
	}
	VK_Add_Question(2, 7740, -1);       // High 08
	VK_Add_Question(2, 7750, -1);       // High 09
	VK_Add_Question(2, 7770, -1);       // High 10

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
	case 7400:                          // Low 01
		VK_Play_Speech_Line(kActorMcCoy, 7400, 0.5f);
		break;
	case 7405:                          // Low 02
		VK_Play_Speech_Line(kActorMcCoy, 7405, 0.5f);
		break;
	case 7410:                          // Low 03
		VK_Play_Speech_Line(kActorMcCoy, 7410, 0.5f);
		break;
	case 7415:                          // Low 04
		VK_Play_Speech_Line(kActorMcCoy, 7415, 0.5f);
		break;
	case 7420:                          // Low 05
		VK_Play_Speech_Line(kActorMcCoy, 7420, 0.5f);
		break;
	case 7425:                          // Low 06
		VK_Play_Speech_Line(kActorMcCoy, 7425, 0.5f);
		break;
	case 7430:                          // Low 07
		VK_Play_Speech_Line(kActorMcCoy, 7430, 0.5f);
		break;
	case 7435:                          // Low 08
		VK_Play_Speech_Line(kActorMcCoy, 7435, 0.5f);
		break;
	case 7440:                          // Low 09
		VK_Play_Speech_Line(kActorMcCoy, 7440, 0.5f);
		break;
	case 7445:                          // Low 10
		VK_Play_Speech_Line(kActorMcCoy, 7445, 0.5f);
		break;
	case 7450:                          // Low 11
		VK_Play_Speech_Line(kActorMcCoy, 7450, 0.5f);
		break;
	case 7455:                          // Low 12
		VK_Play_Speech_Line(kActorMcCoy, 7455, 0.5f);
		break;
	case 7460:                          // Low 13
		VK_Play_Speech_Line(kActorMcCoy, 7460, 0.5f);
		break;
	case 7465:                          // Low 14
		VK_Play_Speech_Line(kActorMcCoy, 7465, 0.5f);
		break;
	case 7470:                          // Low 15
		VK_Play_Speech_Line(kActorMcCoy, 7470, 0.5f);
		break;
	case 7475:                          // Medium 01
		VK_Play_Speech_Line(kActorMcCoy, 7475, 0.5f);
		break;
	case 7480:                          // Medium 02
		VK_Play_Speech_Line(kActorMcCoy, 7480, 0.5f);
		break;
	case 7485:                          // Medium 03
		VK_Play_Speech_Line(kActorMcCoy, 7485, 0.5f);
		break;
	case 7490:                          // Medium 04
		VK_Play_Speech_Line(kActorMcCoy, 7490, 0.5f);
		break;
	case 7495:                          // Medium 05
		VK_Play_Speech_Line(kActorMcCoy, 7495, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7500, 0.5f);
#else
		VK_Play_Speech_Line(kActorMcCoy, 7500, 0.1f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7505, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7510, 0.5f);
		break;
	case 7515:                          // Medium 06
		VK_Play_Speech_Line(kActorMcCoy, 7515, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorRunciter) {
				VK_Play_Speech_Line(kActorMcCoy, 7520, 0.5f);
			}
		} else {
			VK_Play_Speech_Line(kActorMcCoy, 7520, 0.5f);
		}
		break;
	case 7525:                          // Medium 07
		VK_Play_Speech_Line(kActorMcCoy, 7525, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7530, 0.5f);
		break;
	case 7535:                          // Medium 08
		VK_Play_Speech_Line(kActorMcCoy, 7535, 0.5f);
		break;
	case 7540:                          // Medium 09
		VK_Play_Speech_Line(kActorMcCoy, 7540, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7545, 0.5f);
		break;
	case 7550:                          // Medium 10 - you pour yourself a drink...
		VK_Play_Speech_Line(kActorMcCoy, 7550, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorRunciter) {
				VK_Play_Speech_Line(kActorMcCoy, 7555, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7560, 0.5f);
			}
		} else {
			VK_Play_Speech_Line(kActorMcCoy, 7555, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7560, 0.5f);
		}
		break;
	case 7565:                          // Medium 11
		VK_Play_Speech_Line(kActorMcCoy, 7565, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorBulletBob
				&& actorId != kActorRunciter
			) {
				if (actorId == kActorDektora) {
					VK_Play_Speech_Line(kActorDektora, 2200, 0.5f); //   They wouldn't be laughing
				}
				VK_Play_Speech_Line(kActorMcCoy, 7570, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7575, 0.5f);
			}
		} else {
			if (actorId != kActorBulletBob) {
				VK_Play_Speech_Line(kActorMcCoy, 7570, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7575, 0.5f);
			}
		}
		break;
	case 7580:                          // Medium 12
		VK_Play_Speech_Line(kActorMcCoy, 7580, 0.5f);
		break;
	case 7585:                          // Medium 13
		VK_Play_Speech_Line(kActorMcCoy, 7585, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorLucy
				&& actorId != kActorBulletBob
				&& actorId != kActorRunciter
			) {
				VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
			}
		} else {
			if (actorId != kActorLucy && actorId != kActorBulletBob) {
				VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
			}
		}
		break;
	case 7595:                          // Medium 14
		VK_Play_Speech_Line(kActorMcCoy, 7595, 0.5f);
		break;
	case 7600:                          // Medium 15
		VK_Play_Speech_Line(kActorMcCoy, 7600, 0.5f);
		break;
	case 7605:                          // High 01
		VK_Play_Speech_Line(kActorMcCoy, 7605, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7610, 0.5f);
#else
		VK_Play_Speech_Line(kActorMcCoy, 7610, 0.1f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7615, 0.5f);
		break;
	case 7620:                          // High 02
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
	case 7635:                          // High 03
		VK_Play_Speech_Line(kActorMcCoy, 7635, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7640, 0.5f);
		if (actorId != kActorGrigorian && actorId != kActorBulletBob) {
			VK_Play_Speech_Line(kActorMcCoy, 7645, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7650, 0.5f);
			if (_vm->_cutContent) {
				if (actorId != kActorLucy
					&& actorId != kActorRunciter
				) {
					VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
					VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
#else
					VK_Play_Speech_Line(kActorMcCoy, 7660, 0.2f);
#endif // BLADERUNNER_ORIGINAL_BUGS
					VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
				}
			} else {
				if (actorId != kActorLucy) {
					VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
					VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
#else
					VK_Play_Speech_Line(kActorMcCoy, 7660, 0.2f);
#endif // BLADERUNNER_ORIGINAL_BUGS
					VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
				}
			}
		}
		break;
	case 7670:                          // High 04
		VK_Play_Speech_Line(kActorMcCoy, 7670, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7675, 0.5f);
		break;
	case 7680:                          // High 05
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7680, 0.5f);
#else
		VK_Play_Speech_Line(kActorMcCoy, 7680, 0.1f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7685, 0.5f);
		break;
	case 7690:                          // High 06
		VK_Play_Speech_Line(kActorMcCoy, 7690, 0.5f);
		if (actorId != kActorGrigorian) {
			VK_Play_Speech_Line(kActorMcCoy, 7695, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7700, 0.5f);
		}
		break;
	case 7705:                          // High 07
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7705, 0.5f);
#else
		VK_Play_Speech_Line(kActorMcCoy, 7705, 0.1f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 7710, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7715, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorGrigorian
				&& actorId != kActorBulletBob
				&& actorId != kActorRunciter
			) {
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
		} else {
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
		}
		break;
	case 7740:                          // High 08
		VK_Play_Speech_Line(kActorMcCoy, 7740, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7745, 0.5f);
		break;
	case 7750:                          // High 09
		VK_Play_Speech_Line(kActorMcCoy, 7750, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7755, 0.5f);
		if (actorId == kActorDektora) {
			VK_Play_Speech_Line(kActorDektora, 2570, 0.5f);
		}
		VK_Play_Speech_Line(kActorMcCoy, 7760, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7765, 0.5f);
		break;
	case 7770:                          // High 10
		VK_Play_Speech_Line(kActorMcCoy, 7770, 0.5f);
		if (_vm->_cutContent) {
			if (actorId != kActorRunciter) {
				if (actorId == kActorDektora) {
					VK_Play_Speech_Line(kActorDektora, 2620, 0.5f);
				}
				VK_Play_Speech_Line(kActorMcCoy, 7775, 0.5f);
				VK_Play_Speech_Line(kActorMcCoy, 7780, 0.5f);
			}
		} else {
			if (actorId == kActorDektora) {
				VK_Play_Speech_Line(kActorDektora, 2620, 0.5f);
			}
			VK_Play_Speech_Line(kActorMcCoy, 7775, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7780, 0.5f);
		}
		break;
	default:
		break;
	}

	if ( _vm->_debugger->_playFullVk ) {
		if (++_questionCounter >= 40) {
			switch (actorId) {
			case kActorLucy:
				//debug("Last Lucy Question!");
				if (Game_Flag_Query(kFlagLucyIsReplicant)) {
					VK_Subject_Reacts(5, 0, 100, 100); // Replicant result
				} else {
					VK_Subject_Reacts(5, 100, 0, 100); // Human result
				}
				break;
			case kActorDektora:
				//debug("Last Dektora Question!");
				if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
					VK_Subject_Reacts(5, 0, 100, 100); // Replicant result
				} else {
					VK_Subject_Reacts(5, 100, 0, 100); // Human result
				}
				break;
			case kActorGrigorian:
				//debug("Last Grigorian Question!");
				VK_Subject_Reacts(5, 100, 0, 100); // Human result
				break;
			case kActorRunciter:
				//debug("Last Runciter Question!");
				VK_Subject_Reacts(5, 100, 0, 100); // Human result
				break;
			case kActorBulletBob:
				// debug("Last Bullet Bob Question!");
				// don't break for BOB
				// fall through
			default:
				VK_Subject_Reacts(5, 0, 0, 100);
			}
		}
	} else {
		// original behavior
		if (++_questionCounter >= 10) {
			VK_Subject_Reacts(5, 0, 0, 100);
		}
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
	default:
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
		default:
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
		default:
			break;
		}
	}
	VK_Play_Speech_Line(kActorAnsweringMachine, 460, 0.5f);
}

void VKScript::askLucy(int questionId) {
	switch (questionId) {
	case 7385:                          // Calibration 01
		VK_Subject_Reacts(40, 0, 0, 0);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 1240, 0.5f);
		break;
	case 7390:                          // Calibration 02
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
	case 7395:                         // Calibration 03
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1280, 0.5f);
			VK_Subject_Reacts(40, 0, 0, 0);
		} else {
			VK_Subject_Reacts(40, 0, 0, 0);
			VK_Play_Speech_Line(kActorLucy, 1280, 0.5f);
		}
		break;
	case 7400:                          // Low 01
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 0, 8, -5);
		} else {
			VK_Subject_Reacts(30, 9, 0, -10);
		}
		VK_Play_Speech_Line(kActorLucy, 1300, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7405:                          // Low 02
		VK_Play_Speech_Line(kActorLucy, 1310, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(50, 1, 11, 5);
		} else {
			VK_Subject_Reacts(60, 11, 1, 5);
		}
		VK_Play_Speech_Line(kActorLucy, 1320, 0.5f);
		break;
	case 7410:                          // Low 03
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 1, 10, -5);
			VK_Eye_Animates(2);
		} else {
			VK_Subject_Reacts(40, 10, -2, -5);
		}
		VK_Play_Speech_Line(kActorLucy, 1330, 0.5f);
		break;
	case 7415:                          // Low 04
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
	case 7420:                          // Low 05
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
	case 7425:                          // Low 06
		VK_Play_Speech_Line(kActorLucy, 1400, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -2, 9, -2);
		} else {
			VK_Subject_Reacts(20, 9, -2, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1410, 0.5f);
		break;
	case 7430:                          // Low 07
		// Lucy cuts her sentence short here.
		// She is not interrupted.
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
	case 7435:                          // Low 08
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1440, 0.5f);
			VK_Subject_Reacts(30, 5, 10, 2);
		} else {
			VK_Subject_Reacts(30, 9, 3, 2);
			VK_Play_Speech_Line(kActorLucy, 1450, 0.5f);
		}
		break;
	case 7440:                          // Low 09
		// Lucy cuts her sentence short here.
		// She is not interrupted.
		VK_Play_Speech_Line(kActorLucy, 1460, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, 5, 10, 2);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(20, 9, -3, 2);
		}
		VK_Play_Speech_Line(kActorLucy, 1470, 0.5f);
		break;
	case 7445:                          // Low 10
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Play_Speech_Line(kActorLucy, 1480, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7940, 0.5f);
			VK_Subject_Reacts(50, 4, 11, 10);
			VK_Play_Speech_Line(kActorLucy, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(30, 9, -1, -2);
			VK_Play_Speech_Line(kActorLucy, 1510, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 7945, 0.5f);
#else
			// McCoy is interrupted here
			VK_Play_Speech_Line(kActorMcCoy, 7945, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorLucy, 1520, 0.5f);
		}
		break;
	case 7450:                          // Low 11
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 3, 9, -6);
		} else {
			VK_Subject_Reacts(30, 9, 4, -6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1530, 0.5f);
		break;
	case 7455:                          // Low 12
		VK_Play_Speech_Line(kActorLucy, 1540, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7950, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(10, 1, 8, -5);
		} else {
			VK_Subject_Reacts(10, 9, -1, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1550, 0.5f);
		break;
	case 7460:                          // Low 13
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
	case 7465:                          // Low 14
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(40, -1, 11, 2);
			VK_Play_Speech_Line(kActorLucy, 1590, 0.5f);
		} else {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(kActorLucy, 1590, 0.5f);
			VK_Subject_Reacts(20, 9, 2, -8);
		}
		break;
	case 7470:                          // Low 15
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
	case 7475:                          // Medium 01
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
	case 7480:                          // Medium 02
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
	case 7485:                          // Medium 03
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -2, 10, 6);
		} else {
			VK_Subject_Reacts(30, 10, -2, 6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1740, 0.5f);
		break;
	case 7490:                          // Medium 04
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
	case 7495:                          // Medium 05
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(10, 2, 8, 5);
		} else {
			VK_Subject_Reacts(30, 10, -1, 5);
		}
		VK_Play_Speech_Line(kActorLucy, 1770, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 1780, 0.5f);
		break;
	case 7515:                          // Medium 06
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
	case 7525:                          // Medium 07
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, -4, 9, 5);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(40, 11, -3, 7);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(kActorLucy, 1820, 0.5f);
		break;
	case 7535:                          // Medium 08
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
	case 7540:                          // Medium 09
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
	case 7550:                          // Medium 10
		VK_Play_Speech_Line(kActorLucy, 1890, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 2, 9, -1);
		} else {
			VK_Subject_Reacts(10, 8, -1, -2);
		}
		VK_Play_Speech_Line(kActorLucy, 1900, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8015, 0.5f);
		break;
	case 7565:                          // Medium 11
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
	case 7580:                          // Medium 12
		VK_Play_Speech_Line(kActorLucy, 1930, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(20, 5, 10, -1);
		} else {
			VK_Subject_Reacts(30, 10, 3, 0);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8025, 0.5f);
		VK_Play_Speech_Line(kActorLucy, 1940, 0.5f);
		break;
	case 7585:                          // Medium 13
		VK_Play_Speech_Line(kActorLucy, 1950, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8030, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8030, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
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
	case 7595:                          // Medium 14
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
	case 7600:                          // Medium 15
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(75, 4, 13, 15);
		} else {
			VK_Subject_Reacts(60, 12, -2, 10);
		}
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorLucy, 2050, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8050, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8050, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Subject_Reacts(90, 0, 0, 0);
		VK_Play_Speech_Line(kActorLucy, 2060, 0.5f);
		break;
	case 7605:                          // High 01
		VK_Play_Speech_Line(kActorLucy, 2070, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(30, 0, 11, 12);
		} else {
			VK_Subject_Reacts(50, 10, -3, 15);
		}
		VK_Play_Speech_Line(kActorLucy, 2080, 0.5f);
		break;
	case 7620:                          // High 02
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(60, -2, 14, 12);
		} else {
			VK_Subject_Reacts(70, 9, -1, 10);
		}
		VK_Play_Speech_Line(kActorLucy, 2090, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8055, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8055, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 2100, 0.5f);
		break;
	case 7635:                          // High 03
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
	case 7670:                          // High 04
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
	case 7680:                          // High 05
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorLucy, 2170, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(65, 1, 6, 5);
		} else {
			VK_Subject_Reacts(50, 10, 3, 4);
		}
		VK_Play_Speech_Line(kActorLucy, 2180, 0.5f);
		break;
	case 7690:                          // High 06
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
	case 7705:                          // High 07
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
			if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorMcCoy, 8537, 0.5f);
			}
			VK_Play_Speech_Line(kActorLucy, 2270, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 8080, 0.5f);
			VK_Play_Speech_Line(kActorLucy, 2280, 0.5f);
		}
		break;
	case 7740:                          // High 08
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
	case 7750:                          // High 09
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(60, 4, 10, 15);
		} else {
			VK_Subject_Reacts(80, 12, -3, 18);
		}
		VK_Play_Speech_Line(kActorLucy, 2330, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8095, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8095, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorLucy, 2340, 0.5f);
		break;
	case 7770:                          // High 10
		VK_Play_Speech_Line(kActorLucy, 2350, 0.5f);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			VK_Subject_Reacts(90, -3, -1, 12);
		} else {
			VK_Eye_Animates(2);
			VK_Subject_Reacts(90, 13, -4, 12);
		}
		VK_Play_Speech_Line(kActorLucy, 2360, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8538, 0.5f);
		}
		break;
	default:
		break;
	}
}

void VKScript::askGrigorian(int questionId) {
	switch (questionId) {
	case 7385:                          // Calibration 01
		VK_Subject_Reacts(20, 0, 0, 5);
		VK_Play_Speech_Line(kActorGrigorian, 440, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8100, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 450, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8105, 0.5f);
		break;
	case 7390:                          // Calibration 02
		VK_Subject_Reacts(20, 0, 0, 3);
		VK_Play_Speech_Line(kActorGrigorian, 460, 0.5f);
		break;
	case 7395:                          // Calibration 03
		VK_Subject_Reacts(20, 0, 0, 7);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 470, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 480, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8110, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorMcCoy, 8105, 0.5f);
		}
		break;
	case 7400:                          // Low 01
		VK_Subject_Reacts(10, 8, 0, -5);
		VK_Play_Speech_Line(kActorGrigorian, 490, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 500, 0.5f);
		break;
	case 7405:                          // Low 02
		VK_Play_Speech_Line(kActorGrigorian, 510, 0.5f);
		VK_Subject_Reacts(30, 10, 7, 0);
		VK_Play_Speech_Line(kActorGrigorian, 520, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8115, 0.5f);
		break;
	case 7410:                          // Low 03
		VK_Subject_Reacts(20, 12, -3, -3);
		VK_Play_Speech_Line(kActorGrigorian, 530, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 540, 0.5f);
		break;
	case 7415:                          // Low 04
		VK_Play_Speech_Line(kActorGrigorian, 550, 0.5f);
		VK_Subject_Reacts(25, 7, 8, 8);
		VK_Play_Speech_Line(kActorGrigorian, 560, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8120, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8125, 0.5f);
		VK_Subject_Reacts(75, 8, 0, 0);
		VK_Play_Speech_Line(kActorGrigorian, 570, 0.5f);
		break;
	case 7420:                          // Low 05
		VK_Subject_Reacts(10, 6, 8, -5);
		VK_Play_Speech_Line(kActorGrigorian, 580, 0.5f);
		break;
	case 7425:                          // Low 06
		VK_Subject_Reacts(30, 8, -2, 5);
		VK_Play_Speech_Line(kActorGrigorian, 590, 0.5f);
		break;
	case 7430:                          // Low 07
		VK_Eye_Animates(2);
		VK_Subject_Reacts(45, 8, 7, 15);
		VK_Play_Speech_Line(kActorGrigorian, 600, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8130, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8130, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 610, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8135, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8140, 0.5f);
		break;
	case 7435:                          // Low 08
		VK_Play_Speech_Line(kActorGrigorian, 620, 0.5f);
		VK_Subject_Reacts(15, 8, -3, -5);
		VK_Play_Speech_Line(kActorGrigorian, 630, 0.5f);
		break;
	case 7440:                          // Low 09
		VK_Subject_Reacts(25, 4, 7, -3);
		VK_Play_Speech_Line(kActorGrigorian, 640, 0.5f);
		break;
	case 7445:                          // Low 10
		VK_Subject_Reacts(30, 8, 3, 5);
		VK_Play_Speech_Line(kActorGrigorian, 650, 0.5f);
		break;
	case 7450:                          // Low 11
		VK_Play_Speech_Line(kActorGrigorian, 660, 0.5f);
		VK_Subject_Reacts(35, 4, 8, 3);
		VK_Play_Speech_Line(kActorMcCoy, 8145, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 670, 0.5f);
		break;
	case 7455:                          // Low 12
		VK_Play_Speech_Line(kActorGrigorian, 680, 0.5f);
		VK_Subject_Reacts(20, 7, -3, -5);
		VK_Play_Speech_Line(kActorGrigorian, 690, 0.5f);
		break;
	case 7460:                          // Low 13
		VK_Subject_Reacts(15, 4, 8, 0);
		VK_Play_Speech_Line(kActorGrigorian, 700, 0.5f);
		break;
	case 7465:                          // Low 14
		VK_Subject_Reacts(5, 7, -4, -7);
		VK_Play_Speech_Line(kActorGrigorian, 710, 0.5f);
		break;
	case 7470:                          // Low 15
		VK_Subject_Reacts(30, 4, 8, -2);
		VK_Play_Speech_Line(kActorGrigorian, 720, 0.5f);
		break;
	case 7475:                          // Medium 01
		VK_Subject_Reacts(15, 8, 2, 5);
		VK_Play_Speech_Line(kActorGrigorian, 730, 0.5f);
		break;
	case 7480:                          // Medium 02
		VK_Subject_Reacts(20, 9, 4, 8);
		VK_Play_Speech_Line(kActorGrigorian, 740, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 750, 0.5f);
		break;
	case 7485:                          // Medium 03
		VK_Subject_Reacts(15, 8, 8, 5);
		VK_Play_Speech_Line(kActorGrigorian, 760, 0.5f);
		break;
	case 7490:                          // Medium 04
		VK_Eye_Animates(3);
		VK_Subject_Reacts(30, 7, -4, 12);
		VK_Play_Speech_Line(kActorGrigorian, 770, 0.5f);
		break;
	case 7495:                          // Medium 05
		VK_Play_Speech_Line(kActorGrigorian, 780, 0.5f);
		VK_Subject_Reacts(30, 9, 6, 5);
		VK_Play_Speech_Line(kActorGrigorian, 790, 0.5f);
		break;
	case 7515:                          // Medium 06
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 800, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8150, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8155, 0.5f);
		VK_Subject_Reacts(30, 9, -5, 12);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorGrigorian, 810, 0.5f);
#else
		// Grigorian is interrupted here
		VK_Play_Speech_Line(kActorGrigorian, 810, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8160, 0.5f);
		break;
	case 7525:                          // Medium 07
		VK_Subject_Reacts(30, 8, 5, 8);
		VK_Play_Speech_Line(kActorGrigorian, 820, 0.5f);
		break;
	case 7535:                          // Medium 08
		VK_Subject_Reacts(20, 9, 2, 4);
		VK_Play_Speech_Line(kActorGrigorian, 830, 0.5f);
		break;
	case 7540:                          // Medium 09
		VK_Play_Speech_Line(kActorGrigorian, 840, 0.5f);
		VK_Subject_Reacts(20, 5, 1, 8);
		break;
	case 7550:                          // Medium 10
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 850, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8165, 0.5f);
		VK_Subject_Reacts(60, 6, 2, 15);
		VK_Play_Speech_Line(kActorGrigorian, 860, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8170, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7565:                          // Medium 11
		VK_Subject_Reacts(40, 8, 8, 12);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 870, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8175, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 880, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 890, 0.5f);
		break;
	case 7580:                          // Medium 12
		VK_Play_Speech_Line(kActorGrigorian, 900, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8180, 0.5f);
		VK_Subject_Reacts(20, 8, 3, 6);
		VK_Play_Speech_Line(kActorGrigorian, 910, 0.5f);
		break;
	case 7585:                          // Medium 13
		VK_Subject_Reacts(50, 9, 3, 8);
		VK_Play_Speech_Line(kActorGrigorian, 1250, 0.5f);
		break;
	case 7595:                          // Medium 14
		VK_Subject_Reacts(25, 8, -3, 5);
		VK_Play_Speech_Line(kActorGrigorian, 920, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8185, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 930, 0.5f);
		break;
	case 7600:                          // Medium 15
		VK_Subject_Reacts(20, 5, 9, 2);
		VK_Play_Speech_Line(kActorGrigorian, 940, 0.5f);
		break;
	case 7605:                          // High 01
		VK_Eye_Animates(3);
		VK_Subject_Reacts(40, 10, -3, 15);
		VK_Play_Speech_Line(kActorGrigorian, 950, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 7375, 0.5f);     //  M: They're just questions
		}
		break;
	case 7620:                          // High 02
		VK_Subject_Reacts(30, 9, 10, 10);
		VK_Play_Speech_Line(kActorGrigorian, 960, 0.5f);
		break;
	case 7635:                          // High 03
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
	case 7670:                          // High 04
		VK_Subject_Reacts(30, 4, 1, 10);
		VK_Play_Speech_Line(kActorGrigorian, 990, 0.5f);
		break;
	case 7680:                          // High 05
		VK_Play_Speech_Line(kActorGrigorian, 1000, 0.5f);
		VK_Subject_Reacts(30, 9, 3, 10);
		VK_Play_Speech_Line(kActorGrigorian, 1010, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8190, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1020, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1030, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1040, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8535, 0.5f);     // M: yeah...
		}
		break;
	case 7690:                          // High 06
		VK_Play_Speech_Line(kActorGrigorian, 1050, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7695, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7700, 0.5f);
		VK_Subject_Reacts(60, 11, 9, 100);
		VK_Play_Speech_Line(kActorGrigorian, 1060, 0.5f);
		break;
	case 7705:                          // High 07
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 1070, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8536, 0.5f);     // M: But if you did...
		}
		VK_Play_Speech_Line(kActorMcCoy, 7720, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7725, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
		VK_Subject_Reacts(60, 14, 3, 20);
		VK_Play_Speech_Line(kActorGrigorian, 1080, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
		VK_Play_Speech_Line(kActorMcCoy, 8195, 0.5f);
#else
		// McCoy is interrupted here
		VK_Play_Speech_Line(kActorMcCoy, 8195, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorGrigorian, 1090, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8200, 0.5f);
		break;
	case 7740:                          // High 08
		VK_Subject_Reacts(40, 10, 1, 15);
		VK_Play_Speech_Line(kActorGrigorian, 1100, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8205, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorGrigorian, 1110, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8210, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8215, 0.5f);
		}
		break;
	case 7750:                          // High 09
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 9, -4, 20);
		VK_Play_Speech_Line(kActorGrigorian, 1120, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1130, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1140, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8220, 0.5f);
		VK_Play_Speech_Line(kActorGrigorian, 1150, 0.5f);
		break;
	case 7770:                          // High 10
		VK_Play_Speech_Line(kActorGrigorian, 1160, 0.5f);
		VK_Subject_Reacts(5, -8, 7, 10);
		break;
	default:
		break;
	}
}

void VKScript::askDektora(int questionId) {
	switch (questionId) {
	case 7385:                          // Calibration 01
		VK_Subject_Reacts(36, 0, 0, 0);
		VK_Play_Speech_Line(kActorDektora, 1440, 0.5f);
		break;
	case 7390:                          // Calibration 02
		VK_Subject_Reacts(40, 2, 2, 0);
		VK_Play_Speech_Line(kActorDektora, 1450, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7785, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1460, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7790, 0.5f);
		break;
	case 7395:                          // Calibration 03
		VK_Play_Speech_Line(kActorDektora, 1470, 0.5f);
		VK_Subject_Reacts(40, 4, 4, 0);
		VK_Play_Speech_Line(kActorMcCoy, 7795, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1480, 0.5f);
		break;
	case 7400:                          // Low 01
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1490, 0.5f);
			VK_Subject_Reacts(15, -1, 9, 0);
			VK_Play_Speech_Line(kActorDektora, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(15, 13, -1, 0);
			VK_Play_Speech_Line(kActorDektora, 1510, 0.5f);
		}
		break;
	case 7405:                          // Low 02
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1520, 0.5f);
			if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorMcCoy, 8531, 0.5f);
			} else {
				VK_Play_Speech_Line(kActorMcCoy, 7840, 0.5f);
			}
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
	case 7410:                          // Low 03
		VK_Play_Speech_Line(kActorDektora, 1590, 0.5f);
		VK_Subject_Reacts(50, 10, 10, 10);
		VK_Play_Speech_Line(kActorDektora, 1600, 0.5f);
		break;
	case 7415:                          // Low 04
		VK_Eye_Animates(3);
		VK_Subject_Reacts(80, 6, 4, 10);
		VK_Play_Speech_Line(kActorDektora, 1610, 0.5f);
		break;
	case 7420:                          // Low 05
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1620, 0.5f);
			VK_Subject_Reacts(25, -1, 9, 0);
		} else {
			VK_Subject_Reacts(25, 14, -2, 0);
			VK_Play_Speech_Line(kActorDektora, 1630, 0.9f);
			VK_Play_Speech_Line(kActorDektora, 1640, 0.5f);
		}
		break;
	case 7425:                          // Low 06
		VK_Subject_Reacts(40, -1, -1, 0);
		VK_Play_Speech_Line(kActorDektora, 1650, 0.5f);
		break;
	case 7430:                          // Low 07
		VK_Subject_Reacts(65, 4, 6, 10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorDektora, 1660, 0.5f);
		break;
	case 7435:                          // Low 08
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1670, 0.5f);
			VK_Subject_Reacts(60, -2, 9, 0);
			VK_Play_Speech_Line(kActorDektora, 1680, 0.5f);
		} else {
			VK_Subject_Reacts(60, 14, 2, 0);
			VK_Play_Speech_Line(kActorDektora, 1690, 0.5f);
		}
		break;
	case 7440:                          // Low 09
		VK_Subject_Reacts(30, 3, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 1700, 0.5f);
		break;
	case 7445:                          // Low 10
		VK_Play_Speech_Line(kActorDektora, 1710, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7800, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1720, 0.5f);
		VK_Subject_Reacts(45, 4, 6, 0);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 7380, 0.5f); // M: Just answer the question please.
		}
		VK_Play_Speech_Line(kActorDektora, 1730, 0.5f);
		break;
	case 7450:                          // Low 11
		VK_Eye_Animates(2);
		VK_Subject_Reacts(60, 7, 7, 20);
		VK_Play_Speech_Line(kActorDektora, 1740, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7805, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorDektora, 1750, 0.9f);
		VK_Play_Speech_Line(kActorDektora, 1760, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 7806, 0.5f); // M: They are meant to provoke a reaction.
		}
		break;
	case 7455:                          // Low 12
		VK_Play_Speech_Line(kActorDektora, 1780, 0.5f);
		VK_Subject_Reacts(35, 3, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 1790, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7810, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1800, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7815, 0.5f);
		break;
	case 7460:                          // Low 13
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(40, -2, 10, 10);
			VK_Play_Speech_Line(kActorDektora, 1810, 0.5f);
		} else {
			VK_Subject_Reacts(35, 14, 3, 0);
			VK_Play_Speech_Line(kActorDektora, 1820, 0.5f);
		}
		break;
	case 7465:                          // Low 14
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(60, -3, 10, 5);
			VK_Play_Speech_Line(kActorDektora, 1830, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 1840, 0.5f);
			VK_Subject_Reacts(60, 13, 2, 5);
		}
		break;
	case 7470:                          // Low 15
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 1850, 0.5f);
			VK_Subject_Reacts(50, -2, 11, 0);
			VK_Play_Speech_Line(kActorDektora, 1860, 0.5f);
		} else {
			VK_Subject_Reacts(90, 15, -5, 10);
			VK_Play_Speech_Line(kActorDektora, 1870, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 8532, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 1890, 0.5f);
#else
			// Quotes 8532 (McCoy) and 1890 (Dektora) are muted in the ESP version
			// They are completely missing from the ESP version (they don't appear elsewhere).
			// The quotes here are:
			// McCoy: "That's not a single word."
			// Dektora: "All right. Aggressive. Powerful."
			if (_vm->_language != Common::ES_ESP) {
				VK_Play_Speech_Line(kActorMcCoy, 8532, 0.5f);
				VK_Play_Speech_Line(kActorDektora, 1890, 0.5f);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		break;
	case 7475:                          // Medium 01
		VK_Subject_Reacts(28, 0, 0, 0);
		VK_Play_Speech_Line(kActorDektora, 1900, 0.5f);
		break;
	case 7480:                          // Medium 02
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Subject_Reacts(55, -3, 12, 5);
			VK_Play_Speech_Line(kActorDektora, 1910, 0.5f);
		} else {
			VK_Play_Speech_Line(kActorDektora, 1920, 0.5f);
			VK_Subject_Reacts(55, 17, -3, 5);
			VK_Play_Speech_Line(kActorDektora, 1930, 0.5f);
		}
		break;
	case 7485:                          // Medium 03
		VK_Play_Speech_Line(kActorDektora, 1940, 0.5f);
		VK_Subject_Reacts(38, 4, 9, 0);
		break;
	case 7490:                          // Medium 04
		VK_Subject_Reacts(43, 8, 8, 5);
		VK_Play_Speech_Line(kActorDektora, 1950, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7820, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1960, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7825, 0.5f);
		break;
	case 7495:                          // Medium 05
		VK_Play_Speech_Line(kActorDektora, 1970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7830, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 1980, 0.5f);
		VK_Subject_Reacts(65, 4, 4, 5);
		VK_Play_Speech_Line(kActorDektora, 1990, 0.5f);
		break;
	case 7515:                          // Medium 06
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
	case 7525:                          // Medium 07
		VK_Subject_Reacts(40, 6, 6, 0);
		VK_Play_Speech_Line(kActorDektora, 2040, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8533, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2060, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2070, 0.5f);
		break;
	case 7535:                          // Medium 08
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
	case 7540:                          // Medium 09
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
	case 7550:                          // Medium 10
		VK_Play_Speech_Line(kActorDektora, 2170, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7865, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2180, 0.5f);
		VK_Subject_Reacts(55, 6, 5, 0);
		VK_Play_Speech_Line(kActorDektora, 2190, 0.5f);
		break;
	case 7565:                          // Medium 11
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
	case 7580:                          // Medium 12
		VK_Play_Speech_Line(kActorDektora, 2260, 0.5f);
		VK_Subject_Reacts(40, 5, 5, 10);
		VK_Play_Speech_Line(kActorDektora, 2270, 0.5f);
		break;
	case 7585:                          // Medium 13
		VK_Subject_Reacts(50, 8, 8, 7);
		VK_Play_Speech_Line(kActorDektora, 2280, 0.5f);
		break;
	case 7595:                          // Medium 14
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorDektora, 2290, 0.5f);
		VK_Subject_Reacts(30, 5, 5, 8);
		break;
	case 7600:                          // Medium 15
		VK_Play_Speech_Line(kActorDektora, 2300, 0.5f);
		VK_Subject_Reacts(30, 4, 4, 5);
		if (!Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2310, 0.5f);
		}
		break;
	case 7605:                          // High 01
		VK_Subject_Reacts(60, -1, -1, 5);
		VK_Play_Speech_Line(kActorDektora, 2320, 0.5f);
		break;
	case 7620:                          // High 02
		VK_Play_Speech_Line(kActorDektora, 2340, 0.5f);
		VK_Subject_Reacts(72, 9, 9, 5);
		VK_Play_Speech_Line(kActorDektora, 2350, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7885, 0.5f);
		VK_Play_Speech_Line(kActorDektora, 2360, 0.5f);
		break;
	case 7635:                          // High 03
		VK_Subject_Reacts(60, 6, 7, 0);
		VK_Play_Speech_Line(kActorDektora, 2370, 0.5f);
		break;
	case 7670:                          // High 04
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2380, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7890, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2390, 0.5f);
			VK_Subject_Reacts(90, -3, 14, 50);
			if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorDektora, 2400, 0.5f); // D: You want to kidnap me, don't you.
			}
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 7895, 0.5f);
#else
			// McCoy cuts his sentence short here.
			// He is not interrupted. Yet let's still put a pause of 0.0f for consistency
			VK_Play_Speech_Line(kActorMcCoy, 7895, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
		} else {
			VK_Subject_Reacts(80, 18, -3, 10);
			VK_Play_Speech_Line(kActorDektora, 2410, 0.5f);
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 8534, 0.5f);
#else
			// McCoy is interrupted here
			VK_Play_Speech_Line(kActorMcCoy, 8534, 0.0f);
#endif // BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorDektora, 2430, 0.5f);
		}
		break;
	case 7680:                          // High 05
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
	case 7690:                          // High 06
		VK_Play_Speech_Line(kActorDektora, 2470, 0.5f);
		VK_Subject_Reacts(20, 9, 8, 5);
		VK_Play_Speech_Line(kActorDektora, 2480, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7900, 0.5f);
		break;
	case 7705:                          // High 07
		VK_Eye_Animates(3);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2500, 0.5f);
			VK_Subject_Reacts(85, 7, 14, 20);
			VK_Play_Speech_Line(kActorDektora, 2510, 0.5f);
			if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorMcCoy, 7905, 0.5f); // M: Let's keep going, all right?
			}
		} else {
			VK_Subject_Reacts(99, 18, 7, 20);
			VK_Play_Speech_Line(kActorDektora, 2530, 0.5f);
			if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorMcCoy, 7909, 0.5f); // M: Went off the scale on that one.
			}
			VK_Play_Speech_Line(kActorMcCoy, 7910, 0.5f);
			VK_Play_Speech_Line(kActorDektora, 2550, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(kActorMcCoy, 7915, 0.5f);
		}
		break;
	case 7740:                          // High 08
		VK_Subject_Reacts(60, 5, 6, 0);
		VK_Play_Speech_Line(kActorDektora, 2560, 0.5f);
		break;
	case 7750:                          // High 09
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
	case 7770:                          // High 10
		VK_Eye_Animates(2);
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			VK_Play_Speech_Line(kActorDektora, 2630, 0.5f);
			VK_Subject_Reacts(99, 6, 15, 30);
		} else {
			VK_Play_Speech_Line(kActorDektora, 2640, 0.5f);
			VK_Subject_Reacts(99, 15, -4, 30);
		}
		break;
	default:
		break;
	}
}

void VKScript::askRunciter(int questionId) {
	if (_vm->_cutContent) {
		switch (questionId) {
		case 7385:                          // Calibration 01
			VK_Subject_Reacts(20, 4, -2, 5);
			VK_Play_Speech_Line(kActorRunciter, 880, 0.5f); // R: Uptown - your office already has address
			break;
		case 7390:                          // Calibration 02
			VK_Eye_Animates(3);
			VK_Subject_Reacts(15, 8, 3, 2);
			VK_Play_Speech_Line(kActorRunciter, 890, 0.5f); // R: I Trade In Hard To Find Animals
			break;
		case 7395:                          // Calibration 03
			VK_Eye_Animates(2);
			VK_Subject_Reacts(10, 3, 0, 3);
			VK_Play_Speech_Line(kActorRunciter, 900, 0.5f); // R: Emil Runciter
			break;
		case 7400:                          // Low 01
			VK_Subject_Reacts(8, 2, 3, 0);
			VK_Play_Speech_Line(kActorRunciter, 910, 0.5f); // R: I'd avoid him
			break;
		case 7405:                          // Low 02
			VK_Subject_Reacts(50, 12, -1, 5);
			VK_Play_Speech_Line(kActorRunciter, 920, 0.5f); // R: Because...
			VK_Play_Speech_Line(kActorRunciter, 930, 0.5f); // R: all my animals...
			break;
		case 7410:                          // Low 03
			VK_Subject_Reacts(25, 8, 12, 0);
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorRunciter, 940, 0.5f); // R: Capture it
#else
			// Runciter is interrupted here
			VK_Play_Speech_Line(kActorRunciter, 940, 0.0f); // R: Capture it
#endif // BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 8340, 0.5f);
			VK_Subject_Reacts(11, 8, 8, 0);
			VK_Play_Speech_Line(kActorRunciter, 950, 0.5f); // R: Oh I do that first
			break;
		case 7415:                          // Low 04
			VK_Subject_Reacts(20, 6, 3, 0);                 // custom
			VK_Play_Speech_Line(kActorRunciter, 960, 0.5f); // R: No
			break;
		case 7420:                          // Low 05
			VK_Eye_Animates(2);
			VK_Subject_Reacts(40, 12, 7, 0);                // custom
			VK_Play_Speech_Line(kActorRunciter, 970, 0.5f); // R: Sidney for resale
			VK_Play_Speech_Line(kActorMcCoy, 8345, 0.5f);   // R: I wouldn't know
			break;
		case 7425:                          // Low 06
			VK_Eye_Animates(1);
			VK_Subject_Reacts(35, 8, 0, 7);
			VK_Play_Speech_Line(kActorRunciter, 980, 0.5f); // R: Detective I think you know...
			break;
		case 7430:                          // Low 07
			VK_Subject_Reacts(7, 6, 6, 7);
			VK_Play_Speech_Line(kActorRunciter, 990, 0.5f); // R: That's a tired old joke...
			break;
		case 7435:                          // Low 08
			VK_Eye_Animates(3);
			VK_Subject_Reacts(40, 9, -3, 2);
			VK_Play_Speech_Line(kActorRunciter, 1000, 0.5f); // R: Lonely
			VK_Play_Speech_Line(kActorRunciter, 1010, 0.5f); // R: Difficult
			break;
		case 7440:                          // Low 09
			VK_Subject_Reacts(8, 6, 6, 0);
			VK_Play_Speech_Line(kActorRunciter, 1020, 0.5f); // R: I'd ask her out...
			break;
		case 7445:                          // Low 10
			VK_Subject_Reacts(10, 5, 5, 0);
			VK_Play_Speech_Line(kActorRunciter, 1030, 0.5f); // R: And your point is?
			VK_Play_Speech_Line(kActorMcCoy, 8350, 0.5f);    // M: Just say the first thing that pops into your head.
			VK_Subject_Reacts(14, 8, 7, 4);
			VK_Play_Speech_Line(kActorRunciter, 1040, 0.5f); // R: My dreams are my own.
			break;
		case 7450:                          // Low 11
			VK_Subject_Reacts(15, 9, 10, 2);
			VK_Play_Speech_Line(kActorRunciter, 1050, 0.5f); // R: Perhaps I'm being threatened...
			VK_Play_Speech_Line(kActorMcCoy, 8355, 0.5f);    // M: I'm not threatening you.
			VK_Eye_Animates(2);
			VK_Subject_Reacts(45, 17, 8, 2);
			VK_Play_Speech_Line(kActorRunciter, 1060, 0.5f); // R: Stop playing games...
			VK_Play_Speech_Line(kActorRunciter, 1070, 0.5f); // R: I'm not a Replicant...
			VK_Play_Speech_Line(kActorMcCoy, 8360, 0.5f);    // M: We'll let the Machine decide that.
			break;
		case 7455:                          // Low 12
			VK_Subject_Reacts(7, 7, 2, 0);
			VK_Play_Speech_Line(kActorRunciter, 1080, 0.5f); // R:I'd try to forgive him
			break;
		case 7460:                          // Low 13
			VK_Subject_Reacts(8, 2, 0, 0);
			VK_Play_Speech_Line(kActorRunciter, 1090, 0.5f); // R: And?
			VK_Play_Speech_Line(kActorMcCoy, 8365, 0.5f);    // M: How do you feel about that?
			VK_Eye_Animates(1);
			VK_Subject_Reacts(15, 7, 5, 1);
			VK_Play_Speech_Line(kActorRunciter, 1100, 0.5f); // R: I'd probably already know...
			VK_Play_Speech_Line(kActorRunciter, 1110, 0.5f); // R: Why would I feel anything
			VK_Play_Speech_Line(kActorMcCoy, 8370, 0.5f);    // M: It's ok...
			break;
		case 7465:                          // Low 14
			VK_Eye_Animates(2);
			VK_Subject_Reacts(5, 6, 0, 0);
			VK_Play_Speech_Line(kActorRunciter, 1120, 0.5f); // R: Fifty three
			break;
		case 7470:                          // Low 15
			VK_Eye_Animates(2);
			VK_Subject_Reacts(38, 17, -4, 0);
			VK_Play_Speech_Line(kActorRunciter, 1130, 0.5f); // R: Gentle, loving-
			VK_Play_Speech_Line(kActorRunciter, 1140, 0.5f); // R: Comforting
			break;
		case 7475:                          // Medium 01
			VK_Eye_Animates(1);
			VK_Subject_Reacts(50, 18, -1, 1);
			VK_Play_Speech_Line(kActorRunciter, 1150, 0.5f); // R: Childhood is full of harsh...
			break;
		case 7480:                          // Medium 02
			VK_Subject_Reacts(12, 10, 6, 1);
			VK_Play_Speech_Line(kActorRunciter, 1160, 0.5f); // R: We all feel that way sometimes...
			break;
		case 7485:                          // Medium 03
			VK_Subject_Reacts(9, 12, 9, 2);
			VK_Play_Speech_Line(kActorRunciter, 1170, 0.5f); // R: No doubt I made a mistake..
			break;
		case 7490:                          // Medium 04
			VK_Eye_Animates(3);
			VK_Subject_Reacts(14, 10, 3, 3);
			VK_Play_Speech_Line(kActorRunciter, 1180, 0.5f); // R: I--
			VK_Play_Speech_Line(kActorRunciter, 1190, 0.5f); // R: I'm not answering that
			break;
		case 7495:                          // Medium 05
			VK_Eye_Animates(2);
			VK_Subject_Reacts(65, 18, -1, 2);
			VK_Play_Speech_Line(kActorRunciter, 1200, 0.5f); // R: I wouldn't let him touch me
			break;
		case 7515:                          // Medium 06
			VK_Subject_Reacts(4, 8, 2, 0);
			VK_Play_Speech_Line(kActorRunciter, 1210, 0.5f); // R: Adversity builds character-
			VK_Play_Speech_Line(kActorMcCoy, 8375, 0.5f);    // M: Let me finish-
			VK_Eye_Animates(1);
			VK_Play_Speech_Line(kActorMcCoy, 7520, 0.5f);    // M: During a boot-camp hazing...
			VK_Subject_Reacts(14, 10, 4, 0);
			VK_Play_Speech_Line(kActorRunciter, 1220, 0.5f); // R: Oh, I see. The worms...
			break;
		case 7525:                          // Medium 07
			VK_Subject_Reacts(28, 12, 3, 1);
			VK_Play_Speech_Line(kActorRunciter, 1230, 0.5f); // R: A Shameful waste...
			break;
		case 7535:                          // Medium 08
			VK_Subject_Reacts(15, 10, -1, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f); // R: I'd find another dentist.
			break;
		case 7540:                          // Medium 09
			VK_Subject_Reacts(11, 5, 0, 0);
			VK_Play_Speech_Line(kActorRunciter, 1250, 0.5f); // R: Don't be ridiculous...
			break;
		case 7550:                          // Medium 10
			VK_Subject_Reacts(8, 8, 2, 0);
			VK_Play_Speech_Line(kActorRunciter, 1260, 0.5f); // R: Sometimes-
			VK_Play_Speech_Line(kActorMcCoy, 7555, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7560, 0.5f);
			VK_Eye_Animates(3);
			VK_Subject_Reacts(25, 12, -2, 1);
			VK_Play_Speech_Line(kActorRunciter, 1270, 0.5f); // R: We all...
			break;
		case 7565:                          // Medium 11
			VK_Subject_Reacts(8, 8, -2, 0);
			VK_Play_Speech_Line(kActorRunciter, 1280, 0.5f); // R: Why would-
			VK_Play_Speech_Line(kActorMcCoy, 7570, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7575, 0.5f);
			VK_Subject_Reacts(32, 12, 9, 1);
			VK_Play_Speech_Line(kActorRunciter, 1290, 0.5f); // R: I'd leave...
			break;
		case 7580:                          // Medium 12
			VK_Eye_Animates(2);
			VK_Subject_Reacts(22, 12, 10, 0);
			VK_Play_Speech_Line(kActorRunciter, 1300, 0.5f); // R: Wasps are very...
			break;
		case 7585:                          // Medium 13
			VK_Subject_Reacts(15, 3, 6, 1);
			VK_Play_Speech_Line(kActorRunciter, 1310, 0.5f); // R: I don't read filthy magazines
			VK_Play_Speech_Line(kActorMcCoy, 8380, 0.5f);    // M: I haven't finished the question
			VK_Subject_Reacts(10, 5, 0, 2);
			VK_Play_Speech_Line(kActorRunciter, 1320, 0.5f); // R: Don't bother
			break;
		case 7595:                          // Medium 14
			VK_Eye_Animates(3);
			VK_Subject_Reacts(38, 15, 5, 1);
			VK_Play_Speech_Line(kActorRunciter, 1330, 0.5f); // R: I'd report the manufacturer
			break;
		case 7600:                          // Medium 15
			VK_Eye_Animates(2);
			VK_Subject_Reacts(68, 19, 4, 1);
			VK_Play_Speech_Line(kActorRunciter, 1340, 0.5f); // R: A child...
			break;
		case 7605:                          // High 01
			VK_Subject_Reacts(12, 8, 8, 0);
			VK_Play_Speech_Line(kActorRunciter, 1350, 0.5f); // R: That's insane
			VK_Play_Speech_Line(kActorMcCoy, 8385, 0.5f);    // M: They used to do it all the time
			VK_Eye_Animates(2);
			VK_Subject_Reacts(20, 12, 9, 0);
			VK_Play_Speech_Line(kActorRunciter, 1360, 0.5f); // R: Do you have any idea...
			break;
		case 7620:                          // High 02
			VK_Subject_Reacts(9, 5, 7, 0);
			VK_Play_Speech_Line(kActorRunciter, 1370, 0.5f); // R: What do you expect me to say?
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 8390, 0.5f);  // M: You don't have to say anything
#else
			// McCoy is interrupted here
			VK_Play_Speech_Line(kActorMcCoy, 8390, 0.0f);  // M: You don't have to say anything
#endif // BLADERUNNER_ORIGINAL_BUGS
			VK_Subject_Reacts(15, 6, 4, 2);
			VK_Play_Speech_Line(kActorRunciter, 1380, 0.5f); // R: I understand how it works
			break;
		case 7635:                          // High 03
			VK_Eye_Animates(3);
			VK_Subject_Reacts(38, 16, 6, 3);
			VK_Play_Speech_Line(kActorRunciter, 1390, 0.5f); // R: This test was written by a psychopath
			VK_Play_Speech_Line(kActorMcCoy, 7655, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7660, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7665, 0.5f);
			VK_Eye_Animates(1);
			Delay(1500u);
			VK_Play_Speech_Line(kActorMcCoy, 8395, 0.5f);    // M: You don't have anything to say?
			VK_Subject_Reacts(12, 8, 1, 0);
			VK_Play_Speech_Line(kActorRunciter, 1400, 0.5f); // R: What's the point
			break;
		case 7670:                          // High 04
			VK_Subject_Reacts(14, 10, -3, 0);
			VK_Play_Speech_Line(kActorRunciter, 1410, 0.5f); // R: That's not right...
			break;
		case 7680:                          // High 05
			VK_Subject_Reacts(12, 8, 5, 1);
			VK_Play_Speech_Line(kActorRunciter, 1420, 0.5f); // R: How would I know
			VK_Play_Speech_Line(kActorMcCoy, 8541, 0.5f);    // M: How do you think it would feel...
			VK_Play_Speech_Line(kActorRunciter, 1430, 0.5f); // R: They're just machines
			VK_Play_Speech_Line(kActorMcCoy, 8400, 0.5f);    // M: So you don't have any sympathy for them
			VK_Subject_Reacts(8, 4, -2, 0);
			VK_Play_Speech_Line(kActorRunciter, 1450, 0.5f); // R: I suppose it might be--
			VK_Play_Speech_Line(kActorRunciter, 1460, 0.5f); // R: Bad--
			break;
		case 7690:                          // High 06
			VK_Subject_Reacts(25, 17, 4, 4);
			VK_Play_Speech_Line(kActorRunciter, 1470, 0.5f); // R: That's pathetic
			VK_Play_Speech_Line(kActorMcCoy, 8405, 0.5f);    // M: Hey I just read them
			break;
		case 7705:                          // High 07
			VK_Eye_Animates(2);
			VK_Subject_Reacts(18, 10, -7, 0);
			VK_Play_Speech_Line(kActorRunciter, 1480, 0.5f); // R: I wouldn't do that...
			VK_Play_Speech_Line(kActorMcCoy, 7720, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7725, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7730, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7735, 0.5f);
			VK_Subject_Reacts(50, 15, 6, 4);
			VK_Play_Speech_Line(kActorRunciter, 1490, 0.5f); // R: You're disgusting
#if BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorMcCoy, 8410, 0.5f);  // M: Hey I didn't
#else
			// McCoy is interrupted here
			VK_Play_Speech_Line(kActorMcCoy, 8410, 0.0f);  // M: Hey I didn't
#endif // BLADERUNNER_ORIGINAL_BUGS
			VK_Play_Speech_Line(kActorRunciter, 1500, 0.5f); // R: I've always loved animals...
			VK_Play_Speech_Line(kActorMcCoy, 8415, 0.5f);    // M: No one is saying otherwise...
			VK_Subject_Reacts(60, 18, -3, 1);
			VK_Play_Speech_Line(kActorRunciter, 1510, 0.5f); // R: Can you even begin to understand...
			VK_Play_Speech_Line(kActorRunciter, 1520, 0.5f); // R: All at once
			VK_Play_Speech_Line(kActorMcCoy, 8420, 0.5f);    // M: Must be rough
			VK_Subject_Reacts(40, 11, 6, 1);
			VK_Play_Speech_Line(kActorRunciter, 1530, 0.5f); // R: Get out of here!
			break;
		case 7740:                          // High 08
			VK_Subject_Reacts(11, 8, -10, 4);
			VK_Play_Speech_Line(kActorRunciter, 1540, 0.5f); // R: I'd turn it off
			break;
		case 7750:                          // High 09
			VK_Subject_Reacts(25, 13, -9, 2);
			VK_Play_Speech_Line(kActorRunciter, 1550, 0.5f); // R: I could never...
			VK_Play_Speech_Line(kActorRunciter, 1560, 0.5f); // R: Who could?
			VK_Play_Speech_Line(kActorMcCoy, 8425, 0.5f);    // M: No one I know
			break;
		case 7770:                          // High 10
			VK_Eye_Animates(2);
			VK_Subject_Reacts(11, 8, 6, 0);
			VK_Play_Speech_Line(kActorRunciter, 1570, 0.5f); // R: It's very nice
			VK_Play_Speech_Line(kActorMcCoy, 7775, 0.5f);
			VK_Play_Speech_Line(kActorMcCoy, 7780, 0.5f);
			VK_Eye_Animates(1);
			VK_Subject_Reacts(58, 14, -6, 4);
			VK_Play_Speech_Line(kActorRunciter, 1580, 0.5f); // R: You're a sick man
			VK_Play_Speech_Line(kActorMcCoy, 8430, 0.5f);    // M: Yeah, but doctors...
			VK_Play_Speech_Line(kActorMcCoy, 8435, 0.5f);    // M: You're ready for the next one?
			VK_Play_Speech_Line(kActorRunciter, 1590, 0.5f); // R: Let's get this over with
			break;
		default:
			break;
		}
	} else {
		switch (questionId) {
		case 7385:                          // Calibration 01
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7390:                          // Calibration 02
			VK_Eye_Animates(3);
			VK_Subject_Reacts(60, 15, -30, 2);
			VK_Play_Speech_Line(kActorRunciter, 1260, 0.5f);
			break;
		case 7395:                          // Calibration 03
			VK_Eye_Animates(2);
			VK_Subject_Reacts(90, -40, -10, 6);
			VK_Play_Speech_Line(kActorRunciter, 1280, 0.5f);
			break;
		case 7400:                          // Low 01
			VK_Subject_Reacts(70, -10, -10, 0);
			VK_Play_Speech_Line(kActorRunciter, 1300, 0.5f);
			break;
		case 7405:                          // Low 02
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7410:                          // Low 03
			VK_Subject_Reacts(100, 10, 10, 0);
			VK_Play_Speech_Line(kActorRunciter, 1330, 0.5f);
			break;
		case 7415:                          // Low 04
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7420:                          // Low 05
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7425:                          // Low 06
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7430:                          // Low 07
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7435:                          // Low 08
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7440:                          // Low 09
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7445:                          // Low 10
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7450:                          // Low 11
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7455:                          // Low 12
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7460:                          // Low 13
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7465:                          // Low 14
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7470:                          // Low 15
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7475:                          // Medium 01
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7480:                          // Medium 02
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7485:                          // Medium 03
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7490:                          // Medium 04
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7495:                          // Medium 05
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7515:                          // Medium 06
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7525:                          // Medium 07
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7535:                          // Medium 08
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7540:                          // Medium 09
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7550:                          // Medium 10
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7565:                          // Medium 11
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7580:                          // Medium 12
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7585:                          // Medium 13
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7595:                          // Medium 14
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7600:                          // Medium 15
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7605:                          // High 01
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7620:                          // High 02
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7635:                          // High 03
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7670:                          // High 04
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7680:                          // High 05
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7690:                          // High 06
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7705:                          // High 07
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7740:                          // High 08
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7750:                          // High 09
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		case 7770:                          // High 10
			VK_Subject_Reacts(20, 10, 20, 0);
			VK_Play_Speech_Line(kActorRunciter, 1240, 0.5f);
			break;
		default:
			break;
		}
	}
}

void VKScript::askBulletBob(int questionId) {
	switch (questionId) {
	case 7385:                          // Calibration 01
		VK_Eye_Animates(1);
		VK_Subject_Reacts(54, 0, 0, 0);
		VK_Play_Speech_Line(kActorBulletBob, 900, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 910, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8225, 0.5f);
		break;
	case 7390:                          // Calibration 02
		VK_Subject_Reacts(48, 0, 0, 3);
		VK_Play_Speech_Line(kActorBulletBob, 920, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8230, 0.5f);
		break;
	case 7395:                          // Calibration 03
		VK_Play_Speech_Line(kActorBulletBob, 930, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(50, 0, 0, 8);
		VK_Play_Speech_Line(kActorBulletBob, 940, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8235, 0.5f);
		break;
	case 7400:                          // Low 01
		VK_Play_Speech_Line(kActorBulletBob, 950, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, -5, -6, 2);
		VK_Play_Speech_Line(kActorBulletBob, 960, 0.5f);
		break;
	case 7405:                          // Low 02
		VK_Subject_Reacts(25, 8, 7, 4);
		VK_Play_Speech_Line(kActorBulletBob, 970, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8240, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 980, 0.5f);
		break;
	case 7410:                          // Low 03
		VK_Subject_Reacts(40, -6, -5, 5);
		VK_Play_Speech_Line(kActorBulletBob, 990, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8245, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1000, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8250, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(70, 8, 6, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1010, 0.5f);
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8539, 0.5f); // M: Let's keep going
		}
		break;
	case 7415:                          // Low 04
		VK_Subject_Reacts(25, 9, 6, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1020, 0.5f);
		break;
	case 7420:                          // Low 05
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1030, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8255, 0.5f);
		VK_Subject_Reacts(30, 7, 5, 3);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1040, 0.5f);
		break;
	case 7425:                          // Low 06
		VK_Play_Speech_Line(kActorBulletBob, 1050, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1060, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8260, 0.5f);
		VK_Subject_Reacts(5, 5, 6, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1070, 0.5f);
		break;
	case 7430:                          // Low 07
		VK_Subject_Reacts(15, 7, 6, -6);
		VK_Play_Speech_Line(kActorBulletBob, 1080, 0.5f);
		break;
	case 7435:                          // Low 08
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1100, 0.5f);
		VK_Subject_Reacts(25, 8, 5, -7);
		VK_Play_Speech_Line(kActorMcCoy, 8265, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1110, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1120, 0.5f);
		break;
	case 7440:                          // Low 09
		VK_Subject_Reacts(30, 8, 6, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1130, 0.5f);
		break;
	case 7445:                          // Low 10
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1140, 0.5f);
		VK_Subject_Reacts(80, 8, 8, -10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1150, 0.5f);
		break;
	case 7450:                          // Low 11
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1160, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(60, 8, 8, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1170, 0.5f);
		break;
	case 7455:                          // Low 12
		VK_Subject_Reacts(30, 7, 6, 3);
		VK_Play_Speech_Line(kActorBulletBob, 1180, 0.5f);
		break;
	case 7460:                          // Low 13
		VK_Subject_Reacts(10, 4, 4, 2);
		VK_Play_Speech_Line(kActorBulletBob, 1190, 0.5f);
		break;
	case 7465:                          // Low 14
		VK_Subject_Reacts(15, 5, 3, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1200, 0.5f);
		if (Actor_Query_Friendliness_To_Other(kActorBulletBob, kActorMcCoy) <= 40) {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(kActorBulletBob, 1210, 0.5f);
			VK_Eye_Animates(1);
		}
		break;
	case 7470:                          // Low 15
		VK_Subject_Reacts(50, -4, 0, -5);
		if (_vm->_cutContent) {
			Delay(2000u);
			VK_Play_Speech_Line(kActorMcCoy, 8270, 0.5f);
		}
		// Quote 1220 is *boop* in ENG and ITA versions
		// In DEU version it seems largely redundant; it is one word, identical to the first word of quote 1240.
		// In FRA version it is also one word, similar (but not identical) to how quote 1240 starts.
		// In ESP version it can complement the following quote 1240
		if (_vm->_cutContent) {
			if (_vm->_language == Common::ES_ESP) {
				// play both 1220, 1240
				VK_Play_Speech_Line(kActorBulletBob, 1220, 0.5f);
				VK_Play_Speech_Line(kActorBulletBob, 1240, 0.5f);
			} else if (_vm->_language == Common::FR_FRA
			           || _vm->_language == Common::DE_DEU){
				// play either 1220 or 1240
				if (Random_Query(0, 1)) {
					VK_Play_Speech_Line(kActorBulletBob, 1220, 0.5f);
				} else {
					VK_Play_Speech_Line(kActorBulletBob, 1240, 0.5f);
				}
			} else {
				// play only 1240
				VK_Play_Speech_Line(kActorBulletBob, 1240, 0.5f);
			}
		} else {
			// vanilla mode plays only 1240
			VK_Play_Speech_Line(kActorBulletBob, 1240, 0.5f);
		}
		break;
	case 7475:                          // Medium 01
		VK_Play_Speech_Line(kActorBulletBob, 1250, 0.5f);
		VK_Subject_Reacts(30, 9, 7, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1260, 0.5f);
		break;
	case 7480:                          // Medium 02
		VK_Subject_Reacts(60, 10, 8, -6);
		VK_Play_Speech_Line(kActorBulletBob, 1270, 0.5f);
		break;
	case 7485:                          // Medium 03
		VK_Subject_Reacts(70, 8, 9, 10);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1280, 0.5f);
		break;
	case 7490:                          // Medium 04
		VK_Play_Speech_Line(kActorBulletBob, 1290, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1300, 0.5f);
		VK_Subject_Reacts(10, 11, 10, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1310, 0.5f);
		break;
	case 7495:                          // Medium 05
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(kActorBulletBob, 1320, 0.5f);
		VK_Subject_Reacts(10, 8, 7, 7);
		break;
	case 7515:                          // Medium 06
		VK_Subject_Reacts(25, 7, 7, 0);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1330, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1340, 0.5f);
		break;
	case 7525:                          // Medium 07
		VK_Play_Speech_Line(kActorBulletBob, 1350, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 7, 6, 6);
		break;
	case 7535:                          // Medium 08
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1360, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8275, 0.5f);
		if (_vm->_cutContent) {
				VK_Play_Speech_Line(kActorBulletBob, 1370, 0.5f); // B: they ain't animals. we eat plants
				VK_Subject_Reacts(10, 9, 7, -4);
				VK_Play_Speech_Line(kActorMcCoy, 8540, 0.5f);     // M: actually they are organic
		} else {
				VK_Subject_Reacts(10, 9, 7, -4);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8280, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1380, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1390, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8285, 0.5f);
		break;
	case 7540:                          // Medium 09
		VK_Play_Speech_Line(kActorBulletBob, 1400, 0.5f);
		VK_Subject_Reacts(30, 10, 9, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1410, 0.5f);
		break;
	case 7550:                          // Medium 10
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1420, 0.5f);
		VK_Subject_Reacts(20, 7, 7, -5);
		VK_Play_Speech_Line(kActorBulletBob, 1430, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorMcCoy, 8290, 0.5f); // M: Let's continue
		}
		break;
	case 7565:                          // Medium 11
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
	case 7580:                          // Medium 12
		VK_Subject_Reacts(20, 9, 7, 0);
		VK_Play_Speech_Line(kActorBulletBob, 1480, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 8305, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1490, 0.5f);
		break;
	case 7585:                          // Medium 13
		VK_Play_Speech_Line(kActorBulletBob, 1500, 0.5f);
		VK_Play_Speech_Line(kActorMcCoy, 7590, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 8, 7, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1510, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1520, 0.5f);
		break;
	case 7595:                          // Medium 14
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 10, 9, 15);
		VK_Play_Speech_Line(kActorBulletBob, 1530, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7600:                          // Medium 15
		VK_Subject_Reacts(20, 8, 8, 5);
		VK_Play_Speech_Line(kActorBulletBob, 1540, 0.5f);
		break;
	case 7605:                          // High 01
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 9, 8, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1550, 0.5f);
		break;
	case 7620:                          // High 02
		VK_Play_Speech_Line(kActorBulletBob, 1560, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 7630, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(kActorBulletBob, 1570, 0.5f);
		VK_Subject_Reacts(10, 10, 9, 10);
		break;
	case 7635:                          // High 03
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
		if (_vm->_cutContent) {
			VK_Play_Speech_Line(kActorBulletBob, 1610, 0.5f); // B: I've had enough McCoy
		}
		break;
	case 7670:                          // High 04
		VK_Subject_Reacts(50, 12, 7, 10);
		VK_Play_Speech_Line(kActorBulletBob, 1620, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorBulletBob, 1630, 0.5f);
		break;
	case 7680:                          // High 05
		VK_Play_Speech_Line(kActorBulletBob, 1640, 0.5f);
		VK_Subject_Reacts(15, 5, 6, 5);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(kActorMcCoy, 8320, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1650, 0.5f);
		break;
	case 7690:                          // High 06
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 14, 13, 15);
		VK_Play_Speech_Line(kActorBulletBob, 1660, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(kActorMcCoy, 8325, 0.5f);
		VK_Play_Speech_Line(kActorBulletBob, 1670, 0.5f);
		break;
	case 7705:                          // High 07
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
	case 7740:                          // High 08
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
	case 7750:                          // High 09
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
	case 7770:                          // High 10
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
	default:
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
		askDektora(7385);
		break;
	default:
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
		askDektora(7390);
		break;
	default:
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
		askDektora(7395);
		break;
	default:
		break;
	}
}

} // End of namespace BladeRunner
