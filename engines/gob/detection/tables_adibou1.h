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

/* Detection tables for Adibou 1 / A.J.'s World of Discovery / ADI Jnr. */

#ifndef GOB_DETECTION_TABLES_ADIBOU1_H
#define GOB_DETECTION_TABLES_ADIBOU1_H

//  -- French: Adibou --

{
	{
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "6db110188fcb7c5208d9721b5282682a", 4805104),
		FR_FRA,
		kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib,
	0, 0, 0
},

{
	{
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "904a93f46687617bb34e672020fc17a4", 248724),
		FR_FRA,
		kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib | kFeaturesEGA,
	0, "base.tot", 0
},

{
	{
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "228edf921ebcd9f1c6d566856f264ea4", 2647968),
		FR_FRA,
		kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib,
	0, 0, 0
},

//  -- German: ADI Jr. --

{
	{
		"adibou1",
		"ADI Jr. 4-6 Jahre",
		AD_ENTRY1s("intro.stk", "4d4c23da4cd7e080cb1769b49ace1805", 4731020),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib | kFeatures640x480,
	0, 0, 0
},

// -- DOS VGA Floppy --

{
	{
		"ajworld",
		"",
		AD_ENTRY1s("intro.stk", "e453bea7b28a67c930764d945f64d898", 3913628),
		EN_ANY,
		kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib,
	0, 0, 0
},


// -- Amiga Floppy --

{
	{
		"ajworld",
		"",
		AD_ENTRY1s("intro.stk", "71e7db034890885ac96dd1be43a21c38", 556834),
		EN_ANY,
		kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesNone,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_ADIBOU1_H
