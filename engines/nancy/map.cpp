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

#include "engines/nancy/map.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/audio.h"
#include "engines/nancy/scene.h"
#include "engines/nancy/input.h"

namespace Nancy {

void Map::process() {
    switch (_state) {
        case kInit:
            init();
            break;
        case kRun:
            run();
            break;
    }
}

void Map::init() {
    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("MAP");
    if (_decoder.isVideoLoaded()) {
        _decoder.close();
    }

    if (_engine->playState.eventFlags[40] == PlayState::kTrue &&
        _engine->playState.eventFlags[95] == PlayState::kTrue) {
        _mapID = 1;
    } else {
        _mapID = 0;
    }

    // Load the video
    chunk->seek(_mapID * 10, SEEK_SET);
    char name[10];
    chunk->read(name, 10);
    Common::String n(name);
    _decoder.loadFile(n + ".avf");
    _mapImage = *_decoder.decodeNextFrame();

    // Load the audio
    chunk->seek(0x18 + _mapID * 0x20, SEEK_SET);
    chunk->read(name, 10);
    n = Common::String(n);
    uint16 channel = chunk->readUint16LE();
    chunk->skip(0xA);
    uint16 volume = chunk->readUint16LE();
    _engine->sound->loadSound(n, channel, 0, volume);
    _engine->sound->pauseSound(channel, false);

    for (uint i = 0; i < 4; ++i) {
        chunk->seek(0x162 + i * 16, SEEK_SET);
        _locations.push_back(Location());
        Location &h = _locations[i];
        h.hotspot.left = chunk->readUint32LE();
        h.hotspot.top = chunk->readUint32LE();
        h.hotspot.right = chunk->readUint32LE();
        h.hotspot.bottom = chunk->readUint32LE();

        if (_mapID == 1 && (i % 2) != 0) {
            h.isActive = false;
        } else {
            h.isActive = true;
        }

        for (uint j = 0; j < 2; ++j) {
            h.scenes.push_back(Location::SceneChange());
            Location::SceneChange &sc = h.scenes[j];
            chunk->seek(0x1BE + 6 * i * (j + 1), SEEK_SET);
            sc.sceneID = chunk->readUint16LE();
            sc.frameID = chunk->readUint16LE();
            sc.verticalOffset = chunk->readUint16LE();
        }

        chunk->seek(0x9A + i * 16, SEEK_SET);
        h.labelSrc.left = chunk->readUint32LE();
        h.labelSrc.top = chunk->readUint32LE();
        h.labelSrc.right = chunk->readUint32LE();
        h.labelSrc.bottom = chunk->readUint32LE();

        // TODO this gets initialized using MAP and the textbox's on-screen location
        // but the code is annoyingly long so fpr now i just directly write the result
        h.labelDest = Common::Rect(0x56, 0x166, 0x15E, 0x19B);
        
    }

    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("VIEWPORT AVF");
    zr.sourceSurface = &_mapImage;

    _engine->graphics->initMapRenderStructs(_ZRenderFilter);
	_engine->_gameFlow.previousGameState = NancyEngine::kMap;
    _state = kRun;
}

void Map::run() {
    handleMouse();
    int16 hover = _engine->input->hoveredElementID;
    ZRenderStruct &labels = _engine->graphics->getZRenderStruct("MAP LABELS");
    if (hover != -1 && hover < 10000) {
        labels.isActive = true;
        labels.sourceRect = _locations[hover].labelSrc;
        labels.destRect = _locations[hover].labelDest;
    } else {
        labels.isActive = false;
    }

    if (_engine->input->isClickValidLMB()) {
        if (_engine->sceneManager->stateChangeRequests & SceneManager::kMap ||
                hover == InputManager::mapButtonID) {
            _state = kInit;
            _engine->_gameFlow.minGameState = NancyEngine::kScene;
            _engine->_gameFlow.previousGameState = NancyEngine::kMap;
            _engine->sceneManager->stateChangeRequests &= ~NancyEngine::kMap;
            return;
        }

        Location::SceneChange &sc = _locations[hover].scenes[_mapID];
        _engine->sceneManager->changeScene(sc.sceneID, sc.frameID, sc.verticalOffset, false);
        _state = kInit;
        _engine->_gameFlow.minGameState = NancyEngine::kScene;
        _engine->_gameFlow.previousGameState = NancyEngine::kMap;
    }

    _engine->graphics->renderDisplay(_ZRenderFilter);
}

void Map::handleMouse() {
    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("CUR IMAGE CURSOR");
    Common::Point mousePos = _engine->input->getMousePosition();
    zr.destRect.left = zr.destRect.right = mousePos.x;
    zr.destRect.top = zr.destRect.bottom = mousePos.y;
    _engine->input->hoveredElementID = -1;

    View &view = _engine->graphics->viewportDesc;

    // TODO incorrect magic number, figure out where this comes from
    Common::Point viewportMouse = mousePos + Common::Point(10, 10);

    if (view.destination.contains(viewportMouse)) {
        _engine->input->setPointerBitmap(0, 0, 0);
        for (uint i = 0; i < _locations.size(); ++i) {
            // Adjust the hotspot coordinates
            Common::Rect hs = _locations[i].hotspot;
            hs.left += view.destination.left;
            hs.top += view.destination.top;
            hs.right += view.destination.left;
            hs.bottom += view.destination.top;

            if (_locations[i].isActive && hs.contains(viewportMouse)) {
                _engine->input->hoveredElementID = i;
                _engine->input->setPointerBitmap(-1, 1, 0);   
                break;             
            }
        }
    } else {
        if (_engine->graphics->getZRenderStruct("MAP ANIM").destRect.contains(mousePos)) {
            _engine->input->hoveredElementID = InputManager::mapButtonID;
            _engine->input->setPointerBitmap(1, 2, -1);
        } else {
            _engine->input->setPointerBitmap(1, 1, 0);
        }
    }
}

} // End of namespace Nancy