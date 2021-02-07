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

#include "engines/nancy/nancy.h"
#include "engines/nancy/scene.h"
#include "engines/nancy/logic.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/audio.h"

#include "common/file.h"
#include "common/random.h"

namespace Nancy {

// Simple helper function to read rectangles
static void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect) {
    inRect.left = stream.readUint32LE();
    inRect.top = stream.readUint32LE();
    inRect.right = stream.readUint32LE();
    inRect.bottom = stream.readUint32LE();
}

uint16 PlayPrimaryVideoChan0::readData(Common::SeekableReadStream &stream) {
    uint16 bytesRead = stream.pos();

    char name[10];
    stream.read(name, 10);
    videoName = Common::String(name);

    stream.skip(0x13);

    readRect(stream, src);
    readRect(stream, dest);

    char *rawText = new char[1500]();
    stream.read(rawText, 1500);
    assembleText(rawText, text, 1500);
    delete[] rawText;

    stream.read(name, 10);
    soundName = Common::String(name);
    soundChannelID = stream.readUint16LE();

    stream.skip(8);

    numRepeats = stream.readUint16LE();

    stream.skip(4);

    volume = stream.readUint16LE();

    stream.skip(0x29);
    conditionalResponseCharacterID = stream.readByte();
    goodbyeResponseCharacterID = stream.readByte();
    numSceneChanges = stream.readByte();
    shouldPopScene = stream.readByte() == 1;
    SceneChange::readData(stream);

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
            assembleText(rawText, response.text, 400);
            delete[] rawText;

            stream.read(name, 10);
            response.soundName = name;
            stream.skip(1);
            response.sceneChange.readData(stream);
            response.flagDesc.label = stream.readSint16LE();
            response.flagDesc.flag = (PlayState::Flag)stream.readByte();

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
            flagsStruct.label = stream.readSint16LE();
            flagsStruct.flag = (PlayState::Flag)stream.readByte();
        }
    }

    bytesRead = stream.pos() - bytesRead;
    return bytesRead;
}

void PlayPrimaryVideoChan0::execute(NancyEngine *engine) {
    ZRenderStruct &zr = engine->graphics->getZRenderStruct("PRIMARY VIDEO");
    AVFDecoder &decoder = engine->graphics->_primaryVideoDecoder;
    View &viewportDesc = engine->graphics->viewportDesc;
    switch (state) {
        case kBegin:
            zr.sourceRect = src;
            zr.destRect = dest;
            zr.destRect.left += viewportDesc.destination.left;
            zr.destRect.top += viewportDesc.destination.top;
            zr.destRect.right += viewportDesc.destination.left;
            zr.destRect.bottom += viewportDesc.destination.top;
            zr.isActive = true;
            if (decoder.isVideoLoaded()) {
                decoder.close();
            }
            decoder.loadFile(videoName + ".avf");
            engine->sound->loadSound(soundName, soundChannelID, numRepeats, volume);
            engine->sound->pauseSound(soundChannelID, false);
            state = kRun;
            // fall through
        case kRun:
            if (!hasDrawnTextbox) {
                hasDrawnTextbox = true;
                engine->graphics->_textbox.clear();
                engine->graphics->_textbox.processTextLine(text, 1);

                // Add responses when conditions have been satisfied
                if (conditionalResponseCharacterID != 10) {
                    addConditionalResponses(engine);
                }

                if (goodbyeResponseCharacterID != 10) {
                    addGoodbye(engine);
                }

                for (uint i = 0; i < responses.size(); ++i) {
                    auto &res = responses[i];
                    engine->graphics->_textbox.processResponse(res.text, 1, i, res.soundName);
                }

                ZRenderStruct &fr = engine->graphics->getZRenderStruct("FRAME TB SURF");
                fr.isActive = true;
                fr.sourceRect = Common::Rect(fr.destRect.width(), fr.destRect.height());
            }
            if (!engine->sound->isSoundPlaying(soundChannelID)) {
                if (responses.size() == 0) {
                    state = kActionTrigger;
                } else {
                    for (uint i = 0; i < 30; ++i) {
                        if (engine->playState.logicConditions[i] == PlayState::kTrue) {
                            pickedResponse = i;
                            break;
                        }
                    }

                    if (pickedResponse != -1) {
                        sceneChange = responses[pickedResponse].sceneChange;
                        engine->sound->loadSound(responses[pickedResponse].soundName, soundChannelID, numRepeats, volume);
                        engine->sound->pauseSound(soundChannelID, false);
                        state = kActionTrigger;
                    }
                }
            }
            break;
        case kActionTrigger:
            // process flags structs
            for (auto flags : flagsStructs) {
                bool conditionsSatisfied = true;
                for (auto cond : flags.conditionFlags) {
                    // TODO
                    error("Condition flags not evaluated");
                }

                if (conditionsSatisfied) {
                    switch (flags.type) {
                        case FlagsStruct::kEventFlags:
                            engine->playState.eventFlags[flags.label] = flags.flag;
                            break;
                        case FlagsStruct::kInventory:
                            if (flags.flag == PlayState::kTrue) {
                                engine->sceneManager->addObjectToInventory(flags.label);
                            } else {
                                engine->sceneManager->removeObjectFromInventory(flags.label);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }

            if (pickedResponse != -1) {
                int16 label = responses[pickedResponse].flagDesc.label;
                if (label != -1) {
                    engine->playState.eventFlags[label] = responses[pickedResponse].flagDesc.flag;
                }
            }

            if (!engine->sound->isSoundPlaying(soundChannelID)) {
                if (shouldPopScene) {
                    engine->sceneManager->popScene();
                } else {
                    SceneChange::execute(engine);
                }
            }

            // awful hack
            engine->logic->ignorePrimaryVideo = true;

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

                if (engine->playState.eventFlags[cond.label] != cond.flag) {
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
    for (auto res : nancy1Goodbyes) {
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

            file.close();
        }
    }
}

void PlayPrimaryVideoChan0::assembleText(char *rawCaption, Common::String &output, uint size) {
    for (uint i = 0; i < size; ++i) {
        // A single line can be broken up into bits, look for them and
        // concatenate them when we're done
        if (rawCaption[i] != 0) {
            Common::String newBit(rawCaption + i);
            output += newBit;
            i += newBit.size();
        }
    }
}

} // End of namespace Nancy