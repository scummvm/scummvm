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

#include "base/plugins.h"
#include "common/advancedDetector.h"

#include "gob/gob.h"

namespace Gob {

struct GOBGameDescription {
	Common::ADGameDescription desc;

	GameType gameType;
	int32 features;
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
	{"ween", "Ween: The Prophecy"},
	{"bargon", "Bargon Attack"},
	{"ajworld", "A.J's World of Discovery"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"lostintime", "Lost in Time"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"urban", "Urban Runner"},
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
		kGameTypeGob1,
		kFeaturesEGA,
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
		kGameTypeGob1,
		kFeaturesEGA,
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
		kGameTypeGob1,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by raziel_ in bug report #1891864
		{
			"gob1",
			"VGA",
			AD_ENTRY1s("intro.stk", "e157cb59c6d330ca70d12ab0ef1dd12b", 288972),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob1,
		kFeaturesNone,
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
		kGameTypeGob1,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by raina in the forums
		{
			"gob1",
			"",
			AD_ENTRY1s("intro.stk", "6d837c6380d8f4d984c9f6cc0026df4f", 192712),
			EN_ANY,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob1,
		kFeaturesNone,
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
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob1,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by raziel_ in bug report #1891867
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by fac76 in bug report #1883808
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "eebf2810122cfd17399260cd1468e994", 554014),
			EN_ANY,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
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
		kGameTypeGob2,
		kFeaturesNone,
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
		kGameTypeGob2,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by bgk in bug report #1706861
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "4b13c02d1069b86bcfec80f4e474b98b", 554680),
			FR_FRA,
			kPlatformAtariST,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesNone,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "4b278c2678ea01383fd5ca114d947eea"),
			UNK_LANG,
			kPlatformAmiga,
			Common::ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by polluks in bug report #1895126
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "9fa85aea959fa8c582085855fbd99346", 553063),
			UNK_LANG,
			kPlatformAmiga,
			Common::ADGF_DEMO
		},
		kGameTypeGob2,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeWeen,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by pwigren in bug report #1764174
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"music__5.snd", 0, "7d1819b9981ecddd53d3aacbc75f1cc8", 13446},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformAtariST,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesNone,
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
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeWeen,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "8b57cd510da8a3bbd99e3a0297a8ebd1", 7018771),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeWeen,
		kFeaturesAdlib,
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
		kGameTypeBargon,
		kFeaturesNone,
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
		kGameTypeBargon,
		kFeaturesNone,
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
		kGameTypeBargon,
		kFeaturesNone,
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
		kGameTypeBargon,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by pwigren in bugreport #1764174
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "569d679fe41d49972d34c9fce5930dda", 269825),
			EN_ANY,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		"intro"
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"bargon",
			"Fanmade",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeBargon,
		kFeaturesNone,
		"intro"
	},
	{
		{
			"ajworld",
			"",
			AD_ENTRY1s("intro.stk", "e453bea7b28a67c930764d945f64d898", 3913628),
			EN_ANY,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "7b7f48490dedc8a7cb999388e2fadbe3", 3930674),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by Arshlan in the forums
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "3712e7527ba8ce5637d2aadf62783005", 72318),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "f1f78b663893b58887add182a77df151", 3944090),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"lostintime",
			"Demo",
			AD_ENTRY1("demo.stk", "c06f8cc20eb239d4c71f225ce3093edf"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo"
	},
	{
		{
			"lostintime",
			"Non-interactive Demo",
			AD_ENTRY1("demo.stk", "2eba8abd9e3878c57307576012dd2fec"),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by raziel_ in bug report #1891869
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by fac76 in bug report #1742716
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "32b0f57f5ae79a9ae97e8011df38af42", 157084},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{NULL, 0, NULL, 0}
			},
			EN_GRB,
			kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{ // Supplied by Paranoimia on #scummvm
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "fe8144daece35538085adb59c2d29613", 159402),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			EN_GRB,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesNone,
		"menu"
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			DE_DEU,
			kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeGob3,
		kFeaturesNone,
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
		kGameTypeGob3,
		kFeaturesCD,
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
		kGameTypeGob3,
		kFeaturesCD,
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
		kGameTypeGob3,
		kFeaturesCD,
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
		kGameTypeGob3,
		kFeaturesCD,
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
		kGameTypeGob3,
		kFeaturesCD,
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
		kGameTypeGob3,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"gob3",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"gob3",
			"Interactive Demo 2",
			AD_ENTRY1("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d"),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"gob3",
			"Interactive Demo 3",
			AD_ENTRY1s("intro.stk", "9e20ad7b471b01f84db526da34eaf0a2", 395561),
			EN_ANY,
			kPlatformPC,
			Common::ADGF_DEMO
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
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
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			EN_GRB,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			DE_DEU,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			FR_FRA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			IT_ITA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			ES_ESP,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "6190e32404b672f4bbbc39cf76f41fda", 2511470),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		"intro"
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		"intro"
	},
	{
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "3ab2c542bd9216ae5d02cc6f45701ae1", 1252436),
			EN_USA,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeDynasty,
		kFeatures640,
		"intro"
	},
	{ AD_TABLE_END_MARKER, kGameTypeNone, kFeaturesNone, NULL }
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
		kGameTypeGob1,
		kFeaturesNone,
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
		kGameTypeGob1,
		kFeaturesCD,
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
		kGameTypeGob2,
		kFeaturesAdlib,
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
		kGameTypeGob2,
		kFeaturesCD,
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
		kGameTypeBargon,
		kFeaturesNone,
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
		kGameTypeGob3,
		kFeaturesAdlib,
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
		kGameTypeGob3,
		kFeaturesCD,
		"intro"
	},
	{
		{
			"woodruff",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeWoodruff,
		kFeatures640,
		"intro"
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"intro"
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		"intro"
	}
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
	{ &fallbackDescs[7], { "intro.stk", "woodruff.itk", 0 } },
	{ &fallbackDescs[8], { "intro.stk", "commun1.itk", 0 } },
	{ &fallbackDescs[9], { "intro.stk", "commun1.itk", "lost.lic", 0 } },
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
	// Flags
	0
};

class GobMetaEngine : public Common::AdvancedMetaEngine {
public:
	GobMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Gob Engine";
	}

	virtual const char *getCopyright() const {
		return "Goblins Games (C) Coktel Vision";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;
};

bool GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Gob::GOBGameDescription *gd = (const Gob::GOBGameDescription *)desc;
	if (gd) {
		*engine = new Gob::GobEngine(syst);
		((Gob::GobEngine *)*engine)->initGame(gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(GOB)
	REGISTER_PLUGIN_DYNAMIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GOB, PLUGIN_TYPE_ENGINE, GobMetaEngine);
#endif

namespace Gob {

void GobEngine::initGame(const GOBGameDescription *gd) {
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

	_gameType = gd->gameType;
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
}

} // End of namespace Gob
