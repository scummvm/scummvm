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

#include "engines/nancy/ui/fullscreenimage.h"

#include "engines/nancy/commontypes.h"

#include "common/rect.h"

#ifndef NANCY_STATE_HELP_H
#define NANCY_STATE_HELP_H

namespace Nancy {

class NancyEngine;

namespace State {

class Help {
public:
    enum State { kInit, kBegin, kRun, kWaitForSound };
    Help(NancyEngine *engine) : _engine(engine), _state(kInit), _image(engine) {}

    void process();

private:
    void init();
    void begin();
    void run();
    void waitForSound();

    NancyEngine *_engine;
    State _state;
    UI::FullScreenImage _image;
    Common::Rect _hotspot; // Can be an array, but isn't in nancy1
    SoundDescription _sound;
    uint _previousState;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_HELP_H
