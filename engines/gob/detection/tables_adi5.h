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

/* Detection tables for Adi 5 / Addy 5 series. */
/* This Game uses the DEV7 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV7_Information */
/* These games are part of the Adi series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Adi_Games */

#ifndef GOB_DETECTION_TABLES_ADI5_H
#define GOB_DETECTION_TABLES_ADI5_H

// -- French: Adi 5 --

{
	{ // Supplied by BJNFNE

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Adi 5 5.01 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adi5.stk", "5de6b43725b47164e8b181de361d0693", 611309),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},
{
	{ // Supplied by BJNFNE

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Adi 5 5.04 (Engine: DEV7 version 1.10a)
		AD_ENTRY1s("adi5.stk", "17754a1b942c3af34e86820f19971895", 891549),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},

// -- German: Addy 5 --

{
	{ // Supplied by laenion in Bugreport #14956

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy 5 5.01 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adi5.stk", "ec2d6a05d13bec1b4dcfa18d88e317c6", 627942),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},
{
	{ // Supplied by Indy4-Fan

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy 5 5.03 (Engine: DEV7 version 1.10a)
		AD_ENTRY1s("adi5.stk", "b45a85ac21fccbb890edcbba36d11f42", 885616),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},
{
	{ // Supplied by BJNFNE

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy 5 5.04 (Engine: DEV7 version 1.10a)
		AD_ENTRY1s("adi5.stk", "7af169c901981f1fbf4535c194aa4cc0", 892359),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},

// -- Demos --

{
	{ // Supplied by BJNFNE

		"adi5",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy 5 Mathe Demo (Engine: DEV7 version unknown)
		AD_ENTRY1s("adi5.stk", "72fb3c7807845e414d107aa4612f95df", 141858),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO0()
	},
	kFeatures800x600,
	"adi5.stk", "adi5.obc", 0
},

// -- Add-ons : Language --
{
	{
		"adi5language",
		"", // Deutsch (Klasse 7+8)
		AD_ENTRY2s("FR12.ITK", "2f084125fa605a138a77ef7990eb2258", 27226086,
				   "FR13.ITK", "68a8c910f581f5ece90d1decf45bc09f", 17725973),
		DE_DEU,
		kPlatformWindows,
		ADGF_ADDON | ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	0, 0, 0
},
{
	{
		"adi5language",
		"", // Français Math CE1
		AD_ENTRY2s("fr06.itk", "5ad5150e8e0f5d2d2867669ecc5ed3be", 10449682,
				   "fr07.itk", "75daf1e48bf06ad28f3446662fb25253", 58580849),
		FR_FRA,
		kPlatformWindows,
		ADGF_ADDON | ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	0, 0, 0
},
{
	{
		"adi5language",
		"", // Français Math CE1
		AD_ENTRY2s("fr06.itk", "edf5b0d00f1cc670b49db5b8c79b4d0d", 10491811,
				   "fr07.itk", "83b00b904a989a9d1a83e8c6fbcf6478", 59044143),
		FR_FRA,
		kPlatformWindows,
		ADGF_ADDON | ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	0, 0, 0
},
{
	{
		"adi5language",
		"", // Français Math CM1
		AD_ENTRY2s("fr08.itk", "0daba190b67c2404fbdfb3aed3f82b4f", 53645106,
				   "fr09.itk", "0c4f77aa52e76163f25ce4abbf5d4788", 41428906),
		FR_FRA,
		kPlatformWindows,
		ADGF_ADDON | ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	0, 0, 0
},

// -- Add-ons : English --
{
	{
		"adi5anglais",
		"", // Englisch (Klasse 5)
		AD_ENTRY2s("EN07.ITK", "c7a89adebc67ad587e98e5a237ff679a", 95252930,
				   "EN11.ITK", "18cd5b3d9e405cccf27202ca28e1a68f", 54165516),
		DE_DEU,
		kPlatformWindows,
		ADGF_ADDON | ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_ADI5_H
