/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/file.h"
#include "common/md5.h"
#include "common/map.h"
#include "base/plugins.h"
#include "base/gameDetector.h"
#include "backends/fs/fs.h"

#include "saga/rscfile_mod.h"
#include "saga/interface.h"
#include "saga/scene.h"

#include "saga/game.h"

namespace Saga {

static int detectGame(const FSList &fslist, bool mode = false);

// Inherit the Earth - DOS Demo version

static GAME_FILEDESC ITEDEMO_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"ite.dmo", GAME_DEMOFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"voices.rsc", GAME_SOUNDFILE | GAME_VOICEFILE}
};

static GAME_FONTDESC ITEDEMO_GameFonts[] = {
	{GAME_FONT_SMALL, 0},
	{GAME_FONT_MEDIUM, 1}
}; 

static GAME_SOUNDINFO ITEDEMO_GameSound = {
	GAME_SOUND_VOC, 0, 0, 0
};

static GAME_FONTDESC ITEMACDEMO_GameFonts[] = {
	{GAME_FONT_MEDIUM, 0},
	{GAME_FONT_SMALL, 2}
}; 

// Inherit the Earth - Wyrmkeep Win32 Demo version
static GAME_FILEDESC ITEWINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE}
};

// Inherit the Earth -  Wyrmkeep Linux Demo version
static GAME_FILEDESC ITELINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE},
	{"musicd.rsc", GAME_MUSICFILE}
};

// Inherit the Earth -  Wyrmkeep Linux version
static GAME_FILEDESC ITELINCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"voices.rsc", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
};

static GAME_FONTDESC ITEWINDEMO_GameFonts[] = {
	{GAME_FONT_MEDIUM, 0},
	{GAME_FONT_SMALL, 2}
}; 

// Inherit the Earth - Mac Wyrmkeep version
static GAME_FILEDESC ITEMACCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"Inherit the Earth Voices", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
};

// Inherit the Earth - Diskette version
static GAME_FILEDESC ITEDISK_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"voices.rsc", GAME_SOUNDFILE | GAME_VOICEFILE}
};

static GAME_FONTDESC ITEDISK_GameFonts[] = {
	{GAME_FONT_MEDIUM, 0},
	{GAME_FONT_LARGE, 1},
	{GAME_FONT_SMALL, 2}
};

static GAME_RESOURCEDESC ITE_Resources = {
	RID_ITE_SCENE_LUT,  // Scene lookup table RN
	RID_ITE_SCRIPT_LUT, // Script lookup table RN
	RID_ITE_COMMAND_PANEL,
	RID_ITE_DIALOGUE_PANEL
};

static GAME_SOUNDINFO ITE_GameSound = {
	GAME_SOUND_VOC, 0, 0, 0
};

// Inherit the Earth - CD Enhanced version
static GAME_FILEDESC ITECD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"voices.rsc", GAME_VOICEFILE}
};

static GAME_FONTDESC ITECD_GameFonts[] = {
	{GAME_FONT_MEDIUM, 0},
	{GAME_FONT_LARGE, 1},
	{GAME_FONT_SMALL, 2}
};

static GAME_SOUNDINFO ITECD_GameSound = {
	GAME_SOUND_PCM,
	22050,
	16,
	0
};

// I Have No Mouth and I Must Scream - Demo version
static GAME_FILEDESC IHNMDEMO_GameFiles[] = {
	{"scream.res", GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicesd.res", GAME_VOICEFILE}
};

// I Have No Mouth and I Must Scream - Retail CD version
static GAME_FILEDESC IHNMCD_GameFiles[] = {
	{"musicfm.res", GAME_MUSICFILE_FM},
	{"musicgm.res", GAME_MUSICFILE_GM},
	{"scream.res", GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voices1.res", GAME_VOICEFILE},
	{"voices2.res", GAME_VOICEFILE},
	{"voices3.res", GAME_VOICEFILE},
	{"voices4.res", GAME_VOICEFILE},
	{"voices5.res", GAME_VOICEFILE},
	{"voices6.res", GAME_VOICEFILE},
	{"voicess.res", GAME_VOICEFILE}
};

static GAME_FONTDESC IHNMCD_GameFonts[] = {
	{GAME_FONT_MEDIUM, 2},
	{GAME_FONT_LARGE, 3},
	{GAME_FONT_SMALL, 4},
	{GAME_FONT_SMALL2, 5},
	{GAME_FONT_MEDIUM2, 6},
	{GAME_FONT_LARGE2, 7},
	{GAME_FONT_LARGE3, 8}
};

static GAME_RESOURCEDESC IHNM_Resources = {
	RID_IHNM_SCENE_LUT,  // Scene lookup table RN
	RID_IHNM_SCRIPT_LUT, // Script lookup table RN
	RID_IHNM_COMMAND_PANEL,
	RID_IHNM_DIALOGUE_PANEL
};

static GAME_SOUNDINFO IHNM_GameSound = {
	GAME_SOUND_WAV, 0, 0, 0
};

struct GAME_MD5 {
	GAME_IDS id;
	const char *md5;
	const char *filename;
};

#define FILE_MD5_BYTES 5000

static GAME_MD5 game_md5[] = {
	{ GID_ITE_DISK_G,   "8f4315a9bb10ec839253108a032c8b54", "ite.rsc" },
	{ GID_ITE_DISK_G,   "516f7330f8410057b834424ea719d1ef", "scripts.rsc" },
	{ GID_ITE_DISK_G,   "c46e4392fcd2e89bc91e5567db33b62d", "voices.rsc" },

	{ GID_ITE_CD_G,     "8f4315a9bb10ec839253108a032c8b54", "ite.rsc" },
	{ GID_ITE_CD_G,     "50a0d2d7003c926a3832d503c8534e90", "scripts.rsc" },
	{ GID_ITE_CD_G,     "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc" },
	{ GID_ITE_CD_G,     "41bb6b95d792dde5196bdb78740895a6", "voices.rsc" },

	{ GID_ITE_CD,       "8f4315a9bb10ec839253108a032c8b54", "ite.rsc" },
	{ GID_ITE_CD,       "a891405405edefc69c9d6c420c868b84", "scripts.rsc" },
	{ GID_ITE_CD,       "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc" },
	{ GID_ITE_CD,       "41bb6b95d792dde5196bdb78740895a6", "voices.rsc" },

	{ GID_ITE_DEMO_G,   "986c79c4d2939dbe555576529fd37932", "ite.rsc" },
	{ GID_ITE_DEMO_G,   "d5697dd3240a3ceaddaa986c47e1a2d7", "scripts.rsc" },
	{ GID_ITE_DEMO_G,   "c58e67c506af4ffa03fd0aac2079deb0", "voices.rsc" },
	{ GID_ITE_DEMO_G,   "0b9a70eb4e120b6f00579b46c8cae29e", "ite.dmo" },

	{ GID_ITE_MACCD,    "4f7fa11c5175980ed593392838523060", "ite.rsc" },
	{ GID_ITE_MACCD,    "adf1f46c1d0589083996a7060c798ad0", "scripts.rsc" },
	{ GID_ITE_MACCD,    "1a91cd60169f367ecb6c6e058d899b2f", "music.rsc" },
	{ GID_ITE_MACCD,    "95863b89a0916941f6c5e1789843ba14", "sounds.rsc" },
	{ GID_ITE_MACCD,    "c14c4c995e7a0d3828e3812a494301b7", "Inherit the Earth Voices" },

	{ GID_ITE_LINCD,    "8f4315a9bb10ec839253108a032c8b54", "ite.rsc" },
	{ GID_ITE_LINCD,    "a891405405edefc69c9d6c420c868b84", "scripts.rsc" },
	{ GID_ITE_LINCD,    "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc" },
	{ GID_ITE_LINCD,    "41bb6b95d792dde5196bdb78740895a6", "voices.rsc" },
	{ GID_ITE_LINCD,    "d6454756517f042f01210458abe8edd4", "music.rsc" },

	{ GID_ITE_DISK_DE,  "869fc23c8f38f575979ec67152914fee", "ite.rsc" },
	{ GID_ITE_DISK_DE,  "516f7330f8410057b834424ea719d1ef", "scripts.rsc" },
	{ GID_ITE_DISK_DE,  "0c9113e630f97ef0996b8c3114badb08", "voices.rsc" },

	{ GID_ITE_WINDEMO2, "3a450852cbf3c80773984d565647e6ac", "ited.rsc" },
	{ GID_ITE_WINDEMO2, "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc" },
	{ GID_ITE_WINDEMO2, "95a6c148e22e99a8c243f2978223583c", "soundsd.rsc" },
	{ GID_ITE_WINDEMO2, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc" },

	{ GID_ITE_LINDEMO,  "3a450852cbf3c80773984d565647e6ac", "ited.rsc" },
	{ GID_ITE_LINDEMO,  "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc" },
	{ GID_ITE_LINDEMO,  "d6454756517f042f01210458abe8edd4", "musicd.rsc" },
	{ GID_ITE_LINDEMO,  "95a6c148e22e99a8c243f2978223583c", "soundsd.rsc" },
	{ GID_ITE_LINDEMO,  "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc" },

	{ GID_ITE_MACDEMO2, "addfc9d82bc2fa1f4cab23743c652c08", "ited.rsc" },
	{ GID_ITE_MACDEMO2, "fded5c59b8b7c5976229f960d21e6b0b", "scriptsd.rsc" },
	{ GID_ITE_MACDEMO2, "495bdde51fd9f4bea2b9c911091b1ab2", "musicd.rsc" },
	{ GID_ITE_MACDEMO2, "b3a831fbed337d1f1300fee1dd474f6c", "soundsd.rsc" },
	{ GID_ITE_MACDEMO2, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc" },

	{ GID_ITE_WINDEMO1, "3a450852cbf3c80773984d565647e6ac", "ited.rsc" },
	{ GID_ITE_WINDEMO1, "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc" },
	{ GID_ITE_WINDEMO1, "a741139dd7365a13f463cd896ff9969a", "soundsd.rsc" },
	{ GID_ITE_WINDEMO1, "0759eaf5b64ae19fd429920a70151ad3", "voicesd.rsc" },

	{ GID_ITE_MACDEMO1, "addfc9d82bc2fa1f4cab23743c652c08", "ited.rsc" },
	{ GID_ITE_MACDEMO1, "fded5c59b8b7c5976229f960d21e6b0b", "scriptsd.rsc" },
	{ GID_ITE_MACDEMO1, "1a91cd60169f367ecb6c6e058d899b2f", "musicd.rsc" },
	{ GID_ITE_MACDEMO1, "b3a831fbed337d1f1300fee1dd474f6c", "soundsd.rsc" },
	{ GID_ITE_MACDEMO1, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc" },

	// My CD also has a file called 'patch.re_'. I assume it contains
	// bugfixes, but let's ignore it for now.
	{ GID_IHNM_CD, "0439083e3dfdc51b486071d45872ae52", "musicfm.res" },
	{ GID_IHNM_CD, "80f875a1fb384160d1f4b27166eef583", "musicgm.res" },
	{ GID_IHNM_CD, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res" },
	{ GID_IHNM_CD, "be38bbc5a26be809dbf39f13befebd01", "scripts.res" },
	{ GID_IHNM_CD, "1c610d543f32ec8b525e3f652536f269", "sfx.res" },
	{ GID_IHNM_CD, "fc6440b38025f4b2cc3ff55c3da5c3eb", "voices1.res" },
	{ GID_IHNM_CD, "b37f10fd1696ade7d58704ccaaebceeb", "voices2.res" },
	{ GID_IHNM_CD, "3bbc16a8f741dbb511da506c660a0b54", "voices3.res" },
	{ GID_IHNM_CD, "ebfa160122d2247a676ca39920e5d481", "voices4.res" },
	{ GID_IHNM_CD, "1f501ce4b72392bdd1d9ec38f6eec6da", "voices5.res" },
	{ GID_IHNM_CD, "f580ed7568c7d6ef34e934ba20adf834", "voices6.res" },
	{ GID_IHNM_CD, "54b1f2013a075338ceb0e258d97808bd", "voicess.res" },

	{ GID_IHNM_DEMO, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res" },
	{ GID_IHNM_DEMO, "9626bda8978094ff9b29198bc1ed5f9a", "scripts.res" },
	{ GID_IHNM_DEMO, "1c610d543f32ec8b525e3f652536f269", "sfx.res" },
	{ GID_IHNM_DEMO, "3bbc16a8f741dbb511da506c660a0b54", "voicesd.res" },
};

static GAMEDESC GameDescs[] = {
	// Inherit the earth - DOS Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		"Inherit the Earth (DOS Demo)", // Game title
		320, 200, // Logical resolution
		137, // Scene viewport height
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITE_Resources,
		ARRAYSIZE(ITEDEMO_GameFiles), // Game datafiles
		ITEDEMO_GameFiles,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		0, // features
	},

	// Inherit the earth - MAC Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_MACDEMO2,
		"Inherit the Earth (MAC Demo)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES
	},

	// Inherit the earth - early MAC Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_MACDEMO1,
		"Inherit the Earth (early MAC Demo)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD,
		"Inherit the Earth (MAC CD)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACCD_GameFiles),
		ITEMACCD_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_LINDEMO,
		"Inherit the Earth (Linux Demo)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITELINDEMO_GameFiles),
		ITELINDEMO_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES
	},
	
	// Inherit the earth - Linux Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_WINDEMO2,
		"Inherit the Earth (Win32 Demo)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES
	},
	
	// Inherit the earth - Win32 Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_WINDEMO1,
		"Inherit the Earth (early Win32 Demo)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES
	},
	
	// Inherit the earth - Wyrmkeep Linux CD version
	// should be before GID_ITE_CD_G
	{
		"ite",
		GType_ITE,
		GID_ITE_LINCD,
		"Inherit the Earth (Linux CD Version)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITELINCD_GameFiles),
		ITELINCD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		0
	},

	// Inherit the earth - DOS CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_G,
		"Inherit the Earth (DOS CD Version)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		0
	},

	// Inherit the earth - CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD,
		"Inherit the Earth (DOS/Win32 CD Version)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		0
	},

	// Inherit the earth - German Floppy version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_DE,
		"Inherit the Earth (De DOS Floppy)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFiles),
		ITEDISK_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITE_GameSound,
		GF_LANG_DE
	},

	// Inherit the earth - Disk version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_G,
		"Inherit the Earth (DOS Floppy)",
		320, 200,
		137,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFiles),
		ITEDISK_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITE_GameSound,
		0
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		"ihnm-demo",
		GType_IHNM,
		GID_IHNM_DEMO,
		"I Have No Mouth and I Must Scream (DOS Demo)",
		640, 480,
		7,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNMDEMO_GameFiles),
		IHNMDEMO_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		GF_DEFAULT_TO_1X_SCALER
	},

	// I Have No Mouth And I Must Scream - CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD,
		"I Have No Mouth and I Must Scream (DOS)",
		640, 480,
		7,
		152,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFiles),
		IHNMCD_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		GF_DEFAULT_TO_1X_SCALER
	}
};

static GAMEMODULE GameModule;

int SagaEngine::initGame(void) {
	int game_n;
	FSList dummy;

	if ((game_n = detectGame(dummy)) == -1) {
		warning("No valid games were found in the specified directory.");
		return FAILURE;
	}

	if (loadGame(game_n) != SUCCESS) {
		warning("Error loading game resource files.");
		return FAILURE;
	}

	// Load dialogue file 
	loadLanguage();

	return SUCCESS;
}

int SagaEngine::loadLanguage(void) {
	char lang_file[MAXPATH];
	uint16 game_n;

	File test_file;

	game_n = GameModule.game_number;

	if (GameDescs[game_n].gd_game_type == GType_ITE) {
		snprintf(lang_file, MAXPATH, "%s%s.%s", GAME_ITE_LANG_PREFIX, GameModule.game_language, GAME_LANG_EXT);
		if (!test_file.open(lang_file)) {
			debug(0, "Couldn't open language file %s. Using default (US English)", lang_file);
			return SUCCESS;
		}

		test_file.close();

		if (_vm->_interface->registerLang() != SUCCESS) {
			warning("Error registering interface language cvars");
			return FAILURE;
		}

		if (_vm->_scene->ITEIntroRegisterLang() != SUCCESS) {
			warning("Error registering intro sequence language cvars");
			return FAILURE;
		}

		debug(0, "Using language file %s.", lang_file);
		// FIXME
		//CFG_Read(lang_path);
	} else {
		debug(0, "Language support for this game not implemented.");
	}

	return SUCCESS;
}

RSCFILE_CONTEXT *SagaEngine::getFileContext(uint16 type, int param) {
	RSCFILE_CONTEXT *found_ctxt = NULL;
	uint16 i;

	for (i = 0; i < GameModule.gfile_n; i++) {
		if (GameModule.gfile_data[i].file_types & type) {
			found_ctxt = GameModule.gfile_data[i].file_ctxt;
		}
	}

	return found_ctxt;
}

DetectedGameList GAME_ProbeGame(const FSList &fslist) {
	DetectedGameList detectedGames;
	int game_n;

	if ((game_n = detectGame(fslist, true)) != -1)
		detectedGames.push_back(GameDescs[game_n].toGameSettings());

	return detectedGames;
}

int detectGame(const FSList &fslist, bool mode) {
	int game_count = ARRAYSIZE(GameDescs);
	int game_n = -1;
	Common::StringMap filesMD5;

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet filesList;

	uint16 file_count;
	uint16 file_n;
	File test_file;
	bool file_missing;

	Common::String tstr;
	char md5str[32+1];
	uint8 md5sum[16];

	// First we compose list of files which we need MD5s for
	for (int i = 0; i < ARRAYSIZE(game_md5); i++) {
		tstr = Common::String(game_md5[i].filename);
		tstr.toLowercase();
		filesList[tstr] = true;
	}

	if (mode) {
		// Now count MD5s for required files
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				tstr = file->displayName();
				tstr.toLowercase();
				
				if (filesList.contains(tstr)) {
					if (md5_file(file->path().c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
						for (int j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
					}
				}
			}
		}
	} else {
		File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			if (testFile.open(file->_key.c_str())) {
				testFile.close();
				if (md5_file(file->_key.c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
					for (int j = 0; j < 16; j++) {
						sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
					}
					filesMD5[file->_key] = Common::String(md5str);
				}
			}
		}
	}

	for (game_n = 0; game_n < game_count; game_n++) {
		file_count = GameDescs[game_n].gd_filect;
		file_missing = false;

		// Try to open all files for this game
		for (file_n = 0; file_n < file_count; file_n++) {
			tstr = GameDescs[game_n].gd_filedescs[file_n].gf_fname;
			tstr.toLowercase();

			if (!filesMD5.contains(tstr)) {
				file_missing = true;
				break;
			}
		}

		// Try the next game, couldn't find all files for the current 
		// game
		if (file_missing) {
			continue;
		} else {
			bool match = true;

			debug(5, "Probing game: %s", GameDescs[game_n].gd_title);

			for (int i = 0; i < ARRAYSIZE(game_md5); i++) {
				if (game_md5[i].id == GameDescs[game_n].gd_game_id) {
					tstr = game_md5[i].filename;
					tstr.toLowercase();

					if (strcmp(game_md5[i].md5, filesMD5[tstr].c_str())) {
						match = false;
						break;
					}
				}
			}
			if (!match)
				continue;

			debug(5, "Found game: %s", GameDescs[game_n].gd_title);

			return game_n;
		}
	}

	if (!filesMD5.isEmpty()) {
		printf("MD5s of your ITE version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your ITE version:\n");

		for (Common::StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());

	}

	return -1;
}

int SagaEngine::loadGame(int game_n) {
	RSCFILE_CONTEXT *load_ctxt;
	uint16 game_count = ARRAYSIZE(GameDescs);
	const char *game_fname;
	uint16 game_filect;
	uint16 i;

	if (game_n >= game_count) {
		return FAILURE;
	}

	GameModule.game_number = game_n;
	GameModule.gamedesc = &GameDescs[game_n];

	_gameId = GameModule.gamedesc->gd_game_id;
	_gameType = GameModule.gamedesc->gd_game_type;
	_features = GameModule.gamedesc->features;

	game_filect = GameDescs[game_n].gd_filect;
	
	GameModule.gfile_data = (GAME_FILEDATA *)malloc(game_filect * sizeof *GameModule.gfile_data);
	if (GameModule.gfile_data == NULL) {
		return MEM;
	}

	GameModule.gfile_n = game_filect;

	// Load game resource files
	for (i = 0; i < game_filect; i++) {
		load_ctxt = RSC_CreateContext();
		game_fname = GameDescs[game_n].gd_filedescs[i].gf_fname;
		if (RSC_OpenContext(load_ctxt, game_fname) != SUCCESS) {
			return FAILURE;
		}

		debug(0, "Opened resource file: %s", game_fname);
		GameModule.gfile_data[i].file_ctxt = load_ctxt;
		GameModule.gfile_data[i].file_types = GameDescs[game_n].gd_filedescs[i].gf_type;
		GameModule.gfile_data[i].file_flags = 0;
	}

	// Load game font data
	GameModule.gd_fontct = GameDescs[game_n].gd_fontct;
	GameModule.gd_fontdescs = GameDescs[game_n].gd_fontdescs;

	return SUCCESS;
}

const GAME_RESOURCEDESC SagaEngine::getResourceInfo(void) {
	return *GameModule.gamedesc->gd_resource_desc;
}

const GAME_SOUNDINFO SagaEngine::getSoundInfo(void) {
	return *GameModule.gamedesc->gd_soundinfo;
}

int SagaEngine::getDisplayWidth() {
	return GameDescs[GameModule.game_number].gd_logical_w;
}

int SagaEngine::getDisplayHeight() {
	return GameDescs[GameModule.game_number].gd_logical_h;
}

int SagaEngine::getPathYOffset() { //fixme: should be in GameDesc
	if (_gameType == GType_ITE) {
		return 35;
	} else {
		return 35; //fixme i don't know exact value
	}
}

int SagaEngine::getStatusYOffset() { //fixme: should be in GameDesc
	if (_gameType == GType_ITE) {
		return ITE_STATUS_Y;
	} else {
		return IHNM_STATUS_Y;
	}
}

int SagaEngine::getDisplayInfo(GAME_DISPLAYINFO *disp_info) {
	int game_n;

	assert(disp_info != NULL);

	game_n = GameModule.game_number;

	disp_info->logical_w = GameDescs[game_n].gd_logical_w;
	disp_info->logical_h = GameDescs[game_n].gd_logical_h;
	disp_info->scene_h = GameDescs[game_n].gd_scene_h;

	return SUCCESS;
}

int SagaEngine::getFontInfo(GAME_FONTDESC **gf_desc, int *font_n) {
	assert((gf_desc != NULL) && (font_n != NULL));

	*gf_desc = GameModule.gd_fontdescs;
	*font_n = GameModule.gd_fontct;

	return SUCCESS;
}

int SagaEngine::getSceneInfo(GAME_SCENEDESC *gs_desc) {
	assert(gs_desc != NULL);

	gs_desc->first_scene = GameModule.gamedesc->gd_startscene;
	gs_desc->scene_lut_rn = RSC_ConvertID(GameModule.gamedesc->gd_resource_desc->scene_lut_rn);

	return SUCCESS;
}

} // End of namespace Saga
