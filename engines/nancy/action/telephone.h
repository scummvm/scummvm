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

#ifndef NANCY_ACTION_TELEPHONE_H
#define NANCY_ACTION_TELEPHONE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/commontypes.h"

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Action {

class Telephone : public ActionRecord, public RenderObject {
public:
    struct PhoneCall {
        Common::Array<byte> phoneNumber; // 0x0, 11 bytes
        Common::String soundName; // 0xB
        Common::String text; // 0x15, 0xC8 bytes
        SceneChangeDescription sceneChange; // 0xDD
        // shouldStopRendering
        EventFlagDescription flag; // 0xE7
    };

    enum CallState { kWaiting, kButtonPress, kRinging, kBadNumber, kCall, kHangUp };

    Telephone(RenderObject &redrawFrom) :
        RenderObject(redrawFrom),
        callState(kWaiting),
        selected(0) {}
    virtual ~Telephone() {}

    virtual void init() override;

    virtual void readData(Common::SeekableReadStream &stream) override;
    virtual void execute() override;
    virtual void handleInput(NancyInput &input) override;

    Common::String imageName; // 0x00
    Common::Array<Common::Rect> srcRects; // 0xA, 12
    Common::Array<Common::Rect> destRects; // 0xCA, 12
    SoundDescription genericDialogueSound; // 0x18A
    SoundDescription genericButtonSound; // 0x1AC
    SoundDescription ringSound; // 0x1CE
    SoundDescription dialToneSound; // 0x1F0
    SoundDescription dialAgainSound; // 0x212
    SoundDescription hangUpSound; // 0x234
    Common::Array<Common::String> buttonSoundNames; // 0x256, 12 * 0xA
    Common::String addressBookString; // 0x2CE, 0xC8 long
    Common::String dialAgainString; // 0x396
    SceneChangeDescription reloadScene; // 0x45E
    EventFlagDescription flagOnReload; // 0x468 ??
    SceneChangeDescription exitScene; // 0x46C
    EventFlagDescription flagOnExit; // 0x476
    Common::Rect exitHotspot; // 0x47A
    // 0x48A numConvos
    Common::Array<PhoneCall> calls; // 0x48C

    Common::Array<byte> calledNumber;
    Graphics::ManagedSurface image;
    CallState callState;
    uint selected;

protected:
    virtual Common::String getRecordTypeName() const override { return "Telephone"; }

    virtual uint16 getZOrder() const override { return 7; }
    virtual BlitType getBlitType() const override { return kTrans; }
    virtual bool isViewportRelative() const override { return true; }

    void drawButton(uint id);
    void undrawButton(uint id);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TELEPHONE_H