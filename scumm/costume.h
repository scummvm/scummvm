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
	int _id;
	const byte *_baseptr;
	const byte *_animCmds;
	const byte *_dataOffsets;
	const byte *_palette;
	const byte *_frameOffsets;
	byte _numColors;
	byte _numAnim;
	byte _format;
	bool _mirror;

	LoadedCostume(Scumm *vm) :
		_vm(vm), _id(-1), _baseptr(0), _animCmds(0), _dataOffsets(0), _palette(0),
		_frameOffsets(0), _numColors(0), _numAnim(0), _format(0), _mirror(false) {}

	void loadCostume(int id);
	byte increaseAnims(Actor *a);

protected:
	byte increaseAnim(Actor *a, int slot);
};


class CostumeRenderer : public BaseCostumeRenderer {
protected:
	LoadedCostume _loaded;
	
	byte _scaleIndexX;						/* must wrap at 256 */
	byte _scaleIndexY;
	byte _palette[32];

public:
	CostumeRenderer(Scumm *vm) : BaseCostumeRenderer(vm), _loaded(vm) {}

	void setPalette(byte *palette);
	void setFacing(Actor *a);
	void setCostume(int costume);

protected:
	byte drawLimb(const CostumeData &cost, int limb);

	void proc3();
	void proc3_ami();

	void procC64(int actor);
	void c64_ignorePakCols(int num);

	byte mainRoutine(int xmoveCur, int ymoveCur);
};

#endif
