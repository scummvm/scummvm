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

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/action/actionmanager.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/responses.cpp"

#include "common/str.h"

namespace Nancy {
namespace Action {

uint16 SceneChange::readData(Common::SeekableReadStream &stream) {
    sceneChange.readData(stream);
    return 8;
}

void SceneChange::execute(NancyEngine *engine) {
    engine->scene->changeScene(sceneChange);
    isDone = true;
}

uint16 HotMultiframeSceneChange::readData(Common::SeekableReadStream &stream) {
    uint16 ret = SceneChange::readData(stream);
    uint16 numHotspots = stream.readUint16LE();

    for (uint i = 0; i < numHotspots; ++i) {
        hotspots.push_back(HotspotDescription());
        HotspotDescription &newDesc = hotspots[i];
        newDesc.readData(stream);
    }

    return ret + (numHotspots * 0x12) + 2;
}

void HotMultiframeSceneChange::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            // turn main rendering on
            state = kRun;
            // fall through
        case kRun:
            hasHotspot = false;
            for (uint i = 0; i < hotspots.size(); ++i) {
                if (hotspots[i].frameID == engine->scene->getSceneInfo().frameID) {
                    hasHotspot = true;
                    hotspot = hotspots[i].coords;
                }
            }
            break;
        case kActionTrigger:
            SceneChange::execute(engine);
            break;
    }
}

uint16 Hot1FrSceneChange::readData(Common::SeekableReadStream &stream) {
    SceneChange::readData(stream);
    hotspotDesc.readData(stream);
    return 0x1A;
}

void Hot1FrSceneChange::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            hotspot = hotspotDesc.coords;
            state = kRun;
            // fall through
        case kRun:
            if (hotspotDesc.frameID == engine->scene->getSceneInfo().frameID) {
                hasHotspot = true;
            } else {
                hasHotspot = false;
            }
            break;
        case kActionTrigger:
            SceneChange::execute(engine);
            break;
    }
}

uint16 HotMultiframeMultisceneChange::readData(Common::SeekableReadStream &stream) {
    stream.seek(0x14, SEEK_CUR);
    uint size = stream.readUint16LE() * 0x12 + 0x16;
    stream.seek(-0x16, SEEK_CUR);

    return readRaw(stream, size); // TODO
}

uint16 StartFrameNextScene::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x4); // TODO
}

uint16 StartStopPlayerScrolling::readData(Common::SeekableReadStream &stream) {
    type = stream.readByte();    
    return 1;
}

uint16 PlayStaticBitmapAnimation::readData(Common::SeekableReadStream &stream) {
    // TODO
    uint16 bytesRead = stream.pos();
    byte *seek = bitmapData;
    
    uint16 currentSize = 0x72;
    stream.read(seek, currentSize);

    seek += currentSize;
    currentSize = (uint16)(bitmapData[0x18]) - (uint16)(bitmapData[0x16]);
    ++currentSize;
    currentSize *= 16;
    stream.read(seek, currentSize);

    seek += 0x252;
    currentSize = (uint16)(bitmapData[0x70]);
    currentSize *= 34;
    stream.read(seek, currentSize);

    bytesRead= stream.pos() - bytesRead;
    return bytesRead;
}

uint16 MapCall::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
    return 1;
}

void MapCall::execute(NancyEngine *engine) {
    execType = kRepeating;
    engine->scene->requestStateChange(NancyEngine::kMap);
    finishExecution();
}

uint16 MapCallHot1Fr::readData(Common::SeekableReadStream &stream) {
    hotspotDesc.readData(stream);
    return 0x12;
}

void MapCallHot1Fr::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            hotspot = hotspotDesc.coords;
            state = kRun;
            // fall through
        case kRun:
            if (hotspotDesc.frameID == engine->scene->getSceneInfo().frameID) {
                hasHotspot = true;
            }
            break;
        case kActionTrigger:
            MapCall::execute(engine);
            break;
    }
}

uint16 MapCallHotMultiframe::readData(Common::SeekableReadStream &stream) {
    uint16 numDescs = stream.readUint16LE();
    for (uint i = 0; i < numDescs; ++i) {
        hotspots.push_back(HotspotDescription());
        hotspots[i].readData(stream);
    }

    return 2 + numDescs * 0x12;
}

void MapCallHotMultiframe::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            state = kRun;
            // fall through
        case kRun:
            hasHotspot = false;
            for (uint i = 0; i < hotspots.size(); ++i) {
                if (hotspots[i].frameID == engine->scene->getSceneInfo().frameID) {
                    hasHotspot = true;
                    hotspot = hotspots[i].coords;
                }
            }
            break;
        case kActionTrigger:
            MapCall::execute(engine);
            break;  
    }
}

uint16 MapLocationAccess::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x4); // TODO
}

uint16 MapSound::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x10); // TODO
}

uint16 MapAviOverride::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x2); // TODO
}

uint16 MapAviOverrideOff::readData(Common::SeekableReadStream &stream) {
    overrideOffData = stream.readByte();
    return 1;
}

uint16 TextBoxWrite::readData(Common::SeekableReadStream &stream) {
    uint16 size = stream.readUint16LE();
    stream.seek(-2, SEEK_CUR);

    if (size > 0x2710) {
        error("Action Record atTextboxWrite has too many text box chars: %d", size);;
    }

    return readRaw(stream, size+2); // TODO
}

uint16 TextBoxClear::readData(Common::SeekableReadStream &stream) {
    clearData = stream.readByte();
    return 1;
}

uint16 BumpPlayerClock::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x5); // TODO
}

uint16 SaveContinueGame::readData(Common::SeekableReadStream &stream) {
    saveContinueData = stream.readByte();
    return 1;
}

uint16 TurnOffMainRendering::readData(Common::SeekableReadStream &stream) {
    turnOffData = stream.readByte();
    return 1;
}

uint16 TurnOnMainRendering::readData(Common::SeekableReadStream &stream) {
    turnOnData = stream.readByte();
    return 1;
}

uint16 ResetAndStartTimer::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
    return 1;
}

void ResetAndStartTimer::execute(NancyEngine *engine) {
    engine->scene->resetAndStartTimer();
    isDone = true;
}

uint16 StopTimer::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
    return 1;
}

void StopTimer::execute(NancyEngine *engine) {
    engine->scene->stopTimer();
    isDone = true;
}

uint16 EventFlags::readData(Common::SeekableReadStream &stream) {
    flags.readData(stream);
    return 0x28;
}

void EventFlags::execute(NancyEngine *engine) {
    flags.execute(engine);
    isDone = true;
}

uint16 EventFlagsMultiHS::readData(Common::SeekableReadStream &stream) {
    uint16 returnSize = EventFlags::readData(stream);
    uint16 numHotspots = stream.readUint16LE();
    for (uint16 i = 0; i < numHotspots; ++i) {
        hotspots.push_back(HotspotDescription());
        HotspotDescription &newDesc = hotspots[i];
        newDesc.readData(stream);
    }
    returnSize += numHotspots * 0x12 + 0x2;

    return returnSize;
}

void EventFlagsMultiHS::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            // turn main rendering on
            state = kRun;
            // fall through
        case kRun:
            hasHotspot = false;
            for (uint i = 0; i < hotspots.size(); ++i) {
                if (hotspots[i].frameID == engine->scene->getSceneInfo().frameID) {
                    hasHotspot = true;
                    hotspot = hotspots[i].coords;
                }
            }
            break;
        case kActionTrigger:
            hasHotspot = false;
            EventFlags::execute(engine);
            finishExecution();
            break;
    }
}

uint16 LoseGame::readData(Common::SeekableReadStream &stream) {
    loseData = stream.readByte();
    return 1;
}

uint16 PushScene::readData(Common::SeekableReadStream &stream) {
    pushData = stream.readByte();
    return 1;
}

uint16 PopScene::readData(Common::SeekableReadStream &stream) {
    popData = stream.readByte();
    return 1;
}

uint16 WinGame::readData(Common::SeekableReadStream &stream) {
    winData = stream.readByte();
    return 1;
}

uint16 AddInventoryNoHS::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x2); // TODO
}

uint16 RemoveInventoryNoHS::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x2); // TODO
}

uint16 DifficultyLevel::readData(Common::SeekableReadStream &stream) {
    difficulty = stream.readUint16LE();
    flag.label = stream.readSint16LE();
    flag.flag = (NancyFlag)stream.readUint16LE();
    return 6;
}

void DifficultyLevel::execute(NancyEngine *engine) {
    engine->scene->setDifficulty(difficulty);
    engine->scene->setEventFlag(flag);
    isDone = true;
}

void ShowInventoryItem::init() {
    Graphics::Surface srcSurf;
    _engine->_res->loadImage("ciftree", imageName, srcSurf);
    _fullSurface.create(srcSurf.w, srcSurf.h, srcSurf.format);
    _fullSurface.blitFrom(srcSurf);
    srcSurf.free();

    _drawSurface.create(_fullSurface, bitmaps[0].src);

    RenderObject::init();
}

uint16 ShowInventoryItem::readData(Common::SeekableReadStream &stream) {
    objectID = stream.readUint16LE();
    char name[10];
    stream.read(name, 10);
    imageName = Common::String(name);

    uint16 numFrames = stream.readUint16LE();

    for (uint i = 0; i < numFrames; ++i) {
        bitmaps.push_back(BitmapDescription());
        bitmaps[i].readData(stream);
    }

    return 0xE + 0x22 * numFrames;
}

void ShowInventoryItem::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            init();
            registerGraphics();
            state = kRun;
            // fall through
        case kRun: {
            int newFrame = -1;

            for (uint i = 0; i < bitmaps.size(); ++i) {
                if (bitmaps[i].frameID == engine->scene->getSceneInfo().frameID) {
                    newFrame = i;
                    break;
                }
            }

            if (newFrame != drawnFrameID) {
                drawnFrameID = newFrame;
                if (newFrame != -1) {
                    hasHotspot = true;
                    hotspot = bitmaps[newFrame].dest;
                    _drawSurface.create(_fullSurface, bitmaps[newFrame].src);
                    _screenPosition = bitmaps[newFrame].dest;
                    setVisible(true);
                } else {
                    hasHotspot = false;
                    setVisible(false);
                }
            }
                   
            break;
        }
        case kActionTrigger:
            engine->sound->playSound(24); // Hardcoded by original engine
            engine->scene->addItemToInventory(objectID);
            setVisible(false);
            hasHotspot = false;
            finishExecution();
            break;
    }
}

uint16 PlayDigiSoundAndDie::readData(Common::SeekableReadStream &stream) {
    sound.read(stream, SoundDescription::kDIGI);
    sceneChange.readData(stream);
    flagOnTrigger.label = stream.readSint16LE();
    flagOnTrigger.flag = (NancyFlag)stream.readByte();
    stream.skip(1);
    return 0x2B;
}

void PlayDigiSoundAndDie::execute(NancyEngine *engine) {
    switch (state) {
        case kBegin:
            engine->sound->loadSound(sound);
            engine->sound->playSound(sound.channelID);
            state = kRun;
            break;
        case kRun:
            if (!engine->sound->isSoundPlaying(sound.channelID)) {
                state = kActionTrigger;
            }
            break;
        case kActionTrigger:
            if (sceneChange.sceneID != 9999) {
                engine->scene->changeScene(sceneChange);
            }
            
            engine->scene->setEventFlag(flagOnTrigger);
            engine->sound->stopSound(sound.channelID);

            finishExecution();
            break;
    }
}

uint16 PlaySoundPanFrameAnchorAndDie::readData(Common::SeekableReadStream &stream) {
    return readRaw(stream, 0x20); // TODO
}

uint16 PlaySoundMultiHS::readData(Common::SeekableReadStream &stream) {
    stream.seek(0x2F, SEEK_CUR);
    uint16 size = stream.readUint16LE() * 0x12 + 0x31;
    stream.seek(-0x31, SEEK_CUR);

    return readRaw(stream, size); // TODO
}

uint16 HintSystem::readData(Common::SeekableReadStream &stream) {
    characterID = stream.readByte();
    genericSound.read(stream, SoundDescription::kNormal);
    return 0x23;
}

void HintSystem::execute(Nancy::NancyEngine *engine) {
    switch (state) {
        case kBegin:
            if (engine->scene->getHintsRemaining() > 0) {
                selectHint(engine);
            } else {
                getHint(0, engine->scene->getDifficulty());
            }

            engine->scene->getTextbox().clear();
            engine->scene->getTextbox().addTextLine(text);

            engine->sound->loadSound(genericSound);
            engine->sound->playSound(genericSound.channelID);
            state = kRun;
            break;
        case kRun:
            if (!engine->sound->isSoundPlaying(genericSound.channelID)) {
                engine->sound->stopSound(genericSound.channelID);
                state = kActionTrigger;
            } else {
                break;
            }
            // fall through
        case kActionTrigger:
            engine->scene->useHint(hintID, hintWeight);
            engine->scene->getTextbox().clear();

            engine->scene->changeScene(sceneChange);

            isDone = true;
            break;
    }
}

void HintSystem::selectHint(Nancy::NancyEngine *engine) {
    for (auto &hint : nancy1Hints) {
        if (hint.characterID != characterID) {
            continue;
        }

        bool satisfied = true;

        for (auto &flag : hint.flagConditions) {
            if (flag.label == -1) {
                break;
            }
            if (!engine->scene->getEventFlag(flag.label, flag.flag)) {
                satisfied = false;
                break;
            }
        }

        for (auto &inv : hint.inventoryCondition) {
            if (inv.label == -1) {
                break;
            }
            if (engine->scene->hasItem(inv.label) != inv.flag) {
                satisfied = false;
                break;
            }
        }

        if (satisfied) {
            getHint(hint.hintID, engine->scene->getDifficulty());
            break;
        }
    }
}

void HintSystem::getHint(uint hint, uint difficulty) {
    uint fileOffset;
    if (characterID < 3) {
        fileOffset = nancy1HintOffsets[characterID];
    }

    fileOffset += 0x288 * hint;

    Common::File file;
    file.open("game.exe");
    file.seek(fileOffset);

    hintID = file.readSint16LE();
    hintWeight = file.readSint16LE();

    file.seek(difficulty * 10, SEEK_CUR);

    char soundName[10];
    file.read(soundName, 10);
    genericSound.name = soundName;

    file.seek(-(difficulty * 10) - 10, SEEK_CUR);
    file.seek(30 + difficulty * 200, SEEK_CUR);

    char textBuf[200];
    file.read(textBuf, 200);
    text = textBuf;

    file.seek(-(difficulty * 200) - 200, SEEK_CUR);
    file.seek(600, SEEK_CUR);

    sceneChange.readData(file);
}

}
} // End of namespace Nancy
