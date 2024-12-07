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

namespace Dgds {

enum DgdsADGFFlags {
	ADGF_DGDS_EGA = 1,
};

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
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC) German
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
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC) German
	{
		"rise",
		0,
		{
			{"volume.vga", 0, "f4e9cdc69d3177e8eae81ab43ef2e490", 8992},
			{"volume.001", 0, "0ae4a3512c2fd9b90d1670ed7c351ad6", 1168096},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
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
		ADGF_DGDS_EGA,
		GUIO1(GUIO_NONE)
	},

	// Rise of the Dragon (PC) 16 Color, 720kB disks (reported by ns394, #15442)
	{
		"rise",
		"EGA",
		{
			{"volume.ega", 0, "3e4d62eb36fa5c8ca60e5cf59a307798", 6361},
			{"volume.001", 0, "69dd06efe5bb562f4e1b094c5d043d93", 524553},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DGDS_EGA,
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

	// Rise of the Dragon (Amiga) German (reported by gabberhead, #15453)
	{
		"rise",
		0,
		{
			{"volume.rmf", 0, "7737489a8c452f0c497956eead46e4f0", 8964},
			{"volume.001", 0, "4e693c2748ccd71a633b76abe2ed6f12", 525920},
		},
		Common::DE_DEU,
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

	// Adventures of Willy Beamish (GOG CD data)
	{
		"beamish",
		0,
		{
			{"resource.001", 0, "07eaebf5c9e569347308ff097bc6607c", 151525997},
			{"resource.rme", 0, "09bc7bcb83b6d036c5988c81a769cf0c", 44247},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish Demo
	{
		"beamish",
		0,
		{
			{"volume.001", 0, "8ef6966691086a5557b7c874db5b7ff4", 881872},
			{"volume.rmf", 0, "560ec4fc04184a5113c08eeb721ca2ce", 741},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Adventures of Willy Beamish Demo from Joystick Magazine 1995 September disk
	{
		"beamish",
		0,
		{
			{"volume.001", 0, "84bb888d47b535512847d49549d7c289", 947579},
			{"volume.rmf", 0, "924d02d0a767c3a3b91f9d32aa247669", 2492},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
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

	// Heart of China (PC) EGA 720kb Floppy version
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "4e61bfb1d323bf07ae9de7c37fcda0d0", 8208},
			{"volume.001", 0, "9de863e3cb1467fde7d91c02942432de", 342873},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DGDS_EGA,
		GUIO1(GUIO_NONE)
	},

	// Heart of China (PC) EGA 1.2MB Floppy version
	{
		"china",
		0,
		{
			{"volume.rmf", 0, "a1ffe0a1ba0efbb487f4b258ff860623", 8106},
			{"volume.001", 0, "cd535e57d3ece406a054c254a479906e", 867861},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DGDS_EGA,
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

	// Quarky And Quaysoo's Turbo Science
	{
		"quarky",
		0,
		{
			{"resource.map", 0, "639885b4a7c6b8e395b6ca969c94ffd9", 7595},
			{"resource.001", 0, "67a4ebacfa73d3f54e9c489d46f3e512", 1393208},
			{"resource.002", 0, "de8375a3cf6bf347a0d8604b871883d3", 1456460},
			{"resource.003", 0, "d22b65b330873828b18377a8276c071a", 1177135},
			{"turbosci.exe", 0, "a6258fec3119c49c315e9e778e057017", 279008},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
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

	// Johnny Castaway screensaver
	{
		"castaway",
		0,
		{
			{"resource.map", 0, "374e6d05c5e0acd88fb5af748948c899", 1461},
			{"resource.001", 0, "46acc7ab8f6d7f63838ffea5f87380e2", 1175645},
			{"scrantic.exe", 0, "34bd51a9697ce7ce833b90e189ef51b2", 295952},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Johnny Castaway screensaver
	{
		"castaway",
		0,
		{
			{"resource.map", 0, "cf6b8e845a5235b2294fba0da1174f09", 1453},
			{"resource.001", 0, "281ea06786017820969bd7baa0cc2905", 1175278},
			{"scrantic.exe", 0, "46ce1ae10fcfa52dd964fcfc86694509", 295952},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Dgds
