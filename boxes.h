/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#define SIZEOF_BOX 20
struct Box {				/* Internal walkbox file format */
	int16 ulx,uly;
	int16 urx,ury;
	int16 llx,lly;
	int16 lrx,lry;
	byte mask;
	byte flags;
	uint16 scale;
} GCC_PACK;

struct gate_location {
	int     x;
	int     y;
};

struct AdjustBoxResult {	/* Result type of AdjustBox functions */
	int16 x,y;
	uint16 dist;
};

struct BoxCoords {			/* Box coordinates */
	ScummPoint ul;
	ScummPoint ur;
	ScummPoint ll;
	ScummPoint lr;
};

struct PathNode {		/* Linked list of walkpath nodes */
	uint index;
	struct PathNode *left, *right;
};

struct PathVertex {		/* Linked list of walkpath nodes */
	PathNode *left;
	PathNode *right;
};