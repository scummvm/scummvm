/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __SCENERY_H
#define __SCENERY_H

namespace Gob {

#pragma START_PACK_STRUCTS
typedef struct Scen_PieceDesc {
	int16 left;
	int16 right;
	int16 top;
	int16 bottom;
} GCC_PACK Scen_PieceDesc;

typedef struct Scen_StaticPlane {
	char pictIndex;
	char pieceIndex;
	char drawOrder;
	int16 destX;
	int16 destY;
	char transp;
} GCC_PACK Scen_StaticPlane;

typedef struct Scen_StaticLayer {
	int16 backResId;
	int16 planeCount;
	Scen_StaticPlane planes[1];
} GCC_PACK Scen_StaticLayer;

typedef struct Scen_Static {
	int16 layersCount;
	Scen_StaticLayer **layers;
	Scen_PieceDesc **pieces;
	void *unknown;
	char *dataPtr;
} GCC_PACK Scen_Static;

// Animations

typedef struct Scen_AnimFramePiece {
	byte pictIndex;
	byte pieceIndex;
	char destX;
	char destY;
	char notFinal;
} GCC_PACK Scen_AnimFramePiece;

typedef struct Scen_AnimLayer {
	int16 unknown0;
	int16 posX;
	int16 posY;
	int16 animDeltaX;
	int16 animDeltaY;
	char transp;
	int16 framesCount;
	Scen_AnimFramePiece frames[1];
} GCC_PACK Scen_AnimLayer;
#pragma END_PACK_STRUCTS

struct Scen_Animation {
	int16 layersCount;
	Scen_AnimLayer **layers;
	Scen_PieceDesc **pieces;
	void *unknowm;
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
