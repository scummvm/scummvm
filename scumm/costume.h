/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COSTUME_H
#define COSTUME_H

#include "base-costume.h"

class LoadedCostume {
protected:
	Scumm *_vm;

public:
	byte *_ptr;
	byte *_baseptr;
	byte *_dataptr;
	byte _numColors;

	LoadedCostume(Scumm *vm) : _vm(vm), _ptr(0), _dataptr(0), _numColors(0) {}

	void loadCostume(int id);
	byte increaseAnims(Actor *a);

protected:
	byte increaseAnim(Actor *a, int slot);
};

class CostumeRenderer : public BaseCostumeRenderer {
public:
	uint _outheight;

protected:
	LoadedCostume _loaded;
	
	byte *_frameptr;
	byte *_srcptr;
	byte *_bgbak_ptr, *_backbuff_ptr, *_mask_ptr, *_mask_ptr_dest;
	byte _maskval;
	byte _shrval;
	byte _width2;
	byte _height2;
	int _xpos, _ypos;

	int _scaleIndexXStep;
	int _scaleIndexYStep;
	byte _scaleIndexX;						/* must wrap at 256 */
	byte _scaleIndexY, _scaleIndexYTop;
	int _left, _right;
	int _dir2;
	int _top, _bottom;
	int _ypostop;
	int _ypitch;
	byte _docontinue;
	int _imgbufoffs;
	byte _repcolor;
	byte _replen;
	byte _palette[32];

public:
	CostumeRenderer(Scumm *vm) : BaseCostumeRenderer(vm), _loaded(vm) {}

	void setPalette(byte *palette);
	void setFacing(Actor *a);
	void setCostume(int costume);

protected:
	byte drawLimb(const CostumeData &cost, int limb);

	void proc6();
	void proc5();
	void proc4();
	void proc3();
	void proc2();
	void proc1();
	void proc6_ami();
	void proc5_ami();
	void proc4_ami();
	void proc3_ami();
	void proc2_ami();
	void proc1_ami();
	void proc_special(byte mask);
	byte mainRoutine(int limb, int frame);
	void ignorePakCols(int num);
};

#endif
