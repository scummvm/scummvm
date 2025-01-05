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

#ifndef GOT_DATA_SETUP_H
#define GOT_DATA_SETUP_H

#include "common/serializer.h"

namespace Got {

struct SETUP {
    SETUP() :
        f00(0), f01(0), f02(0), f03(0), f04(0), f05(0), f06(0), f07(0), f08(0), f09(0),
        f10(0), f11(0), f12(0), f13(0), f14(0), f15(0), f16(0), f17(0), f18(0), f19(0),
        f20(0), f21(0), f22(0), f23(0), f24(0), f25(0), f26(0), f27(0), f28(0), f29(0),
        f30(0), f31(0), f32(0), f33(0), f34(0), f35(0), f36(0), f37(0), f38(0), f39(0),
        f40(0), f41(0), f42(0), f43(0), f44(0), f45(0), f46(0), f47(0), f48(0), f49(0),
        f50(0), f51(0), f52(0), f53(0), f54(0), f55(0), f56(0), f57(0), f58(0), f59(0),
        f60(0), f61(0), f62(0), f63(0) {
    }

    unsigned int  f00 : 1;
    unsigned int  f01 : 1;
    unsigned int  f02 : 1;
    unsigned int  f03 : 1;
    unsigned int  f04 : 1;
    unsigned int  f05 : 1;
    unsigned int  f06 : 1;
    unsigned int  f07 : 1;

    unsigned int  f08 : 1;
    unsigned int  f09 : 1;
    unsigned int  f10 : 1;
    unsigned int  f11 : 1;
    unsigned int  f12 : 1;
    unsigned int  f13 : 1;
    unsigned int  f14 : 1;
    unsigned int  f15 : 1;

    unsigned int  f16 : 1;
    unsigned int  f17 : 1;
    unsigned int  f18 : 1;
    unsigned int  f19 : 1;
    unsigned int  f20 : 1;
    unsigned int  f21 : 1;
    unsigned int  f22 : 1;
    unsigned int  f23 : 1;

    unsigned int  f24 : 1;
    unsigned int  f25 : 1;
    unsigned int  f26 : 1;
    unsigned int  f27 : 1;
    unsigned int  f28 : 1;
    unsigned int  f29 : 1;
    unsigned int  f30 : 1;
    unsigned int  f31 : 1;

    unsigned int  f32 : 1;
    unsigned int  f33 : 1;
    unsigned int  f34 : 1;
    unsigned int  f35 : 1;
    unsigned int  f36 : 1;
    unsigned int  f37 : 1;
    unsigned int  f38 : 1;
    unsigned int  f39 : 1;

    unsigned int  f40 : 1;
    unsigned int  f41 : 1;
    unsigned int  f42 : 1;
    unsigned int  f43 : 1;
    unsigned int  f44 : 1;
    unsigned int  f45 : 1;
    unsigned int  f46 : 1;
    unsigned int  f47 : 1;

    unsigned int  f48 : 1;
    unsigned int  f49 : 1;
    unsigned int  f50 : 1;
    unsigned int  f51 : 1;
    unsigned int  f52 : 1;
    unsigned int  f53 : 1;
    unsigned int  f54 : 1;
    unsigned int  f55 : 1;

    unsigned int  f56 : 1;
    unsigned int  f57 : 1;
    unsigned int  f58 : 1;
    unsigned int  f59 : 1;
    unsigned int  f60 : 1;
    unsigned int  f61 : 1;
    unsigned int  f62 : 1;
    unsigned int  f63 : 1;

    byte value[16] = {};
    byte junk = 0;
    byte game = 0;
    byte area = 0;          //1,2,3
    byte pc_sound = 0;      //1=enabled
    byte dig_sound = 0;     //1 & !pc_sound = enabled
    byte music = 0;         //1=enabled
    byte speed = 0;         //1=slow mode (for slower 286's)
    byte scroll_flag = 0;   //unused
    byte boss_dead[3] = {};
    byte skill = 0;         //0=easy, 1=normal, 2=hard
    byte game_over = 0;
    byte future[19] = {};   //probably not needed

    void sync(Common::Serializer &s);
};

} // namespace Got

#endif
