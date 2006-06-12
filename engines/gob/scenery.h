/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_SCENERY_H
#define GOB_SCENERY_H

namespace Gob {

class Scenery {
public:
#pragma START_PACK_STRUCTS
	struct PieceDesc {
		int16 left;		//NOTE:
		int16 right;		//These are stored in Little Endian format
		int16 top;		//And should be converted by client code when accessed
		int16 bottom;		//i.e. use FROM_LE_16()
	} GCC_PACK;

	struct StaticPlane {
		int8 pictIndex;
		int8 pieceIndex;
		int8 drawOrder;
		int16 destX;
		int16 destY;
		int8 transp;
	} GCC_PACK;

	struct StaticLayer {
		int16 backResId;
		int16 planeCount;
		StaticPlane planes[1];
	} GCC_PACK;

	// Animations

	struct AnimFramePiece {
		byte pictIndex;
		byte pieceIndex;
		int8 destX;
		int8 destY;
		int8 notFinal;
	} GCC_PACK;

	struct AnimLayer {
		int16 unknown0;
		int16 posX;
		int16 posY;
		int16 animDeltaX;
		int16 animDeltaY;
		int8 transp;
		int16 framesCount;
		AnimFramePiece *frames;
	} GCC_PACK;
#pragma END_PACK_STRUCTS

	struct Static {
		int16 layersCount;
		StaticLayer **layers;
		PieceDesc **pieces;
		int8 *piecesFromExt;
		char *dataPtr;
		Static() : layersCount(0), layers(0), pieces(0),
				   piecesFromExt(0), dataPtr(0) {}
	};

	struct Animation {
		int16 layersCount;
		AnimLayer *layers;
		PieceDesc **pieces;
		int8 *piecesFromExt;
		Animation() : layersCount(0), layers(0), pieces(0),
			              piecesFromExt(0) {}
	};

	// Global variables

	char _spriteRefs[20];
	int16 _spriteResId[20];

	char _staticPictToSprite[70];
	int16 _staticPictCount[10];
	Static _statics[10];
	char _staticFromExt[10];
	int16 _staticResId[10];

	char _animPictToSprite[70];
	int16 _animPictCount[10];
	Animation _animations[10];
	int16 _animResId[10];

	int16 _curStatic;
	int16 _curStaticLayer;

	int16 _toRedrawLeft;
	int16 _toRedrawRight;
	int16 _toRedrawTop;
	int16 _toRedrawBottom;

	int16 _animTop;
	int16 _animLeft;
	int16 _animBottom;
	int16 _animRight;

	int16 *_pCaptureCounter;

	// Functions

	int16 loadStatic(char search);
	void freeStatic(int16 index);
	void renderStatic(int16 scenery, int16 layer);
	void updateStatic(int16 orderFrom);
	int16 loadAnim(char search);
	void freeAnim(int16 animation);
	void interStoreParams(void);

	virtual void updateAnim(int16 layer, int16 frame, int16 animation,
			int16 flags, int16 drawDeltaX, int16 drawDeltaY, char doDraw) = 0;

	Scenery(GobEngine *vm);
	virtual ~Scenery() {};

protected:
	GobEngine *_vm;
};

class Scenery_v1 : public Scenery {
public:
	virtual void updateAnim(int16 layer, int16 frame, int16 animation,
			int16 flags, int16 drawDeltaX, int16 drawDeltaY, char doDraw);

	Scenery_v1(GobEngine *vm);
	virtual ~Scenery_v1() {};
};

class Scenery_v2 : public Scenery_v1 {
public:
	virtual void updateAnim(int16 layer, int16 frame, int16 animation,
			int16 flags, int16 drawDeltaX, int16 drawDeltaY, char doDraw);

	Scenery_v2(GobEngine *vm);
	virtual ~Scenery_v2() {};
};

}				// End of namespace Gob

#endif	/* __SCENERY_H */
