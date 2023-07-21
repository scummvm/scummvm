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
/*  ftmemory.h                                                             */
/*    The FreeType memory management macros (specification).               */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTMEMORY_H
#define AGS_LIB_FREETYPE_FTMEMORY_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/config/ftconfig.h"
#include "engines/ags/lib/freetype-2.1.3/fttypes.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


#undef  FT2_1_3_SET_ERROR
#define FT2_1_3_SET_ERROR(expression) ((error = (expression)) != 0)


FT2_1_3_BASE(FT_Error)
FT_Alloc(FT_Memory memory, FT_Long size, void **P);

FT2_1_3_BASE(FT_Error)
FT_Realloc(FT_Memory memory, FT_Long current, FT_Long size, void **P);

FT2_1_3_BASE(void)
FT_Free(FT_Memory memory, void **P);


#define FT2_1_3_MEM_SET(dest, byte, count) ft_memset(dest, byte, count)
#define FT2_1_3_MEM_COPY(dest, source, count) ft_memcpy(dest, source, count)
#define FT2_1_3_MEM_MOVE(dest, source, count) ft_memmove(dest, source, count)
#define FT2_1_3_MEM_ZERO(dest, count) FT2_1_3_MEM_SET(dest, 0, count)
#define FT2_1_3_ZERO(p) FT2_1_3_MEM_ZERO(p, sizeof(*(p)))

#define FT2_1_3_MEM_ALLOC(_pointer_, _size_) FT_Alloc(memory, _size_, (void **)&(_pointer_))
#define FT2_1_3_MEM_FREE(_pointer_) FT_Free(memory, (void **)&(_pointer_))
#define FT2_1_3_MEM_REALLOC(_pointer_, _current_, _size_) FT_Realloc(memory, _current_, _size_, (void **)&(_pointer_))


/*************************************************************************/
/*                                                                       */
/* The following functions macros expect that their pointer argument is  */
/* _typed_ in order to automatically compute array element sizes.        */
/*                                                                       */

#define FT2_1_3_MEM_NEW(_pointer_) FT2_1_3_MEM_ALLOC(_pointer_, sizeof(*(_pointer_)))
#define FT2_1_3_MEM_NEW_ARRAY(_pointer_, _count_) FT2_1_3_MEM_ALLOC(_pointer_, (_count_) * sizeof(*(_pointer_)))
#define FT2_1_3_MEM_RENEW_ARRAY(_pointer_, _old_, _new_) FT2_1_3_MEM_REALLOC(_pointer_, (_old_) * sizeof(*(_pointer_)), (_new_) * sizeof(*(_pointer_)))

/*************************************************************************/
/*                                                                       */
/* the following macros are obsolete but kept for compatibility reasons  */
/*                                                                       */

#define FT2_1_3_MEM_ALLOC_ARRAY(_pointer_, _count_, _type_) FT2_1_3_MEM_ALLOC(_pointer_, (_count_) * sizeof(_type_))
#define FT2_1_3_MEM_REALLOC_ARRAY(_pointer_, _old_, _new_, _type_) FT2_1_3_MEM_REALLOC(_pointer_, (_old_) * sizeof(_type), (_new_) * sizeof(_type_))

/*************************************************************************/
/*                                                                       */
/* The following macros are variants of their FT2_1_3_MEM_XXXX equivalents;   */
/* they are used to set an _implicit_ `error' variable and return TRUE   */
/* if an error occured (i.e. if 'error != 0').                           */
/*                                                                       */

#define FT2_1_3_ALLOC(_pointer_, _size_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_ALLOC(_pointer_, _size_))
#define FT2_1_3_REALLOC(_pointer_, _current_, _size_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_REALLOC(_pointer_, _current_, _size_))
#define FT2_1_3_FREE(_pointer_) FT2_1_3_MEM_FREE(_pointer_)
#define FT2_1_3_NEW(_pointer_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_NEW(_pointer_))
#define FT2_1_3_NEW_ARRAY(_pointer_, _count_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_NEW_ARRAY(_pointer_, _count_))
#define FT2_1_3_RENEW_ARRAY(_pointer_, _old_, _new_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_RENEW_ARRAY(_pointer_, _old_, _new_))
#define FT2_1_3_ALLOC_ARRAY(_pointer_, _count_, _type_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_ALLOC(_pointer_, (_count_) * sizeof(_type_)))
#define FT2_1_3_REALLOC_ARRAY(_pointer_, _old_, _new_, _type_) FT2_1_3_SET_ERROR(FT2_1_3_MEM_REALLOC(_pointer_, (_old_) * sizeof(_type_), (_new_) * sizeof(_type_)))


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* __FTMEMORY_H__ */
