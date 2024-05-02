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

/* Detection tables for Urban Runner. */
/* This Game uses the DEV6 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV6_Information */

#ifndef GOB_DETECTION_TABLES_URBAN_H
#define GOB_DETECTION_TABLES_URBAN_H

// -- Windows --

{
	{
		"urban",
		"v1.00",
		AD_ENTRY1s("intro.stk", "3ab2c542bd9216ae5d02cc6f45701ae1", 1252436),
		EN_USA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by Collector9 in bug report #5611
	{
		"urban",
		"v1.00",
		AD_ENTRY1s("intro.stk", "6ce3d878178932053267237ec4843ce1", 1252518),
		EN_USA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by gamin in the forums
	{
		"urban",
		"v1.00",
		AD_ENTRY1s("intro.stk", "b991ed1d31c793e560edefdb349882ef", 1276408),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by jvprat on #scummvm
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "4ec3c0864e2b54c5b4ccf9f6ad96528d", 1253328),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by Alex on the gobsmacked blog
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "9ea647085a16dd0fb9ecd84cd8778ec9", 1253436),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by alex86r in bug report #5690
	{
		"urban",
		"v1.01",
		AD_ENTRY1s("intro.stk", "4e4a3c017fe5475353bf94c455fe3efd", 1253448),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4308
	{
		"urban",
		"v1.00",
		AD_ENTRY1s("intro.stk", "4bd31979ea3d77a58a358c09000a85ed", 1253018),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},

// -- Demos --

{
	{
		"urban",
		"Non-Interactive Demo",
		AD_ENTRY3s("wdemo.s24", "14ac9bd51db7a075d69ddb144904b271", 87,
            	   "demo.vmd", "65d04715d871c292518b56dd160b0161", 9091237,
				   "urband.vmd", "60343891868c91854dd5c82766c70ecc", 922461),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesTrueColor | kFeaturesSCNDemo,
	0, 0, 2
},

#endif // GOB_DETECTION_TABLES_URBAN_H
