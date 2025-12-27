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

/************************************************************************/
/************************************************************************/
/*																		*/
/*							Draw World Objects							*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"						/* Our prototypes */

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*					Draw object in a given rectangle					*/
/*																		*/
/************************************************************************/

#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void DRAWTile(RECT *r, int Obj) {
	int x1 = r->left;   					/* Extract values */
	int y1 = r->top;
	int w = r->right - r->left;				/* Calculate width and height */
	int h = r->bottom - r->top;

	switch (Obj)								/* Decide on the object */
	{
	case WT_SPACE:							/* Space does nothing at all */
		break;

	case WT_MOUNTAIN:						/* Mountain the cracked effect */
		HWColour(COL_MOUNTAIN);
		HWLine(X(2), Y(6), X(2), Y(10));
		HWLine(X(0), Y(8), X(2), Y(8));
		HWLine(X(2), Y(6), X(4), Y(6));
		HWLine(X(4), Y(6), X(4), Y(4));
		HWLine(X(2), Y(2), X(4), Y(4));
		HWLine(X(2), Y(2), X(2), Y(0));
		HWLine(X(2), Y(2), X(0), Y(2));
		HWLine(X(8), Y(4), X(4), Y(4));
		HWLine(X(8), Y(4), X(8), Y(0));
		HWLine(X(8), Y(2), X(10), Y(2));
		HWLine(X(6), Y(4), X(6), Y(8));
		HWLine(X(10), Y(8), X(6), Y(8));
		HWLine(X(8), Y(8), X(8), Y(10));
		break;

	case WT_TREE:							/* Tree is just a box */
		HWColour(COL_TREE);
		BOX(3, 3, 7, 7);
		break;

	case WT_TOWN:							/* Down is 5 boxes */
		HWColour(COL_TOWN);
		BOX(2, 2, 4, 4); BOX(4, 4, 6, 6); BOX(6, 6, 8, 8);
		BOX(6, 2, 8, 4); BOX(2, 6, 4, 8);
		break;

	case WT_DUNGEON:						/* Dungeon is a cross */
		HWColour(COL_DUNGEON);
		HWLine(X(3), Y(3), X(7), Y(7));
		HWLine(X(7), Y(3), X(3), Y(7));
		break;

	case WT_BRITISH:						/* British castle */
		HWColour(COL_BRITISH);
		HWLine(X(2), Y(2), X(8), Y(8));
		HWLine(X(8), Y(2), X(2), Y(8));
		BOX(0, 0, 10, 10);
		BOX(2, 2, 8, 8);
		break;

	case WT_PLAYER:
		HWColour(COL_PLAYER);
		HWLine(X(4), Y(5), X(6), Y(5));
		HWLine(X(5), Y(4), X(5), Y(6));
		break;

	default:
		break;
	}
}

/************************************************************************/
/*																		*/
/*					Copy values into a rectangle						*/
/*																		*/
/************************************************************************/

void DRAWSetRect(RECT *r, int x1, int y1, int x2, int y2) {
	r->left = x1; r->right = x2;
	r->top = y1; r->bottom = y2;
}

} // namespace Ultima0
} // namespace Ultima
