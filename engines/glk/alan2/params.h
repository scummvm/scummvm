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

#ifndef GLK_ALAN2_PARAMS
#define GLK_ALAN2_PARAMS

#include "glk/alan2/types.h"

/* Various utility functions for handling parameters:
 *
 *  compact()   Compact a list, i.e remove any NULL elements
 *  lstlen()    Count number of elements
 *  inlst()     Check if an element is in the list
 *  sublst()    Subract two lists
 *  lstcpy()    Copy one list onto another
 *  mrglst()    Merge the paramElems of one list into the first
 *  isect()     Take the intersection of two lists
 *  cpyrefs()   Copy the refs (in dictionary) to a paramList
 */

namespace Glk {
namespace Alan2 {

extern void compact(ParamElem *a);
extern int lstlen(ParamElem *a);
extern Boolean inlst(ParamElem *l, Aword e);
extern void lstcpy(ParamElem *a, ParamElem *b);
extern void sublst(ParamElem *a, ParamElem *b);
extern void mrglst(ParamElem *a, ParamElem *b);
extern void isect(ParamElem *a, ParamElem *b);
extern void cpyrefs(ParamElem *p, Aword *r);

} // End of namespace Alan2
} // End of namespace Glk

#endif
