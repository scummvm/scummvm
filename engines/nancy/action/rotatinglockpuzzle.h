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

#ifndef NANCY_ACTION_ROTATINGLOCKPUZZLE_H
#define NANCY_ACTION_ROTATINGLOCKPUZZLE_H

#include "engines/nancy/action/recordtypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/sound.h"
#include "engines/nancy/time.h"

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

namespace Nancy {
namespace Action {

class RotatingLockPuzzle : public ActionRecord, public RenderObject {
public:
    enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
    RotatingLockPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~RotatingLockPuzzle() {}
    
    virtual void init() override;
    
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;
    virtual void handleInput(NancyInput &input) override;

    Common::String imageName; // 0x00
    // 0xA numDials
    Common::Array<Common::Rect> srcRects; // 0xC, 10
    Common::Array<Common::Rect> destRects; // 0xAC, 8
    Common::Array<Common::Rect> upHotspots; // 0x12C, 8
    Common::Array<Common::Rect> downHotspots; // 0x1AC, 8
    Common::Array<byte> correctSequence; // 0x22C
    Nancy::SoundManager::SoundDescription clickSound; // 0x234, kNormal
    SceneChangeDesc solveExitScene; // 0x256
    FlagDesc flagOnSolve; // 0x260
    uint16 solveSoundDelay; // 0x263
    Nancy::SoundManager::SoundDescription solveSound; // 0x265
    SceneChangeDesc exitScene; // 0x287
    FlagDesc flagOnExit; // 0x291
    Common::Rect exitHotspot; // 0x294

    SolveState solveState = kNotSolved;
    Graphics::ManagedSurface image;
    Common::Array<byte> currentSequence;
    Time solveSoundPlayTime;


protected:
    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void drawDial(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ROTATINGLOCKPUZZLE_H
