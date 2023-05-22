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

#include "freescape/freescape.h"
#include "freescape/detection.h"

namespace Freescape {

static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "3D Kit Game"},
	{"driller", "Driller"},
	{"spacestationoblivion", "Space Station Oblivion"},
	{"darkside", "Dark Side"},
	{"totaleclipse", "Total Eclipse"},
	{"castlemaster", "Castle Master"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	// Original Freescape games
	{
		"driller",
		"",
		{
			{"DRILLER.ZX.DATA", 0, "396c1789a7da3db5058d18eb8d2d35a3", 37590},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_RETAIL,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"DRILLER.ZX.DATA", 0, "6876fc03e754137f428bd4d5f16452b5", 37888},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_DISC,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"DRILLER.ZX.DATA", 0, "2b996ab877f45414f8e2ae4c862746f3", 35214},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_BUDGET,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Not implemented yet",
		{
			{"DRILLER.ZX.DATA", 0, "e571795806ed8a30df0fa3109eaa8ffb", 36000},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"DRILL.BIN", 0, "719f5157391e88b2c391c30576340637", 35320},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		GF_CPC_RETAIL,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		{
			{"SPACESTATIONOBLIVION.C64.DATA", 0, "85680576865e211f868885e9997a08b8", 22782},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},

	{
		"spacestationoblivion",
		"",
		{
			{"SPACESTATIONOBLIVION.C64.DATA", 0, "2b3537e21b8b871ec074df2962999781", 64514},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller", // Commodore Force - Jan 94
		"",
		{
			{"DRILLER.C64.DATA", 0, "511778d3167ff7504d905df507a03ac5", 63490},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"DRILLER.C64.DATA", 0, "73a6f206e54fb13245fe6d92f60fbb34", 41071},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"DRILLER.EXE", 0, "cafc0ea0d3424640a7723af87f8bfc0b", 17427},
			{"DRILLE.EXE", 0, "eb7e9e0acb72e30cf6e9ed20a6480e7a", 51944},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO4(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"x.prg", 0, "1a79e68e6c2c223c96de0ca2d65149ae", 293062},
			{"playseq.prg", 0, "535e9f6baf132831aa7fa066a06f242e", 973},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"driller", 0, "13dab2e10d8e8b9a364c94a660e0d42a", 282384},
			{"title.seq", 0, "4dd1b3b45110b24e8240a6132241c973", 185296},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		GF_AMIGA_RETAIL,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		{
			{"driller", 0, "db1afe151d999f369ae9153d8eeaf254", 175236},
			{"soundfx", 0, "cd91061a1330aef8fcd6b7dc6fa35cf9", 78680},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		GF_AMIGA_BUDGET,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Rolling Demo",
		{
			{"drilldem.com", 0, "5188a7ae06f72263634a107685f52cbe", 889},
			{"d1", 0, "edd57153197ef457f08eeea4d9339208", 16384},
			{"d2", 0, "3690e6461bfc5434bab8969c6cdbb297", 41488},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Rolling Demo",
		{
			{"driller", 0, "f0d0e9447830e329e110e384371955fd", 162880},
			{"data", 0, "0b056286d2d91f302499c97aca235462", 24220},
			{"demo.cmd", 0, "7b90427c5c3c4bbb42d14076af994d50", 4096},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Rolling Demo",
		{
			{"x.prg", 0, "cf96e25a11bee1b57258c2fc0b315699", 157143},
			{"data", 0, "0b056286d2d91f302499c97aca235462", 24220},
			{"demo.cmd", 0, "7b90427c5c3c4bbb42d14076af994d50", 4096},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Packed data",
		{
			{"driller.prg", 0, "ae9b03e247def6f0793174b1cb4352b5", 1821},
			{"data", 0, "0c927fbc6c390afd0d0c15b2d7f8766f", 10893},
			{"demo.cmd", 0, "9c732dcdad26b36b537e632924cd8f0e", 745},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"darkside",
		"",
		{
			{"0.DRK", 0, "9e51b8f93a9af886fb88ab92ed43cf01", 81544},
			{"1.DRK", 0, "50bbaa2b19fc4072ad85efe93116e561", 280704},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"darkside",
		"",
		{
			{"DARKSIDE.EXE", 0, "c6c0d0186ec45e6cecd72bf5550c7f98", 1600},
			{"DSIDEC.EXE", 0, "31e6c169d9270b6de8c1c2e746ac238e", 49504},
			{"DSIDEH.EXE", 0, "5e18e0746647bd04f43b9db24b6a036d", 53232},
			{"DSIDEE.EXE", 0, "524281f7d2dc49e0a41fcb1d38ee2559", 56800},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"darkside",
		"Demo",
		{
			{"DARKSIDE.EXE", 0, "5c69b7c5586031c153cbb58fc6d2bff8", 20905},
			{"DSIDEC.EXE", 0, "796c1d1a9583a0ccf5bba47813fa6a8a", 39640},
			{"DSIDEE.EXE", 0, "1f3f607dad97efe1d8e979ef588bda01", 46952},
			{"DSIDEH.EXE", 0, "7f45c6047798c5443f57c2002e2d2c27", 43368},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"darkside",
		"Demo",
		{
			{"DARKSIDE.ZX.DATA", 0, "0e4d9b6e64ff24801272ff0b18a3caab", 29182},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"totaleclipse",
		"",
		{
			{"TOTAL.EXE", 0, "959703c1cd37b0d9744c492240a8178b", 13327},
			{"TOTEH.EXE", 0, "c68d59874ab2a93cc9cc1b1d3aed8f17", 60628},
			{"TOTEC.EXE", 0, "6c058c48255945a12479c8420321db75", 56900},
			{"TOTET.EXE", 0, "2bc2b0892ca91a77c58e9bedabf45afe", 63732},
			{"TOTEE.EXE", 0, "2ba865d2bdcda0c934baec7b31fb4ab2", 64196},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"spacestationoblivion",
		"",
		{
			{"OBLIVION.EXE", 0, "80783622013750d7c88fd1d35dde919a", 6765},
			{"DRILLE.EXE", 0, "30edf6be0037b2b0e8c6957df62c2a02", 51944},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO4(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		{
			{"x.prg", 0, "10c556ee637bf03bcc1a051277542102", 293264},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"castlemaster",
		"Demo",
		{
			{"CASTLE.EXE", 0, "cbcf475b2d993c831a34a5203d2643e1", 2022},
			{"CMDC.EXE", 0, "278fd1a96c61db71d952af472164ac57", 56526},
			{"CMDE.EXE", 0, "428555ba83bc64d69bc2f7cb385f04f2", 88590},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"castlemaster",
		"Demo",
		{
			{"cm", 0, "936b6ca92be53f122bd904a3397137e2", 1552},
			{"x", 0, "c8c811439da0cf8a193e35feb5b5c6dc", 353388},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	// Stampede Amiga, Issue 1, July 1990
	{
		"castlemaster",
		"Demo",
		{
			{"cm", 0, "b7e713a0742fa09aa81c9606bbbba4af", 4068},
			{"x", 0, "c8c811439da0cf8a193e35feb5b5c6dc", 353388},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"castlemaster",
		"",
		{
			{"CASTLE.EXE", 0, "f1a141df0e47860246716db20d2ba061", 2806},
			{"CMC.EXE", 0, "03af2b79b1aad690684cf89025c5f425", 60240},
			{"CMT.EXE", 0, "a603d3f96e981ab0014b7b1a5a2dc28c", 81840},
			{"CME.EXE", 0, "99d8b4dbaad1fd73c9afdde550dc5195", 92320},
			{"CMH.EXE", 0, "1f3b67e649e718e239ebfd7c56e96d47", 63040},
			AD_LISTEND
		},
		Common::UNK_LANG,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"castlemaster",
		"Virtual Words",
		{
			{"CASTLE.EXE", 0, "f1a141df0e47860246716db20d2ba061", 2806},
			{"CMC.EXE", 0, "7b9275df446f82fdd0c377f6ec2db546", 57168},
			{"CMT.EXE", 0, "5814e68a175f74ebce0773a73e7488c7", 78768},
			{"CME.EXE", 0, "d563ae1475752e6a9a81b1350abebef3", 89248},
			{"CMH.EXE", 0, "26337adc7861300e5395e992e42b6329", 59968},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"castlemaster",
		"Domark PC Collection",
		{
			{"X.EXE", 0, "d51d7db1e06814cbf763c43f411df616", 2157},
			{"CMC.EXE", 0, "7b9275df446f82fdd0c377f6ec2db546", 57168},
			{"CMT.EXE", 0, "78002e4b6c14e0a7924317d27e868985", 40685},
			{"CME.EXE", 0, "494268dea6d8580a7f27afb2a8157cc0", 46557},
			{"CMH.EXE", 0, "03b6f4c5b8931259e42e229de06ac5fc", 35645},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"castlemaster",
		"",
		{
			{"CASTLE.EXE", 0, "42a7d46b418d68e75e31c1cb9d89af14", 2678},
			{"CMC.EXE", 0, "9015c244dc8a97fe55df7b235b31e00c", 57168},
			{"CMT.EXE", 0, "5814e68a175f74ebce0773a73e7488c7", 78768},
			{"CME.EXE", 0, "d563ae1475752e6a9a81b1350abebef3", 89248},
			{"CMH.EXE", 0, "cc68c42d254b3aa0f208cd08731c6805", 59968},
			AD_LISTEND
		},
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// 3D Construction Kit games
	{
		"3dkit",
		"The 3-D Kit Game",
		AD_ENTRY1s("3DKIT.RUN", "f35147729a2f5b6852a504223aeb6a57", 112158),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"3D House",
		AD_ENTRY1s("3DHOUSE.RUN", "3d3320e71831ce533a7107fdd2cf5be3", 100816),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"3D House v2",
		AD_ENTRY1s("3DHOUSE2.RUN", "136f5285bf198967934d3d0b45a6a29b", 100816),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"3D Trek",
		AD_ENTRY1s("3DTREK.RUN", "6a5785fbbfca62d194fc426a6d5d20fd", 99758),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Adam's Game v1.2",
		AD_ENTRY1s("ADAM.RUN", "1420a0beefb51495e18630bd71b35765", 85850),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"B-Dream",
		AD_ENTRY1s("BDREAM.RUN", "f373fd71fda0c460c09329631cf6189f", 97194),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Desert Sands v1.3",
		AD_ENTRY1s("SANDS.RUN", "de4f0391d43ce48b0bb6f589dfe470f5", 73964),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"EarthNap",
		AD_ENTRY1s("EARTHNAP.RUN", "57cbd38e200ff256f4dba5207ec5abb6", 101158),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Eerie Estates",
		AD_ENTRY1s("EERIE.RUN", "5455621ccac85393579aa71670d9d9d8", 105314),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Gaynor Ave. House",
		AD_ENTRY1s("GAYNOR4.RUN", "06b38e2cffc1218a74a59bffbb8bc6c5", 85730),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"The House",
		AD_ENTRY1s("THEHOUSE.RUN", "f8aae4ddd56599a40392911df4cc5e63", 96282),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Lost",
		AD_ENTRY1s("LOST.RUN", "3c8a84f3c5eba28cad6de6edfa6f9afb", 86004),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Menace of Dr. Spoil Sport",
		AD_ENTRY1s("MODSS.RUN", "409ac1100a15447e742ec1415b2741c3", 91176),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Mino Dino Demo",
		AD_ENTRY1s("DINODEMO.RUN", "9da4c0e3940a0d2735bfcb589ec50001", 73526),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Rubber Room v1.0",
		AD_ENTRY1s("RUBROOM.RUN", "81b31463957a2951fd356010bdaba520", 71830),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Tour Graceland v1.0",
		AD_ENTRY1s("BINGO.RUN", "10ed84e92bf71d147dc86f7c06b7c0f4", 115260),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Virtual Reality Studio Demo",
		AD_ENTRY1s("DEMO.RUN", "73c56a77ce637673d5f83bb5909cbc26", 95746),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},

	// 3D Construction Kit II games
	{
		"3dkit",
		"3D House v4",
		AD_ENTRY2s("3DHOUSE4.EXE", "49f7a832104ae76d38851d1b4fd8f0a1", 8859,
				   "RUNNER.3WD", "cc0bfa95f695d2b7eadc1d6f3dd6e8f9", 62434),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"Anarchy Academy",
		AD_ENTRY2s("ANARCHY.PRG", "78d543be4aad9608093c1e91bc100307", 270,
				   "RUNNER.DAT", "1cf7c049ee59503dd7218b0f45828132", 42802),
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"3dkit",
		"The World of Stairs",
		AD_ENTRY2s("RUNNER.EXE", "8d20d849694fb098b5fa7883f770a122", 96439,
				   "RUNNER.3WD", "daf373f23a122dd1b7eda48f33d3aa54", 44694),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER};
} // End of namespace Freescape

static const DebugChannelDef debugFlagList[] = {
	{Freescape::kFreescapeDebugMove, "move", ""},
	{Freescape::kFreescapeDebugParser, "parser", ""},
	{Freescape::kFreescapeDebugCode, "code", ""},
	{Freescape::kFreescapeDebugMedia, "media", ""},
	DEBUG_CHANNEL_END
};

class FreescapeMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	FreescapeMetaEngineDetection() : AdvancedMetaEngineDetection(Freescape::gameDescriptions, sizeof(ADGameDescription), Freescape::freescapeGames) {
		_guiOptions = GUIO6(GUIO_NOMIDI, GAMEOPTION_PRERECORDED_SOUNDS, GAMEOPTION_EXTENDED_TIMER, GAMEOPTION_DISABLE_DEMO_MODE, GAMEOPTION_DISABLE_SENSORS, GAMEOPTION_DISABLE_FALLING);
	}

	const char *getName() const override {
		return "freescape";
	}

	const char *getEngineName() const override {
		return "Freescape";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) 1987 Incentive Software";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override;
};

DetectedGame FreescapeMetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
	DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame);

	// The AdvancedDetector model only allows specifying a single supported game language.
	if (game.gameId == "castlemaster" && game.language == Common::UNK_LANG) {
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::FR_FRA));
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::DE_DEU));
	}
	return game;
}

REGISTER_PLUGIN_STATIC(FREESCAPE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, FreescapeMetaEngineDetection);
