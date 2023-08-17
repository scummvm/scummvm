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

/* Detection tables for Adibou 3 / Adiboo 3 series. */

#ifndef GOB_DETECTION_TABLES_ADIBOU3_H
#define GOB_DETECTION_TABLES_ADIBOU3_H

// -- French: Adibou 3 --

{
	{
		"adibou3",
		_s("Missing game code"), // Adibou 3 3.00 (Engine: DEV7 version 1.2.0.0)
		AD_ENTRY1s("b3_common.stk", "8819bc86b7af241ed336b1a84e34de07", 499731),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0, 0, 0
},

{
	{
		"adibou3",
		_s("Missing game code"), // Adibou 3 3.00 (Engine: DEV7 version 1.2.0.0)
		AD_ENTRY1s("b3_common.stk", "8819bc86b7af241ed336b1a84e34de07", 499731),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0, 0, 0
},

// -- German: Adiboo 3 --

{
	{

		"adibou3",
		_s("Missing game code"), // Adiboo 3 3.00 (Engine: DEV7 version 1.2.0.0)
		AD_ENTRY1s("b3_common.stk", "e3ed6837d19cc0ed19275f3196de2ae3", 523246),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0,0,0
},

{
	{

		"adibou3",
		_s("Missing game code"), // Adiboo 3 3.10 (Engine: DEV7 version 1.3.0.0)
		AD_ENTRY1s("b3_common.stk", "2293ff44a5bb7a36f5219443f0ede5cf", 554569),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0,0,0
},

{
	{

		"adibou3",
		_s("Missing game code"), // Adiboo 3 3.10 Demo (Engine: DEV7 version 1.3.0.0)
		AD_ENTRY1s("b3_common.stk", "0c7624de252a9be3c67616f298ecb34a", 558632),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0,0,0
},

{
	{

		"adibou3",
		_s("Missing game code"), // Adiboo 3 3.10 (shipped with English Application) (Engine: DEV7 version 1.3.0.0)
		AD_ENTRY1s("b3_common.stk", "fc3a619b44366ded7027bc458d34be6a", 554569),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures800x600,
	0,0,0
},

#endif // GOB_DETECTION_TABLES_ADIBOU3_H
