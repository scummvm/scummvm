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
/*  cffparse.h                                                             */
/*    CFF token stream parser (specification)                              */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_CFF_PARSE_H
#define AGS_LIB_FREETYPE_CFF_PARSE_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/cff/cfftypes.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


#define CFF_MAX_STACK_DEPTH  96

#define CFF_CODE_TOPDICT  0x1000
#define CFF_CODE_PRIVATE  0x2000


typedef struct CFF_ParserRec_ {
	FT_Byte *start;
	FT_Byte *limit;
	FT_Byte *cursor;

	FT_Byte *stack[CFF_MAX_STACK_DEPTH + 1];
	FT_Byte **top;

	FT_UInt object_code;
	void    *object;
} CFF_ParserRec, *CFF_Parser;


FT_LOCAL(void)
cff_parser_init(CFF_Parser parser, FT_UInt code, void *object);

FT_LOCAL(FT_Error)
cff_parser_run(CFF_Parser parser, FT_Byte *start, FT_Byte *limit);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_CFF_PARSE_H */
