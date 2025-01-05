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

#include "got/views/dialogs/play_game.h"
#include "got/metaengine.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

void PlayGame::draw() {
    Dialog::draw();
#ifdef TODO
    // Write the title
    GfxSurface s = getSurface(true);
    int titleStart = (s.w - _title.size() * 8) / 2;

    s.print(Common::Point(titleStart, 4), _title, 54);
#endif
}

bool PlayGame::msgAction(const ActionMessage &msg) {
    // Don't allow further actions if selection is in progress

    switch (msg._action) {
    case KEYBIND_UP:
        play_sound(WOOP,1);
        break;

    case KEYBIND_DOWN:
        play_sound(WOOP, 1);
        break;

    case KEYBIND_SELECT:
    case KEYBIND_FIRE:
    case KEYBIND_MAGIC:
        break;

    case KEYBIND_ESCAPE:
        close();

    default:
        break;
    }

    return true;
}

bool PlayGame::tick() {
    return true;
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
