/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

byte Scumm::getMaskFromBox(int box) {
	Box *ptr = getBoxBaseAddr(box);
	return ptr->mask;
}

byte Scumm::getBoxFlags(int box) {
	Box *ptr = getBoxBaseAddr(box);
	return ptr->flags;
}

int Scumm::getBoxScale(int box) {
	Box *ptr = getBoxBaseAddr(box);
	return FROM_LE_16(ptr->scale);
}

byte Scumm::getNumBoxes() {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	return ptr[8];
}

Box *Scumm::getBoxBaseAddr(int box) {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	checkRange(ptr[8]-1, 0, box, "Illegal box %d");
	return (Box*)(ptr + box*SIZEOF_BOX + 10);
}

bool Scumm::checkXYInBoxBounds(int b, int x, int y) {
	if (b==0)
		return 0;

	getBoxCoordinates(b);

	if (x < box.upperLeftX && x < box.upperRightX &&
		x < box.lowerLeftX && x < box.lowerRightX)
			return 0;
	
	if (x > box.upperLeftX && x > box.upperRightX &&
		x > box.lowerLeftX && x > box.lowerRightX)
			return 0;
	
	if (y < box.upperLeftY && y < box.upperRightY &&
		y < box.lowerLeftY && y < box.lowerRightY)
			return 0;

	if (y > box.upperLeftY && y > box.upperRightY &&
		y > box.lowerLeftY && y > box.lowerRightY)
			return 0;
	
	if (box.upperLeftX == box.upperRightX &&
		box.upperLeftY == box.upperRightY &&
		box.lowerLeftX == box.lowerRightX &&
		box.lowerLeftY == box.lowerRightY ||
		box.upperLeftX == box.lowerRightX &&
		box.upperLeftY == box.lowerRightY &&
		box.upperRightX== box.lowerLeftX &&
		box.upperRightY== box.lowerLeftY) {

		Point pt;
		pt = closestPtOnLine(box.upperLeftX, box.upperLeftY, box.lowerLeftX, box.lowerLeftY, x, y);
		if (distanceFromPt(x, y, pt.x,pt.y) <= 4)
			return 1;
	}
	
	if (!getSideOfLine(
		box.upperLeftX, box.upperLeftY, box.upperRightX, box.upperRightY, x,y,b))
			return 0;
	
	if (!getSideOfLine(
		box.upperRightX, box.upperRightY, box.lowerLeftX, box.lowerLeftY, x,y,b))
			return 0;

	if (!getSideOfLine(
		box.lowerLeftX, box.lowerLeftY, box.lowerRightX, box.lowerRightY, x,y,b))
			return 0;

	if (!getSideOfLine(
		box.lowerRightX, box.lowerRightY, box.upperLeftX, box.upperLeftY, x,y,b))
			return 0;

	return 1;
}

void Scumm::getBoxCoordinates(int b) {
	Box *bp = getBoxBaseAddr(b);
	box.upperLeftX = (int16)FROM_LE_16(bp->ulx);
	box.upperRightX = (int16)FROM_LE_16(bp->urx);
	box.lowerLeftX = (int16)FROM_LE_16(bp->llx);
	box.lowerRightX = (int16)FROM_LE_16(bp->lrx);
	box.upperLeftY = (int16)FROM_LE_16(bp->uly);
	box.upperRightY = (int16)FROM_LE_16(bp->ury);
	box.lowerLeftY = (int16)FROM_LE_16(bp->lly);
	box.lowerRightY = (int16)FROM_LE_16(bp->lry);
}

uint Scumm::distanceFromPt(int x, int y, int ptx, int pty) {
	int diffx, diffy;
	
	diffx = abs(ptx-x);

	if (diffx >= 0x100)
		return 0xFFFF;
	
	diffy = abs(pty - y);

	if (diffy >= 0x100)
		return 0xFFFF;
	diffx *= diffx;
	diffy *= diffy;
	return diffx + diffy;
}

bool Scumm::getSideOfLine(int x1,int y1, int x2, int y2, int x, int y, int box) {
	return (x-x1)*(y2-y1) <= (y-y1)*(x2-x1);
}

Point Scumm::closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y) {
	int lydiff,lxdiff;
	int32 dist,a,b,c;
	int x2,y2;
	Point pt;

	if (llx==ulx) {
		x2 = ulx;
		y2 = y;
	} else if (lly==uly) {
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
			if (x2 > ulx) goto type1;
			if (x2 < llx) goto type2;
		}
	} else {
		if (lydiff > 0) {
			if (y2 < uly) goto type1;
			if (y2 > lly) goto type2;
		} else {
			if (y2 > uly) goto type1;
			if (y2 < lly) goto type2;
		}
	}

	pt.x = x2;
	pt.y = y2;
	return pt;
}

bool Scumm::inBoxQuickReject(int b, int x, int y, int threshold) {
	int t;

	getBoxCoordinates(b);

	if (threshold==0)
		return 1;
	
	t = x - threshold;
	if (t > box.upperLeftX && t > box.upperRightX &&
		t > box.lowerLeftX && t > box.lowerRightX)
			return 0;
	
	t = x + threshold;
	if (t < box.upperLeftX && t < box.upperRightX &&
		t < box.lowerLeftX && t < box.lowerRightX)
			return 0;
	
	t = y - threshold;
	if (t > box.upperLeftY && t > box.upperRightY &&
		t > box.lowerLeftY && t > box.lowerRightY)
			return 0;
	
	t = y + threshold;
	if (t < box.upperLeftY && t < box.upperRightY &&
			t < box.lowerLeftY && t < box.lowerRightY)
			return 0;

	return 1;
}

AdjustBoxResult Scumm::getClosestPtOnBox(int b, int x, int y) {
	Point pt;
	AdjustBoxResult best;
	uint dist;
	uint bestdist = (uint)0xFFFF;

	getBoxCoordinates(b);

	pt = closestPtOnLine(box.upperLeftX,box.upperLeftY,box.upperRightX,box.upperRightY,x,y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.upperRightX,box.upperRightY,box.lowerLeftX,box.lowerLeftY,x,y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.lowerLeftX,box.lowerLeftY,box.lowerRightX,box.lowerRightY,x,y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}

	pt = closestPtOnLine(box.lowerRightX,box.lowerRightY,box.upperLeftX,box.upperLeftY,x,y);
	dist = distanceFromPt(x, y, pt.x, pt.y);
	if (dist < bestdist) {
		bestdist = dist;
		best.x = pt.x;
		best.y = pt.y;
	}
	
	best.dist = bestdist;
	return best;
}

byte *Scumm::getBoxMatrixBaseAddr() {
	byte *ptr = getResourceAddress(rtMatrix, 1) + 8;
	if (*ptr==0xFF) ptr++;
	return ptr;
}

int Scumm::getPathToDestBox(int from, int to) {
	byte *boxm;
	int i;

	if (from==to)
		return to;
	
	boxm = getBoxMatrixBaseAddr();

	i=0;
	while (i != from) {
		while (*boxm != 0xFF)
			boxm += 3;
		i++;
		boxm++;
	}

	while (boxm[0]!=0xFF) {
		if (boxm[0] <= to && boxm[1]>=to)
			return boxm[2];
		boxm+=3;
	}
	return 0;
}

int Scumm::findPathTowards(Actor *a, int box1, int box2, int box3) {
	int upperLeftX, upperLeftY;
	int upperRightX, upperRightY;
	int lowerLeftX, lowerLeftY;
	int lowerRightX, lowerRightY;
	int i,j,m,n,p,q,r;
	int tmp_x, tmp_y;
	int tmp;
	
	getBoxCoordinates(box1);
	upperLeftX = box.upperLeftX;
	upperLeftY = box.upperLeftY;
	upperRightX = box.upperRightX;
	upperRightY = box.upperRightY;
	lowerLeftX = box.lowerLeftX;
	lowerLeftY = box.lowerLeftY;
	lowerRightX = box.lowerRightX;
	lowerRightY = box.lowerRightY;
	getBoxCoordinates(box2);

	i = 0;
	do {
		if (i >= 4) goto ExitPos;
		for (j=0; j<4; j++) {
			if (upperRightX==upperLeftX &&
					box.upperLeftX==upperLeftX &&
					box.upperRightX==upperRightX) {
				
ExitPos:;
				n = m = 0;
				if (upperRightY < upperLeftY) {
					m = 1;
					SWAP(upperRightY, upperLeftY);
				}
				if (box.upperRightY < box.upperLeftY) {
					n = 1;
					SWAP(box.upperRightY, box.upperLeftY);
				}
				if (box.upperRightY >= upperLeftY &&
						box.upperLeftY <= upperRightY &&
						(box.upperLeftY != upperRightY &&
						 box.upperRightY!= upperLeftY ||
						 upperRightY==upperLeftY ||
						 box.upperRightY==box.upperLeftY)) {
					if (box2==box3) {
						m = a->walkdata.destx - a->x;
						p = a->walkdata.desty - a->y;
						tmp = upperLeftX - a->x;
						i = a->y;
						if (m) {
							q = tmp * p;
							r = q/m;
							if (r==0 && (q<=0 || m<=0) && (q>=0 || m>=0)) {
								r = -1;
							}
							i += r;
						}
					} else {
						i = a->y;
					}
					q = i;
					if (q < box.upperLeftY)
						q = box.upperLeftY;
					if (q > box.upperRightY)
						q = box.upperRightY;
					if (q < upperLeftY)
						q = upperLeftY;
					if (q > upperRightY)
						q = upperRightY;
					if (q==i && box2==box3)
						return 1;
					_foundPathX = upperLeftX;
					_foundPathY = q;
					return 0;
				} else {
					if (m) {
						SWAP(upperRightY, upperLeftY);
					}
					if (n) {
						SWAP(box.upperRightY, box.upperLeftY);
					}
				}
			}
			if (upperLeftY==upperRightY &&
					box.upperLeftY==upperLeftY &&
					box.upperRightY==upperRightY) {
				n = m = 0;
				if(upperRightX < upperLeftX) {
					m = 1;
					SWAP(upperRightX, upperLeftX);
				}
				if (box.upperRightX < box.upperLeftX) {
					n = 1;
					SWAP(box.upperRightX, box.upperLeftX);
				}
				if (box.upperRightX >= upperLeftX &&
						box.upperLeftX <= upperRightX &&
						(box.upperLeftX != upperRightX &&
						 box.upperRightX!= upperLeftX ||
						 upperRightX==upperLeftX ||
						 box.upperRightX==box.upperLeftX)) {
					if (box2==box3) {
						m = a->walkdata.destx - a->x;
						p = a->walkdata.desty - a->y;
						i = upperLeftY - a->y;
						tmp = a->x;
						if (p) {
							tmp += i * m / p;
						}
					} else {
						tmp = a->x;
					}
					q = tmp;
					if (q < box.upperLeftX)
						q = box.upperLeftX;
					if (q > box.upperRightX)
						q = box.upperRightX;
					if (q < upperLeftX)
						q = upperLeftX;
					if (q > upperRightX)
						q = upperRightX;
					if (tmp==q && box2==box3)
						return 1;
					_foundPathX = q;
					_foundPathY = upperLeftY;
					return 0;
				} else {
					if (m != 0) {
						SWAP(upperRightX, upperLeftX);
					}
					if (n != 0) {
						SWAP(box.upperRightX, box.upperLeftX);
					}
				}
			}
			tmp_x = upperLeftX;
			tmp_y = upperLeftY;
			upperLeftX = upperRightX;
			upperLeftY = upperRightY;
			upperRightX = lowerLeftX;
			upperRightY = lowerLeftY;
			lowerLeftX = lowerRightX;
			lowerLeftY = lowerRightY;
			lowerRightX = tmp_x;
			lowerRightY = tmp_y;
		}

		tmp_x = box.upperLeftX;
		tmp_y = box.upperLeftY;
		box.upperLeftX = box.upperRightX;
		box.upperLeftY = box.upperRightY;
		box.upperRightX = box.lowerLeftX;
		box.upperRightY = box.lowerLeftY;
		box.lowerLeftX = box.lowerRightX;
		box.lowerLeftY = box.lowerRightY;
		box.lowerRightX = tmp_x;
		box.lowerRightY = tmp_y;
		i++;		
	} while (1);
}


void Scumm::setBoxFlags(int box, int val) {
	Box *b = getBoxBaseAddr(box);
	b->flags = val;
}

void Scumm::setBoxScale(int box, int scale) {
	Box *b = getBoxBaseAddr(box);
	b->scale = scale;
}

#define BOX_MATRIX_SIZE 2000

void Scumm::createBoxMatrix() {
	byte *matrix_ptr;
	int num,i,j;
	byte flags;
	int table_1[66],table_2[66];
	int counter,val;
	int code;

	PathVertex *vtx;
	PathNode *node, *node2;

	_maxBoxVertexHeap = 1000;

	createResource(rtMatrix, 4, 1000);
	createResource(rtMatrix, 3, 4160); //65 items of something of size 64
	createResource(rtMatrix, 1, BOX_MATRIX_SIZE+8);

	matrix_ptr = getResourceAddress(rtMatrix, 1);

	/* endian & alignment safe */
	((uint32*)matrix_ptr)[1] = TO_BE_32(BOX_MATRIX_SIZE+8);
	((uint32*)matrix_ptr)[0] = MKID('BOXM');

	_boxMatrixPtr4 = getResourceAddress(rtMatrix, 4);
	_boxMatrixPtr1 = getResourceAddress(rtMatrix, 1) + 8;
	_boxMatrixPtr3 = getResourceAddress(rtMatrix, 3);

	_boxPathVertexHeapIndex = _boxMatrixItem = 0;
	
	num = getNumBoxes();

	for (i=0; i<num; i++) {
		for (j=0; j<num; j++) {
			if (i==j) {
				_boxMatrixPtr3[i*64+j] = 0;	
			} else if (areBoxesNeighbours(i, j)) {
				_boxMatrixPtr3[i*64+j] = 1;
			} else {
				_boxMatrixPtr3[i*64+j] = 250;
			}
		}
	}

	for (j=0; j<num; j++) {
		flags = getBoxFlags(j);
		if (flags & 0x80) {
			addToBoxMatrix(0xFF);
			addToBoxMatrix(j);
			addToBoxMatrix(j);
			addToBoxMatrix(j);
		} else {
			vtx = addPathVertex();
			for (i=0; i<num; i++) {
				flags = getBoxFlags(j);
				if (!(flags&0x80)) {
					node = unkMatrixProc2(vtx, i);
					if (i==j)
						node2 = node;
				}
			}
			table_1[j] = 0;
			table_2[j] = j;
			vtx = unkMatrixProc1(vtx, node2);
			node = vtx ? vtx->left : NULL;

			counter = 250;
			while (node) {
				val = _boxMatrixPtr3[j*64 + node->index];
				table_1[node->index] = val;
				if (val<counter) counter=val;
				
				if (table_1[node->index]!=250)
					table_2[node->index] = node->index;
				else
					table_2[node->index] = -1;

				node = node->left;
			}
			
			while (vtx) {
				counter = 250;
				node2 = node = vtx->left;

				while (node) {
					if ( table_1[node->index] < counter ) {
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
			for (i=1; i<num;) {
				if (table_2[i-1]!=-1) {
					addToBoxMatrix(i-1); /* lo */
					if (table_2[i-1] != table_2[i]) {
						addToBoxMatrix(i-1); /* hi */
						addToBoxMatrix(table_2[i-1]); /* dst */
					} else {
						while (table_2[i-1] == table_2[i]) {
							if (++i==num)
								break;
						}
						addToBoxMatrix(i-1); /* hi */
						addToBoxMatrix(table_2[i-1]); /* dst */
					}
				}
				if (++i==num && table_2[i-1]!=-1) {
					addToBoxMatrix(i-1); /* lo */
					addToBoxMatrix(i-1); /* hi */
					addToBoxMatrix(table_2[i-1]); /* dest */
				}
			}
		}
	}

	addToBoxMatrix(0xFF);
	nukeResource(rtMatrix, 4);
	nukeResource(rtMatrix, 3);
}

PathVertex *Scumm::unkMatrixProc1(PathVertex *vtx, PathNode *node) {
	if (node==NULL || vtx==NULL)
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

PathNode *Scumm::unkMatrixProc2(PathVertex *vtx, int i) {
	PathNode *node;

	if (vtx==NULL)
		return NULL;

	if (!vtx->right) {
		node = (PathNode*)addToBoxVertexHeap(sizeof(PathNode));
		vtx->left = vtx->right = node;

		node->index = i;
		node->left = 0;
		node->right = 0;
	} else {
		node = (PathNode*)addToBoxVertexHeap(sizeof(PathNode));	
		vtx->right->left = node;
		
		node->right = vtx->right;
		node->index = i;
		node->left = 0;

		vtx->right = node;
	}

	return vtx->right;
}

/* Check if two boxes are neighbours */
bool Scumm::areBoxesNeighbours(int box1, int box2) {
	int upperLeftX, upperLeftY;
	int upperRightX, upperRightY;
	int lowerLeftX, lowerLeftY;
	int lowerRightX, lowerRightY;
	int j,k,m,n;
	int tmp_x, tmp_y;
	bool result;

	if (getBoxFlags(box1)&0x80 || getBoxFlags(box2)&0x80)
		return false;

	getBoxCoordinates(box1);

	upperLeftX = box.upperLeftX;
	upperLeftY = box.upperLeftY;
	upperRightX = box.upperRightX;
	upperRightY = box.upperRightY;
	lowerLeftX = box.lowerLeftX;
	lowerLeftY = box.lowerLeftY;
	lowerRightX = box.lowerRightX;
	lowerRightY = box.lowerRightY;

	getBoxCoordinates(box2);
	
	result = false;
	j = 4;
	
	do {
		k = 4;
		do {
			if (upperRightX == upperLeftX &&
				  box.upperLeftX == upperLeftX &&
					box.upperRightX == upperRightX) {
				n = m = 0;
				if (upperRightY < upperLeftY) {
					n = 1;
					SWAP(upperRightY, upperLeftY);
				}
				if (box.upperRightY < box.upperLeftY) {
					m = 1;
					SWAP(box.upperRightY, box.upperLeftY);
				}
				if (box.upperRightY < upperLeftY ||
					  box.upperLeftY > upperRightY ||
						(box.upperLeftY == upperRightY ||
						 box.upperRightY==upperLeftY) &&
						 upperRightY != upperLeftY &&
						 box.upperLeftY!=box.upperRightY) {
					if (n) {
						SWAP(upperRightY, upperLeftY);
					}
					if (m) {
						SWAP(box.upperRightY, box.upperLeftY);
					}
				} else {
					if (n) {
						SWAP(upperRightY, upperLeftY);
					}
					if (m) {
						SWAP(box.upperRightY, box.upperLeftY);
					}
					result = true;
				}	
			}

			if (upperRightY == upperLeftY && 
					box.upperLeftY == upperLeftY &&
					box.upperRightY == upperRightY) {
				n = m = 0;
				if (upperRightX < upperLeftX) {
					n = 1;
					SWAP(upperRightX, upperLeftX);
				}
				if (box.upperRightX < box.upperLeftX) {
					m = 1;
					SWAP(box.upperRightX, box.upperLeftX);
				}
				if (box.upperRightX < upperLeftX ||
					  box.upperLeftX > upperRightX ||
						(box.upperLeftX == upperRightX ||
						 box.upperRightX==upperLeftX) &&
						 upperRightX != upperLeftX &&
						 box.upperLeftX!=box.upperRightX) {

					if (n) {
						SWAP(upperRightX, upperLeftX);
					}
					if (m) {
						SWAP(box.upperRightX, box.upperLeftX);
					}
				} else {
					if (n) {
						SWAP(upperRightX, upperLeftX);
					}
					if (m) {
						SWAP(box.upperRightX, box.upperLeftX);
					}
					result = true;
				}
			}
			
			tmp_x = upperLeftX;
			tmp_y = upperLeftY;
			upperLeftX = upperRightX;
			upperLeftY = upperRightY;
			upperRightX = lowerLeftX;
			upperRightY = lowerLeftY;
			lowerLeftX = lowerRightX;
			lowerLeftY = lowerRightY;
			lowerRightX = tmp_x;
			lowerRightY = tmp_y;
		} while (--k);
		
		tmp_x = box.upperLeftX;
		tmp_y = box.upperLeftY;
		box.upperLeftX = box.upperRightX;
		box.upperLeftY = box.upperRightY;
		box.upperRightX = box.lowerLeftX;
		box.upperRightY = box.lowerLeftY;
		box.lowerLeftX = box.lowerRightX;
		box.lowerLeftY = box.lowerRightY;
		box.lowerRightX = tmp_x;
		box.lowerRightY = tmp_y;
	} while (--j);

	return result;
}

void Scumm::addToBoxMatrix(byte b) {
	if (++_boxMatrixItem > BOX_MATRIX_SIZE)
		error("Box matrix overflow");
	*_boxMatrixPtr1++ = b;
}

void *Scumm::addToBoxVertexHeap(int size) {
	byte *ptr = _boxMatrixPtr4;

	_boxMatrixPtr4 += size;
	_boxPathVertexHeapIndex += size;

	if (_boxPathVertexHeapIndex >= _maxBoxVertexHeap)
		error("Box path vertex heap overflow");

	return ptr;
}

PathVertex *Scumm::addPathVertex() {
	_boxMatrixPtr4 = getResourceAddress(rtMatrix, 4);
	_boxPathVertexHeapIndex = 0;
	return (PathVertex*)addToBoxVertexHeap(sizeof(PathVertex));
}
