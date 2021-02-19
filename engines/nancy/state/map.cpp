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

#include "engines/nancy/state/map.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/resource.h"
#include "engines/nancy/audio.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"

#include "common/stream.h"
#include "common/str.h"

namespace Nancy {
namespace State {

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

    _viewport.init();
    _label.init();

    if (_engine->scene->getEventFlag(40, kTrue) &&
        _engine->scene->getEventFlag(95, kTrue)) {
        _mapID = 1;
    } else {
        _mapID = 0;
    }

    // Load the video
    chunk->seek(_mapID * 10, SEEK_SET);
    char name[10];
    chunk->read(name, 10);
    Common::String n(name);

    _viewport.loadVideo(n, 0, 0);
    _viewport.setEdgesSize(0, 0, 0, 0);

    // Load the audio
    chunk->seek(0x18 + _mapID * 0x20, SEEK_SET);
    chunk->read(name, 10);
    n = Common::String(name);
    uint16 channel = chunk->readUint16LE();
    chunk->skip(0xA);
    uint16 volume = chunk->readUint16LE();
    _engine->sound->loadSound(n, channel, 0, volume);
    _engine->sound->pauseSound(channel, false);

    for (uint i = 0; i < 4; ++i) {
        chunk->seek(0x162 + i * 16, SEEK_SET);
        _locations.push_back(Location());
        Location &loc = _locations[i];
        readRect(*chunk, loc.hotspot);

        // HARDCODED, TODO
        if (_mapID == 1 && (i % 2) != 0) {
            loc.isActive = false;
        } else {
            loc.isActive = true;
        }

        for (uint j = 0; j < 2; ++j) {
            loc.scenes.push_back(Location::SceneChange());
            Location::SceneChange &sc = loc.scenes[j];
            chunk->seek(0x1BE + 6 * i * (j + 1), SEEK_SET);
            sc.sceneID = chunk->readUint16LE();
            sc.frameID = chunk->readUint16LE();
            sc.verticalOffset = chunk->readUint16LE();
        }

        chunk->seek(0x9A + i * 16, SEEK_SET);
        readRect(*chunk, loc.labelSrc);

        // TODO this gets initialized using MAP and the textbox's on-screen location
        // but the code is annoyingly long so fpr now i just directly write the result
        loc.labelDest = Common::Rect(0x56, 0x166, 0x15E, 0x19B);
        
    }

    registerGraphics();
    _engine->cursorManager->setCursorItemID(-1);

    _state = kRun;
}

void Map::run() {
    NancyInput input = _engine->input->getInput();

    _label.setLabel(-1);

    for (uint i = 0; i < 4; ++i) {
        auto &loc = _locations[i];
        if (loc.isActive && _viewport.convertToScreen(loc.hotspot).contains(input.mousePos)) {
            _engine->cursorManager->setCursorType(CursorManager::kHotspotArrow);

            _label.setLabel(i);

            // TODO handle map button as well

            if (input.input & NancyInput::kLeftMouseButtonUp) {
                _engine->setGameState(NancyEngine::kScene);
                _engine->scene->changeScene(loc.scenes[_mapID].sceneID, loc.scenes[_mapID].frameID, loc.scenes[_mapID].verticalOffset, false);
                _state = kInit;
            }
            return;
        }
    }
}

void Map::registerGraphics() {
    _viewport.registerGraphics();
    _label.registerGraphics();
}

void Map::MapLabel::init() {
    setLabel(-1);
    
    RenderObject::init();
}

void Map::MapLabel::setLabel(int labelID) {
    if (labelID == -1) {
        setVisible(false);
    } else {
        _screenPosition = _parent->_locations[labelID].labelDest;
        _drawSurface.create(_engine->graphicsManager->object0, _parent->_locations[labelID].labelSrc);
        setVisible(true);
    }
}

} // End of namespace State
} // End of namespace Nancy
