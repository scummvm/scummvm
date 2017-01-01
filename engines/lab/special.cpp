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

#include "common/file.h"

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

void LabEngine::doNotes() {
	TextFont *noteFont = _resource->getFont("F:Note.fon");
	Common::String noteText = _resource->getText("Lab:Rooms/Notes");

	Common::Rect textRect = Common::Rect(_utils->vgaScaleX(25) + _utils->svgaCord(15), _utils->vgaScaleY(50), _utils->vgaScaleX(295) - _utils->svgaCord(15), _utils->vgaScaleY(148));
	_graphics->flowText(noteFont, -2 + _utils->svgaCord(1), 0, 0, false, false, true, true, textRect, noteText.c_str());
	_graphics->setPalette(_anim->_diffPalette, 256);
	_graphics->freeFont(&noteFont);
}

void LabEngine::doWestPaper() {
	TextFont *paperFont = _resource->getFont("F:News22.fon");
	Common::String paperText = _resource->getText("Lab:Rooms/Date");

	Common::Rect textRect = Common::Rect(_utils->vgaScaleX(57), _utils->vgaScaleY(77) + _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(91));
	_graphics->flowText(paperFont, 0, 0, 0, false, true, false, true, textRect, paperText.c_str());
	_graphics->freeFont(&paperFont);

	paperFont = _resource->getFont("F:News32.fon");
	paperText = _resource->getText("Lab:Rooms/Headline");

	int fileLen = paperText.size() - 1;
	textRect = Common::Rect(_utils->vgaScaleX(57), _utils->vgaScaleY(86) - _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(118));
	int charsPrinted = _graphics->flowText(paperFont, -8, 0, 0, false, true, false, true, textRect, paperText.c_str());

	uint16 y;

	if (charsPrinted < fileLen) {
		y = 130 - _utils->svgaCord(5);
		textRect = Common::Rect(_utils->vgaScaleX(57), _utils->vgaScaleY(86) - _utils->svgaCord(2), _utils->vgaScaleX(262), _utils->vgaScaleY(132));
		_graphics->flowText(paperFont, -8 - _utils->svgaCord(1), 0, 0, false, true, false, true, textRect, paperText.c_str());
	} else
		y = 115 - _utils->svgaCord(5);

	_graphics->freeFont(&paperFont);

	paperFont = _resource->getFont("F:Note.fon");
	paperText = _resource->getText("Lab:Rooms/Col1");
	_graphics->flowText(paperFont, -4, 0, 0, false, false, false, true, _utils->vgaRectScale(45, y, 158, 148), paperText.c_str());

	paperText = _resource->getText("Lab:Rooms/Col2");
	_graphics->flowText(paperFont, -4, 0, 0, false, false, false, true, _utils->vgaRectScale(162, y, 275, 148), paperText.c_str());

	_graphics->freeFont(&paperFont);
	_graphics->setPalette(_anim->_diffPalette, 256);
}

void LabEngine::loadJournalData() {
	if (_journalFont)
		_graphics->freeFont(&_journalFont);

	_journalFont = _resource->getFont("F:Journal.fon");
	updateEvents();

	Common::String filename = "Lab:Rooms/j";

	bool bridge = _conditions->in(kCondBridge0) || _conditions->in(kCondBridge1);
	bool dirty  = _conditions->in(kCondDirty);
	bool news   = !_conditions->in(kCondNoNews);
	bool clean  = !_conditions->in(kCondNoClean);

	if (bridge && clean && news)
		filename += '8';
	else if (clean && news)
		filename += '9';
	else if (bridge && clean)
		filename += '6';
	else if (clean)
		filename += '7';
	else if (bridge && dirty && news)
		filename += '4';
	else if (dirty && news)
		filename += '5';
	else if (bridge && dirty)
		filename += '2';
	else if (dirty)
		filename += '3';
	else if (bridge)
		filename += '1';
	else
		filename += '0';

	_journalText = _resource->getText(filename);
	_journalTextTitle = _resource->getText("Lab:Rooms/jt");

	Common::File *journalFile = _resource->openDataFile("P:JImage");
	_journalButtonList.push_back(_interface->createButton( 80, _utils->vgaScaleY(162) + _utils->svgaCord(1), 0,  Common::KEYCODE_LEFT,  new Image(journalFile, this), new Image(journalFile, this)));	// back
	_journalButtonList.push_back(_interface->createButton(194, _utils->vgaScaleY(162) + _utils->svgaCord(1), 2,  Common::KEYCODE_RIGHT, new Image(journalFile, this), new Image(journalFile, this)));	// forward
	_journalButtonList.push_back(_interface->createButton(144, _utils->vgaScaleY(164) - _utils->svgaCord(1), 1, Common::KEYCODE_ESCAPE, new Image(journalFile, this), new Image(journalFile, this)));	// cancel
	delete journalFile;

	_anim->_noPalChange = true;
	_journalBackImage->setData(new byte[_graphics->_screenBytesPerPage]);
	_graphics->readPict("P:Journal.pic", true, false, _journalBackImage->_imageData);
	_anim->_noPalChange = false;

	// Keep a copy of the blank journal
	_blankJournal = new byte[_graphics->_screenBytesPerPage];
	memcpy(_blankJournal, _journalBackImage->_imageData, _graphics->_screenBytesPerPage);
}

void LabEngine::drawJournalText() {
	uint16 drawingToPage = 1;
	const char *curText = _journalText.c_str();

	assert((_journalPage & 1) == 0);

	while (drawingToPage < _journalPage) {
		updateEvents();

		// flowText without output
		curText += _graphics->flowText(_journalFont, -2, 2, 0, false, false, false, false, _utils->vgaRectScale(52, 32, 152, 148), curText);

		_lastPage = (*curText == 0);

		if (_lastPage) {
			// Reset _journalPage to this page, in case it was set too high
			_journalPage = (drawingToPage / 2) * 2;
			break;
		}

		drawingToPage++;
	}

	if (_journalPage == 0) {
		// draw title page centered
		_graphics->flowText(_journalFont, -2, 2, 0, false, true, true, true, _utils->vgaRectScale(52, 32, 152, 148), _journalTextTitle.c_str(), _journalBackImage);
	} else {
		curText += _graphics->flowText(_journalFont, -2, 2, 0, false, false, false, true, _utils->vgaRectScale(52, 32, 152, 148), curText, _journalBackImage);
	}

	updateEvents();
	curText += _graphics->flowText(_journalFont, -2, 2, 0, false, false, false, true, _utils->vgaRectScale(171, 32, 271, 148), curText, _journalBackImage);

	_lastPage = (*curText == 0);
}

void LabEngine::turnPage(bool fromLeft) {
	if (fromLeft) {
		for (int i = 0; i < _graphics->_screenWidth; i += 8) {
			updateEvents();
			waitTOF();
			_journalBackImage->blitBitmap(i, 0, nullptr, i, 0, 8, _graphics->_screenHeight, false);
		}
	} else {
		for (int i = (_graphics->_screenWidth - 8); i > 0; i -= 8) {
			updateEvents();
			waitTOF();
			_journalBackImage->blitBitmap(i, 0, nullptr, i, 0, 8, _graphics->_screenHeight, false);
		}
	}
}

void LabEngine::drawJournal(uint16 wipenum, bool needFade) {
	_event->mouseHide();
	updateEvents();
	drawJournalText();
	_graphics->loadBackPict("P:Journal.pic", _highPalette);

	if (wipenum == 0)
		_journalBackImage->blitBitmap(0, 0, nullptr, 0, 0, _graphics->_screenWidth, _graphics->_screenHeight, false);
	else
		turnPage((wipenum == 1));

	_interface->toggleButton(_interface->getButton(0), 15, (_journalPage > 0));	// back button
	_interface->toggleButton(_interface->getButton(2), 15, (!_lastPage));	// forward button

	if (needFade)
		_graphics->fade(true);

	// Reset the journal background, so that all the text that has been blitted on it is erased
	memcpy(_journalBackImage->_imageData, _blankJournal, _graphics->_screenBytesPerPage);

	eatMessages();
	_event->mouseShow();
}

void LabEngine::processJournal() {
	while (1) {
		IntuiMessage *msg = _event->getMsg();
		if (shouldQuit()) {
			_quitLab = true;
			return;
		}

		updateEvents();
		_graphics->screenUpdate();
		_system->delayMillis(10);

		if (!msg)
			continue;

		MessageClass msgClass  = msg->_msgClass;

		if ((msgClass == kMessageRightClick) ||
			((msgClass == kMessageRawKey) && (msg->_code == Common::KEYCODE_ESCAPE)))
			return;
		else if (msgClass == kMessageButtonUp) {
			uint16 buttonId  = msg->_code;
			if (buttonId == 0) {
				if (_journalPage >= 2) {
					_journalPage -= 2;
					drawJournal(1, false);
				}
			} else if (buttonId == 1) {
				return;
			} else if (buttonId == 2) {
				if (!_lastPage) {
					_journalPage += 2;
					drawJournal(2, false);
				}
			}
		}
	}	// while
}

void LabEngine::doJournal() {
	_graphics->blackAllScreen();
	_lastPage = false;

	_journalBackImage->_width = _graphics->_screenWidth;
	_journalBackImage->_height = _graphics->_screenHeight;
	_journalBackImage->setData(nullptr, true);

	updateEvents();
	loadJournalData();
	_interface->attachButtonList(&_journalButtonList);
	drawJournal(0, true);
	_event->mouseShow();
	processJournal();
	_interface->attachButtonList(nullptr);
	_graphics->fade(false);
	_event->mouseHide();

	delete[] _blankJournal;
	_blankJournal = nullptr;
	_journalBackImage->setData(nullptr, true);

	_interface->freeButtonList(&_journalButtonList);
	_graphics->freeFont(&_journalFont);

	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1, 0);
	_graphics->blackScreen();
}

void LabEngine::drawMonText(const char *text, TextFont *monitorFont, Common::Rect textRect, bool isinteractive) {
	uint16 drawingToPage = 0, yspacing = 0;

	_event->mouseHide();

	if (*text == '%') {
		text++;
		uint16 numlines = (*text - '0') * 10;
		text++;
		numlines += (*text - '0');
		text += 2;

		uint16 fheight = _graphics->textHeight(monitorFont);
		textRect.left = _monitorButton->_width + _utils->vgaScaleX(3);
		_monitorButtonHeight = _monitorButton->_height + _utils->vgaScaleY(3);

		if (_monitorButtonHeight > fheight)
			yspacing = _monitorButtonHeight - fheight;
		else
			_monitorButtonHeight = fheight;

		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, textRect.bottom, 0);

		for (int i = 0; i < numlines; i++)
			_monitorButton->drawImage(0, i * _monitorButtonHeight);
	} else if (isinteractive) {
		_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, textRect.bottom, 0);
	} else {
		_graphics->rectFill(textRect, 0);
	}

	const char *curText = text;
	while (drawingToPage < _monitorPage) {
		updateEvents();
		curText += _graphics->flowText(monitorFont, yspacing, 0, 0, false, false, false, false, textRect, curText);
		_lastPage = (*curText == 0);

		if (_lastPage)
			_monitorPage = drawingToPage;
		else
			drawingToPage++;
	}

	curText += _graphics->flowText(monitorFont, yspacing, 2, 0, false, false, false, true, textRect, curText);
	_lastPage = (*curText == 0);
	_event->mouseShow();
}

void LabEngine::processMonitor(const Common::String &ntext, TextFont *monitorFont, bool isInteractive, Common::Rect textRect) {
	Common::String startFileName = _monitorTextFilename;
	const CloseData *startClosePtr = _closeDataPtr, *lastClosePtr[10];
	uint16 depth = 0;
	Common::String text = ntext;

	lastClosePtr[0] = _closeDataPtr;

	while (1) {
		if (isInteractive) {
			if (!_closeDataPtr)
				_closeDataPtr = startClosePtr;

			Common::String filename;
			if (_closeDataPtr == startClosePtr)
				filename = startFileName;
			else
				filename = _closeDataPtr->_graphicName;

			if (filename != _monitorTextFilename) {
				_monitorPage = 0;
				_monitorTextFilename = filename;

				text = _resource->getText(_monitorTextFilename);
				_graphics->fade(false);
				drawMonText(text.c_str(), monitorFont, textRect, isInteractive);
				_graphics->fade(true);
			}
		}

		IntuiMessage *msg = _event->getMsg();
		if (shouldQuit()) {
			_quitLab = true;
			return;
		}

		updateEvents();
		_graphics->screenUpdate();
		_system->delayMillis(10);

		if (!msg)
			continue;

		MessageClass msgClass  = msg->_msgClass;

		if ((msgClass == kMessageRightClick) ||
				((msgClass == kMessageRawKey) && (msg->_code == Common::KEYCODE_ESCAPE)))
			return;

		if (msgClass == kMessageLeftClick) {
			int16 mouseX = msg->_mouse.x;
			int16 mouseY = msg->_mouse.y;

			// Check if mouse was in button bar
			if ((mouseY >= _utils->vgaScaleY(171)) && (mouseY <= _utils->vgaScaleY(200))) {
				if (mouseX <= _utils->vgaScaleX(31)) {
					// Exit button
					return;
				}

				if (mouseX <= _utils->vgaScaleX(59)) {
					// Back button
					if (isInteractive) {
						_monitorPage = 0;

						if (depth) {
							depth--;
							_closeDataPtr = lastClosePtr[depth];
						}
					} else if (_monitorPage > 0) {
						_monitorPage = 0;
						drawMonText(text.c_str(), monitorFont, textRect, isInteractive);
					}
				} else if (mouseX < _utils->vgaScaleX(259)) {
					// empty region; ignore
				} else if (mouseX <= _utils->vgaScaleX(289)) {
					// Page down button
					if (!_lastPage) {
						_monitorPage += 1;
						drawMonText(text.c_str(), monitorFont, textRect, isInteractive);
					}
				} else if (_monitorPage >= 1) {
					// Page up button
					_monitorPage -= 1;
					drawMonText(text.c_str(), monitorFont, textRect, isInteractive);
				}
			} else if (isInteractive) {
				const CloseData *tmpClosePtr = _closeDataPtr;
				mouseY = 64 + (mouseY / _monitorButtonHeight) * 42;
				mouseX = 101;
				setCurrentClose(Common::Point(mouseX, mouseY), &_closeDataPtr, false);

				if (tmpClosePtr != _closeDataPtr) {
					lastClosePtr[depth] = tmpClosePtr;
					depth++;
				}
			}
		}
	}	// while
}

void LabEngine::doMonitor(const Common::String background, const Common::String textfile, bool isinteractive, Common::Rect textRect) {
	Common::Rect scaledRect = _utils->vgaRectScale(textRect.left, textRect.top, textRect.right, textRect.bottom);
	_monitorTextFilename = textfile;

	_graphics->blackAllScreen();
	_graphics->readPict("P:Mon/Monitor.1");
	_graphics->readPict("P:Mon/NWD1");
	_graphics->readPict("P:Mon/NWD2");
	_graphics->readPict("P:Mon/NWD3");
	_graphics->blackAllScreen();

	_monitorPage = 0;
	_lastPage = false;
	_graphics->_fadePalette = _highPalette;

	TextFont *monitorFont = _resource->getFont("F:Map.fon");
	Common::File *buttonFile = _resource->openDataFile("P:MonImage");
	_monitorButton = new Image(buttonFile, this);
	delete buttonFile;

	Common::String ntext = _resource->getText(textfile);
	_graphics->loadBackPict(background, _highPalette);
	drawMonText(ntext.c_str(), monitorFont, scaledRect, isinteractive);
	_event->mouseShow();
	_graphics->fade(true);
	processMonitor(ntext, monitorFont, isinteractive, scaledRect);
	_graphics->fade(false);
	_event->mouseHide();
	_graphics->freeFont(&monitorFont);

	_graphics->rectFill(0, 0, _graphics->_screenWidth - 1, _graphics->_screenHeight - 1, 0);
	_graphics->blackAllScreen();
	_graphics->freePict();
}

} // End of namespace Lab
