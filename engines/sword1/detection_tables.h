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

static const ADGameDescription gameDescriptions[] = {
	{
		"sword1",
		"Demo",
		AD_ENTRY4s("clusters/scripts.clu",  "9f6de3bea49a1ef4d8b1b020c41c950e", 1070644,
				   "clusters/swordres.rif", "bc01bc995f23e46bb076f66ba5c516c4", 58388,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 2696228,
				   "smackshi/intro.smk",    "f50d773c362d03a52a6a4d541d09449c", 13298480),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"Demo",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088232,
				   "clusters/swordres.rif", "3786c6850e51ecbadb65bbedb5395664", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3186195,
				   "smackshi/intro.smk",    "95071cd6c12c10c9a30f45a70384cf05", 13448344),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"Demo",
		AD_ENTRY4s("clusters/scripts.clu",  "9f6de3bea49a1ef4d8b1b020c41c950e", 1070644,
				   "clusters/swordres.rif", "babe2ab6c352bdeb0fc256a94c934bb8", 58388,
				   "clusters/text.clu",     "5d5bf40629364115da603da378e9d4c9", 2685487,
				   "smackshi/intro.smk",    "f50d773c362d03a52a6a4d541d09449c", 13298480),
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"Demo",
		AD_ENTRY4s("clusters/scripts.clm",  "6b6d9a32668e6f0285318dbe33f167fe", 1088468,
				   "clusters/swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788,
				   "speech/speech.clu",     "36919b35067bf56b68ad538732a618c2", 45528200,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"Demo",
		AD_ENTRY3s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
				   "english/speech.inf",   "57f6d6949262cd63fc0378dd2375c819", 1662),
		Common::EN_ANY,
		Common::kPlatformPSX,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"Demo",
		AD_ENTRY3s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
				   "italian/speech.inf",   "af982fbfd4fdd39ea7108dc8f77cf1b3", 1652),
		Common::IT_ITA,
		Common::kPlatformPSX,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088232,
				   "clusters/swordres.rif", "08d4942cf7c904182a31a1d5333244f3", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3193923,
				   "smackshi/intro.smk",    "6689aa8f84cb0387b292481d2a2428b4", 13076700),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087240,
				   "clusters/swordres.rif", "d21d6321ee2dbb2d7d7ca2d2a940c34a", 58916,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 2704592,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"Rerelease",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088292,
				   "clusters/swordres.rif", "5463362dc77b6efc36e46ac84998bd2f", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3193159,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"GOG.com",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088292,
				   "clusters/swordres.rif", "5463362dc77b6efc36e46ac84998bd2f", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3193159,
				   "video/intro.dxa",       "e27cd33593c08b66e8d20fbc40938789", 7420364),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087984,
				   "clusters/swordres.rif", "c7df52094d590b568a4ed35b70390d9e", 58916,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 2705446,
				   "smackshi/intro.smk",    "d602a28f5f5c583bf9870a23a94a9bc5", 13525168),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087240,
				   "clusters/swordres.rif", "b0ae5a47aba74dc0acb3442d4c84b225", 58916,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 2705446,
				   "smackshi/intro.smk",    "d1d0e958aeef9b1375b55df8f8831f26", 13281776),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088232,
				   "clusters/swordres.rif", "08d4942cf7c904182a31a1d5333244f3", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3193923,
				   "smackshi/intro.smk",    "95071cd6c12c10c9a30f45a70384cf05", 13448344),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{ // Alternate version
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088232,
				   "clusters/swordres.rif", "08d4942cf7c904182a31a1d5333244f3", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3193923,
				   "smackshi/intro.smk",    "a8c6a8770cb4b2669f4263ece8830985", 13293740),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "clusters/swordres.rif", "239bdd76c405bad0f804a8ae5df4adb0", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3188725,
				   "smackshi/intro.smk",    "83060041aa155d802e51b7211b62ea2f", 13525252),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"English speech",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088468,
				   "clusters/swordres.rif", "34c111f224e75050a523dc758c71d54e", 60612,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3164478,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::PT_PRT,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"English speech",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088292,
				   "clusters/swordres.rif", "ba6f881c3ace6408880c8e07cd8a1dfe", 59788,
				   "clusters/text.clu",     "0c0f9eadf20a497834685ccb3ba53a3f", 397478,
				   "video/intro.smk",       "d07ba8a1be7d8a47de50cc4eac2bc243", 13082688),
		Common::HE_ISR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"English speech",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087240,
				   "clusters/swordres.rif", "88c0793a4fa908083b00f6677c545f78", 58916,
				   "clusters/text.clu",     "7d9e47533fde5333dc310bfd73eaeb5c", 2666944,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"English speech",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088292,
				   "clusters/swordres.rif", "5463362dc77b6efc36e46ac84998bd2f", 59788,
				   "clusters/text.clu",     "cf6a85c2d60386a3c978f0c6fbb377bd", 3193159,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::HU_HUN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"English speech",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "clusters/swordres.rif", "239bdd76c405bad0f804a8ae5df4adb0", 59788,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3199652,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::CS_CZE,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"Akella",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087240,
				   "clusters/swordres.rif", "e7021abec62dd774010d1f432ef9f03a", 58916,
				   "clusters/text.clu",     "524706e42583f6c23a5a7ae3e1784068", 2683625,
				   "smackshi/intro.smk",    "ef3ae780668c087fae00ed9c46c2eb35", 13386716),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"Mediahauz",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1087240,
				   "clusters/swordres.rif", "7a6e896064c8d2ee266e961549487204", 58916,
				   "clusters/text.clu",     "76f93f5feecc8915435105478f3c6615", 3198686,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"Novy Disk",
		AD_ENTRY4s("clusters/scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088292,
				   "clusters/swordres.rif", "b5d9ddbe26d453415a43596f86452435", 59788,
				   "clusters/text.clu",     "8392ae2af0a8bec1dca511b2fedddc4c", 3178811,
				   "video/intro.dxa",       "e27cd33593c08b66e8d20fbc40938789", 7420364),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clm",  "6b6d9a32668e6f0285318dbe33f167fe", 1088468,
				   "clusters/swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788,
				   "smackshi/credits.smk",  "eacbc81d3ef88628d3710abbbcdc9aa0", 17300736,
				   "smackshi/intro.smk",    "6689aa8f84cb0387b292481d2a2428b4", 13076700),
		Common::EN_USA,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clm",  "6b6d9a32668e6f0285318dbe33f167fe", 1088468,
				   "clusters/swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788,
				   "smackshi/credits.smk",  "9a3fe9cb76bc7ca8a9987c173befb90d", 16315740,
				   "smackshi/intro.smk",    "d82a7869ace8fcecaa519c04c4bfc483", 13233268),
		Common::EN_GRB,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("clusters/scripts.clm",  "6b6d9a32668e6f0285318dbe33f167fe", 1088468,
				   "clusters/swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788,
				   "smackshi/credits.smk",  "0e4eb849d60baab975130efd35f15ace", 17528016,
				   "smackshi/intro.smk",    "d602a28f5f5c583bf9870a23a94a9bc5", 13525168),
		Common::FR_FRA,
		Common::kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088468,
				   "swordres.rif", "a810e6dc5c8e636151a3e1370d41d138", 59788,
				   "credits.dat",  "2ec14f1f262cdd2c87dd95acced9e2f6", 3312,
				   "speech.inf",   "ed14c2a235cf5388ac3b5f29db129837", 21310),
		Common::EN_USA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088468,
				   "swordres.rif", "a810e6dc5c8e636151a3e1370d41d138", 59788,
				   "credits.dat",  "69349710eef6b653ed2c02643ed6c4a0", 2799,
				   "speech.inf",   "ed14c2a235cf5388ac3b5f29db129837", 21310),
		Common::EN_GRB,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu" , "72b10193714e8c6e4daca51791c0db0c", 1088468,
				   "swordres.rif", "a810e6dc5c8e636151a3e1370d41d138", 59788,
				   "credits.dat",  "0b119d49f27260e6115504c135b9bb19", 2382,
				   "speech.inf",   "2ccb9be1a3d8d0e33d6efd6a12a24320", 21450),
		Common::FR_FRA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088468,
				   "swordres.rif", "a810e6dc5c8e636151a3e1370d41d138", 59788,
				   "credits.dat",  "c4f84aaa17f80fb549a5c8a867a9836a", 2382,
				   "speech.inf",   "403fb61f9de6ce6cb374edd9985066ae", 21304),
		Common::DE_DEU,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
				   "credits.dat",  "949806fa3eaa4ff3a6c19ee4b5caa9f5", 2823,
				   "speech.inf",   "1165f01823e4d2df72fcc5b592a4960e", 21374),
		Common::IT_ITA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword1",
		"",
		AD_ENTRY4s("scripts.clu",  "72b10193714e8c6e4daca51791c0db0c", 1088372,
				   "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788,
				   "credits.dat",  "cd97e8f5006d91914904b3bfdb0ff588", 2412,
				   "speech.inf",   "d4558d96ce696a906b086c2b44ffb301", 21342),
		Common::ES_ESP,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Sword1
