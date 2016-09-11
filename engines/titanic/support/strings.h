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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_STRINGS_H
#define TITANIC_STRINGS_H

#include "common/str-array.h"

namespace Titanic {

enum StringId {
	BLANK,
	STANDING_OUTSIDE_PELLERATOR,
	BOT_BLOCKING_PELLERATOR,
	SUCCUBUS_IS_IN_STANDBY,
	NOTHING_TO_DELIVER,
	NOTHING_IN_SUCCUBUS_TRAY,
	SUCCUBUS_SINGLE_DELIVERY,
	ONE_ALLOCATED_CHICKEN_PER_CUSTOMER,
	ONE_CHICKEN_PER_CUSTOMER,
	UPGRADED_TO_FIRST_CLASS,
	UPGRADED_TO_SECOND_CLASS,
	ROOM_RESERVED_FOR_FIRST_CLASS,
	NO_LOSERS,
	CLASS_NOT_PERMITTED_IN_AREA,
	EXIT_FROM_OTHER_SIDE,
	TRANSPORT_OUT_OF_ORDER,
	FAN_HAS_BLOWN_A_FUSE,
	POKE_WITH_LONG_STICK,
	TOO_SHORT_TO_REACH_BRANCHES,
	OUTSIDE_ELEVATOR_NUM,
	BOT_BLOCKING_ELEVATOR,
	ELEVATOR_NON_FUNCTIONAL,
	LIGHT_IS_LOOSE,
	LUMI_GLOW_LIGHTS,
	ALREADY_HAVE_STICK,
	GLASS_IS_UNBREAKABLE,
	FOR_STICK_BREAK_GLASS,
	DISPENSOR_HAS_UNBREAKABLE_GLASS,
	CHICKEN_IS_CLEAN,
	ADJUST_VIEWING_APPARATUS,
	CANNOT_TAKE_CAGE_LOCKED,
	ALREADY_AT_DESTINATION,
	CLASS_NOT_ALLOWED_AT_DEST,
	AT_LEAST_3RD_CLASS_FOR_HELP,
	NO_ROOM_ASSIGNED,
	ELEVATOR_NOT_BELOW_27,
	SELECT_GAME_TO_LOAD,
	SELECT_GAME_TO_SAVE,
	SUPPLY_GALACTIC_REFERENCE,
	LOCKED_MUSIC_SYSTEM,
	STUCK_TO_BRANCH,
	FROZEN_TO_BRANCH,
	CHECK_IN_AT_RECEPTION,
	FOODSTUFF_ALREADY_GARNISHED,
	DISPENSOR_IS_EMPTY,
	PUT_FOOD_UNDER_DISPENSOR,
	SEASONAL_SWITCH_NOT_WORKING,
	YOUR_STATEROOM,
	BED_NOT_SUPPORT_YOUR_WEIGHT,
	NOT_YOUR_ASSIGNED_ROOM,
	OUT_OF_REACH,
	SUCCUBUS_DESCRIPTION,
	CANAL_CLOSED_FOR_WINTER,
	AREA_OFF_LIMIT_TO_PASSENGERS,
	GO_WHERE,
	NICE_IF_TAKE_BUT_CANT,
	BOWL_OF_NUTS,
	NOT_A_BOWL_OF_NUTS
};

class Strings : public Common::StringArray {
public:
	Strings();
};

} // End of namespace Titanic

#endif /* TITANIC_STRINGS_H */
