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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

/* Detection tables for the Nathan Vacances series. */
/* This Game uses the DEV7 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV7_Information */

#ifndef GOB_DETECTION_TABLES_NATHANVACANCES_H
#define GOB_DETECTION_TABLES_NATHANVACANCES_H

//#include "common/translation.h"  // Make release checker happy

// -- French: Nathan Vacances --

{
	{
		"nathanvacances",
		_s("Missing game code"), // CM1/CE2 1.00 (DEV7 version 1.20a)
		AD_ENTRY1s("common.stk", "344185d17c593122a548122df63b70cf", 1851672),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"common.stk", "A5HS_Start.obc", 0
},

#endif // GOB_DETECTION_TABLES_NATHANVACANCES_H
