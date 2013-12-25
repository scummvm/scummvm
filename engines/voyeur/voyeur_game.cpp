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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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
	_filesManager.openBoltLib("stampblt.blt", _stampLibPtr);

	_stampLibPtr->getBoltGroup(0);
	_resolvePtr = &RESOLVE_TABLE[0];
	initStamp();

	PtrResource *threadsList = _stampLibPtr->boltEntry(3)._ptrResource;
	ThreadResource *threadP = threadsList->_entries[0]->_threadResource;
	threadP->initThreadStruct(0, 0);

	_voy._isAM = 0;
	_gameHour = 9;
	_gameMinute = 0;
	_eventsManager._v2A0A2 = 0;
	_voy._field46E = 1;

	int buttonId;
	bool breakFlag = false;
	while (!breakFlag && !shouldQuit()) {
		_eventsManager.getMouseInfo();
		_playStamp1 = _playStamp2 = -1;
		_eventsManager._videoComputerBut4 = -1;

		threadP->parsePlayCommands();

		bool flag = breakFlag = (_voy._field478 & 2) != 0;
		 
		switch (_voy._field470) {
		case 5:
			buttonId = threadP->doInterface();
			
			if (buttonId == -2) {
				switch (threadP->doApt()) {
				case 0:
					_voy._field472 = 140;
					break;
				case 1:
					_voy._field478 = -2;
					_voy._field46E = 1;
					threadP->chooseSTAMPButton(22);
					_voy._field472 = 143;
					break;
				case 2:
					_voy._field478 = -2;
					reviewTape();
					_voy._field46E = 1;
					_voy._field472 = 142;
					break;
				case 3:
					_voy._field478 = -2;
					threadP->chooseSTAMPButton(21);
					break;
				case 4:
					breakFlag = true;
					break;
				case 5:
					doGossip();
					_voy._field46E = 1;
					_voy._field472 = 141;
					_voy._field478 = -1;
					break;
				default:
					break;
				}
			} else {
				threadP->chooseSTAMPButton(buttonId);
			}

			flag = true;
			break;

		case 6:
			threadP->doRoom();
			break;

		case 16:
			_voy._transitionId = 17;
			buttonId = threadP->doApt();
			
			switch (buttonId) {
			case 1:
				threadP->chooseSTAMPButton(22);
				flag = true;
				break;
			case 2:
				reviewTape();
				_voy._field46E = 1;
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
			doTapePlaying();
			if (!checkForMurder() && _voy._transitionId <= 15)
				checkForIncriminate();

			if (_voy._videoEventId != -1)
				playAVideoEvent(_voy._videoEventId);
			_voy._field478 &= 0x10;
			threadP->chooseSTAMPButton(0);
			break;

		case 130: {
			//_tmflag = 1;
			if (_bVoy->getBoltGroup(_playStamp1)) {
				_graphicsManager._backgroundPage = _bVoy->boltEntry(_playStamp1)._picResource;
				_graphicsManager._backColors = _bVoy->boltEntry(_playStamp1 + 1)._cMapResource;

				buttonId = getChooseButton();
				if (_voy._fadeFunc)
					buttonId = 4;

				_bVoy->freeBoltGroup(_playStamp1);
				_graphicsManager.screenReset();
				_playStamp1 = -1;
				flag = true;

				if (buttonId == 4) {
					_voy._field470 = 131;
					_eventsManager.checkForKey();
					threadP->chooseSTAMPButton(buttonId);
					flag = true;
				} else {
					threadP->chooseSTAMPButton(buttonId);
					_voy._field46E = 1;
				}
			}
			break;
		}

		default:
			break;
		}

		do {
			if (flag) {
				if (_playStamp2 != -1) {
					_soundManager.stopVOCPlay();
					_playStamp2 = -1;
				}

				_eventsManager._videoComputerBut4 = -1;

				if (_voy._field47A != -1) {
					_bVoy->freeBoltGroup(_voy._field47A);
					_voy._field47A = -1;
				}

				if (_playStamp1 != -1) {
					_bVoy->freeBoltGroup(_playStamp1);
					_playStamp1 = -1;
				}

				// Break out of loop
				flag = false;

			} else if (threadP->_field40 & 2) {
				_eventsManager.getMouseInfo();
				threadP->chooseSTAMPButton(0);
				flag = true;
			} else {
				threadP->chooseSTAMPButton(0);
				flag = true;
			}
		} while (flag);
	}

	_voy._viewBounds = nullptr;
	closeStamp();
	_stampLibPtr->freeBoltGroup(0);
	delete _stampLibPtr;
}

void VoyeurEngine::initStamp() {
	ThreadResource::_stampFlags &= ~1;
	_stackGroupPtr = _controlGroupPtr;

	if (!_controlPtr->_entries[0])
		error("No control entries");

	ThreadResource::initUseCount();
}

void VoyeurEngine::closeStamp() {
	ThreadResource::unloadAllStacks(this);
}

void VoyeurEngine::reviewTape() {
//	int var22 = 0;
	int si = 0;
	int newX = -1;
	int newY = -1;
	int var20 = 7;
	Common::Rect tempRect(58, 30, 58 + 223, 30 + 124);
	Common::Point pt;
	int evtIndex = 0;
	int foundIndex;

	_bVoy->getBoltGroup(0x900);
	PictureResource *cursor = _bVoy->boltEntry(0x903)._picResource;

	if ((_voy._eventCount - 8) != 0)
		si = MAX(_voy._eventCount - 8, 0);

	if ((si + _voy._eventCount) <= 7)
		var20 = si + _voy._eventCount - 1;

	bool breakFlag = false;
	while (!shouldQuit() && !breakFlag) {
		_voy._viewBounds = _bVoy->boltEntry(0x907)._rectResource;
		byte *dataP = _bVoy->memberAddr(0x906);
		int varA = READ_LE_UINT16(dataP);
		_graphicsManager._backColors = _bVoy->boltEntry(0x902)._cMapResource;
		_graphicsManager._backgroundPage = _bVoy->boltEntry(0x901)._picResource;
		(*_graphicsManager._vPort)->setupViewPort(_graphicsManager._backgroundPage);
		_graphicsManager._backColors->startFade();

		(*_graphicsManager._vPort)->_flags |= 8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();
		while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
			_eventsManager.delay(1);

		_graphicsManager.setColor(1, 32, 32, 32);
		_graphicsManager.setColor(2, 96, 96, 96);
		_graphicsManager.setColor(3, 160, 160, 160);
		_graphicsManager.setColor(4, 224, 224, 224);
		_graphicsManager.setColor(9, 24, 64, 24);
		_graphicsManager.setColor(10, 64, 132, 64);
		_graphicsManager.setColor(11, 100, 192, 100);
		_graphicsManager.setColor(12, 120, 248, 120);
		_eventsManager.setCursorColor(128, 1);

		_eventsManager._intPtr.field38 = 1;
		_eventsManager._intPtr._hasPalette = true;
		_graphicsManager._fontPtr->_curFont = _bVoy->boltEntry(0x909)._fontResource;
		_graphicsManager._fontPtr->_fontSaveBack = false;
		_graphicsManager._fontPtr->_fontFlags = 0;

		_eventsManager.getMouseInfo();
		if (newX == -1) {
			_eventsManager.setMousePos(Common::Point((int16)READ_LE_UINT16(dataP + 10) + 12,
				(int16)READ_LE_UINT16(dataP + 12) + 6));
		} else {
			_eventsManager.setMousePos(Common::Point(newX, newY));
		}

		_playStamp2 = 151;
		_voy._vocSecondsOffset = 0;
		bool var1E = true;
		do {
			if (_playStamp2 != -1 && !_soundManager.getVOCStatus()) {
				_voy._field4AC = _voy._RTVNum;
				_soundManager.startVOCPlay(_playStamp2);
			}

			if (var1E) {
				var1E = false;
				(*_graphicsManager._vPort)->_flags |= 8;
				_graphicsManager.flipPage();
				_eventsManager.sWaitFlip();

				_graphicsManager._drawPtr->_penColor = 0;
				_graphicsManager._drawPtr->_pos = Common::Point(tempRect.left, tempRect.top);
				// TODO: Check - does drawText need to work on PictureResources?
				((ViewPortResource *)_graphicsManager._backgroundPage)->sFillBox(tempRect.width(), tempRect.height());

				newX = si;
				int yp = 45;
				evtIndex = si;
				for (int idx = 0; idx < 8 && evtIndex < _voy._eventCount; ++idx) {
					_graphicsManager._fontPtr->_picFlags = 0;
					_graphicsManager._fontPtr->_picSelect = 0xff;
					_graphicsManager._fontPtr->_picPick = 7;
					_graphicsManager._fontPtr->_picOnOff = (idx == var20) ? 8 : 0;
					_graphicsManager._fontPtr->_pos = Common::Point(68, yp);
					_graphicsManager._fontPtr->_justify = ALIGN_LEFT;
					_graphicsManager._fontPtr->_justifyWidth = 0;
					_graphicsManager._fontPtr->_justifyHeight = 0;

					Common::String msg = _eventsManager.getEvidString(evtIndex);
					// TODO: Does drawText need to work on picture resources?
					((ViewPortResource *)_graphicsManager._backgroundPage)->drawText(msg);
					
					yp += 15;
					++evtIndex;
				}

				(*_graphicsManager._vPort)->addSaveRect(
					(*_graphicsManager._vPort)->_lastPage, tempRect);
				(*_graphicsManager._vPort)->_flags |= 8;
				_graphicsManager.flipPage();
				_eventsManager.sWaitFlip();

				(*_graphicsManager._vPort)->addSaveRect(
					(*_graphicsManager._vPort)->_lastPage, tempRect);
			}

			_graphicsManager.sDrawPic(cursor, *_graphicsManager._vPort,
				_eventsManager.getMousePos());
			(*_graphicsManager._vPort)->_flags |= 8;
			_graphicsManager.flipPage();
			_eventsManager.sWaitFlip();

			_eventsManager.getMouseInfo();
			foundIndex = -1;
			Common::Point tempPos = _eventsManager.getMousePos() + Common::Point(14, 7);
			for (int idx = 0; idx < varA; ++idx) {
				if (READ_LE_UINT16(dataP + idx * 8 + 2) <= tempPos.x &&
					READ_LE_UINT16(dataP + idx * 8 + 6) >= tempPos.x &&
					READ_LE_UINT16(dataP + idx * 8 + 4) <= tempPos.y &&
					READ_LE_UINT16(dataP + idx * 8 + 4) <= tempPos.y) {
					// Found hotspot area
					foundIndex = idx;
					break;
				}
			}
			
			pt = _eventsManager.getMousePos();
			if (tempPos.x >= 68 && tempPos.x <= 277 && tempPos.y >= 31 && tempPos.y <= 154) {
				tempPos.y -= 2;
				foundIndex = (tempPos.y - 31) / 15;
				if ((tempPos.y - 31) % 15 >= 12 || (si + foundIndex) >= _voy._eventCount) {
					_eventsManager.setCursorColor(128, 0);
					foundIndex = 999;
				} else if (!_voy._mouseClicked) {
					_eventsManager.setCursorColor(128, 2);
					foundIndex = 999;
				} else {
					_eventsManager.setCursorColor(128, 2);
					var20 = foundIndex;

					(*_graphicsManager._vPort)->_flags |= 8;
					_graphicsManager.flipPage();
					_eventsManager.sWaitFlip();

					_graphicsManager._drawPtr->_penColor = 0;
					_graphicsManager._drawPtr->_pos = Common::Point(tempRect.left, tempRect.top);
					// TODO: Does sFillBox need to work on picture resources?
					((ViewPortResource *)_graphicsManager._backgroundPage)->sFillBox(tempRect.width(), tempRect.height());

					evtIndex = si;
					int yp = 45;
					
					for (int idx = 0; idx < 8 && evtIndex < _voy._eventCount; ++idx) {
						_graphicsManager._fontPtr->_picFlags = 0;
						_graphicsManager._fontPtr->_picSelect = 0xff;
						_graphicsManager._fontPtr->_picPick = 7;
						_graphicsManager._fontPtr->_picOnOff = (idx == var20) ? 8 : 0;
						_graphicsManager._fontPtr->_pos = Common::Point(68, yp);
						_graphicsManager._fontPtr->_justify = ALIGN_LEFT;
						_graphicsManager._fontPtr->_justifyWidth = 0;
						_graphicsManager._fontPtr->_justifyHeight = 0;

						Common::String msg = _eventsManager.getEvidString(evtIndex);
						// TODO: Does sFillBox need to work on picture resources?
						((ViewPortResource *)_graphicsManager._backgroundPage)->drawText(msg);

						yp += 115;
						++evtIndex;
					}

					(*_graphicsManager._vPort)->addSaveRect(
						(*_graphicsManager._vPort)->_lastPage, tempRect);
					(*_graphicsManager._vPort)->_flags |= 8;
					_graphicsManager.flipPage();
					_eventsManager.sWaitFlip();

					(*_graphicsManager._vPort)->addSaveRect(
						(*_graphicsManager._vPort)->_lastPage, tempRect);
					(*_graphicsManager._vPort)->_flags |= 8;
					_graphicsManager.flipPage();
					_eventsManager.sWaitFlip();

					_eventsManager.getMouseInfo();
					foundIndex = 999;
				}
			} else if ((_voy._field478 & 0x40) && _voy._viewBounds->left == pt.x &&
					_voy._viewBounds->bottom == pt.y) {
				foundIndex = 999;
			} else if ((_voy._field478 & 0x40) && _voy._viewBounds->left == pt.x &&
					_voy._viewBounds->top == pt.y) {
				foundIndex = 998;
			} else {
				_eventsManager.setCursorColor(128, (foundIndex == -1) ? 0 : 1);
			}

			_eventsManager._intPtr.field38 = true;
			_eventsManager._intPtr._hasPalette = true;

			if (_voy._incriminate || _voy._fadeICF1) {
				switch (foundIndex) {
				case 2:
					if (si > 0) {
						--si;
						var1E = true;
					}
					foundIndex = -1;
					break;

				case 3:
					if (si > 0) {
						si -= 8;
						if (si < 0)
							si = 0;
						var1E = true;
					}
					foundIndex = -1;
					break;

				case 4:
					if ((_voy._eventCount - 8) > si) {
						++si;
						var1E = true;
					}
					foundIndex = -1;
					break;

				case 5:
					if (_voy._eventCount >= 8 && (_voy._eventCount - 8) != si) {
						si += 8;
						if ((_voy._eventCount - 8) < si)
							si = _voy._eventCount - 8;
						var1E = true;
					}
					foundIndex = -1;
					break;

				default:
					break;
				}

				while (var20 > 0 && (var20 + si) >= _voy._eventCount)
					--var20;
			}

			pt = _eventsManager.getMousePos();
			if (_voy._incriminate && _voy._viewBounds->left == pt.x &&
					(_voy._field478 & 0x40) && _voy._fadeFunc) {
				WRITE_LE_UINT32(_controlPtr->_ptr + 4, (pt.y / 60) + 1);
				foundIndex = -1;
				_voy._fadeFunc = 0;
			}
			
			if (_voy._fadeFunc)
				foundIndex = 0;

		} while (!shouldQuit() && (!_voy._incriminate || foundIndex == -1));



		warning("TODO");
	}

	_graphicsManager._fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	_bVoy->freeBoltGroup(0x900);

	(*_graphicsManager._vPort)->fillPic(0);
	(*_graphicsManager._vPort)->_flags |= 8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();
}

void VoyeurEngine::doGossip() {
	_graphicsManager.resetPalette();
	_graphicsManager.screenReset();

	if (!_bVoy->getBoltGroup(0x300))
		return;

	PictureResource *pic = _bVoy->boltEntry(0x300)._picResource;
	(*_graphicsManager._vPort)->setupViewPort(pic);
	CMapResource *pal = _bVoy->boltEntry(0x301)._cMapResource;
	pal->startFade();

	flipPageAndWaitForFade();

	// Load the gossip animation
	::Video::RL2Decoder decoder;
	decoder.loadFile("a2050100.rl2");

	byte *frameNumsP = _bVoy->memberAddr(0x309);
	byte *posP = _bVoy->memberAddr(0x30A);

	// Main playback loop
	int picCtr = 0;
	decoder.start();
	while (!shouldQuit() && !decoder.endOfVideo() && !_voy._incriminate) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_graphicsManager.setPalette(palette, 0, 256);
		}
		
		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();

			Common::copy((const byte *)frame->getPixels(), (const byte *)frame->getPixels() + 320 * 200,
				(byte *)_graphicsManager._screenSurface.getPixels());

			if (decoder.getCurFrame() >= READ_LE_UINT16(frameNumsP + picCtr * 4)) {
				PictureResource *pic = _bVoy->boltEntry(0x302 + picCtr)._picResource;
				Common::Point pt(READ_LE_UINT16(posP + 4 * picCtr + 2), 
					READ_LE_UINT16(posP + 4 * picCtr));
				_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, pt);
			}

			flipPageAndWait();
		}

		_eventsManager.pollEvents();
		g_system->delayMillis(10);
	}

	decoder.loadFile("a2110100.rl2");
	decoder.start();

	while (!shouldQuit() && !decoder.endOfVideo() && !_voy._incriminate) {
		if (decoder.hasDirtyPalette()) {
			const byte *palette = decoder.getPalette();
			_graphicsManager.setPalette(palette, 0, 256);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();

			Common::copy((const byte *)frame->getPixels(), (const byte *)frame->getPixels() + 320 * 200,
				(byte *)_graphicsManager._screenSurface.getPixels());
		}

		_eventsManager.pollEvents();
		g_system->delayMillis(10);
	}

	_bVoy->freeBoltGroup(0x300);
	_graphicsManager.screenReset();
}

void VoyeurEngine::doTapePlaying() {
	if (!_bVoy->getBoltGroup(0xA00))
		return;

	_eventsManager.getMouseInfo();
	_graphicsManager._backColors = _bVoy->boltEntry(0xA01)._cMapResource;
	_graphicsManager._backgroundPage = _bVoy->boltEntry(0xA00)._picResource;
	PictureResource *pic = _bVoy->boltEntry(0xA02)._picResource;

	(*_graphicsManager._vPort)->setupViewPort(_graphicsManager._backgroundPage);
	_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(57, 30));
	flipPageAndWaitForFade();

	CycleResource *cycle = _bVoy->boltEntry(0xA05)._cycleResource;
	cycle->vStartCycle();

	_soundManager.startVOCPlay("vcr.voc");
	while (!shouldQuit() && !_voy._incriminate && _soundManager.getVOCStatus()) {
		_eventsManager.delay(2);
	}

	_soundManager.stopVOCPlay();
	_bVoy->freeBoltGroup(0xA00);
}

bool VoyeurEngine::checkForMurder() {
	int v = READ_LE_UINT32(_controlPtr->_ptr + 12);

	for (int idx = 0; idx < _voy._eventCount; ++idx) {
		VoyeurEvent &evt = _voy._events[idx];

		if (evt._type == EVTYPE_VIDEO) {
			switch (READ_LE_UINT32(_controlPtr->_ptr + 4)) {
			case 1:
				if (evt._field8 == 41 && evt._computerOn <= 15 &&
						(evt._computerOff + evt._computerOn) >= 16) {
					WRITE_LE_UINT32(_controlPtr->_ptr + 12, 1);
				}
				break;

			case 2:
				if (evt._field8 == 53 && evt._computerOn <= 19 &&
						(evt._computerOff + evt._computerOn) >= 21) {
					WRITE_LE_UINT32(_controlPtr->_ptr + 12, 2);
				}
				break;

			case 3:
				if (evt._field8 == 50 && evt._computerOn <= 28 &&
						(evt._computerOff + evt._computerOn) >= 29) {
					WRITE_LE_UINT32(_controlPtr->_ptr + 12, 3);
				}
				break;

			case 4:
				if (evt._field8 == 43 && evt._computerOn <= 10 &&
						(evt._computerOff + evt._computerOn) >= 14) {
					WRITE_LE_UINT32(_controlPtr->_ptr + 12, 4);
				}
				break;

			default:
				break;
			}
		}

		if (READ_LE_UINT32(_controlPtr->_ptr + 12) == READ_LE_UINT32(_controlPtr->_ptr + 4)) {
			_voy._videoEventId = idx;
			return true;
		}
	}

	WRITE_LE_UINT32(_controlPtr->_ptr + 12, v);
	_voy._videoEventId = -1;
	return false;
}

bool VoyeurEngine::checkForIncriminate() {
	_voy._field4382 = 0;

	for (int idx = 0; idx < _voy._eventCount; ++idx) {
		VoyeurEvent &evt = _voy._events[idx];
		
		if (evt._type == EVTYPE_VIDEO) {
			if (evt._field8 == 44 && evt._computerOn <= 40 &&
					(evt._computerOff + evt._computerOn) >= 70) {
				_voy._field4382 = 1;
			}

			if (evt._field8 == 44 && evt._computerOn <= 79 &&
					(evt._computerOff + evt._computerOn) >= 129) {
				_voy._field4382 = 1;
			}

			if (evt._field8 == 20 && evt._computerOn <= 28 &&
					(evt._computerOff + evt._computerOn) >= 45) {
				_voy._field4382 = 2;
			}

			if (evt._field8 == 35 && evt._computerOn <= 17 &&
					(evt._computerOff + evt._computerOn) >= 36) {
				_voy._field4382 = 3;
			}

			if (evt._field8 == 30 && evt._computerOn <= 80 &&
					(evt._computerOff + evt._computerOn) >= 139) {
				_voy._field4382 = 4;
			}
		}

		if (_voy._field4382) {
			WRITE_LE_UINT32(_controlPtr->_ptr + 12, 88);
			_voy._videoEventId = idx;
			return true;
		}
	}

	_voy._videoEventId = -1;
	return false;
}

void VoyeurEngine::playAVideoEvent(int eventIndex) {
	VoyeurEvent &evt = _voy._events[eventIndex];
	_eventsManager._videoComputerBut4 = evt._field8;
	_voy._vocSecondsOffset = evt._computerOn;
	_eventsManager._videoDead = evt._dead;
	_voy._field478 &= ~1;
	
	playAVideoDuration(_eventsManager._videoComputerBut4, evt._computerOff);
}

int VoyeurEngine::getChooseButton()  {
	int prevIndex = -2;
	Common::Array<Common::Rect> &hotspots = _bVoy->boltEntry(_playStamp1 
		+ 6)._rectResource->_entries;
	int selectedIndex = -1;

	(*_graphicsManager._vPort)->setupViewPort(_graphicsManager._backgroundPage);
	_graphicsManager._backColors->_steps = 0;
	_graphicsManager._backColors->startFade();
	flipPageAndWait();

	_voy._viewBounds = _bVoy->boltEntry(_playStamp1 + 7)._rectResource;
	PictureResource *cursorPic = _bVoy->boltEntry(_playStamp1 + 2)._picResource;

	do {
		do {
			if (_playStamp2 != -1 && !_soundManager.getVOCStatus())
				_soundManager.startVOCPlay(_playStamp2);

			_eventsManager.getMouseInfo();
			selectedIndex = -1;
			Common::Point pt = _eventsManager.getMousePos();
			
			for (uint idx = 0; idx < hotspots.size(); ++idx) {
				if (hotspots[idx].contains(pt)) {
					if (!_voy._field4F0 || (idx + 1) != READ_LE_UINT32(_controlPtr->_ptr + 4)) {
						selectedIndex = idx;
						if (selectedIndex != prevIndex) {
							PictureResource *btnPic = _bVoy->boltEntry(_playStamp1 + 8 + idx)._picResource;
							_graphicsManager.sDrawPic(btnPic, *_graphicsManager._vPort,
								Common::Point(106, 200));

							cursorPic = _bVoy->boltEntry(_playStamp1 + 4)._picResource;
						}
					}
				}
			}

			if (selectedIndex == -1) {
				cursorPic = _bVoy->boltEntry(_playStamp1 + 2)._picResource;
				PictureResource *btnPic = _bVoy->boltEntry(_playStamp1 + 12)._picResource;
				_graphicsManager.sDrawPic(btnPic, *_graphicsManager._vPort,
					Common::Point(106, 200));
			}

			_graphicsManager.sDrawPic(cursorPic, *_graphicsManager._vPort,
				Common::Point(pt.x + 13, pt.y - 12));

			flipPageAndWait();
		} while (!shouldQuit() && !_voy._incriminate);
	} while (!shouldQuit() && selectedIndex == -1 && !_voy._fadeFunc);

	return selectedIndex;
}

void VoyeurEngine::makeViewFinder() {
	_graphicsManager._backgroundPage = _bVoy->boltEntry(0x103)._picResource;
	_graphicsManager.sDrawPic(_graphicsManager._backgroundPage, 
		*_graphicsManager._vPort, Common::Point(0, 0));
	CMapResource *pal = _bVoy->boltEntry(0x104)._cMapResource;

	int palOffset = 0;
	switch (_voy._transitionId) {
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

	(*_graphicsManager._vPort)->drawIfaceTime();
	doTimeBar(true);
	pal->startFade();

	(*_graphicsManager._vPort)->_flags |= 8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
		_eventsManager.delay(1);

	_graphicsManager.setColor(241, 105, 105, 105);
	_graphicsManager.setColor(242, 105, 105, 105);
	_graphicsManager.setColor(243, 105, 105, 105);
	_graphicsManager.setColor(palOffset + 241, 219, 235, 235);

	_eventsManager._intPtr.field38 = 1;
	_eventsManager._intPtr._hasPalette = true;
}

void VoyeurEngine::makeViewFinderP() {
	_graphicsManager.screenReset();
}

void VoyeurEngine::initIFace(){
	int playStamp1 = _playStamp1;
	switch (_voy._transitionId) {
	case 0:
		break;
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		_playStamp1 = 0xB00;
		break;
	case 3:
		_playStamp1 = 0xC00;
		break;
	default:
		_playStamp1 = 0xD00;
		break;
	}
	if (playStamp1 != -1)
		_bVoy->freeBoltGroup(playStamp1, true);

	_bVoy->getBoltGroup(_playStamp1);
	CMapResource *pal = _bVoy->boltEntry(_playStamp1 + 2)._cMapResource;
	pal->startFade();

	doScroll(_eventsManager.getMousePos());
	
	_voy._viewBounds = _bVoy->boltEntry(_playStamp1)._rectResource;

	// Note: the original did two loops to preload members here, which is
	// redundant for ScummVM, since computers are faster these days, and
	// getting resources as needed will be fast enough.
}

void VoyeurEngine::doScroll(const Common::Point &pt) {
	Common::Rect clipRect(72, 47, 72 + 240, 47 + 148);
	(*_graphicsManager._vPort)->setupViewPort(NULL, &clipRect);

	PictureResource *pic;
	int base = 0;
	switch (_voy._transitionId) {
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
		pic = _bVoy->boltEntry(base + 3)._picResource;
 		_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(784 - pt.x - 712, 150 - pt.y - 104));
		pic = _bVoy->boltEntry(base + 4)._picResource;
		_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(784 - pt.x - 712, 150 - pt.y - 44));
		pic = _bVoy->boltEntry(base + 5)._picResource;
		_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 16));
		pic = _bVoy->boltEntry(base + 6)._picResource;
		_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 76));
		pic = _bVoy->boltEntry(base + 7)._picResource;
		_graphicsManager.sDrawPic(pic, *_graphicsManager._vPort, Common::Point(784 - pt.x - 712, 150 - pt.y + 136));
	}

	(*_graphicsManager._vPort)->setupViewPort();
}

void VoyeurEngine::checkTransition(){
	Common::String time, day;

	if (_voy._transitionId != _checkTransitionId) {
		switch (_voy._transitionId) {
		case 0:
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			day = SATURDAY;
			break;
		case 17: 
			day = MONDAY;
			break;
		default:
			day = SUNDAY;
			break;
		}

		if (!day.empty()) {
			_graphicsManager.fadeDownICF(6);

			if (_voy._transitionId != 17) {
				const char *amPm = _voy._isAM ? AM : PM;
				time = Common::String::format("%d:%02d%s",
					_gameHour, _gameMinute, amPm);
			}

			doTransitionCard(day, time);
			_eventsManager.delay(180);
		}

		_checkTransitionId = _voy._transitionId;
	}
}

bool VoyeurEngine::doComputerText(int maxLen) {
	FontInfoResource &font = *_graphicsManager._fontPtr;
	int totalChars = 0;

	font._curFont = _bVoy->boltEntry(0x4910)._fontResource;
	font._foreColor = 129;
	font._fontSaveBack = false;
	font._fontFlags = 0;
	if (_voy._vocSecondsOffset > 60)
		_voy._vocSecondsOffset = 0;

	if (_voy._RTVNum > _voy._field4EE && maxLen == 9999) {
		if (_playStamp2 != -1)
			_soundManager.startVOCPlay(_playStamp2);
		font._justify = ALIGN_LEFT;
		font._justifyWidth = 384;
		font._justifyHeight = 100;
		font._pos = Common::Point(128, 100);
		(*_graphicsManager._vPort)->drawText(END_OF_MESSAGE);
	} else if (_voy._RTVNum < _voy._field4EC && maxLen == 9999) {
		if (_playStamp2 != -1)
			_soundManager.startVOCPlay(_playStamp2);
		font._justify = ALIGN_LEFT;
		font._justifyWidth = 384;
		font._justifyHeight = 100;
		font._pos = Common::Point(120, 100);
		(*_graphicsManager._vPort)->drawText(START_OF_MESSAGE);
	} else {
		char *msg = (char *)_bVoy->memberAddr(0x4900 + _voy._computerTextId);
		font._pos = Common::Point(96, 60);

		bool showEnd = true;
		int yp = 60;
		do {
			if (_playStamp2 != -1 && !_soundManager.getVOCStatus()) {
				if (_voy._vocSecondsOffset > 60)
					_voy._vocSecondsOffset = 0;
				_soundManager.startVOCPlay(_playStamp2);
			}

			char c = *msg++;
			if (c == '\0') {
				if (showEnd) {
					_eventsManager.delay(90);
					_graphicsManager._drawPtr->_pos = Common::Point(54, 96);
					_graphicsManager._drawPtr->_penColor = 254;
					(*_graphicsManager._vPort)->sFillBox(196, 124);
					_graphicsManager._fontPtr->_justify = ALIGN_LEFT;
					_graphicsManager._fontPtr->_justifyWidth = 384;
					_graphicsManager._fontPtr->_justifyHeight = 100;
					_graphicsManager._fontPtr->_pos = Common::Point(128, 100);
					(*_graphicsManager._vPort)->drawText(END_OF_MESSAGE);
				}
				break;
			}

			if (c == '~' || c == '^') {
				if (c == '^') {
					yp += 10;
				} else {
					_eventsManager.delay(90);
					_graphicsManager._drawPtr->_pos = Common::Point(54, 96);
					_graphicsManager._drawPtr->_penColor = 255;
					(*_graphicsManager._vPort)->sFillBox(196, 124);
					yp = 60;
				}

				_graphicsManager._fontPtr->_pos = Common::Point(96, yp);
			} else if (c == '_') {
				showEnd = false;
			} else {
				_graphicsManager._fontPtr->_justify = ALIGN_LEFT;
				_graphicsManager._fontPtr->_justifyWidth = 0;
				_graphicsManager._fontPtr->_justifyHeight = 0;
				(*_graphicsManager._vPort)->drawText(Common::String(c));
				_eventsManager.delay(4);
			}

			(*_graphicsManager._vPort)->_flags |= 8;
			_graphicsManager.flipPage();
			_eventsManager.sWaitFlip();
			_eventsManager.getMouseInfo();
			++totalChars;

		} while (!shouldQuit() && !_voy._incriminate && totalChars < maxLen);

		_voy._field4EE = 0;
	}

	(*_graphicsManager._vPort)->_flags |= 8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	_graphicsManager._fontPtr->_curFont = _bVoy->boltEntry(0x101)._fontResource;
	return totalChars;
}

void VoyeurEngine::getComputerBrush() {
	error("TODO: getComputerBrush");
//	if (_bVoy->getBoltGroup(0x4900)) {
//	}
}

void VoyeurEngine::doTimeBar(bool force) {
	flashTimeBar();

	if ((force || _timeBarVal != _voy._RTVNum) && _voy._field476 > 0) {
		if (_voy._RTVNum > _voy._field476 || _voy._RTVNum < 0)
			_voy._RTVNum = _voy._field476 - 1;
		
		_timeBarVal = _voy._RTVNum;
		int height = ((_voy._field476 - _voy._RTVNum) * 59) / _voy._field476;
		int fullHeight = MAX(151 - height, 93);

		_graphicsManager._drawPtr->_penColor = 134;
		_graphicsManager._drawPtr->_pos = Common::Point(39, 92);

		(*_graphicsManager._vPort)->sFillBox(6, fullHeight - 92);
		if (height > 0) {
			_graphicsManager.setColor(215, 238, 238, 238);
			_eventsManager._intPtr.field38 = 1;
			_eventsManager._intPtr._hasPalette = true;

			_graphicsManager._drawPtr->_penColor = 215;
			_graphicsManager._drawPtr->_pos = Common::Point(39, fullHeight);
			(*_graphicsManager._vPort)->sFillBox(6, height);
		}
	}
}

void VoyeurEngine::flashTimeBar(){
	if (_voy._RTVNum >= 0 && (_voy._field476 - _voy._RTVNum) < 11 &&
		(_eventsManager._intPtr.field1A >= (_flashTimeVal + 15) ||
		_eventsManager._intPtr.field1A < _flashTimeVal)) {
		// Within time range
		_flashTimeVal = _eventsManager._intPtr.field1A;

		if (_flashTimeFlag)
			_graphicsManager.setColor(240, 220, 20, 20);
		else
			_graphicsManager.setColor(240, 220, 220, 220);
		
		_eventsManager._intPtr.field38 = 1;
		_eventsManager._intPtr._hasPalette = true;
		_flashTimeFlag = !_flashTimeFlag;
	}
}

void VoyeurEngine::checkPhoneCall() {
	if ((_voy._field476 - _voy._RTVNum) >= 36 && _voy._field4B8 < 5 && 
			_playStamp2 <= 151 && _playStamp2 > 146) {
		if ((_voy._switchBGNum < _checkPhoneVal || _checkPhoneVal > 180) &&
				!_soundManager.getVOCStatus()) {
			int soundIndex;
			do {
				soundIndex = getRandomNumber(4);
			} while (_voy._field4AE[soundIndex]);
			_playStamp2 = 154 + soundIndex;

			_soundManager.stopVOCPlay();
			_soundManager.startVOCPlay(_playStamp2);
			_checkPhoneVal = _voy._switchBGNum;
			++_voy._field4AE[soundIndex];
			++_voy._field4B8;
		}
	}
}

void VoyeurEngine::doEvidDisplay(int v1, int v2) {
	_eventsManager.getMouseInfo();
	(*_graphicsManager._vPort)->_flags |= 8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	if (_playStamp2 != -1) {
		_voy._vocSecondsOffset = _voy._RTVNum - _voy._field4AC;
		_soundManager.stopVOCPlay();
	}

	error("TODO: doEvidDisplay");
}

} // End of namespace Voyeur
