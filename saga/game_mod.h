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

#include "base/plugins.h"

namespace Saga {

enum GAME_IDS {
	GAME_ITE_DEMO = 0,
	GAME_ITE_DISK = 1,
	GAME_ITE_CD = 2,
	GAME_IHNM_DEMO = 3,
	GAME_IHNM_CD = 4,
	GAME_ITE_WINDEMO = 5
};

enum GAME_FILETYPES {
	GAME_RESOURCEFILE = 0x01,
	GAME_SCRIPTFILE = 0x02,
	GAME_SOUNDFILE = 0x04,
	GAME_VOICEFILE = 0x08,
	GAME_DEMOFILE = 0x10,
	GAME_MUSICFILE = 0x20
};

enum GAME_SOUNDINFO_TYPES {
	GAME_SOUND_PCM = 0,
	GAME_SOUND_VOC,
	GAME_SOUND_WAV,
	GAME_SOUND_VOX
};

enum GAME_FONT_IDS {
	GAME_FONT_SMALL = 0,
	GAME_FONT_MEDIUM,
	GAME_FONT_LARGE,
	GAME_FONT_SMALL2,
	GAME_FONT_MEDIUM2,
	GAME_FONT_LARGE2,
	GAME_FONT_LARGE3
};

enum GAME_FEATURES {
	GF_VOX_VOICES = 1
};

struct GAME_DISPLAYINFO {
	int logical_w;
	int logical_h;
	int scene_h;
};

struct GAME_SOUNDINFO {
	int res_type;
	long freq;
	int sample_size;
	int stereo;
};

struct GAME_FONTDESC {
	uint16 font_id;
	uint32 font_rn;
};

struct GAME_SCENEDESC {
	uint32 scene_lut_rn;
	uint32 first_scene;
};

struct GAME_RESOURCEDESC {
	uint32 scene_lut_rn;
	uint32 script_lut_rn;
	uint32 command_panel_rn;
	uint32 dialogue_panel_rn;
};

int GAME_Register();
int GAME_Init();
RSCFILE_CONTEXT *GAME_GetFileContext(uint16 type, int param);
int GAME_GetFontInfo(GAME_FONTDESC **, int *);
int GAME_GetResourceInfo(GAME_RESOURCEDESC *);
int GAME_GetSoundInfo(GAME_SOUNDINFO *);
int GAME_GetDisplayInfo(GAME_DISPLAYINFO *);
int GAME_GetSceneInfo(GAME_SCENEDESC *);
int GAME_GetGame();
int GAME_GetGameType();
DetectedGameList GAME_ProbeGame(const FSList &fslist);
uint32 GAME_GetFeatures();

} // End of namespace Saga

#endif
