/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "common/scummsys.h"
#include "base/engine.h"
#include "common/util.h"
#include "common/file.h"

#include "resfile.h"
#include "resnames.h"
#include "gamedesc.h"
#include "gamedesc_priv.h"

using namespace SagaGameDesc;

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - Demo version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC ITEDEMO_GameFiles[] = {
	{ "ITE.RSC",	 R_GAME_RESOURCEFILE },
	{ "ITE.DMO",	 R_GAME_DEMOFILE	 },
	{ "SCRIPTS.RSC", R_GAME_SCRIPTFILE	 },
	{ "VOICES.RSC",  R_GAME_SOUNDFILE | R_GAME_VOICEFILE }
};

R_GAME_FONTDESC ITEDEMO_GameFonts[] = {
	{ R_GAME_FONT_SMALL,  0 },
	{ R_GAME_FONT_MEDIUM, 1 }
};

R_GAME_SOUNDINFO ITEDEMO_GameSound = {
	R_GAME_SOUND_VOC, 0, 0, 0
};

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - Diskette version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC ITEDISK_GameFiles[] = {
	{ "ITE.RSC",	 R_GAME_RESOURCEFILE },
	{ "SCRIPTS.RSC", R_GAME_SCRIPTFILE	 },
	{ "VOICES.RSC",  R_GAME_SOUNDFILE | R_GAME_VOICEFILE }
};

R_GAME_FONTDESC ITEDISK_GameFonts[] = {
	{ R_GAME_FONT_MEDIUM, 0 },
	{ R_GAME_FONT_LARGE,  1 },
	{ R_GAME_FONT_SMALL,  2 }
};

R_GAME_DISPLAYINFO ITE_DisplayInfo = {
	320, 200,  /* Logical width and height */
	137,       /* Scene playfield height */
	0, 0       /* Overlay palette start index and length */
};

R_GAME_RESOURCEINFO ITE_Resources = {
	ITE_SCENE_LUT,  /* Scene lookup table RN */
	ITE_SCRIPT_LUT, /* Script lookup table RN */
	
	ITE_OVERLAY_PAL, /* Overlay palette RN */

	ITE_COMMAND_PANEL,
	ITE_COMMAND_BUTTONSPRITES,

	ITE_DIALOGUE_PANEL,
	ITE_DEFAULT_PORTRAITS,

	ITE_ACTOR_PERSONA_TBL
};

R_GAME_SOUNDINFO ITE_GameSound = {
	R_GAME_SOUND_VOC, 0, 0, 0
};

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - CD Enhanced version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC  ITECD_GameFiles[] = {
	{ "ITE.RSC",	 R_GAME_RESOURCEFILE },
	{ "SCRIPTS.RSC", R_GAME_SCRIPTFILE	 },
	{ "SOUNDS.RSC",  R_GAME_SOUNDFILE	 },
	{ "VOICES.RSC",  R_GAME_VOICEFILE	 }
};

R_GAME_FONTDESC ITECD_GameFonts[] = {
	{ R_GAME_FONT_MEDIUM, 0 },
	{ R_GAME_FONT_LARGE,  1 },
	{ R_GAME_FONT_SMALL,  2 }
};

R_GAME_SOUNDINFO ITECD_GameSound = {
	R_GAME_SOUND_PCM,
	22050,
	16,
	0
};

/*--------------------------------------------------------------------------*\
 * I Have No Mouth and I Must Scream - Demo version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC IHNMDEMO_GameFiles[] = {
	{ "SCREAM.RES",  R_GAME_RESOURCEFILE },
	{ "SCRIPTS.RES", R_GAME_SCRIPTFILE	 },
	{ "SFX.RES",	 R_GAME_SOUNDFILE	 },
	{ "VOICESD.RES", R_GAME_VOICEFILE	 }
};

/*--------------------------------------------------------------------------*\
 * I Have No Mouth and I Must Scream - Retail CD version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC IHNMCD_GameFiles[] = {
	{ "MUSICFM.RES", R_GAME_MUSICFILE	 },
	{ "MUSICGM.RES", R_GAME_MUSICFILE	 },
	{ "SCREAM.RES",  R_GAME_RESOURCEFILE },
	{ "SCRIPTS.RES", R_GAME_SCRIPTFILE	 },
	{ "SFX.RES",	 R_GAME_SOUNDFILE	 },
	{ "VOICES1.RES", R_GAME_VOICEFILE	 },
	{ "VOICES2.RES", R_GAME_VOICEFILE	 },
	{ "VOICES3.RES", R_GAME_VOICEFILE	 },
	{ "VOICES4.RES", R_GAME_VOICEFILE	 },
	{ "VOICES5.RES", R_GAME_VOICEFILE	 },
	{ "VOICES6.RES", R_GAME_VOICEFILE	 },
	{ "VOICESS.RES", R_GAME_VOICEFILE	 }
};

R_GAME_FONTDESC IHNMCD_GameFonts[] = {
	{ R_GAME_FONT_MEDIUM,	2 },
	{ R_GAME_FONT_LARGE,	3 },
	{ R_GAME_FONT_SMALL,	4 },
	{ R_GAME_FONT_SMALL2,	5 },
	{ R_GAME_FONT_MEDIUM2,	6 },
	{ R_GAME_FONT_LARGE2,	7 },
	{ R_GAME_FONT_LARGE3,	8 }
};

R_GAME_DISPLAYINFO IHNM_DisplayInfo = {
	640, 480, /* Logical width and height */
	304,      /* Scene playfield height */
	248, 255  /* Overlay palette start index and length */
};

R_GAME_RESOURCEINFO IHNM_Resources[] = {
	IHNM_SCENE_LUT,  /* Scene lookup table RN */
	IHNM_SCRIPT_LUT, /* Script lookup table RN */
	
	IHNM_OVERLAY_PAL, /* Overlay palette RN */

	IHNM_COMMAND_PANEL,
	IHNM_COMMAND_BUTTONSPRITES,

	IHNM_DIALOGUE_PANEL,
	IHNM_DEFAULT_PORTRAITS,

	IHNM_ACTOR_PERSONA_TBL
};


R_GAME_SOUNDINFO IHNM_GameSound = {
	R_GAME_SOUND_WAV, 0, 0, 0
};

R_GAMEDESC GameDescs[] = {
	/* Inherit the earth - Demo version
	\*-------------------------------------------------------------*/
	{ 
		R_GAMETYPE_ITE,
		R_GAME_ITE_DEMO, /* Game id */
		"Inherit the Earth - Demo version", /* Game title */

		&ITE_DisplayInfo,

		ITE_DEFAULT_SCENE, /* Starting scene number */
 
		&ITE_Resources,

		ARRAYSIZE(ITEDEMO_GameFiles), /* Game datafiles */
		ITEDEMO_GameFiles,

		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
	 
		&ITEDEMO_GameSound,

		0 /* Game supported flag */
	},		 

	/* Inherit the earth - Disk version
	\*-------------------------------------------------------------*/
	{ 
		R_GAMETYPE_ITE,
		R_GAME_ITE_DISK,  
		"Inherit the Earth - Disk version", 

		&ITE_DisplayInfo,

		ITE_DEFAULT_SCENE,

		&ITE_Resources,

		ARRAYSIZE(ITEDISK_GameFiles), 	
		ITEDISK_GameFiles, 
	  
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
	  
		&ITE_GameSound,

		1			
	},

	/* Inherit the earth - CD version
	\*-------------------------------------------------------------*/
	{ 
		R_GAMETYPE_ITE,
		R_GAME_ITE_CD,	  
		"Inherit the Earth - CD version",	

		&ITE_DisplayInfo,
	  
		ITE_DEFAULT_SCENE,

		&ITE_Resources,

		ARRAYSIZE( ITECD_GameFiles ), 
		ITECD_GameFiles,
		
		ARRAYSIZE( ITECD_GameFonts ),
		ITECD_GameFonts,

		&ITECD_GameSound,

		1			 
	},

	/* I Have No Mouth And I Must Scream - Demo version
	\*-------------------------------------------------------------*/
	{ 
		R_GAMETYPE_IHNM,
		R_GAME_IHNM_DEMO, 
		"I Have No Mouth - Demo version",

		&IHNM_DisplayInfo,
		  
		0,

		IHNM_Resources,
	  
		ARRAYSIZE(IHNMDEMO_GameFiles),
		IHNMDEMO_GameFiles,

		0,
		NULL,

		&IHNM_GameSound,

		0				 
	},

	/* I Have No Mouth And I Must Scream - CD version
	\*-------------------------------------------------------------*/
	{ 
		R_GAMETYPE_IHNM,
		R_GAME_IHNM_CD,   
		"I Have No Mouth - CD version", 

		&IHNM_DisplayInfo,
			  
		1,

		IHNM_Resources,

		ARRAYSIZE(IHNMCD_GameFiles),
		IHNMCD_GameFiles,

		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,

		&IHNM_GameSound,

		1	   
	}
};

static R_GAMEMODULE GameModule;

void SagaGameDesc::setGameDirectory(const char *gamedir) {
	assert(gamedir != NULL);

	debug(1, "Using game data path: %s", gamedir);

	GameModule.game_dir = gamedir;
}

int SagaGameDesc::detectGame() {
	File test_file;

	bool disqualified = false;
	bool found_game = false;

	int file_n;
	int file_ct;
	const char *file_name;

	int game_n;
	int game_ct = ARRAYSIZE(GameDescs);

	assert(GameModule.game_dir != NULL);

	for (game_n = 0; game_n < game_ct; game_n++) {
		disqualified = false;
		
		/* Attempt to open all files for this game
		 * If we can open them all, then try to open all files on the 
		 * exclude list
		 */
		file_ct = GameDescs[game_n].gd_filect;
		
		for (file_n = 0; file_n < file_ct; file_n++) {
			file_name = GameDescs[game_n].gd_filedescs[file_n].gf_fname;
			if (!test_file.open(file_name, GameModule.game_dir)) {
				disqualified = true;
				break;
			}

			test_file.close();
		}

		if (disqualified) {
			continue;
		}

		switch (GameDescs[game_n].gd_game_id) {

		case R_GAME_ITE_DEMO:
			disqualified = !verifyITEDEMO();
			break;
		case R_GAME_ITE_DISK:
			disqualified = !verifyITEDISK();
			break;

		default:
			break;
		}

		if (!disqualified) {
			found_game = true;
			break;
		}
	}

	if (found_game) {
		debug(1, "Found SAGA game: %s\n", GameDescs[game_n].gd_title);
		return game_n;
	}
	
	return -1;
}

bool SagaGameDesc::openGame() {
	int game_filect;

	if ((GameModule.game_index = detectGame()) < 0) { 
		error("Couldn't locate any valid SAGA games");	
		return false;
	}

	assert(GameModule.game_index < ARRAYSIZE(GameDescs));

	if (!GameModule.game_dir) {
		return false;
	}

	game_filect = GameDescs[GameModule.game_index].gd_filect;

	GameModule.gfile_data = new R_GAME_FILEDATA[game_filect];

	return true;
}

bool verifyITEDEMO() {
	return true;
}	

bool verifyITEDISK() {
	ResourceFile test_file;
	int32 script_lut_rn;
	int32 script_lut_len;

	/* Attempt to verify the disk version of Inherit the Earth
	 * by examining the length of entries in the script lookup
	 * table, which differs from the disk version and the CD 
	 * version.
	 */

	assert (GameModule.game_dir != NULL);

	if (!test_file.open("ITE.RSC", GameModule.game_dir)) {
		return false;
	}

	script_lut_rn = GameDescs[R_GAME_ITE_DISK].gd_resource_info->script_lut_rn;

	script_lut_len = test_file.getResourceLen(script_lut_rn);

	if (script_lut_len % R_SCR_LUT_ENTRYLEN_ITEDISK == 0) {
		return true;
	}

	return false;
}

bool verifyITECD() {
	return true;
}	

bool verifyIHNMDEMO() {
	return true;
}	

bool verifyIHNMCD() {
	return true;
}
