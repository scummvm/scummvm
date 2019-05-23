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

#include "glk/tads/tads2/data.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/run.h"
#include "glk/tads/tads2/vocabulary.h"
#include "glk/tads/os_glk.h"
#include "common/algorithm.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* return size of a data value */
uint datsiz(dattyp typ, const void *val)
{
    switch(typ)
    {
    case DAT_NUMBER:
        return(4);                /* numbers are in 4-byte lsb-first format */

    case DAT_OBJECT:
        return(2);         /* object numbers are in 2-byte lsb-first format */

    case DAT_SSTRING:
    case DAT_DSTRING:
    case DAT_LIST:
        return(osrp2((const char *)val));

    case DAT_NIL:
    case DAT_TRUE:
        return(0);

    case DAT_PROPNUM:
    case DAT_SYN:
    case DAT_FNADDR:
    case DAT_REDIR:
        return(2);
        
    case DAT_TPL:
        /* template is counted array of 10-byte entries, plus length byte */
        return(1 + ((*(const uchar *)val) * VOCTPLSIZ));

    case DAT_TPL2:
        return(1 + ((*(const uchar *)val) * VOCTPL2SIZ));

    default:
        return(0);
    }
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
