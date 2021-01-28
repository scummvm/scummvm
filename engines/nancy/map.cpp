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
        _hotspots.push_back(Hotspot());
        Hotspot &h = _hotspots[i];
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
            h.scenes.push_back(Hotspot::SceneChange());
            Hotspot::SceneChange &sc = h.scenes[j];
            chunk->seek(0x1BE + 6 * i * (j + 1));
            sc.sceneID = chunk->readUint16LE();
            sc.frameID = chunk->readUint16LE();
            sc.verticalOffset = chunk->readUint16LE();
        }
    }

    ZRenderStruct &zr = _engine->graphics->getZRenderStruct("VIEWPORT AVF");
    zr.sourceSurface = &_mapImage;

    _engine->graphics->initMapRenderStructs(_ZRenderFilter);
    _state = kRun;
}

void Map::run() {
    handleMouse();

    if ((_engine->sceneManager->stateChangeRequests & SceneManager::kMap) == 0 &&
            !(_engine->input->isClickValidLMB && _engine->input->hoveredElementID == InputManager::mapButtonID)) {
        if (_engine->input->isClickValidLMB) {
            Hotspot::SceneChange &sc = _hotspots[_engine->input->hoveredElementID].scenes[_mapID];
            _engine->sceneManager->changeScene(sc.sceneID, sc.frameID, sc.verticalOffset, false);
            _state = kInit;
            _engine->_gameFlow.minGameState = NancyEngine::kScene;
            _engine->_gameFlow.previousGameState = NancyEngine::kMap;
        }
    } else {
        _state = kInit;
        _engine->_gameFlow.minGameState = NancyEngine::kScene;
        _engine->_gameFlow.previousGameState = NancyEngine::kMap;
        return;
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
        for (uint i = 0; i < _hotspots.size(); ++i) {
            // Adjust the hotspot coordinates
            Common::Rect hs = _hotspots[i].hotspot;
            hs.left += view.destination.left;
            hs.top += view.destination.top;
            hs.right += view.destination.left;
            hs.bottom += view.destination.top;

            if (_hotspots[i].isActive && hs.contains(viewportMouse)) {
                _engine->input->hoveredElementID = i;
                _engine->input->setPointerBitmap(-1, 1, 0);   
                break;             
            }
        }
    }
}

} // End of namespace Nancy