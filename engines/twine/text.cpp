/** @file text.cpp
	@brief
	This file contains dialogues processing.
	It contains text and font functions.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <math.h>

#include "text.h"
#include "main.h"
#include "hqrdepack.h"
#include "resources.h"
#include "sdlengine.h"
#include "menu.h"
#include "interface.h"
#include "lbaengine.h"
#include "keyboard.h"
#include "screens.h"
#include "renderer.h"
#include "sound.h"

// RECHECK THIS LATER
int32 currentBankIdx = -1; // textVar1
uint8 textVar2[256];
uint8 textVar3;

/** Dialogue text pointer */
uint8 *dialTextPtr;  // bufText
/** Dialogue entry order pointer */
uint8 *dialOrderPtr; // bufOrder
/** Number of dialogues text entries */
int16 numDialTextEntries;

// TODO: refactor this
int32 wordSizeChar;
int32 wordSizePixel;

int16 spaceChar = 0x20;

/** FLA movie extension */
#define VOX_EXT ".vox"
/** Common movie directory */
#define VOX_DIR "vox/"

int8 * LanguagePrefixTypes[] = {
	"EN_",
	"FR_",
	"DE_",
	"SP_",
	"IT_"
};

int8 * LanguageSufixTypes[] = {
	"sys",
	"cre",
	"gam",
	"000",
	"001",
	"002",
	"003",
	"004",
	"005",
	"006",
	"007",
	"008",
	"009",
	"010",
	"011"
};


void initVoxBank(int32 bankIdx) {
	// get the correct vox hqr file
	memset(currentVoxBankFile, 0, sizeof(int8));
	sprintf(currentVoxBankFile, VOX_DIR);
	strcat(currentVoxBankFile, LanguagePrefixTypes[cfgfile.LanguageId]);
	strcat(currentVoxBankFile, LanguageSufixTypes[bankIdx]);
	strcat(currentVoxBankFile, VOX_EXT);

	// TODO check the rest to reverse
}


int32 initVoxToPlay(int32 index) { // setVoxFileAtDigit
	int32 i = 0;
	int32 currIdx = 0;
	int32 orderIdx = 0;

	int16 *localOrderBuf = (int16 *) dialOrderPtr;

	voxHiddenIndex = 0;
	hasHiddenVox = 0;

	// choose right text from order index
	for (i = 0; i < numDialTextEntries; i++) {
		orderIdx = *(localOrderBuf++);
		if (orderIdx == index) {
			currIdx = i;
			break;
		}
	}

	currDialTextEntry = currIdx;

	playVoxSample(currDialTextEntry);

	return 1;
}

int32 playVox(int32 index) {
	if (cfgfile.LanguageCDId && index) {
		if (hasHiddenVox && !isSamplePlaying(index)) {
			playVoxSample(index);
			return 1;
		}
	}

	return 0;
}

int32 playVoxSimple(int32 index) {
	if (cfgfile.LanguageCDId && index) {
		playVox(index);

		if (isSamplePlaying(index)) {
			return 1;
		}
	}

	return 0;
}

void stopVox(int32 index) {
	hasHiddenVox = 0;
	stopSample(index);
}



/** Initialize dialogue
	@param bankIdx Text bank index*/
void initTextBank(int32 bankIdx) { // InitDial
	int32 langIdx;
	int32 hqrSize;

	// don't load if we already have the dialogue text bank loaded
	if (bankIdx == currentBankIdx)
		return;

	currentBankIdx = bankIdx;
	// RECHECK THIS LATER
	textVar2[0] = textVar3;

	// get index according with language
	langIdx = (cfgfile.LanguageId * 14) * 2  + bankIdx * 2;

	hqrSize = hqrGetallocEntry(&dialOrderPtr, HQR_TEXT_FILE, langIdx);

	numDialTextEntries = hqrSize / 2;

	hqrSize = hqrGetallocEntry(&dialTextPtr, HQR_TEXT_FILE, ++langIdx);

	if (cfgfile.LanguageCDId) {
		initVoxBank(bankIdx);
	}
}

/** Draw a certain character in the screen
	@param x X coordinate in screen
	@param y Y coordinate in screen
	@param character ascii character to display */
void drawCharacter(int32 x, int32 y, uint8 character) { // drawCharacter
	uint8 sizeX;
	uint8 sizeY;
	uint8 param1;
	uint8 param2;
	uint8 *data;
	uint8 *screen;

	// int temp=0;
	int32 toNextLine;
	uint8 index;

	// char color;
	uint8 usedColor;
	uint8 number;
	uint8 jump;

	int32 i;

	int32 tempX;
	int32 tempY;

	data = fontPtr + *((int16 *)(fontPtr + character * 4));

	dialTextSize = sizeX = *(data++);
	sizeY = *(data++);

	param1 = *(data++);
	param2 = *(data++);

	x += param1;
	y += param2;

	usedColor = dialTextColor;

	screen = frontVideoBuffer + screenLookupTable[y] + x;

	tempX = x;
	tempY = y;

	toNextLine = SCREEN_WIDTH - sizeX;

	do {
		index = *(data++);
		do {
			jump = *(data++);
			screen += jump;
			tempX += jump;
			if (--index == 0) {
				screen += toNextLine;
				tempY++;
				tempX = x;
				sizeY--;
				if (sizeY <= 0) {
					return;
				}
				break;
			} else {
				number = *(data++);
				for (i = 0; i < number; i++) {
					if (tempX >= SCREEN_TEXTLIMIT_LEFT && tempX < SCREEN_TEXTLIMIT_RIGHT && tempY >= SCREEN_TEXTLIMIT_TOP && tempY < SCREEN_TEXTLIMIT_BOTTOM)
						frontVideoBuffer[SCREEN_WIDTH*tempY + tempX] = usedColor;

					screen++;
					tempX++;
				}

				if (--index == 0) {
					screen += toNextLine;
					tempY++;
					tempX = x;

					sizeY--;
					if (sizeY <= 0) {
						return;
					}
					break;
				}
			}
		} while (1);
	} while (1);

}

/** Draw character with shadow
	@param x X coordinate in screen
	@param y Y coordinate in screen
	@param character ascii character to display
	@param color character color */
void drawCharacterShadow(int32 x, int32 y, uint8 character, int32 color) { // drawDoubleLetter
	int32 left, top, right, bottom;

	if (character != 0x20)
	{
		// shadow color
		setFontColor(0);
		drawCharacter(x + 2, y + 4, character);

		// text color
		setFontColor(color);
		drawCharacter(x, y, character);

		left = x;
		top = y;
		// FIXME: get right font size
		right = x + 32;
		bottom = y + 38;

		copyBlockPhys(left, top, right, bottom);
	}
}

/** Display a certain dialogue text in the screen
	@param x X coordinate in screen
	@param y Y coordinate in screen
	@param dialogue ascii text to display */
void drawText(int32 x, int32 y, int8 *dialogue) { // Font
	uint8 currChar;

	if (fontPtr == 0)   // if the font is not defined
		return;

	do {
		currChar = (uint8) * (dialogue++); // read the next char from the string

		if (currChar == 0) // if the char is 0x0, -> end of string
			break;

		if (currChar == 0x20)  // if it's a space char
			x += dialCharSpace;
		else {
			dialTextSize = *(fontPtr + (*((int16 *)(fontPtr + currChar * 4))));  // get the length of the character
			drawCharacter(x, y, currChar); // draw the character on screen
			// add the length of the space between 2 characters
			x += dialSpaceBetween;
			// add the length of the current character
			x += dialTextSize;
		}
	} while (1);
}

/** Gets dialogue text width size
	@param dialogue ascii text to display */
int32 getTextSize(int8 *dialogue) {  // SizeFont
	uint8 currChar;
	dialTextSize = 0;

	do {
		currChar = (uint8) * (dialogue++);

		if (currChar == 0)
			break;

		if (currChar == 0x20) {
			dialTextSize += dialCharSpace;
		} else {
			dialTextSize += dialSpaceBetween;
			dialTextSize += *(fontPtr + *((int16 *)(fontPtr + currChar * 4)));
		}
	} while (1);

	return (dialTextSize);
}

void initDialogueBox() { // InitDialWindow
	blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (int8*)workVideoBuffer, dialTextBoxLeft, dialTextBoxTop, (int8*)frontVideoBuffer);

	if (newGameVar4 != 0) {
		drawBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
		drawTransparentBox(dialTextBoxLeft + 1, dialTextBoxTop + 1, dialTextBoxRight - 1, dialTextBoxBottom - 1, 3);
	}

	copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
	printText8Var3 = 0;
	blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (int8*)frontVideoBuffer, dialTextBoxLeft, dialTextBoxTop, (int8*)workVideoBuffer);
}

void initInventoryDialogueBox() { // SecondInitDialWindow
	blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (int8*)workVideoBuffer, dialTextBoxLeft, dialTextBoxTop, (int8*)frontVideoBuffer);
	copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
	printText8Var3 = 0;
}

// TODO: refactor this code
void initText(int32 index) { // initText
	printTextVar13 = 0;

	if (!getText(index)) {
		return;
	}

	printText8Ptr1 = buf1;
	printText8Ptr2 = buf2;

	printTextVar13 = 1;

	printText8Var1 = 0;
	buf1[0] = 0;
	buf2[0] = 0;
	printText8Var2 = index;
	printText8Var3 = 0;
	TEXT_CurrentLetterX = dialTextBoxLeft + 8;
	printText8Var5 = 0;
	printText8Var6 = 0;
	TEXT_CurrentLetterY = dialTextBoxTop + 8;
	printText8Var8 = currDialTextPtr;

	// lba font is get while engine start
	setFontParameters(2, 7);
}

void initProgressiveTextBuffer() {
	int32 i = 0;

	buf2[0] = 0;

	while (i < dialTextBufferSize) {
		strcat((char*)buf2, " ");
		i++;
	};

	printText8Ptr2 = buf2;
	addLineBreakX = 16;
	printText8Var1 = 0;
}

void printText8Sub4(int16 a, int16 b, int16 c) {
	int32 temp;
	int32 counter2 = 0;
	int32 var1;
	int32 var2;

	if (printText8Var3 < 32) {
		temp = printText8Var3 * 3;
		pt8s4[temp] = c;
		pt8s4[temp+1] = a;
		pt8s4[temp+2] = b;

		printText8Var3++;
	} else {
		while (counter2 < 31) {
			var1 = (counter2 + 1) * 3;
			var2 = counter2 * 3;
			pt8s4[var2] = pt8s4[var1];
			pt8s4[var2+1] = pt8s4[var1+1];
			pt8s4[var2+2] = pt8s4[var1+2];
			counter2++;
		};
		pt8s4[93] = c;
		pt8s4[94] = a;
		pt8s4[95] = b;
	}
}

void getWordSize(uint8 *arg1, uint8 *arg2) {
	int32 temp = 0;
	uint8 *arg2Save = arg2;

	while (*arg1 != 0 && *arg1 != 1 && *arg1 != 0x20) {
		temp++;
		*arg2++ = *arg1++;
	}

	wordSizeChar = temp;
	*arg2 = 0;
	wordSizePixel = getTextSize((int8*)arg2Save);
}

void processTextLine() {
	int16 var4;
	uint8 *buffer;
	uint8 *temp;

	buffer = printText8Var8;
	dialCharSpace = 7;
	var4 = 1;

	addLineBreakX = 0;
	printText8PrepareBufferVar2 = 0;
	buf2[0] = 0;

	for (;;)
	{
		if (*buffer == 0x20) {
			buffer++;
			continue;
		}

		if (*buffer != 0) {
			printText8Var8 = buffer;
			getWordSize(buffer, buf1);
			if (addLineBreakX + dialCharSpace + wordSizePixel < dialTextBoxParam2) {
				temp = buffer + 1;
				if (*buffer == 1) {
					var4 = 0;
					buffer = temp;
				} else {
					if (*buf1 == 0x40) {
						var4 = 0;
						buffer = temp;
						if (addLineBreakX == 0) {
							addLineBreakX = 7;
							*((int16 *)buf2) = spaceChar;
						}
						if (buf1[1] == 0x50) {
							printText8Var1 = dialTextBoxParam1;
							buffer++;
						}
					} else {
						buffer += wordSizeChar;
						printText8Var8 = buffer;
						strcat((char*)buf2, (char*)buf1);
						strcat((char*)buf2, " ");  // not 100% accurate
						printText8PrepareBufferVar2++;

						addLineBreakX += wordSizePixel + dialCharSpace;
						if (*printText8Var8 != 0) {
							printText8Var8++;
							continue;
						}
					}
				}
			}
		}
		break;
	}

	if (printText8PrepareBufferVar2 != 0)
		printText8PrepareBufferVar2--;

	if (*printText8Var8 != 0 && var4 == 1) {
		dialCharSpace += (dialTextBoxParam2 - addLineBreakX) / printText8PrepareBufferVar2;
		printText10Var1 = dialTextBoxParam2 - addLineBreakX - dialTextBoxParam2 - addLineBreakX;  // stupid... recheck
	}

	printText8Var8 = buffer;

	printText8Ptr2 = buf2;

}

// draw next page arrow polygon
void printText10Sub() { // printText10Sub()
	vertexCoordinates[0] = dialTextStopColor;
	vertexCoordinates[1] = dialTextBoxRight - 3;
	vertexCoordinates[2] = dialTextBoxBottom - 24;
	vertexCoordinates[3] = dialTextStopColor;
	vertexCoordinates[4] = dialTextBoxRight - 24;
	vertexCoordinates[5] = dialTextBoxBottom - 3;
	vertexCoordinates[6] = dialTextStartColor;
	vertexCoordinates[7] = vertexCoordinates[1];
	vertexCoordinates[8] = vertexCoordinates[5];

	polyRenderType = 0; // POLYGONTYPE_FLAT
	numOfVertex = 3;

	if (computePolygons())
	{
		renderPolygons(polyRenderType, dialTextStopColor);
	}

	copyBlockPhys(dialTextBoxRight - 24, dialTextBoxBottom - 24, dialTextBoxRight - 3, dialTextBoxBottom - 3);
}

void printText10Sub2() { // printText10Sub2()
	int32 currentLetter;
	int32 currentIndex;
	int32 counter;
	int32 counter2;
	int16 *ptr;

	currentLetter = printText8Var3;
	currentLetter--;

	currentIndex = currentLetter * 3;

	ptr = pt8s4 + currentIndex;

	sdldelay(15);

	counter = printText8Var3;
	counter2 = dialTextStartColor;

	while (--counter >= 0) {
		setFontColor(counter2);
		drawCharacterShadow(*(ptr + 1), *(ptr + 2), (uint8)*ptr, counter2);
		counter2 -= dialTextStepSize;
		if (counter2 > dialTextStopColor)
			counter2 = dialTextStopColor;
		ptr -= 3;
	};

}

void TEXT_GetLetterSize(uint8 character, int32 *pLetterWidth, int32 *pLetterHeight, uint8 * pFont) { // TEXT_GetLetterSize
	uint8 *temp;

	temp = (uint8*) (pFont + *((int16 *)(pFont + character * 4)));
	*pLetterWidth = *(temp);
	*pLetterHeight = *(temp + 1);
}

// TODO: refactor this code
int printText10() { // printText10()
	int32 charWidth, charHeight; // a, b

	if (printTextVar13 == 0) {
		return 0;
	}

	if (*(printText8Ptr2) == 0) {
		if (printText8Var5 != 0) {
			if (newGameVar5 != 0) {
				printText10Sub();
			}
			printTextVar13 = 0;
			return 0;
		}
		if (printText8Var6 != 0) {
			blitBox(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom, (int8*)workVideoBuffer, dialTextBoxLeft, dialTextBoxTop, (int8*)frontVideoBuffer);
			copyBlockPhys(dialTextBoxLeft, dialTextBoxTop, dialTextBoxRight, dialTextBoxBottom);
			printText8Var3 = 0;
			printText8Var6 = 0;
			TEXT_CurrentLetterX = dialTextBoxLeft + 8;
			TEXT_CurrentLetterY = dialTextBoxTop + 8;
		}
		if (*(printText8Var8) == 0) {
			initProgressiveTextBuffer();
			printText8Var5 = 1;
			return 1;
		}
		processTextLine();
	}

	// RECHECK this later
	if (*(printText8Ptr2) == 0) {
		return 1;
	}

	printText8Sub4(TEXT_CurrentLetterX, TEXT_CurrentLetterY, *printText8Ptr2);
	printText10Sub2();
	TEXT_GetLetterSize(*printText8Ptr2, &charWidth, &charHeight, (uint8*)fontPtr);

	if (*(printText8Ptr2) != 0x20) {
		TEXT_CurrentLetterX += charWidth + 2;
	} else {
		if (printText10Var1 != 0) {
			TEXT_CurrentLetterX++;
			printText10Var1--;
		}
		TEXT_CurrentLetterX += dialCharSpace;
	}

	// next character
	printText8Ptr2++;

	if (*(printText8Ptr2) != 0)
		return 1;

	TEXT_CurrentLetterY += 38;
	TEXT_CurrentLetterX = dialTextBoxLeft + 8;

	if (printText8Var6 == 1 && printText8Var5 == 0) {
	  printText10Sub();
	  return 2;
	}

	printText8Var1++;
	if (printText8Var1 < dialTextBoxParam1) {
		return 1;
	}

	initProgressiveTextBuffer();
	printText8Var6 = 1;

	if (*(printText8Var8) == 0) {
		printText8Var5 = 1;
	}

	return 1;
}

// TODO: refactor this code
void drawTextFullscreen(int32 index) { // printTextFullScreen
	int32 printedText;
	int32 skipText = 0;

	saveClip();
	resetClip();
	copyScreen(frontVideoBuffer, workVideoBuffer);

	// get right VOX entry index
	if (cfgfile.LanguageCDId) {
		initVoxToPlay(index);
	}

	// if we don't display text, than still plays vox file
	if (cfgfile.FlagDisplayText) {
		initText(index);
		initDialogueBox();

		do {
			readKeys();
			printedText = printText10();
			playVox(currDialTextEntry);

			if (printedText == 2) {
				do {
					readKeys();
					if (skipIntro == 0 && skippedKey == 0 && pressedKey == 0) {
						break;
					}
					playVox(currDialTextEntry);
					sdldelay(1);
				} while(1);

				do {
					readKeys();
					if (skipIntro != 0 || skippedKey != 0 || pressedKey != 0) {
						break;
					}
					playVox(currDialTextEntry);
					sdldelay(1);
				} while(1);
			}

			if (skipIntro == 1) {
				skipText = 1;
			}

			if (!printedText && !isSamplePlaying(currDialTextEntry)) {
				break;
			}

			sdldelay(1);
		} while(!skipText);

		hasHiddenVox = 0;

		if (cfgfile.LanguageCDId && isSamplePlaying(currDialTextEntry)) {
			stopVox(currDialTextEntry);
		}

		printTextVar13 = 0;

		if (printedText != 0) {
			loadClip();
			return;
		}

		if (skipText != 0) {
			loadClip();
			return;
		}

		// RECHECK this later
		// wait displaying text
		do {
			readKeys();
			sdldelay(1);
		} while(skipIntro || skippedKey || pressedKey);

		// RECHECK this later
		// wait key to display next text
		do {
			readKeys();
			if (skipIntro != 0) {
				loadClip();
				return;
			}
			if (skippedKey != 0) {
				loadClip();
				return;
			}
			sdldelay(1);
		} while(!pressedKey);
	} else { // RECHECK THIS
		while (playVox(currDialTextEntry) && skipIntro != 1 );
		hasHiddenVox = 0;
		voxHiddenIndex = 0;
	}

	if (cfgfile.LanguageCDId && isSamplePlaying(currDialTextEntry)) {
		stopVox(currDialTextEntry);
	}

	loadClip();
}

void setFont(uint8 *font, int32 spaceBetween, int32 charSpace) {
	fontPtr = font;
	dialCharSpace = charSpace;
	dialSpaceBetween = spaceBetween;
}

/** Set font type parameters
	@param spaceBetween number in pixels of space between characters
	@param charSpace number in pixels of the character space */
void setFontParameters(int32 spaceBetween, int32 charSpace) {
	dialSpaceBetween = spaceBetween;
	dialCharSpace = charSpace;
}

/** Set the font cross color
	@param color color number to choose */
void setFontCrossColor(int32 color) { // TestCoulDial
	dialTextStepSize = -1;
	dialTextBufferSize = 14;
	dialTextStartColor = color << 4;
	dialTextStopColor = (color << 4) + 12;
}

/** Set the font color
	@param color color number to choose */
void setFontColor(int32 color) {
	dialTextColor = color;
}

/** Set font color parameters to process cross color display
	@param stopColor color number to stop
	@param startColor color number to start
	@param stepSize step size to change between those colors */
void setTextCrossColor(int32 stopColor, int32 startColor, int32 stepSize) {
	dialTextStartColor = startColor;
	dialTextStopColor = stopColor;
	dialTextStepSize = stepSize;
	dialTextBufferSize = ((startColor - stopColor) + 1) / stepSize;
}

/** Get dialogue text into text buffer
	@param index dialogue index */
int32 getText(int32 index) { // findString
	int32 currIdx = 0;
	int32 orderIdx = 0;
	int32 numEntries;
	int32 ptrCurrentEntry;
	int32 ptrNextEntry;

	int16 *localTextBuf = (int16 *) dialTextPtr;
	int16 *localOrderBuf = (int16 *) dialOrderPtr;

	numEntries = numDialTextEntries;

	// choose right text from order index
	do {
		orderIdx = *(localOrderBuf++);
		if (orderIdx == index)
			break;
		currIdx++;
	} while (currIdx < numDialTextEntries);

	if (currIdx >= numEntries)
		return 0;

	ptrCurrentEntry = localTextBuf[currIdx];
	ptrNextEntry = localTextBuf[currIdx + 1];

	currDialTextPtr = (dialTextPtr + ptrCurrentEntry);
	currDialTextSize = ptrNextEntry - ptrCurrentEntry;
	numDialTextEntries = numEntries;

	// RECHECK: this was added for vox playback
	currDialTextEntry = currIdx;

	return 1;
}

/** Copy dialogue text
	@param src source text buffer
	@param dst destination text buffer
	@param size text size */
void copyText(int8 *src, int8 *dst, int32 size) { // copyStringToString
	int32 i;
	for (i = 0; i < size; i++)
		*(dst++) = *(src++);
}

/** Gets menu dialogue text
	@param index text index to display
	@param dialogue dialogue text buffer to display */
void getMenuText(int32 index, int8 *text) { // GetMultiText
	if (index == currMenuTextIndex) {
		if (currMenuTextBank == currentTextBank) {
			strcpy(text, currMenuTextBuffer);
			return;
		}
	}
	if (!getText(index)) {
		// if doesn't have text
		text[0] = 0;
		return;
	}

	if ((currDialTextSize - 1) > 0xFF)
		currDialTextSize = 0xFF;

	copyText((int8 *) currDialTextPtr, text, currDialTextSize);
	currDialTextSize++;
	copyText(text, currMenuTextBuffer, currDialTextSize);

	currMenuTextIndex = index;
	currMenuTextBank = currentTextBank;
}

void textClipFull() { // newGame2
	dialTextBoxLeft = 8;
	dialTextBoxTop = 8;
	dialTextBoxRight = 631;

	dialTextBoxBottom = 471;
	dialTextBoxParam1 = 11;
	dialTextBoxParam2 = 607;
}

void textClipSmall() { // newGame4
	dialTextBoxLeft = 16;
	dialTextBoxTop = 334;
	dialTextBoxRight = 623;
	dialTextBoxBottom = 463;
	dialTextBoxParam1 = 3;
	dialTextBoxParam2 = 591;
}

void drawAskQuestion(int32 index) { // MyDial
	int32 textStatus = 1;

	// get right VOX entry index
	if (cfgfile.LanguageCDId) {
		initVoxToPlay(index);
	}

	initText(index);
	initDialogueBox();

	do {
		readKeys();
		textStatus = printText10();

		if (textStatus == 2) {
			do {
				readKeys();
				playVox(currDialTextEntry);
				sdldelay(1);
			} while(skipIntro || skippedKey || pressedKey);

			do {
				readKeys();
				playVox(currDialTextEntry);
				sdldelay(1);
			} while(!skipIntro && !skippedKey && !pressedKey);
		}

		sdldelay(1);
	} while(textStatus);

	if (cfgfile.LanguageCDId) {
		while(playVoxSimple(currDialTextEntry));

		hasHiddenVox = 0;
		voxHiddenIndex = 0;

		if(isSamplePlaying(currDialTextEntry)) {
			stopVox(currDialTextEntry);
		}
	}

	printTextVar13 = 0;
}
