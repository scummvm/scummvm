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

#ifndef NANCY_MAP_H
#define NANCY_MAP_H

#include "engines/nancy/video.h"

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

#include "graphics/surface.h"

namespace Nancy {

class NancyEngine;

class Map {
public:
    enum State { kInit, kRun };
    Map(NancyEngine *engine) : _engine(engine), _state(kInit), _mapID(0) {}

    void process();

private:
    struct Location {
        struct SceneChange {
            uint16 sceneID = 0;
            uint16 frameID = 0;
            uint16 verticalOffset = 0;
        };

        bool isActive = false;
        Common::Rect hotspot;
        Common::Array<SceneChange> scenes;

        Common::Rect labelSrc;
        Common::Rect labelDest;
    };

    void init();
    void run();

    void handleMouse();

    NancyEngine *_engine;
    State _state;
    uint16 _mapID;
    AVFDecoder _decoder;
    Graphics::Surface _mapImage;
    Common::Array<Location> _locations;

    Common::Array<Common::String> _ZRenderFilter;
};

} // End of namespace Nancy

#endif