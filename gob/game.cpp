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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/video.h"
#include "gob/dataio.h"
#include "gob/pack.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/parse.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/util.h"
#include "gob/goblin.h"
#include "gob/cdrom.h"

namespace Gob {

Game_ExtTable *game_extTable = 0;

char *game_totFileData = 0;
Game_TotTextTable *game_totTextData;
Game_TotResTable *game_totResourceTable = 0;
char *game_imFileData = 0;
int16 game_extHandle = 0;
char game_curExtFile[14];
char game_curTotFile[14];
char game_curImaFile[18];

Game_Collision *game_collisionAreas = 0;
char game_shouldPushColls = 0;
char game_collStr[256];

int16 game_lastCollKey;
int16 game_lastCollAreaIndex;
int16 game_lastCollId;

char game_handleMouse;
char game_forceHandleMouse;

char game_tempStr[256];

int16 game_activeCollResId;
int16 game_activeCollIndex;

// Collisions stack
int16 game_collStackSize = 0;
Game_Collision *game_collStack[3];
int16 game_collStackElemSizes[3];

int16 game_mouseButtons = 0;

// Capture
static Common::Rect game_captureStack[20];
static int16 game_captureCount = 0;

Snd_SoundDesc *game_soundSamples[20];

char game_soundFromExt[20];
char game_totToLoad[20];

int32 game_startTimeKey;

char *game_loadExtData(int16 itemId, int16 *pResWidth, int16 *pResHeight) {
	int16 commonHandle;
	int16 itemsCount;
	int32 offset;
	uint32 size;
	Game_ExtItem *item;
	char isPacked;
	int16 handle;
	int32 tableSize;
	char path[20];
	char *dataBuf;
	char *packedBuf;
	char *dataPtr;

	itemId -= 30000;
	if (game_extTable == 0)
		return 0;

	commonHandle = -1;
	itemsCount = game_extTable->itemsCount;
	item = &game_extTable->items[itemId];
	tableSize = szGame_ExtTable + szGame_ExtItem * itemsCount;

	offset = item->offset;
	size = item->size;
	if (item->width & 0x8000)
		isPacked = 1;
	else
		isPacked = 0;

	if (pResWidth != 0) {
		*pResWidth = item->width & 0x7fff;
		*pResHeight = item->height;
		debug(7, "game_loadExtData(%d, %d, %d)", itemId, *pResWidth, *pResHeight);
	}

	debug(7, "game_loadExtData(%d, 0, 0)", itemId);

	if (item->height == 0)
		size += (item->width & 0x7fff) << 16;

	debug(7, "size: %d off: %d", size, offset);
	if (offset >= 0) {
		handle = game_extHandle;
	} else {
		offset = -(offset + 1);
		tableSize = 0;
		data_closeData(game_extHandle);
		strcpy(path, "commun.ex1");
		path[strlen(path) - 1] = *(game_totFileData + 0x3c) + '0';
		commonHandle = data_openData(path);
		handle = commonHandle;
	}

	debug(7, "off: %ld size: %ld", offset, tableSize);
	data_seekData(handle, offset + tableSize, SEEK_SET);
	if (isPacked)
		dataBuf = (char *)malloc(size);
	else
		dataBuf = (char *)malloc(size);

	dataPtr = dataBuf;
	while (size > 32000) {
		// BUG: huge->far conversion. Need normalization?
		data_readData(handle, (char *)dataPtr, 32000);
		size -= 32000;
		dataPtr += 32000;
	}
	data_readData(handle, (char *)dataPtr, size);
	if (commonHandle != -1) {
		data_closeData(commonHandle);
		game_extHandle = data_openData(game_curExtFile);
	}

	if (isPacked != 0) {
		packedBuf = dataBuf;
		dataBuf = (char *)malloc(READ_LE_UINT32(packedBuf));
		unpackData(packedBuf, dataBuf);
		free(packedBuf);
	}

	return dataBuf;

}

void game_clearCollisions() {
	int16 i;
	for (i = 0; i < 250; i++) {
		game_collisionAreas[i].id = 0;
		game_collisionAreas[i].left = -1;
	}
}

void game_addNewCollision(int16 id, int16 left, int16 top, int16 right, int16 bottom,
	    int16 flags, int16 key, int16 funcEnter, int16 funcLeave) {
	int16 i;
	Game_Collision *ptr;

	debug(5, "game_addNewCollision");
	debug(5, "id = %x", id);
	debug(5, "left = %d, top = %d, right = %d, bottom = %d", left, top, right, bottom);
	debug(5, "flags = %x, key = %x", flags, key);
	debug(5, "funcEnter = %d, funcLeave = %d", funcEnter, funcLeave);

	for (i = 0; i < 250; i++) {
		if (game_collisionAreas[i].left != -1)
			continue;

		ptr = &game_collisionAreas[i];
		ptr->id = id;
		ptr->left = left;
		ptr->top = top;
		ptr->right = right;
		ptr->bottom = bottom;
		ptr->flags = flags;
		ptr->key = key;
		ptr->funcEnter = funcEnter;
		ptr->funcLeave = funcLeave;
		return;
	}
	error("game_addNewCollision: Collision array full!\n");
}

void game_freeCollision(int16 id) {
	int16 i;

	for (i = 0; i < 250; i++) {
		if (game_collisionAreas[i].id == id)
			game_collisionAreas[i].left = -1;
	}
}

void game_pushCollisions(char all) {
	Game_Collision *srcPtr;
	Game_Collision *destPtr;
	int16 size;

	debug(0, "game_pushCollisions");
	for (size = 0, srcPtr = game_collisionAreas; srcPtr->left != -1;
	    srcPtr++) {
		if (all || (srcPtr->id & 0x8000))
			size++;
	}

	destPtr = (Game_Collision *)malloc(size * sizeof(Game_Collision));
	game_collStack[game_collStackSize] = destPtr;
	game_collStackElemSizes[game_collStackSize] = size;
	game_collStackSize++;

	for (srcPtr = game_collisionAreas; srcPtr->left != -1; srcPtr++) {
		if (all || (srcPtr->id & 0x8000)) {
			memcpy(destPtr, srcPtr, sizeof(Game_Collision));
			srcPtr->left = -1;
			destPtr++;
		}
	}
}

void game_popCollisions(void) {
	Game_Collision *destPtr;
	Game_Collision *srcPtr;

	debug(0, "game_popCollision");

	game_collStackSize--;
	for (destPtr = game_collisionAreas; destPtr->left != -1; destPtr++);

	srcPtr = game_collStack[game_collStackSize];
	memcpy(destPtr, srcPtr,
	    game_collStackElemSizes[game_collStackSize] *
	    sizeof(Game_Collision));

	free(game_collStack[game_collStackSize]);
}

int16 game_checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Game_Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = game_collisionAreas;
	for (i = 0; ptr->left != -1; ptr++, i++) {
		if (all) {
			if ((ptr->flags & 0xf) > 1)
				continue;

			if ((ptr->flags & 0xff00) != 0)
				continue;

			if (inter_mouseX < ptr->left
			    || inter_mouseX > ptr->right
			    || inter_mouseY < ptr->top
			    || inter_mouseY > ptr->bottom)
				continue;

			if (resId != 0)
				*resId = ptr->id;

			*resIndex = i;
			return ptr->key;
		} else {
			if ((ptr->flags & 0xff00) != 0)
				continue;

			if ((ptr->flags & 0xf) != 1 && (ptr->flags & 0xf) != 2)
				continue;

			if ((ptr->flags & 0xf0) >> 4 != game_mouseButtons - 1
			    && (ptr->flags & 0xf0) >> 4 != 2)
				continue;

			if (inter_mouseX < ptr->left
			    || inter_mouseX > ptr->right
			    || inter_mouseY < ptr->top
			    || inter_mouseY > ptr->bottom)
				continue;

			if (resId != 0)
				*resId = ptr->id;
			*resIndex = i;
			return ptr->key;
		}
	}

	if (game_mouseButtons != 1 && all == 0)
		return 0x11b;

	return 0;
}

void game_capturePush(int16 left, int16 top, int16 width, int16 height) {
	int16 right;

	if (game_captureCount == 20)
		error("game_capturePush: Capture stack overflow!");

	game_captureStack[game_captureCount].left = left;
	game_captureStack[game_captureCount].top = top;
	game_captureStack[game_captureCount].right = left + width;
	game_captureStack[game_captureCount].bottom = top + height;

	draw_spriteTop = top;
	draw_spriteBottom = height;

	right = left + width - 1;
	left &= 0xfff0;
	right |= 0xf;

	draw_spritesArray[30 + game_captureCount] =
	    vid_initSurfDesc(videoMode, right - left + 1, height, 0);

	draw_sourceSurface = 21;
	draw_destSurface = 30 + game_captureCount;

	draw_spriteLeft = left;
	draw_spriteRight = right - left + 1;
	draw_destSpriteX = 0;
	draw_destSpriteY = 0;
	draw_transparency = 0;
	draw_spriteOperation(0);
	game_captureCount++;
}

void game_capturePop(char doDraw) {
	if (game_captureCount <= 0)
		return;

	game_captureCount--;
	if (doDraw) {
		draw_destSpriteX = game_captureStack[game_captureCount].left;
		draw_destSpriteY = game_captureStack[game_captureCount].top;
		draw_spriteRight =
		    game_captureStack[game_captureCount].width();
		draw_spriteBottom =
		    game_captureStack[game_captureCount].height();

		draw_transparency = 0;
		draw_sourceSurface = 30 + game_captureCount;
		draw_destSurface = 21;
		draw_spriteLeft = draw_destSpriteX & 0xf;
		draw_spriteTop = 0;
		draw_spriteOperation(0);
	}
	vid_freeSurfDesc(draw_spritesArray[30 + game_captureCount]);
}

char *game_loadTotResource(int16 id) {
	Game_TotResItem *itemPtr;
	int32 offset;

	itemPtr = &game_totResourceTable->items[id];
	offset = itemPtr->offset;
	if (offset >= 0) {
		return ((char *)game_totResourceTable) + szGame_TotResTable +
		    szGame_TotResItem * game_totResourceTable->itemsCount + offset;
	} else {
		return (char *)(game_imFileData + (int32)READ_LE_UINT32(&((int32 *)game_imFileData)[-offset - 1]));
	}
}

void game_loadSound(int16 slot, char *dataPtr) {
	Snd_SoundDesc *soundDesc;

	soundDesc = (Snd_SoundDesc *)malloc(sizeof(Snd_SoundDesc));

	game_soundSamples[slot] = soundDesc;

	soundDesc->frequency = (dataPtr[4] << 8) + dataPtr[5];
	soundDesc->size = (dataPtr[1] << 16) + (dataPtr[2] << 8) + dataPtr[3];
	soundDesc->data = dataPtr + 6;
	soundDesc->timerTicks = (int32)1193180 / (int32)soundDesc->frequency;

	soundDesc->inClocks = (soundDesc->frequency * 10) / 182;
	soundDesc->flag = 0;
}

void game_interLoadSound(int16 slot) {
	char *dataPtr;
	int16 id;

	if (slot == -1)
		slot = parse_parseValExpr();

	id = inter_load16();
	if (id == -1) {
		inter_execPtr += 9;
		return;
	}

	if (id >= 30000) {
		dataPtr = game_loadExtData(id, 0, 0);
		game_soundFromExt[slot] = 1;
	} else {
		dataPtr = game_loadTotResource(id);
		game_soundFromExt[slot] = 0;
	}

	game_loadSound(slot, dataPtr);
}

void game_freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = parse_parseValExpr();

	if (game_soundSamples[slot] == 0)
		return;

	if (game_soundFromExt[slot] == 1) {
		free(game_soundSamples[slot]->data - 6);
		game_soundFromExt[slot] = 0;
	}

	free(game_soundSamples[slot]);
	game_soundSamples[slot] = 0;
}

int16 game_checkKeys(int16 *pMouseX, int16 *pMouseY, int16 *pButtons, char handleMouse) {
	util_processInput();

	if (VAR(58) != 0) {
		if (mult_frameStart != (int)VAR(58) - 1)
			mult_frameStart++;
		else
			mult_frameStart = 0;

		mult_playMult(mult_frameStart + VAR(57), mult_frameStart + VAR(57), 1,
		    handleMouse);
	}

	if (inter_soundEndTimeKey != 0
	    && util_getTimeKey() >= inter_soundEndTimeKey) {
		snd_stopSound(inter_soundStopVal);
		inter_soundEndTimeKey = 0;
	}

	if (useMouse == 0)
		error("game_checkKeys: Can't work without mouse!");

	util_getMouseState(pMouseX, pMouseY, pButtons);

	if (*pButtons == 3)
		*pButtons = 0;

	return util_checkKey();
}

int16 game_checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
	    int16 *pResIndex) {
	char *savedIP;
	int16 resIndex;
	int16 key;
	int16 oldIndex;
	int16 oldId;
	uint32 timeKey;

	if (deltaTime >= -1) {
		game_lastCollKey = 0;
		game_lastCollAreaIndex = 0;
		game_lastCollId = 0;
	}

	if (pResId != 0)
		*pResId = 0;

	resIndex = 0;

	if (draw_cursorIndex == -1 && handleMouse != 0
	    && game_lastCollKey == 0) {
		game_lastCollKey =
		    game_checkMousePoint(1, &game_lastCollId,
		    &game_lastCollAreaIndex);

		if (game_lastCollKey != 0 && (game_lastCollId & 0x8000) != 0) {
			savedIP = inter_execPtr;
			inter_execPtr = (char *)game_totFileData +
			    game_collisionAreas[game_lastCollAreaIndex].funcEnter;

			inter_funcBlock(0);
			inter_execPtr = savedIP;
		}
	}

	if (handleMouse != 0)
		draw_animateCursor(-1);

	timeKey = util_getTimeKey();
	while (1) {
		if (inter_terminate != 0) {
			if (handleMouse)
				draw_blitCursor();
			return 0;
		}

		if (draw_noInvalidated == 0) {
			if (handleMouse)
				draw_animateCursor(-1);
			else
				draw_blitInvalidated();
		}

		// NOTE: the original asm does the below game_checkKeys call
		// _before_ this check. However, that can cause keypresses to get lost
		// since there's a return statement in this check.
		// Additionally, I added a 'deltaTime == -1' check there, since
		// when this function is called with deltaTime == -1 in game_inputArea,
		// and the return value is then discarded.
		if (deltaTime < 0) {
			uint32 curtime = util_getTimeKey();
			if (deltaTime == -1 || curtime + deltaTime > timeKey) {
				if (pResId != 0)
					*pResId = 0;

				if (pResIndex != 0)
					*pResIndex = 0;

				return 0;
			}
		}

		key = game_checkKeys(&inter_mouseX, &inter_mouseY,
							 &game_mouseButtons, handleMouse);

		if (handleMouse == 0 && game_mouseButtons != 0) {
			util_waitMouseRelease(0);
			key = 3;
		}

		if (key != 0) {

			if (handleMouse == 1)
				draw_blitCursor();

			if (pResId != 0)
				*pResId = 0;

			if (pResIndex != 0)
				*pResIndex = 0;

			if (game_lastCollKey != 0 &&
			    game_collisionAreas[game_lastCollAreaIndex].funcLeave != 0) {
				savedIP = inter_execPtr;
				inter_execPtr = (char *)game_totFileData +
				    game_collisionAreas[game_lastCollAreaIndex].funcLeave;

				inter_funcBlock(0);
				inter_execPtr = savedIP;
			}

			game_lastCollKey = 0;
			if (key != 0)
				return key;
		}

		if (handleMouse != 0) {
			if (game_mouseButtons != 0) {
				oldIndex = 0;

				draw_animateCursor(2);
				if (deltaTime <= 0) {
					if (handleMouse == 1)
						util_waitMouseRelease(1);
				} else if (deltaTime > 0) {
					util_delay(deltaTime);
				}

				draw_animateCursor(-1);
				if (pResId != 0)
					*pResId = 0;

				key = game_checkMousePoint(0, pResId, &resIndex);

				if (pResIndex != 0)
					*pResIndex = resIndex;

				if (key != 0 || (pResId != 0 && *pResId != 0)) {
					if (handleMouse == 1 && (deltaTime <= 0
						|| game_mouseButtons == 0))
						draw_blitCursor();

					if (game_lastCollKey != 0 &&
						game_collisionAreas[game_lastCollAreaIndex].funcLeave != 0) {
						savedIP = inter_execPtr;
						inter_execPtr =
						    (char *)game_totFileData +
						    game_collisionAreas[game_lastCollAreaIndex].funcLeave;

						inter_funcBlock(0);
						inter_execPtr = savedIP;
					}
					game_lastCollKey = 0;
					return key;
				}

				if (game_lastCollKey != 0 &&
				    game_collisionAreas[game_lastCollAreaIndex].funcLeave != 0) {
					savedIP = inter_execPtr;
					inter_execPtr =
					    (char *)game_totFileData +
					    game_collisionAreas[game_lastCollAreaIndex].funcLeave;

					inter_funcBlock(0);
					inter_execPtr = savedIP;
				}

				game_lastCollKey =
				    game_checkMousePoint(1, &game_lastCollId,
				    &game_lastCollAreaIndex);

				if (game_lastCollKey != 0
				    && (game_lastCollId & 0x8000) != 0) {
					savedIP = inter_execPtr;
					inter_execPtr =
					    (char *)game_totFileData +
					    game_collisionAreas[game_lastCollAreaIndex].funcEnter;

					inter_funcBlock(0);
					inter_execPtr = savedIP;
				}
			} else {

				if (handleMouse != 0 &&
				    (inter_mouseX != draw_cursorX
					|| inter_mouseY != draw_cursorY)) {
					oldIndex = game_lastCollAreaIndex;
					oldId = game_lastCollId;

					key =
					    game_checkMousePoint(1,
					    &game_lastCollId,
					    &game_lastCollAreaIndex);

					if (key != game_lastCollKey) {
						if (game_lastCollKey != 0
						    && (oldId & 0x8000) != 0) {
							savedIP = inter_execPtr;
							inter_execPtr = (char *)game_totFileData +
							    game_collisionAreas[oldIndex].funcLeave;

							inter_funcBlock(0);
							inter_execPtr = savedIP;
						}

						game_lastCollKey = key;
						if (game_lastCollKey != 0 && (game_lastCollId & 0x8000) != 0) {
							savedIP = inter_execPtr;
							inter_execPtr = (char *)game_totFileData +
							    game_collisionAreas[game_lastCollAreaIndex].funcEnter;

							inter_funcBlock(0);
							inter_execPtr = savedIP;
						}
					}
				}
			}
		}

		if (handleMouse != 0)
			draw_animateCursor(-1);

		util_delay(10);

		snd_loopSounds();
	}
}

int16 game_inputArea(int16 xPos, int16 yPos, int16 width, int16 height, int16 backColor,
	    int16 frontColor, char *str, int16 fontIndex, char inpType, int16 *pTotTime) {
	int16 handleMouse;
	uint32 editSize;
	FontDesc *pFont;
	char curSym;
	int16 key;
	const char *str1;
	const char *str2;
	int16 i;
	uint32 pos;
	int16 flag;
	int16 savedKey;

	if (game_handleMouse != 0 &&
	    (useMouse != 0 || game_forceHandleMouse != 0))
		handleMouse = 1;
	else
		handleMouse = 0;

	pos = strlen(str);
	pFont = draw_fonts[fontIndex];
	editSize = width / pFont->itemWidth;

	while (1) {
		strcpy(game_tempStr, str);
		strcat(game_tempStr, " ");
		if (strlen(game_tempStr) > editSize)
			strcpy(game_tempStr, str);

		draw_destSpriteX = xPos;
		draw_destSpriteY = yPos;
		draw_spriteRight = editSize * pFont->itemWidth;
		draw_spriteBottom = height;

		draw_destSurface = 21;
		draw_backColor = backColor;
		draw_frontColor = frontColor;
		draw_textToPrint = game_tempStr;
		draw_transparency = 1;
		draw_fontIndex = fontIndex;
		draw_spriteOperation(DRAW_FILLRECT);

		draw_destSpriteY = yPos + (height - 8) / 2;

		draw_spriteOperation(DRAW_PRINTTEXT);
		if (pos == editSize)
			pos--;

		curSym = game_tempStr[pos];

		flag = 1;

		while (1) {
			game_tempStr[0] = curSym;
			game_tempStr[1] = 0;

			draw_destSpriteX = xPos + pFont->itemWidth * pos;
			draw_destSpriteY = yPos + height - 1;
			draw_spriteRight = pFont->itemWidth;
			draw_spriteBottom = 1;
			draw_destSurface = 21;
			draw_backColor = frontColor;
			draw_spriteOperation(DRAW_FILLRECT);

			if (flag != 0) {
				key = game_checkCollisions(handleMouse, -1,
				    &game_activeCollResId,
				    &game_activeCollIndex);
			}
			flag = 0;

			key = game_checkCollisions(handleMouse, -300,
			    &game_activeCollResId, &game_activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					game_activeCollResId = 0;
					break;
				}
			}

			game_tempStr[0] = curSym;
			game_tempStr[1] = 0;
			draw_destSpriteX = xPos + pFont->itemWidth * pos;
			draw_destSpriteY = yPos + height - 1;
			draw_spriteRight = pFont->itemWidth;
			draw_spriteBottom = 1;
			draw_destSurface = 21;
			draw_backColor = backColor;
			draw_frontColor = frontColor;
			draw_textToPrint = game_tempStr;
			draw_transparency = 1;
			draw_spriteOperation(DRAW_FILLRECT);

			draw_destSpriteY = yPos + (height - 8) / 2;
			draw_spriteOperation(DRAW_PRINTTEXT);

			if (key != 0 || game_activeCollResId != 0)
				break;

			key = game_checkCollisions(handleMouse, -300,
			    &game_activeCollResId, &game_activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					game_activeCollResId = 0;
					break;
				}

			}
			if (key != 0 || game_activeCollResId != 0)
				break;

			if (inter_terminate != 0)
				return 0;
		}

		if (key == 0 || game_activeCollResId != 0
		    || inter_terminate != 0)
			return 0;

		switch (key) {
		case 0x4d00:	// Right Arrow
			if (pos < strlen(str) && pos < editSize - 1) {
				pos++;
				continue;
			}
			return 0x5000;

		case 0x4b00:	// Left Arrow
			if (pos > 0) {
				pos--;
				continue;
			}
			return 0x4800;

		case 0xe08:	// Backspace
			if (pos > 0) {
				util_cutFromStr(str, pos - 1, 1);
				pos--;
				continue;
			}

		case 0x5300:	// Del

			if (pos >= strlen(str))
				continue;

			util_cutFromStr(str, pos, 1);
			continue;

		case 0x1c0d:	// Enter
		case 0x3b00:	// F1
		case 0x3c00:	// F2
		case 0x3d00:	// F3
		case 0x3e00:	// F4
		case 0x3f00:	// F5
		case 0x4000:	// F6
		case 0x4100:	// F7
		case 0x4200:	// F8
		case 0x4300:	// F9
		case 0x4400:	// F10
		case 0x4800:	// Up arrow
		case 0x5000:	// Down arrow
			return key;

		case 0x11b:	// Escape
			if (useMouse != 0)
				continue;

			game_forceHandleMouse = !game_forceHandleMouse;

			if (game_handleMouse != 0 &&
			    (useMouse != 0 || game_forceHandleMouse != 0))
				handleMouse = 1;
			else
				handleMouse = 0;

			if (pressedKeys[1] == 0)
				continue;

			while (pressedKeys[1] != 0);
			continue;

		default:

			savedKey = key;
			key &= 0xff;

			if ((inpType == 9 || inpType == 10) && key >= ' '
			    && key <= 0xff) {
				str1 = "0123456789-.,+ ";
				str2 = "0123456789-,,+ ";

				if ((savedKey >> 8) > 1
				    && (savedKey >> 8) < 12)
					key = ((savedKey >> 8) - 1) % 10 + '0';

				for (i = 0; str1[i] != 0; i++) {
					if (key == str1[i]) {
						key = str2[i];
						break;
					}
				}

				if (i == (int16)strlen(str1))
					key = 0;
			}

			if (key >= ' ' && key <= 0xff) {
				if (editSize == strlen(str))
					util_cutFromStr(str, strlen(str) - 1,
					    1);

				if (key >= 'a' && key <= 'z')
					key += ('A' - 'a');

				pos++;
				game_tempStr[0] = key;
				game_tempStr[1] = 0;

				util_insertStr(game_tempStr, str, pos - 1);

				//strupr(str);
			}
		}
	}
}

int16 game_multiEdit(int16 time, int16 index, int16 *pCurPos, Game_InputDesc * inpDesc) {
	Game_Collision *collArea;
	int16 descInd;
	int16 key;
	int16 found = -1;
	int16 i;

	descInd = 0;
	for (i = 0; i < 250; i++) {
		collArea = &game_collisionAreas[i];

		if (collArea->left == -1)
			continue;

		if ((collArea->id & 0x8000) == 0)
			continue;

		if ((collArea->flags & 0x0f) < 3)
			continue;

		if ((collArea->flags & 0x0f) > 10)
			continue;

		strcpy(game_tempStr, inter_variables + collArea->key);

		draw_destSpriteX = collArea->left;
		draw_destSpriteY = collArea->top;
		draw_spriteRight = collArea->right - collArea->left + 1;
		draw_spriteBottom = collArea->bottom - collArea->top + 1;

		draw_destSurface = 21;

		draw_backColor = inpDesc[descInd].backColor;
		draw_frontColor = inpDesc[descInd].frontColor;
		draw_textToPrint = game_tempStr;
		draw_transparency = 1;
		draw_fontIndex = inpDesc[descInd].fontIndex;
		draw_spriteOperation(DRAW_FILLRECT);
		draw_destSpriteY +=
		    ((collArea->bottom - collArea->top + 1) - 8) / 2;

		draw_spriteOperation(DRAW_PRINTTEXT);
		descInd++;
	}

	for (i = 0; i < 40; i++) {
		WRITE_VAR_OFFSET(i * 4 + 0x44, 0);
	}

	while (1) {
		descInd = 0;

		for (i = 0; i < 250; i++) {
			collArea = &game_collisionAreas[i];

			if (collArea->left == -1)
				continue;

			if ((collArea->id & 0x8000) == 0)
				continue;

			if ((collArea->flags & 0x0f) < 3)
				continue;

			if ((collArea->flags & 0x0f) > 10)
				continue;

			if (descInd == *pCurPos) {
				found = i;
				break;
			}

			descInd++;
		}

		assert(found != -1);

		collArea = &game_collisionAreas[found];

		key = game_inputArea(collArea->left, collArea->top,
		    collArea->right - collArea->left + 1,
		    collArea->bottom - collArea->top + 1,
		    inpDesc[*pCurPos].backColor, inpDesc[*pCurPos].frontColor,
		    inter_variables + collArea->key,
		    inpDesc[*pCurPos].fontIndex, collArea->flags, &time);

		if (inter_terminate != 0)
			return 0;

		switch (key) {
		case 0:
			if (game_activeCollResId == 0)
				return 0;

			if ((game_collisionAreas[game_activeCollIndex].
				flags & 0x0f) < 3)
				return 0;

			if ((game_collisionAreas[game_activeCollIndex].
				flags & 0x0f) > 10)
				return 0;

			*pCurPos = 0;
			for (i = 0; i < 250; i++) {
				collArea = &game_collisionAreas[i];

				if (collArea->left == -1)
					continue;

				if ((collArea->id & 0x8000) == 0)
					continue;

				if ((collArea->flags & 0x0f) < 3)
					continue;

				if ((collArea->flags & 0x0f) > 10)
					continue;

				if (i == game_activeCollIndex)
					break;

				pCurPos[0]++;
			}
			break;

		case 0x3b00:
		case 0x3c00:
		case 0x3d00:
		case 0x3e00:
		case 0x3f00:
		case 0x4000:
		case 0x4100:
		case 0x4200:
		case 0x4300:
		case 0x4400:
			return key;

		case 0x1c0d:

			if (index == 1)
				return key;

			if (*pCurPos == index - 1) {
				*pCurPos = 0;
				break;
			}

			pCurPos[0]++;
			break;

		case 0x5000:
			if (index - 1 > *pCurPos)
				pCurPos[0]++;
			break;

		case 0x4800:
			if (*pCurPos > 0)
				pCurPos[0]--;
			break;
		}
	}
}

int16 game_adjustKey(int16 key) {
	if (key <= 0x60 || key >= 0x7b)
		return key;

	return key - 0x20;
}

void game_collisionsBlock(void) {
	Game_InputDesc descArray[20];
	int16 array[250];
	char count;
	int16 collResId;
	char *startIP;
	int16 curCmd;
	int16 cmd;
	int16 cmdHigh;
	int16 key;
	int16 flags;
	int16 left;
	int16 top;
	int16 width;
	int16 height;
	int16 var_22;
	int16 index;
	int16 curEditIndex;
	int16 deltaTime;
	int16 descIndex2;
	int16 stackPos2;
	int16 descIndex;
	int16 timeVal;
	char *str;
	int16 pos;
	int16 savedCollStackSize;
	int16 i;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 collStackPos;
	Game_Collision *collPtr;
	int16 timeKey;
	char *savedIP;

	if (game_shouldPushColls)
		game_pushCollisions(1);

	collResId = -1;
	inter_execPtr++;
	count = *inter_execPtr++;
	game_handleMouse = inter_execPtr[0];
	deltaTime = 1000 * (byte)inter_execPtr[1];
	descIndex2 = (byte)inter_execPtr[2];
	stackPos2 = (byte)inter_execPtr[3];
	descIndex = (byte)inter_execPtr[4];

	if (stackPos2 != 0 || descIndex != 0)
		deltaTime /= 100;

	timeVal = deltaTime;
	inter_execPtr += 6;

	startIP = inter_execPtr;
	WRITE_VAR(16, 0);
	var_22 = 0;
	index = 0;
	curEditIndex = 0;

	for (curCmd = 0; curCmd < count; curCmd++) {
		array[curCmd] = 0;
		cmd = *inter_execPtr++;

		if ((cmd & 0x40) != 0) {
			cmd -= 0x40;
			cmdHigh = (byte)*inter_execPtr;
			inter_execPtr++;
			cmdHigh <<= 8;
		} else {
			cmdHigh = 0;
		}

		if ((cmd & 0x80) != 0) {
			left = parse_parseValExpr();
			top = parse_parseValExpr();
			width = parse_parseValExpr();
			height = parse_parseValExpr();
		} else {
			left = inter_load16();
			top = inter_load16();
			width = inter_load16();
			height = inter_load16();
		}
		cmd &= 0x7f;

		debug(4, "game_collisionsBlock(%d)", cmd);

		switch (cmd) {
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:

			util_waitKey();
			var_22 = 1;
			key = parse_parseVarIndex();
			descArray[index].fontIndex = inter_load16();
			descArray[index].backColor = *inter_execPtr++;
			descArray[index].frontColor = *inter_execPtr++;

			if (cmd < 5 || cmd > 8) {
				descArray[index].ptr = 0;
			} else {
				descArray[index].ptr = inter_execPtr + 2;
				inter_execPtr += inter_load16();
			}

			if (left == -1)
				break;

			if ((cmd & 1) == 0) {
				game_addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    draw_fonts[descArray[index].fontIndex]->
				    itemWidth - 1, top + height - 1, cmd, key,
				    0,
				    inter_execPtr - (char *)game_totFileData);

				inter_execPtr += 2;
				inter_execPtr += READ_LE_UINT16(inter_execPtr);
			} else {
				game_addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    draw_fonts[descArray[index].fontIndex]->
				    itemWidth - 1, top + height - 1, cmd, key,
				    0, 0);
			}
			index++;
			break;

		case 21:
			key = inter_load16();
			array[curCmd] = inter_load16();
			flags = inter_load16() & 3;

			game_addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmdHigh + 2, key,
			    inter_execPtr - (char *)game_totFileData, 0);

			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = inter_load16();
			array[curCmd] = inter_load16();
			flags = inter_load16() & 3;

			game_addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmdHigh + 2, key, 0,
			    inter_execPtr - (char *)game_totFileData);

			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			break;

		case 0:
			inter_execPtr += 6;
			startIP = inter_execPtr;
			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			key = curCmd + 0xA000;

			game_addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    cmd + cmdHigh, key,
			    startIP - (char *)game_totFileData,
			    inter_execPtr - (char *)game_totFileData);

			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			break;

		case 1:
			key = inter_load16();
			array[curCmd] = inter_load16();
			flags = inter_load16() & 3;

			startIP = inter_execPtr;
			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			if (key == 0)
				key = curCmd + 0xa000;

			game_addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmd + cmdHigh, key,
			    startIP - (char *)game_totFileData,
			    inter_execPtr - (char *)game_totFileData);

			inter_execPtr += 2;
			inter_execPtr += READ_LE_UINT16(inter_execPtr);
			break;
		}
	}

	game_forceHandleMouse = 0;
	util_waitKey();

	do {
		if (var_22 != 0) {
			key =
			    game_multiEdit(deltaTime, index, &curEditIndex,
			    descArray);

			if (key == 0x1c0d) {
				for (i = 0; i < 250; i++) {
					if (game_collisionAreas[i].left == -1)
						continue;

					if ((game_collisionAreas[i].id & 0x8000) == 0)
						continue;

					if ((game_collisionAreas[i].flags & 1) != 0)
						continue;

					if ((game_collisionAreas[i].flags & 0x0f) <= 2)
						continue;

					collResId = game_collisionAreas[i].id;
					game_activeCollResId = collResId;
					collResId &= 0x7fff;
					game_activeCollIndex = i;
					break;
				}
				break;
			}
		} else {
			key =
			    game_checkCollisions(game_handleMouse, -deltaTime,
			    &game_activeCollResId, &game_activeCollIndex);
		}

		if ((key & 0xff) >= ' ' && (key & 0xff) <= 0xff &&
		    (key >> 8) > 1 && (key >> 8) < 12) {
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xff00);
		}

		if (game_activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 250; i++) {
					if (game_collisionAreas[i].left == -1)
						continue;

					if ((game_collisionAreas[i].
						id & 0x8000) == 0)
						continue;

					if (game_collisionAreas[i].key == key
					    || game_collisionAreas[i].key ==
					    0x7fff) {

						game_activeCollResId =
						    game_collisionAreas[i].id;
						game_activeCollIndex = i;
						break;
					}
				}

				if (game_activeCollResId == 0) {
					for (i = 0; i < 250; i++) {
						if (game_collisionAreas[i].left == -1)
							continue;

						if ((game_collisionAreas[i].id & 0x8000) == 0)
							continue;

						if ((game_collisionAreas[i].key & 0xff00) != 0)
							continue;

						if (game_collisionAreas[i].key == 0)
							continue;

						if (game_adjustKey(key & 0xff) == game_adjustKey(game_collisionAreas[i].key) || game_collisionAreas[i].key == 0x7fff) {
							game_activeCollResId = game_collisionAreas[i].id;
							game_activeCollIndex = i;
							break;
						}
					}
				}
			} else {

				if (deltaTime != 0 && VAR(16) == 0) {
					if (stackPos2 != 0) {
						collStackPos = 0;
						collPtr = game_collisionAreas;

						for (i = 0, collPtr = game_collisionAreas; collPtr->left != -1; i++, collPtr++) {
							if ((collPtr->id & 0x8000) == 0)
								continue;

							collStackPos++;
							if (collStackPos != stackPos2)
								continue;

							game_activeCollResId = collPtr->id;
							game_activeCollIndex = i;
							WRITE_VAR(2, inter_mouseX);
							WRITE_VAR(3, inter_mouseY);
							WRITE_VAR(4, game_mouseButtons);
							WRITE_VAR(16, array[(uint16)game_activeCollResId & ~0x8000]);

							if (collPtr->funcLeave != 0) {
								timeKey = util_getTimeKey();
								savedIP = inter_execPtr;
								inter_execPtr = (char *)game_totFileData + collPtr->funcLeave;
								game_shouldPushColls = 1;
								savedCollStackSize = game_collStackSize;
								inter_funcBlock(0);

								if (savedCollStackSize != game_collStackSize)
									game_popCollisions();

								game_shouldPushColls = 0;
								inter_execPtr = savedIP;
								deltaTime = timeVal - (util_getTimeKey() - timeKey);

								if (deltaTime < 2)
									deltaTime = 2;
							}

							if (VAR(16) == 0)
								game_activeCollResId = 0;
							break;
						}
					} else {
						if (descIndex != 0) {
							counter = 0;
							for (i = 0; i < 250; i++) {
								if (game_collisionAreas[i].left == -1)
									continue;

								if ((game_collisionAreas[i].id & 0x8000) == 0)
									continue;

								counter++;
								if (counter != descIndex)
									continue;

								game_activeCollResId = game_collisionAreas[i].id;
								game_activeCollIndex = i;
								break;
							}
						} else {
							for (i = 0; i < 250; i++) {
								if (game_collisionAreas[i].left == -1)
									continue;

								if ((game_collisionAreas[i].id & 0x8000) == 0)
									continue;

								game_activeCollResId = game_collisionAreas[i].id;
								game_activeCollIndex = i;
								break;
							}
						}
					}
				} else {
					if (descIndex2 != 0) {
						counter = 0;
						for (i = 0; i < 250; i++) {
							if (game_collisionAreas[i].left == -1)
								continue;

							if ((game_collisionAreas[i].id & 0x8000) == 0)
								continue;

							counter++;
							if (counter != descIndex2)
								continue;

							game_activeCollResId = game_collisionAreas[i].id;
							game_activeCollIndex = i;
							break;
						}
					}
				}
			}
		}

		if (game_activeCollResId == 0)
			continue;

		if (game_collisionAreas[game_activeCollIndex].funcLeave != 0)
			continue;

		WRITE_VAR(2, inter_mouseX);
		WRITE_VAR(3, inter_mouseY);
		WRITE_VAR(4, game_mouseButtons);
		WRITE_VAR(16, array[(uint16)game_activeCollResId & ~0x8000]);

		if (game_collisionAreas[game_activeCollIndex].funcEnter != 0) {
			savedIP = inter_execPtr;
			inter_execPtr = (char *)game_totFileData +
			    game_collisionAreas[game_activeCollIndex].
			    funcEnter;

			game_shouldPushColls = 1;

			collStackPos = game_collStackSize;
			inter_funcBlock(0);
			if (collStackPos != game_collStackSize)
				game_popCollisions();
			game_shouldPushColls = 0;
			inter_execPtr = savedIP;
		}

		WRITE_VAR(16, 0);
		game_activeCollResId = 0;
	}
	while (game_activeCollResId == 0 && inter_terminate == 0);

	if (((uint16)game_activeCollResId & ~0x8000) == collResId) {
		collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 250; i++) {
			if (game_collisionAreas[i].left == -1)
				continue;

			if ((game_collisionAreas[i].id & 0x8000) == 0)
				continue;

			if ((game_collisionAreas[i].flags & 0x0f) < 3)
				continue;

			if ((game_collisionAreas[i].flags & 0x0f) > 10)
				continue;

			if ((game_collisionAreas[i].flags & 0x0f) > 8) {
				strcpy(game_tempStr,
				    inter_variables + game_collisionAreas[i].key);
				while ((pos =
					util_strstr(" ", game_tempStr)) != 0) {
					util_cutFromStr(game_tempStr, pos - 1, 1);
					pos = util_strstr(" ", game_tempStr);
				}
				strcpy(inter_variables + game_collisionAreas[i].key, game_tempStr);
			}

			if ((game_collisionAreas[i].flags & 0x0f) >= 5 &&
			    (game_collisionAreas[i].flags & 0x0f) <= 8) {
				str = descArray[var_24].ptr;

				strcpy(game_tempStr, inter_variables + game_collisionAreas[i].key);

				if ((game_collisionAreas[i].flags & 0x0f) < 7)
					util_prepareStr(game_tempStr);

				pos = 0;
				do {
					strcpy(game_collStr, str);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if ((game_collisionAreas[i].flags & 0x0f) < 7)
						util_prepareStr(game_collStr);

					if (strcmp(game_tempStr, game_collStr) == 0) {
						VAR(17)++;
						WRITE_VAR(17 + var_26, 1);
						break;
					}
				} while (READ_LE_UINT16(descArray[var_24].ptr - 2) > pos);
				collStackPos++;
			} else {
				VAR(17 + var_26) = 2;
			}
			var_24++;
			var_26++;
		}

		if (collStackPos != (int16)VAR(17))
			WRITE_VAR(17, 0);
		else
			WRITE_VAR(17, 1);
	}

	savedIP = 0;
	if (inter_terminate == 0) {
		savedIP = (char *)game_totFileData +
		    game_collisionAreas[game_activeCollIndex].funcLeave;

		WRITE_VAR(2, inter_mouseX);
		WRITE_VAR(3, inter_mouseY);
		WRITE_VAR(4, game_mouseButtons);

		if (VAR(16) == 0) {
			WRITE_VAR(16, array[(uint16)game_activeCollResId & ~0x8000]);
		}
	}

	for (curCmd = 0; curCmd < count; curCmd++) {
		game_freeCollision(curCmd + 0x8000);
	}
	inter_execPtr = savedIP;
}

void game_prepareStart(void) {
	int16 i;

	game_clearCollisions();

	pPaletteDesc->unused2 = draw_unusedPalette2;
	pPaletteDesc->unused1 = draw_unusedPalette1;
	pPaletteDesc->vgaPal = draw_vgaPalette;

	vid_setFullPalette(pPaletteDesc);

	draw_backSurface = vid_initSurfDesc(videoMode, 320, 200, 0);

	vid_fillRect(draw_backSurface, 0, 0, 319, 199, 1);
	draw_frontSurface = pPrimarySurfDesc;
	vid_fillRect(draw_frontSurface, 0, 0, 319, 199, 1);

	util_setMousePos(152, 92);

	draw_cursorX = 152;
	inter_mouseX = 152;

	draw_cursorY = 92;
	inter_mouseY = 92;
	draw_invalidatedCount = 0;
	draw_noInvalidated = 1;
	draw_applyPal = 0;
	draw_paletteCleared = 0;
	draw_cursorWidth = 16;
	draw_cursorHeight = 16;
	draw_transparentCursor = 1;

	for (i = 0; i < 40; i++) {
		draw_cursorAnimLow[i] = -1;
		draw_cursorAnimDelays[i] = 0;
		draw_cursorAnimHigh[i] = 0;
	}

	draw_cursorAnimLow[1] = 0;
	draw_cursorSprites = vid_initSurfDesc(videoMode, 32, 16, 2);
	draw_cursorBack = vid_initSurfDesc(videoMode, 16, 16, 0);
	draw_renderFlags = 0;
	draw_backDeltaX = 0;
	draw_backDeltaY = 0;

	game_startTimeKey = util_getTimeKey();
}

void game_loadTotFile(char *path) {
	int16 handle;

	handle = data_openData(path);
	if (handle >= 0) {
		data_closeData(handle);
		game_totFileData = data_getData(path);
	} else {
		game_totFileData = 0;
	}
}

void game_loadExtTable(void) {
	int16 count, i;

	// Function is correct. [sev]

	game_extHandle = data_openData(game_curExtFile);
	if (game_extHandle < 0)
		return;

	data_readData(game_extHandle, (char *)&count, 2);
	count = FROM_LE_16(count);

	data_seekData(game_extHandle, 0, 0);
	game_extTable = (Game_ExtTable *)malloc(sizeof(Game_ExtTable)
	    + sizeof(Game_ExtItem) * count);

	data_readData(game_extHandle, (char *)&game_extTable->itemsCount, 2);
	game_extTable->itemsCount = FROM_LE_16(game_extTable->itemsCount);
	data_readData(game_extHandle, (char *)&game_extTable->unknown, 1);

	for (i = 0; i < count; i++) {
		data_readData(game_extHandle, (char *)&game_extTable->items[i].offset, 4);
		game_extTable->items[i].offset = FROM_LE_32(game_extTable->items[i].offset);
		data_readData(game_extHandle, (char *)&game_extTable->items[i].size, 2);
		game_extTable->items[i].size = FROM_LE_16(game_extTable->items[i].size);
		data_readData(game_extHandle, (char *)&game_extTable->items[i].width, 2);
		game_extTable->items[i].width = FROM_LE_16(game_extTable->items[i].width);
		data_readData(game_extHandle, (char *)&game_extTable->items[i].height, 2);
		game_extTable->items[i].height = FROM_LE_16(game_extTable->items[i].height);
	}
}

void game_loadImFile(void) {
	char path[20];
	int16 handle;

	if (game_totFileData[0x3d] != 0 && game_totFileData[0x3b] == 0)
		return;

	strcpy(path, "commun.im1");
	if (game_totFileData[0x3b] != 0)
		path[strlen(path) - 1] = '0' + game_totFileData[0x3b];

	handle = data_openData(path);
	if (handle < 0)
		return;

	data_closeData(handle);
	game_imFileData = data_getData(path);
}

void game_playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 captureCounter;
	int16 breakFrom;
	int16 nestLevel;
	char needTextFree;
	char needFreeResTable;
	char *curPtr;
	int32 variablesCount;
	char *filePtr;
	char *savedIP;
	int16 i;

	oldNestLevel = inter_nestLevel;
	oldBreakFrom = inter_breakFromLevel;
	oldCaptureCounter = scen_pCaptureCounter;
	savedIP = inter_execPtr;

	inter_nestLevel = &nestLevel;
	inter_breakFromLevel = &breakFrom;
	scen_pCaptureCounter = &captureCounter;
	strcpy(savedTotName, game_curTotFile);

	if (skipPlay == 0) {
		while (1) {
			for (i = 0; i < 4; i++) {
				draw_fontToSprite[i].sprite = -1;
				draw_fontToSprite[i].base = -1;
				draw_fontToSprite[i].width = -1;
				draw_fontToSprite[i].height = -1;
			}

			cd_stopPlaying();
			draw_animateCursor(4);
			inter_initControlVars();
			mult_initAll();
			mult_zeroMultData();

			for (i = 0; i < 20; i++)
				draw_spritesArray[i] = 0;

			draw_spritesArray[20] = draw_frontSurface;
			draw_spritesArray[21] = draw_backSurface;
			draw_spritesArray[23] = draw_cursorSprites;

			for (i = 0; i < 20; i++)
				game_soundSamples[i] = 0;

			game_totTextData = 0;
			game_totResourceTable = 0;
			game_imFileData = 0;
			game_extTable = 0;
			game_extHandle = -1;

			needFreeResTable = 1;
			needTextFree = 1;

			game_totToLoad[0] = 0;

			if (game_curTotFile[0] == 0 && game_totFileData == 0)
				break;

			game_loadTotFile(game_curTotFile);
			if (game_totFileData == 0) {
				draw_blitCursor();
				break;
			}

			strcpy(game_curImaFile, game_curTotFile);
			strcpy(game_curExtFile, game_curTotFile);

			game_curImaFile[strlen(game_curImaFile) - 4] = 0;
			strcat(game_curImaFile, ".ima");

			game_curExtFile[strlen(game_curExtFile) - 4] = 0;
			strcat(game_curExtFile, ".ext");

			debug(0, "IMA: %s", game_curImaFile);
			debug(0, "EXT: %s", game_curExtFile);

			filePtr = (char *)game_totFileData + 0x30;

			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = game_totFileData;
				game_totTextData =
				    (Game_TotTextTable *) (curPtr +
				    READ_LE_UINT32((char *)game_totFileData + 0x30));

				game_totTextData->itemsCount = (int16)READ_LE_UINT16(&game_totTextData->itemsCount);

				for (i = 0; i < game_totTextData->itemsCount; ++i) {
					game_totTextData->items[i].offset = (int16)READ_LE_UINT16(&game_totTextData->items[i].offset);
					game_totTextData->items[i].size = (int16)READ_LE_UINT16(&game_totTextData->items[i].size);
				}

				needTextFree = 0;
			}

			filePtr = (char *)game_totFileData + 0x34;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = game_totFileData;

				game_totResourceTable =
					(Game_TotResTable *)(curPtr +
				    READ_LE_UINT32((char *)game_totFileData + 0x34));

				game_totResourceTable->itemsCount = (int16)READ_LE_UINT16(&game_totResourceTable->itemsCount);

				for (i = 0; i < game_totResourceTable->itemsCount; ++i) {
					game_totResourceTable->items[i].offset = (int32)READ_LE_UINT32(&game_totResourceTable->items[i].offset);
					game_totResourceTable->items[i].size = (int16)READ_LE_UINT16(&game_totResourceTable->items[i].size);
					game_totResourceTable->items[i].width = (int16)READ_LE_UINT16(&game_totResourceTable->items[i].width);
					game_totResourceTable->items[i].height = (int16)READ_LE_UINT16(&game_totResourceTable->items[i].height);
				}

				needFreeResTable = 0;
			}

			game_loadImFile();
			game_loadExtTable();

			inter_animDataSize = READ_LE_UINT16((char *)game_totFileData + 0x38);
			if (inter_variables == 0) {
				variablesCount = READ_LE_UINT32((char *)game_totFileData + 0x2c);
				inter_variables = (char *)malloc(variablesCount * 4);
				for (i = 0; i < variablesCount; i++)
					WRITE_VAR(i, 0);
			}

			inter_execPtr = (char *)game_totFileData;
			inter_execPtr += READ_LE_UINT32((char *)game_totFileData + 0x64);

			inter_renewTimeInVars();

			WRITE_VAR(13, useMouse);
			WRITE_VAR(14, soundFlags);
			WRITE_VAR(15, videoMode);
			WRITE_VAR(16, language);

			inter_callSub(2);

			if (game_totToLoad[0] != 0)
				inter_terminate = 0;

			variablesCount = READ_LE_UINT32((char *)game_totFileData + 0x2c);
			draw_blitInvalidated();
			free(game_totFileData);
			game_totFileData = 0;

			if (needTextFree)
				free(game_totTextData);
			game_totTextData = 0;

			if (needFreeResTable)
				free(game_totResourceTable);
			game_totResourceTable = 0;

			free(game_imFileData);
			game_imFileData = 0;

			free(game_extTable);
			game_extTable = 0;

			if (game_extHandle >= 0)
				data_closeData(game_extHandle);

			game_extHandle = -1;

			for (i = 0; i < *scen_pCaptureCounter; i++)
				game_capturePop(0);

			mult_checkFreeMult();
			mult_freeAll();

			for (i = 0; i < 20; i++) {
				if (draw_spritesArray[i] != 0)
					vid_freeSurfDesc(draw_spritesArray[i]);
				draw_spritesArray[i] = 0;
			}
			snd_stopSound(0);

			for (i = 0; i < 20; i++)
				game_freeSoundSlot(i);

			if (game_totToLoad[0] == 0)
				break;

			strcpy(game_curTotFile, game_totToLoad);
		}
	}

	strcpy(game_curTotFile, savedTotName);

	inter_nestLevel = oldNestLevel;
	inter_breakFromLevel = oldBreakFrom;
	scen_pCaptureCounter = oldCaptureCounter;
	inter_execPtr = savedIP;
}

void game_start(void) {
	game_collisionAreas = (Game_Collision *)malloc(250 * sizeof(Game_Collision));
	game_prepareStart();
	game_playTot(0);

	free(game_collisionAreas);

	vid_freeSurfDesc(draw_cursorSprites);
	vid_freeSurfDesc(draw_cursorBack);
	vid_freeSurfDesc(draw_backSurface);
}

} // End of namespace Gob
