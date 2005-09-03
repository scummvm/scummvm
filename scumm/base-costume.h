/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#ifndef BASE_COSTUME_H
#define BASE_COSTUME_H

#include "common/scummsys.h"
#include "scumm/actor.h"		// for CostumeData

namespace Scumm {

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct CostumeInfo {
	uint16 width, height;
	int16 rel_x, rel_y;
	int16 move_x, move_y;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif



#ifdef PALMOS_68K
extern const byte *smallCostumeScaleTable;
extern const byte *bigCostumeScaleTable;
#else
extern const byte smallCostumeScaleTable[256];
extern const byte bigCostumeScaleTable[768];
#endif



class Actor;
class ScummEngine;
struct VirtScreen;

class BaseCostumeLoader {
protected:
	ScummEngine *_vm;

public:
	BaseCostumeLoader(ScummEngine *vm) : _vm(vm) {}
	virtual ~BaseCostumeLoader() {}

	virtual void loadCostume(int id) = 0;
	virtual byte increaseAnims(Actor *a) = 0;
	virtual void costumeDecodeData(Actor *a, int frame, uint usemask) = 0;

	bool hasManyDirections(int id) { return false; }
};


/**
 * Base class for both ClassicCostumeRenderer and AkosRenderer.
 */
class BaseCostumeRenderer {
public:
	Common::Rect _clipOverride;
	byte _actorID;

	byte _shadow_mode;
	byte *_shadow_table;

	int _actorX, _actorY;
	byte _zbuf;
	byte _scaleX, _scaleY;

	int _draw_top, _draw_bottom;
	byte _paletteNum;
	bool _skipLimbs;
	bool _actorDrawVirScr;


protected:
	ScummEngine *_vm;

	// Destination
	Graphics::Surface _out;
	int32 _numStrips;

	// Source pointer
	const byte *_srcptr;

	// current move offset
	int _xmove, _ymove;

	// whether to draw the actor mirrored
	bool _mirror;

	// width and height of cel to decode
	int _width, _height;

	struct Codec1 {
		// Parameters for the original ("V1") costume codec.
		const byte *scaletable;
		byte mask, shr;
		byte repcolor;
		byte replen;
		int scaleXstep;
		int x, y;
		int scaleXindex, scaleYindex;
		int skip_width;
		byte *destptr;
		const byte *mask_ptr;
	};

public:
	BaseCostumeRenderer(ScummEngine *scumm) {
		_actorID = 0;
		_shadow_mode = 0;
		_shadow_table = 0;
		_actorX = _actorY = 0;
		_zbuf = 0;
		_scaleX = _scaleY = 0;
		_draw_top = _draw_bottom = 0;

		_vm = scumm;
		_numStrips = -1;
		_srcptr = 0;
		_xmove = _ymove = 0;
		_mirror = false;
		_width = _height = 0;
		_skipLimbs = 0;
		_paletteNum = 0;
	}
	virtual ~BaseCostumeRenderer() {}

	virtual void setPalette(byte *palette) = 0;
	virtual void setFacing(const Actor *a) = 0;
	virtual void setCostume(int costume) = 0;


	byte drawCostume(const VirtScreen &vs, int numStrips, const Actor *a, bool drawToBackBuf);

protected:
	virtual byte drawLimb(const Actor *a, int limb) = 0;

	void codec1_ignorePakCols(Codec1 &v1, int num);
};

} // End of namespace Scumm

#endif
