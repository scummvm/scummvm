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

#ifndef NANCY_ACTION_ORDERINGPUZZLE_H
#define NANCY_ACTION_ORDERINGPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Action {

class OrderingPuzzle : public ActionRecord, public RenderObject {
public:
    enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
    OrderingPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~OrderingPuzzle() {}

    virtual void init() override;
    
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    virtual void handleInput(NancyInput &input) override;
    virtual void onPause(bool pause) override;

    Common::String imageName; // 0x00
    Common::Array<Common::Rect> srcRects; // 0xC, 15
    Common::Array<Common::Rect> destRects; // 0xFC, 15
    uint16 sequenceLength; // 0x1EC;
    Common::Array<byte> correctSequence; // 0x1EE, 15 bytes
    Nancy::SoundDescription clickSound; // 0x1FD, kNormal
    SceneChangeDescription solveExitScene; // 0x21F
    EventFlagDescription flagOnSolve; // 0x229
    uint16 solveSoundDelay; // 0x22C 
    Nancy::SoundDescription solveSound; // 0x22E
    SceneChangeDescription exitScene; // 0x250
    EventFlagDescription flagOnExit; // 0x25A
    Common::Rect exitHotspot; // 0x25D

    SolveState solveState = kNotSolved;
    Graphics::ManagedSurface image;
    Common::Array<int16> clickedSequence;
    Common::Array<bool> drawnElements;
    Time solveSoundPlayTime;

protected:
    virtual Common::String getRecordTypeName() const override { return "OrderingPuzzle"; }
    
    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void drawElement(uint id);
    void undrawElement(uint id);
    void clearAllElements();
};

} // End of namespace Action 
} // End of namespace Nancy

#endif // NANCY_ACTION_ORDERINGPUZZLE_H
