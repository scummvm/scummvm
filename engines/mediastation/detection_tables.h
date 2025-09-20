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
	{ "georgeshrinks", "George Shrinks Interactive Storybook" },
	{ "mousecookie", "If You Give a Mouse a Cookie Interactive Storybook" },
	{ "lionking", "Disney's Animated Storybook: The Lion King" },
	{ "lambchop", "Lamb Chop Loves Music" },
	{ "frogprince", "Fractured Fairy Tales: The Frog Prince" },
	{ "honeytree", "Disney's Animated Storybook: Winnie the Pooh and the Honey Tree" },
	{ "notredame", "Disney's Animated Storybook: The Hunchback of Notre Dame" },
	{ "puzzlecastle", "Puzzle Castle" },
	{ "ibmcrayola", "IBM/Crayola Print Factory" },
	{ "ibmcrayolaholiday", "IBM/Crayola Print Factory Holiday Activity Pack" },
	{ "101dalmatians", "Disney's Animated Storybook: 101 Dalmatians" },
	{ "herculesasb", "Disney's Animated Storybook: Hercules" },
	{ "barbieasrapunzel", "Magic Fairy Tales: Barbie as Rapunzel" },
	{ "tonkasearchandrescue", "Tonka Search and Rescue" },
	{ "arielstorystudio", "Disney presents Ariel's Story Studio" },
	{ "tonkagarage", "Tonka Garage" },
	{ "dwpickyeater", "D.W. the Picky Eater" },
	{ "tonkaworkshop", "Tonka Workshop" },
	{ "tonkaraceway", "Tonka Raceway" },
	{ "stuartlittlebigcity", "Stuart Little: Big City Adventures"},
	{ nullptr, nullptr }
};

// In these entries, the executable must always be listed first.
// The title version can be obtained by pressing Ctrl-V while running
// the original interpreter. Some titles include a built-in language code
// (e.g. "v1.0/DE" or "v1.0/US") but others do not (e.g. "v1.1").
const ADGameDescription gameDescriptions[] = {
	// George Shrinks Interactive Storybook
	{
		"georgeshrinks",
		"v1.0",
		AD_ENTRY3s(
			"GEORGE.EX_", "ae70a2efbe5fbe66ad7bb9f269ea0a2f", 139674, // Packed executable
			"BOOT.STM", "5b7c08398fe6ae016db9d94ad9240241", 6744,
			"103.CXT", "e7d563ff79f1b1416e5f1e0c803f78ec", 1474802
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// If You Give a Mouse a Cookie Interactive Storybook
	{
		"mousecookie",
		"v2.0",
		AD_ENTRY3s(
			"MOUSECKE.EXE", "58350e268ec0cdf4fa21281a9d83fd80", 329568,
			"BOOT.STM", "11d11b2067519d8368175cc8e8caa94f", 59454,
			"100.CXT", "cac48b9bb5f327d035a831cd15f1688c", 1762032
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney's Animated Storybook: The Lion King
	{
		"lionking",
		"v2.0/GB",
		AD_ENTRY3s(
			"LIONKING.EXE", "3239451c477eaa16015110502be031a5", 363232,
			"BOOT.STM", "dd83fd1fb899b680f00c586404cc7b7c", 23610,
			"100.CXT", "d5dc4d49df2ea6f2ff0aa33a3f385506", 1455740
		),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Lamb Chop Loves Music
	{
		"lambchop",
		"v1.0",
		AD_ENTRY3s(
			"LCMUSIC.EXE", "1830080b410abd103c5064f583bdca1e", 329504,
			"BOOT.STM", "c90200e52bcaad52524520d461caef2b", 29884,
			"100.CXT", "ce40843604b8c52701694cd543072a88", 3253600
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Fractured Fairy Tales: The Frog Prince
	{
		"frogprince",
		"v1.1",
		AD_ENTRY3s(
			"FPRINCE.EXE", "cd7aff763bb4879cc3a11def90dd7cb7", 513984,
			"BOOT.STM", "1c6d14c87790d009702be8ba4e4e5906", 13652,
			"100.CXT", "a5ec9a32c3741a20b82e1793e76234b2", 1630762
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney's Animated Storybook: Winnie the Pooh and the Honey Tree
	{
		"honeytree",
		"v2.0/US", // Also includes Spanish as an in-game language option.
		AD_ENTRY3s(
			"WPHTASB.EXE", "916666c49efeeaeae61eb669405fc66f", 433024,
			"BOOT.STM", "9b9f528bf9c9b8ebe194b0c47dbe485e", 55422,
			"100.CXT", "30f010077fd0489933989a562db81ad6", 1971940
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney's Animated Storybook: The Hunchback of Notre Dame
	{
		"notredame",
		"v1.0/US",
		AD_ENTRY3s(
			"HB_ASB.EXE", "f3f2e83562d7941a99d299ae31600f07", 533120,
			"BOOT.STM", "7949e1253a62531e53963a2fffe57211", 55300,
			"100.CXT", "54c11a94888a1b747e1c8935b7315889", 4766278
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Puzzle Castle
	{
		"puzzlecastle",
		"v1.0",
		AD_ENTRY3s(
			"PZCASTLE.EXE", "ce44597dcbad42f2396d4963c06714d5", 528224,
			"BOOT.STM", "7b0faf38da2d76df40b4085eed6f4fc8", 22080,
			"100.CXT", "ebc4b6247b742733c81456dfd299aa55", 3346944
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"puzzlecastle",
		"v1.0 Demo",
		AD_ENTRY3s(
			"DEMO.EXE", "63dedc1e2cdf8a39725ef9ca99273cc4", 514496,
			"BOOT.STM", "b7ce005e0d67021f792ebb73e7fbe34c", 5960,
			"100.CXT", "cc64a6fcb3af2736d622658cff3ef2b5", 1262
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},

	// IBM/Crayola Print Factory
	{
		"ibmcrayola",
		"v1.0/US",
		AD_ENTRY3s(
			"PRINTFAC.EXE", "2571746dcb8b8d386f2ef07255e715ba", 721248,
			"BOOT.STM", "359542015c6665c70252cf21a8467cdb", 11044,
			"100.CXT", "42bffe4165640dd1e64a6e8565f48af3", 5125226
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// IBM/Crayola Print Factory Holiday Activity Pack
	{
		"ibmcrayolaholiday",
		"v1.0/US",
		AD_ENTRY3s(
			"HOLIDAY.EXE", "10b70a2cb94f92295d26f43540129f14", 742048,
			"BOOT.STM", "50f30298bf700f357d98c4390f75cb7a", 10932,
			"100.CXT", "8110f70f1d01d0f42cac9b1bb6d2de12", 4967390
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney's Animated Storybook: 101 Dalmatians
	{
		"101dalmatians",
		"v1.0/US",
		AD_ENTRY3s(
			"101_ASB.EXE", "42d7d258652bdc7ecd0e39e8b326bc38", 528736,
			"BOOT.STM", "ee6725a718cbce640d02acec2b84825f", 47970,
			"100.CXT", "2df853283a3fd2d079b06bc27b50527f", 6784502
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"101dalmatians",
		"v1.0/US",
		AD_ENTRY3s(
			"101 Dalmatians StoryBook", "1611f83747b3ac4dd33c8b866535e425", 1046272,
			"BOOT.STM", "ee6725a718cbce640d02acec2b84825f", 47970,
			"100.CXT", "2df853283a3fd2d079b06bc27b50527f", 6784502
		),
		Common::EN_USA,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney's Animated Storybook: Hercules
	{
		"herculesasb",
		"v1.0/US",
		AD_ENTRY3s(
			"HERC_ASB.EXE", "23663fabde2db43a2e8f6a23e7495e01", 543040,
			"BOOT.STM", "afc773416e46e30873f743e234794957", 26924,
			"100.CXT", "56875e1640320909e9697f11b5a8c9a6", 4895998
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Magic Fairy Tales: Barbie as Rapunzel
	{
		"barbieasrapunzel",
		"v1.0",
		AD_ENTRY3s(
			"RAPUNZEL.EXE", "e47a752fe748258ebc0f5ee6f31b385b", 535840,
			"BOOT.STM", "eef6bdf54d2ae25af0ec29361fd4c126", 17530,
			"100.CXT", "f0bcc27b61bfb33328db2dd537b2b6e3", 1688902
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Tonka Search and Rescue
	{
		"tonkasearchandrescue",
		"v1.0/US",
		AD_ENTRY3s(
			"TONKA_SR.EXE", "46037a28e0cbf6df9ed3218e58ee1ae2", 561984, // 32-bit (PE)
			"BOOT.STM", "90c5f17734219c3a442316d21e6833f8", 25362,
			"100.CXT", "85a05487b6c499ba3ce86d043305ddfd", 6410562
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Disney presents Ariel's Story Studio
	{
		"arielstorystudio",
		"v1.0/US",
		AD_ENTRY3s(
			"ARIEL_SS.EXE", "bb2afc5205a852e59d77631c454fde5d", 606720,
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
		"v1.1/US",
		AD_ENTRY3s(
			"MERMAID.EXE", "2eabe2910cf5a2df32dcc889ebd90cea", 634240,
			"BOOT.STM", "7d53a551efde620fe5b332d7b1f009ab", 65450,
			"100.CXT", "993252bca0aa6791ca3da30b1ae6f5f8", 6532022
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Tonka Garage
	{
		"tonkagarage",
		"v1.1/US",
		AD_ENTRY3s(
			"TONKA_GR.EXE", "4e7e75ac11c996454b334f9add38c691", 1297408,
			"BOOT.STM", "fc8863bb302e94d3b778b3a97556601b", 25208,
			"100.CXT", "13683c2a06275920181d9dda5b2b69e7", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// D.W. the Picky Eater (Living Books)
	{
		"dwpickyeater",
		"v1.0/US (32-bit)",
		AD_ENTRY3s(
			"DW_32.EXE", "3612aa19a2809f9cb6ee48046e5d7068", 1079296, // 32-bit (PE)
			"BOOT.STM", "80cc94e3e894ee8c5a22a9c07a33d891", 26402,
			"100.CXT", "e65e359ab25d7a639cf369a01b9a21c0", 2163750
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Tonka Workshop
	{
		"tonkaworkshop",
		"v1.0/US",
		AD_ENTRY3s(
			"TONKA_W.EXE", "f3e480c57967093b87db68cb8f3f3a18", 1097728, // 32-bit (PE)
			"BOOT.STM", "15e6d32925f557f3196fd0bb79b25375", 38190,
			"100.CXT", "1cb35998f2e044eee59a96120b3bda6c", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Tonka Raceway
	{
		"tonkaraceway",
		"v1.0/US",
		AD_ENTRY3s(
			"TONKA_RA.EXE", "cccd33d4d9e824bada6a1ca115794226", 1735680, // 32-bit (PE)
			"BOOT.STM", "da512cb9bcd18465294e544ed790881c", 12272,
			"100.CXT", "30802327b29fbfa722a707c3d3b0f8f8", 2691398
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Stuart Little: Big City Adventures
	{
		"stuartlittlebigcity",
		"v1.0/US",
		AD_ENTRY3s(
			"STUARTCD.EXE", "8aaa593c9a1a17a0e41f424d046b3de8", 1191936, // 32-bit (PE)
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
