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

#ifndef SCUMM_BASE_COSTUME_H
#define SCUMM_BASE_COSTUME_H

#include "common/scummsys.h"
#include "scumm/actor.h"		// for CostumeData

namespace Scumm {

#include "common/pack-start.h"	// START STRUCT PACKING

struct CostumeInfo {
	uint16 width, height;
	int16 relX, relY;
	int16 moveX, moveY;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


extern const byte smallCostumeScaleTable[256];
extern const byte bigCostumeScaleTable[768];


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
	virtual bool increaseAnims(Actor *a) = 0;
	virtual void costumeDecodeData(Actor *a, int frame, uint usemask) = 0;

	virtual bool hasManyDirections(int id) { return false; }
};


/**
 * Base class for both ClassicCostumeRenderer and AkosRenderer.
 */
class BaseCostumeRenderer {
public:
	Common::Rect _clipOverride;
	byte _actorID;

	byte _shadowMode;
	byte *_shadowTable;

	int _actorX, _actorY;
	byte _zbuf;
	byte _scaleX, _scaleY;

	int _drawTop, _drawBottom;
	byte _paletteNum;
	bool _skipLimbs;
	bool _actorDrawVirScr;


protected:
	ScummEngine *_vm;

	// Destination
	Graphics::Surface _out;
	int32 _numStrips;

	// Source pointer
	const byte *_srcPtr;

	// current move offset
	int _xMove, _yMove;

	// whether to draw the actor mirrored
	bool _mirror;

	int _numBlocks;

	// width and height of cel to decode
	int _width, _height;

public:
	struct ByleRLEData {
		// Parameters for the original ("V1") costume codec.
		// These ones are accessed from ARM code. Don't reorder.
		int x;
		int y;
		const byte *scaleTable;
		int height;
		int width;
		int skipWidth;
		byte *destPtr;
		const byte *maskPtr;
		int scaleXStep;
		byte mask, shr;
		byte repColor;
		byte repLen;
		// These ones aren't accessed from ARM code.
		Common::Rect boundsRect;
		int scaleXIndex, scaleYIndex;
	};

	BaseCostumeRenderer(ScummEngine *scumm) {
		_actorID = 0;
		_shadowMode = 0;
		_shadowTable = nullptr;
		_actorX = _actorY = 0;
		_zbuf = 0;
		_scaleX = _scaleY = 0;
		_drawTop = _drawBottom = 0;

		_vm = scumm;
		_numStrips = -1;
		_srcPtr = nullptr;
		_xMove = _yMove = 0;
		_mirror = false;
		_width = _height = 0;
		_skipLimbs = false;
		_paletteNum = 0;
		_actorDrawVirScr = false;
		_numBlocks = 0;
	}
	virtual ~BaseCostumeRenderer() {}

	virtual void setPalette(uint16 *palette) = 0;
	virtual void setFacing(const Actor *a) = 0;
	virtual void setCostume(int costume, int shadow) = 0;


	byte drawCostume(const VirtScreen &vs, int numStrips, const Actor *a, bool drawToBackBuf);

protected:
	virtual byte drawLimb(const Actor *a, int limb) = 0;

	void skipCelLines(ByleRLEData &compData, int num);
};

} // End of namespace Scumm

#endif
