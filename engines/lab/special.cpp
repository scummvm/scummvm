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

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/interface.h"
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

static uint16 MonGadHeight = 1;
static uint16 hipal[20];

static TextFont *journalFont;
static char *journaltext, *journaltexttitle;
static uint16 JPage = 0;
static bool lastpage = false;
static Image JBackImage, ScreenImage;
static bool GotBackImage = false;
static uint16 monitorPage;
static const char *TextFileName;

Image *MonButton;

#define INCL(BITSET,BIT) ((BITSET) |= (BIT))
#define SETBIT(BITSET,BITNUM)   INCL(BITSET, (1 << (BITNUM)))
#define INBIT(BITSET,BITNUM)    ( ((1 << (BITNUM)) & (BITSET)) > 0 )
#define BRIDGE0     148
#define BRIDGE1     104
#define DIRTY       175
#define NONEWS      135
#define NOCLEAN     152


static byte *loadBackPict(const char *fileName, bool tomem) {
	byte *res = nullptr;

	g_lab->_graphics->FadePalette = hipal;
	g_lab->_anim->_noPalChange = true;

	if (tomem)
		res = g_lab->_graphics->readPictToMem(fileName, g_lab->_graphics->_screenWidth, g_lab->_graphics->_screenHeight);
	else
		g_lab->_graphics->readPict(fileName, true);

	for (uint16 i = 0; i < 16; i++) {
		hipal[i] = ((g_lab->_anim->_diffPalette[i * 3] >> 2) << 8) +
				((g_lab->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
				((g_lab->_anim->_diffPalette[i * 3 + 2] >> 2));
	}

	g_lab->_anim->_noPalChange = false;

	return res;
}

/**
 * Does the things to properly set up the detective notes.
 */
void LabEngine::doNotes() {
	TextFont *noteFont = g_lab->_resource->getFont("P:Note.fon");
	char *ntext = g_lab->_resource->getText("Lab:Rooms/Notes");

	g_lab->_graphics->flowText(noteFont, -2 + g_lab->_utils->svgaCord(1), 0, 0, false, false, true, true, g_lab->_utils->vgaScaleX(25) + g_lab->_utils->svgaCord(15), g_lab->_utils->vgaScaleY(50), g_lab->_utils->vgaScaleX(295) - g_lab->_utils->svgaCord(15), g_lab->_utils->vgaScaleY(148), ntext);
	g_lab->_graphics->setPalette(g_lab->_anim->_diffPalette, 256);

	g_lab->_graphics->closeFont(noteFont);
	delete[] ntext;
}


/**
 * Does the things to properly set up the old west newspaper.  Assumes that
 * OpenHiRes already called.
 */
void LabEngine::doWestPaper() {
	char *ntext;
	TextFont *paperFont;
	int32 FileLen, CharsPrinted;
	uint16 y = 268;

	paperFont = g_lab->_resource->getFont("P:News22.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Date");
	g_lab->_graphics->flowText(paperFont, 0, 0, 0, false, true, false, true, g_lab->_utils->vgaScaleX(57), g_lab->_utils->vgaScaleY(77) + g_lab->_utils->svgaCord(2), g_lab->_utils->vgaScaleX(262), g_lab->_utils->vgaScaleY(91), ntext);
	g_lab->_graphics->closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:News32.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Headline");
	FileLen = strlen(ntext) - 1;
	CharsPrinted = g_lab->_graphics->flowText(paperFont, -8, 0, 0, false, true, false, true, g_lab->_utils->vgaScaleX(57), g_lab->_utils->vgaScaleY(86) - g_lab->_utils->svgaCord(2), g_lab->_utils->vgaScaleX(262), g_lab->_utils->vgaScaleY(118), ntext);
	if (CharsPrinted < FileLen) {
		y = 130 - g_lab->_utils->svgaCord(5);
		g_lab->_graphics->flowText(paperFont, -8 - g_lab->_utils->svgaCord(1), 0, 0, false, true, false, true, g_lab->_utils->vgaScaleX(57), g_lab->_utils->vgaScaleY(86) - g_lab->_utils->svgaCord(2), g_lab->_utils->vgaScaleX(262), g_lab->_utils->vgaScaleY(132), ntext);
	} else
		y = 115 - g_lab->_utils->svgaCord(5);
	g_lab->_graphics->closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:Note.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Col1");
	CharsPrinted = g_lab->_graphics->flowTextScaled(paperFont, -4, 0, 0, false, false, false, true, 45, y, 158, 148, ntext);
	delete[] ntext;
	ntext = g_lab->_resource->getText("Lab:Rooms/Col2");
	CharsPrinted = g_lab->_graphics->flowTextScaled(paperFont, -4, 0, 0, false, false, false, true, 162, y, 275, 148, ntext);
	delete[] ntext;
	g_lab->_graphics->closeFont(paperFont);

	g_lab->_graphics->setPalette(g_lab->_anim->_diffPalette, 256);
}

/**
 * Loads in the data for the journal.
 */
void LabEngine::loadJournalData() {
	char filename[20];
	bool bridge, dirty, news, clean;

	journalFont = g_lab->_resource->getFont("P:Journal.fon");

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

	Common::File *journalFile = g_lab->_resource->openDataFile("P:JImage");
	Utils *utils = g_lab->_utils;
	_journalGadgetList.push_back(createButton( 80, utils->vgaScaleY(162) + utils->svgaCord(1), 0, VKEY_LTARROW, new Image(journalFile), new Image(journalFile)));	// back
	_journalGadgetList.push_back(createButton(144, utils->vgaScaleY(164) - utils->svgaCord(1), 1, VKEY_RTARROW, new Image(journalFile), new Image(journalFile)));	// foward
	_journalGadgetList.push_back(createButton(194, utils->vgaScaleY(162) + utils->svgaCord(1), 2,            0, new Image(journalFile), new Image(journalFile)));	// cancel
	delete journalFile;
}

/**
 * Draws the text to the back journal screen to the appropriate Page number
 */
static void drawJournalText() {
	uint16 DrawingToPage = 1;
	int32 CharsDrawn    = 0L;
	char *CurText = journaltext;

	while (DrawingToPage < JPage) {
		g_lab->_music->updateMusic();
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += g_lab->_graphics->flowTextScaled(journalFont, -2, 2, 0, false, false, false, false, 52, 32, 152, 148, CurText);

		lastpage = (*CurText == 0);

		if (lastpage)
			JPage = (DrawingToPage / 2) * 2;
		else
			DrawingToPage++;
	}

	if (JPage <= 1) {
		CurText = journaltexttitle;
		g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, true, true, true, g_lab->_utils->vgaScaleX(52), g_lab->_utils->vgaScaleY(32), g_lab->_utils->vgaScaleX(152), g_lab->_utils->vgaScaleY(148), CurText);
	} else {
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, g_lab->_utils->vgaScaleX(52), g_lab->_utils->vgaScaleY(32), g_lab->_utils->vgaScaleX(152), g_lab->_utils->vgaScaleY(148), CurText);
	}

	g_lab->_music->updateMusic();
	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = (*CurText == 0);
	g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, g_lab->_utils->vgaScaleX(171), g_lab->_utils->vgaScaleY(32), g_lab->_utils->vgaScaleX(271), g_lab->_utils->vgaScaleY(148), CurText);

	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = lastpage || (*CurText == 0);
}

/**
 * Does the turn page wipe.
 */
static void turnPage(bool FromLeft) {
	if (FromLeft) {
		for (int i = 0; i < g_lab->_graphics->_screenWidth; i += 8) {
			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			ScreenImage._imageData = g_lab->_graphics->getCurrentDrawingBuffer();
			JBackImage.blitBitmap(i, 0, &ScreenImage, i, 0, 8, g_lab->_graphics->_screenHeight, false);
		}
	} else {
		for (int i = (g_lab->_graphics->_screenWidth - 8); i > 0; i -= 8) {
			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			ScreenImage._imageData = g_lab->_graphics->getCurrentDrawingBuffer();
			JBackImage.blitBitmap(i, 0, &ScreenImage, i, 0, 8, g_lab->_graphics->_screenHeight, false);
		}
	}
}

/**
 * Draws the journal from page x.
 */
void LabEngine::drawJournal(uint16 wipenum, bool needFade) {
	_event->mouseHide();

	_music->updateMusic();

	if (!GotBackImage)
		JBackImage._imageData = loadBackPict("P:Journal.pic", true);

	drawJournalText();

	ScreenImage._imageData = _graphics->getCurrentDrawingBuffer();

	if (wipenum == 0)
		JBackImage.blitBitmap(0, 0, &ScreenImage, 0, 0, _graphics->_screenWidth, _graphics->_screenHeight, false);
	else
		turnPage((bool)(wipenum == 1));

	Gadget *backGadget = _event->getGadget(0);
	Gadget *forwardGadget = _event->getGadget(1);

	if (JPage == 0)
		disableGadget(backGadget, 15);
	else
		enableGadget(backGadget);

	if (lastpage)
		disableGadget(forwardGadget, 15);
	else
		enableGadget(forwardGadget);


	if (needFade)
		_graphics->fade(true, 0);

	g_lab->_anim->_noPalChange = true;
	JBackImage._imageData = _graphics->readPictToMem("P:Journal.pic", _graphics->_screenWidth, _graphics->_screenHeight);
	GotBackImage = true;

	eatMessages();
	_event->mouseShow();

	g_lab->_anim->_noPalChange = false;
}

/**
 * Processes user input.
 */
void LabEngine::processJournal() {
	IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, GadID;

	while (1) {
		// Make sure we check the music at least after every message
		_music->updateMusic();
		Msg = getMsg();

		if (Msg == NULL) {
			_music->updateMusic();
		} else {
			Class     = Msg->_msgClass;
			Qualifier = Msg->_qualifier;
			GadID     = Msg->_code;

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

/**
 * Does the journal processing.
 */
void LabEngine::doJournal() {
	_graphics->blackAllScreen();

	lastpage    = false;
	GotBackImage = false;

	JBackImage._width = _graphics->_screenWidth;
	JBackImage._height = _graphics->_screenHeight;
	JBackImage._imageData = NULL;

	ScreenImage = JBackImage;
	ScreenImage._imageData = _graphics->getCurrentDrawingBuffer();

	_music->updateMusic();
	loadJournalData();

	drawJournal(0, true);

	_event->attachGadgetList(&_journalGadgetList);
	_event->mouseShow();
	processJournal();
	_event->attachGadgetList(NULL);
	_graphics->fade(false, 0);
	_event->mouseHide();

	freeButtonList(&_journalGadgetList);
	g_lab->_graphics->closeFont(journalFont);

	ScreenImage._imageData = _graphics->getCurrentDrawingBuffer();

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	_graphics->blackScreen();
}

/**
 * Draws the text for the monitor.
 */
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

		fheight = g_lab->_graphics->textHeight(monitorFont);
		x1 = MonButton->_width + _utils->vgaScaleX(3);
		MonGadHeight = MonButton->_height + _utils->vgaScaleY(3);

		if (MonGadHeight > fheight)
			yspacing = MonGadHeight - fheight;
		else
			MonGadHeight = fheight;

		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, y2);

		for (uint16 i = 0; i < numlines; i++)
			MonButton->drawImage(0, i * MonGadHeight);
	} else if (isinteractive) {
		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, y2);
	} else {
		_graphics->setAPen(0);
		_graphics->rectFill(x1, y1, x2, y2);
	}

	while (DrawingToPage < monitorPage) {
		_music->updateMusic();
		CurText = (char *)(text + CharsDrawn);
		CharsDrawn += _graphics->flowText(monitorFont, yspacing, 0, 0, false, false, false, false, x1, y1, x2, y2, CurText);
		lastpage = (*CurText == 0);

		if (lastpage)
			monitorPage = DrawingToPage;
		else
			DrawingToPage++;
	}

	CurText = (char *)(text + CharsDrawn);
	lastpage = (*CurText == 0);
	CharsDrawn = _graphics->flowText(monitorFont, yspacing, 2, 0, false, false, false, true, x1, y1, x2, y2, CurText);
	CurText += CharsDrawn;
	lastpage = lastpage || (*CurText == 0);

	_event->mouseShow();
}

/**
 * Processes user input.
 */
void LabEngine::processMonitor(char *ntext, TextFont *monitorFont, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	IntuiMessage *msg;
	uint32 msgClass;
	uint16 qualifier, code, mouseX, mouseY;
	const char *Test = " ", *StartFileName = TextFileName;
	CloseDataPtr startClosePtr = _closeDataPtr, lastClosePtr[10];
	uint16 depth = 0;

	lastClosePtr[0] = _closeDataPtr;

	while (1) {
		if (isinteractive) {
			if (_closeDataPtr == NULL)
				_closeDataPtr = startClosePtr;

			if (_closeDataPtr == startClosePtr)
				Test = StartFileName;
			else
				Test = _closeDataPtr->_graphicName;

			if (strcmp(Test, TextFileName)) {
				monitorPage      = 0;
				TextFileName = Test;

				ntext = g_lab->_resource->getText(TextFileName);
				_graphics->fade(false, 0);
				drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
				_graphics->fade(true, 0);
				delete[] ntext;
			}
		}

		// Make sure we check the music at least after every message
		_music->updateMusic();
		msg = getMsg();

		if (msg == NULL) {
			_music->updateMusic();
		} else {
			msgClass     = msg->_msgClass;
			qualifier = msg->_qualifier;
			mouseX    = msg->_mouseX;
			mouseY    = msg->_mouseY;
			code      = msg->_code;

			if (((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & qualifier)) ||
				  ((msgClass == RAWKEY) && (code == 27)))
				return;

			else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & qualifier)) {
				if ((mouseY >= g_lab->_utils->vgaScaleY(171)) && (mouseY <= g_lab->_utils->vgaScaleY(200))) {
					if ((mouseX >= g_lab->_utils->vgaScaleX(259)) && (mouseX <= g_lab->_utils->vgaScaleX(289))) {
						if (!lastpage) {
							monitorPage += 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((mouseX >= g_lab->_utils->vgaScaleX(0)) && (mouseX <= g_lab->_utils->vgaScaleX(31))) {
						return;
					} else if ((mouseX >= g_lab->_utils->vgaScaleX(290)) && (mouseX <= g_lab->_utils->vgaScaleX(320))) {
						if (monitorPage >= 1) {
							monitorPage -= 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((mouseX >= g_lab->_utils->vgaScaleX(31)) && (mouseX <= g_lab->_utils->vgaScaleX(59))) {
						if (isinteractive) {
							monitorPage = 0;

							if (depth) {
								depth--;
								_closeDataPtr = lastClosePtr[depth];
							}
						} else if (monitorPage > 0) {
							monitorPage = 0;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					}
				} else if (isinteractive) {
					CloseDataPtr tmpClosePtr = _closeDataPtr;
					mouseY = 64 + (mouseY / MonGadHeight) * 42;
					mouseX = 101;
					setCurrentClose(Common::Point(mouseX, mouseY), &_closeDataPtr, false);

					if (tmpClosePtr != _closeDataPtr) {
						lastClosePtr[depth] = tmpClosePtr;
						depth++;
					}
				}
			}
		}
	}
}

/**
 * Does what's necessary for the monitor.
 */
void LabEngine::doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	char *ntext;

	x1 = _utils->vgaScaleX(x1);
	x2 = _utils->vgaScaleX(x2);
	y1 = _utils->vgaScaleY(y1);
	y2 = _utils->vgaScaleY(y2);

	TextFileName = textfile;

	_graphics->blackAllScreen();
	_graphics->readPict("P:Mon/Monitor.1", true);
	_graphics->readPict("P:Mon/NWD1", true);
	_graphics->readPict("P:Mon/NWD2", true);
	_graphics->readPict("P:Mon/NWD3", true);
	_graphics->blackAllScreen();

	monitorPage = 0;
	lastpage = false;
	_graphics->FadePalette = hipal;

	TextFont *monitorFont = _resource->getFont("P:Map.fon");
	Common::File *buttonFile = g_lab->_resource->openDataFile("P:MonImage");
	MonButton = new Image(buttonFile);
	delete buttonFile;

	ntext = _resource->getText(textfile);
	loadBackPict(background, false);
	drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
	_event->mouseShow();
	_graphics->fade(true, 0);
	processMonitor(ntext, monitorFont, isinteractive, x1, y1, x2, y2);
	_graphics->fade(false, 0);
	_event->mouseHide();
	delete[] ntext;
	g_lab->_graphics->closeFont(monitorFont);

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	_graphics->blackAllScreen();
	_graphics->freePict();
}

} // End of namespace Lab
