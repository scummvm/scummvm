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


#ifndef GAMOS_PROC_H
#define GAMOS_PROC_H

#include "common/events.h"

namespace Gamos {

enum ACT2 {
    ACT_NONE = 0xe,
    ACT2_81 = 0x81,
    ACT2_82 = 0x82,
    ACT2_83 = 0x83,
    ACT2_84 = 0x84,
    ACT2_8f = 0x8f,
};

class SystemProc {
public:

    void processMessage(const Common::Event &ev);

public:
    uint8 _act1 = 0;
    uint8 _act2 = 0;

    uint16 _rawKeyCode = 0;
    uint16 _ascii = 0;


    Common::Point _mouseReportedPos;
    Common::Point _mouseActPos;

    uint8 _gd2flags = 0; /* 0x4 */
    uint16 _keyCodes[12]; /* 0x40 */

};


}

#endif //GAMOS_PROC_H
