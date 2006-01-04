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
#ifndef GOB_INTERPRET_H
#define GOB_INTERPRET_H

namespace Gob {

class Inter {
public:
	int16 _animPalLowIndex;
	int16 _animPalHighIndex;
	int16 _animPalDir;
	uint32 _soundEndTimeKey;
	int16 _soundStopVal;
	char _terminate;
	char _breakFlag;
	int16 *_breakFromLevel;
	int16 *_nestLevel;

	int16 load16(void);
	int16 peek16(char *ptr);
	int32 peek32(char *ptr);

	void setMousePos(void);
	char evalExpr(int16 *pRes);
	char evalBoolResult(void);
	void storeResult(void);
	void printText(void);
	void animPalette(void);
	void animPalInit(void);
	void loadMult(void);
	void playMult(void);
	void freeMult(void);
	void initCursor(void);
	void initCursorAnim(void);
	void clearCursorAnim(void);
	void drawOperations(void);
	void getFreeMem(void);
	void manageDataFile(void);
	void writeData(void);
	void checkData(void);
	void readData(void);
	void loadFont(void);
	void freeFont(void);
	void prepareStr(void);
	void insertStr(void);
	void cutStr(void);
	void strstr(void);
	void setFrameRate(void);
	void istrlen(void);
	void strToLong(void);
	void invalidate(void);
	void loadSpriteContent(void);
	void copySprite(void);
	void putPixel(void);
	void fillRect(void);
	void drawLine(void);
	void createSprite(void);
	void freeSprite(void);
	void renewTimeInVars(void);
	void playComposition(void);
	void stopSound(void);
	void playSound(void);
	void loadCursor(void);
	void loadSpriteToPos(void);
	void funcBlock(int16 retFlag);
	void loadTot(void);
	void storeKey(int16 key);
	void keyFunc(void);
	void checkSwitchTable(char **ppExec);
	void repeatUntil(void);
	void whileDo(void);
	void callSub(int16 retFlag);
	void initControlVars(void);

	Inter(GobEngine *vm);

protected:
	GobEngine *_vm;

	void evaluateStore(void);
	void capturePush(void);
	void capturePop(void);
};

}				// End of namespace Gob

#endif
