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
/*  psnames.h                                                              */
/*    High-level interface for the `PSNames' module (in charge of          */
/*    various functions related to Postscript glyph names conversion).     */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_PSNAMES_H
#define AGS_LIB_FREETYPE_PSNAMES_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


typedef FT_UInt32 (*PS_Unicode_Value_Func)(const char *glyph_name);
typedef FT_UInt   (*PS_Unicode_Index_Func)(FT_UInt num_glyphs, const char **glyph_names, FT_ULong unicode);

typedef const char *(*PS_Macintosh_Name_Func)(FT_UInt name_index);

typedef const char *(*PS_Adobe_Std_Strings_Func)(FT_UInt string_index);


typedef struct PS_UniMap_ {
	FT_UInt unicode;
	FT_UInt glyph_index;
} PS_UniMap;

typedef struct PS_Unicodes_ {
	FT_UInt num_maps;
	PS_UniMap *maps;
} PS_Unicodes;

typedef FT_Error (*PS_Build_Unicodes_Func)(FT_Memory memory, FT_UInt num_glyphs, const char **glyph_names, PS_Unicodes *unicodes);
typedef FT_UInt  (*PS_Lookup_Unicode_Func)(PS_Unicodes *unicodes, FT_UInt unicode);
typedef FT_ULong (*PS_Next_Unicode_Func)(PS_Unicodes *unicodes, FT_ULong unicode);


typedef struct PSNames_Interface_ {
	PS_Unicode_Value_Func  unicode_value;
	PS_Build_Unicodes_Func build_unicodes;
	PS_Lookup_Unicode_Func lookup_unicode;
	PS_Macintosh_Name_Func macintosh_name;

	PS_Adobe_Std_Strings_Func  adobe_std_strings;
	const unsigned short	   *adobe_std_encoding;
	const unsigned short	   *adobe_expert_encoding;

	PS_Next_Unicode_Func next_unicode;
} PSNames_Interface;

typedef PSNames_Interface *PSNames_Service;


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_PSNAMES_H */
