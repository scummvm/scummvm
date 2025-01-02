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

namespace MediaStation {

const PlainGameDescriptor mediastationGames[] = {
	// Commercially released games
	{ "georgeshrinks", "George Shrinks Interactive Storybook" },
	{ "mousecookie", "If You Give a Mouse a Cookie Interactive Storybook" },
	{ "lionking", "Disney's Animated Storybook: The Lion King" },
	{ "lambchop", "Lambchop Loves Music" },
	{ "frogprince", "Fractured Fairy Tales: The Frog Prince" },
	{ "honeytree", "Disney's Animated Storybook: Winnie the Pooh and the Honey Tree" },
	{ "notredame", "Disney's Animated Storybook: The Hunchback of Notre Dame" },
	{ "puzzlecastle", "Puzzle Castle" },
	{ "ibmcrayola", "IBM/Crayola Print Factory" },
	{ "ibmcrayolaholiday", "IBM/Crayola Print Factory Holiday Activity Pack" },
	{ "101dalmatians", "Disney's Animated Storybook: 101 Dalmatians" },
	{ "rupertsinteractiveadventures", "Rupert's Interactive Adventures" },
	{ "herculesasb", "Disney's Animated Storybook: Hercules" },
	{ "barbieasrapunzel", "Magic Fairy Tales: Barbie as Rapunzel" },
	{ "tonkasearchandrescue", "Tonka Search and Rescue" },
	{ "arielstorystudio", "Disney presents Ariel's Story Studio" },
	{ "tonkagarage", "Tonka Garage" },
	{ "dwpickyeater", "D.W. the Picky Eater" },
	{ "tonkaworkshop", "Tonka Workshop" },
	{ "tonkaraceway", "Tonka Raceway" },
	{ "stuartlittlebigcity", "Stuart Little: Big City Adventures"},

	// Released demos
	{ "puzzlecastledemo", "Puzzle Castle Demo" }, // From Frog Prince CD-ROM

	// For development purposes - detect any folder as a game
	{ "mediastation", "Media Station Game" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	// For testing purposes, any folder with a "MediaStation" file in it can be run.
	{
		"mediastation",
		nullptr,
		AD_ENTRY1s("MediaStation", 0, AD_NO_SIZE),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Commercially released games
	{
		"georgeshrinks", 
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "5b7c08398fe6ae016db9d94ad9240241", 6744,
			"103.CXT", "e7d563ff79f1b1416e5f1e0c803f78ec", 1474802
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"mousecookie",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "11d11b2067519d8368175cc8e8caa94f", 59454,
			"100.CXT", "cac48b9bb5f327d035a831cd15f1688c", 1762032
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"lionking",
		"2.0GB",
		AD_ENTRY2s(
			"BOOT.STM", "c90200e52bcaad52524520d461caef2b", 29884,
			"100.CXT", "ce40843604b8c52701694cd543072a88", 3253600
		),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"lambchop",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "c90200e52bcaad52524520d461caef2b", 29884,
			"100.CXT", "ce40843604b8c52701694cd543072a88", 3253600
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"frogprince",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "1c6d14c87790d009702be8ba4e4e5906", 13652,
			"100.CXT", "a5ec9a32c3741a20b82e1793e76234b2", 1630762
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"honeytree",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "694bcc9887f159137f3a0d937cfbbb08", 53904,
			"100.CXT", "fde1e528d69fbd060dfc2691320bc05b", 1971658
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"notredame",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "7949e1253a62531e53963a2fffe57211", 55300,
			"100.CXT", "54c11a94888a1b747e1c8935b7315889", 4766278
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"puzzlecastle",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "7b0faf38da2d76df40b4085eed6f4fc8", 22080,
			"100.CXT", "ebc4b6247b742733c81456dfd299aa55", 3346944
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"puzzlecastledemo",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "b7ce005e0d67021f792ebb73e7fbe34c", 5960,
			"100.CXT", "cc64a6fcb3af2736d622658cff3ef2b5", 1262
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"ibmcrayola",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "359542015c6665c70252cf21a8467cdb", 11044,
			"100.CXT", "42bffe4165640dd1e64a6e8565f48af3", 5125226
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"ibmcrayolaholiday",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "50f30298bf700f357d98c4390f75cb7a", 10932,
			"100.CXT", "8110f70f1d01d0f42cac9b1bb6d2de12", 4967390
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"101dalmatians",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "50f30298bf700f357d98c4390f75cb7a", 10932,
			"100.CXT", "8110f70f1d01d0f42cac9b1bb6d2de12", 4967390
		),	
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"rupertsinteractiveadventures",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "051ff838587d43edc9836dc3a9888c16", 13112,
			"100.CXT", "65326647eedc2ad9a8c0ccef274b3389", 5180650
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"herculesasb",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "afc773416e46e30873f743e234794957", 26924,
			"100.CXT", "56875e1640320909e9697f11b5a8c9a6", 4895998
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"barbieasrapunzel",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "eef6bdf54d2ae25af0ec29361fd4c126", 17530,
			"100.CXT", "f0bcc27b61bfb33328db2dd537b2b6e3", 1688902
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"tonkasearchandrescue",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "eef6bdf54d2ae25af0ec29361fd4c126", 17530,
			"100.CXT", "f0bcc27b61bfb33328db2dd537b2b6e3", 1688902
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"arielstorystudio",
		"1.0",
		AD_ENTRY2s(
			"BOOT.STM", "297670b908f887ed6c97b364406575d0", 65480,
			"100.CXT", "c12c5b784ad931eca293a9816c11043b", 6532022
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"arielstorystudio",
		"1.1",
		AD_ENTRY2s(
			"BOOT.STM", "7d53a551efde620fe5b332d7b1f009ab", 65450,
			"100.CXT", "993252bca0aa6791ca3da30b1ae6f5f8", 6532022
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"tonkagarage",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "fc8863bb302e94d3b778b3a97556601b", 25208,
			"100.CXT", "13683c2a06275920181d9dda5b2b69e7", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"dwpickyeater",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "80cc94e3e894ee8c5a22a9c07a33d891", 26402,
			"100.CXT", "e65e359ab25d7a639cf369a01b9a21c0", 2163750
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"tonkaworkshop",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "15e6d32925f557f3196fd0bb79b25375", 38190,
			"100.CXT", "1cb35998f2e044eee59a96120b3bda6c", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"tonkaraceway",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "da512cb9bcd18465294e544ed790881c", 12272,
			"100.CXT", "30802327b29fbfa722a707c3d3b0f8f8", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"stuartlittlebigcity",
		nullptr,
		AD_ENTRY2s(
			"BOOT.STM", "992787bf30104a4b7aa2ead64dda21ff", 10974,
			"100.CXT", "21f44a1d1de6abf8bd67341c155dfead", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace MediaStation
