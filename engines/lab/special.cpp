/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/translation.h"

#include "gui/message.h"
#include "gui/saveload.h"

#include "lab/lab.h"
#include "lab/labfun.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/diff.h"
#include "lab/vga.h"
#include "lab/text.h"
#include "lab/mouse.h"
#include "lab/timing.h"
#include "lab/stddefines.h"
#include "lab/parsetypes.h"

namespace Lab {

#ifdef GAME_TRIAL
int g_IsRegistered;
#endif

extern bool nopalchange, DoBlack, IsHiRes;

extern BitMap *DispBitMap, *DrawBitMap;
extern char diffcmap[3 * 256];

extern uint32 VGAScreenWidth, VGAScreenHeight;


#define COMBINATIONUNLOCKED  130
#define BRICKOPEN            115


static uint16 hipal[20];
extern uint16 *FadePalette;

#define INCL(BITSET,BIT) ((BITSET) |= (BIT))

#define SETBIT(BITSET,BITNUM)   INCL(BITSET, (1 << (BITNUM)))

#define INBIT(BITSET,BITNUM)    ( ((1 << (BITNUM)) & (BITSET)) > 0 )


static byte *loadBackPict(const char *fileName, bool tomem) {
	uint16 counter;
	byte *res = NULL;

	FadePalette = hipal;
	nopalchange = true;

	if (tomem)
		res = readPictToMem(fileName, VGAScreenWidth, VGAScreenHeight);
	else
		readPict(fileName, true);

	for (counter = 0; counter < 16; counter++) {
		hipal[counter] = ((diffcmap[counter * 3] >> 2) << 8) +
		                 ((diffcmap[counter * 3 + 1] >> 2) << 4) +
		                 ((diffcmap[counter * 3 + 2] >> 2));
	}

	nopalchange = false;

	return res;
}



/*----------------------------------------------------------------------------*/
/*-------------------------- Combination Lock Rules --------------------------*/
/*----------------------------------------------------------------------------*/




static Image *Images[10];


byte combination[6] = {0, 0, 0, 0, 0, 0}, solution[] = {0, 4, 0, 8, 7, 2};

static uint16 combx[] = {45, 83, 129, 166, 211, 248};



/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
static void doCombination(void) {
	uint16 counter;

	for (counter = 0; counter <= 5; counter++)
		drawImage(Images[combination[counter]], VGAScaleX(combx[counter]), VGAScaleY(65));
}


extern byte *TempScrollData;

/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void showCombination(const char *filename) {
	uint16 CurBit;
	byte **buffer;

	resetBuffer();
	DoBlack = true;
	nopalchange = true;
	readPict(filename, true);
	nopalchange = false;

	blackScreen();

	buffer = g_music->newOpen("P:Numbers");

	for (CurBit = 0; CurBit < 10; CurBit++)
		readImage(buffer, &(Images[CurBit]));

	allocFile((void **)&TempScrollData, Images[0]->Width * Images[0]->Height * 2L, "tempdata");

	doCombination();

	VGASetPal(diffcmap, 256);
}



/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
static void changeCombination(uint16 number) {
	Image display;
	uint16 counter, combnum;
	bool unlocked = true;

	if (combination[number] < 9)
		(combination[number])++;
	else
		combination[number] = 0;

	combnum = combination[number];

	display.ImageData = getVGABaseAddr();
	display.Width     = VGAScreenWidth;
	display.Height    = VGAScreenHeight;

	/* NYI:
	   readPict("Music:Thunk", true);
	 */

	for (counter = 1; counter <= (Images[combnum]->Height / 2); counter++) {
		if (IsHiRes) {
			if (counter & 1)
				waitTOF();
		} else
			waitTOF();

		display.ImageData = getVGABaseAddr();

		scrollDisplayY(2, VGAScaleX(combx[number]), VGAScaleY(65), VGAScaleX(combx[number]) + (Images[combnum])->Width - 1, VGAScaleY(65) + (Images[combnum])->Height);

		bltBitMap(Images[combnum], 0, (Images[combnum])->Height - (2 * counter), &(display), VGAScaleX(combx[number]), VGAScaleY(65), (Images[combnum])->Width, 2);
	}

	for (counter = 0; counter < 6; counter++)
		unlocked = (combination[counter] == solution[counter]) && unlocked;

	if (unlocked)
		g_lab->_conditions->inclElement(COMBINATIONUNLOCKED);
	else
		g_lab->_conditions->exclElement(COMBINATIONUNLOCKED);

	ungetVGABaseAddr();
}





/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseCombination(uint16 x, uint16 y) {
	uint16 number;

	x = VGAUnScaleX(x);
	y = VGAUnScaleY(y);

	if ((y >= 63) && (y <= 99)) {
		if ((x >= 44) && (x < 83))
			number = 0;
		else if (x < 127)
			number = 1;
		else if (x < 165)
			number = 2;
		else if (x < 210)
			number = 3;
		else if (x < 245)
			number = 4;
		else if (x < 286)
			number = 5;
		else
			return;

		changeCombination(number);
	}
}



/*----------------------------------------------------------------------------*/
/*----------------------------- Tile Puzzle Rules ----------------------------*/
/*----------------------------------------------------------------------------*/

Image *Tiles[16];
uint16 CurTile[4] [4] = {
	{ 1, 5,  9, 13 },
	{ 2, 6, 10, 14 },
	{ 3, 7, 11, 15 },
	{ 4, 8, 12,  0 }
}, TileSolution[4] [4] = {
	{ 7,   1,  8,  3 },
	{ 2,  11, 15,  4 },
	{ 9,   5, 14,  6 },
	{ 10, 13, 12,  0}
};


/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
static void doTile(bool showsolution) {
	uint16 row = 0, col = 0, rowm, colm, num;
	int16 rows, cols;

	if (showsolution) {
		rowm = VGAScaleY(23);
		colm = VGAScaleX(27);

		rows = VGAScaleY(31);
		cols = VGAScaleX(105);
	} else {
		setAPen(0);
		rectFill(VGAScaleX(97), VGAScaleY(22), VGAScaleX(220), VGAScaleY(126));

		rowm = VGAScaleY(25);
		colm = VGAScaleX(30);

		rows = VGAScaleY(25);
		cols = VGAScaleX(100);
	}

	while (row < 4) {
		while (col < 4) {
			if (showsolution)
				num = TileSolution[col] [row];
			else
				num = CurTile[col] [row];

			if (showsolution || num)
				drawImage(Tiles[num], cols + (col * colm), rows + (row * rowm));

			col++;
		}

		row++;
		col = 0;
	}
}




/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void showTile(const char *filename, bool showsolution) {
	uint16 CurBit, start;
	byte **buffer;

	resetBuffer();
	DoBlack = true;
	nopalchange = true;
	readPict(filename, true);
	nopalchange = false;
	blackScreen();

	if (showsolution) {
		start  = 0;
		buffer = g_music->newOpen("P:TileSolution");
	} else {
		start  = 1;
		buffer = g_music->newOpen("P:Tile");
	}

	if (!buffer)
		return;

	for (CurBit = start; CurBit < 16; CurBit++)
		readImage(buffer, &(Tiles[CurBit]));

	allocFile((void **)&TempScrollData, Tiles[1]->Width * Tiles[1]->Height * 2L, "tempdata");

	doTile(showsolution);

	VGASetPal(diffcmap, 256);
}



#define LEFTSCROLL     1
#define RIGHTSCROLL    2
#define UPSCROLL       3
#define DOWNSCROLL     4




static void scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if (dx)
		scrollDisplayX(dx, x1, y1, x2, y2);

	if (dy)
		scrollDisplayY(dy, x1, y1, x2, y2);
}




/*****************************************************************************/
/* Does the scrolling for the tiles on the tile puzzle.                      */
/*****************************************************************************/
static void doTileScroll(uint16 col, uint16 row, uint16 scrolltype) {
	int16 dX = 0, dY = 0, dx = 0, dy = 0, sx = 0, sy = 0;
	uint16 last = 0, x1, y1;
	uint16 counter;

	if (scrolltype == LEFTSCROLL) {
		dX =  VGAScaleXs(5);
		sx =  VGAScaleXs(5);
		last = 6;
	} else if (scrolltype == RIGHTSCROLL) {
		dX = VGAScaleXs(-5);
		dx = VGAScaleXs(-5);
		sx =  VGAScaleX(5);
		last = 6;
	} else if (scrolltype == UPSCROLL) {
		dY =  VGAScaleYs(5);
		sy =  VGAScaleYs(5);
		last = 5;
	} else if (scrolltype == DOWNSCROLL) {
		dY = VGAScaleYs(-5);
		dy = VGAScaleYs(-5);
		sy =  VGAScaleYs(5);
		last = 5;
	}

	sx += SVGACord(2);

	x1 = VGAScaleX(100) + (col * VGAScaleX(30)) + dx;
	y1 = VGAScaleY(25) + (row * VGAScaleY(25)) + dy;

	for (counter = 0; counter < last; counter++) {
		waitTOF();
		scrollRaster(dX, dY, x1, y1, x1 + VGAScaleX(28) + sx, y1 + VGAScaleY(23) + sy);
		x1 += dX;
		y1 += dY;
	}
}



/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
static void changeTile(uint16 col, uint16 row) {
	bool check;
	int16 scrolltype = -1;

	if (row > 0) {
		if (CurTile[col] [row - 1] == 0) {
			CurTile[col] [row - 1] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = DOWNSCROLL;
		}
	}

	if (col > 0) {
		if (CurTile[col - 1] [row] == 0) {
			CurTile[col - 1] [row] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = RIGHTSCROLL;
		}
	}

	if (row < 3) {
		if (CurTile[col] [row + 1] == 0) {
			CurTile[col] [row + 1] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = UPSCROLL;
		}
	}

	if (col < 3) {
		if (CurTile[col + 1] [row] == 0) {
			CurTile[col + 1] [row] = CurTile[col] [row];
			CurTile[col] [row] = 0;
			scrolltype = LEFTSCROLL;
		}
	}

	if (scrolltype != -1) {
		/* NYI:
		    readPict("Music:Click", true);
		 */
		doTileScroll(col, row, scrolltype);

#if defined(LABDEMO)
		return;
#endif

#if defined(GAME_TRIAL)

		if (!g_IsRegistered)
			return;

#endif

		check = true;
		row   = 0;
		col   = 0;

		while (row < 4) {
			while (col < 4) {
				check = check && (CurTile[row] [col] == TileSolution[row] [col]);
				col++;
			}

			row++;
			col = 0;
		}

		if (check) {
			g_lab->_conditions->inclElement(BRICKOPEN);  /* unlocked combination */
			DoBlack = true;
			check = readPict("p:Up/BDOpen", true);
		}
	}
}





/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseTile(uint16 x, uint16 y) {
	x = VGAUnScaleX(x);
	y = VGAUnScaleY(y);

	if ((x < 101) || (y < 26))
		return;

	x = (x - 101) / 30;
	y = (y -  26) / 25;

	if ((x < 4) && (y < 4))
		changeTile(x, y);
}


/*---------------------------------------------------------------------------*/
/*------------------------ Does the detective notes. ------------------------*/
/*---------------------------------------------------------------------------*/

extern TextFont *MsgFont;
static TextFont *BigMsgFont;
static TextFont bmfont;


/*****************************************************************************/
/* Does the things to properly set up the detective notes.                   */
/*****************************************************************************/
void doNotes(void) {
	char *ntext;

	/* Load in the data */
	BigMsgFont = &bmfont;

	if (!getFont("P:Note.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return;
	}

	if ((ntext = getText("Lab:Rooms/Notes")) == NULL)
		return;

	flowText(BigMsgFont, -2 + SVGACord(1), 0, 0, false, false, true, true, VGAScaleX(25) + SVGACord(15), VGAScaleY(50), VGAScaleX(295) - SVGACord(15), VGAScaleY(148), ntext);

	VGASetPal(diffcmap, 256);
	freeAllStolenMem();
}




/*---------------------------------------------------------------------------*/
/*---------------------- Does the Old West newspaper.  ----------------------*/
/*---------------------------------------------------------------------------*/



/*****************************************************************************/
/* Does the things to properly set up the old west newspaper.  Assumes that  */
/* OpenHiRes already called.                                                 */
/*****************************************************************************/
void doWestPaper(void) {
	char *ntext;
	int32 FileLen, CharsPrinted;
	uint16 y = 268;

	BigMsgFont = &bmfont;

	if (!getFont("P:News22.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return;
	}

	if ((ntext = getText("Lab:Rooms/Date")) == NULL)
		return;

	flowText(BigMsgFont, 0, 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(77) + SVGACord(2), VGAScaleX(262), VGAScaleY(91), ntext);

	BigMsgFont = &bmfont;

	if (!getFont("P:News32.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return;
	}

	if ((ntext = getText("Lab:Rooms/Headline")) == NULL)
		return;

	FileLen = strlen(ntext) - 1;
	CharsPrinted = flowText(BigMsgFont, -8, 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(86) - SVGACord(2), VGAScaleX(262), VGAScaleY(118), ntext);

	if (CharsPrinted < FileLen) {
		y = 130 - SVGACord(5);
		flowText(BigMsgFont, -8 - SVGACord(1), 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(86) - SVGACord(2), VGAScaleX(262), VGAScaleY(132), ntext);
	} else
		y = 115 - SVGACord(5);

	BigMsgFont = &bmfont;

	if (!getFont("P:Note.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return;
	}

	if ((ntext = getText("Lab:Rooms/Col1")) == NULL)
		return;

	CharsPrinted = flowText(BigMsgFont, -4, 0, 0, false, false, false, true, VGAScaleX(45), VGAScaleY(y), VGAScaleX(158), VGAScaleY(148), ntext);

	if ((ntext = getText("Lab:Rooms/Col2")) == NULL)
		return;

	CharsPrinted = flowText(BigMsgFont, -4, 0, 0, false, false, false, true, VGAScaleX(162), VGAScaleY(y), VGAScaleX(275), VGAScaleY(148), ntext);

	VGASetPal(diffcmap, 256);
	freeAllStolenMem();
}




/*---------------------------------------------------------------------------*/
/*---------------------------- The Journal stuff ----------------------------*/
/*---------------------------------------------------------------------------*/


#define BRIDGE0   148
#define BRIDGE1   104
#define DIRTY     175
#define NONEWS    135
#define NOCLEAN   152


static char *journaltext, *journaltexttitle;
static uint16 JPage = 0;

static bool lastpage = false;

static Image *JCancel, *JCancelAlt, *JLeft, *JLeftAlt, *JRight, *JRightAlt, JBackImage, ScreenImage;

static uint16 JGadX[3] = {80, 144, 194}, JGadY[3] = {162, 164, 162};
static Gadget ForwardG, CancelG, BackG;




/*****************************************************************************/
/* Loads in the data for the journal.                                        */
/*****************************************************************************/
static bool loadJournalData() {
	byte **buffer;
	char filename[20];
	Gadget *TopGadget = &BackG;
	uint16 counter;
	bool bridge, dirty, news, clean;

	BigMsgFont = &bmfont;

	if (!getFont("P:Journal.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return false;
	}

	g_music->checkMusic();

	strcpy(filename, "Lab:Rooms/j0");
	bridge = g_lab->_conditions->in(BRIDGE0) || g_lab->_conditions->in(BRIDGE1);
	dirty  = g_lab->_conditions->in(DIRTY);
	news   = !g_lab->_conditions->in(NONEWS);
	clean  = !g_lab->_conditions->in(NOCLEAN);

	if (bridge && clean && news)
		filename[11] = '8';
	else if (clean && news)
		filename[11] = '9';
	else if (bridge && clean)
		filename[11] = '6';
	else if (clean)
		filename[11] = '7';
	else if (bridge && dirty && news)
		filename[11] = '4';
	else if (dirty && news)
		filename[11] = '5';
	else if (bridge && dirty)
		filename[11] = '2';
	else if (dirty)
		filename[11] = '3';
	else if (bridge)
		filename[11] = '1';

	if ((journaltext = getText(filename)) == NULL)
		return false;

	if ((journaltexttitle = getText("Lab:Rooms/jt")) == NULL)
		return false;

	buffer = g_music->newOpen("P:JImage");

	if (!buffer)
		return false;

	readImage(buffer, &JLeft);
	readImage(buffer, &JLeftAlt);
	readImage(buffer, &JRight);
	readImage(buffer, &JRightAlt);
	readImage(buffer, &JCancel);
	readImage(buffer, &JCancelAlt);

	BackG.Im = JLeft;
	BackG.ImAlt = JLeftAlt;
	BackG.KeyEquiv = VKEY_LTARROW;

	ForwardG.Im = JRight;
	ForwardG.ImAlt = JRightAlt;
	ForwardG.KeyEquiv = VKEY_RTARROW;

	CancelG.Im = JCancel;
	CancelG.ImAlt = JCancelAlt;

	counter = 0;

	while (TopGadget) {
		TopGadget->x = VGAScaleX(JGadX[counter]);

		if (counter == 1)
			TopGadget->y = VGAScaleY(JGadY[counter]) + SVGACord(1);
		else
			TopGadget->y = VGAScaleY(JGadY[counter]) - SVGACord(1);

		TopGadget->GadgetID = counter;
		TopGadget = TopGadget->NextGadget;
		counter++;
	}

	return true;
}



/*****************************************************************************/
/* Draws the text to the back journal screen to the appropriate Page number  */
/*****************************************************************************/
static void drawJournalText(void) {
	uint16 DrawingToPage = 1;
	int32 CharsDrawn    = 0L;
	char *CurText = journaltext;

	while (DrawingToPage < JPage) {
		g_music->newCheckMusic();
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += flowText(BigMsgFont, -2, 2, 0, false, false, false, false, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);

		lastpage = (*CurText == 0);

		if (lastpage)
			JPage = (DrawingToPage / 2) * 2;
		else
			DrawingToPage++;
	}

	if (JPage <= 1) {
		CurText = journaltexttitle;
		flowTextToMem(&JBackImage, BigMsgFont, -2, 2, 0, false, true, true, true, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);
	} else {
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += flowTextToMem(&JBackImage, BigMsgFont, -2, 2, 0, false, false, false, true, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);
	}

	g_music->checkMusic();
	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = (*CurText == 0);
	flowTextToMem(&JBackImage, BigMsgFont, -2, 2, 0, false, false, false, true, VGAScaleX(171), VGAScaleY(32), VGAScaleX(271), VGAScaleY(148), CurText);

	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = lastpage || (*CurText == 0);
}




/*****************************************************************************/
/* Does the turn page wipe.                                                  */
/*****************************************************************************/
static void turnPage(bool FromLeft) {
	uint16 counter;

	if (FromLeft) {
		for (counter = 0; counter < VGAScreenWidth; counter += 8) {
			g_music->updateMusic();
			waitTOF();
			ScreenImage.ImageData = getVGABaseAddr();
			bltBitMap(&JBackImage, counter, 0, &ScreenImage, counter, 0, 8, VGAScreenHeight);
		}
	} else {
		for (counter = (VGAScreenWidth - 8); counter > 0; counter -= 8) {
			g_music->updateMusic();
			waitTOF();
			ScreenImage.ImageData = getVGABaseAddr();
			bltBitMap(&JBackImage, counter, 0, &ScreenImage, counter, 0, 8, VGAScreenHeight);
		}
	}
}



static bool GotBackImage = false;

/*****************************************************************************/
/* Draws the journal from page x.                                            */
/*****************************************************************************/
static void drawJournal(uint16 wipenum, bool needFade) {
	mouseHide();

	g_music->checkMusic();

	if (!GotBackImage)
		JBackImage.ImageData = loadBackPict("P:Journal.pic", true);

	drawJournalText();

	ScreenImage.ImageData = getVGABaseAddr();

	if (wipenum == 0)
		bltBitMap(&JBackImage, 0, 0, &ScreenImage, 0, 0, VGAScreenWidth, VGAScreenHeight);
	else
		turnPage((bool)(wipenum == 1));

	if (JPage == 0)
		ghoastGadget(&BackG, 15);
	else
		unGhoastGadget(&BackG);

	if (lastpage)
		ghoastGadget(&ForwardG, 15);
	else
		unGhoastGadget(&ForwardG);

	ungetVGABaseAddr();


	if (needFade)
		fade(true, 0);

	nopalchange = true;
	JBackImage.ImageData = readPictToMem("P:Journal.pic", VGAScreenWidth, VGAScreenHeight);
	GotBackImage = true;

	eatMessages();
	mouseShow();

	nopalchange = false;
}




/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
static void processJournal() {
	IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, GadID;

	while (1) {
		g_music->checkMusic();  /* Make sure we check the music at least after every message */
		Msg = (IntuiMessage *) getMsg();

		if (Msg == NULL) {
			g_music->newCheckMusic();
		} else {
			Class     = Msg->Class;
			Qualifier = Msg->Qualifier;
			GadID     = Msg->Code;

			replyMsg((void *) Msg);

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (GadID == 27)))
				return;

			else if (Class == GADGETUP) {
				if (GadID == 0) {
					if (JPage >= 2) {
						JPage -= 2;
						drawJournal(1, false);
					}
				} else if (GadID == 1) {
					return;
				} else if (GadID == 2) {
					if (!lastpage) {
						JPage += 2;
						drawJournal(2, false);
					}
				}
			}
		}
	}
}



/*****************************************************************************/
/* Does the journal processing.                                              */
/*****************************************************************************/
void doJournal() {
	resetBuffer();
	blackAllScreen();

	lastpage    = false;
	GotBackImage = false;

	JBackImage.Width = VGAScreenWidth;
	JBackImage.Height = VGAScreenHeight;
	JBackImage.ImageData   = NULL;

	BackG.NextGadget = &CancelG;
	CancelG.NextGadget = &ForwardG;

	ScreenImage = JBackImage;
	ScreenImage.ImageData = getVGABaseAddr();

	g_music->checkMusic();
	loadJournalData();

	drawJournal(0, true);

	attachGadgetList(&BackG);
	mouseShow();
	processJournal();
	attachGadgetList(NULL);
	fade(false, 0);
	mouseHide();

	ScreenImage.ImageData = getVGABaseAddr();

	setAPen(0);
	rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
	blackScreen();

	freeAllStolenMem();

	ungetVGABaseAddr();
}

#define QUARTERNUM           30

extern InventoryData *Inventory;
extern uint16 RoomNum, Direction;

bool saveRestoreGame(void) {
	bool isOK = false;

	// The original had one screen for saving/loading. We have two.
	// Ask the user which screen to use.
	GUI::MessageDialog saveOrLoad(_("Would you like to save or restore a game?"), _("Save"), _("Restore"));

	int choice = saveOrLoad.runModal();
	if (choice == GUI::kMessageOK) {
		// Save
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			Common::String desc = dialog->getResultString();

			if (desc.empty()) {
				// create our own description for the saved game, the user didn't enter it
				desc = dialog->createDefaultSaveDescription(slot);
			}

			isOK = saveGame(RoomNum, Direction, Inventory[QUARTERNUM].Many, slot, desc);
		}
	} else {
		// Restore
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			isOK = loadGame(&RoomNum, &Direction, &(Inventory[QUARTERNUM].Many), slot);
			if (isOK)
				g_music->resetMusic();
		}
	}

	WSDL_UpdateScreen();
	
	return isOK;
}

/*---------------------------------------------------------------------------*/
/*--------------------------- The Monitors stuff ----------------------------*/
/*---------------------------------------------------------------------------*/


extern CloseDataPtr CPtr;

static uint16 monpage;
static const char *TextFileName;


Image *MonButton, *AltMonButton, *MonQuit, *AltMonQuit, *MonBack, *AltMonBack,
		*MonDown, *AltMonDown, *MonUp, *AltMonUp;




/*****************************************************************************/
/* Makes sure that the buttons are in memory.                                */
/*****************************************************************************/
static void getMonImages(void) {
	byte **buffer;

	resetBuffer();

	buffer = g_music->newOpen("P:MonImage");

	if (!buffer)
		return;

	readImage(buffer, &MonButton);

	stealBufMem(sizeOfFile("P:MonImage"));  /* Trick: protects the memory where the buttons are so they won't be over-written */
}


static uint16 MonGadHeight = 1;


/*****************************************************************************/
/* Draws the text for the monitor.                                           */
/*****************************************************************************/
static void drawMonText(char *text, uint16 x1, uint16 y1, uint16 x2, uint16 y2, bool isinteractive) {
	uint16 DrawingToPage = 0, yspacing = 0, numlines, fheight, counter;
	int32 CharsDrawn    = 0L;
	char *CurText = text;

	mouseHide();

	if (*text == '%') {
		text++;
		numlines = (*text - '0') * 10;
		text++;
		numlines += (*text - '0');
		text += 2;

		fheight = textHeight(BigMsgFont);
		x1 = MonButton->Width + VGAScaleX(3);
		MonGadHeight = MonButton->Height + VGAScaleY(3);

		if (MonGadHeight > fheight)
			yspacing = MonGadHeight - fheight;
		else
			MonGadHeight = fheight;

		setAPen(0);
		rectFill(0, 0, VGAScreenWidth - 1, y2);

		for (counter = 0; counter < numlines; counter++)
			drawImage(MonButton, 0, counter * MonGadHeight);
	} else if (isinteractive) {
		setAPen(0);
		rectFill(0, 0, VGAScreenWidth - 1, y2);
	} else {
		setAPen(0);
		rectFill(x1, y1, x2, y2);
	}

	while (DrawingToPage < monpage) {
		g_music->newCheckMusic();
		CurText = (char *)(text + CharsDrawn);
		CharsDrawn += flowText(BigMsgFont, yspacing, 0, 0, false, false, false, false, x1, y1, x2, y2, CurText);
		lastpage = (*CurText == 0);

		if (lastpage)
			monpage = DrawingToPage;
		else
			DrawingToPage++;
	}

	CurText = (char *)(text + CharsDrawn);
	lastpage = (*CurText == 0);
	CharsDrawn = flowText(BigMsgFont, yspacing, 2, 0, false, false, false, true, x1, y1, x2, y2, CurText);
	CurText += CharsDrawn;
	lastpage = lastpage || (*CurText == 0);

	mouseShow();
}

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
static void processMonitor(char *ntext, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, Code, MouseX, MouseY;
	const char *Test = " ", *StartFileName = TextFileName;
	CloseDataPtr StartCPtr = CPtr, TestCPtr, LastCPtr[10];
	uint16 depth = 0;

	LastCPtr[0] = CPtr;

	while (1) {
		if (isinteractive) {
			if (CPtr == NULL)
				CPtr = StartCPtr;

			if (CPtr == StartCPtr)
				Test = StartFileName;
			else
				Test = CPtr->GraphicName;

			if (strcmp(Test, TextFileName)) {
				monpage      = 0;
				TextFileName = Test;

				ntext = getText(TextFileName);
				fade(false, 0);
				drawMonText(ntext, x1, y1, x2, y2, isinteractive);
				fade(true, 0);
			}
		}

		g_music->checkMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			g_music->newCheckMusic();
		} else {
			Class     = Msg->Class;
			Qualifier = Msg->Qualifier;
			MouseX    = Msg->MouseX;
			MouseY    = Msg->MouseY;
			Code      = Msg->Code;

			replyMsg(Msg);

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (Code == 27)))
				return;

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if ((MouseY >= VGAScaleY(171)) && (MouseY <= VGAScaleY(200))) {
					if ((MouseX >= VGAScaleX(259)) && (MouseX <= VGAScaleX(289))) {
						if (!lastpage) {
							monpage += 1;
							drawMonText(ntext, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= VGAScaleX(0)) && (MouseX <= VGAScaleX(31))) {
						return;
					} else if ((MouseX >= VGAScaleX(290)) && (MouseX <= VGAScaleX(320))) {
						if (monpage >= 1) {
							monpage -= 1;
							drawMonText(ntext, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= VGAScaleX(31)) && (MouseX <= VGAScaleX(59))) {
						if (isinteractive) {
							monpage = 0;

							if (depth) {
								depth--;
								CPtr = LastCPtr[depth];
							}
						} else if (monpage > 0) {
							monpage = 0;
							drawMonText(ntext, x1, y1, x2, y2, isinteractive);
						}
					}
				} else if (isinteractive) {
					TestCPtr = CPtr;
					MouseY = 64 + (MouseY / MonGadHeight) * 42;
					MouseX = 101;
					setCurCloseAbs(MouseX, MouseY, &CPtr);

					if (TestCPtr != CPtr) {
						LastCPtr[depth] = TestCPtr;
						depth++;
					}
				}
			}
		}
	}
}




/*****************************************************************************/
/* Does what's necessary for the monitor.                                    */
/*****************************************************************************/
void doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	char *ntext;

	x1 = VGAScaleX(x1);
	x2 = VGAScaleX(x2);
	y1 = VGAScaleY(y1);
	y2 = VGAScaleY(y2);

	TextFileName = textfile;

	blackAllScreen();

	readPict("P:Mon/Monitor.1", true);
	readPict("P:Mon/NWD1", true);
	readPict("P:Mon/NWD2", true);
	readPict("P:Mon/NWD3", true);
	blackAllScreen();

	resetBuffer();
	monpage = 0;
	lastpage = false;
	FadePalette = hipal;

	BigMsgFont = &bmfont;

	if (!getFont("P:Map.font", BigMsgFont)) {
		freeAllStolenMem();
		BigMsgFont = NULL;
		return;
	}

	getMonImages();

	if ((ntext = getText(textfile)) == NULL) {
		freeAllStolenMem();
		return;
	}

	loadBackPict(background, false);

	drawMonText(ntext, x1, y1, x2, y2, isinteractive);

	mouseShow();
	fade(true, 0);
	processMonitor(ntext, isinteractive, x1, y1, x2, y2);
	fade(false, 0);
	mouseHide();

	freeAllStolenMem();

	setAPen(0);
	rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
	blackAllScreen();
}

#if defined(LABDEMO)
void doTrialBlock() {
	IntuiMessage *Msg;

	uint32 Class;
	uint16 Qualifier, MouseX, MouseY, Code, Temp;
	int i;

	loadBackPict("P:Warning", false);
	mouseShow();

	VGASetPal(diffcmap, 256);
	memcpy(g_CommonPalette, diffcmap, 3 * 256);

	while (1) {
		g_music->checkMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			g_music->newCheckMusic();
		} else {
			Class     = Msg->Class;
			Qualifier = Msg->Qualifier;
			MouseX    = Msg->MouseX;
			MouseY    = Msg->MouseY;
			Code      = Msg->Code;

			replyMsg(Msg);

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (Code == 27)))
				return;

			if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if (MouseY > 399) {
					// click on control panel, exit
					break;
				}

				if (MouseX >= 0 && MouseX <= 319 && MouseY >= 0 && MouseY <= 399) {
					extern void getItNow();
					getItNow();
				} else if (MouseX >= 320 && MouseX <= 639 && MouseY >= 0 && MouseY <= 399) {
					break;
				}
			}
		}
	}

	eatMessages();
	mouseHide();
}
#endif

} // End of namespace Lab
