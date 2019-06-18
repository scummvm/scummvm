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

#include <stdio.h>
#include "glk/alan2/types.h"
#include "glk/alan2/params.h"

namespace Glk {
namespace Alan2 {

#ifdef _PROTOTYPES_
void compact(ParamElem a[])
#else
void compact(a)
     ParamElem a[];
#endif
{
  int i, j;
  
  for (i = 0, j = 0; a[j].code != (Aword)EOF; j++)
    if (a[j].code != 0)
      a[i++] = a[j];
  a[i].code = (Aword)EOF;
}


#ifdef _PROTOTYPES_
int lstlen(ParamElem a[])
#else
int lstlen(a)
     ParamElem a[];
#endif
{
  int i = 0;

  while (a[i].code != (Aword)EOF)
    i++;
  return (i);
}


#ifdef _PROTOTYPES_
Boolean inlst(ParamElem l[], Aword e)
#else
Boolean inlst(l, e)
     ParamElem l[];
     Aword e;
#endif
{
  int i;

  for (i = 0; l[i].code != (Aword)EOF && l[i].code != e; i++);
  return (l[i].code == e);
}


#ifdef _PROTOTYPES_
void lstcpy(ParamElem a[], ParamElem b[])
#else
void lstcpy(a, b)
     ParamElem a[], b[];
#endif
{
  int i;

  for (i = 0; b[i].code != (Aword)EOF; i++)
    a[i] = b[i];
  a[i].code = (Aword)EOF;
}


#ifdef _PROTOTYPES_
void sublst(ParamElem a[], ParamElem b[])
#else
void sublst(a, b)
     ParamElem a[], b[];
#endif
{
  int i;

  for (i = 0; a[i].code != (Aword)EOF; i++)
    if (inlst(b, a[i].code))
      a[i].code = 0;		/* Mark empty */
  compact(a);
}


#ifdef _PROTOTYPES_
void mrglst(ParamElem a[], ParamElem b[])
#else
void mrglst(a, b)
     ParamElem a[], b[];
#endif
{
  int i,last;

  for (last = 0; a[last].code != (Aword)EOF; last++); /* Find end of list */
  for (i = 0; b[i].code != (Aword)EOF; i++)
    if (!inlst(a, b[i].code)) {
      a[last++] = b[i];
      a[last].code = (Aword)EOF;
    }
}


#ifdef _PROTOTYPES_
void isect(ParamElem a[], ParamElem b[])
#else
void isect(a, b)
     ParamElem a[], b[];
#endif
{
  int i, last = 0;

  for (i = 0; a[i].code != (Aword)EOF; i++)
    if (inlst(b, a[i].code))
      a[last++] = a[i];
  a[last].code = (Aword)EOF;
}


#ifdef _PROTOTYPES_
void cpyrefs(ParamElem p[], Aword r[])
#else
void cpyrefs(p, r)
     ParamElem p[];
     Aword r[];
#endif
{
  int i;

  for (i = 0; r[i] != (Aword)EOF; i++) {
    p[i].code = r[i];
    p[i].firstWord = (Aword)EOF;
  }
  p[i].code = (Aword)EOF;
}

} // End of namespace Alan2
} // End of namespace Glk
