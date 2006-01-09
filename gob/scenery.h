/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
#ifndef GOB_SCENERY_H
#define GOB_SCENERY_H

namespace Gob {

class Scenery {
public:
#pragma START_PACK_STRUCTS
	typedef struct PieceDesc {
		int16 left;		//NOTE:
		int16 right;		//These are stored in Little Endian format
		int16 top;		//And should be converted by client code when accessed
		int16 bottom;		//i.e. use FROM_LE_16()
	} GCC_PACK PieceDesc;

	typedef struct StaticPlane {
		int8 pictIndex;
		int8 pieceIndex;
		int8 drawOrder;
		int16 destX;
		int16 destY;
		int8 transp;
	} GCC_PACK StaticPlane;

	typedef struct StaticLayer {
		int16 backResId;
		int16 planeCount;
		StaticPlane planes[1];
	} GCC_PACK StaticLayer;

	// Animations

	typedef struct AnimFramePiece {
		byte pictIndex;
		byte pieceIndex;
		int8 destX;
		int8 destY;
		int8 notFinal;
	} GCC_PACK AnimFramePiece;

	typedef struct AnimLayer {
		int16 unknown0;
		int16 posX;
		int16 posY;
		int16 animDeltaX;
		int16 animDeltaY;
		int8 transp;
		int16 framesCount;
		AnimFramePiece frames[1];
	} GCC_PACK AnimLayer;
#pragma END_PACK_STRUCTS

	typedef struct Static {
		int16 layersCount;
		StaticLayer **layers;
		PieceDesc **pieces;
		int8 *piecesFromExt;
		char *dataPtr;
		Static() : layersCount(0), layers(0), pieces(0),
				   piecesFromExt(0), dataPtr(0) {}
	} Static;

	struct Animation {
		int16 layersCount;
		AnimLayer **layers;
		PieceDesc **pieces;
		int8 *piecesFromExt;
		char *dataPtr;
		Animation() : layersCount(0), layers(0), pieces(0),
			              piecesFromExt(0), dataPtr(0) {}
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
	char _animFromExt[10];
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

	int16 *_pCaptureCounter;

	// Functions

	int16 loadStatic(char search);
	void freeStatic(int16 index);
	void renderStatic(int16 scenery, int16 layer);
	void interRenderStatic(void);
	void interLoadCurLayer(void);
	void updateStatic(int16 orderFrom);
	int16 loadAnim(char search);
	void updateAnim(int16 layer, int16 frame, int16 animation, int16 flags,
					int16 drawDeltaX, int16 drawDeltaY, char doDraw);
	void freeAnim(int16 animation);
	void interUpdateAnim(void);
	void interStoreParams(void);

	Scenery(GobEngine *vm);

protected:
	GobEngine *_vm;
};

}				// End of namespace Gob

#endif	/* __SCENERY_H */
