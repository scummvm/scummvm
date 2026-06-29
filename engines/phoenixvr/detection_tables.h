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

#include "advancedDetector.h"
namespace PhoenixVR {

// clang-format off

const PlainGameDescriptor phoenixvrGames[] = {
	{"necrono", "Necronomicon: The Dawning of Darkness"},
	{"lochness", "The Cameron Files: The Secret at Loch Ness"},
	{"messenger", "The Messenger/Louvre: The Final Curse"},
	{"dracula1", "Dracula: Resurrection"},
	{"dracula2", "Dracula 2: The Last Sanctuary"},
	{"amerzone", "Amerzone: The Explorer's Legacy"},
	{0, 0}
};

const ADGameDescription gameDescriptions[] = {
	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "f795f35b079cb8ef599724a2a7336c7e", 5319),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

    {"necrono",
		nullptr,
		AD_ENTRY4s("script.pak", "da42a18dd02fc01f116228d5c219b2fd", 215,
				   "textes.txt", "f795f35b079cb8ef599724a2a7336c7e", 5319,
				   "cd1/Data/Script1.pak", "626cac1db5160142313fdcf483fda2bf", 57410,
				   "cd2/Data/Script3.pak", "4f122b65627903da15b3f15156252c8d", 63979),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD,
		GUIO1(GUIO_NONE)
	},

    {"necrono",
		nullptr,
		AD_ENTRY4s("script.pak", "da42a18dd02fc01f116228d5c219b2fd", 215,
				   "textes.txt", "c3af2d55b4fa55d200b047b6ec0d9f73", 5507,
				   "cd1/Data/Script1.pak", "626cac1db5160142313fdcf483fda2bf", 57410,
				   "cd2/Data/Script3.pak", "4252ea1b2458264e8d2a3afb67242820", 64012),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "542b626e7d56e4b3b5a73616e772a246", 5503),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "8a6fde4fbe90262cad639903a0469863", 5394),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		nullptr,
		AD_ENTRY2s("script.pak", "da42a18dd02fc01f116228d5c219b2fd", 215,
				   "textes.txt", "ab8efb7f5e92d2b76863c181bf2dbd10", 4959),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "c3af2d55b4fa55d200b047b6ec0d9f73", 5507),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "e54b9e37d3b96b52f78e7ca266116c4b", 5423),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"necrono",
		"GOG/Steam release",
		AD_ENTRY2s("script.pak", "86294b9c445c3e06e24269c84036a207", 223,
				   "textes.txt", "4a0055fb933c8f044a7fa1a321eac7db", 5265),
		Common::PT_PRT,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "294efb30581661615359ce234e2e85fb", 1596),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)}
	,

	{"lochness",
		nullptr,
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "d1546d04243ee63f9ff6c5fc551082e1", 1763),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD,
		GUIO1(GUIO_NONE)}
	,

	{"lochness",
		nullptr,
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "d1546d04243ee63f9ff6c5fc551082e1", 1763),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "f1d44e0b71736e5b0e2516dbfe9bf7e3", 1770),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "b38570cda689d7994806635e0e34ed7f", 1825),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "640d6d3f53986b73f97e1eaad700093a", 1732),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "4474a0c84b6e7363e1d65cc67278b923", 1769),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"lochness",
		"Steam release",
		AD_ENTRY2s("script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
				   "textes.txt", "5ea7264941b31b5961860c7016537641", 1776),
		Common::PT_PRT,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	{"messenger",
		nullptr,
		AD_ENTRY5s("script.pak", "1e0f9cb47bc203e9e2983b03ffa85174", 185,
				   "textes.txt", "23f577d1201bc3024ca49cb11f9f7347", 5261,
				   "cd1/Data/interface.vr", "110c55142ec95dcbdc0dd86a68013d2a", 188422,
				   "cd1/Data/Script1.pak", "e0cf6ed93ea338dc91d639c6b70b75c1", 54920,
				   "cd2/Data/Script2.pak", "26fe454a74da294ff622cdc5663f2c8c", 82085),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{"messenger",
		nullptr,
		AD_ENTRY4s("script.pak", "0c9c4b2bfa3b1399b89f6b62063a2e80", 210,
				   "cd1/Data/interface.vr", "c128d4c10122ae50ad50e2d3c4ccfbc5", 218081,
				   "cd1/Data/Script.pak", "b8bf0acf2202f1929ed34305346303d9", 54752,
				   "cd2/Data/Script2.pak", "01bccbbb33e901c9addfa0de51d1bea3", 81955),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{"messenger",
		nullptr,
		AD_ENTRY4s("script.pak", "0c9c4b2bfa3b1399b89f6b62063a2e80", 210,
				   "cd1/Data/interface.vr", "a11ca0314af1d863bbd289e3af4a52c9", 214009,
				   "cd1/Data/Script.pak", "62980f95be48448d738747a3cac9b2b2", 54944,
				   "cd2/Data/Script2.pak", "ea95c1be10f24c4ad5ad4cb4ad130699", 82104),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_CD | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"GOG release",
		AD_ENTRY2s("script.lst", "78060b78cf403ddb7e22903ba7b269d6", 548,
				   "Interface.vr", "b51691a530ec7398bed8c0c094885225", 85245),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		nullptr,
		AD_ENTRY2s("script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Interface.vr", "b51691a530ec7398bed8c0c094885225", 85245),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"EU release",
		AD_ENTRY3s("Install/script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Data/Logo.vr", "d098ec163dcf6aff866300b1319913be", 39542,
				   "Interface.vr", "b51691a530ec7398bed8c0c094885225", 85245),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"1.1 Dreamcatcher",
		AD_ENTRY3s("Install/script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Data/Logo.vr", "43f7b756cb11c691d0eb21003af64e90", 50366,
				   "Interface.vr", "b51691a530ec7398bed8c0c094885225", 85245),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"GOG release",
		AD_ENTRY2s("script.lst", "78060b78cf403ddb7e22903ba7b269d6", 548,
				   "Interface.vr", "70362765ee83bb822d5bb08948317f23", 98844),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"GOG release",
		AD_ENTRY2s("script.lst", "78060b78cf403ddb7e22903ba7b269d6", 548,
				   "Interface.vr", "195a7b2b6185245b64145f3e43e4f508", 90805),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		nullptr,
		AD_ENTRY2s("script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Interface.vr", "5d7d770a74d6035957c7deffc4865e12", 106762),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"GOG release",
		AD_ENTRY2s("script.lst", "78060b78cf403ddb7e22903ba7b269d6", 548,
				   "Interface.vr", "5d7d770a74d6035957c7deffc4865e12", 106762),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"Retail version",
		AD_ENTRY3s("Install/script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Install/InsertCD.pak", "f2a35e531a4ef5895e972e3c98330752", 499,
				   "Interface.vr", "5d7d770a74d6035957c7deffc4865e12", 106762),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"Retail version",
		AD_ENTRY2s("Install/script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Interface.vr", "47099471f31f794be8a7e5a3c382bf62", 104304),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"GOG release",
		AD_ENTRY2s("script.lst", "78060b78cf403ddb7e22903ba7b269d6", 548,
				   "Interface.vr", "47099471f31f794be8a7e5a3c382bf62", 104304),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula1",
		"7 Wolf",
		AD_ENTRY2s("Install/script.pak", "6998262fcce9cb5d3cc8a555f8ee024a", 265,
				   "Interface.vr", "61c68db8bb7a9e22387d3f27fb24bad4", 107904),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"GOG release",
		AD_ENTRY3s("script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "InsertCD.vr", "54d1ea71f27a849a4a088cf21ca64aff", 53817),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"Retail version",
		AD_ENTRY4s("Install/script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Install/Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "Install/InsertCD.vr", "54d1ea71f27a849a4a088cf21ca64aff", 53817,
				   "First.exe", "403417a28a11a08deb8443dc08a2213c", 31770),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"GOG release",
		AD_ENTRY3s("script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "InsertCD.vr", "0bc196a016845e99d99a0bc634f51e9d", 53670),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"GOG release",
		AD_ENTRY3s("script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "InsertCD.vr", "699f7519cc1caa740fe27434f6ca9766", 54394),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"GOG release",
		AD_ENTRY3s("script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "InsertCD.vr", "21b1cb64d71010799368a8e7097c133b", 54210),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"GOG release",
		AD_ENTRY3s("script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "InsertCD.vr", "2d5d17124caeebad668661abaff0c8fc", 54442),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"Retail version",
		AD_ENTRY4s("Install/script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Install/Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "Install/InsertCD.vr", "be865f78d00c699e55e6e3bd2e83df26", 52983,
				   "First.exe", "4b106f5fffb4b3c033edcdaa0893d616", 16384),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"Retail version",
		AD_ENTRY4s("Install/script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Install/Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "Install/InsertCD.vr", "7f54847d09c772d41c8726d5c5529a9a", 54005,
				   "First.exe", "403417a28a11a08deb8443dc08a2213c", 31770),
		Common::PT_PRT,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	{"dracula2",
		"Russobit-M",
		AD_ENTRY3s("Install/script.pak", "ff52d2000eddc5c438f3c8ef50fea858", 277,
				   "Install/Compiler.dat", "ff30c7ea065af1e86182b428e1f2cc7b", 42650,
				   "Install/InsertCD.vr", "bd73f26480428507118d9a9b7a047f4a", 53851),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// GOG release
	{"amerzone",
		"GOG release",
		AD_ENTRY3s("insertCD.vr", "715f238ae8265eebac84993715c93cf4", 75463,
				   "01VR_PHARE/INTERFACE.VR", "7852b9cb6edb918fd2dfcddd859ef09a", 73495,
				   "01VR_PHARE/AMERZONE.LST", "85da49d770959e6db34a7e2216f8b525", 163076
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{"amerzone",
		"USA 4 CDs release",
		AD_ENTRY3s("insertCD.vr", "924dcfe4417514d31e11225df2b5220d", 327976,
				   "01VR_PHARE/INTERFACE.VR", "7852b9cb6edb918fd2dfcddd859ef09a", 73495,
				   "01VR_PHARE/AMERZONE.PAK", "2f538af7bdb1492c883a7333a6fb4141", 26404
		),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_UNSTABLE | ADGF_CD,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};
// clang-format on

} // End of namespace PhoenixVR
