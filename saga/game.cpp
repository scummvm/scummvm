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
/*
 Description:   
 
    Game detection, general game parameters

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"
#include "common/file.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "rscfile_mod.h"
#include "cvar_mod.h"
#include "ite_introproc_mod.h"
#include "interface_mod.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "game_mod.h"
#include "game.h"

namespace Saga {

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - Demo version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC ITEDEMO_GameFiles[] = {

	{"ITE.RSC", R_GAME_RESOURCEFILE}
	,
	{"ITE.DMO", R_GAME_DEMOFILE}
	,
	{"SCRIPTS.RSC", R_GAME_SCRIPTFILE}
	,
	{"VOICES.RSC", R_GAME_SOUNDFILE | R_GAME_VOICEFILE}
};

R_GAME_FONTDESC ITEDEMO_GameFonts[] = {

	{R_GAME_FONT_SMALL, 0}
	,
	{R_GAME_FONT_MEDIUM, 1}
};

R_GAME_SOUNDINFO ITEDEMO_GameSound = {

	R_GAME_SOUND_VOC, 0, 0, 0
};

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - Diskette version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC ITEDISK_GameFiles[] = {

	{"ITE.RSC", R_GAME_RESOURCEFILE}
	,
	{"SCRIPTS.RSC", R_GAME_SCRIPTFILE}
	,
	{"VOICES.RSC", R_GAME_SOUNDFILE | R_GAME_VOICEFILE}
};

R_GAME_FONTDESC ITEDISK_GameFonts[] = {

	{R_GAME_FONT_MEDIUM, 0}
	,
	{R_GAME_FONT_LARGE, 1}
	,
	{R_GAME_FONT_SMALL, 2}
};

R_GAME_RESOURCEDESC ITE_Resources = {
	ITE_SCENE_LUT,		/* Scene lookup table RN */
	ITE_SCRIPT_LUT,		/* Script lookup table RN */

	ITE_COMMAND_PANEL,
	ITE_DIALOGUE_PANEL
};

R_GAME_SOUNDINFO ITE_GameSound = {

	R_GAME_SOUND_VOC, 0, 0, 0
};

/*--------------------------------------------------------------------------*\
 * Inherit the Earth - CD Enhanced version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC ITECD_GameFiles[] = {

	{"ITE.RSC", R_GAME_RESOURCEFILE}
	,
	{"SCRIPTS.RSC", R_GAME_SCRIPTFILE}
	,
	{"SOUNDS.RSC", R_GAME_SOUNDFILE}
	,
	{"VOICES.RSC", R_GAME_VOICEFILE}
};

R_GAME_FONTDESC ITECD_GameFonts[] = {

	{R_GAME_FONT_MEDIUM, 0}
	,
	{R_GAME_FONT_LARGE, 1}
	,
	{R_GAME_FONT_SMALL, 2}
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

	{"SCREAM.RES", R_GAME_RESOURCEFILE}
	,
	{"SCRIPTS.RES", R_GAME_SCRIPTFILE}
	,
	{"SFX.RES", R_GAME_SOUNDFILE}
	,
	{"VOICESD.RES", R_GAME_VOICEFILE}
};

/*--------------------------------------------------------------------------*\
 * I Have No Mouth and I Must Scream - Retail CD version
\*--------------------------------------------------------------------------*/

R_GAME_FILEDESC IHNMCD_GameFiles[] = {

	{"MUSICFM.RES", R_GAME_MUSICFILE}
	,
	{"MUSICGM.RES", R_GAME_MUSICFILE}
	,
	{"SCREAM.RES", R_GAME_RESOURCEFILE}
	,
	{"SCRIPTS.RES", R_GAME_SCRIPTFILE}
	,
	{"SFX.RES", R_GAME_SOUNDFILE}
	,
	{"VOICES1.RES", R_GAME_VOICEFILE}
	,
	{"VOICES2.RES", R_GAME_VOICEFILE}
	,
	{"VOICES3.RES", R_GAME_VOICEFILE}
	,
	{"VOICES4.RES", R_GAME_VOICEFILE}
	,
	{"VOICES5.RES", R_GAME_VOICEFILE}
	,
	{"VOICES6.RES", R_GAME_VOICEFILE}
	,
	{"VOICESS.RES", R_GAME_VOICEFILE}
};

R_GAME_FONTDESC IHNMCD_GameFonts[] = {

	{R_GAME_FONT_MEDIUM, 2}
	,
	{R_GAME_FONT_LARGE, 3}
	,
	{R_GAME_FONT_SMALL, 4}
	,
	{R_GAME_FONT_SMALL2, 5}
	,
	{R_GAME_FONT_MEDIUM2, 6}
	,
	{R_GAME_FONT_LARGE2, 7}
	,
	{R_GAME_FONT_LARGE3, 8}
};

R_GAME_RESOURCEDESC IHNM_Resources = {
	IHNM_SCENE_LUT,		/* Scene lookup table RN */
	IHNM_SCRIPT_LUT,	/* Script lookup table RN */

	IHNM_COMMAND_PANEL,
	IHNM_DIALOGUE_PANEL
};

R_GAME_SOUNDINFO IHNM_GameSound = {

	R_GAME_SOUND_WAV, 0, 0, 0
};

R_GAMEDESC GameDescs[] = {

	/* Inherit the earth - Demo version
	 * \*------------------------------------------------------------- */
	{
		    R_GAMETYPE_ITE,
		    R_GAME_ITE_DEMO,	/* Game id */
		    "Inherit the Earth - Demo version",	/* Game title */
		    320, 200,	/* Logical resolution */
		    137,	/* Scene viewport height */

		    ITE_DEFAULT_SCENE,	/* Starting scene number */

		    &ITE_Resources,

		    ARRAYSIZE(ITEDEMO_GameFiles),	/* Game datafiles */
		    ITEDEMO_GameFiles,

		    ARRAYSIZE(ITEDEMO_GameFonts),
		    ITEDEMO_GameFonts,

		    &ITEDEMO_GameSound,

		    Verify_ITEDEMO,	/* Game verification func */
		    0		/* Game supported flag */
	    }
	,

	/* Inherit the earth - Disk version
	 * \*------------------------------------------------------------- */
	{
		    R_GAMETYPE_ITE,
		    R_GAME_ITE_DISK,
		    "Inherit the Earth - Disk version",
		    320, 200,
		    137,

		    ITE_DEFAULT_SCENE,

		    &ITE_Resources,

		    ARRAYSIZE(ITEDISK_GameFiles),
		    ITEDISK_GameFiles,

		    ARRAYSIZE(ITEDISK_GameFonts),
		    ITEDISK_GameFonts,

		    &ITE_GameSound,

		    Verify_ITEDISK,
	    1}
	,

	/* Inherit the earth - CD version
	 * \*------------------------------------------------------------- */
	{
		    R_GAMETYPE_ITE,
		    R_GAME_ITE_CD,
		    "Inherit the Earth - CD version",
		    320, 200,
		    137,

		    ITE_DEFAULT_SCENE,

		    &ITE_Resources,

		    ARRAYSIZE(ITECD_GameFiles),
		    ITECD_GameFiles,

		    ARRAYSIZE(ITECD_GameFonts),
		    ITECD_GameFonts,

		    &ITECD_GameSound,

		    NULL,
	    1}
	,

	/* I Have No Mouth And I Must Scream - Demo version
	 * \*------------------------------------------------------------- */
	{
		    R_GAMETYPE_IHNM,
		    R_GAME_IHNM_DEMO,
		    "I Have No Mouth - Demo version",
		    640, 480,
		    304,

		    0,

		    &IHNM_Resources,

		    ARRAYSIZE(IHNMDEMO_GameFiles),
		    IHNMDEMO_GameFiles,

		    0,
		    NULL,

		    &IHNM_GameSound,

		    NULL,
	    0}
	,

	/* I Have No Mouth And I Must Scream - CD version
	 * \*------------------------------------------------------------- */
	{
		    R_GAMETYPE_IHNM,
		    R_GAME_IHNM_CD,
		    "I Have No Mouth - CD version",
		    640, 480,
		    304,

		    1,

		    &IHNM_Resources,

		    ARRAYSIZE(IHNMCD_GameFiles),
		    IHNMCD_GameFiles,

		    ARRAYSIZE(IHNMCD_GameFonts),
		    IHNMCD_GameFonts,

		    &IHNM_GameSound,

		    NULL,
	    1}
};

static R_GAMEMODULE GameModule;

void GAME_setGameDirectory(const char *gamedir) {
	assert(gamedir != NULL);

	debug(0, "Using game data path: %s", gamedir);

	strcpy(GameModule.game_dir, gamedir);
	strcpy(GameModule.data_dir, ".");
}

int GAME_Register(void)
{

	return R_SUCCESS;

	/* Register "gamedir" cfg cvar
	 * \*----------------------------------------- */
	strncpy(GameModule.game_dir, "./", R_MAXPATH);

	if (CVAR_Register_S(GameModule.game_dir,
		"gamedir", NULL, R_CVAR_CFG, R_MAXPATH) != R_SUCCESS) {

		return R_FAILURE;
	}

	/* Register "datadir" cfg cvar
	 * \*----------------------------------------- */
	strncpy(GameModule.data_dir, "./", R_MAXPATH);

	if (CVAR_Register_S(GameModule.data_dir,
		"datadir", NULL, R_CVAR_CFG, R_MAXPATH) != R_SUCCESS) {

		return R_FAILURE;
	}

	/* Register "g_language" cfg cvar
	 * \*----------------------------------------- */
	strncpy(GameModule.game_dir, "us", R_MAXPATH);

	if (CVAR_Register_S(GameModule.game_language,
		"g_language",
		NULL, R_CVAR_CFG, R_GAME_LANGSTR_LIMIT) != R_SUCCESS) {

		return R_FAILURE;
	}

	/* Register "g_skipintro" cfg cvar
	 * \*----------------------------------------- */
	if (CVAR_Register_I(&GameModule.g_skipintro,
		"g_skipintro", NULL, R_CVAR_CFG, 0, 1) != R_SUCCESS) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int GAME_Init(void)
{
	uint game_n;
	char *game_dir;

	game_dir = GameModule.game_dir;

	if (DetectGame(game_dir, &game_n) != R_SUCCESS) {

		GameModule.err_str = "No valid games were found in the "
		    "specified directory.";

		return R_FAILURE;
	}

	if (!GameDescs[game_n].gd_supported) {

		GameModule.err_str = "This game is not currently supported.";

		return R_FAILURE;
	}

	if (LoadGame(game_dir, game_n) != R_SUCCESS) {

		GameModule.err_str = "Error loading game resource files.";

		return R_FAILURE;
	}

	/* Load dialogue file 
	 * \*------------------------------------------------------------- */
	LoadLanguage();

	return R_SUCCESS;
}

int LoadLanguage(void)
{

	char lang_file[R_MAXPATH];
	// char lang_path[R_MAXPATH];
	uint game_n;

	File test_file;

	game_n = GameModule.game_number;

	if (GameDescs[game_n].gd_game_type == R_GAMETYPE_ITE) {

		snprintf(lang_file, R_MAXPATH,
		    "%s%s.%s",
		    R_GAME_ITE_LANG_PREFIX,
		    GameModule.game_language, R_GAME_LANG_EXT);

		if (!test_file.open(lang_file)) {
			R_printf(R_STDOUT,
			    "Couldn't open language file %s. "
			    "Using default (US English)\n", lang_file);

			return R_SUCCESS;
		}

		test_file.close();

		if (INTERFACE_RegisterLang() != R_SUCCESS) {
			R_printf(R_STDERR,
			    "Error registering interface language cvars.");

			return R_FAILURE;
		}

		if (ITE_IntroRegisterLang() != R_SUCCESS) {

			R_printf(R_STDERR,
			    "Error registering intro sequence language cvars.");

			return R_FAILURE;
		}

		R_printf(R_STDOUT, "Using language file %s.\n", lang_file);

		// FIXME
		//CFG_Read(lang_path);
	} else {

		R_printf(R_STDOUT,
		    "Language support for this game not implemented.\n");
	}

	return R_SUCCESS;
}

int GAME_GetErrN(void)
{
	return 0;
}

const char *GAME_GetErrS(void)
{

	return GameModule.err_str == NULL
	    ? "No error description." : GameModule.err_str;
}

int GAME_GetFileContext(R_RSCFILE_CONTEXT ** ctxt_p, uint r_type, int param)
{

	R_RSCFILE_CONTEXT *found_ctxt = NULL;
	uint i;

	YS_IGNORE_PARAM(param);

	if (ctxt_p == NULL) {

		return R_FAILURE;
	}

	for (i = 0; i < GameModule.gfile_n; i++) {

		if (GameModule.gfile_data[i].file_types & r_type) {

			found_ctxt = GameModule.gfile_data[i].file_ctxt;
		}
	}

	if (found_ctxt != NULL) {

		*ctxt_p = found_ctxt;
	} else {

		*ctxt_p = NULL;
	}

	return R_SUCCESS;
}

int DetectGame(const char *game_dir, uint * game_n_p)
{
	uint game_count = ARRAYSIZE(GameDescs);
	uint game_n;

	uint file_count;
	uint file_n;

	File test_file;

	int file_missing = 0;
	int found_game = 0;

	if ((game_dir == NULL) || (game_n_p == NULL)) {

		return R_FAILURE;
	}

	for (game_n = 0; (game_n < game_count) && !found_game; game_n++) {

		file_count = GameDescs[game_n].gd_filect;

		file_missing = 0;

		/* Try to open all files for this game */
		for (file_n = 0; file_n < file_count; file_n++) {

			if (!test_file.open(GameDescs[game_n].gd_filedescs[file_n].gf_fname)) {
				file_missing = 1;
				break;
			}

			test_file.close();
		}

		/* Try the next game, couldn't find all files for the current 
		 * game */
		if (file_missing) {
			continue;
		}

		/* If there's a verification function for this game, use it, 
		 * otherwise assume we've found the game if all files are found. */
		found_game = 1;

		if (GameDescs[game_n].gd_verifyf != NULL &&
		    GameDescs[game_n].gd_verifyf(game_dir) != R_SUCCESS) {

			found_game = 0;
		}

		if (found_game) {
			R_printf(R_STDOUT,
			    "Found game: %s\n", GameDescs[game_n].gd_title);

			*game_n_p = game_n;

			return R_SUCCESS;
		}
	}

	return R_FAILURE;
}

int LoadGame(const char *game_dir, uint game_n)
{

	R_RSCFILE_CONTEXT *load_ctxt;

	uint game_count = ARRAYSIZE(GameDescs);

	const char *game_fname;
	uint game_filect;

	uint i;

	if ((game_dir == NULL) || (game_n >= game_count)) {

		return R_FAILURE;
	}

	game_filect = GameDescs[game_n].gd_filect;

	GameModule.gfile_data = (R_GAME_FILEDATA *)malloc(game_filect *
	    sizeof *GameModule.gfile_data);
	if (GameModule.gfile_data == NULL) {

		return R_MEM;
	}

	GameModule.gfile_n = game_filect;

	/* Load game resource files
	 * \*------------------------------------------------------------- */
	for (i = 0; i < game_filect; i++) {

		load_ctxt = RSC_CreateContext();

		game_fname = GameDescs[game_n].gd_filedescs[i].gf_fname;

		if (RSC_OpenContext(load_ctxt, game_fname) != R_SUCCESS) {

			return R_FAILURE;
		}

		R_printf(R_STDOUT, "Opened resource file: %s\n", game_fname);

		GameModule.gfile_data[i].file_ctxt = load_ctxt;
		GameModule.gfile_data[i].file_types =
		    GameDescs[game_n].gd_filedescs[i].gf_type;

		GameModule.gfile_data[i].file_flags = 0;
	}

	/* Load game font data
	 * \*------------------------------------------------------------- */
	GameModule.gd_fontct = GameDescs[game_n].gd_fontct;
	GameModule.gd_fontdescs = GameDescs[game_n].gd_fontdescs;

	/* Finish initialization
	 * \*------------------------------------------------------------- */
	GameModule.game_number = game_n;
	GameModule.gamedesc = &GameDescs[game_n];

	GameModule.game_init = 1;

	return R_SUCCESS;
}

int GAME_GetResourceInfo(R_GAME_RESOURCEDESC * rsc_desc)
{
	assert(rsc_desc != NULL);

	*rsc_desc = *GameModule.gamedesc->gd_resource_desc;

	return R_SUCCESS;
}

int GAME_GetSoundInfo(R_GAME_SOUNDINFO * snd_info)
{
	assert(snd_info != NULL);

	*snd_info = *GameModule.gamedesc->gd_soundinfo;

	return R_SUCCESS;
}

int GAME_GetDisplayInfo(R_GAME_DISPLAYINFO * disp_info)
{

	int game_n;

	assert(disp_info != NULL);

	if (!GameModule.game_init) {
		return R_FAILURE;
	}

	game_n = GameModule.game_number;

	disp_info->logical_w = GameDescs[game_n].gd_logical_w;
	disp_info->logical_h = GameDescs[game_n].gd_logical_h;
	disp_info->scene_h = GameDescs[game_n].gd_scene_h;

	return R_SUCCESS;
}

int GAME_GetFontInfo(R_GAME_FONTDESC ** gf_desc, int *font_n)
{

	assert((gf_desc != NULL) && (font_n != NULL));

	*gf_desc = GameModule.gd_fontdescs;
	*font_n = GameModule.gd_fontct;

	return R_SUCCESS;
}

int GAME_GetSceneInfo(R_GAME_SCENEDESC * gs_desc)
{

	assert(gs_desc != NULL);

	gs_desc->first_scene = GameModule.gamedesc->gd_startscene;
	gs_desc->scene_lut_rn =
	    GameModule.gamedesc->gd_resource_desc->scene_lut_rn;

	return R_SUCCESS;
}

int GAME_GetGame(void)
{

	return GameModule.gamedesc->gd_game_id;
}

int GAME_GetGameType(void)
{

	return GameModule.gamedesc->gd_game_type;
}

int Verify_ITEDEMO(const char *game_dir)
{

	YS_IGNORE_PARAM(game_dir);

	return R_SUCCESS;
}

int Verify_ITEDISK(const char *game_dir)
{

	R_RSCFILE_CONTEXT *test_ctx;

	ulong script_lut_len;
	ulong script_lut_rn;

	int verified = 0;

	test_ctx = RSC_CreateContext();

	if (RSC_OpenContext(test_ctx, "ITE.RSC") != R_SUCCESS) {
		return R_FAILURE;
	}

	script_lut_rn =
	    GameDescs[R_GAME_ITE_DISK].gd_resource_desc->script_lut_rn;

	if (RSC_GetResourceSize(test_ctx,
		script_lut_rn, &script_lut_len) != R_SUCCESS) {

		RSC_DestroyContext(test_ctx);

		return R_FAILURE;
	}

	RSC_DestroyContext(test_ctx);

	if (script_lut_len % R_SCR_LUT_ENTRYLEN_ITEDISK == 0) {

		verified = 1;
	}

	if (!verified) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Verify_ITECD(const char *game_dir)
{

	YS_IGNORE_PARAM(game_dir);

	return R_SUCCESS;
}

int Verify_IHNMDEMO(const char *game_dir)
{

	YS_IGNORE_PARAM(game_dir);

	return R_SUCCESS;
}

int Verify_IHNMCD(const char *game_dir)
{

	YS_IGNORE_PARAM(game_dir);

	return R_SUCCESS;
}

} // End of namespace Saga
