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

#ifndef NANCY_ACTION_SLIDERPUZZLE_H
#define NANCY_ACTION_SLIDERPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Action {

class SliderPuzzle: public ActionRecord, public RenderObject {
public:
    enum SolveState { kNotSolved, kWaitForSound };
    SliderPuzzle(RenderObject &redrawFrom) : RenderObject(redrawFrom) {}
    virtual ~SliderPuzzle() {}
    
    virtual void init() override;
    
    virtual uint16 readData(Common::SeekableReadStream &stream) override;
    virtual void execute(Nancy::NancyEngine *engine) override;
    virtual void handleInput(NancyInput &input) override;

    Common::String imageName; // 0x00
    uint16 width; // 0xA
    uint16 height; // 0xC
    Common::Array<Common::Array<Common::Rect>> srcRects; // 0x0E, size 0x240
    Common::Array<Common::Array<Common::Rect>> destRects; // 0x24E, size 0x240
    Common::Array<Common::Array<int16>> correctTileOrder; // 0x48E, size 0x48
    SoundDescription clickSound; // 0x4D6
    SceneChangeDescription solveExitScene; // 0x4F8
    EventFlagDescription flagOnSolve; // 0x502
    SoundDescription solveSound; // 0x505
    SceneChangeDescription exitScene; // 0x527
    EventFlagDescription flagOnExit; // 0x531
    Common::Rect exitHotspot; // 0x534

    SolveState solveState = kNotSolved;
    Graphics::ManagedSurface image;

    static Common::Array<Common::Array<int16>> playerTileOrder;
    static bool playerHasTriedPuzzle;

protected:
    virtual Common::String getRecordTypeName() const override { return "SliderPuzzle"; }

    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void drawTile(uint tileID, uint posX, uint posY);
    void undrawTile(uint posX, uint posY);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SLIDERPUZZLE_H
