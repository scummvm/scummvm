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

#ifndef SAGA_GAME_H_
#define SAGA_GAME_H_

#include "saga/saga.h"

namespace Saga {

#define GAME_LANGSTR_LIMIT 3
#define GAME_PATH_LIMIT 512

#define GAME_ITE_LANG_PREFIX "ite_"
#define GAME_LANG_EXT "lng"

// Script lookup table entry sizes for game verification
#define SCR_LUT_ENTRYLEN_ITECD 22
#define SCR_LUT_ENTRYLEN_ITEDISK 16

struct GAME_FILEDESC {
	const char *gf_fname;
	uint16 gf_type;
};

struct GAMEDESC {
	const char *name;
	SAGAGameType gd_game_type;
	GAME_IDS gd_game_id;
	const char *gd_title;
	int gd_logical_w;
	int gd_logical_h;
	int gd_scene_h;
	int gd_startscene;
	GAME_RESOURCEDESC *gd_resource_desc;
	int gd_filect;
	GAME_FILEDESC *gd_filedescs;
	int gd_fontct;
	GAME_FONTDESC *gd_fontdescs;
	GAME_SOUNDINFO *gd_soundinfo;
	uint32 features;

	GameSettings toGameSettings() const {
		GameSettings dummy = { name, gd_title, features };
		return dummy;
	}
};

struct GAME_FILEDATA {
	RSCFILE_CONTEXT *file_ctxt;
	uint16 file_types;
	uint16 file_flags;
};

struct GAMEMODULE {
	int game_number;
	GAMEDESC *gamedesc;
	int g_skipintro;
	char game_language[GAME_LANGSTR_LIMIT];
	uint16 gfile_n;
	GAME_FILEDATA *gfile_data;
	uint16 gd_fontct;
	GAME_FONTDESC *gd_fontdescs;
	int err_n;
	const char *err_str;
};

DetectedGameList GAME_ProbeGame(const FSList &fslist);

} // End of namespace Saga

#endif
