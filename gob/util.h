/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __UTIL_H
#define __UTIL_H

#include "gob/video.h"

namespace Gob {

struct Util_ListNode;
typedef struct Util_ListNode {
	void *pData;
	struct Util_ListNode *pNext;
	struct Util_ListNode *pPrev;
} Util_ListNode;

typedef struct Util_List {
	Util_ListNode *pHead;
	Util_ListNode *pTail;
} Util_List;

void util_initInput(void);
void util_processInput(void);
void util_waitKey(void);
int16 util_getKey(void);
int16 util_checkKey(void);
int16 util_getRandom(int16 max);
void util_getMouseState(int16 *pX, int16 *pY, int16 *pButtons);
void util_setMousePos(int16 x, int16 y);
void util_delay(uint16 msecs);
void util_beep(int16 freq);
uint32 util_getTimeKey(void);
void util_waitMouseUp(void);
void util_waitMouseDown(void);

void keyboard_init(void);
void keyboard_release(void);

void util_clearPalette(void);

void asm_setPaletteBlock(char *tmpPalBuffer, int16 start, int16 end);

void vid_waitRetrace(int16 mode);

FontDesc *util_loadFont(const char *path);
void util_freeFont(FontDesc * fontDesc);
void util_clearPalette(void);
void util_insertStr(const char *str1, char *str2, int16 pos);
void util_cutFromStr(char *str, int16 from, int16 cutlen);
int16 util_strstr(const char *str1, char *str2);
void util_waitEndFrame(void);
void util_setFrameRate(int16 rate);

void util_listInsertBack(Util_List * list, void *data);
void util_listInsertFront(Util_List * list, void *data);
void util_listDropFront(Util_List * list);
void util_deleteList(Util_List * list);
void util_prepareStr(char *str);
void util_waitMouseRelease(char drawMouse);

}				// End of namespace Gob

#endif
