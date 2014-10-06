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

#define MODERNGAMESAVE  1

#if defined(MODERNGAMESAVE)
#include "lab/modernsavegame.h"
#endif

namespace Lab {

#ifdef GAME_TRIAL
int g_IsRegistered;
#endif

extern bool nopalchange, DoBlack, IsHiRes;

extern struct BitMap *DispBitMap, *DrawBitMap;
extern char diffcmap[3 * 256];

extern uint32 VGAScreenWidth, VGAScreenHeight;


#define COMBINATIONUNLOCKED  130
#define BRICKOPEN            115


static uint16 hipal[20];
extern uint16 *FadePalette;


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




static struct Image *Images[10];


uint16 combination[6] = {0, 0, 0, 0, 0, 0}, solution[] = {0, 4, 0, 8, 7, 2};

static uint16 combx[] = {45, 83, 129, 166, 211, 248};



/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
static void doCombination(void) {
	uint16 counter;

	for (counter = 0; counter <= 5; counter++)
		drawImage(Images[combination[counter]], VGAScaleX(combx[counter]), VGAScaleY(65));
}


extern char *TempScrollData;

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
static void changeCombination(LargeSet Conditions, uint16 number) {
	struct Image display;
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

#if !defined(DOSCODE)
		display.ImageData = getVGABaseAddr();
#endif

		scrollDisplayY(2, VGAScaleX(combx[number]), VGAScaleY(65), VGAScaleX(combx[number]) + (Images[combnum])->Width - 1, VGAScaleY(65) + (Images[combnum])->Height);

		bltBitMap(Images[combnum], 0, (Images[combnum])->Height - (2 * counter), &(display), VGAScaleX(combx[number]), VGAScaleY(65), (Images[combnum])->Width, 2);
	}

	/*
	   if (memcmp(combination, solution, (size_t) 12) == 0)
	    inclElement(Conditions, COMBINATIONUNLOCKED);
	   else
	    exclElement(Conditions, COMBINATIONUNLOCKED);
	 */

	for (counter = 0; counter < 6; counter++)
		unlocked = (combination[counter] == solution[counter]) && unlocked;

	if (unlocked)
		inclElement(Conditions, COMBINATIONUNLOCKED);
	else
		exclElement(Conditions, COMBINATIONUNLOCKED);

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif
}





/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseCombination(LargeSet Conditions, uint16 x, uint16 y) {
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

		changeCombination(Conditions, number);
	}
}



/*----------------------------------------------------------------------------*/
/*----------------------------- Tile Puzzle Rules ----------------------------*/
/*----------------------------------------------------------------------------*/




struct Image *Tiles[16];
int16 CurTile[4] [4] = {
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
static void changeTile(LargeSet Conditions, uint16 col, uint16 row) {
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
			inclElement(Conditions, BRICKOPEN);  /* unlocked combination */
			DoBlack = true;
			check = readPict("p:Up/BDOpen", true);
		}
	}
}





/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseTile(LargeSet Conditions, uint16 x, uint16 y) {
	x = VGAUnScaleX(x);
	y = VGAUnScaleY(y);

	if ((x < 101) || (y < 26))
		return;

	x = (x - 101) / 30;
	y = (y -  26) / 25;

	if ((x < 4) && (y < 4))
		changeTile(Conditions, x, y);
}


/*---------------------------------------------------------------------------*/
/*------------------------ Does the detective notes. ------------------------*/
/*---------------------------------------------------------------------------*/

extern struct TextFont *MsgFont;
static struct TextFont *BigMsgFont;
static struct TextFont bmfont;


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

static struct Image *JCancel, *JCancelAlt, *JLeft, *JLeftAlt, *JRight, *JRightAlt, JBackImage, ScreenImage;

static uint16 JGadX[3] = {80, 144, 194}, JGadY[3] = {162, 164, 162};
static struct Gadget ForwardG, CancelG, BackG;




/*****************************************************************************/
/* Loads in the data for the journal.                                        */
/*****************************************************************************/
static bool loadJournalData(LargeSet Conditions) {
	byte **buffer;
	char filename[20];
	struct Gadget *TopGadget = &BackG;
	uint16 counter;
	bool bridge, dirty, news, clean;

	BigMsgFont = &bmfont;

	if (!getFont("P:Journal.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return false;
	}

	g_music->checkMusic();

	strcpy(filename, "Lab:Rooms/j0");
	bridge = In(Conditions, BRIDGE0) || In(Conditions, BRIDGE1);
	dirty  = In(Conditions, DIRTY);
	news   = !In(Conditions, NONEWS);
	clean  = !In(Conditions, NOCLEAN);

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
#if !defined(DOSCODE)
	BackG.KeyEquiv = VKEY_LTARROW;
#endif
	ForwardG.Im = JRight;
	ForwardG.ImAlt = JRightAlt;
#if !defined(DOSCODE)
	ForwardG.KeyEquiv = VKEY_RTARROW;
#endif
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
#if !defined(DOSCODE)
			ScreenImage.ImageData = getVGABaseAddr();
#endif
			bltBitMap(&JBackImage, counter, 0, &ScreenImage, counter, 0, 8, VGAScreenHeight);
		}
	} else {
		for (counter = (VGAScreenWidth - 8); counter > 0; counter -= 8) {
			g_music->updateMusic();
			waitTOF();
#if !defined(DOSCODE)
			ScreenImage.ImageData = getVGABaseAddr();
#endif
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

#if !defined(DOSCODE)
	ScreenImage.ImageData = getVGABaseAddr();
#endif

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

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif


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
	struct IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, GadID;

	while (1) {
		g_music->checkMusic();  /* Make sure we check the music at least after every message */
		Msg = (struct IntuiMessage *) getMsg();

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
/* Cleans up behind all memory allocations.                                  */
/*****************************************************************************/
static void journalCleanUp(void) {
	freeAllStolenMem();
}





/*****************************************************************************/
/* Does the journal processing.                                              */
/*****************************************************************************/
void doJournal(LargeSet Conditions) {
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
	loadJournalData(Conditions);

	drawJournal(0, true);

	attachGadgetList(&BackG);
	mouseShow();
	processJournal();
	attachGadgetList(NULL);
	fade(false, 0);
	mouseHide();

#if !defined(DOSCODE)
	ScreenImage.ImageData = getVGABaseAddr();
#endif

	setAPen(0);
	rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
	blackScreen();

	journalCleanUp();

#if !defined(DOSCODE)
	ungetVGABaseAddr();
#endif
}



/*---------------------------------------------------------------------------*/
/*------------------------- The Save/Restore stuff --------------------------*/
/*---------------------------------------------------------------------------*/


struct Image *Arrow1, *NoArrow1, *DriveButton;


extern InventoryData *Inventory;
extern uint16 RoomNum, Direction;
extern LargeSet Conditions, RoomsFound;



extern char *SAVETEXT, *LOADTEXT, *BOOKMARKTEXT, *PERSONALTEXT, *DISKTEXT, *SAVEBOOK, *RESTOREBOOK, *SAVEFLASH, *RESTOREFLASH, *SAVEDISK, *RESTOREDISK, *SELECTDISK, *NODISKINDRIVE, *WRITEPROTECTED, *FORMATFLOPPY, *FORMATTING;

static uint16 device;

#define MAXDEVNAMES  5

static char DriveName[5] [MAXDEVNAMES];






/*----- Gets the devices -----*/

/*****************************************************************************/
/* Finds all the disk drives, puts them in an array of strings, sorts them,  */
/* and returned the number of drives that it found.                          */
/*****************************************************************************/
uint16 doDisks(void) {
#if defined(DOSCODE)
	uint16 many = 0, counter = 2;
	union REGS regs;
	char str[5];

	if (manydisks)
		return manydisks;

	while ((counter < 7) && (many < MAXDEVNAMES)) {
		memset(&regs, 0, sizeof(regs));
		/*
		    regs.h.ah = 0x32;
		    regs.h.dl = counter+1;
		    int386(0x21, &regs, &regs);
		    if (regs.h.al == 0)
		    {
		 */
		regs.h.ah = 0x36;
		regs.h.dl = counter + 1;
		int386(0x21, &regs, &regs);

		if (regs.w.ax != 0xFFFF) {
			str[0] = counter + 'A';
			str[1] = ':';
			str[2] = '\\';
			str[3] = 0;
			strcpy(DriveName[many], str);
			many++;
		}

		counter++;
	}

	return many;
#elif defined(WIN32)
	extern void winGetDriveLetters(char cdLetters[], int size);
	char letters[28];
	char str[5];
	uint16 many = 0, counter = 2;

	if (manydisks)
		return manydisks;

	winGetDriveLetters(letters, 28);

	while ((counter < 7) && (many < MAXDEVNAMES) && letters[many] != 0) {
		str[0] = letters[many];
		str[1] = ':';
		str[2] = '\\';
		str[3] = 0;
		strcpy(DriveName[many], str);
		many++;
		counter++;
	}

	return many;
#else
	// !!!!goofy test code
	char str[5];
	str[0] = 'C';
	str[1] = ':';
	str[2] = '\\';
	str[3] = 0;
	strcpy(DriveName[0], str);
	return 1;
#endif
}





/*****************************************************************************/
/* Does the user interface to save or restore a game position                */
/*****************************************************************************/
#if defined(MODERNGAMESAVE)

extern const byte ThePalMap[];

void getRoomMessage(int MyRoomNum, int MyDirection, char *msg);

#define QUARTERNUM           30

#define NEXTEMPTYSLOTTEXT   "Next Empty Slot"

extern char g_SaveGamePath[512];
extern char g_PathSeperator[4];

struct SaveGameInfo g_SaveGameInfo[MAX_SAVED_GAMES];
int g_TotalSavedGames;
char g_CommonPalette[3 * 256];
int g_LastSaveGameNumber = 0;
int g_CurSaveGameNumber = 0;
int g_CurSaveSet = 0;
int g_PendingNewSave = 0;

enum UI_Ident {
	ID_SAVE, ID_LOAD, ID_NEWSLOT, ID_1_TO_5, ID_6_TO_10, ID_11_TO_15, ID_SLOT_A, ID_SLOT_B, ID_SLOT_C, ID_SLOT_D, ID_SLOT_E, ID_CANCEL, ID_LAST
};

struct ModernUI {
	int id;
	int x, y, w, h;
};

struct ModernUI theUI[ID_LAST] = {
	{ ID_LOAD,      491, 182, 128,  54 }, { ID_SAVE,      491, 255, 128,  54 }, { ID_NEWSLOT,   491, 328, 128,  54 }, { ID_1_TO_5,     27,  40, 146,  25 }, { ID_6_TO_10,   175,  40, 146,  25 }, { ID_11_TO_15,  323,  40, 146,  25 }, { ID_SLOT_A,     27,  67, 442,  72 }, { ID_SLOT_B,     27, 142, 442,  72 }, { ID_SLOT_C,     27, 217, 442,  72 }, { ID_SLOT_D,     27, 292, 442,  72 }, { ID_SLOT_E,     27, 367, 442,  72 }, { ID_CANCEL,    531, 405,  52,  52 }
};

enum {
	SG_BLACK = 1, SG_TAN = 14, SG_DKTAN = 38, SG_WHITE = 105, SG_YELLOW = 118
};

/*****************************************************************************/
/* Draw display                                                              */
/*****************************************************************************/
static void flowTextBold(void *font,      /* the TextAttr pointer */
                         uint16 spacing,   /* How much vertical spacing between the lines */
                         uint16 pencolor,  /* pen number to use for text */
                         uint16 backpen,   /* the background color */
                         bool outline,          /* Whether to outline in background color */
                         bool centerh,          /* Whether to center the text horizontally */
                         bool centerv,          /* Whether to center the text vertically */
                         bool output,           /* Whether to output any text */
                         uint16 x1,        /* Cords */
                         uint16 y1, uint16 x2, uint16 y2,  const char *text) { /* The text itself */

	if (outline) {
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 - 1, y1, x2 - 1, y2, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 - 1, y1 - 1, x2 - 1, y2 - 1, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1, y1 + 2, x2, y2 + 2, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 - 1, y1 + 2, x2 - 1, y2 + 2, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 + 2, y1, x2 + 2, y2, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 + 2, y1 + 2, x2 + 2, y2 + 2, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1, y1 - 1, x2, y2 - 1, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 + 2, y1 - 1, x2 + 2, y2 - 1, text);
	}

	flowText(font, spacing, pencolor, 0, false, centerh, centerv, output, x1, y1, x2, y2, text);
	flowText(font, spacing, pencolor, 0, false, centerh, centerv, output, x1 + 1, y1, x2 + 1, y2, text);
	flowText(font, spacing, pencolor, 0, false, centerh, centerv, output, x1, y1 + 1, x2, y2 + 1, text);
	flowText(font, spacing, pencolor, 0, false, centerh, centerv, output, x1 + 1, y1 + 1, x2 + 1, y2 + 1, text);
}

/*****************************************************************************/
/* Draw display                                                              */
/*****************************************************************************/
static void flowTextShadow(void *font,      /* the TextAttr pointer */
                           uint16 spacing,   /* How much vertical spacing between the lines */
                           uint16 pencolor,  /* pen number to use for text */
                           uint16 backpen,   /* the background color */
                           bool outline,          /* Whether to outline in background color */
                           bool centerh,          /* Whether to center the text horizontally */
                           bool centerv,          /* Whether to center the text vertically */
                           bool output,           /* Whether to output any text */
                           uint16 x1,        /* Cords */
                           uint16 y1, uint16 x2, uint16 y2, char *text) { /* The text itself */

	if (outline) {
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 - 1, y1 - 1, x2 - 1, y2 - 1, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 - 1, y1 + 1, x2 - 1, y2 + 1, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 + 1, y1 + 1, x2 + 1, y2 + 1, text);
		flowText(font, spacing, backpen, 0, false, centerh, centerv, output, x1 + 1, y1 - 1, x2 + 1, y2 - 1, text);
	}

	flowText(font, spacing, pencolor, 0, false, centerh, centerv, output, x1, y1, x2, y2, text);
}

static void drawSaveLoad() {
	int i, j;

	flowTextBold(MsgFont, 0, g_CurSaveGameNumber < g_TotalSavedGames ? SG_YELLOW : SG_DKTAN, SG_BLACK, true, true, true, true, 491 + 3, 182, 619 + 3, 236, LOADTEXT);
	flowTextBold(MsgFont, 0, g_TotalSavedGames + g_PendingNewSave > 0 ? SG_YELLOW : SG_DKTAN, SG_BLACK, true, true, true, true, 491 + 3, 255, 619 + 3, 309, SAVETEXT);
	flowTextBold(MsgFont, 0, g_PendingNewSave ? SG_DKTAN : SG_YELLOW, SG_BLACK, true, true, true, true, 491 + 3, 328, 619 + 3, 382, NEXTEMPTYSLOTTEXT);

	flowTextBold(MsgFont, 0, (g_CurSaveSet == 0 ? SG_YELLOW : SG_WHITE), SG_BLACK, true, true, true, true, 27 + 3, 40, 175 + 3, 65, "Saves 1-5");
	flowTextBold(MsgFont, 0, (g_CurSaveSet == 1 ? SG_YELLOW : SG_WHITE), SG_BLACK, true, true, true, true, 175 + 3, 40, 321 + 3, 65, "Saves 6-10");
	flowTextBold(MsgFont, 0, (g_CurSaveSet == 2 ? SG_YELLOW : SG_WHITE), SG_BLACK, true, true, true, true, 323 + 3, 40, 469 + 3, 65, "Saves 11-15");

	for (i = 0, j = 5 * g_CurSaveSet; i < 5; i++, j++) {
		uint16 hue, y;
		char num_text[4];
		struct Image thumbnail, screen;

		if (j < g_TotalSavedGames + g_PendingNewSave) {
			char msg[256];

			getRoomMessage(g_SaveGameInfo[j].RoomNumber, g_SaveGameInfo[j].Direction, msg);

			hue = (j == g_CurSaveGameNumber ? SG_YELLOW : SG_WHITE);
			y = 67 + 2 + i * 75;
			flowText(MsgFont, 0, hue, 0, false, false, false, true, 202 + 2, y, 469 - 2, y + 48, msg);
			y += 46;
			flowText(MsgFont, 0, hue, 0, false, false, false, true, 202 + 2, y, 469 - 2, y + 24, g_SaveGameInfo[j].SaveGameDate);

			// blast image
			thumbnail.Width = 128;
			thumbnail.Height = 72;
			thumbnail.ImageData = g_SaveGameInfo[j].SaveGameImage;
			screen.Width = 640;
			screen.Height = 480;
			screen.ImageData = getVGABaseAddr();
			bltBitMap(&thumbnail, 0, 0, &screen, 72, 67 + i * 75, 128, 72);
			ungetVGABaseAddr();

			hue = (j == g_CurSaveGameNumber ? SG_YELLOW : SG_WHITE);
		} else {
			y = 67 + 2 + i * 75;
			flowText(MsgFont, 0, SG_TAN, 0, false, true, true, true, 202 + 2, y, 469 - 2, y + 70, "[Empty Slot]");

			hue = SG_DKTAN;
		}

		y = 67 + i * 75;
		sprintf(num_text, "%d", j + 1);
		flowTextShadow(BigMsgFont, 0, hue, SG_BLACK, true, true, true, true, 27 + 4, y, 72 + 4, y + 72, num_text);
	}

	// Add ".1" to version number
	setAPen(SG_WHITE);
	rectFill(271, 454, 271, 454);
	rectFill(275, 449, 275, 454);
	rectFill(274, 450, 274, 450);
	rectFill(274, 454, 276, 454);

	g_system->updateScreen();
}

static void makeThumbnail(struct SaveGameInfo *info) {
	char *pictName;
	CloseDataPtr CPtr = NULL;
	byte *BitMapMem;
	int x, y, u, v;

	// load pict
	pictName = getPictName(&CPtr);
	nopalchange = true;
	BitMapMem = readPictToMem(pictName, VGAScreenWidth, VGAScreenHeight);
	nopalchange = false;

	for (y = 0; y < 72; y++) {
		for (x = 0; x < 128; x++) {
			unsigned int r = 0, g = 0, b = 0;

			for (v = 5 * y; v < 5 * y + 5; v++) {
				for (u = 5 * x; u < 5 * x + 5; u++) {
					byte n = (byte)BitMapMem[u + v * VGAScreenWidth];
					// 6-bit color (VGA)
					r += (unsigned int)diffcmap[3 * n + 0];
					g += (unsigned int)diffcmap[3 * n + 1];
					b += (unsigned int)diffcmap[3 * n + 2];
				}
			}

			r = (r / 25) >> 1;
			g = (g / 25) >> 1;
			b = (b / 25) >> 1;
			warning("STUB: makeThumbnail");
			info->SaveGameImage[x + 128 * y] = ThePalMap[(r << 10) | (g << 5) | b];
		}
	}
}

static void addSaveSlot() {
	struct SaveGameInfo *info;

	if (g_PendingNewSave || g_TotalSavedGames == MAX_SAVED_GAMES)
		return;

	g_PendingNewSave = 1;
	g_CurSaveGameNumber = g_TotalSavedGames;
	g_CurSaveSet = g_CurSaveGameNumber / 5;

	// set-up saved game
	info = &g_SaveGameInfo[g_CurSaveGameNumber];
	info->RoomNumber = RoomNum;
	info->Direction = Direction;

	// not really a date yet
	strcpy(info->SaveGameDate, "Click SAVE GAME to Confirm");

	info->SaveGameImage = (byte *)malloc(SAVED_IMAGE_SIZE);
	makeThumbnail(info);

	mouseHide();
	warning("STUB: addSaveSlot()");
	//SDL_IgnoreUpdateDisplay(1);
	loadBackPict("P:ModSave", false);
	//SDL_IgnoreUpdateDisplay(0);
	drawSaveLoad();
	mouseShow();
}

static void selectSave(int n) {
	if (g_CurSaveGameNumber == n || n >= g_TotalSavedGames + g_PendingNewSave)
		return;

	g_CurSaveGameNumber = n;

	mouseHide();
	warning("STUB: selectSave()");
	//SDL_IgnoreUpdateDisplay(1);
	loadBackPict("P:ModSave", false);
	//SDL_IgnoreUpdateDisplay(0);
	drawSaveLoad();
	mouseShow();
}

static void selectSaveSet(int n) {
	if (g_CurSaveSet != n) {
		g_CurSaveSet = n;

		mouseHide();
		warning("STUB: selectSaveSet");
		//SDL_IgnoreUpdateDisplay(1);
		loadBackPict("P:ModSave", false);
		//SDL_IgnoreUpdateDisplay(0);
		drawSaveLoad();
		mouseShow();
	}
}

/*****************************************************************************/
/* Do modern save.                                                           */
/*****************************************************************************/
static bool doSaveGame() {
	bool isok;
	char DrivePath[260];

	if (g_CurSaveGameNumber != g_TotalSavedGames) {
		makeThumbnail(&g_SaveGameInfo[g_CurSaveGameNumber]);
	} else {
		// set time of save for new saved game
		//struct tm *create_date;
		//time_t ticks;

		warning("STUB: doSaveGame");
		//ticks = time(NULL);
		//create_date = localtime(&ticks);
		//strcpy(g_SaveGameInfo[g_CurSaveGameNumber].SaveGameDate, asctime(create_date));
	}

	memcpy(g_SaveGameImage, g_SaveGameInfo[g_CurSaveGameNumber].SaveGameImage, SAVED_IMAGE_SIZE);

	sprintf(DrivePath, "%s%s%d", g_SaveGamePath, g_PathSeperator, g_CurSaveGameNumber);

	isok = saveFloppy(DrivePath, RoomNum, Direction, Inventory[QUARTERNUM].Many, Conditions, RoomsFound, g_CurSaveGameNumber, device);
	g_music->resetMusic();

	if (isok)
		g_LastSaveGameNumber = g_CurSaveGameNumber;

	return isok;
}

/*****************************************************************************/
/* Do modern load.                                                           */
/*****************************************************************************/
static bool doLoadGame() {
	bool isok;
	char drivePath[260];

	snprintf(drivePath, 260, "%s%s%d", g_SaveGamePath, g_PathSeperator, g_CurSaveGameNumber);

	isok = readFloppy(drivePath, &RoomNum, &Direction, &(Inventory[QUARTERNUM].Many), Conditions, RoomsFound, g_CurSaveGameNumber, device);
	g_music->resetMusic();

	if (isok)
		g_LastSaveGameNumber = g_CurSaveGameNumber;

	return isok;
}

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
static bool processSaveLoad() {
	struct IntuiMessage *Msg;

	uint32 Class;
	uint16 Qualifier, MouseX, MouseY, Code;
	int i;

	drawSaveLoad();

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
				return -1;

			if (Class == RAWKEY) {
				if (Code == 'l' || Code == 'L') {
					if (g_CurSaveGameNumber < g_TotalSavedGames)
						return doLoadGame();
				} else if (Code == 's' || Code == 'S') {
					if (g_TotalSavedGames + g_PendingNewSave > 0)
						return doSaveGame();
				} else if (Code == 'n' || Code == 'N') {
					addSaveSlot();
				} else if (Code == VKEY_LTARROW) {
					i = g_CurSaveSet - 1;

					if (i < 0) i = 2;

					selectSaveSet(i);
				} else if (Code == VKEY_RTARROW) {
					i = g_CurSaveSet + 1;

					if (i > 2) i = 0;

					selectSaveSet(i);
				} else if (Code == VKEY_UPARROW) {
					if (g_TotalSavedGames + g_PendingNewSave > 0) {
						i = g_CurSaveGameNumber - 1;

						if (i < 0)
							i = g_TotalSavedGames + g_PendingNewSave - 1;

						if (i / 5 != g_CurSaveSet)
							selectSaveSet(i / 5);

						selectSave(i);
					}
				} else if (Code == VKEY_DNARROW) {
					if (g_TotalSavedGames + g_PendingNewSave > 0) {
						i = g_CurSaveGameNumber + 1;

						if (i >= g_TotalSavedGames + g_PendingNewSave)
							i = 0;

						if (i / 5 != g_CurSaveSet)
							selectSaveSet(i / 5);

						selectSave(i);
					}
				}
			} else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				for (i = 0; i < ID_LAST; i++) {
					if (MouseX >= theUI[i].x && MouseY >= theUI[i].y &&
					        MouseX < theUI[i].x + theUI[i].w && MouseY < theUI[i].y + theUI[i].h) {
						switch (theUI[i].id) {
						case ID_SAVE:
							if (g_TotalSavedGames + g_PendingNewSave > 0)
								return doSaveGame();

							break;

						case ID_LOAD:
							if (g_CurSaveGameNumber < g_TotalSavedGames)
								return doLoadGame();

							break;

						case ID_NEWSLOT:
							addSaveSlot();
							break;

						case ID_1_TO_5:
						case ID_6_TO_10:
						case ID_11_TO_15:
							selectSaveSet(theUI[i].id - ID_1_TO_5);
							break;

						case ID_SLOT_A:
						case ID_SLOT_B:
						case ID_SLOT_C:
						case ID_SLOT_D:
						case ID_SLOT_E:
							selectSave(5 * g_CurSaveSet + theUI[i].id - ID_SLOT_A);
							break;

						case ID_CANCEL:
							return -1;
						}
					}
				}
			}
		}
	}
}

bool saveRestoreGame(void) {
	byte **buffer;
	bool isok = true;

	blackAllScreen();

	BigMsgFont = &bmfont;

	if (!getFont("P:Map.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return false;
	}

	buffer = g_music->newOpen("P:ModSave");

	if (!buffer) {
		freeAllStolenMem();
		return false;
	}

	g_TotalSavedGames = getSaveGameList(g_SaveGameInfo, MAX_SAVED_GAMES);
	g_CurSaveGameNumber = g_LastSaveGameNumber;
	g_CurSaveSet = g_CurSaveGameNumber / 5;
	g_PendingNewSave = 0;

	loadBackPict("P:ModSave", false);

	mouseShow();

	VGASetPal(diffcmap, 256);
	memcpy(g_CommonPalette, diffcmap, 3 * 256);

	isok = processSaveLoad();
	eatMessages();

	mouseHide();
	memset(diffcmap, 0, 3 * 256);
	VGASetPal(diffcmap, 256);
	setAPen(0);
	rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
	blackScreen();
	g_system->updateScreen();

	journalCleanUp();

	freeAllStolenMem();

	freeSaveGameList(g_SaveGameInfo, g_TotalSavedGames);

	return isok;
}
#else

static uint16 manydisks = 0;

static uint16 issave, ghoast, arrow, ManyDrives, DriveInitX, DriveNum = -1, FileNum = -1;

/*****************************************************************************/
/* Draws the number arrows.                                                  */
/*****************************************************************************/
static void doNumArrows(void) {
	uint16 counterx, countery, curnum, cordx[3], cordy[3];

	cordx[0] = VGAScaleX(53) + SVGACord(3);
	cordx[1] = VGAScaleX(126) + SVGACord(1);
	cordx[2] = VGAScaleX(197) + SVGACord(3);

	cordy[0] = VGAScaleY(58) + SVGACord(2);
	cordy[1] = VGAScaleY(86) + SVGACord(3);
	cordy[2] = VGAScaleY(114) + SVGACord(3);

	mouseHide();

	for (countery = 0; countery < 3; countery++) {
		for (counterx = 0; counterx < 3; counterx++) {
			curnum = countery + counterx * 3;

			if INBIT(arrow, curnum)
				drawImage(Arrow1, cordx[counterx], cordy[countery]);
			else
				drawImage(NoArrow1, cordx[counterx], (int32) cordy[countery]);

			if INBIT(ghoast, curnum)
				ghoastRect(0, cordx[counterx], cordy[countery], cordx[counterx] + VGAScaleX(69), cordy[countery] + NoArrow1->Height);
		}
	}

	mouseShow();
}

/*****************************************************************************/
/* Does the drive buttons for the final save/restore screen.                 */
/*****************************************************************************/
static void doDriveButtons(void) {
	uint16 curx, counter;

	if (ManyDrives > 5)
		ManyDrives = 5;

	DriveInitX = (VGAScreenWidth / 2) - ((ManyDrives * DriveButton->Width) / 2);
	curx       = DriveInitX;

	mouseHide();

	for (counter = 0; counter < ManyDrives; counter++) {
		drawImage(DriveButton, curx, VGAScaleY(153));

		flowText(BigMsgFont, 0, 1, 0, false, true, true, true, curx + VGAScaleX(5), VGAScaleY(158), curx + DriveButton->Width - VGAScaleX(5), VGAScaleY(148) + DriveButton->Height, DriveName[counter]);

		curx += DriveButton->Width;
	}

	mouseShow();
}


static void drawSRMessage(char *rtext) {
	mouseHide();
	flowText(BigMsgFont, 0, 1, 10, true, true, true, true, VGAScaleX(22), VGAScaleY(21), VGAScaleX(289), VGAScaleY(48), rtext);
	mouseShow();
}

/*****************************************************************************/
/* Draws the correct message to the message box.                             */
/*****************************************************************************/
static void doSaveRestoreText(void) {
	char *rtext, text[100];

	if (DriveNum >= ManyDrives) {
		rtext = SELECTDISK;
	} else if (issave) {
		strcpy(text, SAVEDISK);
		strcat(text, " ");
		strcat(text, DriveName[DriveNum]);
		rtext = text;
	} else {
		strcpy(text, RESTOREDISK);
		strcat(text, " ");
		strcat(text, DriveName[DriveNum]);
		rtext = text;
	}

	drawSRMessage(rtext);
}

static uint16 processSaveRestore(uint16 type);

static char DrivePath[50];

/*****************************************************************************/
/* Checks for the existence of previous saved game positions on disk.        */
/*****************************************************************************/
static void floppyCheckFiles(void) {
	char temp[7], *name, len;
	int fl;
	uint16 counter;

	doSaveRestoreText();

	arrow = 0;
	ghoast = 0;

	strcpy(DrivePath, DriveName[DriveNum]);
	strcat(DrivePath, "LabSaves");

	warning("STUB: floppyCheckFiles");

#if 0
#if defined(WIN32)
	mkdir(DrivePath);
#else
	mkdir(DrivePath, 0x644);
#endif
	strcat(DrivePath, "\\");

	len = strlen(DrivePath);

	for (counter = 0; counter < 9; counter++) {
		name = numtostr(temp, counter);
		strcat(DrivePath, name);

		if ((fl = open(DrivePath, O_RDONLY)) != -1) {
			close(fl);
			SETBIT(arrow, counter);
		} else
			SETBIT(ghoast, counter);

		DrivePath[len] = 0;
	}
#endif
}

/*****************************************************************************/
/* Checks for the existence of previously saved game positions.              */
/*****************************************************************************/
static void checkFiles(void) {
	ghoast = -1;
	arrow  = 0;

	g_music->fillUpMusic(true);

	/* NYI: check for empty drive */
	floppyCheckFiles();

	if (issave)
		ghoast = 0;
}

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
static uint16 processSaveRestore(uint16 type) {
	struct IntuiMessage *Msg;

	uint32 Class;
	uint16 Qualifier, MouseX, MouseY, Code, Temp;

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
				return -1;

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if (type == 0) { /* The save or restore screen */
					if ((MouseX >= VGAScaleX(64)) && (MouseX <= VGAScaleX(257))) {
						if ((MouseY >= VGAScaleY(57)) && (MouseY <= VGAScaleY(92)))
							return true;
						else if ((MouseY >= VGAScaleY(108)) && (MouseY <= VGAScaleY(142)))
							return false;
					}
				}

				else if (type == 2) { /* The files screen */
					if ((MouseY >= VGAScaleY(153)) && (MouseY <= VGAScaleY(180))) {  /* the drive buttons */
						Temp = ((MouseX - DriveInitX) / DriveButton->Width);

						if (Temp < ManyDrives) {
							DriveNum = Temp;

							fade(false, 0);
							checkFiles();

							loadBackPict("P:Disk/Nums.pic", false);
							doNumArrows();
							doDriveButtons();
							doSaveRestoreText();

							fade(true, 0);
						}
					}

					else if ((MouseX >= VGAScaleX(53)) && (MouseY >= VGAScaleY(58)) &&
					         (MouseX <= VGAScaleX(266)) && (MouseY <= VGAScaleY(142))) {
						MouseX = (MouseX - VGAScaleX(53)) / VGAScaleX(71);
						MouseY = (MouseY - VGAScaleY(58)) / VGAScaleY(28);

						Temp = MouseY + (MouseX * 3);

						if (!(INBIT(ghoast, Temp))) {

							SETBIT(arrow, Temp);
							doNumArrows();

							FileNum = Temp;
							return FileNum;
						}
					}
				}
			}
		}
	}
}




/*****************************************************************************/
/* Sets up the Save or Restore screen.                                       */
/*****************************************************************************/
static uint16 saveOrRestore(void) {
	uint16 res;

	mouseHide();
	loadBackPict("P:Disk/Choose.pic", false);

	flowText(BigMsgFont, 0, 1, 4, false, true, true, true, VGAScaleX(74), VGAScaleY(65), VGAScaleX(247), VGAScaleY(84), SAVETEXT);
	flowText(BigMsgFont, 0, 1, 4, false, true, true, true, VGAScaleX(74), VGAScaleY(116), VGAScaleX(247), VGAScaleY(135), LOADTEXT);
	mouseShow();

	fade(true, 0);
	res = processSaveRestore(0);
	fade(false, 0);
	eatMessages();

	return res;
}





/*****************************************************************************/
/* Sets up the final save/restore screen.                                    */
/*****************************************************************************/
static uint16 saveRestore(void) {
	uint16 res;

	loadBackPict("P:Disk/Nums.pic", false);

	if ((DriveNum < ManyDrives)) {
		checkFiles();
	} else {
		ghoast = -1;
		arrow  =  0;
	}

	doNumArrows();
	doDriveButtons();
	doSaveRestoreText();
	g_music->newCheckMusic();

	eatMessages();
	fade(true, 0);
	res = processSaveRestore(2);
	fade(false, 0);

	return res;
}


#define QUARTERNUM           30


bool saveRestoreGame(void) {
	uint16 filenum;
	byte **buffer;
	char temp[10], *name;
	bool isok = true;

	blackAllScreen();

	ManyDrives = doDisks();

	FadePalette = hipal;
	memset(&(hipal[0]), 0, 32L);

	BigMsgFont = &bmfont;

	if (!getFont("P:Map.fon", BigMsgFont)) {
		BigMsgFont = NULL;
		return false;
	}

	buffer = g_music->newOpen("P:SaveImage");

	if (!buffer) {
		freeAllStolenMem();
		return false;
	}

	readImage(buffer, &Arrow1);
	readImage(buffer, &NoArrow1);
	readImage(buffer, &DriveButton);

	mouseShow();

	if ((issave = saveOrRestore()) != (uint16) - 1) {
		eatMessages();

		if ((filenum = saveRestore()) != (uint16) - 1) {
			name = numtostr(temp, filenum);
			strcat(DrivePath, name);

			eatMessages();

			if (issave)
				isok = saveFloppy(DrivePath, RoomNum, Direction, Inventory[QUARTERNUM].Many, Conditions, RoomsFound, filenum, device);
			else {
				isok = readFloppy(DrivePath, &RoomNum, &Direction, &(Inventory[QUARTERNUM].Many), Conditions, RoomsFound, filenum, device);
				g_music->resetMusic();
			}
		}
	}

	mouseHide();
	setAPen(0);
	rectFill(0, 0, VGAScreenWidth - 1, VGAScreenHeight - 1);
	blackScreen();

	journalCleanUp();

	freeAllStolenMem();

	return isok;
}

#endif



/*---------------------------------------------------------------------------*/
/*--------------------------- The Monitors stuff ----------------------------*/
/*---------------------------------------------------------------------------*/


extern CloseDataPtr CPtr;

static uint16 monpage;
static const char *TextFileName;


struct Image *MonButton, *AltMonButton, *MonQuit, *AltMonQuit, *MonBack, *AltMonBack, *MonDown, *AltMonDown, *MonUp, *AltMonUp;




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
	struct IntuiMessage *Msg;
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
	struct IntuiMessage *Msg;

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
