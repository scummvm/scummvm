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

PlayPrimaryVideoChan0::~PlayPrimaryVideoChan0() {
    _decoder.close();
	if (activePrimaryVideo == this) {
		activePrimaryVideo = nullptr;
	}
    _engine->scene->getTextbox().setVisible(false);
}

void PlayPrimaryVideoChan0::init() {
    _decoder.loadFile(videoName + ".avf");
    _drawSurface.create(src.width(), src.height(), _decoder.getPixelFormat());

    RenderObject::init();
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
}

uint16 PlayPrimaryVideoChan0::readData(Common::SeekableReadStream &stream) {
    uint16 bytesRead = stream.pos();

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
    numSceneChanges = stream.readByte();
    shouldPopScene = stream.readByte() == 1;
    sceneChange.readData(stream);

    stream.seek(bytesRead + 0x69C);

    uint16 numResponses = stream.readUint16LE();
    if (numResponses > 0) {
        for (uint i = 0; i < numResponses; ++i) {
            uint16 numConditionFlags = stream.readUint16LE();
            responses.push_back(ResponseStruct());
            ResponseStruct &response = responses[i];

            if (numConditionFlags > 0) {
                for (uint16 j = 0; j < numConditionFlags; ++j) {
                    response.conditionFlags.push_back(ConditionFlags());
                    ConditionFlags &flags = response.conditionFlags[j];
                    stream.read(flags.unknown, 5);
                }
            }
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
    if (numFlagsStructs > 0)  {
        for (uint16 i = 0; i < numFlagsStructs; ++i) {
            uint16 numConditionFlags = stream.readUint16LE();
            flagsStructs.push_back(FlagsStruct());
            FlagsStruct &flagsStruct = flagsStructs[flagsStructs.size()-1];

            if (numConditionFlags > 0) {
                // Not sure about this
                if (numConditionFlags > 0) {
                    for (uint16 j = 0; j < numConditionFlags; ++j) {
                        flagsStruct.conditionFlags.push_back(ConditionFlags());
                        ConditionFlags &flags = flagsStruct.conditionFlags[flagsStruct.conditionFlags.size()-1];
                        stream.read(flags.unknown, 5);
                    }
                }
            }

            flagsStruct.type = (FlagsStruct::ConditionType)stream.readByte();
            flagsStruct.flagDesc.label = stream.readSint16LE();
            flagsStruct.flagDesc.flag = (NancyFlag)stream.readByte();
        }
    }

    bytesRead = stream.pos() - bytesRead;
    return bytesRead;
}

void PlayPrimaryVideoChan0::execute(NancyEngine *engine) {
	if (activePrimaryVideo != this && activePrimaryVideo != nullptr) {
        return;
    }

    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            engine->sound->loadSound(sound);
            engine->sound->playSound(sound.channelID);
            state = kRun;
		    activePrimaryVideo = this;
            // fall through
        case kRun:
            if (!hasDrawnTextbox) {
                hasDrawnTextbox = true;
                engine->scene->getTextbox().clear();
                engine->scene->getTextbox().addTextLine(text);

                // Add responses when conditions have been satisfied
                if (conditionalResponseCharacterID != 10) {
                    addConditionalResponses(engine);
                }

                if (goodbyeResponseCharacterID != 10) {
                    addGoodbye(engine);
                }

                for (uint i = 0; i < responses.size(); ++i) {
                    auto &res = responses[i];
                    engine->scene->getTextbox().addTextLine(res.text);
                }
            }

            if (!engine->sound->isSoundPlaying(sound.channelID)) {
                engine->sound->stopSound(sound.channelID);
                if (responses.size() == 0) {
                    // NPC has finished talking with no responses available, auto-advance to next scene
                    state = kActionTrigger;
                } else {
                    // NPC has finished talking, we have responses
                    for (uint i = 0; i < 30; ++i) {
                        if (engine->scene->getLogicCondition(i, kTrue)) {
                            pickedResponse = i;
                            break;
                        }
                    }

                    if (pickedResponse != -1) {
                        // Player has picked response, play sound file and change state
                        sceneChange = responses[pickedResponse].sceneChange;
                        responseGenericSound.name = responses[pickedResponse].soundName;
                        // TODO this is probably not correct
                        engine->sound->loadSound(responseGenericSound);
                        engine->sound->playSound(responseGenericSound.channelID);
                        state = kActionTrigger;
                    }
                }
            }
            break;
        case kActionTrigger:
            // process flags structs
            for (auto flags : flagsStructs) {
                bool conditionsSatisfied = true;
                if (flags.conditionFlags.size()) {
                    error("Condition flags not evaluated, please fix");
                }

                if (conditionsSatisfied) {
                    switch (flags.type) {
                        case FlagsStruct::kEventFlags:
                            engine->scene->setEventFlag(flags.flagDesc);
                            break;
                        case FlagsStruct::kInventory:
                            if (flags.flagDesc.flag == kTrue) {
                                engine->scene->addItemToInventory(flags.flagDesc.label);
                            } else {
                                engine->scene->removeItemFromInventory(flags.flagDesc.label);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            
            if (pickedResponse != -1) {
                // Set response's event flag, if any
                engine->scene->setEventFlag(responses[pickedResponse].flagDesc);
            }

            if (!engine->sound->isSoundPlaying(responseGenericSound.channelID)) {
                engine->sound->stopSound(responseGenericSound.channelID);
                if (shouldPopScene) {
                    // Exit dialogue
                    engine->scene->popScene();
                } else {
                    // Continue to next dialogue scene
                    engine->scene->changeScene(sceneChange);
                }
                
                finishExecution();
            }

            break;
    }
}

void PlayPrimaryVideoChan0::addConditionalResponses(NancyEngine *engine) {
    for (auto &res : nancy1ConditionalResponses) {
        if (res.characterID == conditionalResponseCharacterID) {
            bool isSatisfied = true;
            for (auto & cond : res.conditions) {
                if (cond.label == -1) {
                    break;
                }

                if (!engine->scene->getEventFlag(cond.label, cond.flag)) {
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

void PlayPrimaryVideoChan0::addGoodbye(NancyEngine *engine) {
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
            newResponse.sceneChange.sceneID = res.sceneIDs[engine->_rnd->getRandomNumber(3)];
            newResponse.sceneChange.doNotStartSound = true;

            file.close();
        }
    }
}

} // End of namespace Action
} // End of namespace Nancy
