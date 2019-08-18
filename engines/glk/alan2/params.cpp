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

#include "glk/alan2/types.h"
#include "glk/alan2/params.h"

namespace Glk {
namespace Alan2 {

void compact(ParamElem a[]) {
	int i, j;

	for (i = 0, j = 0; a[j].code != (Aword)EOD; j++)
		if (a[j].code != 0)
			a[i++] = a[j];
	a[i].code = (Aword)EOD;
}

int lstlen(ParamElem a[]) {
	int i = 0;

	while (a[i].code != (Aword)EOD)
		i++;
	return (i);
}

Boolean inlst(ParamElem l[], Aword e) {
	int i;

	for (i = 0; l[i].code != (Aword)EOD && l[i].code != e; i++);
	return (l[i].code == e);
}

void lstcpy(ParamElem a[], ParamElem b[]) {
	int i;

	for (i = 0; b[i].code != (Aword)EOD; i++)
		a[i] = b[i];
	a[i].code = (Aword)EOD;
}

void sublst(ParamElem a[], ParamElem b[]) {
	int i;

	for (i = 0; a[i].code != (Aword)EOD; i++)
		if (inlst(b, a[i].code))
			a[i].code = 0;        /* Mark empty */
	compact(a);
}

void mrglst(ParamElem a[], ParamElem b[]) {
	int i, last;

	for (last = 0; a[last].code != (Aword)EOD; last++); /* Find end of list */
	for (i = 0; b[i].code != (Aword)EOD; i++)
		if (!inlst(a, b[i].code)) {
			a[last++] = b[i];
			a[last].code = (Aword)EOD;
		}
}

void isect(ParamElem a[], ParamElem b[]) {
	int i, last = 0;

	for (i = 0; a[i].code != (Aword)EOD; i++)
		if (inlst(b, a[i].code))
			a[last++] = a[i];
	a[last].code = (Aword)EOD;
}

void cpyrefs(ParamElem p[], Aword r[]) {
	int i;

	for (i = 0; r[i] != (Aword)EOD; i++) {
		p[i].code = r[i];
		p[i].firstWord = (Aword)EOD;
	}
	p[i].code = (Aword)EOD;
}

} // End of namespace Alan2
} // End of namespace Glk
