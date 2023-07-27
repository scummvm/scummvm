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

/*
 *
 *  fthash.h  - fast dynamic hash tables
 *
 *  Copyright 2002 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg
 *
 *  This file is part of the FreeType project, and may only be used,
 *  modified, and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 */

#ifndef AGS_LIB_FREETYPE_FTHASH_H
#define AGS_LIB_FREETYPE_FTHASH_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/fttypes.h"

namespace AGS3 {
namespace FreeType213 {


FT_BEGIN_HEADER


typedef struct FT_HashRec_ *FT_Hash;
typedef struct FT_HashNodeRec_ *FT_HashNode;
typedef FT_HashNode *FT_HashLookup;

typedef FT_Int (*FT_Hash_EqualFunc)(FT_HashNode node1, FT_HashNode node2);

typedef struct FT_HashRec_ {
	FT_HashNode			 *buckets;
	FT_UInt              p;
	FT_UInt              mask;  /* really maxp-1 */
	FT_Long              slack;
	FT_Hash_EqualFunc    node_equal;
	FT_Memory            memory;
} FT_HashRec;

typedef struct FT_HashNodeRec_ {
	FT_HashNode  link;
	FT_UInt32    hash;
} FT_HashNodeRec;

typedef void (*FT_Hash_ForeachFunc)(const FT_HashNode node, const FT_Pointer data);

// TODO: These functions are only used in ftobject and should be removed
FT_BASE(FT_Error)
ft_hash_init(FT_Hash table, FT_Hash_EqualFunc compare, FT_Memory memory);

FT_BASE_DEF(FT_HashLookup)
ft_hash_lookup(FT_Hash table, FT_HashNode keynode);

FT_BASE(FT_Error)
ft_hash_add(FT_Hash table, FT_HashLookup lookup, FT_HashNode new_node);

FT_BASE(FT_Error)
ft_hash_remove(FT_Hash table, FT_HashLookup lookup);

FT_BASE(FT_UInt)
ft_hash_get_size(FT_Hash table);

FT_BASE(void)
ft_hash_foreach(FT_Hash table, FT_Hash_ForeachFunc foreach_func, const FT_Pointer foreach_data);

FT_BASE(void)
ft_hash_done(FT_Hash table, FT_Hash_ForeachFunc item_func, const FT_Pointer item_data);

/* compute bucket index from hash value in a dynamic hash table */
/* this is only used to break encapsulation to speed lookups in */
/* the FreeType cache manager !!                                */
/*                                                              */

// TODO: remove
#define FT_HASH_COMPUTE_INDEX(_table, _hash, _index)     \
	{                                                         \
		FT_UInt _mask = (_table)->mask;                       \
		FT_UInt _hash0 = (_hash);                             \
                                                              \
		(_index) = (FT_UInt)((_hash0)&_mask);                 \
		if ((_index) < (_table)->p)                           \
			(_index) = (FT_UInt)((_hash0) & (2 * _mask + 1)); \
	}


FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTHASH_H */
