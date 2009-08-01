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
 * $URL$
 * $Id$
 */

#include "asylum/scriptman.h"

#include "common/system.h"

DECLARE_SINGLETON(Asylum::ScriptManager);

namespace Asylum {

static bool g_initialized = false;

ScriptManager::ScriptManager() {
	if (!g_initialized) {
		g_initialized		  = true;
		_currentLine 		  = 0;
		_currentLoops 		  = 0;
		_processing 		  = false;
		_delayedSceneIndex 	  = -1;
		_delayedVideoIndex 	  = -1;
		_allowInput			  = true;

		memset(_gameFlags, 0, 1512);
	}
}

void ScriptManager::setScript(ActionDefinitions *action) {
	_currentScript = action;

	if (_currentScript) {
		for (uint8 i = 0; i < _currentScript->commands[0].numLines; i++) {
			debugC(kDebugLevelScripts,
				"Line: %02d/%02d :: 0x%02X (%d, %d, %d, %d, %d, %d, %d, %d, %d)",
				i,
				_currentScript->commands[0].numLines - 1,
				_currentScript->commands[i].opcode,
				_currentScript->commands[i].param1,
				_currentScript->commands[i].param2,
				_currentScript->commands[i].param3,
				_currentScript->commands[i].param4,
				_currentScript->commands[i].param5,
				_currentScript->commands[i].param6,
				_currentScript->commands[i].param7,
				_currentScript->commands[i].param8,
				_currentScript->commands[i].param9);
		}
	}
}

void ScriptManager::setScene(Scene* scene) {
	_scene = scene;
}

void ScriptManager::setScriptIndex(uint32 index) {
	_currentScript 	= 0;
	_currentLine 	= 0;
	setScript(_scene->getActionList(index));
}

void ScriptManager::setGameFlag(int flag) {
	_gameFlags[flag] &= ~(1 << flag);
}

void ScriptManager::processActionList() {
	bool done = false, waitCycle = false;
	int lineIncrement = 1;

	_processing = true;

	if (_currentScript) {
		while (!done && !waitCycle) {
			lineIncrement = 1;	//	Reset line increment value

			if (_currentLoops > 1000) {
				//	TODO - processActionLists has run too many iterations
			}

			ActionCommand currentCommand = _currentScript->commands[_currentLine];

			switch (currentCommand.opcode) {

/* 0x00 */ 	case kReturn0:
				done 		  = true;
				lineIncrement = 0;
				break;

/* 0x01 */  case kSetGameFlag:
				setGameFlag(currentCommand.param1);
				break;

/* 0x02 */  case kClearGameFlag: {
				int flagNum = currentCommand.param1;
				_gameFlags[flagNum] &= ~(1 << flagNum);
			}
				break;

/* 0x03 */  //case kToogleGameFlag:
/* 0x04 */  case kJumpIfGameFlag:
				if (currentCommand.param1) {
					bool doJump = false;
					if (currentCommand.param2)
						doJump = _gameFlags[currentCommand.param1] == 0;
					else
						doJump = _gameFlags[currentCommand.param1] != 0;
					
					if(doJump)
						_currentLine = currentCommand.param3;
				}
				break;

/* 0x05 */  case kHideCursor:
				_scene->_screen->hideCursor();
				_allowInput = false;
				break;

/* 0x06 */  case kShowCursor:
				_scene->_screen->showCursor();
				_allowInput = true;
				break;

/* 0x07 */  case kPlayAnimation: {
				int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				if (barrierIndex >= 0)
					_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags |= 0x20;	//	TODO - enums for flags (0x20 is visible/playing?)
				else
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

/* 0x08 */  //case kMoveScenePosition:
/* 0x09 */  case kHideActor: {
				uint32 actorIndex = 0;
				if (currentCommand.param1 == -1)
					;//actorIndex = _scene->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand.param1;

				if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors))
					_scene->actorVisible(actorIndex, false);
				else
					debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

/* 0x0A */  case kShowActor: {
				uint32 actorIndex = 0;
				if (currentCommand.param1 == -1)
					;//actorIndex = _scene->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand.param1;

				if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
					_scene->actorVisible(actorIndex, true);
				}
				else
					debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

/* 0x0B */  case kSetActorStats: {
				uint32 actorIndex = 0;
				if (currentCommand.param1 == -1)
					;//actorIndex = _scene->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand.param1;

				if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
					_scene->setActorPosition(actorIndex, currentCommand.param2, currentCommand.param3);
					_scene->setActorAction(actorIndex, currentCommand.param4);
				}
				else
					debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

/* 0x0C */  //case kSetSceneMotionStat:
/* 0x0D */  //case kDisableActor:
/* 0x0E */  case kEnableActor: {
				int actorIndex = 0;
				if (currentCommand.param1 == -1)
					;//actorIndex = _scene->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand.param1;

				if (_scene->_sceneResource->getWorldStats()->actors[actorIndex].field_40 == 5) {
					/*
					  v14 = 2468 * characterIndex;
					  v19 = scene.characters[characterIndex].direction;
					  if ( v19 > 4 )
						v20 = 8 - v19;
					  else
						v20 = scene.characters[characterIndex].direction;
					  v21 = scene.characters[0].grResTable[characterIndex + v20 + 616 * characterIndex + 5];
					 LABEL_82:
					  *(int *)((char *)&scene.characters[0].grResId + v14) = v21;
					  v16 = v21;
				LABEL_83:
					  *(int *)((char *)&scene.characters[0].frameCount + v14) = grGetFrameCount__(v16);
					  *(int *)((char *)&scene.characters[0].frameNumber + v14) = 0;
					  goto LABEL_84;
					default:
					  goto LABEL_84;
				  }
				  while ( 1 )
				  {
					v32 = *v30 < (unsigned __int8)*v31;
					if ( *v30 != *v31 )
					  break;
					if ( !*v30 )
					  goto LABEL_70;
					v42 = *(v30 + 1);
					v43 = *(v31 + 1);
					v33 = *(v30 + 1);
					v32 = v42 < v43;
					if ( v42 != v43 )
					  break;
					v30 += 2;
					v31 += 2;
					if ( !v33 )
					{
				LABEL_70:
					  v34 = 0;
					  goto LABEL_72;
					}
				  }
				  v34 = -v32 - (v32 - 1);
				LABEL_72:
				  if ( !v34 )
					v3 = 4;
				LABEL_84:
				  result = 617 * v4;
				  scene.characters[v4].field_40 = v3;

					 */
				}
			}
				break;

/* 0x0F */  case kEnableBarriers: {
				int barIdx = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				int sndIdx = currentCommand.param3;
				int v59 = currentCommand.param2;
				int v61 = _currentScript->counter; // actionList + 1773

				if (!v61 && _scene->getSceneIndex() != 13)
						_scene->_sound->playSfx(_scene->_resPack, ((sndIdx != 0) & 5) - 2146303999);

				if (v61 >= 3 * v59 - 1) {
					_currentScript->counter = 0;
					_scene->_sceneResource->getWorldStats()->barriers[barIdx].field_67C = 0;
					// processActionListSub02(currentCommand.opcode, v59, _currentScript->counter, 2);
					_currentLoops = 1; // v4 = 1;
				} else {
					int v64;
					int v62 = v61 + 1;
					_currentScript->counter = v62;
					if (sndIdx) {
						v64 = 1;
						int v170 = 3 - v62 / v59;
						_scene->_sceneResource->getWorldStats()->barriers[barIdx].field_67C = v170;
					} else {
						v64 = 0;
						_scene->_sceneResource->getWorldStats()->barriers[barIdx].field_67C = v62 / v59 + 1;
					}

					//processActionListSub02(currentCommand.opcode, v59, _currentScript->counter, v64);

				}
			}
				break;

/* 0x10 */  case kReturn:
				done 		  = true;
				lineIncrement = 0;
				break;

/* 0x11 */  case kDestroyObject: {
				int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				if (barrierIndex >= 0)
					_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags &= 0xFFFFDF;	//	TODO - enums for flags (0x20 is visible/playing?)
				else
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

/* 0x12 */  //case k_unk12_JMP_WALK_ACTOR:
/* 0x13 */  //case k_unk13_JMP_WALK_ACTOR:
/* 0x14 */  //case k_unk14_JMP_WALK_ACTOR:
/* 0x15 */  //case k_unk15:
/* 0x16 */  //case kResetAnimation:
/* 0x17 */  //case kClearFlag1Bit0:
/* 0x18 */  //case k_unk18_PLAY_SND:
/* 0x19 */  //case kJumpIfFlag2Bit0:
/* 0x1A */  //case kSetFlag2Bit0:
/* 0x1B */  //case kClearFlag2Bit0:
/* 0x1C */  //case kJumpIfFlag2Bit2:
/* 0x1D */  //case kSetFlag2Bit2:
/* 0x1E */  //case kClearFlag2Bit2:
/* 0x1F */  //case kJumpIfFlag2Bit1:
/* 0x20 */  //case kSetFlag2Bit1:
/* 0x21 */  //case kClearFlag2Bit1:
/* 0x22 */  //case k_unk22:
/* 0x23 */  //case k_unk23:
/* 0x24 */  //case k_unk24:
/* 0x25 */  //case kRunEncounter:
/* 0x26 */  //case kJumpIfFlag2Bit4:
/* 0x27 */  //case kSetFlag2Bit4:
/* 0x28 */  //case kClearFlag2Bit4:
/* 0x29 */  //case kSetActorField638:
/* 0x2A */  //case kJumpIfActorField638:
/* 0x2B */  case kChangeScene:
				_delayedSceneIndex = currentCommand.param1 + 4;
				debug(kDebugLevelScripts, "Queueing Scene Change to scene %d...", currentCommand.param1 + 4);
				break;

/* 0x2C */  //case k_unk2C_ActorSub:
/* 0x2D */  case kPlayMovie:
				_delayedVideoIndex = currentCommand.param1;
				break;

/* 0x2E */  //case kStopAllObjectsSounds:
/* 0x2F */  //case kSetActionFlag01:
/* 0x30 */  //case kClearActionFlag01:
/* 0x31 */  //case kResetSceneRect:
/* 0x32 */  //case kChangeMusicById:
/* 0x33 */  //case kStopMusic:
/* 0x34 */  //case k_unk34_Status:
/* 0x35 */  //case k_unk35:
/* 0x36 */  //case k_unk36:
/* 0x37 */  case kRunBlowUpPuzzle: {
				int blowUpPuzzleIdx = currentCommand.param1;
				// TODO: do proper blow up puzzle initialization
			}
				break;

/* 0x38 */  //case kJumpIfFlag2Bit3:
/* 0x39 */  //case kSetFlag2Bit3:
/* 0x3A */  //case kClearFlag2Bit3:
/* 0x3B */  //case k_unk3B_PALETTE_MOD:
/* 0x3C */  //case k_unk3C_CMP_VAL:
/* 0x3D */  case kWaitUntilFramePlayed: {
				int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				if (barrierIndex >= 0) {
					uint32 frameNum = 0;
					if (currentCommand.param2 == -1)
						frameNum = _scene->_sceneResource->getWorldStats()->barriers[barrierIndex].frameCount - 1;
					else
						frameNum = currentCommand.param2;

					if (_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].tickCount < frameNum) {
						lineIncrement = 0;
						waitCycle = true;
					}
				} else
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.",
							currentCommand.param1,
							_scene->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x3E */  //case kUpdateMatteBars:
/* 0x3F */  //case k_unk3F:
/* 0x40 */  //case k_unk40_SOUND:
/* 0x41 */  case kPlaySpeech:
				//	TODO - Add support for other param options
				if (currentCommand.param1 >= 0) {
					if (currentCommand.param3 && currentCommand.param1 > 0)	//	HACK - Find out why sometimes an offset is needed and other times not
						_scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1 - 9);
					else
						_scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1);
				} else
					debugC(kDebugLevelScripts, "Requested invalid sound ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
				break;

/* 0x42 */  //case k_unk42:
/* 0x43 */  //case k_unk43:
/* 0x44 */  //case kPaletteFade:
/* 0x45 */  //case kStartPaletteFadeThread:
/* 0x46 */  //case k_unk46:
/* 0x47 */  //case kActorFaceObject:
/* 0x48 */  //case k_unk48_MATTE_01:
/* 0x49 */  //case k_unk49_MATTE_90:
/* 0x4A */  //case kJumpIfSoundPlaying:
/* 0x4B */  //case kChangePlayerCharacterIndex:
/* 0x4C */  //case kChangeActorField40:
/* 0x4D */  //case kStopSound:
/* 0x4E */  //case k_unk4E_RANDOM_COMMAND:
/* 0x4F */  //case kDrawGame:
/* 0x50 */  //case kQuit:
/* 0x51 */  //case kJumpObjectFrame:
/* 0x52 */  //case k_unk52:
/* 0x53 */  //case k_unk53:
/* 0x54 */  //case k_unk54_SET_ACTIONLIST_6EC:
/* 0x55 */  //case k_unk55:
/* 0x56 */  //case k_unk56:
/* 0x57 */  //case k_unk57:
/* 0x58 */  //case k_unk58:
/* 0x59 */  //case k_unk59:
/* 0x5A */  //case k_unk5A:
/* 0x5B */  //case k_unk5B:
/* 0x5C */  //case k_unk5C:
/* 0x5D */  //case k_unk5D:
/* 0x5E */  //case k_unk5E:
/* 0x5F */  //case k_unk5F:
/* 0x60 */  //case k_unk60_SET_OR_CLR_ACTIONAREA_FLAG:
/* 0x61 */  //case k_unk61:
/* 0x62 */  //case k_unk62_SHOW_OPTIONS_SCREEN:

			default:
				debugC(kDebugLevelScripts, "Unhandled opcode 0x%02X in Scene %d Line %d.",
						currentCommand.opcode,
						_scene->getSceneIndex(),
						_currentLine);
				break;

			}	// end switch

			_currentLine += lineIncrement;
			_currentLoops++;

		}	// end while

		if (done) {
			_currentLine 	= 0;
			_currentLoops	= 0;
			_currentScript 	= 0;
		}

	}

	_processing = false;
}

void ScriptManager::processActionListSub02(ActionCommand *command, int a3, int a4) {
	int v4 = 0;
	int result;
	int barrierIdx = 0;
	if (a4) {
		if (a4 == 1) {
			;
		} else {
			result = a4 - 2;
			int v8 = command->param4;

			for (int i = 7; i > 0; i--) {
				barrierIdx = _scene->getResources()->getBarrierIndexById(v8);
				if (barrierIdx)
					_scene->getResources()->getWorldStats()->barriers[barrierIdx].field_67C = 0;
				v8 += 4;
			}
		}

		switch (_scene->getSceneIndex()) {
		case 7:
			break;
		case 6:
			break;
		case 8:
			break;
		case 3:
			break;
		case 4:
			break;
		default:
			return;
		}


	}
}

ScriptManager::~ScriptManager() {
	// TODO Auto-generated destructor stub
}

} // end of namespace Asylum
