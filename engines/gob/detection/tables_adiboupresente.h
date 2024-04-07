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

/* Detection tables for Adibou présente / Adiboo presents series. */
/* This Game uses the DEV7 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV7_Information */

#ifndef GOB_DETECTION_TABLES_ADIBOUPRESENTE_H
#define GOB_DETECTION_TABLES_ADIBOUPRESENTE_H

// -- French: Adibou présente Dessin --

{
	{
		"adiboudessin",
		_s("Missing game code"), // Adibou présente Dessin 1.00 (Engine: DEV7 version 1.1.0.0)
		AD_ENTRY1s("adibou.stk", "14e3f8e9c237d4236d93e08c60b784bc", 217172),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- French: Adibou présente Cuisine --

{
	{
		"adiboucuisine",
		_s("Missing game code"), // Adibou présente Cuisine 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adibou.stk", "cb2d576f6d546485af7693d4eaf1142b", 174027),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- French: Adibou présente Magie --

{
	{
		"adiboumagie",
		_s("Missing game code"), // Adibou présente Magie 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY2s("adibou.stk", "977d2449d398f3df23238d718fca35b5", 61097,
				   "magic.stk", "9776765dead3e338a32c43bf344b5819", 302664),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_ADIBOUPRESENTE_H
