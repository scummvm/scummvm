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

#ifndef SAGA_GAMEDESC_H
#define SAGA_GAMEDESC_H

#include "resfile.h"

/* Public stuff */

namespace SagaGameDesc {

enum R_GAME_BASETYPES {
	
	R_GAMETYPE_ITE,
	R_GAMETYPE_IHNM
};

enum R_GAME_IDS {
	
	R_GAME_ITE_DEMO  = 0,
	R_GAME_ITE_DISK  = 1,
	R_GAME_ITE_CD    = 2,
	R_GAME_IHNM_DEMO = 3,
	R_GAME_IHNM_CD   = 4
};

enum R_GAME_FILETYPES {

	R_GAME_RESOURCEFILE = 0x01,
	R_GAME_SCRIPTFILE   = 0x02,
	R_GAME_SOUNDFILE    = 0x04,
	R_GAME_VOICEFILE    = 0x08,
	R_GAME_DEMOFILE     = 0x10,
	R_GAME_MUSICFILE    = 0x20,
	
	R_GAME_EXCLUDE = 0x8000
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

typedef struct R_GAME_DISPLAYINFO_tag {

	int logical_w;
	int logical_h;
	int scene_h;

	int ovl_pal_start;
	int ovl_pal_end;

} R_GAME_DISPLAYINFO;

typedef struct R_GAMESOUND_INFO_tag {

	int  res_type;
	long freq;
	int  sample_size;
	int  stereo;
	
} R_GAME_SOUNDINFO;

typedef struct R_GAMEFONT_DESC_tag {
	
	unsigned int  font_id;
	unsigned long font_rn;
	
} R_GAME_FONTDESC;

typedef struct R_GAMESCENE_DESC_tag {

	unsigned long scene_lut_rn;
	unsigned long first_scene;

} R_GAME_SCENEDESC;

typedef struct R_GAME_RESOURCEINFO_tag {
	
	unsigned long scene_lut_rn;
	unsigned long script_lut_rn;

	unsigned long overlay_pal_rn;

	unsigned long command_panel_rn;
	unsigned long command_buttons_rn;

	unsigned long dialogue_panel_rn;
	unsigned long lportraits_rn;

	unsigned long actor_tbl_rn;

} R_GAME_RESOURCEINFO;

	void setGameDirectory( const char *gamedir );
	int detectGame();
	bool openGame();

} // end namespace 




#endif






