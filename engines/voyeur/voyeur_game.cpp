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

#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"
#include "voyeur/animation.h"

namespace Voyeur {

void VoyeurEngine::playStamp() {
	_stampLibPtr = NULL;
	_filesManager->openBoltLib("stampblt.blt", _stampLibPtr);

	_stampLibPtr->getBoltGroup(0);
	_controlPtr->_state = _stampLibPtr->boltEntry(_controlPtr->_stateId >> 16)._stateResource;
	assert(_controlPtr->_state);

	_resolvePtr = &RESOLVE_TABLE[0];
	initStamp();

	PtrResource *threadsList = _stampLibPtr->boltEntry(3)._ptrResource;
	_mainThread = threadsList->_entries[0]->_threadResource;
	_mainThread->initThreadStruct(0, 0);

	_voy->_isAM = false;
	_gameHour = 9;
	_gameMinute = 0;
	_voy->_abortInterface = true;

	int buttonId;
	bool breakFlag = false;
	while (!breakFlag && !shouldQuit()) {
		_voyeurArea = AREA_NONE;
		_eventsManager->getMouseInfo();
		_playStampGroupId = _currentVocId = -1;
		_audioVideoId = -1;

		_mainThread->parsePlayCommands();

		bool flag = breakFlag = (_voy->_eventFlags & EVTFLAG_2) != 0;

		switch (_voy->_playStampMode) {
		case 5:
			buttonId = _mainThread->doInterface();
			if (shouldQuit())
				return;

			if (buttonId == -2) {
				switch (_mainThread->doApt()) {
				case 0:
					_voy->_aptLoadMode = 140;
					break;
				case 1:
					_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
					_voy->_abortInterface = true;
					_mainThread->chooseSTAMPButton(22);
					_voy->_aptLoadMode = 143;
					break;
				case 2:
					_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
					reviewTape();
					_voy->_abortInterface = true;
					_voy->_aptLoadMode = 142;
					break;
				case 3:
					_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
					_mainThread->chooseSTAMPButton(21);
					break;
				case 4:
					breakFlag = true;
					_eventsManager->_mouseClicked = false;
					break;
				case 5:
					doGossip();
					_voy->_abortInterface = true;
					_voy->_aptLoadMode = 141;
					_voy->_eventFlags &= ~EVTFLAG_100;
					break;
				default:
					break;
				}
			} else {
				_mainThread->chooseSTAMPButton(buttonId);
			}

			flag = true;
			break;

		case 6:
			_mainThread->doRoom();
			flag = true;
			break;

		case 16:
			_voy->_transitionId = 17;
			buttonId = _mainThread->doApt();

			switch (buttonId) {
			case 1:
				_mainThread->chooseSTAMPButton(22);
				flag = true;
				break;
			case 2:
				reviewTape();
				_voy->_abortInterface = true;
				break;
			case 4:
				flag = true;
				breakFlag = true;
				break;
			default:
				break;
			}
			break;

		case 17:
			// Called the police, showing the tape
			doTapePlaying();
			if (!checkForMurder() && _voy->_transitionId <= 15)
				checkForIncriminate();

			if (_voy->_videoEventId != -1) {
				// Show the found video that is of interest to the police
				playAVideoEvent(_voy->_videoEventId);
				_voy->_eventFlags &= ~EVTFLAG_RECORDING;
			}

			// Handle response
			_mainThread->chooseSTAMPButton(0);
			flag = true;
			break;

		case 130: {
			// user selected to send the tape
			if (_bVoy->getBoltGroup(_playStampGroupId)) {
				_screen->_backgroundPage = _bVoy->boltEntry(_playStampGroupId)._picResource;
				_screen->_backColors = _bVoy->boltEntry(_playStampGroupId + 1)._cMapResource;

				buttonId = getChooseButton();
				if (_eventsManager->_rightClick)
					// Aborted out of selecting a recipient
					buttonId = 4;

				_bVoy->freeBoltGroup(_playStampGroupId);
				_screen->screenReset();
				_playStampGroupId = -1;
				flag = true;

				if (buttonId != 4) {
					_voy->_playStampMode = 131;
					_voy->checkForKey();
					_mainThread->chooseSTAMPButton(buttonId);
				} else {
					_mainThread->chooseSTAMPButton(buttonId);
					_voy->_abortInterface = true;
				}
			}
			break;
		}

		default:
			break;
		}

		do {
			if (flag) {
				if (_currentVocId != -1) {
					_soundManager->stopVOCPlay();
					_currentVocId = -1;
				}

				_audioVideoId = -1;

				if (_voy->_boltGroupId2 != -1) {
					_bVoy->freeBoltGroup(_voy->_boltGroupId2);
					_voy->_boltGroupId2 = -1;
				}

				if (_playStampGroupId != -1) {
					_bVoy->freeBoltGroup(_playStampGroupId);
					_playStampGroupId = -1;
				}

				// Break out of loop
				flag = false;

			} else if (_mainThread->_stateFlags & 2) {
				_eventsManager->getMouseInfo();
				_mainThread->chooseSTAMPButton(0);
				flag = true;
			} else {
				_mainThread->chooseSTAMPButton(0);
				flag = true;
			}
		} while (flag);
	}

	_voy->_viewBounds = nullptr;
	closeStamp();
	_stampLibPtr->freeBoltGroup(0);
	delete _stampLibPtr;
}

void VoyeurEngine::initStamp() {
	_stampFlags &= ~1;
	_stackGroupPtr = _controlGroupPtr;

	if (!_controlPtr->_entries[0])
		error("No control entries");

	ThreadResource::initUseCount();
}

void VoyeurEngine::closeStamp() {
	ThreadResource::unloadAllStacks(this);
}

void VoyeurEngine::doTailTitle() {
	_screen->_vPort->setupViewPort(NULL);
	_screen->screenReset();

	if (_bVoy->getBoltGroup(0x600)) {
		if (!getIsDemo()) {
			RL2Decoder decoder;
			decoder.loadRL2File("a1100200.rl2", false);
			decoder.start();
			decoder.play(this);
		}

		if (!shouldQuit() && !_eventsManager->_mouseClicked) {
			doClosingCredits();

			if (!shouldQuit() && !_eventsManager->_mouseClicked) {
				_screen->screenReset();

				PictureResource *pic = _bVoy->boltEntry(0x602)._picResource;
				CMapResource *pal = _bVoy->boltEntry(0x603)._cMapResource;

				_screen->_vPort->setupViewPort(pic);
				pal->startFade();
				flipPageAndWaitForFade();
				_eventsManager->delayClick(300);

				pic = _bVoy->boltEntry(0x604)._picResource;
				pal = _bVoy->boltEntry(0x605)._cMapResource;

				_screen->_vPort->setupViewPort(pic);
				pal->startFade();
				flipPageAndWaitForFade();
				_eventsManager->delayClick(120);

				_soundManager->stopVOCPlay();
			}
		}

		_bVoy->freeBoltGroup(0x600);
	}

	if (!shouldQuit()) {
		_bVoy->getBoltGroup(0x100);
		doPiracy();
	}
}

void VoyeurEngine::doClosingCredits() {
	if (!_bVoy->getBoltGroup(0x400))
		return;

	const char *msg = (const char *)_bVoy->memberAddr(0x404);
	const byte *creditList = (const byte *)_bVoy->memberAddr(0x405);

	_screen->_vPort->setupViewPort(NULL);
	_screen->setColor(1, 180, 180, 180);
	_screen->setColor(2, 200, 200, 200);
	_eventsManager->_intPtr._hasPalette = true;

	_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x402)._fontResource;
	_screen->_fontPtr->_foreColor = 2;
	_screen->_fontPtr->_backColor = 2;
	_screen->_fontPtr->_fontSaveBack = false;
	_screen->_fontPtr->_fontFlags = DISPFLAG_NONE;

	_soundManager->startVOCPlay(152);
	FontInfoResource &fi = *_screen->_fontPtr;

	for (int idx = 0; idx < 78; ++idx) {
		const byte *entry = creditList + idx * 6;
		int flags = READ_LE_UINT16(entry + 4);

		if (flags & 0x10)
			_screen->_vPort->fillPic(0);

		if (flags & 1) {
			fi._foreColor = 1;
			fi._curFont = _bVoy->boltEntry(0x402)._fontResource;
			fi._justify = ALIGN_CENTER;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(0, READ_LE_UINT16(entry));

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;
		}

		if (flags & 0x40) {
			fi._foreColor = 2;
			fi._curFont = _bVoy->boltEntry(0x400)._fontResource;
			fi._justify = ALIGN_CENTER;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(0, READ_LE_UINT16(entry));

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;
		}

		if (flags & 2) {
			fi._foreColor = 1;
			fi._curFont = _bVoy->boltEntry(0x400)._fontResource;
			fi._justify = ALIGN_LEFT;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(38, READ_LE_UINT16(entry));

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;

			fi._foreColor = 2;
			fi._justify = ALIGN_LEFT;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(198, READ_LE_UINT16(entry));

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;
		}

		if (flags & 4) {
			fi._foreColor = 1;
			fi._curFont = _bVoy->boltEntry(0x402)._fontResource;
			fi._justify = ALIGN_CENTER;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(0, READ_LE_UINT16(entry));

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;

			fi._foreColor = 2;
			fi._curFont = _bVoy->boltEntry(0x400)._fontResource;
			fi._justify = ALIGN_CENTER;
			fi._justifyWidth = 384;
			fi._justifyHeight = 240;
			fi._pos = Common::Point(0, READ_LE_UINT16(entry) + 13);

			_screen->_vPort->drawText(msg);
			msg += strlen(msg) + 1;
		}

		if (flags & 0x20) {
			flipPageAndWait();
			_eventsManager->delayClick(READ_LE_UINT16(entry + 2) * 60);
		}

		if (shouldQuit() || _eventsManager->_mouseClicked)
			break;
	}

	_soundManager->stopVOCPlay();
	_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	_bVoy->freeBoltGroup(0x400);
}

void VoyeurEngine::doPiracy() {
	_screen->screenReset();
	_screen->setColor(1, 0, 0, 0);
	_screen->setColor(2, 255, 255, 255);
	_eventsManager->_intPtr._hasPalette = true;
	_screen->_vPort->setupViewPort(NULL);
	_screen->_vPort->fillPic(1);

	FontInfoResource &fi = *_screen->_fontPtr;
	fi._curFont = _bVoy->boltEntry(0x101)._fontResource;
	fi._foreColor = 2;
	fi._backColor = 2;
	fi._fontSaveBack = false;
	fi._fontFlags = DISPFLAG_NONE;
	fi._justify = ALIGN_CENTER;
	fi._justifyWidth = 384;
	fi._justifyHeight = 230;

	// Loop through the piracy message array to draw each line
	for (int idx = 0, yp = 33; idx < 10; ++idx) {
		fi._pos = Common::Point(0, yp);
		_screen->_vPort->drawText(PIRACY_MESSAGE[idx]);

		yp += fi._curFont->_fontHeight + 4;
	}

	flipPageAndWait();
	_eventsManager->getMouseInfo();
	_eventsManager->delayClick(720);
}

void VoyeurEngine::reviewTape() {
	int eventStart = 0;
	int newX = -1;
	int newY = -1;
	int eventLine = 7;
	Common::Rect tempRect(58, 30, 58 + 223, 30 + 124);
	Common::Point pt;
	int foundIndex;

	_bVoy->getBoltGroup(0x900);
	PictureResource *cursor = _bVoy->boltEntry(0x903)._picResource;

	if ((_voy->_eventCount - 8) != 0)
		eventStart = MAX(_voy->_eventCount - 8, 0);

	if ((eventStart + _voy->_eventCount) <= 7)
		eventLine = eventStart + _voy->_eventCount - 1;

	bool breakFlag = false;
	while (!shouldQuit() && !breakFlag) {
		_voy->_viewBounds = _bVoy->boltEntry(0x907)._rectResource;
		Common::Array<RectEntry> &hotspots = _bVoy->boltEntry(0x906)._rectResource->_entries;

		_screen->_backColors = _bVoy->boltEntry(0x902)._cMapResource;
		_screen->_backgroundPage = _bVoy->boltEntry(0x901)._picResource;
		_screen->_vPort->setupViewPort(_screen->_backgroundPage);
		_screen->_backColors->startFade();

		flipPageAndWaitForFade();

		_screen->setColor(1, 32, 32, 32);
		_screen->setColor(2, 96, 96, 96);
		_screen->setColor(3, 160, 160, 160);
		_screen->setColor(4, 224, 224, 224);
		_screen->setColor(9, 24, 64, 24);
		_screen->setColor(10, 64, 132, 64);
		_screen->setColor(11, 100, 192, 100);
		_screen->setColor(12, 120, 248, 120);
		_eventsManager->setCursorColor(128, 1);

		_eventsManager->_intPtr._hasPalette = true;
		_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x909)._fontResource;
		_screen->_fontPtr->_fontSaveBack = false;
		_screen->_fontPtr->_fontFlags = DISPFLAG_NONE;

		_eventsManager->getMouseInfo();
		if (newX == -1) {
			_eventsManager->setMousePos(Common::Point(hotspots[1].left + 12, hotspots[1].top + 6));
		} else {
			_eventsManager->setMousePos(Common::Point(newX, newY));
		}

		_currentVocId = 151;
		_voy->_vocSecondsOffset = 0;
		bool needRedraw = true;
		do {
			if (_currentVocId != -1 && !_soundManager->getVOCStatus()) {
				_voy->_musicStartTime = _voy->_RTVNum;
				_soundManager->startVOCPlay(_currentVocId);
			}

			if (needRedraw) {
				needRedraw = false;
				flipPageAndWait();

				_screen->_drawPtr->_penColor = 0;
				_screen->_drawPtr->_pos = Common::Point(tempRect.left, tempRect.top);
				_screen->_backgroundPage->sFillBox(tempRect.width(), tempRect.height());

				int yp = 45;
				int eventNum = eventStart;
				for (int lineNum = 0; lineNum < 8 && eventNum < _voy->_eventCount; ++lineNum, ++eventNum) {
					_screen->_fontPtr->_picFlags = DISPFLAG_NONE;
					_screen->_fontPtr->_picSelect = 0xff;
					_screen->_fontPtr->_picPick = 7;
					_screen->_fontPtr->_picOnOff = (lineNum == eventLine) ? 8 : 0;
					_screen->_fontPtr->_pos = Common::Point(68, yp);
					_screen->_fontPtr->_justify = ALIGN_LEFT;
					_screen->_fontPtr->_justifyWidth = 0;
					_screen->_fontPtr->_justifyHeight = 0;

					Common::String msg = _eventsManager->getEvidString(eventNum);
					_screen->_backgroundPage->drawText(msg);

					yp += 15;
				}

				_screen->_vPort->addSaveRect(
					_screen->_vPort->_lastPage, tempRect);
				flipPageAndWait();

				_screen->_vPort->addSaveRect(
					_screen->_vPort->_lastPage, tempRect);
			}

			_screen->sDrawPic(cursor, _screen->_vPort,
				_eventsManager->getMousePos());
			flipPageAndWait();

			_eventsManager->getMouseInfo();
			foundIndex = -1;

			Common::Point tempPos = _eventsManager->getMousePos() + Common::Point(14, 7);
			for (uint idx = 0; idx < hotspots.size(); ++idx) {
				if (hotspots[idx].contains(tempPos)) {
					// Found hotspot area
					foundIndex = idx;
					break;
				}
			}

			pt = _eventsManager->getMousePos();
			if (tempPos.x >= 68 && tempPos.x <= 277 && tempPos.y >= 31 && tempPos.y <= 154) {
				tempPos.y -= 2;
				foundIndex = (tempPos.y - 31) / 15;
				if ((tempPos.y - 31) % 15 >= 12 || (eventStart + foundIndex) >= _voy->_eventCount) {
					_eventsManager->setCursorColor(128, 0);
					foundIndex = 999;
				} else if (!_eventsManager->_leftClick) {
					_eventsManager->setCursorColor(128, 2);
					foundIndex = -1;
				} else {
					_eventsManager->setCursorColor(128, 2);
					eventLine =  foundIndex;

					flipPageAndWait();

					_screen->_drawPtr->_penColor = 0;
					_screen->_drawPtr->_pos = Common::Point(tempRect.left, tempRect.top);
					_screen->_backgroundPage->sFillBox(tempRect.width(), tempRect.height());

					int yp = 45;
					int eventNum = eventStart;
					for (int idx = 0; idx < 8 && eventNum < _voy->_eventCount; ++idx, ++eventNum) {
						_screen->_fontPtr->_picFlags = DISPFLAG_NONE;
						_screen->_fontPtr->_picSelect = 0xff;
						_screen->_fontPtr->_picPick = 7;
						_screen->_fontPtr->_picOnOff = (idx == eventLine) ? 8 : 0;
						_screen->_fontPtr->_pos = Common::Point(68, yp);
						_screen->_fontPtr->_justify = ALIGN_LEFT;
						_screen->_fontPtr->_justifyWidth = 0;
						_screen->_fontPtr->_justifyHeight = 0;

						Common::String msg = _eventsManager->getEvidString(eventNum);
						_screen->_backgroundPage->drawText(msg);

						yp += 15;
					}

					_screen->_vPort->addSaveRect(
						_screen->_vPort->_lastPage, tempRect);
					flipPageAndWait();

					_screen->_vPort->addSaveRect(
						_screen->_vPort->_lastPage, tempRect);
					flipPageAndWait();

					_eventsManager->getMouseInfo();
					foundIndex = -1;
				}
			} else if ((_voy->_eventFlags & EVTFLAG_40) && _voy->_viewBounds->left == pt.x &&
					_voy->_viewBounds->bottom == pt.y) {
				foundIndex = 999;
			} else if ((_voy->_eventFlags & EVTFLAG_40) && _voy->_viewBounds->left == pt.x &&
					_voy->_viewBounds->top == pt.y) {
				foundIndex = 998;
			} else {
				_eventsManager->setCursorColor(128, (foundIndex == -1) ? 0 : 1);
			}

			_eventsManager->_intPtr._hasPalette = true;

			if (_eventsManager->_mouseClicked) {
				switch (foundIndex) {
				case 2:
					if (eventStart > 0) {
						--eventStart;
						needRedraw = true;
					}
					foundIndex = -1;
					break;

				case 3:
					if (eventStart > 0) {
						eventStart -= 8;
						if (eventStart < 0)
							eventStart = 0;
						needRedraw = true;
					}
					foundIndex = -1;
					break;

				case 4:
					if ((_voy->_eventCount - 8) > eventStart) {
						++eventStart;
						needRedraw = true;
					}
					foundIndex = -1;
					break;

				case 5:
					if (_voy->_eventCount >= 8 && (_voy->_eventCount - 8) != eventStart) {
						eventStart += 8;
						if ((_voy->_eventCount - 8) < eventStart)
							eventStart = _voy->_eventCount - 8;
						needRedraw = true;
					}
					foundIndex = -1;
					break;

				default:
					break;
				}

				while (eventLine > 0 && (eventLine + eventStart) >= _voy->_eventCount)
					--eventLine;
			}

			pt = _eventsManager->getMousePos();
			if (_eventsManager->_mouseClicked && _voy->_viewBounds->left == pt.x &&
					(_voy->_eventFlags & EVTFLAG_40) && _eventsManager->_rightClick) {
				_controlPtr->_state->_victimIndex = (pt.y / 60) + 1;
				foundIndex = -1;
				_eventsManager->_rightClick = 0;
			}

			if (_eventsManager->_rightClick)
				foundIndex = 0;

		} while (!shouldQuit() && (!_eventsManager->_mouseClicked || foundIndex == -1));

		if (shouldQuit())
			return;

		newY = _eventsManager->getMousePos().y;
		_voy->_fadingType = 0;
		_voy->_viewBounds = nullptr;
		_screen->_vPort->setupViewPort(NULL);

		if (_currentVocId != -1) {
			_voy->_vocSecondsOffset = _voy->_RTVNum - _voy->_musicStartTime;
			_soundManager->stopVOCPlay();
		}

		// Break out if the exit button was pressed
		if (!foundIndex)
			break;

		int eventIndex = eventStart + eventLine;
		VoyeurEvent &e = _voy->_events[eventIndex];
		switch (e._type) {
		case EVTYPE_VIDEO:
			playAVideoEvent(eventIndex);
			break;

		case EVTYPE_AUDIO: {
			_audioVideoId = e._audioVideoId;
			_voy->_vocSecondsOffset = e._computerOn;

			_bVoy->getBoltGroup(0x7F00);
			_screen->_backgroundPage = _bVoy->boltEntry(0x7F00 +
				BLIND_TABLE[_audioVideoId] * 2)._picResource;
			_screen->_backColors = _bVoy->boltEntry(0x7F01 +
				BLIND_TABLE[_audioVideoId] * 2)._cMapResource;

			_screen->_vPort->setupViewPort(_screen->_backgroundPage);
			_screen->_backColors->startFade();
			flipPageAndWaitForFade();

			_eventsManager->_intPtr._flashStep = 1;
			_eventsManager->_intPtr._flashTimer = 0;
			_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;

			// Play sound for the given duration
			_soundManager->setVOCOffset(_voy->_vocSecondsOffset);
			Common::String filename = _soundManager->getVOCFileName(
				_audioVideoId + 159);
			_soundManager->startVOCPlay(filename);

			uint32 secondsDuration = e._computerOff;

			_eventsManager->getMouseInfo();
			while (!_eventsManager->_mouseClicked && _soundManager->getVOCStatus() &&
					_soundManager->getVOCFrame() < secondsDuration) {
				_eventsManager->getMouseInfo();
				_eventsManager->delay(10);
			}

			_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
			_soundManager->stopVOCPlay();
			_bVoy->freeBoltGroup(0x7F00);
			break;
		}

		case EVTYPE_EVID:
			_voy->reviewAnEvidEvent(eventIndex);

			_voy->_vocSecondsOffset = _voy->_RTVNum - _voy->_musicStartTime;
			_soundManager->stopVOCPlay();
			_bVoy->getBoltGroup(0x900);
			break;

		case EVTYPE_COMPUTER:
			_voy->reviewComputerEvent(eventIndex);

			_voy->_vocSecondsOffset = _voy->_RTVNum - _voy->_musicStartTime;
			_soundManager->stopVOCPlay();
			_bVoy->getBoltGroup(0x900);
			break;

		default:
			break;
		}
	}

	_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;

	_screen->_vPort->fillPic(0);
	flipPageAndWait();
	_bVoy->freeBoltGroup(0x900);
}

void VoyeurEngine::doGossip() {
	_screen->resetPalette();
	_screen->screenReset();

	if (!_bVoy->getBoltGroup(0x300))
		return;

	// Load the gossip animation
	RL2Decoder decoder;
	decoder.loadRL2File("a2050100.rl2", false);
	decoder.start();

	// Get the resource data for the first gossip video
	PictureResource *bgPic = _bVoy->boltEntry(0x300)._picResource;
	CMapResource *pal = _bVoy->boltEntry(0x301)._cMapResource;
	pal->startFade();

	// Transfer initial background to video decoder
	PictureResource videoFrame(decoder.getRL2VideoTrack()->getBackSurface());
	bgPic->_bounds.moveTo(0, 0);
	_screen->sDrawPic(bgPic, &videoFrame, Common::Point(0, 0));

	byte *frameNumsP = _bVoy->memberAddr(0x309);
	byte *posP = _bVoy->boltEntry(0x30A)._data;

	// Play the initial gossip video
	decoder.play(this, 0x302, frameNumsP, posP);
	decoder.close();

	// Reset the palette and clear the screen
	_screen->resetPalette();
	_screen->screenReset();

	// Play interview video
	RL2Decoder decoder2;
	decoder2.loadRL2File("a2110100.rl2", true);
	decoder2.start();

	_eventsManager->getMouseInfo();
	decoder2.play(this);
	decoder2.close();

	_bVoy->freeBoltGroup(0x300);
	_screen->screenReset();
}

void VoyeurEngine::doTapePlaying() {
	if (!_bVoy->getBoltGroup(0xA00))
		return;

	_eventsManager->getMouseInfo();
	_screen->_backColors = _bVoy->boltEntry(0xA01)._cMapResource;
	_screen->_backgroundPage = _bVoy->boltEntry(0xA00)._picResource;
	PictureResource *pic = _bVoy->boltEntry(0xA02)._picResource;
	VInitCycleResource *cycle = _bVoy->boltEntry(0xA05)._vInitCycleResource;

	_screen->_vPort->setupViewPort(_screen->_backgroundPage);
	_screen->sDrawPic(pic, _screen->_vPort, Common::Point(57, 30));
	_screen->_backColors->startFade();
	flipPageAndWaitForFade();

	cycle->vStartCycle();

	_soundManager->startVOCPlay("vcr.voc");
	while (!shouldQuit() && !_eventsManager->_mouseClicked && _soundManager->getVOCStatus()) {
		_eventsManager->delayClick(2);
	}

	_soundManager->stopVOCPlay();
	cycle->vStopCycle();
	_bVoy->freeBoltGroup(0xA00);
}

bool VoyeurEngine::checkForMurder() {
	int oldMurderIndex = _controlPtr->_state->_victimMurderIndex;

	for (int idx = 0; idx < _voy->_eventCount; ++idx) {
		VoyeurEvent &evt = _voy->_events[idx];

		if (evt._type == EVTYPE_VIDEO) {
			switch (_controlPtr->_state->_victimIndex) {
			case 1:
				if (evt._audioVideoId == 41 && evt._computerOn <= 15 &&
						(evt._computerOff + evt._computerOn) >= 16) {
					_controlPtr->_state->_victimMurderIndex = 1;
				}
				break;

			case 2:
				if (evt._audioVideoId == 53 && evt._computerOn <= 19 &&
						(evt._computerOff + evt._computerOn) >= 21) {
					_controlPtr->_state->_victimMurderIndex = 2;
				}
				break;

			case 3:
				if (evt._audioVideoId == 50 && evt._computerOn <= 28 &&
						(evt._computerOff + evt._computerOn) >= 29) {
					_controlPtr->_state->_victimMurderIndex = 3;
				}
				break;

			case 4:
				if (evt._audioVideoId == 43 && evt._computerOn <= 10 &&
						(evt._computerOff + evt._computerOn) >= 14) {
					_controlPtr->_state->_victimMurderIndex = 4;
				}
				break;

			default:
				break;
			}
		}

		if (_controlPtr->_state->_victimMurderIndex == _controlPtr->_state->_victimIndex) {
			_voy->_videoEventId = idx;
			return true;
		}
	}

	_controlPtr->_state->_victimMurderIndex = oldMurderIndex;
	_voy->_videoEventId = -1;
	return false;
}

bool VoyeurEngine::checkForIncriminate() {
	_voy->_incriminatedVictimNumber = 0;

	for (int idx = 0; idx < _voy->_eventCount; ++idx) {
		VoyeurEvent &evt = _voy->_events[idx];

		if (evt._type == EVTYPE_VIDEO) {
			if (evt._audioVideoId == 44 && evt._computerOn <= 40 &&
					(evt._computerOff + evt._computerOn) >= 70) {
				_voy->_incriminatedVictimNumber = 1;
			}

			if (evt._audioVideoId == 44 && evt._computerOn <= 79 &&
					(evt._computerOff + evt._computerOn) >= 129) {
				_voy->_incriminatedVictimNumber = 1;
			}

			if (evt._audioVideoId == 20 && evt._computerOn <= 28 &&
					(evt._computerOff + evt._computerOn) >= 45) {
				_voy->_incriminatedVictimNumber = 2;
			}

			if (evt._audioVideoId == 35 && evt._computerOn <= 17 &&
					(evt._computerOff + evt._computerOn) >= 36) {
				_voy->_incriminatedVictimNumber = 3;
			}

			if (evt._audioVideoId == 30 && evt._computerOn <= 80 &&
					(evt._computerOff + evt._computerOn) >= 139) {
				_voy->_incriminatedVictimNumber = 4;
			}
		}

		if (_voy->_incriminatedVictimNumber) {
			_controlPtr->_state->_victimMurderIndex = 88;
			_voy->_videoEventId = idx;
			return true;
		}
	}

	_voy->_videoEventId = -1;
	return false;
}

void VoyeurEngine::playAVideoEvent(int eventIndex) {
	VoyeurEvent &evt = _voy->_events[eventIndex];
	_audioVideoId = evt._audioVideoId;
	_voy->_vocSecondsOffset = evt._computerOn;
	_eventsManager->_videoDead = evt._dead;
	_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;

	playAVideoDuration(_audioVideoId, evt._computerOff);

	_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
	if (_eventsManager->_videoDead != -1) {
		_bVoy->freeBoltGroup(0xE00);
		_eventsManager->_videoDead = -1;
		flipPageAndWait();
		_eventsManager->_videoDead = -1;
	}

	_audioVideoId = -1;
	if (_eventsManager->_videoDead != -1) {
		_bVoy->freeBoltGroup(0xE00);
		_eventsManager->_videoDead = -1;
		flipPageAndWait();
	}
}

int VoyeurEngine::getChooseButton()  {
	int prevIndex = -2;
	Common::Array<RectEntry> &hotspots = _bVoy->boltEntry(_playStampGroupId
		+ 6)._rectResource->_entries;
	int selectedIndex = -1;

	_screen->_vPort->setupViewPort(_screen->_backgroundPage);
	_screen->_backColors->_steps = 0;
	_screen->_backColors->startFade();
	flipPageAndWait();

	_voy->_viewBounds = _bVoy->boltEntry(_playStampGroupId + 7)._rectResource;
	PictureResource *cursorPic = _bVoy->boltEntry(_playStampGroupId + 2)._picResource;

	do {
		do {
			if (_currentVocId != -1 && !_soundManager->getVOCStatus())
				_soundManager->startVOCPlay(_currentVocId);

			_eventsManager->getMouseInfo();
			selectedIndex = -1;
			Common::Point pt = _eventsManager->getMousePos();

			for (uint idx = 0; idx < hotspots.size(); ++idx) {
				if (hotspots[idx].contains(pt)) {
					if (!_voy->_victimMurdered || ((int)idx + 1) != _controlPtr->_state->_victimIndex) {
						selectedIndex = idx;
						if (selectedIndex != prevIndex) {
							PictureResource *btnPic = _bVoy->boltEntry(_playStampGroupId + 8 + idx)._picResource;
							_screen->sDrawPic(btnPic, _screen->_vPort,
								Common::Point(106, 200));

							cursorPic = _bVoy->boltEntry(_playStampGroupId + 4)._picResource;
						}
					}
				}
			}

			if (selectedIndex == -1) {
				cursorPic = _bVoy->boltEntry(_playStampGroupId + 2)._picResource;
				PictureResource *btnPic = _bVoy->boltEntry(_playStampGroupId + 12)._picResource;
				_screen->sDrawPic(btnPic, _screen->_vPort,
					Common::Point(106, 200));
			}

			_screen->sDrawPic(cursorPic, _screen->_vPort,
				Common::Point(pt.x + 13, pt.y - 12));

			flipPageAndWait();
		} while (!shouldQuit() && !_eventsManager->_mouseClicked);
	} while (!shouldQuit() && selectedIndex == -1 && !_eventsManager->_rightClick);

	return selectedIndex;
}

void VoyeurEngine::makeViewFinder() {
	_screen->_backgroundPage = _bVoy->boltEntry(0x103)._picResource;
	_screen->sDrawPic(_screen->_backgroundPage,
		_screen->_vPort, Common::Point(0, 0));
	CMapResource *pal = _bVoy->boltEntry(0x104)._cMapResource;

	int palOffset = 0;
	switch (_voy->_transitionId) {
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 17:
		palOffset = 0;
		break;
	case 3:
		palOffset = 1;
		break;
	case 4:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		palOffset = 2;
		break;
	default:
		break;
	}

	_screen->_vPort->drawIfaceTime();
	doTimeBar();
	pal->startFade();

	flipPageAndWaitForFade();

	_screen->setColor(241, 105, 105, 105);
	_screen->setColor(242, 105, 105, 105);
	_screen->setColor(243, 105, 105, 105);
	_screen->setColor(palOffset + 241, 219, 235, 235);

	_eventsManager->_intPtr._hasPalette = true;
}

void VoyeurEngine::makeViewFinderP() {
	_screen->screenReset();
}

void VoyeurEngine::initIFace() {
	int playStamp1 = _playStampGroupId;
	switch (_voy->_transitionId) {
	case 0:
		break;
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		_playStampGroupId = 0xB00;
		break;
	case 3:
		_playStampGroupId = 0xC00;
		break;
	default:
		_playStampGroupId = 0xD00;
		break;
	}
	if (playStamp1 != -1)
		_bVoy->freeBoltGroup(playStamp1);

	_bVoy->getBoltGroup(_playStampGroupId);
	CMapResource *pal = _bVoy->boltEntry(_playStampGroupId + 2)._cMapResource;
	pal->startFade();

	// Reset the mansion view off to it's prior position (if any)
	doScroll(_mansionViewPos);

	_voy->_viewBounds = _bVoy->boltEntry(_playStampGroupId)._rectResource;

	// Show the cursor using ScummVM functionality
	_eventsManager->showCursor();

	// Note: the original did two loops to preload members here, which is
	// redundant for ScummVM, since computers are faster these days, and
	// getting resources as needed will be fast enough.
}

void VoyeurEngine::doScroll(const Common::Point &pt) {
	Common::Rect clipRect(72, 47, 72 + 240, 47 + 148);
	_screen->_vPort->setupViewPort(NULL, &clipRect);

	int base = 0;
	switch (_voy->_transitionId) {
	case 0:
		break;
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		base = 0xB00;
		break;
	case 3:
		base = 0xC00;
		break;
	default:
		base = 0xD00;
	}

	if (base) {
		PictureResource *pic = _bVoy->boltEntry(base + 3)._picResource;
 		_screen->sDrawPic(pic, _screen->_vPort, Common::Point(784 - pt.x - 712, 150 - pt.y - 104));
		pic = _bVoy->boltEntry(base + 4)._picResource;
		_screen->sDrawPic(pic, _screen->_vPort, Common::Point(784 - pt.x - 712, 150 - pt.y - 44));
		pic = _bVoy->boltEntry(base + 5)._picResource;
		_screen->sDrawPic(pic, _screen->_vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 16));
		pic = _bVoy->boltEntry(base + 6)._picResource;
		_screen->sDrawPic(pic, _screen->_vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 76));
		pic = _bVoy->boltEntry(base + 7)._picResource;
		_screen->sDrawPic(pic, _screen->_vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 136));
	}

	_screen->_vPort->setupViewPort(NULL);
}

void VoyeurEngine::checkTransition() {
	Common::String time, day;

	if (_voy->_transitionId != _checkTransitionId) {
		// Get the day
		day = getDayName();

		// Only proceed if a valid day string was returned
		if (!day.empty()) {
			_screen->fadeDownICF(6);

			// Get the time of day string
			time = getTimeOfDay();

			// Show a transition card with the day and time, and wait
			doTransitionCard(day, time);
			_eventsManager->delayClick(180);
		}

		_checkTransitionId = _voy->_transitionId;
		centerMansionView();
	}
}

Common::String VoyeurEngine::getDayName() {
	switch (_voy->_transitionId) {
	case 0:
		return "";
	case 1:
	case 2:
	case 3:
	case 4:
		return SATURDAY;
	case 17:
		return MONDAY;
	default:
		return SUNDAY;
	}
}

Common::String VoyeurEngine::getTimeOfDay() {
	if (_voy->_transitionId == 17)
		return "";

	return Common::String::format("%d:%02d%s", _gameHour, _gameMinute, _voy->_isAM ? AM : PM);
}

int VoyeurEngine::doComputerText(int maxLen) {
	FontInfoResource &font = *_screen->_fontPtr;
	int totalChars = 0;

	font._curFont = _bVoy->boltEntry(0x4910)._fontResource;
	font._foreColor = 129;
	font._fontSaveBack = false;
	font._fontFlags = DISPFLAG_NONE;
	if (_voy->_vocSecondsOffset > 60)
		_voy->_vocSecondsOffset = 0;

	if (_voy->_RTVNum > _voy->_computerTimeMax && maxLen == 9999) {
		if (_currentVocId != -1)
			_soundManager->startVOCPlay(_currentVocId);
		font._justify = ALIGN_LEFT;
		font._justifyWidth = 384;
		font._justifyHeight = 100;
		font._pos = Common::Point(128, 100);
		_screen->_vPort->drawText(END_OF_MESSAGE);
	} else if (_voy->_RTVNum < _voy->_computerTimeMin && maxLen == 9999) {
		if (_currentVocId != -1)
			_soundManager->startVOCPlay(_currentVocId);
		font._justify = ALIGN_LEFT;
		font._justifyWidth = 384;
		font._justifyHeight = 100;
		font._pos = Common::Point(120, 100);
		_screen->_vPort->drawText(START_OF_MESSAGE);
	} else {
		char *msg = (char *)_bVoy->memberAddr(0x4900 + _voy->_computerTextId);
		font._pos = Common::Point(96, 60);

		bool showEnd = true;
		int yp = 60;
		do {
			if (_currentVocId != -1 && !_soundManager->getVOCStatus()) {
				if (_voy->_vocSecondsOffset > 60)
					_voy->_vocSecondsOffset = 0;
				_soundManager->startVOCPlay(_currentVocId);
			}

			char c = *msg++;
			if (c == '\0') {
				if (showEnd) {
					_eventsManager->delay(90);
					_screen->_drawPtr->_pos = Common::Point(96, 54);
					_screen->_drawPtr->_penColor = 254;
					_screen->_vPort->sFillBox(196, 124);
					_screen->_fontPtr->_justify = ALIGN_LEFT;
					_screen->_fontPtr->_justifyWidth = 384;
					_screen->_fontPtr->_justifyHeight = 100;
					_screen->_fontPtr->_pos = Common::Point(128, 100);
					_screen->_vPort->drawText(END_OF_MESSAGE);
				}
				break;
			}

			if (c == '~' || c == '^') {
				if (c == '^') {
					yp += 10;
				} else {
					_eventsManager->delay(90);
					_screen->_drawPtr->_pos = Common::Point(96, 54);
					_screen->_drawPtr->_penColor = 255;
					_screen->_vPort->sFillBox(196, 124);
					yp = 60;
				}

				_screen->_fontPtr->_pos = Common::Point(96, yp);
			} else if (c == '_') {
				showEnd = false;
			} else {
				_screen->_fontPtr->_justify = ALIGN_LEFT;
				_screen->_fontPtr->_justifyWidth = 0;
				_screen->_fontPtr->_justifyHeight = 0;
				_screen->_vPort->drawText(Common::String(c));
				_eventsManager->delay(4);
			}

			flipPageAndWait();
			_eventsManager->getMouseInfo();
			++totalChars;

		} while (!shouldQuit() && !_eventsManager->_mouseClicked && totalChars < maxLen);

		_voy->_computerTimeMax = 0;
	}

	flipPageAndWait();

	_screen->_fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	return totalChars;
}

void VoyeurEngine::getComputerBrush() {
	if (!_bVoy->getBoltGroup(0x4900))
		return;

	PictureResource *pic = _bVoy->boltEntry(0x490E)._picResource;
	int xp = (384 - pic->_bounds.width()) / 2;
	int yp = (240 - pic->_bounds.height()) / 2 - 4;

	_screen->_vPort->drawPicPerm(pic, Common::Point(xp, yp));

	CMapResource *pal = _bVoy->boltEntry(0x490F)._cMapResource;
	pal->startFade();
}

void VoyeurEngine::doTimeBar() {
	flashTimeBar();

	if (_voy->_RTVLimit > 0) {
		if (_voy->_RTVNum > _voy->_RTVLimit || _voy->_RTVNum < 0)
			_voy->_RTVNum = _voy->_RTVLimit - 1;

		_timeBarVal = _voy->_RTVNum;
		int height = ((_voy->_RTVLimit - _voy->_RTVNum) * 59) / _voy->_RTVLimit;
		int fullHeight = MAX(151 - height, 93);

		_screen->_drawPtr->_penColor = 134;
		_screen->_drawPtr->_pos = Common::Point(39, 92);

		_screen->_vPort->sFillBox(6, fullHeight - 92);
		if (height > 0) {
			_screen->setColor(215, 238, 238, 238);
			_eventsManager->_intPtr._hasPalette = true;

			_screen->_drawPtr->_penColor = 215;
			_screen->_drawPtr->_pos = Common::Point(39, fullHeight);
			_screen->_vPort->sFillBox(6, height);
		}
	}
}

void VoyeurEngine::flashTimeBar() {
	if (_voy->_RTVNum >= 0 && (_voy->_RTVLimit - _voy->_RTVNum) < 11 &&
		(_eventsManager->_intPtr._flashTimer >= (_flashTimeVal + 15) ||
		_eventsManager->_intPtr._flashTimer < _flashTimeVal)) {
		// Within camera low power range
		_flashTimeVal = _eventsManager->_intPtr._flashTimer;

		if (_flashTimeFlag)
			_screen->setColor(240, 220, 20, 20);
		else
			_screen->setColor(240, 220, 220, 220);

		_eventsManager->_intPtr._hasPalette = true;
		_flashTimeFlag = !_flashTimeFlag;
	}
}

void VoyeurEngine::checkPhoneCall() {
	if ((_voy->_RTVLimit - _voy->_RTVNum) >= 36 && _voy->_totalPhoneCalls < 5 &&
			_currentVocId <= 151 && _currentVocId > 146) {
		if ((_voy->_switchBGNum < _checkPhoneVal || _checkPhoneVal > 180) &&
				!_soundManager->getVOCStatus()) {
			int soundIndex;
			do {
				soundIndex = getRandomNumber(4);
			} while (_voy->_phoneCallsReceived[soundIndex]);
			_currentVocId = 154 + soundIndex;

			_soundManager->stopVOCPlay();
			_soundManager->startVOCPlay(_currentVocId);
			_checkPhoneVal = _voy->_switchBGNum;
			_voy->_phoneCallsReceived[soundIndex] = true;
			++_voy->_totalPhoneCalls;
		}
	}
}

void VoyeurEngine::doEvidDisplay(int evidId, int eventId) {
	_eventsManager->getMouseInfo();
	flipPageAndWait();

	if (_currentVocId != -1) {
		_voy->_vocSecondsOffset = _voy->_RTVNum - _voy->_musicStartTime;
		_soundManager->stopVOCPlay();
	}

	_bVoy->getBoltGroup(_voy->_boltGroupId2);
	PictureResource *pic = _bVoy->boltEntry(_voy->_boltGroupId2 + evidId * 2)._picResource;
	_screen->sDrawPic(pic, _screen->_vPort, Common::Point(
		(384 - pic->_bounds.width()) / 2, (240 - pic->_bounds.height()) / 2));
	_bVoy->freeBoltMember(_voy->_boltGroupId2 + evidId * 2);

	CMapResource *pal = _bVoy->boltEntry(_voy->_boltGroupId2 + evidId * 2 + 1)._cMapResource;
	pal->startFade();

	while (!shouldQuit() && (_eventsManager->_fadeStatus & 1))
		_eventsManager->delay(1);
	_bVoy->freeBoltMember(_voy->_boltGroupId2 + evidId * 2 + 1);

	Common::Array<RectEntry> &hotspots = _bVoy->boltEntry(_playStampGroupId + 4)._rectResource->_entries;
	int count = hotspots[evidId]._count;

	if (count > 0) {
		for (int idx = 1; idx <= count; ++idx) {
			_voy->_evPicPtrs[idx - 1] = _bVoy->boltEntry(_voy->_boltGroupId2 +
				(evidId + idx) * 2)._picResource;
			_voy->_evCmPtrs[idx - 1] = _bVoy->boltEntry(_voy->_boltGroupId2 +
				(evidId + idx) * 2 + 1)._cMapResource;
		}
	}

	flipPageAndWait();
	_eventsManager->stopEvidDim();

	if (eventId == 999)
		_voy->addEvidEventStart(evidId);

	_eventsManager->getMouseInfo();

	int arrIndex = 0;
	int evidIdx = evidId;

	while (!shouldQuit() && !_eventsManager->_rightClick) {
		_voyeurArea = AREA_EVIDENCE;

		if (_currentVocId != -1 && !_soundManager->getVOCStatus()) {
			if (_voy->_vocSecondsOffset > 60)
				_voy->_vocSecondsOffset = 0;

			_soundManager->startVOCPlay(_currentVocId);
		}

		_eventsManager->delayClick(600);
		if (_eventsManager->_rightClick)
			break;
		if (count == 0 || evidIdx >= eventId)
			continue;

		pic = _voy->_evPicPtrs[arrIndex];
		_screen->sDrawPic(pic, _screen->_vPort,
			Common::Point((384 - pic->_bounds.width()) / 2,
			(240 - pic->_bounds.height()) / 2));
		_voy->_evCmPtrs[arrIndex]->startFade();
		while (!shouldQuit() && (_eventsManager->_fadeStatus & 1))
			_eventsManager->delay(1);

		flipPageAndWait();
		_eventsManager->delay(6);

		++evidIdx;
		++arrIndex;
		--count;
	}

	if (eventId == 999)
		_voy->addEvidEventEnd(evidIdx);

	for (int idx = 1; idx <= hotspots[evidId]._count; ++idx) {
		_bVoy->freeBoltMember(_voy->_boltGroupId2 + (evidId + idx) * 2);
		_bVoy->freeBoltMember(_voy->_boltGroupId2 + (evidId + idx) * 2 + 1);
	}
}

void VoyeurEngine::centerMansionView() {
	_mansionViewPos = Common::Point((MANSION_MAX_X - MANSION_VIEW_WIDTH) / 2,
		(MANSION_MAX_Y - MANSION_VIEW_HEIGHT) / 2);
}

} // End of namespace Voyeur
