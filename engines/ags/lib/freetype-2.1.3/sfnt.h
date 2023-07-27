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

/***************************************************************************/
/*                                                                         */
/*  sfnt.h                                                                 */
/*                                                                         */
/*    High-level `sfnt' driver interface (specification).                  */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_SFNT_H
#define AGS_LIB_FREETYPE_SFNT_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdriver.h"
#include "engines/ags/lib/freetype-2.1.3/tttypes.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef FT_Error (*TT_Init_Face_Func)(FT_Stream stream, TT_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);
typedef FT_Error (*TT_Load_Face_Func)(FT_Stream stream, TT_Face face, FT_Int face_index, FT_Int num_params, FT_Parameter *params);
typedef void 	 (*TT_Done_Face_Func)(TT_Face face);

typedef FT_Module_Interface (*SFNT_Get_Interface_Func)(FT_Module module, const char *func_interface);

typedef FT_Error (*TT_Load_SFNT_HeaderRec_Func)(TT_Face face, FT_Stream stream, FT_Long face_index, SFNT_Header sfnt);
typedef FT_Error (*TT_Load_Directory_Func)(TT_Face face, FT_Stream stream, SFNT_Header sfnt);
typedef FT_Error (*TT_Load_Any_Func)(TT_Face face, FT_ULong tag, FT_Long offset, FT_Byte *buffer, FT_ULong *length);
typedef FT_Error (*TT_Load_SBit_Image_Func)(TT_Face face, FT_ULong strike_index, FT_UInt glyph_index, FT_UInt load_flags, FT_Stream stream, FT_Bitmap *amap, TT_SBit_MetricsRec *ametrics);

typedef FT_Error (*TT_Set_SBit_Strike_Func)(TT_Face face, FT_Int x_ppem, FT_Int y_ppem, FT_ULong *astrike_index);
typedef FT_Error (*TT_Get_PS_Name_Func)(TT_Face face, FT_UInt idx, FT_String **PSname);
typedef FT_Error (*TT_Load_Metrics_Func)(TT_Face face, FT_Stream stream, FT_Bool vertical);

typedef FT_Error (*TT_CharMap_Load_Func)(TT_Face face, TT_CMapTable cmap, FT_Stream input);
typedef FT_Error (*TT_CharMap_Free_Func)(TT_Face face, TT_CMapTable cmap);

typedef FT_Error (*TT_Load_Table_Func)(TT_Face face, FT_Stream stream);
typedef void 	 (*TT_Free_Table_Func)(TT_Face face);


typedef struct  SFNT_Interface_ {
	TT_Loader_GotoTableFunc      goto_table;

	TT_Init_Face_Func            init_face;
	TT_Load_Face_Func            load_face;
	TT_Done_Face_Func            done_face;
	SFNT_Get_Interface_Func      get_interface;

	TT_Load_Any_Func             load_any;
	TT_Load_SFNT_HeaderRec_Func  load_sfnt_header;
	TT_Load_Directory_Func       load_directory;

	/* these functions are called by `load_face' but they can also  */
	/* be called from external modules, if there is a need to do so */
	TT_Load_Table_Func           load_header;
	TT_Load_Metrics_Func         load_metrics;
	TT_Load_Table_Func           load_charmaps;
	TT_Load_Table_Func           load_max_profile;
	TT_Load_Table_Func           load_os2;
	TT_Load_Table_Func           load_psnames;

	TT_Load_Table_Func           load_names;
	TT_Free_Table_Func           free_names;

	/* optional tables */
	TT_Load_Table_Func           load_hdmx;
	TT_Free_Table_Func           free_hdmx;

	TT_Load_Table_Func           load_kerning;
	TT_Load_Table_Func           load_gasp;
	TT_Load_Table_Func           load_pclt;

	/* see `ttload.h' */
	TT_Load_Table_Func           load_bitmap_header;

	/* see `ttsbit.h' */
	TT_Set_SBit_Strike_Func      set_sbit_strike;
	TT_Load_Table_Func           load_sbits;
	TT_Load_SBit_Image_Func      load_sbit_image;
	TT_Free_Table_Func           free_sbits;

	/* see `ttpost.h' */
	TT_Get_PS_Name_Func          get_psname;
	TT_Free_Table_Func           free_psnames;

	/* see `ttcmap.h' */
	TT_CharMap_Load_Func         load_charmap;
	TT_CharMap_Free_Func         free_charmap;
} SFNT_Interface;


/* transitional */
typedef SFNT_Interface *SFNT_Service;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_SFNT_H */
