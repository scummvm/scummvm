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

/* Detection tables for Bargon Attack. */

#ifndef GOB_DETECTION_TABLES_BARGON_H
#define GOB_DETECTION_TABLES_BARGON_H

// -- DOS VGA Floppy --

{
	{
		"bargon",
		"",
		AD_ENTRY1("intro.stk", "da3c54be18ab73fbdb32db24624a9c23"),
		UNK_LANG,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by cesardark in bug #3123
	{
		"bargon",
		"",
		AD_ENTRY1s("intro.stk", "11103b304286c23945560b391fd37e7d", 3181890),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by paul66 in bug #3143
	{
		"bargon",
		"",
		AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by kizkoool in bugreport #3926
	{
		"bargon",
		"",
		AD_ENTRY1s("intro.stk", "00f6b4e2ee26e5c40b488e2df5adcf03", 3975580),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by glorfindel in bugreport #3193
	{
		"bargon",
		"Fanmade",
		AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
		IT_ITA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},
{ // Russian fan translation by PRCA
	{
		"bargon",
		"Fanmade",
		AD_ENTRY1s("intro.stk", "0937f20c9177a9c4111e48f8916fea47", 3185593),
		RU_RUS,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},

// -- Amiga --

{ // Supplied by pwigren in bugreport #3355
	{
		"bargon",
		"",
		AD_ENTRY1s("intro.stk", "569d679fe41d49972d34c9fce5930dda", 269825),
		EN_ANY,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},

// -- Atari ST --

{ // Supplied by Trekky in the forums
	{
		"bargon",
		"",
		AD_ENTRY1s("intro.stk", "2f54b330d21f65b04b7c1f8cca76426c", 262109),
		FR_FRA,
		kPlatformAtariST,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeBargon,
	kFeaturesNone,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_BARGON_H
