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

#pragma START_PACK_STRUCTS
typedef struct Scen_PieceDesc {
	int16 left;		//NOTE:
	int16 right;		//These are stored in Little Endian format
	int16 top;		//And should be converted by client code when accessed
	int16 bottom;		//i.e. use FROM_LE_16()
} GCC_PACK Scen_PieceDesc;

typedef struct Scen_StaticPlane {
	int8 pictIndex;
	int8 pieceIndex;
	int8 drawOrder;
	int16 destX;
	int16 destY;
	int8 transp;
} GCC_PACK Scen_StaticPlane;

typedef struct Scen_StaticLayer {
	int16 backResId;
	int16 planeCount;
	Scen_StaticPlane planes[1];
} GCC_PACK Scen_StaticLayer;

// Animations

typedef struct Scen_AnimFramePiece {
	byte pictIndex;
	byte pieceIndex;
	int8 destX;
	int8 destY;
	int8 notFinal;
} GCC_PACK Scen_AnimFramePiece;

typedef struct Scen_AnimLayer {
	int16 unknown0;
	int16 posX;
	int16 posY;
	int16 animDeltaX;
	int16 animDeltaY;
	int8 transp;
	int16 framesCount;
	Scen_AnimFramePiece frames[1];
} GCC_PACK Scen_AnimLayer;
#pragma END_PACK_STRUCTS

typedef struct Scen_Static {
	int16 layersCount;
	Scen_StaticLayer **layers;
	Scen_PieceDesc **pieces;
	int8 *piecesFromExt;
	char *dataPtr;
} Scen_Static;

struct Scen_Animation {
	int16 layersCount;
	Scen_AnimLayer **layers;
	Scen_PieceDesc **pieces;
	int8 *piecesFromExt;
	char *dataPtr;
};

// Global variables

extern char scen_spriteRefs[20];
extern int16 scen_spriteResId[20];

extern char scen_staticPictToSprite[70];
extern int16 scen_staticPictCount[10];
extern Scen_Static scen_statics[10];
extern char scen_staticFromExt[10];
extern int16 scen_staticResId[10];

extern char scen_animPictToSprite[70];
extern int16 scen_animPictCount[10];
extern char scen_animFromExt[10];
extern Scen_Animation scen_animations[10];
extern int16 scen_animResId[10];

extern int16 scen_curStatic;
extern int16 scen_curStaticLayer;

extern int16 scen_toRedrawLeft;
extern int16 scen_toRedrawRight;
extern int16 scen_toRedrawTop;
extern int16 scen_toRedrawBottom;

extern int16 scen_animTop;
extern int16 scen_animLeft;

extern int16 *scen_pCaptureCounter;

// Functions

int16 scen_loadStatic(char search);
void scen_freeStatic(int16 index);
void scen_renderStatic(int16 scenery, int16 layer);
void scen_interRenderStatic(void);
void scen_interLoadCurLayer(void);
void scen_updateStatic(int16 orderFrom);
int16 scen_loadAnim(char search);
void scen_updateAnim(int16 layer, int16 frame, int16 animation, int16 flags,
    int16 drawDeltaX, int16 drawDeltaY, char doDraw);
void scen_freeAnim(int16 animation);
void scen_interUpdateAnim(void);
void scen_interStoreParams(void);

}				// End of namespace Gob

#endif	/* __SCENERY_H */
