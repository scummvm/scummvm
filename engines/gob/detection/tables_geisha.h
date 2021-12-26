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

/* Detection tables for Geisha. */

#ifndef GOB_DETECTION_TABLES_GEISHA_H
#define GOB_DETECTION_TABLES_GEISHA_H

// -- DOS EGA Floppy --

{
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "6eebbb98ad90cd3c44549fc2ab30f632", 212153),
		EN_ANY,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "6eebbb98ad90cd3c44549fc2ab30f632", 212153),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{ // Supplied by misterhands in bug report #6079
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "0c4c16090921664f50baefdfd24d7f5d", 211889),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{ // Supplied by einstein95 in bug report #6102
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "49107ac897e7c00af6c4ecd78a74a710", 212169),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{ // Supplied by einstein95 in bug report #6102
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "49107ac897e7c00af6c4ecd78a74a710", 212169),
		IT_ITA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{ // Supplied by alestedx in bug report #6269
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "49107ac897e7c00af6c4ecd78a74a710", 212164),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},
{
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "f4d4d9d20f7ad1f879fc417d47faba89", 336732),
		UNK_LANG,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA | kFeaturesAdLib,
	"disk1.stk", "intro.tot", 0
},

// -- Amiga --

{
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "e5892f00917c62423e93f5fd9920cf47", 208120),
		EN_ANY,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA,
	"disk1.stk", "intro.tot", 0
},

{ // Supplied by CaptainHIT in bug report #11594
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "260abe99a1fe0aa0ca76348e9f9f7746", 208133),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA,
	"disk1.stk", "intro.tot", 0
},

{ // Supplied by CaptainHIT in bug report #11593
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "948a74459c9433273bb4c7a2b4ccbf6c", 208135),
		FR_FRA,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA,
	"disk1.stk", "intro.tot", 0
},

{ // Supplied by CaptainHIT in bug report #11595
	{
		"geisha",
		"",
		AD_ENTRY1s("disk1.stk", "84e2b52fbfa965c59dc6a6db52b39450", 208148),
		ES_ESP,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGeisha,
	kFeaturesEGA,
	"disk1.stk", "intro.tot", 0
},

#endif // GOB_DETECTION_TABLES_GEISHA_H
