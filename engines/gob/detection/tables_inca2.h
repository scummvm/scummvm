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

/* Detection tables for Inca II: Wiracocha. */

#ifndef GOB_DETECTION_TABLES_INCA2_H
#define GOB_DETECTION_TABLES_INCA2_H

// -- DOS VGA Floppy --

{
	{
		"inca2",
		"v1.000",
		AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"v1.000",
		AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"v1.000",
		AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// US floppy box dated 18.03.1994
{
	{
		"inca2",
		"v1.0",
		AD_ENTRY1s("intro.stk", "48cc6e6b0b0b343f876290d2700d8eba", 804780),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA CD --

{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{	// Bugreport #12757
		"inca2",
		"v1.07",
		AD_ENTRY1s("intro.stk", "b56e4147acc5852c6fc2de5985ab94b0", 804796),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
		IT_ITA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},

// -- Windows --

{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
		EN_USA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"inca2",
		"",
		AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Demos --

{
	{
		"inca2",
		"Non-Interactive Demo (v2.0)", // dated 8/1/93
		AD_ENTRY1s("cons.imd", "f896ba0c4a1ac7f7260d342655980b49", 17804),
		EN_ANY,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib | kFeaturesBATDemo,
	0, 0, 7
},

#endif // GOB_DETECTION_TABLES_INCA2_H
