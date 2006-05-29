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
#ifndef GOB_GAME_H
#define GOB_GAME_H

#include "gob/sound.h"
#include "gob/video.h"

namespace Gob {

class Game {
public:

#pragma START_PACK_STRUCTS
	struct Collision {
		int16 id;
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		int16 flags;
		int16 key;
		int16 funcEnter;
		int16 funcLeave;
		int16 field_12; // New in GOB2
	} GCC_PACK;

#define szGame_TotResItem (4 + 2 + 2 + 2)
	struct TotResItem {
		int32 offset;	// if > 0, then offset from end of resource table.
						// If < 0, then -offset-1 is index in .IM file table
		int16 size;
		int16 width;
		int16 height;
	} GCC_PACK;

#define szGame_TotResTable (2 + 1)
	struct TotResTable {
		int16 itemsCount;
		byte unknown;
		TotResItem items[1];
	} GCC_PACK;

#define szGame_ExtItem (4 + 2 + 2 + 2)
	struct ExtItem {
		int32 offset;		// offset from the table end
		uint16 size;
		int16 width;		// width&0x7fff - width, width&0x8000 - pack flag
		int16 height;		// not zero
	} GCC_PACK;

#define szGame_ExtTable (2 + 1)
	struct ExtTable {
		int16 itemsCount;
		byte unknown;
		ExtItem* items;
	} GCC_PACK;

#define szGame_TotTextItem (2 + 2)
	struct TotTextItem {
		int16 offset;
		int16 size;
	} GCC_PACK;

#define szGame_TotTextTable (2)
	struct TotTextTable {
		int16 itemsCount;
		TotTextItem items[1];
	} GCC_PACK;

	struct InputDesc {
		int16 fontIndex;
		int16 backColor;
		int16 frontColor;
		char *ptr;
	} GCC_PACK;

	struct ImdCoord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	} GCC_PACK;

	struct Imd {
		int16 fileHandle;
		int16 verMin;
		int16 framesCount;
		int16 x;
		int16 y;
		int16 width;
		int16 height;
		int16 field_E;
		int16 curFrame;
		Video::Color *palette;
		Video::SurfaceDesc *surfDesc;
		int32 *framesPos;
		int32 firstFramePos;
		int16 stdX;
		int16 stdY;
		int16 stdWidth;
		int16 stdHeight;
		int32 filePos;
		ImdCoord *frameCoords;
		int32 frameDataSize;
		int32 vidBufferSize;
	} GCC_PACK;
#pragma END_PACK_STRUCTS

	TotResTable *_totResourceTable;
	Collision *_collisionAreas;
	Collision *_collStack[5];

	TotTextTable *_totTextData;

	char _curTotFile[14];
	char _curExtFile[14];

	char *_imFileData;
	char *_totFileData;

	int16 _extHandle;

	Snd::SoundDesc *_soundSamples[60];
	int16 _soundIds[60];
	int8 _soundTypes[60];

	char _totToLoad[20];

	int32 _startTimeKey;
	int16 _mouseButtons;

	char _soundFromExt[20];

	// For totSub()
	int8 _backupedCount;
	int8 _curBackupPos;
	int16 _cursorXDeltaArray[5];
	int16 _cursorYDeltaArray[5];
	TotTextTable *_totTextDataArray[5];
	char *_totFileDataArray[5];
	TotResTable *_totResourceTableArray[5];
	ExtTable *_extTableArray[5];
	int16 _extHandleArray[5];
	char *_imFileDataArray[5];
	char *_variablesArray[5];
	char _curTotFileArray[5][14];

	Imd *_imdFile;
	char _curImdFile[15];
	int16 _imdX;
	int16 _imdY;
	int16 _imdFrameDataSize;
	int16 _imdVidBufferSize;
	byte *_imdFrameData;
	byte *_imdVidBuffer;
	int8 _byte_2FC82;
	int8 _byte_2FC83;
	byte *_word_2FC80;

	Game(GobEngine *vm);
	virtual ~Game() {};

	char *loadExtData(int16 dataId, int16 *pResWidth, int16 *pResHeight);
	char *loadTotResource(int16 id);

	void capturePush(int16 left, int16 top, int16 width, int16 height);

	void capturePop(char doDraw);
	void freeSoundSlot(int16 slot);
	void freeCollision(int16 id);

	void loadSound(int16 slot, char *dataPtr);
	int16 inputArea(int16 xPos, int16 yPos, int16 width, int16 height, int16 backColor,
			int16 frontColor, char *str, int16 fontIndex, char inpType, int16 *pTotTime);
	int16 multiEdit(int16 time, int16 index, int16 *pCurPos,
					InputDesc * inpDesc);
	int16 adjustKey(int16 key);
	void collisionsBlock(void);
	void loadTotFile(char *path);
	void loadExtTable(void);
	void loadImFile(void);
	void start(void);
	void totSub(int8 flags, char *newTotFile);
	void switchTotSub(int16 index, int16 skipPlay);
	char *loadLocTexts(void);
	Snd::SoundDesc *loadSND(const char *path, int8 arg_4);

	Imd *loadImdFile(const char *path, Video::SurfaceDesc *surfDesc, int8 flags);
	int8 openImd(const char *path, int16 x, int16 y, int16 repeat, int16 flags);
	void closeImd(void);
	void finishImd(Imd *imdPtr);
	void setImdXY(Imd *imdPtr, int16 x, int16 y);
	void playImd(int16 arg_0, int16 arg_2, int16 arg_4, int16 arg_6, int16 arg_8, int16 arg_A);
	int16 viewImd(Game::Imd *imdPtr, int16 arg_4);
	void imdDrawFrame(Imd *imdPtr, int16 arg_4, int16 arg_6, int16 arg_8);
	void imdRenderFrame(Imd *imdPtr);
	void imdFrameUncompressor(byte *dest, byte *src);
	int16 sub_2C825(Imd *imdPtr);

	virtual void playTot(int16 skipPlay) = 0;
	virtual void clearCollisions(void) = 0;
	virtual void addNewCollision(int16 id, int16 left, int16 top, int16 right,
			int16 bottom, int16 flags, int16 key, int16 funcEnter, int16 funcLeave) = 0;
	virtual int16 checkKeys(int16 *pMousex, int16 *pMouseY, int16 *pButtons,
					char handleMouse) = 0;
	virtual int16 checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
						  int16 *pResIndex) = 0;
	virtual void prepareStart(void) = 0;

	int8 _byte_2FC9B;
	int16 _word_2FC9C;
	int16 _word_2FC9E;
	int16 _word_2E51F;
	int32 _dword_2F2B6;
	Video::SurfaceDesc *_off_2E51B;
	Video::SurfaceDesc *_off_2E517;
	void sub_ADD2(void);
	void sub_BB28(void);

protected:

	int16 _lastCollKey;
	int16 _lastCollAreaIndex;
	int16 _lastCollId;

	int16 _activeCollResId;
	int16 _activeCollIndex;
	char _handleMouse;
	char _forceHandleMouse;

	char _tempStr[256];

	ExtTable *_extTable;
	char _curImaFile[18];

	int16 _collStackSize;
	int16 _collStackElemSizes[5];

	char _shouldPushColls;

	// Capture
	static Common::Rect _captureStack[20];
	static int16 _captureCount;

	char _collStr[256];

	GobEngine *_vm;
		
	int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex);
	void setCollisions(void);
	void collSub(int16 offset);
	void collAreaSub(int16 index, int8 enter);
	int16 openLocTextFile(char *locTextFile, int language);

	virtual void pushCollisions(char all) = 0;
	virtual void popCollisions(void) = 0;
};

class Game_v1 : public Game {
public:
	virtual void playTot(int16 skipPlay);
	virtual void clearCollisions(void);
	virtual void addNewCollision(int16 id, int16 left, int16 top, int16 right,
			int16 bottom, int16 flags, int16 key, int16 funcEnter, int16 funcLeave);
	virtual int16 checkKeys(int16 *pMousex, int16 *pMouseY, int16 *pButtons,
					char handleMouse);
	virtual int16 checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
						  int16 *pResIndex);
	virtual void prepareStart(void);

	Game_v1(GobEngine *vm);
	virtual ~Game_v1() {};

protected:
	virtual void pushCollisions(char all);
	virtual void popCollisions(void);
};

class Game_v2 : public Game_v1 {
public:
	virtual void playTot(int16 skipPlay);
	virtual void clearCollisions(void);
	virtual void addNewCollision(int16 id, int16 left, int16 top, int16 right,
			int16 bottom, int16 flags, int16 key, int16 funcEnter, int16 funcLeave);
	virtual int16 checkKeys(int16 *pMousex, int16 *pMouseY, int16 *pButtons,
					char handleMouse);
	virtual int16 checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
						  int16 *pResIndex);
	virtual void prepareStart(void);

	Game_v2(GobEngine *vm);
	virtual ~Game_v2() {};

protected:
	struct CollLast {
		int16 key;
		int16 id;
		int16 areaIndex;
	};

	CollLast _collLasts[5];

	virtual void pushCollisions(char all);
	virtual void popCollisions(void);
};

}				// End of namespace Gob

#endif
