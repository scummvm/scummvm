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

#ifndef NANCY_INPUT_H
#define NANCY_INPUT_H

#include "common/rect.h"
namespace Common {
template <class T>
class Array;

class Keymap;
typedef class Array<Keymap*> KeymapArray;
}

namespace Nancy {

class NancyEngine;

class InputManager {
enum NancyAction {
    kNancyActionMoveUp,
    kNancyActionMoveDown,
    kNancyActionMoveLeft,
    kNancyActionMoveRight,
    kNancyActionMoveFast,
    kNancyActionLeftClick,
    kNancyActionRightClick,
    kNancyActionFastConvoToggle,
    kNancyActionEndConvoToggle,
    kNancyActionReloadSave,
    kNancyActionRequestMainMenu,
    kNancyActionRequestSaveLoad,
    kNancyActionRequestSetupMenu,
    kNancyActionRequestCredits,
    kNancyActionRequestMap
};


public:
enum InputType : byte {
    kLeftMouseButton    = 1 << 0,
    kRightMouseButton   = 1 << 1,
    kMoveUp             = 1 << 2,
    kMoveDown           = 1 << 3,
    kMoveLeft           = 1 << 4,
    kMoveRight          = 1 << 5,
    kMoveFastModifier   = 1 << 6
};

    InputManager(NancyEngine *engine) : _engine(engine), _inputs(0) {}
    void processEvents();

    bool getInput(InputType type);

    static void initKeymaps(Common::KeymapArray &keymaps);

private:
    NancyEngine *_engine;

    byte _inputs;
    Common::Point _mousePos;
};

} // End of namespace Nancy

#endif // NANCY_INPUT_H