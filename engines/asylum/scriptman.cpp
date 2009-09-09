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

#include "common/system.h"

#include "asylum/scriptman.h"
#include "asylum/shared.h"

DECLARE_SINGLETON(Asylum::ScriptManager);

namespace Asylum {

static bool g_initialized = false;

ScriptManager::ScriptManager() {
	if (!g_initialized) {
		g_initialized		= true;
		_currentLine 		= 0;
		_currentLoops 		= 0;
		_processing 		= false;
		_delayedSceneIndex 	= -1;
		_delayedVideoIndex 	= -1;
		_allowInput			= true;
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

void ScriptManager::setScriptIndex(uint32 index) {
	_currentScript 	= 0;
	_currentLine 	= 0;
	setScript(Shared.getScene()->getActionList(index));
}

int ScriptManager::checkBarrierFlags(int barrierId) {
    int flags = Shared.getScene()->getResources()->getBarrierById(barrierId)->flags;
    return flags & 1 && (flags & 8 || flags & 0x10000);
}

int ScriptManager::setBarrierNextFrame(int barrierId, int barrierFlags) {
    int barrierIndex = Shared.getScene()->getResources()->getBarrierIndexById(barrierId);

    BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierByIndex(barrierIndex);
    int newFlag = barrierFlags | 1 | barrier->flags;
    barrier->flags |= barrierFlags | 1;

    if(newFlag & 0x10000) {
        barrier->frameIdx = barrier->frameCount - 1;
    } else {
        barrier->frameIdx = 0;
    }

    return barrierIndex;
}

int ScriptManager::processActionList() {
	bool done = false, waitCycle = false;
	int lineIncrement = 1;

	_processing = true;

	if (_currentScript) {
		while (!done && !waitCycle) {
			lineIncrement = 1;	//	Reset line increment value

			if (_currentLoops > 1000) {
				//	TODO - processActionLists has run too many iterations
			}

			ActionCommand *currentCommand = &_currentScript->commands[_currentLine];

			switch (currentCommand->opcode) {

/* 0x00 */ 	case kReturn0:
				done 		  = true;
				lineIncrement = 0;
				break;

/* 0x01 */  case kSetGameFlag: {
                int flagNum = currentCommand->param1;
                if(flagNum >= 0)
				    Shared.setGameFlag(currentCommand->param1);
            }
				break;

/* 0x02 */  case kClearGameFlag: {
				int flagNum = currentCommand->param1;
				if(flagNum >= 0)
				    Shared.clearGameFlag(currentCommand->param1);
			}
				break;

/* 0x03 */  case kToogleGameFlag: {
                int flagNum = currentCommand->param1;
				if(flagNum >= 0)
				    Shared.toggleGameFlag(currentCommand->param1);
            }
                break;
/* 0x04 */  case kJumpIfGameFlag: {
                int flagNum = currentCommand->param1;
				if (flagNum) {
					bool doJump = Shared.isGameFlagSet(flagNum);
					if (currentCommand->param2)
						doJump = Shared.isGameFlagNotSet(flagNum);
					
					if (doJump)
						_currentLine = currentCommand->param3;
				}
            }
				break;

/* 0x05 */  case kHideCursor:
				Shared.getScene()->getCursor()->hide();
				_allowInput = false;
				break;

/* 0x06 */  case kShowCursor:
				Shared.getScene()->getCursor()->show();
				_allowInput = true;
                // TODO: clear_flag_01()
				break;

/* 0x07 */  case kPlayAnimation: {
                int barrierId = currentCommand->param1;
                if(currentCommand->param2 == 2) {
                    if(!checkBarrierFlags(barrierId)) {
                        currentCommand->param2 = 1;
                        break;
                    }
                    lineIncrement = 1;
                } else {
                    int barrierIndex = Shared.getScene()->getResources()->getBarrierIndexById(barrierId);
                    BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierByIndex(barrierIndex);

                    if(currentCommand->param4) { // RECHECK THIS
                        int newBarriedIndex = 213 * barrierIndex;
                        barrier->flags &= 0xFFFEF1C7;
                        Shared.getScene()->getResources()->getBarrierByIndex(newBarriedIndex)->flags = barrier->flags | 0x20;
                    } else if(currentCommand->param3) {
                        barrier->flags &= 0xFFFEF1C7;
                        barrier->flags |= 0x10000;
                    } else {
                        barrier->flags &= 0x10000;
                        if(barrier->flags == 0) {
                            barrier->flags &= 0x10E38;
                            if(barrier->flags == 0) {
                                barrier->flags |= 8;
                            }
                        } else {
                            barrier->flags |= 8;
                            barrier->flags &= 0xFFFEFFFF;
                        }
                    }

                    setBarrierNextFrame(barrierId, barrier->flags);

                    if(barrier->field_688 == 1) {
                        // TODO: get barrier position
                    }

                    if(currentCommand->param2) {
                        currentCommand->param2 = 2;
                        lineIncrement = 1;
                    }
                }
			}
				break;

/* 0x08 */  case kMoveScenePosition: {
                WorldStats   *ws = Shared.getScene()->getResources()->getWorldStats();
                Common::Rect *sr = &ws->sceneRects[ws->sceneRectIdx];

                if (currentCommand->param3 < 1) {
                    ws->xLeft = currentCommand->param1;
                    ws->yTop  = currentCommand->param2;
                    ws->motionStatus = 3;
                } else if (!currentCommand->param4) {
                    ws->motionStatus = 5;
                    ws->targetX  = currentCommand->param1;
                    ws->targetY  = currentCommand->param2;
                    ws->field_A0 = currentCommand->param3;

                    if (ws->targetX < (uint32)sr->left)
                        ws->targetX = sr->left;
                    if (ws->targetY < (uint32)sr->top)
                        ws->targetY = sr->top;
                    if (ws->targetX + 640 > (uint32)sr->right)
                        ws->targetX = sr->right - 640;
                    if (ws->targetY + 480 > (uint32)sr->bottom)
                        ws->targetY = sr->bottom - 480;

                    // TODO: reverse asm block

                } else if (currentCommand->param5) {
                    if (ws->motionStatus == 2)
                        lineIncrement = 1;
                    else
                        currentCommand->param5 = 0;
                } else {
                    currentCommand->param5 = 1;
                    ws->motionStatus = 2;
                    ws->targetX  = currentCommand->param1;
                    ws->targetY  = currentCommand->param2;
                    ws->field_A0 = currentCommand->param3;

                    if (ws->targetX + 640 > ws->width)
                        ws->targetX = ws->width - 640;
                    if (ws->targetX < (uint32)sr->left)
                        ws->targetX = sr->left;
                    if (ws->targetY < (uint32)sr->top)
                        ws->targetY = sr->top;
                    if (ws->targetX + 640 > (uint32)sr->right)
                        ws->targetX = sr->right - 640;
                    if (ws->targetY + 480 > (uint32)sr->bottom)
                        ws->targetY = sr->bottom - 480;

                    // TODO: reverse asm block
                }
            }
                break;
/* 0x09 */  case kHideActor: {
				uint32 actorIndex = 0;
				if (currentCommand->param1 == -1)
					;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand->param1;

				if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors))
					Shared.getScene()->actorVisible(actorIndex, false);
				else
					debugC(kDebugLevelScripts,
							"Requested invalid actor ID:0x%02X in Scene %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x0A */  case kShowActor: {
				uint32 actorIndex = 0;
				if (currentCommand->param1 == -1)
					;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand->param1;

				if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors))
					Shared.getScene()->actorVisible(actorIndex, true);
				else
					debugC(kDebugLevelScripts,
							"Requested invalid actor ID:0x%02X in Scene %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x0B */  case kSetActorStats: {
				uint32 actorIndex = 0;
				if (currentCommand->param1 == -1)
					;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand->param1;

				if ((actorIndex >= 0) && (actorIndex < Shared.getScene()->getResources()->getWorldStats()->numActors)) {
					Shared.getScene()->setActorPosition(actorIndex, currentCommand->param2, currentCommand->param3);
					Shared.getScene()->setActorAction(actorIndex, currentCommand->param4);
				}
				else
					debugC(kDebugLevelScripts,
							"Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x0C */  case kSetSceneMotionStat:
                Shared.getScene()->getResources()->getWorldStats()->motionStatus = currentCommand->param1;
                break;
/* 0x0D */  case kDisableActor: {
				int actorIndex = 0;
				if (currentCommand->param1 == -1)
					;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand->param1;
				Shared.getScene()->getResources()->getMainActor()->disable(actorIndex);
			}
				break;
/* 0x0E */  case kEnableActor: {
				int actorIndex = 0;
				if (currentCommand->param1 == -1)
					;//actorIndex = Shared.getScene()->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand->param1;

				if (Shared.getScene()->getResources()->getWorldStats()->actors[actorIndex].field_40 == 5) {
					enableActorSub(actorIndex, 4);
				}
			}
				break;

/* 0x0F */  case kEnableBarriers: {
				int barIdx = Shared.getScene()->getResources()->getBarrierIndexById(currentCommand->param1);
				uint32 sndIdx = currentCommand->param3;
				uint32 v59    = currentCommand->param2;

				if (!_currentScript->counter && Shared.getScene()->getSceneIndex() != 13 && sndIdx != 0) {
					ResourcePack *sfx = new ResourcePack(18);
					Shared.getSound()->playSfx(sfx, ((unsigned int)(sndIdx != 0) & 5) + 0x80120001);
					delete sfx;
					//Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(),sndIdx + 86);
				}

				if (_currentScript->counter >= 3 * v59 - 1) {
					_currentScript->counter = 0;
					Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = 0;
					processActionListSub02(_currentScript, currentCommand, 2);
					_currentLoops = 1; // v4 = 1;
				} else {
					int v64;
					int v62 = _currentScript->counter + 1;
					_currentScript->counter = v62;
					if (sndIdx) {
						v64 = 1;
						int v170 = 3 - v62 / v59;
						Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = v170;
					} else {
						v64 = 0;
						Shared.getScene()->getResources()->getWorldStats()->barriers[barIdx].field_67C = v62 / v59 + 1;
					}

					processActionListSub02(_currentScript, currentCommand, v64);

				}
			}
				break;

/* 0x10 */  case kReturn:
				done 		  = true;
				lineIncrement = 0;
				break;

/* 0x11 */  case kDestroyBarrier: {
				BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
                if (barrier) {
                    barrier->flags &= 0xFFFFFFFE;
                    barrier->flags |= 0x20000;
                    Shared.getScreen()->deleteGraphicFromQueue(barrier->resId);
                } else
					debugC(kDebugLevelScripts,
							"Requested invalid object ID:0x%02X in Scene %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x12 */  //case k_unk12_JMP_WALK_ACTOR:
/* 0x13 */  //case k_unk13_JMP_WALK_ACTOR:
/* 0x14 */  //case k_unk14_JMP_WALK_ACTOR:
/* 0x15 */  //case k_unk15:
/* 0x16 */  case kResetAnimation: {
                BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
                if ((barrier->flags & 0x10000) == 0) {
                    barrier->frameIdx = 0;
                } else {
                    barrier->frameIdx = barrier->frameCount - 1;
                }
            }
                break;

/* 0x17 */  case kClearFlag1Bit0: {
                BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
                barrier->flags &= 0xFFFFFFFE;
            }
                break;

/* 0x18 */  //case k_unk18_PLAY_SND:
/* 0x19 */  case kJumpIfFlag2Bit0: {
				int targetType = currentCommand->param2;
				if (targetType <= 0)
					done = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1)->flags2 & 1 == 0;
				else
					if (targetType == 1) // v4 == 1, so 1
						done = Shared.getScene()->getResources()->getActionAreaById(currentCommand->param1)->actionType & 1 == 0;
					else
						done = Shared.getScene()->getResources()->getWorldStats()->actors[currentCommand->param1].flags2 & 1 == 0;
			}
				break;

/* 0x1A */  case kSetFlag2Bit0: {
				int targetType = currentCommand->param2;
				if (targetType == 2)
					Shared.getScene()->getResources()->getWorldStats()->actors[currentCommand->param1].flags2 |= 1;
				else
					if (targetType == 1)
						Shared.getScene()->getResources()->getActionAreaById(currentCommand->param1)->actionType |= 1;
					else
						Shared.getScene()->getResources()->getBarrierById(currentCommand->param1)->flags2 |= 1;
			}
				break;
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
				_delayedSceneIndex = currentCommand->param1 + 4;
				debug(kDebugLevelScripts,
						"Queueing Scene Change to scene %d...",
						_delayedSceneIndex);
				break;

/* 0x2C */  //case k_unk2C_ActorSub:
/* 0x2D */  case kPlayMovie:
                // TODO: add missing code here
				_delayedVideoIndex = currentCommand->param1;
				break;

/* 0x2E */  case kStopAllBarriersSounds:
                // TODO: do this for all barriers that have sfx playing
                Shared.getSound()->stopSfx();
                break;

/* 0x2F */  //case kSetActionFlag01:
/* 0x30 */  //case kClearActionFlag01:
/* 0x31 */  //case kResetSceneRect:
/* 0x32 */  //case kChangeMusicById:
/* 0x33 */  case kStopMusic:
                Shared.getSound()->stopMusic();
                break;

/* 0x34 */  case k_unk34_Status:
                if (currentCommand->param1 >= 2) {
                    currentCommand->param1 = 0;
                } else {
                    currentCommand->param1++;
                    lineIncrement = 1;
                }
                break;
/* 0x35 */  //case k_unk35:
/* 0x36 */  //case k_unk36:
/* 0x37 */  case kRunBlowUpPuzzle: { // FIXME: improve this to call other blowUpPuzzles than VCR
				int blowUpPuzzleIdx = currentCommand->param1;
                Shared.getScene()->setBlowUpPuzzle(new BlowUpPuzzleVCR());
                Shared.getScene()->getBlowUpPuzzle()->openBlowUp();
			}
				break;

/* 0x38 */  //case kJumpIfFlag2Bit3:
/* 0x39 */  //case kSetFlag2Bit3:
/* 0x3A */  //case kClearFlag2Bit3:
/* 0x3B */  //case k_unk3B_PALETTE_MOD:
/* 0x3C */  case k_unk3C_CMP_VAL: {
				if (currentCommand->param1) {
                    if (currentCommand->param2 >= currentCommand->param1) {
						currentCommand->param2 = 0;
                    } else {
						currentCommand->param2++;
						lineIncrement = 1;
                    }
				}
			}
				break;
/* 0x3D */  case kWaitUntilFramePlayed: {
				BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
				if (barrier) {
					uint32 frameNum = 0;
					if (currentCommand->param2 == -1)
						frameNum = barrier->frameCount - 1;
					else
						frameNum = currentCommand->param2;

					if (barrier->frameIdx < frameNum) {
						lineIncrement = 0;
						waitCycle = true;
					}
				} else
					debugC(kDebugLevelScripts,
							"Requested invalid object ID:0x%02X in Scene %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
				break;

/* 0x3E */  case kUpdateWideScreen: {
                int barSize = currentCommand->param1;
                if(barSize >= 22) {
                    currentCommand->param1 = 0;
                } else {
                    Shared.getScreen()->drawWideScreen(4 * barSize);
                    currentCommand->param1++;
                }
            }
                break;
            
/* 0x3F */  //case k_unk3F:
/* 0x40 */  //case k_unk40_SOUND:
/* 0x41 */  case kPlaySpeech: {
				//	TODO - Add support for other param options
				uint32 sndIdx = currentCommand->param1;
				if ((int)sndIdx >= 0) {
					if (sndIdx >= 259) {
						sndIdx -= 9;
						Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(), sndIdx - 0x7FFD0000);
					} else {
						Shared.getSound()->playSfx(Shared.getScene()->getSpeechPack(), sndIdx);
					}

				} else
					debugC(kDebugLevelScripts,
							"Requested invalid sound ID:0x%02X in Scene %d Line %d.",
							currentCommand->param1,
							Shared.getScene()->getSceneIndex(),
							_currentLine);
			}
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
/* 0x4C */  case kChangeActorField40: { // TODO: figure out what is this field and what values are set
                int actorIdx = currentCommand->param1;
                int fieldType = currentCommand->param2;
                if(fieldType) {
                    if(Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 < 11) {
                        Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 = 14;
                    }
                } else {
                    Shared.getScene()->getResources()->getWorldStats()->actors[actorIdx].field_40 = 4;
                }
            }
                break;
/* 0x4D */  //case kStopSound:
/* 0x4E */  //case k_unk4E_RANDOM_COMMAND:
/* 0x4F */  case kClearScreen:
                if(currentCommand->param1) {
                    Shared.getScreen()->clearScreen();
                }
                break;

/* 0x50 */  //case kQuit:
/* 0x51 */  case kJumpBarrierFrame: {
				BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
				if (currentCommand->param2 == -1) {
					currentCommand->param2 = barrier->frameCount - 1;
				}

				if (currentCommand->param3 && currentCommand->param2 == barrier->frameIdx) {
					break;
				} else if (currentCommand->param4 && currentCommand->param2 < barrier->frameIdx) {
					break;
				} else if (currentCommand->param5 && currentCommand->param2 > barrier->frameIdx) {
					break;
				} else if (currentCommand->param6 && currentCommand->param2 <= barrier->frameIdx) {
					break;
				} else if (currentCommand->param7 && currentCommand->param2 >= barrier->frameIdx) {
					break;
				} else if (currentCommand->param8 && currentCommand->param2 != barrier->frameIdx) {
					break;
				}

				ActionCommand *cmd = &_currentScript->commands[currentCommand->param9];
				if (cmd->opcode != kReturn && cmd->opcode) {
					done = true;
				}
			}
				break;
/* 0x52 */  //case k_unk52:
/* 0x53 */  //case k_unk53:
/* 0x54 */  case k_unk54_SET_ACTIONLIST_6EC:
                if (currentCommand->param2) {
                    _currentScript->field_1BB0 = rand() % currentCommand->param1;
                } else {
                    _currentScript->field_1BB0 = currentCommand->param1;
                }
                break;
/* 0x55 */  case k_unk55: {
                if (!currentCommand->param2) {
                    if (currentCommand->param3 && _currentScript->field_1BB0 < currentCommand->param1)
                        break;                       
                    else if (currentCommand->param4 && _currentScript->field_1BB0 > currentCommand->param1)
                        break;
                    else if (currentCommand->param5 && _currentScript->field_1BB0 <= currentCommand->param1)
                        break;
                    else if (currentCommand->param6 && _currentScript->field_1BB0 >= currentCommand->param1)
                        break;
                    else if (currentCommand->param7 && _currentScript->field_1BB0 != currentCommand->param1)
                        break;
                } else if(_currentScript->field_1BB0 == currentCommand->param1)
                    break;
                
                ActionCommand *cmd = &_currentScript->commands[currentCommand->param8];
				if (cmd->opcode != kReturn && cmd->opcode) {
					done = true;
                } else {
                    lineIncrement = currentCommand->param8;
                }
            }
                break;
/* 0x56 */  //case k_unk56:
/* 0x57 */  case kSetResourcePalette: {
                if (currentCommand->param1 > 0) {
                    Shared.getScreen()->setPalette(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[currentCommand->param1]);
                }
            }    
                break;
/* 0x58 */  case kSetBarrierFrameIdxFlaged: {
                BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
                if (currentCommand->param3) {
                    barrier->flags = 1 | barrier->flags;
                } else {
                    barrier->flags = barrier->flags & 0xFFFFFFFE;
                }
                barrier->frameIdx = currentCommand->param2;
            }    
                break;           
/* 0x59 */  //case k_unk59:
/* 0x5A */  //case k_unk5A:
/* 0x5B */  //case k_unk5B:
/* 0x5C */  //case k_unk5C:
/* 0x5D */  //case k_unk5D:
/* 0x5E */  //case k_unk5E:
/* 0x5F */  case kSetBarrierLastFrameIdx: {
                BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierById(currentCommand->param1);
                if (barrier->frameIdx == barrier->frameCount - 1) {
                    lineIncrement = 0;
                    barrier->flags &= 0xFFFEF1C7;
                } else {
                    lineIncrement = 1;
                }
            }
                break;
/* 0x60 */  //case k_unk60_SET_OR_CLR_ACTIONAREA_FLAG:
/* 0x61 */  case k_unk61:
                if (currentCommand->param2) {
                    if (Shared.getScene()->getResources()->getWorldStats()->field_E860C == -1) {
                        lineIncrement = 0;
                        currentCommand->param2 = 0;
                    } else {
                        lineIncrement = 1;
                    }
                } else {
                    // TODO: do something for scene number 9
                    currentCommand->param2 = 1;
                    lineIncrement = 1;
                }
                break;
/* 0x62 */  //case k_unk62_SHOW_OPTIONS_SCREEN:
/* 0x63 */  //case k_unk61:

			default:
				warning("Unhandled opcode 0x%02X in Scene %d Line %d.",
						currentCommand->opcode,
						Shared.getScene()->getSceneIndex(),
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

			Shared.clearGameFlag(183);
		}

	}

	_processing = false;

    return 0;
}

void ScriptManager::processActionListSub02(ActionDefinitions* script, ActionCommand *command, int a4) {
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
				barrierIdx = Shared.getScene()->getResources()->getBarrierIndexById(v8);
				if (barrierIdx >= 0)
					Shared.getScene()->getResources()->getWorldStats()->barriers[barrierIdx].field_67C = 0;
				v8 += 4;
			}
		}
		// TODO
		switch (Shared.getScene()->getSceneIndex()) {
		case 7:
			warning("Scene 7 / v4 != 0 Not Implemented");
			break;
		case 6:
			warning("Scene 6 / v4 != 0 Not Implemented");
			break;
		case 8:
			warning("Scene 8 / v4 != 0 Not Implemented");
			break;
		case 3:
			warning("Scene 3 / v4 != 0 Not Implemented");
			break;
		case 4:
			warning("Scene 4 / v4 != 0 Not Implemented");
			break;
		default:
			return;
		}
	} else {
		int v13 = command->param4;
		int v4 = script->counter / command->param2 + 4;
		for (int i = 7; i > 0; i--) {
			barrierIdx = Shared.getScene()->getResources()->getBarrierIndexById(v13);
			if (barrierIdx >= 0)
				Shared.getScene()->getResources()->getWorldStats()->barriers[barrierIdx].field_67C = v4;
			v13 += 4;
		}
		// TODO
		switch (Shared.getScene()->getSceneIndex()) {
		case 7:
			warning("Scene 7 / v4 = 0 Not Implemented");
			break;
		case 6:
			warning("Scene 6 / v4 = 0 Not Implemented");
			break;
		case 8:
			warning("Scene 8 / v4 = 0 Not Implemented");
			break;
		case 3:
			warning("Scene 3 / v4 = 0 Not Implemented");
			break;
		case 4:
			warning("Scene 4 / v4 = 0 Not Implemented");
			break;
		default:
			return;
		}
	}

}

void ScriptManager::enableActorSub(int actorIndex, int condition) {

}

ScriptManager::~ScriptManager() {
	// TODO Auto-generated destructor stub
}

} // end of namespace Asylum
