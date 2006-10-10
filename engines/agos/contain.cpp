/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

int AGOSEngine::canPlace(Item *x, Item *y) {
	Item *z = derefItem(x->parent);
	SubObject *o = (SubObject *)findChildOfType(y, 2);
	int ct;
	int cap = 0;

	if (o == NULL)
		return(0);	/* Fits Fine */

	xPlace(x,NULL);		/* Avoid disturbing figures */
	if (o)
		cap = sizeContents(y);

	xPlace(x, z);
	if ((o) && (o->objectFlags & kOFVolume)) {
		ct = getOffsetOfChild2Param(o, kOFVoice);
		cap = o->objectFlagValue[ct] - cap;
		cap -= sizeOfRec(x, 0);	/* - size of item going in */
		if (cap < 0)
			return -1;	/* Too big to fit */
	}

	return 0;
}

void AGOSEngine::xPlace(Item *x, Item *y) {
	if (x->parent != 0)
		unlinkItem(x);

	linkItem(x, y);
}

int AGOSEngine::sizeContents(Item *x) {
	return sizeRec(x, 0);
}

int AGOSEngine::sizeRec(Item *x, int d) {
	Item *o;
	int n = 0;

	o = derefItem(x->child);

	if (d > 32)
		return(0);
	while (o) {
		n += sizeOfRec(o,d);
		o = derefItem(o->child);
	}

	return n;
}

int AGOSEngine::sizeOfRec(Item *o, int d) {
	SubObject *a = (SubObject *)findChildOfType(o, 2);
	int ct;

	if ((a) && (a->objectFlags & kOFSoft)) {
		if (a->objectFlags & kOFSize) {
			ct = getOffsetOfChild2Param(a, kOFSize);
			return a->objectFlagValue[ct] + sizeRec(o, d + 1);
		}
		return sizeRec(o, d + 1);
	}
	if ((a) && (a->objectFlags & kOFSize)) {
		ct = getOffsetOfChild2Param(a, kOFSize);
		return a->objectFlagValue[ct];
	}
	return 0;
}

int AGOSEngine::weighUp(Item *x) {
	return weightRec(x, 0);
}

int AGOSEngine::weightRec(Item *x, int d) {
	int n = weightOf(x);
	Item *o;

	if (d > 32)
		return 0;
	o = derefItem(x->child);
	while (o) {
		n += weightRec(o, d + 1);
		o = derefItem(o->next);
	}

	return n;
}

int AGOSEngine::weightOf(Item *x) {
	SubObject *o = (SubObject *)findChildOfType(x, 2);
	SubPlayer *p = (SubPlayer *)findChildOfType(x, 3);
	if (o)
		return o->objectWeight;
	if (p)
		return p->weight;
	return 0;
}

} // End of namespace AGOS
