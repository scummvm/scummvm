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

#ifndef M4_RIDDLE_ROOMS_SECTION6_H
#define M4_RIDDLE_ROOMS_SECTION6_H

#include "m4/riddle/rooms/section.h"
#include "m4/riddle/rooms/section6/room603.h"
#include "m4/riddle/rooms/section6/room604.h"
#include "m4/riddle/rooms/section6/room605.h"
#include "m4/riddle/rooms/section6/room607.h"
#include "m4/riddle/rooms/section6/room608.h"
#include "m4/riddle/rooms/section6/room610.h"
#include "m4/riddle/rooms/section6/room615.h"
#include "m4/riddle/rooms/section6/room620.h"
#include "m4/riddle/rooms/section6/room621.h"
#include "m4/riddle/rooms/section6/room622.h"
#include "m4/riddle/rooms/section6/room623.h"
#include "m4/riddle/rooms/section6/room624.h"
#include "m4/riddle/rooms/section6/room625.h"
#include "m4/riddle/rooms/section6/room626.h"
#include "m4/riddle/rooms/section6/room627.h"
#include "m4/riddle/rooms/section6/room628.h"
#include "m4/riddle/rooms/section6/room629.h"
#include "m4/riddle/rooms/section6/room630.h"
#include "m4/riddle/rooms/section6/room631.h"
#include "m4/riddle/rooms/section6/room632.h"
#include "m4/riddle/rooms/section6/room633.h"
#include "m4/riddle/rooms/section6/room634.h"
#include "m4/riddle/rooms/section6/room635.h"
#include "m4/riddle/rooms/section6/room636.h"
#include "m4/riddle/rooms/section6/room637.h"
#include "m4/riddle/rooms/section6/room638.h"
#include "m4/riddle/rooms/section6/room640.h"
#include "m4/riddle/rooms/section6/room641.h"
#include "m4/riddle/rooms/section6/room642.h"
#include "m4/riddle/rooms/section6/room643.h"
#include "m4/riddle/rooms/section6/room644.h"
#include "m4/riddle/rooms/section6/room645.h"
#include "m4/riddle/rooms/section6/room646.h"
#include "m4/riddle/rooms/section6/room647.h"
#include "m4/riddle/rooms/section6/room648.h"
#include "m4/riddle/rooms/section6/room649.h"
#include "m4/riddle/rooms/section6/room650.h"
#include "m4/riddle/rooms/section6/room651.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

extern const int16 SECTION6_NORMAL_DIRS[];
extern const char *SECTION6_NORMAL_NAMES[];
extern const int16 SECTION6_SHADOW_DIRS[];
extern const char *SECTION6_SHADOW_NAMES[];

class Section6 : public Section {
private:
	Room603 _room603;
	Room604 _room604;
	Room605 _room605;
	Room607 _room607;
	Room608 _room608;
	Room610 _room610;
	Room615 _room615;
	Room620 _room620;
	Room621 _room621;
	Room622 _room622;
	Room623 _room623;
	Room624 _room624;
	Room625 _room625;
	Room626 _room626;
	Room627 _room627;
	Room628 _room628;
	Room629 _room629;
	Room630 _room630;
	Room631 _room631;
	Room632 _room632;
	Room633 _room633;
	Room634 _room634;
	Room635 _room635;
	Room636 _room636;
	Room637 _room637;
	Room638 _room638;
	Room640 _room640;
	Room641 _room641;
	Room642 _room642;
	Room643 _room643;
	Room644 _room644;
	Room645 _room645;
	Room646 _room646;
	Room647 _room647;
	Room648 _room648;
	Room649 _room649;
	Room650 _room650;
	Room651 _room651;

public:
	Section6();
	virtual ~Section6() {}

	void daemon() override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
