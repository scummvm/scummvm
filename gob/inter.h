/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __INTERPRET_H
#define __INTERPRET_H

namespace Gob {

extern int16 inter_animPalLowIndex;
extern int16 inter_animPalHighIndex;
extern int16 inter_animPalDir;
extern uint32 inter_soundEndTimeKey;
extern int16 inter_soundStopVal;
extern char inter_terminate;
extern char inter_breakFlag;
extern int16 *inter_breakFromLevel;
extern int16 *inter_nestLevel;

int16 inter_load16(void);
int16 inter_peek16(char *ptr);
int32 inter_peek32(char *ptr);

void inter_setMousePos(void);
char inter_evalExpr(int16 *pRes);
char inter_evalBoolResult(void);
void inter_storeResult(void);
void inter_printText(void);
void inter_animPalette(void);
void inter_animPalInit(void);
void inter_loadMult(void);
void inter_playMult(void);
void inter_freeMult(void);
void inter_initCursor(void);
void inter_initCursorAnim(void);
void inter_clearCursorAnim(void);
void inter_drawOperations(void);
void inter_getFreeMem(void);
void inter_manageDataFile(void);
void inter_getFreeMem(void);
void inter_manageDataFile(void);
void inter_writeData(void);
void inter_checkData(void);
void inter_readData(void);
void inter_loadFont(void);
void inter_freeFont(void);
void inter_prepareStr(void);
void inter_insertStr(void);
void inter_cutStr(void);
void inter_strstr(void);
void inter_setFrameRate(void);
void inter_strlen(void);
void inter_strToLong(void);
void inter_invalidate(void);
void inter_loadSpriteContent(void);
void inter_copySprite(void);
void inter_putPixel(void);
void inter_fillRect(void);
void inter_drawLine(void);
void inter_createSprite(void);
void inter_freeSprite(void);
void inter_renewTimeInVars(void);
void inter_playComposition(void);
void inter_stopSound(void);
void inter_playSound(void);
void inter_loadCursor(void);
void inter_loadSpriteToPos(void);
void inter_funcBlock(int16 retFlag);
void inter_loadTot(void);
void inter_storeKey(int16 key);
void inter_keyFunc(void);
void inter_checkSwitchTable(char **ppExec);
void inter_repeatUntil(void);
void inter_whileDo(void);
void inter_funcBlock(int16 retFlag);
void inter_callSub(int16 retFlag);
void inter_initControlVars(void);
void inter_callSub(int16 retFlag);

}				// End of namespace Gob

#endif
