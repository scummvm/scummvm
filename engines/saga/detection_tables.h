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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

namespace Saga {

static const GameResourceDescription ITE_Resources = {
	RID_ITE_SCENE_LUT,  // Scene lookup table RN
	RID_ITE_SCRIPT_LUT, // Script lookup table RN
	RID_ITE_MAIN_PANEL,
	RID_ITE_CONVERSE_PANEL,
	RID_ITE_OPTION_PANEL,
	RID_ITE_MAIN_SPRITES,
	RID_ITE_MAIN_PANEL_SPRITES,
	RID_ITE_DEFAULT_PORTRAITS,
	RID_ITE_MAIN_STRINGS,
	RID_ITE_ACTOR_NAMES
};

static const GameResourceDescription ITEDemo_Resources = {
	RID_ITEDEMO_SCENE_LUT,  // Scene lookup table RN
	RID_ITEDEMO_SCRIPT_LUT, // Script lookup table RN
	RID_ITEDEMO_MAIN_PANEL,
	RID_ITEDEMO_CONVERSE_PANEL,
	RID_ITEDEMO_OPTION_PANEL,
	RID_ITEDEMO_MAIN_SPRITES,
	RID_ITEDEMO_MAIN_PANEL_SPRITES,
	RID_ITEDEMO_DEFAULT_PORTRAITS,
	RID_ITEDEMO_MAIN_STRINGS,
	RID_ITEDEMO_ACTOR_NAMES
};

// Inherit the Earth - DOS Demo version
static const GameFontDescription ITEDEMO_GameFonts[] = {
	{0},
	{1}
};

static const GameSoundInfo ITEDEMO_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Win32 Demo version

static const GameFontDescription ITEWINDEMO_GameFonts[] = {
	{2},
	{0}
};

static const GameSoundInfo ITEWINDEMO1_GameSound = {
	kSoundPCM,
	22050,
	8,
	false,
	false,
	false
};

static const GameSoundInfo ITEWINDEMO2_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static const GameSoundInfo ITEWINDEMO2_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Mac Demo version
static const GameSoundInfo ITEMACDEMO_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static const GameSoundInfo ITEMACDEMO_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static const GameSoundInfo ITEMACDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Linux Demo version
static const GameSoundInfo ITELINDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

static const GameSoundInfo ITEMACCD_G_GameSound = {
	kSoundMacPCM,
	22050,
	8,
	false,
	false,
	false
};

// Inherit the Earth - Mac Wyrmkeep version
static const GameSoundInfo ITEMACCD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static const GameSoundInfo ITEMACCD_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

// Inherit the Earth - Diskette version
static const GameFontDescription ITEDISK_GameFonts[] = {
	{2},
	{0},
	{1}
};

static const GameSoundInfo ITEDISK_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

static const GameFontDescription ITECD_GameFonts[] = {
	{2},
	{0},
	{1}
};

static const GameSoundInfo ITECD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

static const GamePatchDescription ITEWinPatch1_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkhall.mid", GAME_RESOURCEFILE, 12, NULL},
	{ "mouse.mid", GAME_RESOURCEFILE, 13, NULL},
	{ "darkclaw.mid", GAME_RESOURCEFILE, 14, NULL},
	{ "birdchrp.mid", GAME_RESOURCEFILE, 15, NULL},
	{ "orbtempl.mid", GAME_RESOURCEFILE, 16, NULL},
	{ "spooky.mid", GAME_RESOURCEFILE, 17, NULL},
	{ "catfest.mid", GAME_RESOURCEFILE, 18, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "boarking.mid", GAME_RESOURCEFILE, 22, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "exploreb.mid", GAME_RESOURCEFILE, 24, NULL},
	{ "explorec.mid", GAME_RESOURCEFILE, 25, NULL},
	{ "sunstatm.mid", GAME_RESOURCEFILE, 26, NULL},
	{ "nitstrlm.mid", GAME_RESOURCEFILE, 27, NULL},
	{ "humruinm.mid", GAME_RESOURCEFILE, 28, NULL},
	{ "damexplm.mid", GAME_RESOURCEFILE, 29, NULL},
	{ "tychom.mid", GAME_RESOURCEFILE, 30, NULL},
	{ "kitten.mid", GAME_RESOURCEFILE, 31, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},
	{ "brutalmt.mid", GAME_RESOURCEFILE, 33, NULL},
	{ "shiala.mid", GAME_RESOURCEFILE, 34, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.voc", GAME_VOICEFILE, 4, NULL}
};

static const GamePatchDescription ITEWinPatch2_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
/*	boarhall.bbm
	elkenter.bbm
	ferrets.bbm
	ratdoor.bbm
	sanctuar.bbm
	tycho.bbm*/
};

static const GamePatchDescription ITEMacPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3m.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4m.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITEMACCD_GameSound}
};

static const GamePatchDescription ITELinPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "P2_A.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
};

// IHNM section

static const GameResourceDescription IHNM_Resources = {
	RID_IHNM_SCENE_LUT,  // Scene lookup table RN
	RID_IHNM_SCRIPT_LUT, // Script lookup table RN
	RID_IHNM_MAIN_PANEL,
	RID_IHNM_CONVERSE_PANEL,
	RID_IHNM_OPTION_PANEL,
	RID_IHNM_MAIN_SPRITES,
	RID_IHNM_MAIN_PANEL_SPRITES,
	0,
	RID_IHNM_MAIN_STRINGS,
	0
};

static const GameFontDescription IHNMDEMO_GameFonts[] = {
	{2},
	{3},
	{4}
};

static const GameFontDescription IHNMCD_GameFonts[] = {
	{2},
	{3},
	{4},
	{5},
	{6},  // kIHNMFont8
	{7},
	{8}   // kIHNMMainFont
};

static const GameSoundInfo IHNM_GameSound = {
	kSoundWAV,
	-1,
	-1,
	false,
	false,
	true
};

static const SAGAGameDescription gameDescriptions[] = {
	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		{
			"ite",
			"Demo", // Game title
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"986c79c4d2939dbe555576529fd37932", -1},
	//{"ite.dmo",	GAME_DEMOFILE},						"0b9a70eb4e120b6f00579b46c8cae29e"
	{"scripts.rsc", GAME_SCRIPTFILE,					"d5697dd3240a3ceaddaa986c47e1a2d7", -1},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c58e67c506af4ffa03fd0aac2079deb0", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		0, // features
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITEDemo_Resources,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		&ITEDEMO_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - MAC Demo version 2
	{
		{
			"ite",
			"Demo 2",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c", -1},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4", -1},
	{"musicd.rsc",		GAME_MUSICFILE,		"495bdde51fd9f4bea2b9c911091b1ab2", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_MACDEMO2,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACDEMO_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - MAC Demo version 1
	{
		{
			"ite",
			"Demo 1",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c", -1},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4", -1},
	{"musicd.rsc",		GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_MACDEMO1,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - MAC CD Guild version
	{
		{
			"ite",
			"CD",
			{
	{"ite resources.bin",	GAME_RESOURCEFILE | GAME_MACBINARY,	"0bd506aa887bfc7965f695c6bd28237d", -1},
	{"ite scripts.bin",		GAME_SCRIPTFILE | GAME_MACBINARY,	"af0d7a2588e09ad3ecbc5b474ea238bf", -1},
	{"ite sounds.bin",		GAME_SOUNDFILE | GAME_MACBINARY,	"441426c6bb2a517f65c7e49b57f7a345", -1},
	{"ite music.bin",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"c1d20324b7cdf1650e67061b8a93251c", -1},
	{"ite voices.bin",		GAME_VOICEFILE | GAME_MACBINARY,	"dba92ae7d57e942250fe135609708369", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_MACCD_G,
		GF_BIG_ENDIAN_DATA | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_G_GameSound,
		&ITEMACCD_G_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		{
			"ite",
			"Wyrmkeep CD",
			{
	{"ite.rsc",						GAME_RESOURCEFILE,	"4f7fa11c5175980ed593392838523060", -1},
	{"scripts.rsc",					GAME_SCRIPTFILE,	"adf1f46c1d0589083996a7060c798ad0", -1},
	{"sounds.rsc",					GAME_SOUNDFILE,		"95863b89a0916941f6c5e1789843ba14", -1},
	{"inherit the earth voices",	GAME_VOICEFILE,		"c14c4c995e7a0d3828e3812a494301b7", -1},
	{"music.rsc",					GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_MACCD,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		{
			"ite",
			"Demo",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c", 2026769},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4", -1},
	{"musicd.rsc",		GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_LINDEMO,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		&ITELINDEMO_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
	},

	// Inherit the earth - Win32 Demo version 3
	{
		{
			"ite",
			"Demo 3",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c", 2005074},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4", -1},
	{"musicd.rsc",		GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_WINDEMO3,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		&ITELINDEMO_GameMusic,
		ARRAYSIZE(ITEWinPatch2_Files),
		ITEWinPatch2_Files,
	},

	// Inherit the earth - Win32 Demo version 2
	{
		{
			"ite",
			"Demo 2",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c", 2005074},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_WINDEMO2,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch2_Files),
		ITEWinPatch2_Files,
	},

	// Inherit the earth - Win32 Demo version 1
	{
		{
			"ite",
			"Demo 1",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", -1},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", -1},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"a741139dd7365a13f463cd896ff9969a", -1},
	{"voicesd.rsc",		GAME_VOICEFILE,		"0759eaf5b64ae19fd429920a70151ad3", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			Common::ADGF_DEMO
		},
		GType_ITE,
		GID_ITE_WINDEMO1,
		GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO1_GameSound,
		&ITEWINDEMO1_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
	},

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD

	// version is different from the other Wyrmkeep re-releases in that it does
	// not have any substitute files. Presumably the ite.rsc file has been
	// modified to include the Wyrmkeep changes. The resource files are little-
	// endian, except for the voice file which is big-endian.
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
	{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9", -1},
	{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",					GAME_SOUNDFILE,						"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"inherit the earth voices",	GAME_VOICEFILE | GAME_SWAPENDIAN,	"c14c4c995e7a0d3828e3812a494301b7", -1},
	{"music.rsc",					GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformUnknown,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_MULTICD,
		GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITEMACCD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - Wyrmkeep Linux CD version
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6", -1},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_LINCD,
		GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
	},

	// Inherit the earth - Wyrmkeep Windows CD version
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_WINCD,
		GF_WYRMKEEP | GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
	},

	// Inherit the earth - DOS CD version
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_CD_G,
		GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD version with digital music
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6", -1},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_CD_G2,
		GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD German version
	// reported by mld. Bestsellergamers cover disk
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_CD_DE,
		GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD German version with digital music
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718", -1},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_CD_DE2,
		GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - CD version
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe", -1},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_CD,
		GF_CD_FX,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - German Floppy version
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_DISK_DE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - German Floppy version with digital music
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08", -1},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_DISK_DE2,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - Disk version
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_DISK_G,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - Disk version with digital music
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d", -1},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_ITE,
		GID_ITE_DISK_G2,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		{
			"ihnm",
			"Demo",
			{
	{"scream.res",		GAME_RESOURCEFILE,	"46bbdc65d164ba7e89836a0935eec8e6", -1},
	{"scripts.res",		GAME_SCRIPTFILE,	"9626bda8978094ff9b29198bc1ed5f9a", -1},
	{"sfx.res",			GAME_SOUNDFILE,		"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicesd.res",		GAME_VOICEFILE,		"3bbc16a8f741dbb511da506c660a0b54", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_DEMO
		},
		GType_IHNM,
		GID_IHNM_DEMO,
		0,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNMDEMO_GameFonts),
		IHNMDEMO_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicess.res",	GAME_VOICEFILE,						"54b1f2013a075338ceb0e258d97808bd", -1}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"fc6440b38025f4b2cc3ff55c3da5c3eb", -1},
	{"voices2.res",	GAME_VOICEFILE,						"b37f10fd1696ade7d58704ccaaebceeb", -1},
	{"voices3.res",	GAME_VOICEFILE,						"3bbc16a8f741dbb511da506c660a0b54", -1},
	{"voices4.res",	GAME_VOICEFILE,						"ebfa160122d2247a676ca39920e5d481", -1},
	{"voices5.res",	GAME_VOICEFILE,						"1f501ce4b72392bdd1d9ec38f6eec6da", -1},
	{"voices6.res",	GAME_VOICEFILE,						"f580ed7568c7d6ef34e934ba20adf834", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_IHNM,
		GID_IHNM_CD,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - De CD version
	// Censored CD version (without Nimdok)
	// Reported by mld. German Retail
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicess.res",	GAME_VOICEFILE,						"8b09a196a52627cacb4eab13bfe0b2c3", -1}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2", -1},
	{"voices2.res",	GAME_VOICEFILE,						"c270e0980782af43641a86e4a14e2a32", -1},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9", -1},
	{"voices5.res",	GAME_VOICEFILE,						"c477443c52a0aa56e686ebd8d051e4ab", -1},
	{"voices6.res",	GAME_VOICEFILE,						"2b9aea838f74b4eecfb29a8f205a2bd4", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_IHNM,
		GID_IHNM_CD_DE,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Sp CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicess.res",	GAME_VOICEFILE,						"d869de9883c8faea7f687217a9ec7057", -1}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"dc6a34e3d1668730ea46815a92c7847f", -1},
	{"voices2.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c", -1},
	{"voices3.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c", -1},
	{"voices4.res",	GAME_VOICEFILE,						"0f87400b804232a58dd22e404420cc45", -1},
	{"voices5.res",	GAME_VOICEFILE,						"172668cfc5d8c305cb5b1a9b4d995fc0", -1},
	{"voices6.res",	GAME_VOICEFILE,						"96c9bda9a5f41d6bc232ed7bf6d371d9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_IHNM,
		GID_IHNM_CD_ES,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Ru CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicess.res",	GAME_VOICEFILE,						"9df7cd3b18ddaa16b5291b3432567036", -1}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"d6100d2dc3b2b9f2e1ad247f613dce9b", -1},
	{"voices2.res",	GAME_VOICEFILE,						"84f6f48ecc2832841ea6417a9a379430", -1},
	{"voices3.res",	GAME_VOICEFILE,						"ebb9501283047f27a0f54e27b3c8ba1e", -1},
	{"voices4.res",	GAME_VOICEFILE,						"4c145da5fa6d1306162a7ca8ce5a4f2e", -1},
	{"voices5.res",	GAME_VOICEFILE,						"871a559644281917677eca4af1b05620", -1},
	{"voices6.res",	GAME_VOICEFILE,						"211be5c24f066d69a2f6cfa953acfba6", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_IHNM,
		GID_IHNM_CD_RU,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Fr CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{"voicess.res",	GAME_VOICEFILE,						"b8642e943bbebf89cef2f48b31cb4305", -1}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2", -1},
	{"voices2.res",	GAME_VOICEFILE,						"c2d93a35d2c2def9c3d6d242576c794b", -1},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9", -1},
	{"voices5.res",	GAME_VOICEFILE,						"f4c415de7c03de86b73f9a12b8bd632f", -1},
	{"voices6.res",	GAME_VOICEFILE,						"3fc5358a5d8eee43bdfab2740276572e", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_IHNM,
		GID_IHNM_CD_FR,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},
	{ AD_TABLE_END_MARKER, 0, 0, 0, 0, NULL, 0, NULL, NULL, NULL, NULL, 0, NULL }
};

} // End of namespace Saga
