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

#ifndef NANCY_MENU_H
#define NANCY_MENU_H

namespace Graphics {
struct Surface;
}

namespace Nancy {

class NancyEngine;

class MainMenu {
public:
    MainMenu(NancyEngine *engine) :
        _engine(engine),
        _state(kInit),
        _surf(nullptr) {}

    void process();

private:
    void init();
    void startSound();
    void run();

    enum State {
        kInit,
        kStartSound,
        kRun
    };

    NancyEngine *_engine;
    State _state;
    Graphics::Surface *_surf;
    Graphics::Surface *_menuImage;
    Graphics::Surface *_cursor;
};
} // End of namespace Nancy

#endif // NANCY_MENU_H