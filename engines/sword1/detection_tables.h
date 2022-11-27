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


namespace Sword1 {

static const SwordGameDescription gameDescriptions[] = {
	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200156,
			           "swordres.rif", "bc01bc995f23e46bb076f66ba5c516c4", 58388),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "3786c6850e51ecbadb65bbedb5395664", 59788),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200156,
			           "swordres.rif", "babe2ab6c352bdeb0fc256a94c934bb8", 58388),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY3s("compacts.clm", "3a77d3cd0ca6c91a4f4e94459b1da773", 200852,
			           "speech.clu",   "36919b35067bf56b68ad538732a618c2", 45528200,
			           "intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
			           "speech.dat",   "4b4123155288d2c17da2c97b8b72ee6e", 14598144),
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
			           "speech.dat",   "a8b0813e172f008714eb1a6d715115ab", 14014464),
			Common::IT_ITA,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("text.clu",     "76f93f5feecc8915435105478f3c6615", 3193923,
			           "history.smk",  "c3f85299b685d49a01ed8a2ab51f0cb5", 3374148,
			           "intro.smk",    "6689aa8f84cb0387b292481d2a2428b4", 13076700),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "d21d6321ee2dbb2d7d7ca2d2a940c34a", 58916,
			           "intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"Rerelease",
			AD_ENTRY3s("text.clu",     "76f93f5feecc8915435105478f3c6615", 3193159,
			           "intro.txt",    "c1215c3a92cf7e14104a8ddfda458bb1", 198,
			           "1m2.wav",      "6b43257c8b22decfc7f289da7b653d57", 846792),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"GoG.com",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "5463362dc77b6efc36e46ac84998bd2f", 59788,
			           "1m2.ogg",      "f51b159c89b0c59217ccda6795da3e15", 137715),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "c7df52094d590b568a4ed35b70390d9e", 58916),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "b0ae5a47aba74dc0acb3442d4c84b225", 58916),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "08d4942cf7c904182a31a1d5333244f3", 59788,
			           "intro.smk",    "95071cd6c12c10c9a30f45a70384cf05", 13448344),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "08d4942cf7c904182a31a1d5333244f3", 59788,
			           "intro.smk",    "a8c6a8770cb4b2669f4263ece8830985", 13293740),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "239bdd76c405bad0f804a8ae5df4adb0", 59788),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"English speech",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "34c111f224e75050a523dc758c71d54e", 60612),
			Common::PT_PRT,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"English speech",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "ba6f881c3ace6408880c8e07cd8a1dfe", 59788),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"English speech",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "88c0793a4fa908083b00f6677c545f78", 58916),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"English speech",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "finale.txt",   "2d415af7012c2d76b5cc4e67548bc836", 246,
			           "intro.txt",    "0ceae1bbd3da45b9a0452118a13bd8ae", 169),
			Common::HU_HUN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"English speech",
			AD_ENTRY3s("finale.txt",   "c88bb433473fe9171bca4471ebf82f9b", 238,
			           "history.txt",  "e973f8667d3729de1ed2c313ce524990", 1098,
			           "intro.txt",    "b9c6d2b274ea093feeecd50d0736f679", 188),
			Common::CS_CZE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"Akella",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "e7021abec62dd774010d1f432ef9f03a", 58916),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"Mediahauz",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "7a6e896064c8d2ee266e961549487204", 58916),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"ND",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "b5d9ddbe26d453415a43596f86452435", 59788),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clm", "3a77d3cd0ca6c91a4f4e94459b1da773", 200852,
			           "paris2.clm",   "6e096da5dc806b3b554fa371f6e0d749", 23981648,
			           "credits.smk",  "eacbc81d3ef88628d3710abbbcdc9aa0", 17300736),
			Common::EN_USA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clm", "3a77d3cd0ca6c91a4f4e94459b1da773", 200852,
			           "swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788,
			           "credits.smk",  "9a3fe9cb76bc7ca8a9987c173befb90d", 16315740),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "2ec14f1f262cdd2c87dd95acced9e2f6", 3312),
			Common::EN_USA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "69349710eef6b653ed2c02643ed6c4a0", 2799),
			Common::EN_GRB,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "0b119d49f27260e6115504c135b9bb19", 2382),
			Common::FR_FRA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "c4f84aaa17f80fb549a5c8a867a9836a", 2382),
			Common::DE_DEU,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "949806fa3eaa4ff3a6c19ee4b5caa9f5", 2823,
			           "speech.dat",   "73348cb38e6ddd444100806c45febb18", 187961344),
			Common::IT_ITA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword1",
			"",
			AD_ENTRY3s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "credits.dat",  "cd97e8f5006d91914904b3bfdb0ff588", 2412,
			           "speech.dat",   "be1f9c4fd157711c2a0f6e020352334d", 182177792),
			Common::ES_ESP,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},


	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Sword1
