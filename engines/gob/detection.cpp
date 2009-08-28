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
#include "engines/advancedDetector.h"

#include "gob/gob.h"

namespace Gob {

struct GOBGameDescription {
	ADGameDescription desc;

	GameType gameType;
	int32 features;
	const char *startStkBase;
	const char *startTotBase;
	uint32 demoIndex;
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
	{"littlered", "Little Red Riding Hood"},
	{"ajworld", "A.J's World of Discovery"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"lostintime", "Lost in Time"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"urban", "Urban Runner"},
	{"playtoons1", "Playtoons 1 - Uncle Archibald"},
	{"playtoons2", "Playtoons 2 - The Case of the Counterfeit Collaborator"},
	{"playtoons3", "Playtoons 3 - The Secret of the Castle"},
	{"playtoons4", "Playtoons 4 - The Mandarine Prince"},
	{"playtoons5", "Playtoons 5 - The Stone of Wakan"},
	{"playtnck1", "Playtoons Construction Kit 1 - Monsters"},
	{"playtnck2", "Playtoons Construction Kit 2 - Knights"},
	{"playtnck3", "Playtoons Construction Kit 3 - Far West"},
	{"bambou", "Playtoons Limited Edition - Bambou le sauveur de la jungle"},
	{"fascination", "Fascination"},
	{"geisha", "Geisha"},
	{"adi2", "ADI v2"},
	{"adibou4", "Adibou v4"},
	{"adibouunknown", "Adibou (not yet supported)"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

namespace Gob {

using Common::GUIO_NOSPEECH;
using Common::GUIO_NOSUBTITLES;
using Common::GUIO_NONE;

static const GOBGameDescription gameDescriptions[] = {
	{ // Supplied by Florian Zeitz on scummvm-devel
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"gob1",
			"EGA",
			AD_ENTRY1("intro.stk", "f9233283a0be2464248d83e14b95f09c"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesEGA,
		0, 0, 0
	},
	{ // Supplied by Theruler76 in bug report #1201233
		{
			"gob1",
			"VGA",
			AD_ENTRY1("intro.stk", "26a9118c0770fa5ac93a9626761600b2"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by raziel_ in bug report #1891864
		{
			"gob1",
			"VGA",
			AD_ENTRY1s("intro.stk", "e157cb59c6d330ca70d12ab0ef1dd12b", 288972),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.000 version.
		{
			"gob1cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // CD 1.02 version. Multilingual
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
			HU_HUN,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob1cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob1",
			"Demo",
			AD_ENTRY1("intro.stk", "972f22c6ff8144a6636423f0354ca549"),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "e72bd1e3828c7dec4c8a3e58c48bdfdb"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "a796096280d5efd48cf8e7dfbe426eb5", 193595),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2785958
		{
			"gob1",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "35a098571af9a03c04e2303aec7c9249", 116582),
			FR_FRA,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by raina in the forums
		{
			"gob1",
			"",
			AD_ENTRY1s("intro.stk", "6d837c6380d8f4d984c9f6cc0026df4f", 192712),
			EN_ANY,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			EN_ANY,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			DE_DEU,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			IT_ITA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob1",
			"",
			AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
			ES_ESP,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			IT_ITA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			ES_ESP,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			ES_ESP,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			IT_ITA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob1",
			"",
			{
				{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
				{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by fac76 in bug report #1883808
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "eebf2810122cfd17399260cd1468e994", 554014),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "d28b9e9b41f31acfa58dcd12406c7b2c"),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2602057
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "686c88f7302a80b744aae9f8413e853d"),
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by bgk in bug report #1706861
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "4b13c02d1069b86bcfec80f4e474b98b", 554680),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by fac76 in bug report #1673397
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "b45b984ee8017efd6ea965b9becd4d66", 828443},
				{"musmac1.mid", 0, "7f96f491448c7a001b32df89cf8d2af2", 1658},
				{0, 0, 0, 0}
			},
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by koalet in bug report #2478585
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275},
				{"musmac1.mid", 0, "7f96f491448c7a001b32df89cf8d2af2", 1658},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "b45b984ee8017efd6ea965b9becd4d66"),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "dedb5d31d8c8050a8cf77abedcc53dae"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by raziel_ in bug report #1891867
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by blackwhiteeagle in bug report #1605235
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "3e4e7db0d201587dd2df4003b2993ef6"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "a13892cdf4badda85a6f6fb47603a128"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2602017
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "c47faf1d406504e6ffe63243610bb1f4"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			AD_ENTRY1("intro.stk", "cd3e1df8b273636ee32e34b7064f50e8"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by arcepi in bug report #1659884
		{
			"gob2",
			"",
			AD_ENTRY1s("intro.stk", "5f53c56e3aa2f1e76c2e4f0caa15887f", 829232),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "9de5fbb41cf97182109e5fecc9d90347"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
			HU_HUN,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob2cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2",
			"Non-Interactive Demo",
			AD_ENTRY1("intro.stk", "8b1c98ff2ab2e14f47a1b891e9b92217"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, "usa.tot", 0
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "cf1c95b2939bd8ff58a25c756cb6125e"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "4b278c2678ea01383fd5ca114d947eea"),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by polluks in bug report #1895126
		{
			"gob2",
			"Interactive Demo",
			AD_ENTRY1s("intro.stk", "9fa85aea959fa8c582085855fbd99346", 553063),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "285d7340f98ebad65d465585da12910b", 837286},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "25a99827cd59751a80bed9620fb677a0", 893302},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{0, 0, 0, 0}
			},
			EN_USA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "25a99827cd59751a80bed9620fb677a0", 893302},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2",
			"",
			{
				{"intro.stk", 0, "25a99827cd59751a80bed9620fb677a0", 893302},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "257fe669705ac4971efdfd5656eef16a", 457719),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "dffd1ab98fe76150d6933329ca6f4cc4", 459458),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by vampir_raziel in bug report #1658373
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "af83debf2cbea21faa591c7b4608fe92", 458192),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2563539
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "dffd1ab98fe76150d6933329ca6f4cc4", 459458},
				{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
				{0, 0, 0, 0}
			},
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by pwigren in bug report #1764174
		{
			"ween",
			"",
			{
				{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
				{"music__5.snd", 0, "7d1819b9981ecddd53d3aacbc75f1cc8", 13446},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "e6d13fb3b858cb4f78a8780d184d5b2c"),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "2bb8878a8042244dd2b96ff682381baa"),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "de92e5c6a8c163007ffceebef6e67f7d", 7117568),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by cybot_tmin in bug report #1667743
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "6d60f9205ecfbd8735da2ee7823a70dc", 7014426),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "4b10525a3782aa7ecd9d833b5c1d308b"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"ween",
			"",
			AD_ENTRY1("intro.stk", "63170e71f04faba88673b3f510f9c4c8"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"ween",
			"",
			AD_ENTRY1s("intro.stk", "8b57cd510da8a3bbd99e3a0297a8ebd1", 7018771),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "2e9c2898f6bf206ede801e3b2e7ee428"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, "show.tot", 0
	},
	{
		{
			"ween",
			"Demo",
			AD_ENTRY1("intro.stk", "15fb91a1b9b09684b28ac75edf66e504"),
			EN_USA,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWeen,
		kFeaturesAdlib,
		0, "show.tot", 0
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1("intro.stk", "da3c54be18ab73fbdb32db24624a9c23"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by Trekky in the forums
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "2f54b330d21f65b04b7c1f8cca76426c", 262109),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by cesardark in bug #1681649
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "11103b304286c23945560b391fd37e7d", 3181890),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug #1692667
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by pwigren in bugreport #1764174
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "569d679fe41d49972d34c9fce5930dda", 269825),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by kizkoool in bugreport #2089734
		{
			"bargon",
			"",
			AD_ENTRY1s("intro.stk", "00f6b4e2ee26e5c40b488e2df5adcf03", 3975580),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ // Supplied by glorfindel in bugreport #1722142
		{
			"bargon",
			"Fanmade",
			AD_ENTRY1s("intro.stk", "da3c54be18ab73fbdb32db24624a9c23", 3181825),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			AD_ENTRY1s("intro.stk", "0b72992f5d8b5e6e0330572a5753ea25", 256490),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib | kFeaturesEGA,
		0, 0, 0
	},
	{
		{
			"littlered",
			"",
			{
				{"intro.stk", 0, "0b72992f5d8b5e6e0330572a5753ea25", 256490},
				{"mod.babayaga", 0, "43484cde74e0860785f8e19f0bc776d1", 60248},
				{0, 0, 0, 0}
			},
			UNK_LANG,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"ajworld",
			"",
			AD_ENTRY1s("intro.stk", "e453bea7b28a67c930764d945f64d898", 3913628),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "7b7f48490dedc8a7cb999388e2fadbe3", 3930674),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "e0767783ff662ed93665446665693aef", 4371238),
			HB_ISR,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Arshlan in the forums
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "3712e7527ba8ce5637d2aadf62783005", 72318),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by cartman_ on #scummvm
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "f1f78b663893b58887add182a77df151", 3944090),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2105220
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "cd322cb3c64ef2ba2f2134aa2122cfe9", 3936700),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by koalet in bug report #2479034
		{
			"lostintime",
			"",
			{
				{"intro.stk", 0, "af98bcdc70e1f1c1635577fd726fe7f1", 3937310},
				{"musmac1.mid", 0, "ae7229bb09c6abe4e60a2768b24bc890", 9398},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "6263d09e996c1b4e84ef2d650b820e57", 4831170),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2093672
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "795be7011ec31bf5bb8ce4efdb9ee5d3", 4838904),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "0ddf39cea1ec30ecc8bfe444ebd7b845", 4207330),
			EN_GRB,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "0ddf39cea1ec30ecc8bfe444ebd7b845", 4207330),
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"",
			AD_ENTRY1s("intro.stk", "0ddf39cea1ec30ecc8bfe444ebd7b845", 4207330),
			ES_ESP,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"fascination",
			"CD Version (Censored)",
			AD_ENTRY1s("disk0.stk", "9c61e9c22077f72921f07153e37ccf01", 545953),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES
		},
		kGameTypeFascination,
		kFeaturesCD,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("disk0.stk", "a50a8495e1b2d67699fb562cb98fc3e2", 1064387),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesAdlib,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("intro.stk", "d6e45ce548598727e2b5587a99718eba", 1055909),
			HB_ISR,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesAdlib,
		"intro.stk", 0, 0
	},
	{ // Supplied by sanguine
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("disk0.stk", "c14330d052fe4da5a441ac9d81bc5891", 1061955),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesAdlib,
		"disk0.stk", 0, 0
	},
	{ // Supplied by windlepoons in bug report #2809247
		{
			"fascination",
			"VGA 3 disks edition",
			AD_ENTRY1s("disk0.stk", "3a24e60a035250189643c86a9ceafb97", 1062480),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesAdlib,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"VGA",
			AD_ENTRY1s("disk0.stk", "e8ab4f200a2304849f462dc901705599", 183337),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesAdlib,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "68b1c01564f774c0b640075fbad1b695", 189968),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "7062117e9c5adfb6bfb2dac3ff74df9e", 189951),
			EN_ANY,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "55c154e5a3e8e98afebdcff4b522e1eb", 190005),
			FR_FRA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk", 0, 0
	},
	{
		{
			"fascination",
			"",
			AD_ENTRY1s("disk0.stk", "7691827fff35df7799f14cfd6be178ad", 189931),
			IT_ITA,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk", 0, 0
	},
	{
		{
			"geisha",
			"",
			AD_ENTRY1s("disk1.stk", "6eebbb98ad90cd3c44549fc2ab30f632", 212153),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGeisha,
		kFeaturesNone,
		"disk1.stk", "intro.tot", 0
	},
	{
		{
			"geisha",
			"",
			AD_ENTRY1s("disk1.stk", "f4d4d9d20f7ad1f879fc417d47faba89", 336732),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGeisha,
		kFeaturesNone,
		"disk1.stk", "intro.tot", 0
	},
	{
		{
			"lostintime",
			"Demo",
			AD_ENTRY1("demo.stk", "c06f8cc20eb239d4c71f225ce3093edf"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo.stk", "demo.tot", 0
	},
	{
		{
			"lostintime",
			"Non-interactive Demo",
			AD_ENTRY1("demo.stk", "2eba8abd9e3878c57307576012dd2fec"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		"demo.stk", "demo.tot", 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "904fc32032295baa3efb3a41f17db611", 178582),
			HB_ISR,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by raziel_ in bug report #1891869
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
				{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
				{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
				{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by fac76 in bug report #1742716
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "32b0f57f5ae79a9ae97e8011df38af42", 157084},
				{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
				{0, 0, 0, 0}
			},
			EN_GRB,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "1e2f64ec8dfa89f42ee49936a27e66e7"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by paul66 in bug report #1652352
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "f6d225b25a180606fa5dbe6405c97380"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "e42a4f2337d6549487a80864d7826972"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by Paranoimia on #scummvm
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "fe8144daece35538085adb59c2d29613", 159402),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "4e3af248a48a2321364736afab868527"),
			RU_RUS,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "8d28ce1591b0e9cc79bf41cad0fc4c9c"),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{ // Supplied by SiRoCs in bug report #2098621
		{
			"gob3",
			"",
			AD_ENTRY1s("intro.stk", "d3b72938fbbc8159198088811f9e6d19", 160382),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			EN_GRB,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesNone,
		0, "menu.tot", 0
	},
	{
		{
			"gob3",
			"",
			AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
			DE_DEU,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesNone,
		0, "menu.tot", 0
	},
	{
		{
			"gob3cd",
			"v1.000",
			AD_ENTRY1("intro.stk", "6f2c226c62dd7ab0ab6f850e89d3fc47"),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
			HU_HUN,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2810082
		{
			"gob3cd",
			"v1.02",
			AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob3",
			"Interactive Demo",
			AD_ENTRY1("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d"),
			FR_FRA,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"Interactive Demo 2",
			AD_ENTRY1("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d"),
			FR_FRA,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"Non-interactive Demo",
			AD_ENTRY1("intro.stk", "b9b898fccebe02b69c086052d5024a55"),
			UNK_LANG,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"Interactive Demo 3",
			AD_ENTRY1s("intro.stk", "9e20ad7b471b01f84db526da34eaf0a2", 395561),
			EN_ANY,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3",
			"",
			{
				{"intro.stk", 0, "edd7403e5dc2a14459d2665a4c17714d", 209534},
				{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "47c3b452767c4f49ea7b109143e77c30", 916828),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "1fa92b00fe80a20f34ec34a8e2fa869e", 923072),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
			EN_USA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
			DE_DEU,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
			IT_ITA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
			ES_ESP,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"",
			AD_ENTRY1s("intro.stk", "d33011df8758ac64ca3dca77c7719001", 908612),
			FR_FRA,
			kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"inca2",
			"Non-Interactive Demo",
			{
				{"cons.imd", 0, "f896ba0c4a1ac7f7260d342655980b49", 17804},
				{"conseil.imd", 0, "aaedd5482d5b271e233e86c5a03cf62e", 33999},
				{"int.imd", 0, "6308222fcefbcb20925f01c1aff70dee", 30871},
				{"inter.imd", 0, "39bd6d3540f3bedcc97293f352c7f3fc", 191719},
				{"machu.imd", 0, "c0bc8211d93b467bfd063b63fe61b85c", 34609},
				{"post.imd", 0, "d75cad0e3fc22cb0c8b6faf597f509b2", 1047709},
				{"posta.imd", 0, "2a5b3fe75681ddf4d21ac724db8111b4", 547250},
				{"postb.imd", 0, "24260ce4e80a4c472352b76637265d09", 868312},
				{"postc.imd", 0, "24accbcc8b83a9c2be4bd82849a2bd29", 415637},
				{"tum.imd", 0, "0993d4810ec9deb3f77c5e92095320fd", 20330},
				{"tumi.imd", 0, "bf53f229480d694de0947fe3366fbec6", 248952},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeInca2,
		kFeaturesAdlib | kFeaturesBATDemo,
		0, 0, 7
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "dccf9d31cb720b34d75487408821b77e", 20296390),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "b50fee012a5abcd0ac2963e1b4b56bec", 20298108),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "5f5f4e0a72c33391e67a47674b120cc6", 20296422),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "270529d9b8cce770b1575908a3800b52", 20296452),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by Hkz on #scummvm
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "f4c344023b073782d2fddd9d8b515318", 7069736),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			EN_GRB,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by DjDiabolik in bug report #1971294
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "60348a87651f92e8492ee070556a96d8", 7069736),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2098838
		{
			"woodruff",
			"",
			AD_ENTRY1s("intro.stk", "08a96bf061af1fa4f75c6a7cc56b60a4", 20734979),
			PL_POL,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"Non-Interactive Demo",
			{
				{"demo.scn", 0, "16bb85fc5f8e519147b60475dbf33962", 89},
				{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 1
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "6190e32404b672f4bbbc39cf76f41fda", 2511470),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "b3f8472484b7a1df94557b51e7b6fca0", 2322644),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"",
			AD_ENTRY1s("intro.stk", "bdbdac8919200a5e71ffb9fb0709f704", 2446652),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"Demo",
			AD_ENTRY1s("intro.stk", "464538a17ed39755d7f1ba9c751af1bd", 1847864),
			EN_USA,
			kPlatformPC,
			ADGF_DEMO,
			GUIO_NONE
		},
		kGameTypeDynasty,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"dynasty",
			"Demo",
			AD_ENTRY1s("lda1.stk", "0e56a899357cbc0bf503260fd2dd634e", 15032774),
			UNK_LANG,
			kPlatformWindows,
			ADGF_DEMO,
			GUIO_NONE
		},
		kGameTypeDynasty,
		kFeatures640,
		"lda1.stk", 0, 0
	},
	{
		{
			"dynasty",
			"Demo",
			AD_ENTRY1s("lda1.stk", "8669ea2e9a8239c070dc73958fbc8753", 15567724),
			DE_DEU,
			kPlatformWindows,
			ADGF_DEMO,
			GUIO_NONE
		},
		kGameTypeDynasty,
		kFeatures640,
		"lda1.stk", 0, 0
	},
	{
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "3ab2c542bd9216ae5d02cc6f45701ae1", 1252436),
			EN_USA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeUrban,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by gamin in the forums
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "b991ed1d31c793e560edefdb349882ef", 1276408),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeUrban,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by jvprat on #scummvm
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "4ec3c0864e2b54c5b4ccf9f6ad96528d", 1253328),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeUrban,
		kFeatures640,
		0, 0, 0
	},
	{ // Supplied by goodoldgeorg in bug report #2770340
		{
			"urban",
			"",
			AD_ENTRY1s("intro.stk", "4bd31979ea3d77a58a358c09000a85ed", 1253018),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeUrban,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"urban",
			"Non-Interactive Demo",
			{
				{"wdemo.s24", 0, "14ac9bd51db7a075d69ddb144904b271", 87},
				{"demo.vmd", 0, "65d04715d871c292518b56dd160b0161", 9091237},
				{"urband.vmd", 0, "60343891868c91854dd5c82766c70ecc", 922461},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeUrban,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 2
	},
	{
		{
			"playtoons1",
			"",
			{
				{"playtoon.stk", 0, "8c98e9a11be9bb203a55e8c6e68e519b", 25574338},
				{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons1",
			"Pack mes histoires anim\xE9""es",
			{
				{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
				{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons1",
			"",
			{
				{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
				{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons1",
			"Non-Interactive Demo",
			{
				{"play123.scn", 0, "4689a31f543915e488c3bc46ea358add", 258},
				{"archi.vmd", 0, "a410fcc8116bc173f038100f5857191c", 5617210},
				{"chato.vmd", 0, "5a10e39cb66c396f2f9d8fb35e9ac016", 5445937},
				{"genedeb.vmd", 0, "3bb4a45585f88f4d839efdda6a1b582b", 1244228},
				{"generik.vmd", 0, "b46bdd64b063e86927fb2826500ad512", 603242},
				{"genespi.vmd", 0, "b7611916f32a370ae9832962fc17ef72", 758719},
				{"spirou.vmd", 0, "8513dbf7ac51c057b21d371d6b217b47", 2550788},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 3
	},
	{
		{
			"playtoons1",
			"Non-Interactive Demo",
			{
				{"e.scn", 0, "8a0db733c3f77be86e74e8242e5caa61", 124},
				{"demarchg.vmd", 0, "d14a95da7d8792faf5503f649ffcbc12", 5619415},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 4
	},
	{
		{
			"playtoons1",
			"Non-Interactive Demo",
			{
				{"i.scn", 0, "8b3294474d39970463663edd22341730", 285},
				{"demarita.vmd", 0, "84c8672b91c7312462603446e224bfec", 5742533},
				{"dembouit.vmd", 0, "7a5fdf0a4dbdfe72e31dd489ea0f8aa2", 3536786},
				{"demo5.vmd", 0, "2abb7b6a26406c984f389f0b24b5e28e", 13290970},
				{"demoita.vmd", 0, "b4c0622d14c8749965cd0f5dfca4cf4b", 1183566},
				{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
				{0, 0, 0, 0}
			},
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 5
	},
	{
		{
			"playtoons1",
			"Non-Interactive Demo",
			{
				{"s.scn", 0, "1f527010626b5490761f16ba7a6f639a", 251},
				{"demaresp.vmd", 0, "3f860f944056842b35a5fd05416f208e", 5720619},
				{"demboues.vmd", 0, "3a0caa10c98ef92a15942f8274075b43", 3535838},
				{"demo5.vmd", 0, "2abb7b6a26406c984f389f0b24b5e28e", 13290970},
				{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
				{0, 0, 0, 0}
			},
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640 | kFeaturesSCNDemo,
		0, 0, 6
	},
	{
		{
			"playtoons2",
			"",
			{
				{"playtoon.stk", 0, "4772c96be88a57f0561519e4a1526c62", 24406262},
				{"spirou.stk", 0, "5d9c7644d0c47840169b4d016765cc1a", 9816201},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons2",
			"",
			{
				{"playtoon.stk", 0, "55a85036dd93cce93532d8f743d90074", 17467154},
				{"spirou.stk", 0, "e3e1b6148dd72fafc3637f1a8e5764f5", 9812043},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons2",
			"",
			{
				{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
				{"spirou.stk", 0, "91080dc148de1bbd6a97321c1a1facf3", 9817086},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons3",
			"",
			{
				{"playtoon.stk", 0, "8c98e9a11be9bb203a55e8c6e68e519b", 25574338},
				{"chato.stk", 0, "4fa4ed96a427c344e9f916f9f236598d", 6033793},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons3",
			"",
			{
				{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
				{"chato.stk", 0, "8fc8d0da5b3e758908d1d7298d497d0b", 6041026},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons3",
			"Pack mes histoires anim\xE9""es",
			{
				{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
				{"chato.stk", 0, "4fa4ed96a427c344e9f916f9f236598d", 6033793},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons3",
			"",
			{
				{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
				{"chato.stk", 0, "3c6cb3ac8a5a7cf681a19971a92a748d", 6033791},
				{0, 0, 0, 0}
			},
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons4",
			"",
			{
				{"playtoon.stk", 0, "b7f5afa2dc1b0f75970b7c07d175db1b", 24340406},
				{"manda.stk", 0, "92529e0b927191d9898a34c2892e9a3a", 6485072},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{ //Supplied by goodoldgeorg in bug report #2820006
		{
			"playtoons4",
			"",
			{
				{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
				{"manda.stk", 0, "69a79c9f61b2618e482726f2ff68078d", 6499208},
				{0, 0, 0, 0}
			},
			EN_ANY,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtoons5",
			"",
			{
				{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
				{"wakan.stk", 0, "f493bf82851bc5ba74d57de6b7e88df8", 5520153},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"bambou",
			"",
			{
				{"intro.stk", 0, "2f8db6963ff8d72a8331627ebda918f4", 3613238},
				{"bambou.itk", 0, "0875914d31126d0749313428f10c7768", 114440192},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBambou,
		kFeatures640,
		"intro.stk", "intro.tot", 0
	},
	{
		{
			"playtnck1",
			"",
			{
				{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
				{"dan.itk", 0, "906d67b3e438d5e95ec7ea9e781a94f3", 3000320},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtnck2",
			"",
			{
				{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
				{"dan.itk", 0, "74eeb075bd2cb47b243349730264af01", 3213312},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"playtnck3",
			"",
			{
				{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
				{"dan.itk", 0, "9a8f62809eca5a52f429b5b6a8e70f8f", 2861056},
				{0, 0, 0, 0}
			},
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		"intro2.stk", 0, 0
	},
	{
		{
			"adi2",
			"Adi 2.0 for Teachers",
			AD_ENTRY1s("adi2.stk", "da6f1fb68bff32260c5eecdf9286a2f5", 1533168),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdi2,
		kFeaturesNone,
		"adi2.stk", "ediintro.tot", 0
	},
	{
		{
			"adi2",
			"Adi 2.6",
			AD_ENTRY1s("adi2.stk", "2fb940eb8105b12871f6b88c8c4d1615", 16780058),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdi2,
		kFeatures640,
		"adi2.stk", "ediintro.tot", 0
	},
	{
		{
			"adi2",
			"Adi 2.7.1",
			AD_ENTRY1s("adi2.stk", "6fa5dffebf5c7243c6af6b8c188ee00a", 19278008),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdi2,
		kFeatures640,
		"adi2.stk", "ediintro.tot", 0
	},
	{
		{
			"adi2",
			"ADI 2 Spanish",
			AD_ENTRY1s("adi2.stk", "2a40bb48ccbd4e6fb3f7f0fc2f069d80", 17720132),
			ES_ESP,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdi2,
		kFeatures640,
		"adi2.stk", "ediintro.tot", 0
	},
	{
		{
			"adibou4",
			"",
			AD_ENTRY1s("intro.stk", "a3c35d19b2d28ea261d96321d208cb5a", 6021466),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibou4,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"adibou4",
			"Addy 4 Grundschule Basis CD",
			AD_ENTRY1s("intro.stk", "d2f0fb8909e396328dc85c0e29131ba8", 5847588),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibou4,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"adibou4",
			"Addy 4 Sekundarstufe Basis CD",
			AD_ENTRY1s("intro.stk", "367340e59c461b4fa36651cd74e32c4e", 5847378),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibou4,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"adibou4",
			"ADI 4.21 French",
			AD_ENTRY1s("intro.stk", "c5b9f6222c0b463f51dab47317c5b687", 5950490),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibou4,
		kFeaturesNone,
		0, 0, 0
	},	{
		{
			"adibouunknown",
			"",
			AD_ENTRY1s("intro.stk", "904a93f46687617bb34e672020fc17a4", 248724),
			FR_FRA,
			kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, "base.tot", 0
	},
	{
		{
			"adibouunknown",
			"",
			AD_ENTRY1s("intro.stk", "1e49c39a4a3ce6032a84b712539c2d63", 8738134),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"adibouunknown",
			"ADIBOU 2 French",
			AD_ENTRY1s("intro.stk", "94ae7004348dc8bf99c23a9a6ef81827", 956162),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"adibouunknown",
			"ADIBOU 2 environnement",
			AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
			FR_FRA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"adibouunknown",
			"ADIBOU 2",
			AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
			DE_DEU,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"adibouunknown",
			"ADIB\xD9 2 Ambiente",
			AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"adibouunknown",
				"ADIB\xD9 prima elementare : Imparo a leggere e a contare",
			{
				{"intro.stk", 0, "092707829555f27706920e4cacf1fada", 8737958},
				{"appbou2.itk", 0, "f7bf045f6bdce5a7607c720e36704f33", 200005632},
				{0, 0, 0, 0}
			},
			IT_ITA,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeAdibouUnknown,
		kFeaturesNone,
		"adibou.stk", "intro.tot", 0
	},
	{ AD_TABLE_END_MARKER, kGameTypeNone, kFeaturesNone, 0, 0, 0}
};

static const GOBGameDescription fallbackDescs[] = {
	{
		{
			"gob1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob1cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"gob2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2mac",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob2cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"bargon",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{
		{
			"gob3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"gob3cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"woodruff",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeWoodruff,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesAdlib,
		0, 0, 0
	},
	{
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"urban",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeUrban,
		kFeaturesCD,
		0, 0, 0
	},
	{
		{
			"playtoons1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"playtoons2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"playtoons3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"playtoons4",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"playtoons5",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"playtoons construction kit",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypePlaytoons,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"bambou",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeBambou,
		kFeatures640,
		0, 0, 0
	},
	{
		{
			"fascination",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeFascination,
		kFeaturesNone,
		"disk0.stk", 0, 0
	},
	{
		{
			"geisha",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeGeisha,
		kFeaturesNone,
		"disk1.stk", "intro.tot", 0
	},
	{
		{
			"adi2",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeAdi2,
		kFeatures640,
		"adi2.stk", 0, 0
	},
	{
		{
			"adibou4",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSUBTITLES | GUIO_NOSPEECH
		},
		kGameTypeAdibou4,
		kFeatures640,
		"adif41.stk", 0, 0
	},
	{
		{
			"coktelplayer",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kGameTypeUrban,
		kFeaturesAdlib | kFeatures640 | kFeaturesSCNDemo,
		"", "", 8
	}
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[ 0], { "intro.stk", "disk1.stk", "disk2.stk", "disk3.stk", "disk4.stk", 0 } },
	{ &fallbackDescs[ 1], { "intro.stk", "gob.lic", 0 } },
	{ &fallbackDescs[ 2], { "intro.stk", 0 } },
	{ &fallbackDescs[ 2], { "intro.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[ 3], { "intro.stk", "disk2.stk", "disk3.stk", "musmac1.mid", 0 } },
	{ &fallbackDescs[ 4], { "intro.stk", "gobnew.lic", 0 } },
	{ &fallbackDescs[ 5], { "intro.stk", "scaa.imd", "scba.imd", "scbf.imd", 0 } },
	{ &fallbackDescs[ 6], { "intro.stk", "imd.itk", 0 } },
	{ &fallbackDescs[ 7], { "intro.stk", "mus_gob3.lic", 0 } },
	{ &fallbackDescs[ 8], { "intro.stk", "woodruff.itk", 0 } },
	{ &fallbackDescs[ 9], { "intro.stk", "commun1.itk", 0 } },
	{ &fallbackDescs[10], { "intro.stk", "commun1.itk", "musmac1.mid", 0 } },
	{ &fallbackDescs[11], { "intro.stk", "commun1.itk", "lost.lic", 0 } },
	{ &fallbackDescs[12], { "intro.stk", "cd1.itk", "objet1.itk", 0 } },
	{ &fallbackDescs[13], { "playtoon.stk", "archi.stk", 0 } },
	{ &fallbackDescs[14], { "playtoon.stk", "spirou.stk", 0 } },
	{ &fallbackDescs[15], { "playtoon.stk", "chato.stk", 0 } },
	{ &fallbackDescs[16], { "playtoon.stk", "manda.stk", 0 } },
	{ &fallbackDescs[17], { "playtoon.stk", "wakan.stk", 0 } },
	{ &fallbackDescs[18], { "playtoon.stk", "dan.itk" } },
	{ &fallbackDescs[19], { "intro.stk", "bambou.itk", 0 } },
	{ &fallbackDescs[20], { "disk0.stk", "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[21], { "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[22], { "adi2.stk", 0 } },
	{ &fallbackDescs[23], { "adif41.stk", "adim41.stk", 0 } },
	{ &fallbackDescs[24], { "coktelplayer.scn", 0 } },
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
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NOLAUNCHLOAD
};

class GobMetaEngine : public AdvancedMetaEngine {
public:
	GobMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Gob Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Goblins Games (C) Coktel Vision";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool GobMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Gob::GobEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}
bool GobMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
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
	if (gd->startTotBase == 0)
		_startTot = "intro.tot";
	else
		_startTot = gd->startTotBase;

	if (gd->startStkBase == 0)
		_startStk = "intro.stk";
	else
		_startStk = gd->startStkBase;

	_demoIndex = gd->demoIndex;

	_gameType = gd->gameType;
	_features = gd->features;
	_language = gd->desc.language;
	_platform = gd->desc.platform;
}
} // End of namespace Gob
