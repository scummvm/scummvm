/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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


class LoadedCostume {
protected:
	Scumm *_vm;

public:
	byte *_ptr;
	byte *_dataptr;
	byte _numColors;

	LoadedCostume(Scumm *vm) : _vm(vm), _ptr(0), _dataptr(0), _numColors(0) {}

	void loadCostume(int id);
	byte increaseAnims(Actor *a);

protected:
	byte increaseAnim(Actor *a, int slot);
};


class CostumeRenderer {
protected:
	Scumm *_vm;
	
	LoadedCostume _loaded;
	
public:
	byte *_shadow_table;

	byte *_frameptr;
	byte *_srcptr;
	byte *_bgbak_ptr, *_backbuff_ptr, *_mask_ptr, *_mask_ptr_dest;
	int _actorX, _actorY;
	byte _zbuf;
	uint _scaleX, _scaleY;
	int _xmove, _ymove;
	bool _mirror;
	byte _maskval;
	byte _shrval;
	byte _width2;
	int _width;
	byte _height2;
	int _height;
	int _xpos, _ypos;

	uint _outheight;
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
	byte _transEffect[0x100];

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
	void proc_special(Actor *a, byte mask);
	byte mainRoutine(Actor *a, int slot, int frame);
	void ignorePakCols(int num);

	byte drawOneSlot(Actor *a, int slot);
	byte drawCostume(Actor *a);

	void setPalette(byte *palette);
	void setFacing(uint16 facing);
	void setCostume(int costume);

public:
	CostumeRenderer(Scumm *vm) : _vm(vm), _loaded(vm) {}
};

#endif
