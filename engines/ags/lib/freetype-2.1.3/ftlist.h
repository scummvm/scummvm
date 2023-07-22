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
/*  ftlist.h                                                               */
/*    Generic list support for FreeType (specification).                   */
/*                                                                         */
/***************************************************************************/

#ifndef AGS_LIB_FREETYPE_FTLIST_H
#define AGS_LIB_FREETYPE_FTLIST_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT_BEGIN_HEADER


FT_EXPORT(FT_ListNode)
FT_List_Find(FT_List list, void *data);

FT_EXPORT(void)
FT_List_Add(FT_List list, FT_ListNode node);

FT_EXPORT(void)
FT_List_Insert(FT_List list, FT_ListNode node);

FT_EXPORT(void)
FT_List_Remove(FT_List list, FT_ListNode node);

FT_EXPORT(void)
FT_List_Up(FT_List list, FT_ListNode node);

typedef FT_Error (*FT_List_Iterator)(FT_ListNode node, void *user);

FT_EXPORT( FT_Error )
FT_List_Iterate(FT_List list, FT_List_Iterator iterator, void *user);

typedef void (*FT_List_Destructor)(FT_Memory memory, void *data, void *user);

FT_EXPORT( void )
FT_List_Finalize(FT_List list, FT_List_Destructor destroy, FT_Memory memory, void *user);


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTLIST_H */
