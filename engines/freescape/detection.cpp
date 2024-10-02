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

#include "freescape/games/driller/driller.h"
#include "freescape/games/eclipse/eclipse.h"

namespace Freescape {

static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "3D Kit Game"},
	{"driller", "Driller"},
	{"spacestationoblivion", "Space Station Oblivion"},
	{"darkside", "Dark Side"},
	{"totaleclipse", "Total Eclipse"},
	{"totaleclipse2", "Total Eclipse 2"},
	{"castlemaster", "Castle Master"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	// Original Freescape games
	// Driller
	{
		"driller",
		"",
		AD_ENTRY1s("DRILLER.ZX.DATA", "396c1789a7da3db5058d18eb8d2d35a3", 37590),
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_RETAIL,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERZX, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		AD_ENTRY1s("DRILLER.ZX.DATA", "6876fc03e754137f428bd4d5f16452b5", 37888),
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_DISC,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERZX, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		AD_ENTRY1s("DRILLER.ZX.DATA", "2b996ab877f45414f8e2ae4c862746f3", 35214),
		Common::EN_ANY,
		Common::kPlatformZX,
		GF_ZX_BUDGET,
		GUIO3(GUIO_NOMIDI,  GUIO_RENDERZX, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Not implemented yet",
		AD_ENTRY1s("DRILLER.ZX.DATA", "e571795806ed8a30df0fa3109eaa8ffb", 36000),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO3(GUIO_NOMIDI,  GUIO_RENDERZX, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		AD_ENTRY1s("DRILL.BIN", "719f5157391e88b2c391c30576340637", 35320),
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		GF_CPC_RETAIL,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERCPC, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller", // Commodore Force - Jan 94
		"",
		AD_ENTRY1s("DRILLER.C64.DATA", "511778d3167ff7504d905df507a03ac5", 63490),
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"",
		AD_ENTRY1s("DRILLER.C64.DATA", "73a6f206e54fb13245fe6d92f60fbb34", 41071),
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
			{"DRILLC.EXE", 0, "908dd1f8732ebcbaece7d8f0cffd8830", 43864},
			{"DRILLH.EXE", 0, "033d2d45390886d0bff78cf53e83b6ed", 47496},
			{"DRILLT.EXE", 0, "afce0e5e0ad8c508e0c31c3e0b18b9a5", 51096},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO5(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA, GUIO_RENDERHERCGREEN, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{ // Retail release
		"driller",
		"",
		AD_ENTRY1s("x.prg", "d002983a8b652f25b18a09425db78c4c", 293159),
		Common::EN_ANY,
		Common::kPlatformAtariST,
		GF_ATARI_RETAIL,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{ // Budget release
		"driller",
		"",
		AD_ENTRY1s("x.prg", "1a79e68e6c2c223c96de0ca2d65149ae", 293062),
		Common::EN_ANY,
		Common::kPlatformAtariST,
		GF_ATARI_BUDGET,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{ // Virtual Worlds release
		"driller",
		"This release requires unpacking, check the wiki for instructions: https://wiki.scummvm.org/index.php?title=Driller#AtariST_releases",
		{
			{"d.pak", 0, "607b44b9d31e0da5668b653e03d25efe", 706},
			{"dril.all", 0, "65277222effa1eb4d73b234245001d75", 158158},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
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
		GUIO3(GUIO_NOMIDI, GUIO_RENDERAMIGA, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{ // Virtual Worlds release
		"driller",
		"",
		{
			{"driller", 0, "45312c9ccdb567c332f953059e4819a5", 282384},
			{"text", 0, "bf949a753634cdcb376e0dab9447b08b", 215},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		GF_AMIGA_RETAIL,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERAMIGA, GAMEOPTION_AUTOMATIC_DRILLING)
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
		GUIO3(GUIO_NOMIDI, GUIO_RENDERAMIGA, GAMEOPTION_AUTOMATIC_DRILLING)
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
		GUIO3(GUIO_NOMIDI, GUIO_RENDERCGA, GAMEOPTION_AUTOMATIC_DRILLING)
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
		GUIO3(GUIO_NOMIDI, GUIO_RENDERAMIGA, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Demo",
		{
			{"driller", 0, "ff9b84a6526de87a301f7aba5c2d87d9", 164736},
			{"data", 0, "6add87a10042d0a9fd32c62ed56b7690", 24220},
			{"demo.cmd", 0, "247601bec1bfa858955749364c876609", 4096},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_DEMO | GF_AMIGA_MAGAZINE_DEMO,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERAMIGA, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"Demo",
		{
			{"auto_x.prg", 0, "14388ba3cfd222938e5e48f0e316b335", 158919},
			{"data", 0, "6add87a10042d0a9fd32c62ed56b7690", 24220},
			{"demo.cmd", 0, "7b90427c5c3c4bbb42d14076af994d50", 4096},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_DEMO | GF_ATARI_MAGAZINE_DEMO,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
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
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"driller",
		"This demo requires unpacking, check the wiki for instructions: https://wiki.scummvm.org/index.php?title=Driller/Demos#AtariST_demos",
		{
			{"driller.prg", 0, "ae9b03e247def6f0793174b1cb4352b5", 1821},
			{"data", 0, "0c927fbc6c390afd0d0c15b2d7f8766f", 10893},
			{"demo.cmd", 0, "9c732dcdad26b36b537e632924cd8f0e", 745},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO3(GUIO_NOMIDI, GUIO_RENDERATARIST, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	// Space station oblivion
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
		AD_ENTRY1s("x.prg", "bf546ee243c38f51d9beb25c203ccb93", 292624),
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		AD_ENTRY1s("x.prg", "7fc6ef316be4819d88d8031738571a50", 293264),
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		AD_ENTRY1s("oblivion", "7fc6ef316be4819d88d8031738571a50", 293264),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		AD_ENTRY1s("SPACESTATIONOBLIVION.C64.DATA", "85680576865e211f868885e9997a08b8", 22782),
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},
	{
		"spacestationoblivion",
		"",
		AD_ENTRY1s("SPACESTATIONOBLIVION.C64.DATA", "2b3537e21b8b871ec074df2962999781", 64514),
		Common::EN_ANY,
		Common::kPlatformC64,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_AUTOMATIC_DRILLING)
	},

	// Dark Side
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
		GUIO2(GUIO_NOMIDI, GUIO_RENDERAMIGA)
	},
	// Microstatus release decrypted
	{
		"darkside",
		"",
		{
			{"0.DRK", 0, "5aa6473a75991f1e1217974a7844a042", 77900},
			{"1.DRK", 0, "49cc73a5ca7adb26065286a7afd4d0a4", 280704},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},
	// Cinemaware release
	{
		"darkside",
		"",
		{
			{"0.DRK", 0, "e237ae052a85ff92e70bd3528442e1e4", 78298},
			{"1.DRK", 0, "9e700d991054f4393b1ccef706586b5b", 223272},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},
	// Microstatus release
	{
		"darkside",
		"Encrypted executable with CopyLock, follow the instructions here: https://wiki.scummvm.org/index.php?title=Freescape/DRM",
		{
			{"0.DRK", 0, "c19bc332f6550e21e7b8ef79bcf3d99e", 81840},
			{"1.DRK", 0, "9e700d991054f4393b1ccef706586b5b", 223272},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},
	{ // Stampede AtariST, Issue 1
		"darkside",
		"",
		{
			{"0.DRK", 0, "86794a6cb2d371cdf892a2173e3b7568", 47966},
			{"1.DRK", 0, "9e700d991054f4393b1ccef706586b5b", 223272},
  			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},
	// Stampede Amiga, Issue 1, July 1990
	{
		"darkside",
		"",
		{
			{"DARKSIDE", 0, "5cf22e88959284097790189cf05b1862", 84004},
			{"1.DRK", 0, "49cc73a5ca7adb26065286a7afd4d0a4", 280704},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERAMIGA)
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
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"darkside",
		"",
		{
			{"DARKSIDE.EXE", 0, "477c8249665c00e001a84f94911b5960", 21325},
			{"DSIDEC.EXE", 0, "d4d443777411d6141e94bbf464914251", 49504},
			{"DSIDEE.EXE", 0, "9d9ba99a8b7c1ae49f9cc328bfa862dd", 56800},
			{"DSIDEH.EXE", 0, "7764dadc5a0baf0960e9323a8629e321", 53232},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"darkside",
		"",
		{
			{"DARKSIDE.EXE", 0, "5a936e23a890f4a96b6fdd865bd1f4a5", 20905},
			{"DSIDEC.EXE", 0, "31e6c169d9270b6de8c1c2e746ac238e", 49504},
			{"DSIDEE.EXE", 0, "524281f7d2dc49e0a41fcb1d38ee2559", 56800},
			{"DSIDEH.EXE", 0, "5e18e0746647bd04f43b9db24b6a036d", 53232},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
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
		ADGF_DEMO,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"darkside",
		"Demo",
		AD_ENTRY1s("DARKSIDE.ZX.DATA", "0e4d9b6e64ff24801272ff0b18a3caab", 29182),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"darkside",
		"",
		AD_ENTRY1s("DARKSIDE.ZX.DATA", "e840db278f1256d1d3a1a34d49644aee", 34460),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"darkside",
		"",
		AD_ENTRY1s("DARKCODE.BIN", "203ce55b7582c556c811acd6a1cf4ab6", 35385),
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERCPC)
	},
	// Total Eclipse
	{
		"totaleclipse",
		"Demo",
		{
			{"TECON.BIN", 0, "990f9a5e6d29142f98d53e53b1e43cc9", 16512},
			{"TEPROG.BIN", 0, "093a3a9a65c7914b93e13675fedafcc6", 29824},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERCPC)
	},
	{
		"totaleclipse",
		"Demo",
		{
			{"totaleclipse.zx.border", 0, "4df153a0c8986d1581b2aa58222c0eb8", 6912},
			{"totaleclipse.zx.data", 0, "af0b6408fb63082ce2b6cd5985908be1", 28142},
			{"totaleclipse.zx.title", 0, "1c1ab8a9994c1936e684b2980dba431b", 6912},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_DEMO | GF_ZX_DEMO_MICROHOBBY,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"totaleclipse",
		"Demo",
		{
			{"totaleclipse.zx.border", 0, "4df153a0c8986d1581b2aa58222c0eb8", 6912},
			{"totaleclipse.zx.data", 0, "13cd96820a65c84e63ca57c86f25881a", 29692},
			{"totaleclipse.zx.title", 0, "b25897736ffcafee174f525cd1f14b42", 6912},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_DEMO | GF_ZX_DEMO_CRASH,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"totaleclipse",
		"",
		AD_ENTRY1s("totaleclipse.zx.data", "5bc0c4ff50407d16bb409776eda7f4a8", 36055),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"totaleclipse2",
		"",
		AD_ENTRY1s("totaleclipse.zx.data", "5e80cb6a518d5ab2192b845801b1a32e", 35661),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_TESTING,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERZX)
	},
	{
		"totaleclipse",
		"",
		{
			{"TECODE.BIN", 0, "369d87a5a38df89cb8aa396995bb49d5", 35794},
			{"TECON.SCR", 0, "f6381a7e07b3bdcd569cb7423a269fd3", 16512},
			{"TESCR.SCR", 0, "9635f9401dae0936e22c556065c254f6", 16512},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		ADGF_TESTING,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERCPC)
	},
	{
		"totaleclipse2",
		"",
		{
			{"TE2.BI1", 0, "fc6e1a240e76a68e02ce1db5ad9a689a", 16512},
			{"TE2.BI2", 0, "c346262234e509ba5576c0a3362bc414", 35456},
			{"TE2.BI3", 0, "d73485df2eccb90592bb598693b10555", 16512},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmstradCPC,
		ADGF_TESTING,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERCPC)
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
		ADGF_TESTING,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{ // Erbe Software release
		"totaleclipse",
		"",
		{
			{"S.EXE", 0, "959703c1cd37b0d9744c492240a8178b", 13327},
			{"TOTEH.EXE", 0, "c68d59874ab2a93cc9cc1b1d3aed8f17", 60628},
			{"TOTEC.EXE", 0, "6c058c48255945a12479c8420321db75", 56900},
			{"TOTET.EXE", 0, "2bc2b0892ca91a77c58e9bedabf45afe", 63732},
			{"TOTEE.EXE", 0, "2ba865d2bdcda0c934baec7b31fb4ab2", 64196},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"totaleclipse",
		"",
		{
			{"0.TEC", 0, "e8b362cd913a2d58f8601e4df11a643a", 89500},
			{"1.TEC", 0, "3a2e304706bde3e34ac2a730eff8567b", 305286},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERAMIGA)
	},
	{ // Virtual Worlds release
		"totaleclipse",
		"",
		{
			{"0.TEC", 0, "8daa244d6814f06b9a081b7bd0ec542d", 89500},
			{"1.TEC", 0, "c1b022ad7284578f97aa2ff46ed3db38", 304030},
  			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERAMIGA)
	},
	{
		"totaleclipse",
		"",
		{
			{"0.TEC", 0, "01a2a7b2ce34b121e7ee4dab3bdac114", 45860},
			{"1.TEC", 0, "fb65655f60b11040285aa947d4ed1193", 263453},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},
	{ // Stampede Atari, Issue 7
		"totaleclipse",
		"",
		{
			{"0.TEC", 0, "c20a3bfea7147f73dc4fcd8aecf4faf8", 49402},
			{"1.TEC", 0, "fb65655f60b11040285aa947d4ed1193", 263453},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAtariST,
		ADGF_UNSUPPORTED,
		GUIO2(GUIO_NOMIDI, GUIO_RENDERATARIST)
	},

	// Castle Master
	// Tape release
	{
		"castlemaster",
		"",
		AD_ENTRY1s("castlemaster.zx.data", "e2ed83c30cd0ed7119e349d0f677ae91", 36096),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_UNSTABLE,
		GUIO3(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDERZX)
	},
	// Disc release
	{
		"castlemaster",
		"",
		AD_ENTRY1s("castlemaster.zx.data", "98513a4438ba93971d793a0fbc875b70", 36309),
		Common::EN_ANY,
		Common::kPlatformZX,
		ADGF_UNSTABLE | GF_ZX_DISC,
		GUIO3(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDERZX)
	},
	// Spanish release was disc-only?
	{
		"castlemaster",
		"",
		AD_ENTRY1s("castlemaster.zx.data", "3e6f6b283fa00a3073edce2392950601", 36309),
		Common::ES_ESP,
		Common::kPlatformZX,
		ADGF_UNSTABLE | GF_ZX_DISC,
		GUIO3(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDERZX)
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
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
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
		GUIO3(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDERAMIGA)
	},
	// Stampede Amiga, Issue 1, July 1990
	{
		"castlemaster",
		"",
		{
			{"cm", 0, "b7e713a0742fa09aa81c9606bbbba4af", 4068},
			{"x", 0, "c8c811439da0cf8a193e35feb5b5c6dc", 353388},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO3(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDERAMIGA)
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
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"castlemaster",
		"Virtual Worlds",
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
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
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
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
	},
	{
		"castlemaster",
		"CD release",
		{
			{"CASTLE.EXE", 0, "7601f74572b3c99bfc8d3f6d9d43d356", 29804},
			{"CMC.EXE", 0, "c7f651402307693048604e0f47c5c9e9", 57216},
			{"CME.EXE", 0, "e01f1561cbdc48d0a20823fdb852bdf1", 89296},
			{"CMH.EXE", 0, "76ac58ed66451689e84823706e000b6e", 60016},
			{"CMT.EXE", 0, "8c8621b5927d090bb7a4dca8d39cbfcf", 78816},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
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
		GUIO4(GUIO_NOMIDI, GAMEOPTION_TRAVEL_ROCK, GUIO_RENDEREGA, GUIO_RENDERCGA)
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

class FreescapeMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	FreescapeMetaEngineDetection() : AdvancedMetaEngineDetection(Freescape::gameDescriptions, Freescape::freescapeGames) {
		_guiOptions = GUIO7(GUIO_NOMIDI, GAMEOPTION_EXTENDED_TIMER, GAMEOPTION_DISABLE_DEMO_MODE, GAMEOPTION_DISABLE_SENSORS, GAMEOPTION_DISABLE_FALLING, GAMEOPTION_INVERT_Y, GAMEOPTION_AUTHENTIC_GRAPHICS);
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
