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

// Game detection, general game parameters - module header

#ifndef SAGA_GAME_MOD_H__
#define SAGA_GAME_MOD_H__

namespace Saga {

enum R_GAME_BASETYPES {
	R_GAMETYPE_ITE,
	R_GAMETYPE_IHNM
};

enum R_GAME_IDS {
	R_GAME_ITE_DEMO = 0,
	R_GAME_ITE_DISK = 1,
	R_GAME_ITE_CD = 2,
	R_GAME_IHNM_DEMO = 3,
	R_GAME_IHNM_CD = 4,
	R_GAME_ITE_WINDEMO = 5
};

enum R_GAME_FILETYPES {
	R_GAME_RESOURCEFILE = 0x01,
	R_GAME_SCRIPTFILE = 0x02,
	R_GAME_SOUNDFILE = 0x04,
	R_GAME_VOICEFILE = 0x08,
	R_GAME_DEMOFILE = 0x10,
	R_GAME_MUSICFILE = 0x20
};

enum R_GAME_SOUNDINFO_TYPES {
	R_GAME_SOUND_PCM = 0,
	R_GAME_SOUND_VOC,
	R_GAME_SOUND_WAV
};

enum R_GAME_FONT_IDS {
	R_GAME_FONT_SMALL = 0,
	R_GAME_FONT_MEDIUM,
	R_GAME_FONT_LARGE,
	R_GAME_FONT_SMALL2,
	R_GAME_FONT_MEDIUM2,
	R_GAME_FONT_LARGE2,
	R_GAME_FONT_LARGE3
};

struct R_GAME_DISPLAYINFO {
	int logical_w;
	int logical_h;
	int scene_h;
};

struct R_GAME_SOUNDINFO {
	int res_type;
	long freq;
	int sample_size;
	int stereo;
};

struct R_GAME_FONTDESC {
	uint16 font_id;
	uint32 font_rn;
};

struct R_GAME_SCENEDESC {
	uint32 scene_lut_rn;
	uint32 first_scene;
};

struct R_GAME_RESOURCEDESC {
	uint32 scene_lut_rn;
	uint32 script_lut_rn;
	uint32 command_panel_rn;
	uint32 dialogue_panel_rn;
};

int GAME_Register();
int GAME_Init();
int GAME_GetFileContext(R_RSCFILE_CONTEXT **ctxt_p, uint16 r_type, int param);
int GAME_GetFontInfo(R_GAME_FONTDESC **, int *);
int GAME_GetResourceInfo(R_GAME_RESOURCEDESC *);
int GAME_GetSoundInfo(R_GAME_SOUNDINFO *);
int GAME_GetDisplayInfo(R_GAME_DISPLAYINFO *);
int GAME_GetSceneInfo(R_GAME_SCENEDESC *);
int GAME_GetGame();
int GAME_GetGameType();
int GAME_GetErrN();
void GAME_setGameDirectory(const char *gamedir);
const char *GAME_GetErrS();

} // End of namespace Saga

#endif
