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
#include "lab/parsetypes.h"
#include "lab/resource.h"

namespace Lab {

static uint16 MonGadHeight = 1;
static uint16 hipal[20];

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

Image *MonButton;

extern uint16 *FadePalette;
extern BitMap *DispBitMap, *DrawBitMap;
extern uint16 Direction;

#define INCL(BITSET,BIT) ((BITSET) |= (BIT))
#define SETBIT(BITSET,BITNUM)   INCL(BITSET, (1 << (BITNUM)))
#define INBIT(BITSET,BITNUM)    ( ((1 << (BITNUM)) & (BITSET)) > 0 )
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
		res = g_lab->_graphics->readPictToMem(fileName, g_lab->_screenWidth, g_lab->_screenHeight);
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

/*****************************************************************************/
/* Does the things to properly set up the detective notes.                   */
/*****************************************************************************/
void doNotes() {
	TextFont *noteFont = g_lab->_resource->getFont("P:Note.fon");
	char *ntext = g_lab->_resource->getText("Lab:Rooms/Notes");

	g_lab->_graphics->flowText(noteFont, -2 + g_lab->_graphics->SVGACord(1), 0, 0, false, false, true, true, g_lab->_graphics->VGAScaleX(25) + g_lab->_graphics->SVGACord(15), g_lab->_graphics->VGAScaleY(50), g_lab->_graphics->VGAScaleX(295) - g_lab->_graphics->SVGACord(15), g_lab->_graphics->VGAScaleY(148), ntext);
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
	g_lab->_graphics->flowText(paperFont, 0, 0, 0, false, true, false, true, g_lab->_graphics->VGAScaleX(57), g_lab->_graphics->VGAScaleY(77) + g_lab->_graphics->SVGACord(2), g_lab->_graphics->VGAScaleX(262), g_lab->_graphics->VGAScaleY(91), ntext);
	closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:News32.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Headline");
	FileLen = strlen(ntext) - 1;
	CharsPrinted = g_lab->_graphics->flowText(paperFont, -8, 0, 0, false, true, false, true, g_lab->_graphics->VGAScaleX(57), g_lab->_graphics->VGAScaleY(86) - g_lab->_graphics->SVGACord(2), g_lab->_graphics->VGAScaleX(262), g_lab->_graphics->VGAScaleY(118), ntext);
	if (CharsPrinted < FileLen) {
		y = 130 - g_lab->_graphics->SVGACord(5);
		g_lab->_graphics->flowText(paperFont, -8 - g_lab->_graphics->SVGACord(1), 0, 0, false, true, false, true, g_lab->_graphics->VGAScaleX(57), g_lab->_graphics->VGAScaleY(86) - g_lab->_graphics->SVGACord(2), g_lab->_graphics->VGAScaleX(262), g_lab->_graphics->VGAScaleY(132), ntext);
	} else
		y = 115 - g_lab->_graphics->SVGACord(5);
	closeFont(paperFont);
	delete[] ntext;

	paperFont = g_lab->_resource->getFont("P:Note.fon");
	ntext = g_lab->_resource->getText("Lab:Rooms/Col1");
	CharsPrinted = g_lab->_graphics->flowText(paperFont, -4, 0, 0, false, false, false, true, g_lab->_graphics->VGAScaleX(45), g_lab->_graphics->VGAScaleY(y), g_lab->_graphics->VGAScaleX(158), g_lab->_graphics->VGAScaleY(148), ntext);
	delete[] ntext;
	ntext = g_lab->_resource->getText("Lab:Rooms/Col2");
	CharsPrinted = g_lab->_graphics->flowText(paperFont, -4, 0, 0, false, false, false, true, g_lab->_graphics->VGAScaleX(162), g_lab->_graphics->VGAScaleY(y), g_lab->_graphics->VGAScaleX(275), g_lab->_graphics->VGAScaleY(148), ntext);
	delete[] ntext;
	closeFont(paperFont);

	g_lab->setPalette(g_lab->_anim->_diffPalette, 256);
}

/*****************************************************************************/
/* Loads in the data for the journal.                                        */
/*****************************************************************************/
static bool loadJournalData() {
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

	Common::File *journalFile = g_lab->_resource->openDataFile("P:JImage");

	BackG._image = new Image(journalFile);
	BackG._altImage = new Image(journalFile);
	ForwardG._image = new Image(journalFile);
	ForwardG._altImage = new Image(journalFile);
	CancelG._image = new Image(journalFile);
	CancelG._altImage = new Image(journalFile);

	delete journalFile;

	BackG.KeyEquiv = VKEY_LTARROW;
	ForwardG.KeyEquiv = VKEY_RTARROW;

	uint16 counter = 0;

	while (TopGadget) {
		TopGadget->x = g_lab->_graphics->VGAScaleX(JGadX[counter]);

		if (counter == 1)
			TopGadget->y = g_lab->_graphics->VGAScaleY(JGadY[counter]) + g_lab->_graphics->SVGACord(1);
		else
			TopGadget->y = g_lab->_graphics->VGAScaleY(JGadY[counter]) - g_lab->_graphics->SVGACord(1);

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
		CharsDrawn += g_lab->_graphics->flowText(journalFont, -2, 2, 0, false, false, false, false, g_lab->_graphics->VGAScaleX(52), g_lab->_graphics->VGAScaleY(32), g_lab->_graphics->VGAScaleX(152), g_lab->_graphics->VGAScaleY(148), CurText);

		lastpage = (*CurText == 0);

		if (lastpage)
			JPage = (DrawingToPage / 2) * 2;
		else
			DrawingToPage++;
	}

	if (JPage <= 1) {
		CurText = journaltexttitle;
		g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, true, true, true, g_lab->_graphics->VGAScaleX(52), g_lab->_graphics->VGAScaleY(32), g_lab->_graphics->VGAScaleX(152), g_lab->_graphics->VGAScaleY(148), CurText);
	} else {
		CurText = (char *)(journaltext + CharsDrawn);
		CharsDrawn += g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, g_lab->_graphics->VGAScaleX(52), g_lab->_graphics->VGAScaleY(32), g_lab->_graphics->VGAScaleX(152), g_lab->_graphics->VGAScaleY(148), CurText);
	}

	g_lab->_music->updateMusic();
	CurText = (char *)(journaltext + CharsDrawn);
	lastpage = (*CurText == 0);
	g_lab->_graphics->flowTextToMem(&JBackImage, journalFont, -2, 2, 0, false, false, false, true, g_lab->_graphics->VGAScaleX(171), g_lab->_graphics->VGAScaleY(32), g_lab->_graphics->VGAScaleX(271), g_lab->_graphics->VGAScaleY(148), CurText);

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
			ScreenImage._imageData = g_lab->getCurrentDrawingBuffer();
			JBackImage.blitBitmap(i, 0, &ScreenImage, i, 0, 8, g_lab->_screenHeight, false);
		}
	} else {
		for (int i = (g_lab->_screenWidth - 8); i > 0; i -= 8) {
			g_lab->_music->updateMusic();
			g_lab->waitTOF();
			ScreenImage._imageData = g_lab->getCurrentDrawingBuffer();
			JBackImage.blitBitmap(i, 0, &ScreenImage, i, 0, 8, g_lab->_screenHeight, false);
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
		JBackImage._imageData = loadBackPict("P:Journal.pic", true);

	drawJournalText();

	ScreenImage._imageData = getCurrentDrawingBuffer();

	if (wipenum == 0)
		JBackImage.blitBitmap(0, 0, &ScreenImage, 0, 0, _screenWidth, _screenHeight, false);
	else
		turnPage((bool)(wipenum == 1));

	if (JPage == 0)
		disableGadget(&BackG, 15);
	else
		enableGadget(&BackG);

	if (lastpage)
		disableGadget(&ForwardG, 15);
	else
		enableGadget(&ForwardG);


	if (needFade)
		fade(true, 0);

	g_lab->_anim->_noPalChange = true;
	JBackImage._imageData = _graphics->readPictToMem("P:Journal.pic", _screenWidth, _screenHeight);
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
	_graphics->blackAllScreen();

	lastpage    = false;
	GotBackImage = false;

	JBackImage._width = _screenWidth;
	JBackImage._height = _screenHeight;
	JBackImage._imageData   = NULL;

	BackG.NextGadget = &CancelG;
	CancelG.NextGadget = &ForwardG;

	ScreenImage = JBackImage;
	ScreenImage._imageData = getCurrentDrawingBuffer();

	_music->updateMusic();
	loadJournalData();

	drawJournal(0, true);

	_event->attachGadgetList(&BackG);
	_event->mouseShow();
	processJournal();
	_event->attachGadgetList(NULL);
	fade(false, 0);
	_event->mouseHide();

	ScreenImage._imageData = getCurrentDrawingBuffer();

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);
	_graphics->blackScreen();
}

bool LabEngine::saveRestoreGame() {
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

			isOK = saveGame(Direction, _inventory[QUARTERNUM].Many, slot, desc);
		}
	} else {
		// Restore
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int slot = dialog->runModalWithCurrentTarget();
		if (slot >= 0) {
			isOK = loadGame(&Direction, &(_inventory[QUARTERNUM].Many), slot);
			if (isOK)
				_music->resetMusic();
		}
	}

	_graphics->screenUpdate();

	return isOK;
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
		x1 = MonButton->_width + _graphics->VGAScaleX(3);
		MonGadHeight = MonButton->_height + _graphics->VGAScaleY(3);

		if (MonGadHeight > fheight)
			yspacing = MonGadHeight - fheight;
		else
			MonGadHeight = fheight;

		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _screenWidth - 1, y2);

		for (uint16 i = 0; i < numlines; i++)
			MonButton->drawImage(0, i * MonGadHeight);
	} else if (isinteractive) {
		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _screenWidth - 1, y2);
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

/*****************************************************************************/
/* Processes user input.                                                     */
/*****************************************************************************/
void LabEngine::processMonitor(char *ntext, TextFont *monitorFont, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	IntuiMessage *Msg;
	uint32 Class;
	uint16 Qualifier, Code, MouseX, MouseY;
	const char *Test = " ", *StartFileName = TextFileName;
	CloseDataPtr startcptr = _cptr, testcptr, lastcptr[10];
	uint16 depth = 0;

	lastcptr[0] = _cptr;

	while (1) {
		if (isinteractive) {
			if (_cptr == NULL)
				_cptr = startcptr;

			if (_cptr == startcptr)
				Test = StartFileName;
			else
				Test = _cptr->GraphicName;

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
				if ((MouseY >= g_lab->_graphics->VGAScaleY(171)) && (MouseY <= g_lab->_graphics->VGAScaleY(200))) {
					if ((MouseX >= g_lab->_graphics->VGAScaleX(259)) && (MouseX <= g_lab->_graphics->VGAScaleX(289))) {
						if (!lastpage) {
							monitorPage += 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= g_lab->_graphics->VGAScaleX(0)) && (MouseX <= g_lab->_graphics->VGAScaleX(31))) {
						return;
					} else if ((MouseX >= g_lab->_graphics->VGAScaleX(290)) && (MouseX <= g_lab->_graphics->VGAScaleX(320))) {
						if (monitorPage >= 1) {
							monitorPage -= 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					} else if ((MouseX >= g_lab->_graphics->VGAScaleX(31)) && (MouseX <= g_lab->_graphics->VGAScaleX(59))) {
						if (isinteractive) {
							monitorPage = 0;

							if (depth) {
								depth--;
								_cptr = lastcptr[depth];
							}
						} else if (monitorPage > 0) {
							monitorPage = 0;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
						}
					}
				} else if (isinteractive) {
					testcptr = _cptr;
					MouseY = 64 + (MouseY / MonGadHeight) * 42;
					MouseX = 101;
					setCurClose(Common::Point(MouseX, MouseY), &_cptr, true);

					if (testcptr != _cptr) {
						lastcptr[depth] = testcptr;
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

	x1 = _graphics->VGAScaleX(x1);
	x2 = _graphics->VGAScaleX(x2);
	y1 = _graphics->VGAScaleY(y1);
	y2 = _graphics->VGAScaleY(y2);

	TextFileName = textfile;

	_graphics->blackAllScreen();
	_graphics->readPict("P:Mon/Monitor.1", true);
	_graphics->readPict("P:Mon/NWD1", true);
	_graphics->readPict("P:Mon/NWD2", true);
	_graphics->readPict("P:Mon/NWD3", true);
	_graphics->blackAllScreen();

	monitorPage = 0;
	lastpage = false;
	FadePalette = hipal;

	TextFont *monitorFont = _resource->getFont("P:Map.fon");
	Common::File *buttonFile = g_lab->_resource->openDataFile("P:MonImage");
	MonButton = new Image(buttonFile);
	delete buttonFile;

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

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _screenWidth - 1, _screenHeight - 1);
	_graphics->blackAllScreen();
	_graphics->freePict();
}

} // End of namespace Lab
