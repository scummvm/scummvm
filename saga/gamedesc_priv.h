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

#ifndef SAGA_GAMEDESC_PRIV_H
#define SAGA_GAMEDESC_PRIV_H

#include "gamedesc.h"

using namespace SagaGameDesc;

//-------------------------------------------------------------------
// Private stuff 
//-------------------------------------------------------------------

#define R_GAME_LANGSTR_LIMIT 3
#define R_GAME_PATH_LIMIT 512

#define R_GAME_ITE_LANG_PREFIX "ite_"
#define R_GAME_LANG_EXT "lng"

/* Script lookup table entry sizes for game verification */
#define R_SCR_LUT_ENTRYLEN_ITECD 22
#define R_SCR_LUT_ENTRYLEN_ITEDISK 16

typedef bool (ResourceFile::*R_GAME_VERIFYFUNC)();

typedef struct R_GAME_FILEDESC_tag {

    const char * gf_fname;
    unsigned int         gf_type;

} R_GAME_FILEDESC;

typedef struct R_GAMEDESC_tag {
    
    int gd_game_type;
    int gd_game_id;

    const char * gd_title;

    R_GAME_DISPLAYINFO * gd_display_info;

    int   gd_startscene;

    R_GAME_RESOURCEINFO * gd_resource_info;

    int               gd_filect;
    R_GAME_FILEDESC * gd_filedescs;
    
    int               gd_fontct;
    R_GAME_FONTDESC * gd_fontdescs;
    
    R_GAME_SOUNDINFO * gd_soundinfo;

    int gd_supported;

} R_GAMEDESC;


typedef struct R_GAME_FILEDATA_tag {

    ResourceFile *file_ctxt;

    unsigned int  file_types;
    unsigned int  file_flags;

} R_GAME_FILEDATA;


typedef struct R_GAMEMODULE_tag {

    int game_init;
    int game_index;
    
    R_GAMEDESC * gamedesc;

    int g_skipintro;

    const char *game_dir;
    
    char game_language[ R_GAME_LANGSTR_LIMIT ];

    unsigned int gfile_n;
    R_GAME_FILEDATA * gfile_data;

    unsigned int gd_fontct;
    R_GAME_FONTDESC * gd_fontdescs;

    int          err_n;
    const char * err_str;

} R_GAMEMODULE;

bool verifyITEDEMO();
bool verifyITEDISK();
bool verifyITECD();
bool verifyIHNMDEMO();
bool verifyIHNMCD();

#endif // SAGA_GAMEDESC_PRIV_H



