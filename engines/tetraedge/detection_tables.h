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

namespace Tetraedge {

const PlainGameDescriptor GAME_NAMES[] = {
	{ "amerzone", "Amerzone" },
	{ "syberia", "Syberia" },
	{ "syberia2", "Syberia II" },
	{ 0, 0 }
};

const ADGameDescription GAME_DESCRIPTIONS[] = {
	// Amerzone GOG release
	{
		"amerzone",
		nullptr,
		AD_ENTRY1s("MacOS/Amerzone", "d:cde4144aeea5a99602ee903554585178", 6380272),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO0()
	},

	// GOG and Steam releases
	// Note: Full sum of GOG and Steam are different,
	// but size and first 5000 bytes are the same.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("MacOS/Syberia", "d:6951fb8f71fe06f34684564625f73cd8", 10640592),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GAMEOPTION_CORRECT_MOVIE_ASPECT, GAMEOPTION_RESTORE_SCENES)
	},

	// iOS "free" release v1.1.3.  Not supported as we can't properly support
	// the in-app purchase to enable the full game.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("Syberia", "d:be658efbcf4541f56b656f92a05d271a", 15821120),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// iOS paid release v1.2.  Not yet tested.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("Syberia", "d:1425707556476013e859979562c5d753", 15794272),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_CORRECT_MOVIE_ASPECT, GAMEOPTION_RESTORE_SCENES)
	},

	// Nintendo Switch, from Syberia1-3 cartridge
	{
		"syberia",
	        nullptr,
		AD_ENTRY2s("InGame.lua", "acaf61504a12aebf3862648e04cf29aa", 3920,
			   "texts/de.xml", "14681ac50bbfa50427058d2793b415eb", (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformNintendoSwitch,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia2",
		nullptr,
		AD_ENTRY3s("Debug.lua", "a2ea493892e96bea64013819195c081e", 7024,
			   "InGame.lua", "a7df110fe816cb342574150c6f992964", 4654,
			   "texts/de.xml", "dabad822a917b1f87de8f09eadc3ec85", (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformNintendoSwitch,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// Nintendo Switch, from Syberia1+2 Online bundle, v0
	{
		"syberia",
	        nullptr,
		AD_ENTRY2s("InGame.lua", "ca319e6f014d04baaf1e77f13f89b44f", 4271,
			   "texts/de.xml", "14681ac50bbfa50427058d2793b415eb", (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformNintendoSwitch,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// Nintendo Switch, from Syberia1+2 Online bundle, v196608
	{
		"syberia",
	        nullptr,
		AD_ENTRY2s("InGame.lua", "ca319e6f014d04baaf1e77f13f89b44f", 4271,
			   "texts/de.xml", "17d7a875e81a7761d2b30698bd947c15", (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformNintendoSwitch,
		ADGF_NO_FLAGS | GF_UTF8,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia2",
		nullptr,
		AD_ENTRY3s("Debug.lua", "a2ea493892e96bea64013819195c081e", 7024,
			   "InGame.lua", "7d7fdb9005675618220e7cd8962c6482", 4745,
			   "texts/de.xml", "78ed3567b3621459229f39c03132e5bb", (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformNintendoSwitch,
		ADGF_NO_FLAGS | GF_UTF8,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// Android v1.0.5
	{
		"syberia",
		"Extracted",
		AD_ENTRY1s("InGame.lua", "12ee6a8eade070b905136cd4cdfc3726", 4471),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	{
		"syberia",
		nullptr,
		AD_ENTRY1s("main.12.com.microids.syberia.obb", "b82f9295c4bafe4af58450cbacfd261e", 1000659045),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// Buka release v1.0.3
	{
		"syberia",
		"Extracted",
		AD_ENTRY1s("InGame.lua", "6577b0151ca4532e94a63a91c22a17c1", 2646),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("main.2.ru.buka.syberia1.obb", "7af875e74acfceee5d9b78c705da212e", 771058907),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// v1.0.1
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("main.5.com.microids.syberia.obb", "6a39b40edca885bb9508ec09675c1923", 1389534445),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia",
		"Extracted",
		AD_ENTRY1s("InGame.lua", "8698770015e103725db60a65f3e21657", 2478),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	{
		"syberia",
	        nullptr,
		AD_ENTRY1s("InGame.data", "5cb78f2c8aac837fe53596ecfe921b38", 2195),
		Common::UNK_LANG,
		Common::kPlatformPS3,
		ADGF_UNSUPPORTED,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// v1.0.2 Buka release
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("main.2.ru.buka.syberia2.obb", "e9d8516610d33f375a3f6800232e3224", 1038859725),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia2",
		"Extracted",
		AD_ENTRY2s("Debug.lua", "a2ea493892e96bea64013819195c081e", 7024,
			   "filelist.bin", "eb189789a74286c5023e102ec1c44fd4", 2099822),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// v1.0.1 Android release
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("main.4.com.microids.syberia2.obb", "d8aa60562ffad83d3bcaa7b611fc4299", 1473221971),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},
	{
		"syberia2",
		"Extracted",
		AD_ENTRY2s("Debug.lua", "a2ea493892e96bea64013819195c081e", 7024,
			   "filelist.bin", "dc40f150ee291a30e0bc6cd8a0127aab", 2100007),
		Common::UNK_LANG,
		Common::kPlatformAndroid,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// v1.0.0 iOS release
	{
		"syberia2",
		nullptr,
		AD_ENTRY2s("Debug.lua", "a2ea493892e96bea64013819195c081e", 7024,
			   "Info.plist", nullptr, (uint32_t)-1),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},


	// GOG release
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("MacOS/Syberia 2", "d:c447586a3cb3d46d6127b467e7fb9a86", 12021136),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// iOS release v1.0.1.  Not yet tested.
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("Syberia 2", "d:17d0ded9b87b5096207117bf0cfb5138", 15881248),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSTABLE,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("Debug.data", "d5cfcba9b725e746df39109e7e1b0564", 7024),
		Common::UNK_LANG,
		Common::kPlatformPS3,
		ADGF_UNSUPPORTED,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	AD_TABLE_END_MARKER
};

} // namespace Tetraedge
