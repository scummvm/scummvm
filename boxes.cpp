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

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include <math.h>

byte Scumm::getMaskFromBox(int box)
{
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;

	return ptr->mask;
}

byte Scumm::getBoxFlags(int box)
{
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;
	return ptr->flags;
}

int Scumm::getBoxScale(int box)
{
	if (_features & GF_NO_SCALLING)
		return (255);
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 255;
	return FROM_LE_16(ptr->scale);
}

byte Scumm::getNumBoxes()
{
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr)
		return 0;
	return ptr[0];
}

Box *Scumm::getBoxBaseAddr(int box)
{
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr)
		return NULL;
	checkRange(ptr[0] - 1, 0, box, "Illegal box %d");
	if (_features & GF_SMALL_HEADER) {
		if (_features & GF_OLD256)
			return (Box *) (ptr + box * (SIZEOF_BOX - 2) + 1);
		else
			return (Box *) (ptr + box * SIZEOF_BOX + 1);
	} else
		return (Box *) (ptr + box * SIZEOF_BOX + 2);
}

int Scumm::getSpecialBox(int param1, int param2)
{
	int i;
	int numOfBoxes;
	byte flag;
	
	numOfBoxes = getNumBoxes() - 1;

	for(i=numOfBoxes;i>=0;i--)
	{
		flag = getBoxFlags(i);

		if(!(flag & 0x80) && (flag & 0x20))
			return(-1);

		if(checkXYInBoxBounds(i,param1,param2))
			return(i);
	}

	return(-1);
}

bool Scumm::checkXYInBoxBounds(int b, int x, int y)
{
	BoxCoords box;

	if (b == 0 && (!(_features & GF_SMALL_HEADER)))
		return 0;

	getBoxCoordinates(b, &box);

	if (x < box.ul.x && x < box.ur.x && x < box.ll.x && x < box.lr.x)
		return 0;

	if (x > box.ul.x && x > box.ur.x && x > box.ll.x && x > box.lr.x)
		return 0;

	if (y < box.ul.y && y < box.ur.y && y < box.ll.y && y < box.lr.y)
		return 0;

	if (y > box.ul.y && y > box.ur.y && y > box.ll.y && y > box.lr.y)
		return 0;

	if (box.ul.x == box.ur.x &&
			box.ul.y == box.ur.y &&
			box.ll.x == box.lr.x &&
			box.ll.y == box.lr.y ||
			box.ul.x == box.lr.x &&
			box.ul.y == box.lr.y && box.ur.x == box.ll.x && box.ur.y == box.ll.y) {

		ScummPoint pt;
		pt = closestPtOnLine(box.ul.x, box.ul.y, box.ll.x, box.ll.y, x, y);
		if (distanceFromPt(x, y, pt.x, pt.y) <= 4)
			return 1;
	}

	if (!getSideOfLine(box.ul.x, box.ul.y, box.ur.x, box.ur.y, x, y, b))
		return 0;

	if (!getSideOfLine(box.ur.x, box.ur.y, box.ll.x, box.ll.y, x, y, b))
		return 0;

	if (!getSideOfLine(box.ll.x, box.ll.y, box.lr.x, box.lr.y, x, y, b))
		return 0;

	if (!getSideOfLine(box.lr.x, box.lr.y, box.ul.x, box.ul.y, x, y, b))
		return 0;

	return 1;
}

void Scumm::getBoxCoordinates(int boxnum, BoxCoords * box)
{
	Box *bp = getBoxBaseAddr(boxnum);

	box->ul.x = (int16) FROM_LE_16(bp->ulx);
	box->ul.y = (int16) FROM_LE_16(bp->uly);
	box->ur.x = (int16) FROM_LE_16(bp->urx);
	box->ur.y = (int16) FROM_LE_16(bp->ury);

	if (_features & GF_OLD256) {
		box->ll.x = (int16) FROM_LE_16(bp->lrx);
		box->ll.y = (int16) FROM_LE_16(bp->lry);
		box->lr.x = (int16) FROM_LE_16(bp->llx);
		box->lr.y = (int16) FROM_LE_16(bp->lly);
	} else {
		box->ll.x = (int16) FROM_LE_16(bp->llx);
		box->ll.y = (int16) FROM_LE_16(bp->lly);
		box->lr.x = (int16) FROM_LE_16(bp->lrx);
		box->lr.y = (int16) FROM_LE_16(bp->lry);
	}

}

uint Scumm::distanceFromPt(int x, int y, int ptx, int pty)
{
	int diffx, diffy;

	diffx = abs(ptx - x);

	if (diffx >= 0x100)
		return 0xFFFF;

	diffy = abs(pty - y);

	if (diffy >= 0x100)
		return 0xFFFF;
	diffx *= diffx;
	diffy *= diffy;
	return diffx + diffy;
}

bool Scumm::getSideOfLine(int x1, int y1, int x2, int y2, int x, int y,
													int box)
{
	return (x - x1) * (y2 - y1) <= (y - y1) * (x2 - x1);
}

ScummPoint Scumm::closestPtOnLine(int ulx, int uly, int llx, int lly, int x,
																	int y)
{
	int lydiff, lxdiff;
	int32 dist, a, b, c;
	int x2, y2;
	ScummPoint pt;

	if (llx == ulx) {
		x2 = ulx;
		y2 = y;
	} else if (lly == uly) {
		x2 = x;
		y2 = uly;
	} else {
		lydiff = lly - uly;

		lxdiff = llx - ulx;

		if (abs(lxdiff) > abs(lydiff)) {
			dist = lxdiff * lxdiff + lydiff * lydiff;

			a = ulx * lydiff / lxdiff;
			b = x * lxdiff / lydiff;

			c = (a + b - uly + y) * lydiff * lxdiff / dist;

			x2 = c;
			y2 = c * lydiff / lxdiff - a + uly;
		} else {
			dist = lydiff * lydiff + lxdiff * lxdiff;

			a = uly * lxdiff / lydiff;
			b = y * lydiff / lxdiff;

			c = (a + b - ulx + x) * lydiff * lxdiff / dist;

			y2 = c;
			x2 = c * lxdiff / lydiff - a + ulx;
		}
	}

	lxdiff = llx - ulx;
	lydiff = lly - uly;

	if (abs(lydiff) < abs(lxdiff)) {
		if (lxdiff > 0) {
			if (x2 < ulx) {
			type1:;
				x2 = ulx;
				y2 = uly;
			} else if (x2 > llx) {
			type2:;
				x2 = llx;
				y2 = lly;
			}
		} else {
			if (x2 > ulx)
				goto type1;
			if (x2 < llx)
				goto type2;
		}
	} else {
		if (lydiff > 0) {
			if (y2 < uly)
				goto type1;
			if (y2 > lly)
				goto type2;
		} else {
			if (y2 > uly)
				goto type1;
			if (y2 < lly)
				goto type2;
		}
	}

	pt.x = x2;
	pt.y = y2;
	return pt;
}

bool Scumm::inBoxQuickReject(int b, int x, int y, int threshold)
{
	int t;
	BoxCoords box;

	getBoxCoordinates(b, &box);

	if (threshold == 0)
		return 1;

	t = x - threshold;
	if (t > box.ul.x && t > box.ur.x && t > box.ll.x && t > box.lr.x)
		return 0;

	t = x + threshold;
	if (t < box.ul.x && t < box.ur.x && t < box.ll.x && t < box.lr.x)
		return 0;

	t = y - threshold;
	if (t > box.ul.y && t > box.ur.y && t > box.ll.y && t > box.lr.y)
		return 0;

	t = y + threshold;
	if (t < box.ul.y && t < box.ur.y && t < box.ll.y && t < box.lr.y)
		return 0;

	return 1;
}

AdjustBoxResult Scumm::getClosestPtOnBox(int b, int x, int y)
{
	ScummPoint pt;
	AdjustBoxResult best;
	uint dist;
	uint bestdist = (uint) 0xFFFF;
	BoxCoords box;

	getBoxCoordinates(b, &box);

	pt = closestPtOnLine(box.ul.x, box.ul.y, box.ur.x, box.ur.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.ur.x, box.ur.y, box.ll.x, box.ll.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.ll.x, box.ll.y, box.lr.x, box.lr.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.lr.x, box.lr.y, box.ul.x, box.ul.y, x, y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	best.dist = bestdist;
	return best;
}

byte *Scumm::getBoxMatrixBaseAddr()
{
	byte *ptr = getResourceAddress(rtMatrix, 1);
	if (*ptr == 0xFF)
		ptr++;
	return ptr;
}

int Scumm::getPathToDestBox(byte from, byte to)
{
	byte *boxm;
	byte i;

	if (from == to)
		return to;

	boxm = getBoxMatrixBaseAddr();

	i = 0;
	while (i != from) {
		while (*boxm != 0xFF)
			boxm += 3;
		i++;
		boxm++;
	}

	while (boxm[0] != 0xFF) {
		if (boxm[0] <= to && boxm[1] >= to)
			return boxm[2];
		boxm += 3;
	}
	return -1;
}

int Scumm::findPathTowards(Actor * a, byte box1nr, byte box2nr, byte box3nr)
{
	BoxCoords box1;
	BoxCoords box2;
	ScummPoint tmp;
	int i, j;
	int flag;
	int q, pos;

	getBoxCoordinates(box1nr, &box1);
	getBoxCoordinates(box2nr, &box2);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (box1.ul.x == box1.ur.x &&
					box1.ul.x == box2.ul.x && box1.ul.x == box2.ur.x) {
				flag = 0;
				if (box1.ul.y > box1.ur.y) {
					SWAP(box1.ul.y, box1.ur.y);
					flag |= 1;
				}

				if (box2.ul.y > box2.ur.y) {
					SWAP(box2.ul.y, box2.ur.y);
					flag |= 2;
				}

				if (box1.ul.y > box2.ur.y || box2.ul.y > box1.ur.y ||
						(box1.ur.y == box2.ul.y || box2.ur.y == box1.ul.y) &&
						box1.ul.y != box1.ur.y && box2.ul.y != box2.ur.y) {
					if (flag & 1)
						SWAP(box1.ul.y, box1.ur.y);
					if (flag & 2)
						SWAP(box2.ul.y, box2.ur.y);
				} else {
					if (box2nr == box3nr) {
						int diffX = a->walkdata.destx - a->x;
						int diffY = a->walkdata.desty - a->y;
						int boxDiffX = box1.ul.x - a->x;

						if (diffX != 0) {
							int t;

							diffY *= boxDiffX;
							t = diffY / diffX;
							if (t == 0 && (diffY <= 0 || diffX <= 0)
									&& (diffY >= 0 || diffX >= 0))
								t = -1;
							pos = a->y + t;
						} else {
							pos = a->y;
						}
					} else {
						pos = a->y;
					}

					q = pos;
					if (q < box2.ul.y)
						q = box2.ul.y;
					if (q > box2.ur.y)
						q = box2.ur.y;
					if (q < box1.ul.y)
						q = box1.ul.y;
					if (q > box1.ur.y)
						q = box1.ur.y;
					if (q == pos && box2nr == box3nr)
						return 1;
					_foundPathY = q;
					_foundPathX = box1.ul.x;
					return 0;
				}
			}

			if (box1.ul.y == box1.ur.y &&
					box1.ul.y == box2.ul.y && box1.ul.y == box2.ur.y) {
				flag = 0;
				if (box1.ul.x > box1.ur.x) {
					SWAP(box1.ul.x, box1.ur.x);
					flag |= 1;
				}

				if (box2.ul.x > box2.ur.x) {
					SWAP(box2.ul.x, box2.ur.x);
					flag |= 2;
				}

				if (box1.ul.x > box2.ur.x || box2.ul.x > box1.ur.x ||
						(box1.ur.x == box2.ul.x || box2.ur.x == box1.ul.x) &&
						box1.ul.x != box1.ur.x && box2.ul.x != box2.ur.x) {
					if (flag & 1)
						SWAP(box1.ul.x, box1.ur.x);
					if (flag & 2)
						SWAP(box2.ul.x, box2.ur.x);
				} else {

					if (box2nr == box3nr) {
						int diffX = a->walkdata.destx - a->x;
						int diffY = a->walkdata.desty - a->y;
						int boxDiffY = box1.ul.y - a->y;

						pos = a->x;
						if (diffY != 0) {
							pos += diffX * boxDiffY / diffY;
						}
					} else {
						pos = a->x;
					}

					q = pos;
					if (q < box2.ul.x)
						q = box2.ul.x;
					if (q > box2.ur.x)
						q = box2.ur.x;
					if (q < box1.ul.x)
						q = box1.ul.x;
					if (q > box1.ur.x)
						q = box1.ur.x;
					if (q == pos && box2nr == box3nr)
						return 1;
					_foundPathX = q;
					_foundPathY = box1.ul.y;
					return 0;
				}
			}
			tmp = box1.ul;
			box1.ul = box1.ur;
			box1.ur = box1.ll;
			box1.ll = box1.lr;
			box1.lr = tmp;
		}
		tmp = box2.ul;
		box2.ul = box2.ur;
		box2.ur = box2.ll;
		box2.ll = box2.lr;
		box2.lr = tmp;
	}
	return 0;
}
void Scumm::setBoxFlags(int box, int val)
{
	/* FULL_THROTTLE stuff */
	if (val & 0xC000) {
		assert(box >= 0 && box < 65);
		_extraBoxFlags[box] = val;
	} else {
		Box *b = getBoxBaseAddr(box);
		b->flags = val;
	}
}

void Scumm::setBoxScale(int box, int scale)
{
	Box *b = getBoxBaseAddr(box);
	b->scale = scale;
}

#define BOX_MATRIX_SIZE 2000

void Scumm::createBoxMatrix()
{
	byte *matrix_ptr;
	int num, i, j;
	byte flags;
	int table_1[66], table_2[66];
	int counter, val;
	int code;

	PathVertex *vtx;
	PathNode *node, *node2 = NULL;

	_maxBoxVertexHeap = 1000;

	createResource(rtMatrix, 4, 1000);
	createResource(rtMatrix, 3, 4160);	//65 items of something of size 64
	createResource(rtMatrix, 1, BOX_MATRIX_SIZE);

	matrix_ptr = getResourceAddress(rtMatrix, 1);

	_boxMatrixPtr4 = getResourceAddress(rtMatrix, 4);
	_boxMatrixPtr1 = getResourceAddress(rtMatrix, 1);
	_boxMatrixPtr3 = getResourceAddress(rtMatrix, 3);

	_boxPathVertexHeapIndex = _boxMatrixItem = 0;

	num = getNumBoxes();

	for (i = 0; i < num; i++) {
		for (j = 0; j < num; j++) {
			if (i == j) {
				_boxMatrixPtr3[i * 64 + j] = 0;
			} else if (areBoxesNeighbours(i, j)) {
				_boxMatrixPtr3[i * 64 + j] = 1;
			} else {
				_boxMatrixPtr3[i * 64 + j] = 250;
			}
		}
	}

	for (j = 0; j < num; j++) {
		flags = getBoxFlags(j);
		if (flags & 0x80) {
			addToBoxMatrix(0xFF);
			addToBoxMatrix(j);
			addToBoxMatrix(j);
			addToBoxMatrix(j);
		} else {
			vtx = addPathVertex();
			for (i = 0; i < num; i++) {
				flags = getBoxFlags(j);
				if (!(flags & 0x80)) {
					node = unkMatrixProc2(vtx, i);
					if (i == j)
						node2 = node;
				}
			}
			table_1[j] = 0;
			table_2[j] = j;
			vtx = unkMatrixProc1(vtx, node2);
			node = vtx ? vtx->left : NULL;

			counter = 250;
			while (node) {
				val = _boxMatrixPtr3[j * 64 + node->index];
				table_1[node->index] = val;
				if (val < counter)
					counter = val;

				if (table_1[node->index] != 250)
					table_2[node->index] = node->index;
				else
					table_2[node->index] = -1;

				node = node->left;
			}

			while (vtx) {
				counter = 250;
				node2 = node = vtx->left;

				while (node) {
					if (table_1[node->index] < counter) {
						counter = table_1[node->index];
						node2 = node;
					}
					node = node->left;
				}
				vtx = unkMatrixProc1(vtx, node2);
				node = vtx ? vtx->left : NULL;
				while (node) {
					code = _boxMatrixPtr3[node2->index * 64 + node->index];
					code += table_1[node2->index];
					if (code < table_1[node->index]) {
						table_1[node->index] = code;
						table_2[node->index] = table_2[node2->index];
					}
					node = node->left;
				}
			}

			addToBoxMatrix(0xFF);
			for (i = 1; i < num;) {
				if (table_2[i - 1] != -1) {
					addToBoxMatrix(i - 1);	/* lo */
					if (table_2[i - 1] != table_2[i]) {
						addToBoxMatrix(i - 1);	/* hi */
						addToBoxMatrix(table_2[i - 1]);	/* dst */
					} else {
						while (table_2[i - 1] == table_2[i]) {
							if (++i == num)
								break;
						}
						addToBoxMatrix(i - 1);	/* hi */
						addToBoxMatrix(table_2[i - 1]);	/* dst */
					}
				}
				if (++i == num && table_2[i - 1] != -1) {
					addToBoxMatrix(i - 1);	/* lo */
					addToBoxMatrix(i - 1);	/* hi */
					addToBoxMatrix(table_2[i - 1]);	/* dest */
				}
			}
		}
	}

	addToBoxMatrix(0xFF);
	nukeResource(rtMatrix, 4);
	nukeResource(rtMatrix, 3);
}

PathVertex *Scumm::unkMatrixProc1(PathVertex * vtx, PathNode * node)
{
	if (node == NULL || vtx == NULL)
		return NULL;

	if (!node->right) {
		vtx->left = node->left;
	} else {
		node->right->left = node->left;
	}

	if (!node->left) {
		vtx->right = node->right;
	} else {
		node->left->right = node->right;
	}

	if (vtx->left)
		return vtx;

	return NULL;
}

PathNode *Scumm::unkMatrixProc2(PathVertex * vtx, int i)
{
	PathNode *node;

	if (vtx == NULL)
		return NULL;

	if (!vtx->right) {
		node = (PathNode *) addToBoxVertexHeap(sizeof(PathNode));
		vtx->left = vtx->right = node;

		node->index = i;
		node->left = 0;
		node->right = 0;
	} else {
		node = (PathNode *) addToBoxVertexHeap(sizeof(PathNode));
		vtx->right->left = node;

		node->right = vtx->right;
		node->index = i;
		node->left = 0;

		vtx->right = node;
	}

	return vtx->right;
}

/* Check if two boxes are neighbours */
bool Scumm::areBoxesNeighbours(int box1nr, int box2nr)
{
	int j, k, m, n;
	int tmp_x, tmp_y;
	bool result;
	BoxCoords box;
	BoxCoords box2;

	if (getBoxFlags(box1nr) & 0x80 || getBoxFlags(box2nr) & 0x80)
		return false;

	getBoxCoordinates(box1nr, &box2);
	getBoxCoordinates(box2nr, &box);

	result = false;
	j = 4;

	do {
		k = 4;
		do {
			if (box2.ur.x == box2.ul.x &&
					box.ul.x == box2.ul.x && box.ur.x == box2.ur.x) {
				n = m = 0;
				if (box2.ur.y < box2.ul.y) {
					n = 1;
					SWAP(box2.ur.y, box2.ul.y);
				}
				if (box.ur.y < box.ul.y) {
					m = 1;
					SWAP(box.ur.y, box.ul.y);
				}
				if (box.ur.y < box2.ul.y ||
						box.ul.y > box2.ur.y ||
						(box.ul.y == box2.ur.y ||
						 box.ur.y == box2.ul.y) &&
						box2.ur.y != box2.ul.y && box.ul.y != box.ur.y) {
					if (n) {
						SWAP(box2.ur.y, box2.ul.y);
					}
					if (m) {
						SWAP(box.ur.y, box.ul.y);
					}
				} else {
					if (n) {
						SWAP(box2.ur.y, box2.ul.y);
					}
					if (m) {
						SWAP(box.ur.y, box.ul.y);
					}
					result = true;
				}
			}

			if (box2.ur.y == box2.ul.y &&
					box.ul.y == box2.ul.y && box.ur.y == box2.ur.y) {
				n = m = 0;
				if (box2.ur.x < box2.ul.x) {
					n = 1;
					SWAP(box2.ur.x, box2.ul.x);
				}
				if (box.ur.x < box.ul.x) {
					m = 1;
					SWAP(box.ur.x, box.ul.x);
				}
				if (box.ur.x < box2.ul.x ||
						box.ul.x > box2.ur.x ||
						(box.ul.x == box2.ur.x ||
						 box.ur.x == box2.ul.x) &&
						box2.ur.x != box2.ul.x && box.ul.x != box.ur.x) {

					if (n) {
						SWAP(box2.ur.x, box2.ul.x);
					}
					if (m) {
						SWAP(box.ur.x, box.ul.x);
					}
				} else {
					if (n) {
						SWAP(box2.ur.x, box2.ul.x);
					}
					if (m) {
						SWAP(box.ur.x, box.ul.x);
					}
					result = true;
				}
			}

			tmp_x = box2.ul.x;
			tmp_y = box2.ul.y;
			box2.ul.x = box2.ur.x;
			box2.ul.y = box2.ur.y;
			box2.ur.x = box2.ll.x;
			box2.ur.y = box2.ll.y;
			box2.ll.x = box2.lr.x;
			box2.ll.y = box2.lr.y;
			box2.lr.x = tmp_x;
			box2.lr.y = tmp_y;
		} while (--k);

		tmp_x = box.ul.x;
		tmp_y = box.ul.y;
		box.ul.x = box.ur.x;
		box.ul.y = box.ur.y;
		box.ur.x = box.ll.x;
		box.ur.y = box.ll.y;
		box.ll.x = box.lr.x;
		box.ll.y = box.lr.y;
		box.lr.x = tmp_x;
		box.lr.y = tmp_y;
	} while (--j);

	return result;
}

void Scumm::addToBoxMatrix(byte b)
{
	if (++_boxMatrixItem > BOX_MATRIX_SIZE)
		error("Box matrix overflow");
	*_boxMatrixPtr1++ = b;
}

void *Scumm::addToBoxVertexHeap(int size)
{
	byte *ptr = _boxMatrixPtr4;

	_boxMatrixPtr4 += size;
	_boxPathVertexHeapIndex += size;

	if (_boxPathVertexHeapIndex >= _maxBoxVertexHeap)
		error("Box path vertex heap overflow");

	return ptr;
}

PathVertex *Scumm::addPathVertex()
{
	_boxMatrixPtr4 = getResourceAddress(rtMatrix, 4);
	_boxPathVertexHeapIndex = 0;

	return (PathVertex *) addToBoxVertexHeap(sizeof(PathVertex));
}

int Scumm::findPathTowardsOld(Actor * a, byte trap1, byte trap2,
															byte final_trap)
{
	GetGates(trap1, trap2);
	ScummPoint pt;
	// FIXME - is parameter "a" ignored on purpose?!?
	Actor *actor = getFirstActor();

	gateLoc[1].x = actor->x;
	gateLoc[1].y = actor->y;
	gateLoc[2].x = 32000;
	gateLoc[3].x = 32000;
	gateLoc[4].x = 32000;

	if (trap2 == final_trap) {		/* next = final box? */
		gateLoc[4].x = actor->walkdata.destx;
		gateLoc[4].y = actor->walkdata.desty;

		if (getMaskFromBox(trap1) == getMaskFromBox(trap2) || 1) {
			if (CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gate1ax, gate1ay) !=
					CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gate1bx, gate1by) &&
					CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gate2ax, gate2ay) !=
					CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[4].x, gateLoc[4].y, gate2bx, gate2by)) {
				return 0;								/* same zplane and between both gates? */
			}
		}
	}

	pt = closestPtOnLine(gate2ax, gate2ay, gate2bx, gate2by, gateLoc[1].x, gateLoc[1].y);
	gateLoc[3].x = pt.x;
	gateLoc[3].y = pt.y;

	if (CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[3].x, gateLoc[3].y, gate1ax, gate1ay) ==
			CompareSlope(gateLoc[1].x, gateLoc[1].y, gateLoc[3].x, gateLoc[3].y, gate1bx, gate1by)) {
		closestPtOnLine(gate1ax, gate1ay, gate1bx, gate1by, gateLoc[1].x, gateLoc[1].y);
		gateLoc[2].x = pt.x;							/* if point 2 between gates, ignore! */
		gateLoc[2].y = pt.y;
	}

	return 0;
}

void Scumm::GetGates(int trap1, int trap2)
{
	int i;
	int Closest1 = 0, Closest2 = 0, Closest3 = 0;
	int Dist[8];
	int Dist1, Dist2, Dist3;
	int Box1, Box2, Box3;
	BoxCoords box;
	int polyx[8];
	int polyy[8];
	AdjustBoxResult pt;

	getBoxCoordinates(trap1, &box);
	polyx[0] = box.ul.x;
	polyy[0] = box.ul.y;
	polyx[1] = box.ur.x;
	polyy[1] = box.ur.y;
	polyx[2] = box.ll.x;
	polyy[2] = box.ll.y;
	polyx[3] = box.lr.x;
	polyy[3] = box.lr.y;
	for (i = 0; i < 4; i++) {
		pt = getClosestPtOnBox(trap2, polyx[i], polyy[i]);
		Dist[i] = pt.dist;
		CloX[i] = pt.x;
		CloY[i] = pt.y;
	}

	getBoxCoordinates(trap2, &box);
	polyx[4] = box.ul.x;
	polyy[4] = box.ul.y;
	polyx[5] = box.ur.x;
	polyy[5] = box.ur.y;
	polyx[6] = box.ll.x;
	polyy[6] = box.ll.y;
	polyx[7] = box.lr.x;
	polyy[7] = box.lr.y;
	for (i = 4; i < 8; i++) {
		pt = getClosestPtOnBox(trap1, polyx[i], polyy[i]);
		Dist[i] = pt.dist;
		CloX[i] = pt.x;
		CloY[i] = pt.y;
	}


	Dist1 = 0xFFFF;
	for (i = 0; i < 8; i++) {
		if (Dist[i] < Dist1) {
			Dist1 = Dist[i];
			Closest1 = i;
		}
	}
	Dist[Closest1] = 0xFFFF;

	Dist2 = 0xFFFF;
	for (i = 0; i < 8; i++) {
		if (Dist[i] < Dist2) {
			Dist2 = Dist[i];
			Closest2 = i;
		}
	}
	Dist[Closest2] = 0xFFFF;

	Dist3 = 0xFFFF;
	for (i = 0; i < 8; i++) {
		if (Dist[i] < Dist3) {
			Dist3 = Dist[i];
			Closest3 = i;
		}
	}

	Box1 = (Closest1 > 3);
	Box2 = (Closest2 > 3);
	Box3 = (Closest3 > 3);

	Dist1 = (int)sqrt(Dist1);
	Dist2 = (int)sqrt(Dist2);
	Dist3 = (int)sqrt(Dist3);

	if (Box1 == Box2 && abs(Dist1 - Dist2) < 4) {
		SetGate(Closest1, Closest2, polyx, polyy);

	} else if (Box1 == Box2 && Dist1 == Dist2) {	/* parallel */
		SetGate(Closest1, Closest2, polyx, polyy);
	} else if (Box1 == Box3 && Dist1 == Dist3) {	/* parallel */
		SetGate(Closest1, Closest3, polyx, polyy);
	} else if (Box2 == Box3 && Dist2 == Dist3) {	/* parallel */
		SetGate(Closest2, Closest3, polyx, polyy);


	} else if (Box1 == Box3 && abs(Dist1 - Dist3) < 4) {
		SetGate(Closest1, Closest3, polyx, polyy);
	} else if (abs(Dist1 - Dist3) < 4) {	/* if 1 close to 3 then use 2-3 */
		SetGate(Closest2, Closest3, polyx, polyy);
	} else if (abs(Dist1 - Dist2) < 4) {
		SetGate(Closest1, Closest2, polyx, polyy);
	} else {
		SetGate(Closest1, Closest1, polyx, polyy);
	}
}

int Scumm::CompareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3)
{
	if ((Y2 - Y1) * (X3 - X1) > (Y3 - Y1) * (X2 - X1))
		return (0);
	return (1);
}

void Scumm::SetGate(int line1, int line2, int polyx[8], int polyy[8])
{

	if (line1 < 4) {							/* from box 1 to box 2 */
		gate1ax = polyx[line1];
		gate1ay = polyy[line1];
		gate2ax = CloX[line1];
		gate2ay = CloY[line1];

	} else {
		gate2ax = polyx[line1];
		gate2ay = polyy[line1];
		gate1ax = CloX[line1];
		gate1ay = CloY[line1];
	}

	if (line2 < 4) {							/* from box */
		gate1bx = polyx[line2];
		gate1by = polyy[line2];
		gate2bx = CloX[line2];
		gate2by = CloY[line2];

	} else {
		gate2bx = polyx[line2];
		gate2by = polyy[line2];
		gate1bx = CloX[line2];
		gate1by = CloY[line2];
	}
}
