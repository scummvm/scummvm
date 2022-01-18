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

/* Detection tables for The Last Dynasty. */

#ifndef GOB_DETECTION_TABLES_DYNASTY_H
#define GOB_DETECTION_TABLES_DYNASTY_H

// -- Windows --

{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "6190e32404b672f4bbbc39cf76f41fda", 2511470),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "b3f8472484b7a1df94557b51e7b6fca0", 2322644),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "bdbdac8919200a5e71ffb9fb0709f704", 2446652),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "a4a50c70d001b4398b174f1bff1987f6", 2607984),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "4bfcc878f2fb2f0809d1f257e1180cf1", 2857990),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},

// -- Demos --

{	// Non-interactive
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("intro.stk", "464538a17ed39755d7f1ba9c751af1bd", 1847864),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{	// Non-interactive
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("intro.stk", "e49340fe5078e38e9f9290dfb75f98a5", 1348),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("lda1.stk", "0e56a899357cbc0bf503260fd2dd634e", 15032774),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	"lda1.stk", 0, 0
},
{
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("lda1.stk", "8669ea2e9a8239c070dc73958fbc8753", 15567724),
		DE_DEU,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	"lda1.stk", 0, 0
},
{
	{
		"dynasty",
		"Demo",
		AD_ENTRY2s("demo.scn",	"a0d801c43a560b7471114744858b129c", 89,
				   "demo5.vmd", "2abb7b6a26406c984f389f0b24b5e28e", 13290970),
		EN_ANY,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480 | kFeaturesSCNDemo,
	"demo.scn", 0, 1
},
// Combined demo for Woodruff and The Last Dynasty
{
	{
		"dynastywood",
		"Non-Interactive Demos",
		AD_ENTRY2s("demo.scn",		"040a00b7276aa86fe7a51f5f362f63c7", 124,
				   "demo5.vmd",		"2abb7b6a26406c984f389f0b24b5e28e", 13290970),
		EN_ANY,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)	
	},
	kGameTypeDynastyWood,
	kFeatures640x480 | kFeaturesSCNDemo,
	"demo.scn", 0, 1
},

#endif // GOB_DETECTION_TABLES_DYNASTY_H
