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
#include "lab/labsets.h"
#include "lab/music.h"
#include "lab/processroom.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {
#define BRIDGE0     148
#define BRIDGE1     104
#define DIRTY       175
#define NONEWS      135
#define NOCLEAN     152

/**
 * Does the things to properly set up the detective notes.
 */
void LabEngine::doNotes() {
	TextFont *noteFont = _resource->getFont("P:Note.fon");
	char *ntext = _resource->getText("Lab:Rooms/Notes");

	_graphics->flowText(noteFont, -2 + _utils->svgaCord(1), 0, 0, false, false, true, true, _utils->vgaScaleX(25) + _utils->svgaCord(15), _utils->vgaScaleY(50), _utils->vgaScaleX(295) - _utils->svgaCord(15), _utils->vgaScaleY(148), ntext);
	_graphics->setPalette(_anim->_diffPalette, 256);

	_graphics->closeFont(noteFont);
	delete[] ntext;
}

/**
 * Does the things to properly set up the old west newspaper.  Assumes that
 * OpenHiRes already called.
 */
void LabEngine::doWestPaper() {
	TextFont *paperFont = _resource->getFont("P:News22.fon");
	char *ntext = _resource->getText("Lab:Rooms/Date");
	_graphics->flowText(paperFont, 0, 0, 0, false, true, false, true, _utils->vgaScaleX(57), _utils->vgaScaleY(77) + _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(91), ntext);
	_graphics->closeFont(paperFont);
	delete[] ntext;

	paperFont = _resource->getFont("P:News32.fon");
	ntext = _resource->getText("Lab:Rooms/Headline");

	int fileLen = strlen(ntext) - 1;
	int charsPrinted = _graphics->flowText(paperFont, -8, 0, 0, false, true, false, true, _utils->vgaScaleX(57), _utils->vgaScaleY(86) - _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(118), ntext);

	uint16 y;

	if (charsPrinted < fileLen) {
		y = 130 - _utils->svgaCord(5);
		_graphics->flowText(paperFont, -8 - _utils->svgaCord(1), 0, 0, false, true, false, true, _utils->vgaScaleX(57), _utils->vgaScaleY(86) - _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(132), ntext);
	} else
		y = 115 - _utils->svgaCord(5);

	_graphics->closeFont(paperFont);
	delete[] ntext;

	paperFont = _resource->getFont("P:Note.fon");
	ntext = _resource->getText("Lab:Rooms/Col1");
	charsPrinted = _graphics->flowTextScaled(paperFont, -4, 0, 0, false, false, false, true, 45, y, 158, 148, ntext);
	delete[] ntext;
	ntext = _resource->getText("Lab:Rooms/Col2");
	charsPrinted = _graphics->flowTextScaled(paperFont, -4, 0, 0, false, false, false, true, 162, y, 275, 148, ntext);
	delete[] ntext;
	_graphics->closeFont(paperFont);

	_graphics->setPalette(_anim->_diffPalette, 256);
}

/**
 * Loads in the data for the journal.
 */
void LabEngine::loadJournalData() {
	_journalFont = _resource->getFont("P:Journal.fon");
	_music->updateMusic();

	char filename[20];
	strcpy(filename, "Lab:Rooms/j0");

	bool bridge = _conditions->in(BRIDGE0) || _conditions->in(BRIDGE1);
	bool dirty  = _conditions->in(DIRTY);
	bool news   = !_conditions->in(NONEWS);
	bool clean  = !_conditions->in(NOCLEAN);

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

	_journalText = _resource->getText(filename);
	_journalTextTitle = _resource->getText("Lab:Rooms/jt");

	Common::File *journalFile = _resource->openDataFile("P:JImage");
	Utils *utils = _utils;
	_journalButtonList.push_back(_event->createButton( 80, utils->vgaScaleY(162) + utils->svgaCord(1), 0, VKEY_LTARROW, new Image(journalFile, this), new Image(journalFile, this)));	// back
	_journalButtonList.push_back(_event->createButton(194, utils->vgaScaleY(162) + utils->svgaCord(1), 2,            0, new Image(journalFile, this), new Image(journalFile, this)));	// cancel
	_journalButtonList.push_back(_event->createButton(144, utils->vgaScaleY(164) - utils->svgaCord(1), 1, VKEY_RTARROW, new Image(journalFile, this), new Image(journalFile, this)));	// forward
	delete journalFile;

	_anim->_noPalChange = true;
	_journalBackImage->_imageData = new byte[_graphics->_screenWidth * _graphics->_screenHeight];
	_graphics->readPict("P:Journal.pic", true, false, _journalBackImage->_imageData);
	_anim->_noPalChange = false;

	// Keep a copy of the blank journal
	_blankJournal = new byte[_graphics->_screenWidth * _graphics->_screenHeight];
	memcpy(_blankJournal, _journalBackImage->_imageData, _graphics->_screenWidth * _graphics->_screenHeight);

	_screenImage->_imageData = _graphics->getCurrentDrawingBuffer();
}

/**
 * Draws the text to the back journal screen to the appropriate Page number
 */
void LabEngine::drawJournalText() {
	uint16 drawingToPage = 1;
	int32 charsDrawn = 0;
	char *curText = _journalText;

	while (drawingToPage < _journalPage) {
		_music->updateMusic();
		curText = (char *)(_journalText + charsDrawn);
		charsDrawn += _graphics->flowTextScaled(_journalFont, -2, 2, 0, false, false, false, false, 52, 32, 152, 148, curText);

		_lastPage = (*curText == 0);

		if (_lastPage)
			_journalPage = (drawingToPage / 2) * 2;
		else
			drawingToPage++;
	}

	if (_journalPage <= 1) {
		curText = _journalTextTitle;
		_graphics->flowTextToMem(_journalBackImage, _journalFont, -2, 2, 0, false, true, true, true, _utils->vgaScaleX(52), _utils->vgaScaleY(32), _utils->vgaScaleX(152), _utils->vgaScaleY(148), curText);
	} else {
		curText = (char *)(_journalText + charsDrawn);
		charsDrawn += _graphics->flowTextToMem(_journalBackImage, _journalFont, -2, 2, 0, false, false, false, true, _utils->vgaScaleX(52), _utils->vgaScaleY(32), _utils->vgaScaleX(152), _utils->vgaScaleY(148), curText);
	}

	_music->updateMusic();
	curText = (char *)(_journalText + charsDrawn);
	_lastPage = (*curText == 0);
	_graphics->flowTextToMem(_journalBackImage, _journalFont, -2, 2, 0, false, false, false, true, _utils->vgaScaleX(171), _utils->vgaScaleY(32), _utils->vgaScaleX(271), _utils->vgaScaleY(148), curText);

	curText = (char *)(_journalText + charsDrawn);
	_lastPage = _lastPage || (*curText == 0);
}

/**
 * Does the turn page wipe.
 */
void LabEngine::turnPage(bool fromLeft) {
	if (fromLeft) {
		for (int i = 0; i < _graphics->_screenWidth; i += 8) {
			_music->updateMusic();
			waitTOF();
			_screenImage->_imageData = _graphics->getCurrentDrawingBuffer();
			_journalBackImage->blitBitmap(i, 0, _screenImage, i, 0, 8, _graphics->_screenHeight, false);
		}
	} else {
		for (int i = (_graphics->_screenWidth - 8); i > 0; i -= 8) {
			_music->updateMusic();
			waitTOF();
			_screenImage->_imageData = _graphics->getCurrentDrawingBuffer();
			_journalBackImage->blitBitmap(i, 0, _screenImage, i, 0, 8, _graphics->_screenHeight, false);
		}
	}
}

/**
 * Draws the journal from page x.
 */
void LabEngine::drawJournal(uint16 wipenum, bool needFade) {
	_event->mouseHide();
	_music->updateMusic();
	drawJournalText();
	_graphics->loadBackPict("P:Journal.pic", _highPalette);

	if (wipenum == 0)
		_journalBackImage->blitBitmap(0, 0, _screenImage, 0, 0, _graphics->_screenWidth, _graphics->_screenHeight, false);
	else
		turnPage((bool)(wipenum == 1));

	Button *backButton = _event->getButton(0);
	Button *forwardButton = _event->getButton(2);

	if (_journalPage == 0)
		_event->disableButton(backButton, 15);
	else
		_event->enableButton(backButton);

	if (_lastPage)
		_event->disableButton(forwardButton, 15);
	else
		_event->enableButton(forwardButton);

	if (needFade)
		_graphics->fade(true, 0);

	// Reset the journal background, so that all the text that has been blitted on it is erased
	memcpy(_journalBackImage->_imageData, _blankJournal, _graphics->_screenWidth * _graphics->_screenHeight);

	eatMessages();
	_event->mouseShow();
}

/**
 * Processes user input.
 */
void LabEngine::processJournal() {
	while (1) {
		// Make sure we check the music at least after every message
		_music->updateMusic();
		IntuiMessage *msg = _event->getMsg();

		if (msg == NULL) {
			_music->updateMusic();
		} else {
			uint32 msgClass  = msg->_msgClass;
			uint16 qualifier = msg->_qualifier;
			uint16 gadID     = msg->_code;

			if (((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RIGHTBUTTON & qualifier)) ||
				  ((msgClass == RAWKEY) && (gadID == 27)))
				return;
			else if (msgClass == BUTTONUP) {
				if (gadID == 0) {
					if (_journalPage >= 2) {
						_journalPage -= 2;
						drawJournal(1, false);
					}
				} else if (gadID == 1) {
					return;
				} else if (gadID == 2) {
					if (!_lastPage) {
						_journalPage += 2;
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
	_lastPage = false;

	_screenImage->_width = _journalBackImage->_width = _graphics->_screenWidth;
	_screenImage->_height = _journalBackImage->_height = _graphics->_screenHeight;
	_journalBackImage->_imageData = nullptr;
	_screenImage->_imageData = _graphics->getCurrentDrawingBuffer();

	_music->updateMusic();
	loadJournalData();
	_event->attachButtonList(&_journalButtonList);
	drawJournal(0, true);
	_event->mouseShow();
	processJournal();
	_event->attachButtonList(NULL);
	_graphics->fade(false, 0);
	_event->mouseHide();

	delete[] _blankJournal;
	delete[] _journalBackImage->_imageData;
	_event->freeButtonList(&_journalButtonList);
	_graphics->closeFont(_journalFont);

	_screenImage->_imageData = _graphics->getCurrentDrawingBuffer();

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	_graphics->blackScreen();
}

/**
 * Draws the text for the monitor.
 */
void LabEngine::drawMonText(char *text, TextFont *monitorFont, uint16 x1, uint16 y1, uint16 x2, uint16 y2, bool isinteractive) {
	uint16 drawingToPage = 0, yspacing = 0, numlines, fheight;
	int32 charsDrawn    = 0L;
	char *curText = text;

	_event->mouseHide();

	if (*text == '%') {
		text++;
		numlines = (*text - '0') * 10;
		text++;
		numlines += (*text - '0');
		text += 2;

		fheight = _graphics->textHeight(monitorFont);
		x1 = _monitorButton->_width + _utils->vgaScaleX(3);
		_monitorButtonHeight = _monitorButton->_height + _utils->vgaScaleY(3);

		if (_monitorButtonHeight > fheight)
			yspacing = _monitorButtonHeight - fheight;
		else
			_monitorButtonHeight = fheight;

		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, y2);

		for (uint16 i = 0; i < numlines; i++)
			_monitorButton->drawImage(0, i * _monitorButtonHeight);
	} else if (isinteractive) {
		_graphics->setAPen(0);
		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, y2);
	} else {
		_graphics->setAPen(0);
		_graphics->rectFill(x1, y1, x2, y2);
	}

	while (drawingToPage < _monitorPage) {
		_music->updateMusic();
		curText = (char *)(text + charsDrawn);
		charsDrawn += _graphics->flowText(monitorFont, yspacing, 0, 0, false, false, false, false, x1, y1, x2, y2, curText);
		_lastPage = (*curText == 0);

		if (_lastPage)
			_monitorPage = drawingToPage;
		else
			drawingToPage++;
	}

	curText = (char *)(text + charsDrawn);
	_lastPage = (*curText == 0);
	charsDrawn = _graphics->flowText(monitorFont, yspacing, 2, 0, false, false, false, true, x1, y1, x2, y2, curText);
	curText += charsDrawn;
	_lastPage = _lastPage || (*curText == 0);

	_event->mouseShow();
}

/**
 * Processes user input.
 */
void LabEngine::processMonitor(char *ntext, TextFont *monitorFont, bool isInteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	const char *test = " ", *startFileName = _monitorTextFilename;
	CloseDataPtr startClosePtr = _closeDataPtr, lastClosePtr[10];
	uint16 depth = 0;

	lastClosePtr[0] = _closeDataPtr;

	while (1) {
		if (isInteractive) {
			if (_closeDataPtr == NULL)
				_closeDataPtr = startClosePtr;

			if (_closeDataPtr == startClosePtr)
				test = startFileName;
			else
				test = _closeDataPtr->_graphicName;

			if (strcmp(test, _monitorTextFilename)) {
				_monitorPage      = 0;
				_monitorTextFilename = test;

				ntext = _resource->getText(_monitorTextFilename);
				_graphics->fade(false, 0);
				drawMonText(ntext, monitorFont, x1, y1, x2, y2, isInteractive);
				_graphics->fade(true, 0);
				delete[] ntext;
			}
		}

		// Make sure we check the music at least after every message
		_music->updateMusic();
		IntuiMessage *msg = _event->getMsg();

		if (msg == NULL) {
			_music->updateMusic();
		} else {
			uint32 msgClass  = msg->_msgClass;
			uint16 qualifier = msg->_qualifier;
			uint16 mouseX    = msg->_mouseX;
			uint16 mouseY    = msg->_mouseY;
			uint16 code      = msg->_code;

			if (((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RIGHTBUTTON & qualifier)) ||
				  ((msgClass == RAWKEY) && (code == 27)))
				return;

			else if ((msgClass == MOUSEBUTTONS) && (IEQUALIFIER_LEFTBUTTON & qualifier)) {
				if ((mouseY >= _utils->vgaScaleY(171)) && (mouseY <= _utils->vgaScaleY(200))) {
					if ((mouseX >= _utils->vgaScaleX(259)) && (mouseX <= _utils->vgaScaleX(289))) {
						if (!_lastPage) {
							_monitorPage += 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isInteractive);
						}
					} else if ((mouseX >= _utils->vgaScaleX(0)) && (mouseX <= _utils->vgaScaleX(31))) {
						return;
					} else if ((mouseX >= _utils->vgaScaleX(290)) && (mouseX <= _utils->vgaScaleX(320))) {
						if (_monitorPage >= 1) {
							_monitorPage -= 1;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isInteractive);
						}
					} else if ((mouseX >= _utils->vgaScaleX(31)) && (mouseX <= _utils->vgaScaleX(59))) {
						if (isInteractive) {
							_monitorPage = 0;

							if (depth) {
								depth--;
								_closeDataPtr = lastClosePtr[depth];
							}
						} else if (_monitorPage > 0) {
							_monitorPage = 0;
							drawMonText(ntext, monitorFont, x1, y1, x2, y2, isInteractive);
						}
					}
				} else if (isInteractive) {
					CloseDataPtr tmpClosePtr = _closeDataPtr;
					mouseY = 64 + (mouseY / _monitorButtonHeight) * 42;
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

	_monitorTextFilename = textfile;

	_graphics->blackAllScreen();
	_graphics->readPict("P:Mon/Monitor.1", true);
	_graphics->readPict("P:Mon/NWD1", true);
	_graphics->readPict("P:Mon/NWD2", true);
	_graphics->readPict("P:Mon/NWD3", true);
	_graphics->blackAllScreen();

	_monitorPage = 0;
	_lastPage = false;
	_graphics->FadePalette = _highPalette;

	TextFont *monitorFont = _resource->getFont("P:Map.fon");
	Common::File *buttonFile = _resource->openDataFile("P:MonImage");
	_monitorButton = new Image(buttonFile, this);
	delete buttonFile;

	ntext = _resource->getText(textfile);
	_graphics->loadBackPict(background, _highPalette);
	drawMonText(ntext, monitorFont, x1, y1, x2, y2, isinteractive);
	_event->mouseShow();
	_graphics->fade(true, 0);
	processMonitor(ntext, monitorFont, isinteractive, x1, y1, x2, y2);
	_graphics->fade(false, 0);
	_event->mouseHide();
	delete[] ntext;
	_graphics->closeFont(monitorFont);

	_graphics->setAPen(0);
	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1);
	_graphics->blackAllScreen();
	_graphics->freePict();
}

} // End of namespace Lab
