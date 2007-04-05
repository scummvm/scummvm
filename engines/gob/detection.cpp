/* ScummVM - Scumm Interpreter
 * Copyright (C) 2007 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "base/plugins.h"
#include "common/advancedDetector.h"

#include "gob/gob.h"

namespace Gob {

struct GOBGameDescription {
	Common::ADGameDescription desc;

	uint32 features;
	const char *startTotBase;
};

}

using namespace Common;

static const PlainGameDescriptor gobGames[] = {
	{"gob", "Gob engine game"},
	{"gob1", "Gobliiins"},
	{"gob1cd", "Gobliiins CD"},
	{"gob2", "Gobliins 2"},
	{"gob2cd", "Gobliins 2 CD"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"bargon", "Bargon Attack"},
	{"ween", "Ween: The Prophecy"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

namespace Gob {

static const GOBGameDescription gameDescriptions[] = {
	{ // Supplied by Florian Zeitz on scummvm-devel
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_EGA,
		"intro"
	},
	{
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "f9233283a0be2464248d83e14b95f09c"),
			RU_RUS,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_EGA,
		"intro"
	},
	{ // Supplied by Theruler76 in bug report #1201233
		{
			"gob1",
			"VGA",
			AD_ENTRY1("intro.stk", "26a9118c0770fa5ac93a9626761600b2"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{
		{
			"gob1",
			"Demo",
			AD_ENTRY1("intro.stk", "972f22c6ff8144a6636423f0354ca549"),
			UNK_LANG,
			kPlatformAmiga,
			Common::ADGF_DEMO
		},
		GF_GOB1,
		"intro"
	},
	{
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "e72bd1e3828c7dec4c8a3e58c48bdfdb"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB1,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			EN_ANY,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			DE_DEU,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			FR_FRA,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			IT_ITA,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			ES_ESP,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "a13ecb4f6d8fd881ebbcc02e45cb5475"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by arcepi in bug report #1659884
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "5f53c56e3aa2f1e76c2e4f0caa15887f", 829232),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "b45b984ee8017efd6ea965b9becd4d66"),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by fac76 in bug report #1673397
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "b45b984ee8017efd6ea965b9becd4d66", 828443},
				{"musmac1.mid", 0, "7f96f491448c7a001b32df89cf8d2af2", 1658},
				{NULL, 0, NULL, 0}
			},
			UNK_LANG,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "dedb5d31d8c8050a8cf77abedcc53dae"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "a13892cdf4badda85a6f6fb47603a128"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "cd3e1df8b273636ee32e34b7064f50e8"),
			RU_RUS,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "d28b9e9b41f31acfa58dcd12406c7b2c"),
			DE_DEU,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by blackwhiteeagle in bug report #1605235
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "3e4e7db0d201587dd2df4003b2993ef6"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "9de5fbb41cf97182109e5fecc9d90347"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			EN_ANY,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"gob2",
			"Non-Interactive Demo",
			AD_ENTRY1("intro.stk", "8b1c98ff2ab2e14f47a1b891e9b92217"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB2,
		"usa"
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "cf1c95b2939bd8ff58a25c756cb6125e"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "4b278c2678ea01383fd5ca114d947eea"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "2bb8878a8042244dd2b96ff682381baa"),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "de92e5c6a8c163007ffceebef6e67f7d", 7117568),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by cybot_tmin in bug report #1667743
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "6d60f9205ecfbd8735da2ee7823a70dc", 7014426),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570),
			EN_GRB,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "af83debf2cbea21faa591c7b4608fe92", 458192),
			DE_DEU,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "257fe669705ac4971efdfd5656eef16a", 457719),
			FR_FRA,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "dffd1ab98fe76150d6933329ca6f4cc4", 459458),
			FR_FRA,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "e6d13fb3b858cb4f78a8780d184d5b2c"),
			FR_FRA,
			kPlatformAtariST,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "4b10525a3782aa7ecd9d833b5c1d308b"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "63170e71f04faba88673b3f510f9c4c8"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "2e9c2898f6bf206ede801e3b2e7ee428"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB2,
		"show"
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "15fb91a1b9b09684b28ac75edf66e504"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB2,
		"show"
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1("intro.stk", "da3c54be18ab73fbdb32db24624a9c23"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_BARGON,
		"intro"
	},
	{ // Supplied by Trekky in the forums
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "2f54b330d21f65b04b7c1f8cca76426c", 262109),
			FR_FRA,
			kPlatformAtariST,
			Common::ADGF_NO_FLAGS
		},
		GF_BARGON,
		"intro"
	},
	{ // Supplied by cesardark in bug #1681649
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "11103b304286c23945560b391fd37e7d", 3181890),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_BARGON,
		"intro"
	},
	{ // Supplied by paul66 in bug #1692667
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_BARGON,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "1e2f64ec8dfa89f42ee49936a27e66e7"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "f6d225b25a180606fa5dbe6405c97380"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "e42a4f2337d6549487a80864d7826972"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "4e3af248a48a2321364736afab868527"),
			RU_RUS,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "8d28ce1591b0e9cc79bf41cad0fc4c9c"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			UNK_LANG,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"menu"
	},
	{
		{
			"gob3cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "6f2c226c62dd7ab0ab6f850e89d3fc47"),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			EN_ANY,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"Non-interactive Demo",
			AD_ENTRY1("intro.stk", "b9b898fccebe02b69c086052d5024a55"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3",
			"Interactive Demo 2",
			AD_ENTRY1("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_WOODRUFF,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_WOODRUFF,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "5f5f4e0a72c33391e67a47674b120cc6", 20296422),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_WOODRUFF,
		"intro"
	},
	{ AD_TABLE_END_MARKER, 0, NULL }
};

static const GOBGameDescription fallbackDescs[] = {
	{
		{
			"gob1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1,
		"intro"
	},
	{
		{
			"gob1cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB1 | GF_CD,
		"intro"
	},
	{
		{
			"gob2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2,
		"intro"
	},
	{
		{
			"gob2cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB2 | GF_CD,
		"intro"
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_BARGON,
		"intro"
	},
	{
		{
			"gob3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3,
		"intro"
	},
	{
		{
			"gob3cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_GOB3 | GF_CD,
		"intro"
	},
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[0], { "intro.stk", "disk1.stk", "disk2.stk", "disk3.stk", "disk4.stk", 0 } },
	{ &fallbackDescs[1], { "intro.stk", "gob.lic", 0 } },
	{ &fallbackDescs[2], { "intro.stk", 0 } },
	{ &fallbackDescs[2], { "intro.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[3], { "intro.stk", "gobnew.lic", 0 } },
	{ &fallbackDescs[4], { "intro.stk", "scaa.imd", "scba.imd", "scbf.imd", 0 } },
	{ &fallbackDescs[5], { "intro.stk", "imd.itk", 0 } },
	{ &fallbackDescs[6], { "intro.stk", "mus_gob3.lic", 0 } },
	{ 0, { 0 } }
};

}

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Gob::gameDescriptions,
	// Size of that superset structure
	sizeof(Gob::GOBGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	gobGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"gob",
	// List of files for file-based fallback detection (optional)
	Gob::fileBased,
	// Fallback callback
	0,
	// Flags
	kADFlagAugmentPreferredTarget
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(GOB, Gob::GobEngine, detectionParams);

REGISTER_PLUGIN(GOB, "Gob Engine", "Goblins Games (C) Coktel Vision");


namespace Gob {

bool GobEngine::detectGame() {
	const GOBGameDescription *gd = (const GOBGameDescription *)Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	if (gd == 0)
		return false;

	if (gd->startTotBase == 0) {
		_startTot = new char[10];
		_startTot0 = new char[11];
		strcpy(_startTot, "intro.tot");
		strcpy(_startTot0, "intro0.tot");
	} else {
		_startTot = new char[strlen(gd->startTotBase) + 5];
		_startTot0 = new char[strlen(gd->startTotBase) + 6];
		strcpy(_startTot, gd->startTotBase);
		strcpy(_startTot0, gd->startTotBase);
		strcat(_startTot, ".tot");
		strcat(_startTot0, "0.tot");
	}

	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;

	return true;
}

} // End of namespace Gob
