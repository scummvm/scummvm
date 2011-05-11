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

#ifndef M4_BURGER_DATA_H
#define M4_BURGER_DATA_H

#include "m4/graphics.h"
#include "m4/actor.h"

namespace M4 {

InventoryObject burger_inventory [] = {
	// name					scene	icon
	//--------------------		-----	-----
	{ "empty jug",				303,	14 },
	{ "distilled juice",		999,	15 },
	{ "broken puz dispenser",	999,	16 },
	{ "puz dispenser",			999,	17 },
	{ "broken mouse trap",		999,	18 },
	{ "mouse trap",				999,	19 },
	{ "kindling",				999,	20 },
	{ "burning kindling",		999,	21 },
	{ "lights",					508,	22 },
	{ "lights on",				508,	23 },
	{ "bottle",					999,	24 },
	{ "carrot juice",			999,	25 },
	{ "soapy water",			999,	26 },
	{ "iron filings",			999,	27 },
	{ "waxed hair",				999,	28 },
	{ "fish",					999,	29 },
	{ "hook",					999,	30 },
	{ "keys",					999,	31 },
	{ "records",				999,	32 },
	{ "collar",					999,	33 },
	{ "amp",					999,	34 },
	{ "rubber gloves",			999,	35 },
	{ "sock",					504,	36 },
	{ "jaws of life",			999,	37 },
	{ "deed",					999,	38 },
	{ "burger morsel",			999,	39 },
	{ "whistle",				999,	40 },
	{ "coin",					999,	41 },
	{ "matches",				999,	42 },
	{ "phone cord",				999,	43 },
	{ "kibble",					602,	44 },		// picked up from tray
	{ "pantyhose",				999,	45 },
	{ "fan belt",				999,	46 },
	{ "spring",					999,	47 },
	{ "mirror",					999,	48 },
	{ "grate",					999,	49 },
	{ "ray gun",				604,	50 },		// given to Wilbur when he enters 604
	{ "grasshoppers",			999,	51 },
	{ "rolling pin",			999,	52 },
	{ "rubber duck",			999,	53 },
	{ "ladder",					999,	54 },
	{ "money",					999,	55 },
	{ "crow bar",				999,	56 },
	{ "Wilbur",					999,	57 }
};

} // End of namespace M4

#endif
