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
#include "lab/image.h"
#include "lab/parsefun.h"
#include "lab/interface.h"
#include "lab/anim.h"
#include "lab/text.h"
#include "lab/mouse.h"
#include "lab/parsetypes.h"
#include "lab/resource.h"

namespace Lab {

static uint16 MonGadHeight = 1;
static uint16 hipal[20];

// Combination lock rules
static Image *Images[10];
byte combination[6] = { 0, 0, 0, 0, 0, 0 }, solution[] = { 0, 4, 0, 8, 7, 2 };
static uint16 combx[] = { 45, 83, 129, 166, 211, 248 };

static TextFont *journalFont;
static char *journaltext, *journaltexttitle;
static uint16 JPage = 0;
static bool lastpage = false;
static Image JBackImage, ScreenImage;
static uint16 JGadX[3] = { 80, 144, 194 }, JGadY[3] = { 162, 164, 162 };
static Gadget ForwardG, CancelG, BackG;
static bool GotBackImage = false;
static uint16 monitorPage;
static const char *TextFileName;

Image *MonButton, *AltMonButton, *MonQuit, *AltMonQuit, *MonBack, *AltMonBack,
*MonDown, *AltMonDown, *MonUp, *AltMonUp;

// Tile puzzle rules
Image *Tiles[16];
uint16 CurTile[4][4] = {
	{ 1, 5, 9, 13 },
	{ 2, 6, 10, 14 },
	{ 3, 7, 11, 15 },
	{ 4, 8, 12, 0 }
}, TileSolution[4][4] = {
	{ 7, 1, 8, 3 },
	{ 2, 11, 15, 4 },
	{ 9, 5, 14, 6 },
	{ 10, 13, 12, 0 }
};

extern TextFont *MsgFont;
extern uint16 *FadePalette;
extern BitMap *DispBitMap, *DrawBitMap;
extern CloseDataPtr CPtr;
extern InventoryData *Inventory;
extern uint16 Direction;

#define COMBINATIONUNLOCKED  130
#define BRICKOPEN            115
#define INCL(BITSET,BIT) ((BITSET) |= (BIT))
#define SETBIT(BITSET,BITNUM)   INCL(BITSET, (1 << (BITNUM)))
#define INBIT(BITSET,BITNUM)    ( ((1 << (BITNUM)) & (BITSET)) > 0 )
#define LEFTSCROLL     1
#define RIGHTSCROLL    2
#define UPSCROLL       3
#define DOWNSCROLL     4
#define BRIDGE0   148
#define BRIDGE1   104
#define DIRTY     175
#define NONEWS    135
#define NOCLEAN   152
#define QUARTERNUM           30


static byte *loadBackPict(const char *fileName, bool tomem) {
	byte *res = NULL;

	FadePalette = hipal;
	g_lab->_anim->_noPalChange = true;

	if (tomem)
		res = readPictToMem(fileName, g_lab->_screenWidth, g_lab->_screenHeight);
	else
		readPict(fileName, true);

	for (uint16 i = 0; i < 16; i++) {
		hipal[i] = ((g_lab->_anim->_diffPalette[i * 3] >> 2) << 8) +
		           ((g_lab->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
		           ((g_lab->_anim->_diffPalette[i * 3 + 2] >> 2));
	}

	g_lab->_anim->_noPalChange = false;

	return res;
}

/*****************************************************************************/
/* Draws the images of the combination lock to the display bitmap.           */
/*****************************************************************************/
static void doCombination() {
	for (uint16 i = 0; i <= 5; i++)
		Images[combination[i]]->drawImage(VGAScaleX(combx[i]), VGAScaleY(65));
}

/*****************************************************************************/
/* Reads in a backdrop picture.                                              */
/*****************************************************************************/
void showCombination(const char *filename) {
	byte **buffer;

	resetBuffer();
	g_lab->_anim->_doBlack = true;
	g_lab->_anim->_noPalChange = true;
	readPict(filename, true);
	g_lab->_anim->_noPalChange = false;

	blackScreen();

	buffer = g_lab->_music->newOpen("P:Numbers");

	for (uint16 CurBit = 0; CurBit < 10; CurBit++)
		Images[CurBit] = new Image(buffer);

	allocFile((void **)&g_lab->_tempScrollData, Images[0]->Width * Images[0]->Height * 2L, "tempdata");

	doCombination();

	g_lab->setPalette(g_lab->_anim->_diffPalette, 256);
}



/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
static void changeCombination(uint16 number) {
	Image display;
	uint16 combnum;
	bool unlocked = true;

	if (combination[number] < 9)
		(combination[number])++;
	else
		combination[number] = 0;

	combnum = combination[number];

	display.ImageData = g_lab->getCurrentDrawingBuffer();
	display.Width     = g_lab->_screenWidth;
	display.Height    = g_lab->_screenHeight;

	for (uint16 i = 1; i <= (Images[combnum]->Height / 2); i++) {
		if (g_lab->_isHiRes) {
			if (i & 1)
				g_lab->waitTOF();
		} else
			g_lab->waitTOF();

		display.ImageData = g_lab->getCurrentDrawingBuffer();

		g_lab->scrollDisplayY(2, VGAScaleX(combx[number]), VGAScaleY(65), VGAScaleX(combx[number]) + (Images[combnum])->Width - 1, VGAScaleY(65) + (Images[combnum])->Height);

		g_lab->bltBitMap(Images[combnum], 0, (Images[combnum])->Height - (2 * i), &(display), VGAScaleX(combx[number]), VGAScaleY(65), (Images[combnum])->Width, 2);
	}

	for (uint16 i = 0; i < 6; i++)
		unlocked = (combination[i] == solution[i]) && unlocked;

	if (unlocked)
		g_lab->_conditions->inclElement(COMBINATIONUNLOCKED);
	else
		g_lab->_conditions->exclElement(COMBINATIONUNLOCKED);
}


/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseCombination(Common::Point pos) {
	uint16 number;

	int x = VGAUnScaleX(pos.x);
	int y = VGAUnScaleY(pos.y);

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
		g_lab->setAPen(0);
		g_lab->rectFill(VGAScaleX(97), VGAScaleY(22), VGAScaleX(220), VGAScaleY(126));

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
				Tiles[num]->drawImage(cols + (col * colm), rows + (row * rowm));

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
	uint16 start;
	byte **buffer;

	resetBuffer();
	g_lab->_anim->_doBlack = true;
	g_lab->_anim->_noPalChange = true;
	readPict(filename, true);
	g_lab->_anim->_noPalChange = false;
	blackScreen();

	if (showsolution) {
		start  = 0;
		buffer = g_lab->_music->newOpen("P:TileSolution");
	} else {
		start  = 1;
		buffer = g_lab->_music->newOpen("P:Tile");
	}

	if (!buffer)
		return;

	for (uint16 curBit = start; curBit < 16; curBit++)
		Tiles[curBit] = new Image(buffer);

	allocFile((void **)&g_lab->_tempScrollData, Tiles[1]->Width * Tiles[1]->Height * 2L, "tempdata");

	doTile(showsolution);

	g_lab->setPalette(g_lab->_anim->_diffPalette, 256);
}

static void scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if (dx)
		g_lab->scrollDisplayX(dx, x1, y1, x2, y2);

	if (dy)
		g_lab->scrollDisplayY(dy, x1, y1, x2, y2);
}

/*****************************************************************************/
/* Does the scrolling for the tiles on the tile puzzle.                      */
/*****************************************************************************/
static void doTileScroll(uint16 col, uint16 row, uint16 scrolltype) {
	int16 dX = 0, dY = 0, dx = 0, dy = 0, sx = 0, sy = 0;
	uint16 last = 0, x1, y1;

	if (scrolltype == LEFTSCROLL) {
		dX =  VGAScaleX(5);
		sx =  VGAScaleX(5);
		last = 6;
	} else if (scrolltype == RIGHTSCROLL) {
		dX = VGAScaleX(-5);
		dx = VGAScaleX(-5);
		sx =  VGAScaleX(5);
		last = 6;
	} else if (scrolltype == UPSCROLL) {
		dY =  VGAScaleY(5);
		sy =  VGAScaleY(5);
		last = 5;
	} else if (scrolltype == DOWNSCROLL) {
		dY = VGAScaleY(-5);
		dy = VGAScaleY(-5);
		sy =  VGAScaleY(5);
		last = 5;
	}

	sx += SVGACord(2);

	x1 = VGAScaleX(100) + (col * VGAScaleX(30)) + dx;
	y1 = VGAScaleY(25) + (row * VGAScaleY(25)) + dy;

	for (uint16 i = 0; i < last; i++) {
		g_lab->waitTOF();
		scrollRaster(dX, dY, x1, y1, x1 + VGAScaleX(28) + sx, y1 + VGAScaleY(23) + sy);
		x1 += dX;
		y1 += dY;
	}
}

/*****************************************************************************/
/* Changes the combination number of one of the slots                        */
/*****************************************************************************/
static void changeTile(uint16 col, uint16 row) {
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
		doTileScroll(col, row, scrolltype);

		if (g_lab->getFeatures() & GF_WINDOWS_TRIAL) {
			GUI::MessageDialog trialMessage("This puzzle is not available in the trial version of the game");
			trialMessage.runModal();
			return;
		}

		bool check = true;
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
			g_lab->_anim->_doBlack = true;
			check = readPict("p:Up/BDOpen", true);
		}
	}
}


/*****************************************************************************/
/* Processes mouse clicks and changes the combination.                       */
/*****************************************************************************/
void mouseTile(Common::Point pos) {
	int x = VGAUnScaleX(pos.x);
	int y = VGAUnScaleY(pos.y);

	if ((x < 101) || (y < 26))
		return;

	x = (x - 101) / 30;
	y = (y -  26) / 25;

	if ((x < 4) && (y < 4))
		changeTile(x, y);
}


/*****************************************************************************/
/* Does the things to properly set up the detective notes.                   */
/*****************************************************************************/
void doNotes() {
	TextFont *noteFont = g_lab->_resource->getFont("P:Note.fon");
	char *ntext = g_lab->_resource->getText("Lab:Rooms/Notes");

	flowText(noteFont, -2 + SVGACord(1), 0, 0, false, false, true, true, VGAScaleX(25) + SVGACord(15), VGAScaleY(50), VGAScaleX(295) - SVGACord(15), VGAScaleY(148), ntext);
	g_lab->setPalette(g_lab->_anim->_diffPalette, 256);

	closeFont(noteFont);
	delete[] ntext;
}


/*****************************************************************************/
/* Does the things to properly set up the old west newspaper.  Assumes that  */
/* OpenHiRes already called.                                                 */
/*****************************************************************************/
void doWestPaper() {
	char *ntext;
	TextFont *paperFont;
	int32 FileLen, CharsPrinted;
	uint16 y = 268;

	paperFont = g_lab->_resource->getFont("P:News22.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Date");
	flowText(paperFont, 0, 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(77) + SVGACord(2), VGAScaleX(262), VGAScaleY(91), ntext);
	closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:News32.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Headline");
	FileLen = strlen(ntext) - 1;
	CharsPrinted = flowText(paperFont, -8, 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(86) - SVGACord(2), VGAScaleX(262), VGAScaleY(118), ntext);
	if (CharsPrinted < FileLen) {
		y = 130 - SVGACord(5);
		flowText(paperFont, -8 - SVGACord(1), 0, 0, false, true, false, true, VGAScaleX(57), VGAScaleY(86) - SVGACord(2), VGAScaleX(262), VGAScaleY(132), ntext);
	} else
		y = 115 - SVGACord(5);
	closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:Note.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Col1");
	CharsPrinted = flowText(paperFont, -4, 0, 0, false, false, false, true, VGAScaleX(45), VGAScaleY(y), VGAScaleX(158), VGAScaleY(148), ntext);
	delete[] ntext;
	ntext = g_lab->_resource->getText("Lab:Rooms/Col2");
	CharsPrinted = flowText(paperFont, -4, 0, 0, false, false, false, true, VGAScaleX(162), VGAScaleY(y), VGAScaleX(275), VGAScaleY(148), ntext);
	delete[] ntext;
	closeFont(paperFont);

	g_lab->setPalette(g_lab->_anim->_diffPalette, 256);
	freeAllStolenMem();
}

/*****************************************************************************/
/* Loads in the data for the journal.                                        */
/*****************************************************************************/
static bool loadJournalData() {
	byte **buffer;
	char filename[20];
	Gadget *TopGadget = &BackG;
	bool bridge, dirty, news, clean;

	journalFont = g_lab->_resource->getFont("P:Journal.fon");	// FIXME: memory leak

	g_lab->_music->updateMusic();

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

	journaltext = g_lab->_resource->getText(filename);
	journaltexttitle = g_lab->_resource->getText("Lab:Rooms/jt");

	buffer = g_lab->_music->newOpen("P:JImage");

	if (!buffer)
		return false;

	BackG._image = new Image(buffer);
	BackG._altImage = new Image(buffer);
	ForwardG._image = new Image(buffer);
	ForwardG._altImage = new Image(buffer);
	CancelG._image = new Image(buffer);
	CancelG._altImage = new Image(buffer);

	BackG.KeyEquiv = VKEY_LTARROW;
	ForwardG.KeyEquiv = VKEY_RTARROW;

	uint16 counter = 0;

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
static void drawJournalText() {
	uint16 DrawingToPage = 1;
	int32 CharsDrawn    = 0L;
	char *CurText = journaltext;

	while (DrawingToPage < JPage) {
		g_lab->_music->updateMusic();
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += flowText(journalFont, -2, 2, 0, false, false, false, false, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);

		lastpage = (*CurText == 0);

		if (lastpage)
			JPage = (DrawingToPage / 2) * 2;
		else
			DrawingToPage++;
	}

	if (JPage <= 1) {
		CurText = journaltexttitle;
		flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, true, true, true, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);
	} else {
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, VGAScaleX(52), VGAScaleY(32), VGAScaleX(152), VGAScaleY(148), CurText);
	}

	g_lab->_music->updateMusic();
	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = (*CurText == 0);
	flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, VGAScaleX(171), VGAScaleY(32), VGAScaleX(271), VGAScaleY(148), CurText);

	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = lastpage || (*CurText == 0);
}

/*****************************************************************************/
/* Does the turn page wipe.                                                  */
/*****************************************************************************/
static void turnPage(bool FromLeft) {
	if (FromLeft) {
		for (int i = 0; i < g_lab->_screenWidth; i += 8) {
			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			ScreenImage.ImageData = g_lab->getCurrentDrawingBuffer();
			g_lab->bltBitMap(&JBackImage, i, 0, &ScreenImage, i, 0, 8, g_lab->_screenHeight);
		}
	} else {
		for (int i = (g_lab->_screenWidth - 8); i > 0; i -= 8) {
			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			ScreenImage.ImageData = g_lab->getCurrentDrawingBuffer();
			g_lab->bltBitMap(&JBackImage, i, 0, &ScreenImage, i, 0, 8, g_lab->_screenHeight);
		}
	}
}


/*****************************************************************************/
/* Draws the journal from page x.                                            */
/*****************************************************************************/
void LabEngine::drawJournal(uint16 wipenum, bool needFade) {
	_event->mouseHide();

	_music->updateMusic();

	if (!GotBackImage)
		JBackImage.ImageData = loadBackPict("P:Journal.pic", true);

	drawJournalText();

	ScreenImage.ImageData = getCurrentDrawingBuffer();

	if (wipenum == 0)
		bltBitMap(&JBackImage, 0, 0, &ScreenImage, 0, 0, _screenWidth, _screenHeight);
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


	if (needFade)
		fade(true, 0);

	g_lab->_anim->_noPalChange = true;
	JBackImage.ImageData = readPictToMem("P:Journal.pic", _screenWidth, _screenHeight);
	GotBackImage = true;

	eatMessages();
	_event->mouseShow();

	g_lab->_anim->_noPalChange = false;
}

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
void LabEngine::processJournal() {
	IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, GadID;

	while (1) {
		_music->updateMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			_music->updateMusic();
		} else {
			Class     = Msg->msgClass;
			Qualifier = Msg->qualifier;
			GadID     = Msg->code;

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
void LabEngine::doJournal() {
	resetBuffer();
	blackAllScreen();

	lastpage    = false;
	GotBackImage = false;

	JBackImage.Width = _screenWidth;
	JBackImage.Height = _screenHeight;
	JBackImage.ImageData   = NULL;

	BackG.NextGadget = &CancelG;
	CancelG.NextGadget = &ForwardG;

	ScreenImage = JBackImage;
	ScreenImage.ImageData = getCurrentDrawingBuffer();

	_music->updateMusic();
	loadJournalData();

	drawJournal(0, true);

	_event->attachGadgetList(&BackG);
	_event->mouseShow();
	processJournal();
	_event->attachGadgetList(NULL);
	fade(false, 0);
	_event->mouseHide();

	ScreenImage.ImageData = getCurrentDrawingBuffer();

	setAPen(0);
	rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);
	blackScreen();

	freeAllStolenMem();
}

bool saveRestoreGame() {
	bool isOK = false;

	//g_lab->showMainMenu();

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

			isOK = saveGame(Direction, Inventory[QUARTERNUM].Many, slot, desc);
		}
	} else {
		// Restore
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			isOK = loadGame(&Direction, &(Inventory[QUARTERNUM].Many), slot);
			if (isOK)
				g_lab->_music->resetMusic();
		}
	}

	g_lab->screenUpdate();

	return isOK;
}

/*****************************************************************************/
/* Makes sure that the buttons are in memory.                                */
/*****************************************************************************/
static void getMonImages() {
	byte **buffer;
	uint32 bufferSize;

	resetBuffer();

	buffer = g_lab->_music->newOpen("P:MonImage", bufferSize);

	if (!buffer)
		return;

	MonButton = new Image(buffer);

	stealBufMem(bufferSize);  /* Trick: protects the memory where the buttons are so they won't be over-written */
}


/*****************************************************************************/
/* Draws the text for the monitor.                                           */
/*****************************************************************************/
void LabEngine::drawMonText(char *text, TextFont *monitorFont, uint16 x1, uint16 y1, uint16 x2, uint16 y2, bool isinteractive) {
	uint16 DrawingToPage = 0, yspacing = 0, numlines, fheight;
	int32 CharsDrawn    = 0L;
	char *CurText = text;

	_event->mouseHide();

	if (*text == '%') {
		text++;
		numlines = (*text - '0') * 10;
		text++;
		numlines += (*text - '0');
		text += 2;

		fheight = textHeight(monitorFont);
		x1 = MonButton->Width + VGAScaleX(3);
		MonGadHeight = MonButton->Height + VGAScaleY(3);

		if (MonGadHeight > fheight)
			yspacing = MonGadHeight - fheight;
		else
			MonGadHeight = fheight;

		setAPen(0);
		rectFill(0, 0, _screenWidth - 1, y2);

		for (uint16 i = 0; i < numlines; i++)
			MonButton->drawImage(0, i * MonGadHeight);
	} else if (isinteractive) {
		setAPen(0);
		rectFill(0, 0, _screenWidth - 1, y2);
	} else {
		setAPen(0);
		rectFill(x1, y1, x2, y2);
	}

	while (DrawingToPage < monitorPage) {
		_music->updateMusic();
		CurText = (char *)(text + CharsDrawn);
		CharsDrawn += flowText(monitorFont, yspacing, 0, 0, false, false, false, false, x1, y1, x2, y2, CurText);
		lastpage = (*CurText == 0);

		if (lastpage)
			monitorPage = DrawingToPage;
		else
			DrawingToPage++;
	}

	CurText = (char *)(text + CharsDrawn);
	lastpage = (*CurText == 0);
	CharsDrawn = flowText(monitorFont, yspacing, 2, 0, false, false, false, true, x1, y1, x2, y2, CurText);
	CurText += CharsDrawn;
	lastpage = lastpage || (*CurText == 0);

	_event->mouseShow();
}

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
void LabEngine::processMonitor(char *ntext, TextFont *monitorFont, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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
				monitorPage      = 0;
				TextFileName = Test;

				ntext = g_lab->_resource->getText(TextFileName);
				fade(false, 0);
				drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
				fade(true, 0);
				delete[] ntext;
			}
		}

		_music->updateMusic();  /* Make sure we check the music at least after every message */
		Msg = getMsg();

		if (Msg == NULL) {
			_music->updateMusic();
		} else {
			Class     = Msg->msgClass;
			Qualifier = Msg->qualifier;
			MouseX    = Msg->mouseX;
			MouseY    = Msg->mouseY;
			Code      = Msg->code;

			if (((Class == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & Qualifier)) ||
			        ((Class == RAWKEY) && (Code == 27)))
				return;

			else if ((Class == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & Qualifier)) {
				if ((MouseY >= VGAScaleY(171)) && (MouseY <= VGAScaleY(200))) {
					if ((MouseX >= VGAScaleX(259)) && (MouseX <= VGAScaleX(289))) {
						if (!lastpage) {
							monitorPage += 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= VGAScaleX(0)) && (MouseX <= VGAScaleX(31))) {
						return;
					} else if ((MouseX >= VGAScaleX(290)) && (MouseX <= VGAScaleX(320))) {
						if (monitorPage >= 1) {
							monitorPage -= 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= VGAScaleX(31)) && (MouseX <= VGAScaleX(59))) {
						if (isinteractive) {
							monitorPage = 0;

							if (depth) {
								depth--;
								CPtr = LastCPtr[depth];
							}
						} else if (monitorPage > 0) {
							monitorPage = 0;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					}
				} else if (isinteractive) {
					TestCPtr = CPtr;
					MouseY = 64 + (MouseY / MonGadHeight) * 42;
					MouseX = 101;
					setCurClose(Common::Point(MouseX, MouseY), &CPtr, true);

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
void LabEngine::doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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
	monitorPage = 0;
	lastpage = false;
	FadePalette = hipal;

	TextFont *monitorFont = _resource->getFont("P:Map.fon");
	getMonImages();

	ntext = _resource->getText(textfile);
	loadBackPict(background, false);
	drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
	_event->mouseShow();
	fade(true, 0);
	processMonitor(ntext, monitorFont, isinteractive, x1, y1, x2, y2);
	fade(false, 0);
	_event->mouseHide();
	delete[] ntext;
	closeFont(monitorFont);
	freeAllStolenMem();

	setAPen(0);
	rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);
	blackAllScreen();
}

} // End of namespace Lab
