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

void SceneChange::readData(Common::SeekableReadStream &stream) {
    sceneChange.readData(stream);
}

void SceneChange::execute() {
    NancySceneState.changeScene(sceneChange);
    isDone = true;
}

void HotMultiframeSceneChange::readData(Common::SeekableReadStream &stream) {
    SceneChange::readData(stream);
    uint16 numHotspots = stream.readUint16LE();

    for (uint i = 0; i < numHotspots; ++i) {
        hotspots.push_back(HotspotDescription());
        HotspotDescription &newDesc = hotspots[i];
        newDesc.readData(stream);
    }
}

void HotMultiframeSceneChange::execute() {
    switch (state) {
    case kBegin:
        // turn main rendering on
        state = kRun;
        // fall through
    case kRun:
        hasHotspot = false;
        for (uint i = 0; i < hotspots.size(); ++i) {
            if (hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
                hasHotspot = true;
                hotspot = hotspots[i].coords;
            }
        }
        break;
    case kActionTrigger:
        SceneChange::execute();
        break;
    }
}

void Hot1FrSceneChange::readData(Common::SeekableReadStream &stream) {
    SceneChange::readData(stream);
    hotspotDesc.readData(stream);
}

void Hot1FrSceneChange::execute() {
    switch (state) {
    case kBegin:
        hotspot = hotspotDesc.coords;
        state = kRun;
        // fall through
    case kRun:
        if (hotspotDesc.frameID == NancySceneState.getSceneInfo().frameID) {
            hasHotspot = true;
        } else {
            hasHotspot = false;
        }
        break;
    case kActionTrigger:
        SceneChange::execute();
        break;
    }
}

void HotMultiframeMultisceneChange::readData(Common::SeekableReadStream &stream) {
    stream.seek(0x14, SEEK_CUR);
    uint size = stream.readUint16LE() * 0x12;
    stream.skip(size);
}

void StartFrameNextScene::readData(Common::SeekableReadStream &stream) {
    stream.skip(4);
}

void StartStopPlayerScrolling::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void MapCall::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void MapCall::execute() {
    execType = kRepeating;
    NancySceneState.requestStateChange(NancyEngine::kMap);
    finishExecution();
}

void MapCallHot1Fr::readData(Common::SeekableReadStream &stream) {
    stream.skip(0x12);
}

void MapCallHot1Fr::execute() {
    switch (state) {
    case kBegin:
        hotspot = hotspotDesc.coords;
        state = kRun;
        // fall through
    case kRun:
        if (hotspotDesc.frameID == NancySceneState.getSceneInfo().frameID) {
            hasHotspot = true;
        }
        break;
    case kActionTrigger:
        MapCall::execute();
        break;
    }
}

void MapCallHotMultiframe::readData(Common::SeekableReadStream &stream) {
    uint16 numDescs = stream.readUint16LE();
    for (uint i = 0; i < numDescs; ++i) {
        hotspots.push_back(HotspotDescription());
        hotspots[i].readData(stream);
    }
}

void MapCallHotMultiframe::execute() {
    switch (state) {
    case kBegin:
        state = kRun;
        // fall through
    case kRun:
        hasHotspot = false;
        for (uint i = 0; i < hotspots.size(); ++i) {
            if (hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
                hasHotspot = true;
                hotspot = hotspots[i].coords;
            }
        }
        break;
    case kActionTrigger:
        MapCall::execute();
        break;  
    }
}

void MapLocationAccess::readData(Common::SeekableReadStream &stream) {
    stream.skip(4);
}

void MapSound::readData(Common::SeekableReadStream &stream) {
    stream.skip(0x10);
}

void MapAviOverride::readData(Common::SeekableReadStream &stream) {
    stream.skip(2);
}

void MapAviOverrideOff::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void TextBoxWrite::readData(Common::SeekableReadStream &stream) {
    uint16 size = stream.readUint16LE();
    stream.skip(size);

    if (size > 10000) {
        error("Action Record atTextboxWrite has too many text box chars: %d", size);;
    }
}

void TextBoxClear::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void BumpPlayerClock::readData(Common::SeekableReadStream &stream) {
    stream.skip(5);
}

void SaveContinueGame::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void TurnOffMainRendering::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void TurnOnMainRendering::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void ResetAndStartTimer::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void ResetAndStartTimer::execute() {
    NancySceneState.resetAndStartTimer();
    isDone = true;
}

void StopTimer::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void StopTimer::execute() {
    NancySceneState.stopTimer();
    isDone = true;
}

void EventFlags::readData(Common::SeekableReadStream &stream) {
    flags.readData(stream);
}

void EventFlags::execute() {
    flags.execute();
    isDone = true;
}

void EventFlagsMultiHS::readData(Common::SeekableReadStream &stream) {
    EventFlags::readData(stream);
    uint16 numHotspots = stream.readUint16LE();

    for (uint16 i = 0; i < numHotspots; ++i) {
        hotspots.push_back(HotspotDescription());
        HotspotDescription &newDesc = hotspots[i];
        newDesc.readData(stream);
    }
}

void EventFlagsMultiHS::execute() {
    switch (state) {
    case kBegin:
        // turn main rendering on
        state = kRun;
        // fall through
    case kRun:
        hasHotspot = false;

        for (uint i = 0; i < hotspots.size(); ++i) {
            if (hotspots[i].frameID == NancySceneState.getSceneInfo().frameID) {
                hasHotspot = true;
                hotspot = hotspots[i].coords;
            }
        }

        break;
    case kActionTrigger:
        hasHotspot = false;
        EventFlags::execute();
        finishExecution();
        break;
    }
}

void LoseGame::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void LoseGame::execute() {
    NanEngine.sound->stopAndUnloadSpecificSounds();
    NanEngine.setState(NancyEngine::kMainMenu);
    NancySceneState.resetStateToInit();
    isDone = true;
}

void PushScene::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void PopScene::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void WinGame::readData(Common::SeekableReadStream &stream) {
    stream.skip(1);
}

void WinGame::execute() {
    NanEngine.sound->stopAndUnloadSpecificSounds();
    NanEngine.setState(NancyEngine::kCredits, NancyEngine::kMainMenu);
    
    // TODO replace with destroy()?
    NancySceneState.resetStateToInit();
    isDone = true;
}

void AddInventoryNoHS::readData(Common::SeekableReadStream &stream) {
    itemID = stream.readUint16LE();
}

void AddInventoryNoHS::execute() {
    if (NancySceneState.hasItem(itemID) == kFalse) {
        NancySceneState.addItemToInventory(itemID);
    }

    isDone = true;
}

void RemoveInventoryNoHS::readData(Common::SeekableReadStream &stream) {
    stream.skip(2);
}

void DifficultyLevel::readData(Common::SeekableReadStream &stream) {
    difficulty = stream.readUint16LE();
    flag.label = stream.readSint16LE();
    flag.flag = (NancyFlag)stream.readUint16LE();
}

void DifficultyLevel::execute() {
    NancySceneState.setDifficulty(difficulty);
    NancySceneState.setEventFlag(flag);
    isDone = true;
}

void ShowInventoryItem::init() {
    NanEngine.resource->loadImage(imageName, _fullSurface);

    _drawSurface.create(_fullSurface, bitmaps[0].src);

    RenderObject::init();
}

void ShowInventoryItem::readData(Common::SeekableReadStream &stream) {
    objectID = stream.readUint16LE();
    char name[10];
    stream.read(name, 10);
    imageName = Common::String(name);

    uint16 numFrames = stream.readUint16LE();

    for (uint i = 0; i < numFrames; ++i) {
        bitmaps.push_back(BitmapDescription());
        bitmaps[i].readData(stream);
    }
}

void ShowInventoryItem::execute() {
    switch (state) {
    case kBegin:
        init();
        registerGraphics();
        state = kRun;
        // fall through
    case kRun: {
        int newFrame = -1;

        for (uint i = 0; i < bitmaps.size(); ++i) {
            if (bitmaps[i].frameID == NancySceneState.getSceneInfo().frameID) {
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
        NanEngine.sound->playSound(24); // Hardcoded by original engine
        NancySceneState.addItemToInventory(objectID);
        setVisible(false);
        hasHotspot = false;
        finishExecution();
        break;
    }
}

void ShowInventoryItem::onPause(bool pause) {
    if (pause) {
        registerGraphics();
    }
}

void PlayDigiSoundAndDie::readData(Common::SeekableReadStream &stream) {
    sound.read(stream, SoundDescription::kDIGI);
    sceneChange.readData(stream);
    flagOnTrigger.label = stream.readSint16LE();
    flagOnTrigger.flag = (NancyFlag)stream.readByte();
    stream.skip(2);
}

void PlayDigiSoundAndDie::execute() {
    switch (state) {
    case kBegin:
        NanEngine.sound->loadSound(sound);
        NanEngine.sound->playSound(sound);
        state = kRun;
        break;
    case kRun:
        if (!NanEngine.sound->isSoundPlaying(sound)) {
            state = kActionTrigger;
        }

        break;
    case kActionTrigger:
        if (sceneChange.sceneID != 9999) {
            NancySceneState.changeScene(sceneChange);
        }
        
        NancySceneState.setEventFlag(flagOnTrigger);
        NanEngine.sound->stopSound(sound);

        finishExecution();
        break;
    }
}

void PlaySoundPanFrameAnchorAndDie::readData(Common::SeekableReadStream &stream) {
    stream.skip(0x20);
}

void PlaySoundMultiHS::readData(Common::SeekableReadStream &stream) {
    sound.read(stream, SoundDescription::kNormal);
    sceneChange.readData(stream);
    flag.label = stream.readSint16LE();
    flag.flag = (NancyFlag)stream.readByte();
    stream.skip(2);
    uint16 numHotspots = stream.readUint16LE();

    for (uint i = 0; i < numHotspots; ++i) {
        hotspots.push_back(HotspotDescription());
        hotspots.back().frameID = stream.readUint16LE();
        readRect(stream, hotspots.back().coords);
    }
}

void PlaySoundMultiHS::execute() {
    switch (state) {
    case kBegin:
        state = kRun;
        // fall through
    case kRun: {
        hasHotspot = false;
        uint currentFrame = NancySceneState.getSceneInfo().frameID;

        for (uint i = 0; i < hotspots.size(); ++i) {
            if (hotspots[i].frameID == currentFrame) {
                hotspot = hotspots[i].coords;
                hasHotspot = true;
                break;
            }
        }

        break;
    }
    case kActionTrigger:
        NanEngine.sound->loadSound(sound);
        NanEngine.sound->playSound(sound);
        NancySceneState.changeScene(sceneChange);
        NancySceneState.setEventFlag(flag);
        finishExecution();
        break;
    }
}

void HintSystem::readData(Common::SeekableReadStream &stream) {
    characterID = stream.readByte();
    genericSound.read(stream, SoundDescription::kNormal);
}

void HintSystem::execute() {
    switch (state) {
    case kBegin:
        if (NancySceneState.getHintsRemaining() > 0) {
            selectHint();
        } else {
            getHint(0, NancySceneState.getDifficulty());
        }

        NancySceneState.getTextbox().clear();
        NancySceneState.getTextbox().addTextLine(text);

        NanEngine.sound->loadSound(genericSound);
        NanEngine.sound->playSound(genericSound);
        state = kRun;
        break;
    case kRun:
        if (!NanEngine.sound->isSoundPlaying(genericSound)) {
            NanEngine.sound->stopSound(genericSound);
            state = kActionTrigger;
        } else {
            break;
        }

        // fall through
    case kActionTrigger:
        NancySceneState.useHint(hintID, hintWeight);
        NancySceneState.getTextbox().clear();

        NancySceneState.changeScene(sceneChange);

        isDone = true;
        break;
    }
}

void HintSystem::selectHint() {
    for (auto &hint : nancy1Hints) {
        if (hint.characterID != characterID) {
            continue;
        }

        bool satisfied = true;

        for (auto &flag : hint.flagConditions) {
            if (flag.label == -1) {
                break;
            }
            
            if (!NancySceneState.getEventFlag(flag.label, flag.flag)) {
                satisfied = false;
                break;
            }
        }

        for (auto &inv : hint.inventoryCondition) {
            if (inv.label == -1) {
                break;
            }

            if (NancySceneState.hasItem(inv.label) != inv.flag) {
                satisfied = false;
                break;
            }
        }

        if (satisfied) {
            getHint(hint.hintID, NancySceneState.getDifficulty());
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
