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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/interpreter.h"
#include "asylum/sceneres.h"

namespace Asylum {

Interpreter::Interpreter(Scene *scene): _scene(scene) {
    _currentLine = 0;
    _currentScriptIndex = _scene->getDefaultActionIndex();
    _currentLoops = 0;
    _processing = false;
}

Interpreter::~Interpreter() {
}

void Interpreter::processActionLists() {
    bool done = false, waitCycle = false;
    int lineIncrement = 1;
    _processing = true;

    if (_currentScriptIndex != -1) {
        ActionDefinitions *_currentScript = _scene->getActionList(_currentScriptIndex);
        if (!_currentScript)
            return;
        
        while (!done && !waitCycle) {
            lineIncrement = 1;  //  Reset line increment value
            
            if (_currentLoops > 1000) {
                //  TODO - processActionLists has run too many interations
            }
            
            ActionCommand currentCommand = _currentScript->commands[_currentLine];
            
            switch (currentCommand.opcode) {
                case kReturn0:
                    done = true;
                    lineIncrement = 0;
                    break;
                case kShowCursor:
                    _scene->_screen->showCursor();
                    //  TODO - Enable click events
                    break;
                case kHideCursor:
                    _scene->_screen->hideCursor();
                    //  TODO - Disable click events
                    break;
                case kPlayAnimation: {
                    int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
                    if (barrierIndex >= 0)
                        _scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags |= 0x20;  //  TODO - enums for flags (0x20 is visible/playing?)
                    else
                        debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kHideActor:
                 {
                    int actorIndex = 0;
                    if (currentCommand.param1 == -1)
                        ;//actorIndex = _scene->getWorldStats()->playerActor;
                    else
                        actorIndex = currentCommand.param1;
                        
                    if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
                        _scene->actorVisible(actorIndex, false);
                    }
                    else
                        debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kShowActor:
                 {
                    int actorIndex = 0;
                    if (currentCommand.param1 == -1)
                        ;//actorIndex = _scene->getWorldStats()->playerActor;
                    else
                        actorIndex = currentCommand.param1;
                        
                    if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
                        _scene->actorVisible(actorIndex, true);
                    }
                    else
                        debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kSetActorStats: {
                    int actorIndex = 0;
                    if (currentCommand.param1 == -1)
                        ;//actorIndex = _scene->getWorldStats()->playerActor;
                    else
                        actorIndex = currentCommand.param1;
                        
                    if ((actorIndex >= 0) && (actorIndex < _scene->_sceneResource->getWorldStats()->numActors)) {
                        _scene->setActorPosition(actorIndex, currentCommand.param2, currentCommand.param3);
                        _scene->setActorAction(actorIndex, 112);//currentCommand.param4);
                    }
                    else
                        debugC(kDebugLevelScripts, "Requested invalid actor ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kReturn:
                    done = true;
                    lineIncrement = 0;
                    break;
                case kDestroyObject: {
                    int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
                    if (barrierIndex >= 0)
                        _scene->_sceneResource->getWorldStats()->barriers[barrierIndex].flags &= 0xFFFFDF;  //  TODO - enums for flags (0x20 is visible/playing?)
                    else
                        debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kWaitUntilFramePlayed: {
                    int barrierIndex = _scene->_sceneResource->getBarrierIndexById(currentCommand.param1);
                    if (barrierIndex >= 0) {
                        int frameNum = 0;
                        if (currentCommand.param2 == -1)
                            frameNum = _scene->_sceneResource->getWorldStats()->barriers[barrierIndex].frameCount - 1;
                        else
                            frameNum = currentCommand.param2;
                        
                        if (_scene->_sceneResource->getWorldStats()->barriers[barrierIndex].tickCount < frameNum) {
                            lineIncrement = 0;
                            waitCycle = true;
                        }
                    }
                    else
                        debugC(kDebugLevelScripts, "Requested invalid object ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    }
                    break;
                case kPlaySpeech:
                    if (currentCommand.param1 >= 0) {
                        if (currentCommand.param3)  //  HACK - Find out why sometimes an offset is needed and other times not
                            _scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1 - 9);
                        else
                            _scene->_sound->playSfx(_scene->_speechPack, currentCommand.param1);      
                    }
                    else
                        debugC(kDebugLevelScripts, "Requested invalid sound ID:0x%02X in Scene %d Script %d Line %d.", currentCommand.param1, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    break;
                default:
                    debugC(kDebugLevelScripts, "Unhandled opcode 0x%02X in Scene %d Script %d Line %d.", _currentScript->commands[_currentLine].opcode, _scene->getSceneIndex(), _currentScriptIndex, _currentLine);
                    break;
            }   // end switch
            
            _currentLine += lineIncrement;
            
        }   // end while

        if(done) {
            _currentLine = 0;
            _currentScriptIndex = 0;
        }
    }
    _processing = false;
}

} // end of namespace Asylum
