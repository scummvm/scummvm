/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Dgds {

static const ADGameDescription gameDescriptions[] = {
	// Rise of the Dragon (PC) GOG
	{
		"rise",
		0,
		{
			{"volume.vga", 0, "2d08870dbfeff4f5e06061dd277d666d", 8992},
			{"volume.001", 0, "5210b0a77f89bfa2544970d56b23f9e4", 1153936},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC)
	{
		"rise",
		0,
		{
			{"volume.vga", 0, "b0583c199614ed1c161a25398c5c7fba", 7823},
			{"volume.001", 0, "3483f61b9bf0023c00a7fc1b568a54fa", 769811},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC)
	{
		"rise",
		0,
		{
			{"volume.vga", 0, "59265803753cda6939901f41e324d004", 7823},
			{"volume.001", 0, "3483f61b9bf0023c00a7fc1b568a54fa", 776190},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC) 16 Color, 1.2MB disks
	{
		"rise",
		"EGA",
		{
			{"volume.ega", 0, "20508ad920355c00e14043f728163f80", 6163},
			{"volume.001", 0, "34fdc6addd1992d72d4f947af7905c75", 845289},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (Amiga)
	{
		"rise",
		0,
		{
			{"volume.rmf", 0, "44cd1ffdfeb385dcfcd60563e1036167", 8972},
			{"volume.001", 0, "71b0b4a623166dc4aeba9bd19d71697f", 519385},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (Macintosh)
	{
		"rise",
		0,
		{
			{"volume.rmf", 0, "fe8d0b0f68bb4068793f2ea438d28d97", 7079},
			{"volume.001", 0, "90b30eb275d468e21d308ca836a3d3b8", 1403672},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish (PC)
	{
		"beamish",
		0,
		{
			{"volume.rmf", 0, "c2be5cd4693dfcdbe45dd0e74dd5306d", 9896},
			{"volume.001", 0, "7e9f3b0b7a5ec9989d3149f5e1f011a9", 1263007},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish (PC)
	{
		"beamish",
		0,
		{
			{"volume.rmf", 0, "e5c3ddb2700fe4b1a33363ef6393e635", 9896},
			{"volume.001", 0, "48752f0f7798e81bc7f343c32325eaaa", 1317322},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish (Macintosh)
	{
		"beamish",
		0,
		{
			{"volume.rmf", 0, "a8cd2d95b9c972fd33bf22b6de0b50c8", 9832},
			{"volume.001", 0, "0849203c8da5f2b7868e11a77a537ee5", 1359359},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish (GOG FDD data)
	{
		"beamish",
		"FDD",
		{
			{"volume.rmf", 0, "d270e05a95d85dd4096a099d9471438f", 9943},
			{"volume.001", 0, "be08868abb909dcf24808676d063cba1", 1170013},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Heart of China (PC) GOG
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "94402b65f07606a2fb5591f9dc514c19", 10008},
			{"volume.001", 0, "26354d54b9f2e220620b0c1d31ed5a83", 1096322},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Heart of China (PC)
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "677b91bc6961824f1997c187292f174e", 9791},
			{"volume.001", 0, "3efe89a72940e85d2137162609b8b883", 851843},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Heart of China (PC)
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "203de252953ef96c0a6b83ccd137bc4a", 10032},
			{"volume.001", 0, "f3b05430e2fd64598fe9e7b422ae63e3", 1122957},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Heart of China (Macintosh)
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "6bc1730f371c7330333bed4c66fe7511", 9918},
			{"volume.001", 0, "bca16136f0fd36d25b1b1ba1870aa97f", 1240128},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// SQ5 demo
	{
		"sq5demo",
		0,
		{
			{"cesdemo.ads", 0, "8b5d56353aae62c69fe81a3ef80c3789", 2394},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// "Coming Attractions" non-interactive demo
	{
		"comingattractions",
		0,
		{
			{"demo.ads", 0, "bc709c5defe472f1ddc03db8cf6c83df", 94},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Dgds

