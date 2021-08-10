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

#ifndef DIRECTOR_LINGO_LINGO_UTILS_H
#define DIRECTOR_LINGO_LINGO_UTILS_H

#define ARGNUMCHECK(n) \
	if (nargs != (n)) { \
		warning("%s: expected %d argument%s, got %d", __FUNCTION__, (n), ((n) == 1 ? "" : "s"), nargs); \
		g_lingo->dropStack(nargs); \
		return; \
	}

#define TYPECHECK(datum,t) \
	if ((datum).type != (t)) { \
		warning("%s: %s arg should be of type %s, not %s", __FUNCTION__, #datum, #t, (datum).type2str()); \
		return; \
	}

#define TYPECHECK2(datum, t1, t2)	\
	if ((datum).type != (t1) && (datum).type != (t2)) { \
		warning("%s: %s arg should be of type %s or %s, not %s", __FUNCTION__, #datum, #t1, #t2, (datum).type2str()); \
		return; \
	}

#define TYPECHECK3(datum, t1, t2, t3)	\
	if ((datum).type != (t1) && (datum).type != (t2) && (datum).type != (t3)) { \
		warning("%s: %s arg should be of type %s, %s, or %s, not %s", __FUNCTION__, #datum, #t1, #t2, #t3, (datum).type2str()); \
		return; \
	}

#define ARRBOUNDSCHECK(idx,array) \
	if ((idx)-1 < 0 || (idx) > (int)(array).u.farr->arr.size()) { \
		warning("%s: index out of bounds (%d of %d)", __FUNCTION__, (idx), (array).u.farr->arr.size()); \
		return; \
	}

#endif
