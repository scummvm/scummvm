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
 */

#include "engines/nancy/action/primaryvideo.h"

#include "engines/nancy/action/responses.cpp"
#include "engines/nancy/action/actionmanager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "common/file.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

PlayPrimaryVideoChan0 *PlayPrimaryVideoChan0::activePrimaryVideo = nullptr;

void PlayPrimaryVideoChan0::ConditionFlag::read(Common::SeekableReadStream &stream) {
    type = (ConditionType)stream.readByte();
    flag.label = stream.readSint16LE();
    flag.flag = (NancyFlag)stream.readByte();
    orFlag = stream.readByte();
}

bool PlayPrimaryVideoChan0::ConditionFlag::isSatisfied() const {
    switch (type) {
    case ConditionFlag::kEventFlags:
        return NancySceneState.getEventFlag(flag);
    case ConditionFlag::kInventory:
        return NancySceneState.hasItem(flag.label) == flag.flag;
    default:
        return false;
    }
}

void PlayPrimaryVideoChan0::ConditionFlag::set() const {
    switch (type) {
    case ConditionFlag::kEventFlags:
        NancySceneState.setEventFlag(flag);
        break;
    case ConditionFlag::kInventory:
        if (flag.flag == kTrue) {
            NancySceneState.addItemToInventory(flag.label);
        } else {
            NancySceneState.removeItemFromInventory(flag.label);
        }

        break;
    default:
        break;
    }
}

void PlayPrimaryVideoChan0::ConditionFlags::read(Common::SeekableReadStream &stream) {
    uint16 numFlags = stream.readUint16LE();
    
    for (uint i = 0; i < numFlags; ++i) {
        conditionFlags.push_back(ConditionFlag());
        conditionFlags.back().read(stream);
    }
}

bool PlayPrimaryVideoChan0::ConditionFlags::isSatisfied() const {
    bool orFlag = false;

    for (uint i = 0; i < conditionFlags.size(); ++i) {
        const ConditionFlag &cur = conditionFlags[i];
        
        if (!cur.isSatisfied()) {
            if (orFlag) {
                return false;
            } else {
                orFlag = true;
            }
        }
    }

    if (orFlag) {
        return false;
    } else {
        return true;
    }
}

PlayPrimaryVideoChan0::~PlayPrimaryVideoChan0() {
    _decoder.close();

	if (activePrimaryVideo == this) {
		activePrimaryVideo = nullptr;
	}
    
    NancySceneState.setShouldClearTextbox(true);
    NancySceneState.getTextbox().setVisible(false);
}

void PlayPrimaryVideoChan0::init() {
    _decoder.loadFile(videoName + ".avf");
    _drawSurface.create(src.width(), src.height(), _decoder.getPixelFormat());

    RenderObject::init();
    
    NancySceneState.setShouldClearTextbox(false);
}

void PlayPrimaryVideoChan0::updateGraphics() {
    if (!_decoder.isVideoLoaded()) {
        return;
    }

    if (!_decoder.isPlaying()) {
        _decoder.start();
    }

    if (_decoder.needsUpdate()) {
        _drawSurface.blitFrom(*_decoder.decodeNextFrame(), src, Common::Point());
        _needsRedraw = true;
    }

    RenderObject::updateGraphics();
}

void PlayPrimaryVideoChan0::onPause(bool pause) {
    _decoder.pauseVideo(pause);

    if (pause) {
        registerGraphics();
    }
}

void PlayPrimaryVideoChan0::readData(Common::SeekableReadStream &stream) {
    uint16 beginOffset = stream.pos();

    char name[10];
    stream.read(name, 10);
    videoName = Common::String(name);

    stream.skip(0x13);

    readRect(stream, src);
    readRect(stream, _screenPosition);

    char *rawText = new char[1500]();
    stream.read(rawText, 1500);
    UI::Textbox::assembleTextLine(rawText, text, 1500);
    delete[] rawText;

    sound.read(stream, SoundDescription::kNormal);
    responseGenericSound.read(stream, SoundDescription::kNormal);
    stream.skip(1);
    conditionalResponseCharacterID = stream.readByte();
    goodbyeResponseCharacterID = stream.readByte();
    isDialogueExitScene = (NancyFlag)stream.readByte();
    doNotPop = (NancyFlag)stream.readByte();
    sceneChange.readData(stream);

    stream.seek(beginOffset + 0x69C);

    uint16 numResponses = stream.readUint16LE();
    if (numResponses > 0) {
        for (uint i = 0; i < numResponses; ++i) {
            responses.push_back(ResponseStruct());
            ResponseStruct &response = responses[i];
            response.conditionFlags.read(stream);
            rawText = new char[400];
            stream.read(rawText, 400);
            UI::Textbox::assembleTextLine(rawText, response.text, 400);
            delete[] rawText;

            stream.read(name, 10);
            response.soundName = name;
            stream.skip(1);
            response.sceneChange.readData(stream);
            response.flagDesc.label = stream.readSint16LE();
            response.flagDesc.flag = (NancyFlag)stream.readByte();

            stream.skip(0x32);
        }
    }

    uint16 numSceneBranchStructs = stream.readUint16LE();
    if (numSceneBranchStructs > 0) {
        // TODO
    }

    uint16 numFlagsStructs = stream.readUint16LE();
    if (numFlagsStructs > 0) {
        for (uint16 i = 0; i < numFlagsStructs; ++i) {
            flagsStructs.push_back(FlagsStruct());
            FlagsStruct &flagsStruct = flagsStructs.back();
            flagsStruct.conditions.read(stream);
            flagsStruct.flagToSet.type = (ConditionFlag::ConditionType)stream.readByte();
            flagsStruct.flagToSet.flag.label = stream.readSint16LE();
            flagsStruct.flagToSet.flag.flag = (NancyFlag)stream.readByte();
        }
    }
}

void PlayPrimaryVideoChan0::execute() {
	if (activePrimaryVideo != this && activePrimaryVideo != nullptr) {
        return;
    }

    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        g_nancy->sound->loadSound(sound);
        g_nancy->sound->playSound(sound);
        state = kRun;
        activePrimaryVideo = this;
        // fall through
    case kRun:
        if (!hasDrawnTextbox) {
            hasDrawnTextbox = true;
            NancySceneState.getTextbox().clear();
            NancySceneState.getTextbox().addTextLine(text);

            // Add responses when conditions have been satisfied
            if (conditionalResponseCharacterID != 10) {
                addConditionalResponses();
            }

            if (goodbyeResponseCharacterID != 10) {
                addGoodbye();
            }

            for (uint i = 0; i < responses.size(); ++i) {
                auto &res = responses[i];

                if (res.conditionFlags.isSatisfied()) {
                    NancySceneState.getTextbox().addTextLine(res.text);
                }
            }
        }

        if (!g_nancy->sound->isSoundPlaying(sound) && _decoder.endOfVideo()) {
            g_nancy->sound->stopSound(sound);
            
            if (responses.size() == 0) {
                // NPC has finished talking with no responses available, auto-advance to next scene
                state = kActionTrigger;
            } else {
                // NPC has finished talking, we have responses
                for (uint i = 0; i < 30; ++i) {
                    if (NancySceneState.getLogicCondition(i, kTrue)) {
                        pickedResponse = i;
                        break;
                    }
                }

                if (pickedResponse != -1) {
                    // Player has picked response, play sound file and change state
                    responseGenericSound.name = responses[pickedResponse].soundName;
                    // TODO this is probably not correct
                    g_nancy->sound->loadSound(responseGenericSound);
                    g_nancy->sound->playSound(responseGenericSound);
                    state = kActionTrigger;
                }
            }
        }
        break;
    case kActionTrigger:
        // process flags structs
        for (auto flags : flagsStructs) {
            if (flags.conditions.isSatisfied()) {
                flags.flagToSet.set();
            }
        }
        
        if (pickedResponse != -1) {
            // Set response's event flag, if any
            NancySceneState.setEventFlag(responses[pickedResponse].flagDesc);
        }

        if (!g_nancy->sound->isSoundPlaying(responseGenericSound)) {
            g_nancy->sound->stopSound(responseGenericSound);
            
            if (pickedResponse != -1) {
                NancySceneState.changeScene(responses[pickedResponse].sceneChange);
            } else {
                // Evaluate scene branch structs here

                if (isDialogueExitScene == kFalse) {
                    NancySceneState.changeScene(sceneChange);
                } else if (doNotPop == kFalse) {
                    // Exit dialogue
                    NancySceneState.popScene();
                }
            }
            
            finishExecution();
        }

        break;
    }
}

void PlayPrimaryVideoChan0::addConditionalResponses() {
    for (auto &res : nancy1ConditionalResponses) {
        if (res.characterID == conditionalResponseCharacterID) {
            bool isSatisfied = true;
            for (auto & cond : res.conditions) {
                if (cond.label == -1) {
                    break;
                }

                if (!NancySceneState.getEventFlag(cond.label, cond.flag)) {
                    isSatisfied = false;
                    break;
                }
            }

            if (isSatisfied) {
                Common::File file;
                char snd[10];

                file.open("game.exe");
                file.seek(nancy1ResponseBaseFileOffset + res.fileOffset);
                file.read(snd, 8);

                responses.push_back(ResponseStruct());
                ResponseStruct &newResponse = responses.back();
                newResponse.soundName = snd;
                newResponse.text = file.readString();
                newResponse.sceneChange.sceneID = res.sceneID;
                newResponse.sceneChange.doNotStartSound = true;

                file.close();
            }
        }
    }
}

void PlayPrimaryVideoChan0::addGoodbye() {
    for (auto &res : nancy1Goodbyes) {
        if (res.characterID == goodbyeResponseCharacterID) {
            Common::File file;
            char snd[10];

            file.open("game.exe");
            file.seek(nancy1ResponseBaseFileOffset + res.fileOffset);
            file.read(snd, 8);

            responses.push_back(ResponseStruct());
            ResponseStruct &newResponse = responses.back();
            newResponse.soundName = snd;
            newResponse.text = file.readString();
            // response is picked randomly
            newResponse.sceneChange.sceneID = res.sceneIDs[g_nancy->randomSource->getRandomNumber(3)];
            newResponse.sceneChange.doNotStartSound = true;

            file.close();
        }
    }
}

} // End of namespace Action
} // End of namespace Nancy
