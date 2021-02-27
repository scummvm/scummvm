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

#ifndef NANCY_ACTION_PASSWORDPUZZLE_H
#define NANCY_ACTION_PASSWORDPUZZLE_H

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/sound.h"
#include "engines/nancy/time.h"

#include "common/str.h"
#include "common/rect.h"

namespace Nancy {
namespace Action {

class PasswordPuzzle : public ActionRecord, public RenderObject {
public:
    enum SolveState { kNotSolved, kFailed, kSolved };
    PasswordPuzzle(RenderObject &redrawFrom) :
        RenderObject(redrawFrom),
        passwordFieldIsActive(false),
        playerHasHitReturn(false),
        solveState(kNotSolved) {}
    virtual ~PasswordPuzzle() {}

    virtual void init() override;
    
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;
    virtual void handleInput(NancyInput &input) override;

    uint16 fontID; // 0x00
    Time cursorBlinkTime; // 0x2
    Common::Rect nameBounds; // 0x4
    Common::Rect passwordBounds; // 0x14
    // _screenPosition 0x24
    Common::String name; // 0x34, 20 bytes long
    Common::String password; // 0x48, 20 bytes long
    SceneChangeDesc solveExitScene; // 0x5A
    FlagDesc flagOnSolve; // 0x66
    SoundManager::SoundDescription solveSound; // 0x69
    SceneChangeDesc failExitScene; // 0x8B
    FlagDesc flagOnFail; // 0x95
    SoundManager::SoundDescription failSound; // 0x98
    SceneChangeDesc exitScene; // 0xBA
    FlagDesc flagOnExit; // 0xC4
    Common::Rect exitHotspot; // 0xC7

    Common::String playerNameInput;
    Common::String playerPasswordInput;
    Time nextBlinkTime;
    bool passwordFieldIsActive;
    bool playerHasHitReturn;
    SolveState solveState;

protected:
    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void drawText();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PASSWORDPUZZLE_H
