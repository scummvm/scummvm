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

#include "engines/nancy/datatypes.h"

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
enum InputType : uint16 {
    kLeftMouseButtonDown    = 1 << 0,
    kRightMouseButtonDown   = 1 << 1,
    kLeftMouseButtonUp    = 1 << 2,
    kRightMouseButtonUp   = 1 << 3,
    kMoveUp             = 1 << 4,
    kMoveDown           = 1 << 5,
    kMoveLeft           = 1 << 6,
    kMoveRight          = 1 << 7,
    kMoveFastModifier   = 1 << 8
};

    InputManager(NancyEngine *engine) : _engine(engine), _inputs(0), hoveredElementID(-1) {}
    void processEvents();

    bool getInput(InputType type);
    byte getInput() { return _inputs; }
    void clearInput();
    bool isClickValidLMB() { return hoveredElementID != -1 && _inputs & kLeftMouseButtonUp; }
    bool isClickValidRMB() { return hoveredElementID != -1 && _inputs & kRightMouseButtonUp; }

    Common::Point getMousePosition();
    void setMousePosition(const Common::Point &newPos);
    void setPointerBitmap(int16 id, int16 style, int16 itemHeld = 0);

    static void initKeymaps(Common::KeymapArray &keymaps);

    int16 hoveredElementID;
    Cursors cursorsData;
    
    // TODO consider using a namespace for these
    static const int16 mapButtonID;
    static const int16 textBoxID;
    static const int16 textBoxScrollbarID;
    static const int16 helpButtonID;
    static const int16 menuButtonID;
    static const int16 inventoryScrollbarID;
    static const int16 inventoryItemTakeID;
    static const int16 inventoryItemReturnID;

    static const int16 orderingPuzzleID;
    static const int16 orderingPuzzleEndID;
    static const int16 rotatingLockPuzzleUpID;
    static const int16 rotatingLockPuzzleDownID;
    static const int16 rotatingLockPuzzleEndID;
    static const int16 leverPuzzleID;
    static const int16 leverPuzzleEndID;
    static const int16 telephoneID;
    static const int16 telephoneEndID;
    static const int16 sliderPuzzleID;
    static const int16 sliderPuzzleEndID;
    static const int16 passwordPuzzleEndID;

private:
    NancyEngine *_engine;

    int16 pointerId = 0;
    int16 pointerStyle = 0;
    bool itemHeld = false;

    uint16 _inputs;
};

} // End of namespace Nancy

#endif // NANCY_INPUT_H