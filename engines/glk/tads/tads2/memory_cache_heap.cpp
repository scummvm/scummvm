/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/tads/tads2/memory_cache_heap.h"
#include "glk/tads/tads2/error.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* global to keep track of all allocations */
IF_DEBUG(ulong mchtotmem;)

uchar *mchalo(errcxdef *ctx, size_t siz, const char *comment) {
    uchar *ret;

    VARUSED(comment);
    IF_DEBUG(mchtotmem += siz;)

    ret = (uchar *)osmalloc(siz);
    if (ret)
        return(ret);
    else {
        errsig(ctx, ERR_NOMEM);
        NOTREACHEDV(uchar *);
        return 0;
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
