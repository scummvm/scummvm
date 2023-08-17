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

/* Detection tables for Adi 5 / Addy 5 series. */

#ifndef GOB_DETECTION_TABLES_ADI5_H
#define GOB_DETECTION_TABLES_ADI5_H

// -- French: Adi 5 --

// -- German: Addy 5 --

{
	{

		"adi5",
		_s("Missing game code"), // Addy 5 5.04 (Engine: DEV7 version 1.1.0.0)
		AD_ENTRY1s("adi5.stk", "7af169c901981f1fbf4535c194aa4cc0", 892359),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0,0,0
},

#endif // GOB_DETECTION_TABLES_ADI5_H
