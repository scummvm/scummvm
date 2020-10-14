/** @file menuoptions.cpp
	@brief
	This file contains menu routines

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

#include "menuoptions.h"
#include "lbaengine.h"
#include "main.h"
#include "screens.h"
#include "resources.h"
#include "sdlengine.h"
#include "text.h"
#include "gamestate.h"
#include "music.h"
#include "keyboard.h"
#include "flamovies.h"
#include "scene.h"
#include "interface.h"
#include "menu.h"
#include "text.h"

/** Main menu continue game option key */
#define MAINMENU_CONTINUEGAME		21
/** Main menu enter players name */
#define MAINMENU_ENTERPLAYERNAME	42

int8 allowedCharIndex[] = " ABCDEFGHIJKLM.NOPQRSTUVWXYZ-abcdefghijklm?nopqrstuvwxyz!0123456789\040\b\r\0";


void newGame() {
	int32 tmpFlagDisplayText;

	stopMusic();

	tmpFlagDisplayText = cfgfile.FlagDisplayText;
	cfgfile.FlagDisplayText = 1;

	// intro screen 1 - twinsun
	loadImage(RESSHQR_INTROSCREEN1IMG, 1);

	newGameVar4 = 0;
	newGameVar5 = 1;

	initTextBank(2);
	textClipFull();
	setFontCrossColor(15);

	drawTextFullscreen(150);
	readKeys();

	if (skipIntro != 1) {
		// intro screen 1 - twinsun
		loadImage(RESSHQR_INTROSCREEN2IMG, 1);
		drawTextFullscreen(151);
		readKeys();

		if (skipIntro != 1) {
			loadImage(RESSHQR_INTROSCREEN3IMG, 1);
			drawTextFullscreen(152);
		}
	}

	newGameVar5 = 0;
	textClipSmall();
	newGameVar4 = 1;

	fadeToBlack(paletteRGBACustom);
	clearScreen();
	flip();

	playMidiMusic(1, 0);
	playFlaMovie(FLA_INTROD);

	clearScreen();
	flip();

	// set main palette back
	setPalette(paletteRGBA);

	cfgfile.FlagDisplayText = tmpFlagDisplayText;
}

void showCredits() {
	int32 tmpShadowMode, tmpLanguageCDIdx;

	canShowCredits = 1;
	tmpShadowMode = cfgfile.ShadowMode;
	tmpLanguageCDIdx = cfgfile.LanguageCDId;
	cfgfile.ShadowMode = 0;
	cfgfile.LanguageCDId = 0;
	initEngineVars(1);
	currentSceneIdx = 119;
	needChangeScene = 119;

	gameEngineLoop();

	canShowCredits = 0;
	cfgfile.ShadowMode = tmpShadowMode;
	cfgfile.LanguageCDId = tmpLanguageCDIdx;

	clearScreen();
	flip();

	playFlaMovie(FLA_THEEND);

	clearScreen();
	flip();
	setPalette(paletteRGBA);
}

void drawSelectableCharacter(int32 x, int32 y, int32 arg) {
	int8 buffer[256];
	int32 centerX, left, top, centerY, bottom, right, right2;

	buffer[0] = allowedCharIndex[y + x * 14];

	centerX = y * 45 + 25;
	left = centerX - 20;
	right = centerX + 20;
	top = x * 56 + 200 - 25;
	buffer[1] = 0;
	centerY = x * 56 + 200;
	bottom = x * 56 + 200 + 25;

	if (arg != 0) {
		drawSplittedBox(left, top, right, bottom, 91);
	} else {
		blitBox(left, top, right, bottom, (int8 *) workVideoBuffer, left, top, (int8 *)frontVideoBuffer);
		right2 = right;
		drawTransparentBox(left, top, right2, bottom, 4);
	}

	drawBox(left, top, right, bottom);
	right2 = right;

	setFontColor(15);
	drawText(centerX - getTextSize(buffer) / 2, centerY - 18, buffer);

	copyBlockPhys(left, top, right2, bottom);
}

void drawSelectableCharacters(void) {
	int8 x, y;

	for (x = 0; x < 5; x++) {
		for (y = 0; y < 14; y++) {
			drawSelectableCharacter(x, y, 0);
		}
	}
}

// 0001F18C
void drawPlayerName(int32 centerx, int32 top, int8* playerName, int32 type) {
/*
	int v4; // ebp@0
  int v6; // [sp+0h] [bp-14h]@0
  int v7; // [sp+0h] [bp-14h]@4
  int v8; // [sp+4h] [bp-10h]@0
  int v9; // [sp+4h] [bp-10h]@4

  LOWORD(v8) = a1 - buttonDrawVar1 / 2;
  if ( !a4 )
  {
    v6 = (signed __int16)(a2 + 25);
    blitRectangle(v4);
    drawBoxInsideTrans(v4);
  }
  if ( a4 == 1 )
  {
    makeFireEffect(v4);
    if ( !(rand(v6, v8) % 5) )
      *(_BYTE *)(10 * rand(v7, v9) % 320 + bufSpeak + 6400) = -1;
  }
  if ( a4 == 2 )
    Box(v4);
  DrawCadre();
  CoulFont(0xFu);
  SizeFont(a3);
  Font(v4);
  return CopyBlockPhys(v4);
	*/

	// TODO: implement the other types (don't seam to be used)
	/*if (type == 1) {
		processPlasmaEffect(top, 1);
	}

	drawBox(x, top, dialTextBoxRight, dialTextBoxBottom);
	drawTransparentBox(dialTextBoxLeft + 1, dialTextBoxTop + 1, dialTextBoxRight - 1, dialTextBoxBottom - 1, 3);

	setFontColor(15);
	drawText(centerX - getTextSize(playerName) / 2, top, playerName);

	copyBlockPhys(x, y, x + 320, y + 25);*/
}

int32 enterPlayerName(int32 textIdx) {
	int8 buffer[256];

	while(1) {
		copyScreen(workVideoBuffer, frontVideoBuffer);
		flip(); //frontVideoBuffer
		initTextBank(0);
		getMenuText(textIdx, buffer);
		setFontColor(15);
		drawText(320 - (getTextSize(buffer) / 2), 20, buffer);
		copyBlockPhys(0, 0, 639, 99);
		playerName[0] = enterPlayerNameVar1;
		drawPlayerName(320, 100, playerName, 1);
		drawSelectableCharacters();

		do {
			readKeys();
			do {
				readKeys();
			} while(skipIntro);
		} while(skippedKey);

		enterPlayerNameVar2 = 1;

		do {
			readKeys();
		} while(pressedKey);

		while (!skipIntro) {
			readKeys();
			// TODO
			drawPlayerName(320, 100, playerName, 1);
		}

		// FIXME: remove this lines after implementing everything
		if (skipIntro)
			break;
	}

	enterPlayerNameVar2 = 0;
	copyScreen(workVideoBuffer, frontVideoBuffer);
	flip(); // frontVideoBuffer

	return 1;
}

/** Main menu new game options */
void newGameMenu() {
	//TODO: process players name
	if(enterPlayerName(MAINMENU_ENTERPLAYERNAME))
	{
		initEngineVars(1);
		newGame();

		if (gameEngineLoop()) {
			showCredits();
		}

		copyScreen(frontVideoBuffer, workVideoBuffer);
		// TODO: recheck this
		do {
			readKeys();
			do {
				readKeys();
			} while(skippedKey != 0);
		} while(skipIntro != 0);
	}
}

/** Main menu continue game options */
void continueGameMenu() {
	//TODO: get list of saved games
	//if(chooseSave(MAINMENU_CONTINUEGAME))
	{
		initEngineVars(-1); // will load game
		if (gameChapter == 0 && currentSceneIdx == 0) {
			newGame();
		} else {
			newGameVar5 = 0;
			textClipSmall();
			newGameVar4 = 1;
		}

		if (gameEngineLoop()) {
			showCredits();
		}

		copyScreen(frontVideoBuffer, workVideoBuffer);
		// TODO: recheck this
		do {
			readKeys();
			do {
				readKeys();
			} while(skippedKey != 0);
		} while(skipIntro != 0);
	}
}
