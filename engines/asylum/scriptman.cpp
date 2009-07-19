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
		_currentTarget		  = kTargetNothing;
		_currentTargetBarrier = 0;
		_currentTargetAction  = 0;

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
	} else {
		_currentTarget = kTargetNothing;
	}
}

void ScriptManager::setScriptTarget(BarrierItem *barrier) {
	_currentTarget = kTargetBarrier;
	_currentTargetBarrier = barrier;
	setScriptIndex(_currentTargetBarrier->actionListIdx);
}
void ScriptManager::setScriptTarget(ActionItem *action) {
	_currentTarget = kTargetAction;
	_currentTargetAction = action;
	setScriptIndex(_currentTargetAction->actionListIdx1);
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

			case kReturn0:
				done 		  = true;
				lineIncrement = 0;
				break;

			case kJumpIfGameFlag:
				if (currentCommand.param1) {
					bool doJump = false;
					if (currentCommand.param2) {
						doJump = _gameFlags[currentCommand.param1] == 0;
					}
					else {
						doJump = _gameFlags[currentCommand.param1] != 0;
					}
					
					if(doJump)
						_currentLine = currentCommand.param3;
				}
				break;

			case kSetGameFlag:
				setGameFlag(currentCommand.param1);
				break;

			case kClearGameFlag: {
				int flagNum = currentCommand.param1;
				_gameFlags[flagNum] &= ~(1 << flagNum);
			}
				break;

			case kShowCursor:
				_scene->_screen->showCursor();
				_allowInput = true;
				break;

			case kHideCursor:
				_scene->_screen->hideCursor();
				_allowInput = false;
				break;

			case kPlayAnimation: {
				int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				if (barrierIndex >= 0)
					_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags |= 0x20;	//	TODO - enums for flags (0x20 is visible/playing?)
				else
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

			case kHideActor: {
				uint32 actorIndex = 0;
				if (currentCommand.param1 == -1)
					;//actorIndex = _scene->getWorldStats()->playerActor;
				else
					actorIndex = currentCommand.param1;

				if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
					_scene->actorVisible(actorIndex, false);
				}
				else
					debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

			case kShowActor:  {
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

			case kSetActorStats: {
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

			case kReturn:
				done 		  = true;
				lineIncrement = 0;
				break;

			case kDestroyObject: {
				int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
				if (barrierIndex >= 0)
					_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags &= 0xFFFFDF;	//	TODO - enums for flags (0x20 is visible/playing?)
				else
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
			}
				break;

			case kChangeScene:
				_delayedSceneIndex = currentCommand.param1 + 4;
				debug(kDebugLevelScripts, "Queueing Scene Change to scene %d...", currentCommand.param1 + 4);
				break;

			case kPlayMovie:
				_delayedVideoIndex = currentCommand.param1;
				break;

            case kRunBlowUpPuzzle: {
                int blowUpPuzzleIdx = currentCommand.param1;
                // TODO: do proper blow up puzzle initialization
                //_scene->_blowUp = new BlowUpPuzzleVCR(_scene->_screen, _scene->_sound, _scene);
            }
                break;

			case kWaitUntilFramePlayed: {
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
				} else {
					debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
				}
				break;
			}

			case kPlaySpeech:
				//	TODO - Add support for other param options
				if (currentCommand.param1 >= 0) {
 					if (currentCommand.param3 && currentCommand.param1 > 0)	//	HACK - Find out why sometimes an offset is needed and other times not
						_scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1 - 9);
					else
						_scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1);
				} else
					debugC(kDebugLevelScripts, "Requested invalid sound ID:0x%02X in Scene %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentLine);
				break;

			default:
				debugC(kDebugLevelScripts, "Unhandled opcode 0x%02X in Scene %d Line %d.", currentCommand.opcode, _scene->getSceneIndex(), _currentLine);
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

ScriptManager::~ScriptManager() {
	// TODO Auto-generated destructor stub
}

} // end of namespace Asylum
