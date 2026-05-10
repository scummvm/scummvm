/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */
 
#include "graphics/cursorman.h"
#include "common/system.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/enums.h"
#include "avalanche/outro.h"

namespace Avalanche {

    Outro::Outro(AvalancheEngine *vm) : _vm(vm) {
    }

    void Outro::run() {
        CursorMan.showMouse(false);
        _vm->_graphics->blackOutScreen();
        _vm->_graphics->drawQuittingPic();
        _vm->_graphics->refreshScreen();

        // Wait 5 seconds or keypress/click
        uint32 startTime = g_system->getMillis();
        Common::Event event;
        while (!_vm->shouldQuit()) {
            if (g_system->getMillis() - startTime >= 5000)
                return;
            while (_vm->getEvent(event)) {
                switch (event.type) {
                case Common::EVENT_KEYDOWN:
                case Common::EVENT_LBUTTONDOWN:
                    CursorMan.showMouse(true);
                    return;
                default:
                    break;
                }
            }
            g_system->delayMillis(10);
        }

        CursorMan.showMouse(true);
    }
}
