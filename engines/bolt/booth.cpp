/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/bolt.h"

namespace Bolt {

void BoltEngine::startCycle(byte *cycleResource) {
	if (!cycleResource)
		return;

	XPCycleState specs[4];
	boltCycleToXPCycle(cycleResource, specs);
	_xp->startCycle(specs);
}

void BoltEngine::displayBooth(int16 page) {
	_xp->setTransparency(page);
	_xp->displayPic(&_boothLetterSprite, _displayX, _displayY, page);
	displayColors(_boothPalCycleData, page, 0);

	if (page != stFront)
		_xp->fillDisplay(0, stFront);

	_xp->updateDisplay();
}

void BoltEngine::playAVOverBooth(int16 animIndex) {
	_xp->hideCursor();
	displayBooth(stBack);
	playAV(_rtfHandle, animIndex, _displayWidth, _displayHeight, _displayX, _displayY);
	displayBooth(stFront);
	_xp->showCursor();
}

int16 BoltEngine::hucksBooth(int16 prevBooth) {
	int16 result = openBooth(3);
	closeBooth();

	switch (result) {
	case -1:
		return 0; // exit
	case 0:
		return 9; // main entrance
	case 1:
		return 4; // right neighbor (Fred)
	case 2:
		return 10; // default (play game)
	case 3:
		return _isDemo ? 17 : 10; // play game
	default:
		return 10;
	}
}

int16 BoltEngine::fredsBooth(int16 prevBooth) {
	int16 result = openBooth(4);
	closeBooth();

	switch (result) {
	case -1:
		return 0;
	case 0:
		return 3; // left neighbor (Hucks)
	case 1:
		return 5; // right neighbor (Scooby)
	case 2:
		return 11;
	case 3:
		return _isDemo ? 17 : 11;
	default:
		return 11;
	}
}

int16 BoltEngine::scoobysBooth(int16 prevBooth) {
	int16 result = openBooth(5);
	closeBooth();

	switch (result) {
	case -1:
		return 0;
	case 0:
		return 4; // left (Fred)
	case 1:
		return 6; // right (Yogi)
	case 2:
		return 12;
	case 3:
		if (_isDemo && _scoobyWins >= 2)
			return 17;

		return 12;
	default:
		return 12;
	}
}

int16 BoltEngine::yogisBooth(int16 prevBooth) {
	int16 result = openBooth(6);
	closeBooth();

	switch (result) {
	case -1:
		return 0;
	case 0:
		return 5; // left (Scooby)
	case 1:
		return 7; // right (George)
	case 2:
		return 13;
	case 3:
		if (_isDemo && _yogiWins >= 2)
			return 17;

		return 13;
	default:
		return 13;
	}
}

int16 BoltEngine::georgesBooth(int16 prevBooth) {
	int16 result = openBooth(7);
	closeBooth();

	switch (result) {
	case -1:
		return 0;
	case 0:
		return 6; // left (Yogi)
	case 1:
		return 8; // right (TopCat)
	case 2:
		return 14;
	case 3:
		if (_isDemo && _georgeWins >= 2)
			return 17;

		return 14;
	default:
		return 14;
	}
}

int16 BoltEngine::topCatsBooth(int16 prevBooth) {
	int16 result = openBooth(8);
	closeBooth();

	switch (result) {
	case -1:
		return 0;
	case 0:
		return 7; // left (George)
	case 1:
		return 9; // right (main entrance)
	case 2:
		return 15;
	case 3:
		return _isDemo ? 17 : 15;
	default:
		return 15;
	}
}

int16 BoltEngine::mainEntrance(int16 prevBooth) {
	int16 result = openBooth(9);
	closeBooth();

	switch (result) {
	case -1:
		return _isDemo ? 18 : 0; // exit
	case 0:
		return 8; // left (TopCat)
	case 1:
		return 3; // right (Hucks)
	case 2:
		return 9; // self (stay)
	case 3:
		return 9; // self
	case 4:
		return 9; // self
	case 5:
		return _isDemo ? 18 : 0; // exit
	case 6:
		return _isDemo ? 18 : 9; // Case only available in the demo
	default:
		return 9;
	}
}

bool BoltEngine::loadBooth(int16 boothId) {
	switch (boothId) {
	case 3:
		if (_boothLoadedMask & 0x01)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x100, 1))
			return false;
		_boothLoadedMask |= 0x01;
		break;

	case 4:
		if (_boothLoadedMask & 0x02)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x200, 1))
			return false;
		_boothLoadedMask |= 0x02;
		break;

	case 5:
		if (_boothLoadedMask & 0x04)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x300, 1))
			return false;
		_boothLoadedMask |= 0x04;
		break;

	case 6:
		if (_boothLoadedMask & 0x08)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x400, 1))
			return false;
		_boothLoadedMask |= 0x08;
		break;

	case 7:
		if (_boothLoadedMask & 0x10)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x500, 1))
			return false;
		_boothLoadedMask |= 0x10;
		break;

	case 8:
		if (_boothLoadedMask & 0x20)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x600, 1))
			return false;
		_boothLoadedMask |= 0x20;
		break;

	case 9:
		if (_boothLoadedMask & 0x40)
			break;
		if (!getBOLTGroup(_boothsBoltLib, 0x700, 1))
			return false;
		if (!getBOLTGroup(_boothsBoltLib, 0x800 + (_lettersWon << 8), 1))
			return false;
		_boothLoadedMask |= 0x40;
		break;
	}

	return true;
}

void BoltEngine::unloadBooth() {
	if (_boothLoadedMask & 0x01)
		freeBOLTGroup(_boothsBoltLib, 0x100, 1);

	if (_boothLoadedMask & 0x02)
		freeBOLTGroup(_boothsBoltLib, 0x200, 1);

	if (_boothLoadedMask & 0x04)
		freeBOLTGroup(_boothsBoltLib, 0x300, 1);

	if (_boothLoadedMask & 0x08)
		freeBOLTGroup(_boothsBoltLib, 0x400, 1);

	if (_boothLoadedMask & 0x10)
		freeBOLTGroup(_boothsBoltLib, 0x500, 1);

	if (_boothLoadedMask & 0x20)
		freeBOLTGroup(_boothsBoltLib, 0x600, 1);

	if (_boothLoadedMask & 0x40) {
		freeBOLTGroup(_boothsBoltLib, 0x800 + (_lettersWon << 8), 1);
		freeBOLTGroup(_boothsBoltLib, 0x700, 1);
	}

	_boothLoadedMask = 0;
}

int16 BoltEngine::openBooth(int16 boothId) {
	int16 baseResId;
	int16 resId;

	_currentBoothScene = boothId;

	if (!loadBooth(boothId))
		return -1;

	baseResId = (boothId - 3) << 8;

	// Main entrance has more hotspots/animations than regular booths
	if (boothId == 9) {
		_boothNumHotspots = 8;
		_boothNumAnimations = 7;
	} else {
		_boothNumHotspots = 5;
		_boothNumAnimations = 4;
	}

	// Background scene descriptor
	_boothSceneDesc = memberAddr(_boothsBoltLib, baseResId + 0x100);

	// Load 4 palette descriptors (at DS:0x13AC, members +0x105..+0x108)
	resId = baseResId + 0x105;
	for (int16 i = 0; i < 4; i++) {
		_boothHotPalDescs[i] = memberAddr(_boothsBoltLib, resId);
		resId++;
	}

	// Load animation descriptors
	if (boothId == 9) {
		for (int16 i = 0; i < _boothNumAnimations; i++)
			_boothAnimDescs[i] = memberAddr(_boothsBoltLib, 0x709 + i);
	} else {
		resId = baseResId + 0x109;
		for (int16 i = 0; i < _boothNumAnimations; i++) {
			_boothAnimDescs[i] = memberAddr(_boothsBoltLib, resId);
			resId++;
		}
	}

	// Load hotspot descriptors
	if (boothId == 9) {
		for (int16 i = 0; i < _boothNumHotspots; i++)
			_boothHotspotDescs[i] = memberToRect(memberAddr(_boothsBoltLib, 0x710 + i));
	} else {
		resId = baseResId + 0x10D;
		for (int16 i = 0; i < _boothNumHotspots; i++) {
			_boothHotspotDescs[i] = memberToRect(memberAddr(_boothsBoltLib, resId));
			resId++;
		}
	}

	if (boothId == 9) {
		// Main entrance palette cycling data
		_boothPalCycleData = memberAddr(_boothsBoltLib, 0x718);

		// Letter marquee sprite (display mode selects variant)
		byte *sprite = memberAddr(_boothsBoltLib, (_displayMode != 0 ? 0x801 : 0x800) + (_lettersWon << 8));
		boltPict2Pict(&_boothLetterSprite, sprite);

		// Visit sign sprite and its off variant
		_boothVisitSignOn = memberAddr(_boothsBoltLib, 0x747);
		_boothVisitSignOff = memberAddr(_boothsBoltLib, 0x748);
	} else {
		_boothPalCycleData = memberAddr(_boothsBoltLib, baseResId + 0x112);

		byte *sprite = memberAddr(_boothsBoltLib, baseResId + (_displayMode != 0 ? 0x114 : 0x113));
		boltPict2Pict(&_boothLetterSprite, sprite);
	}

	displayBooth(stFront);
	loadColors();
	_xp->setCursorColor(255, 255, 255);

	// First-time cursor position from resource data
	if (_needInitCursorPos) {
		_cursorX = (int16)READ_UINT16(memberAddr(_boothsBoltLib, 1));
		_cursorY = (int16)READ_UINT16(memberAddr(_boothsBoltLib, 2));
		_needInitCursorPos = false;
	}

	_xp->setCursorPos(_cursorX, _cursorY);

	flushInput();

	if (_isDemo && boothId == 9) {
		_helpFlag = 1;
		_helpTimer = _xp->startTimer(500);
	}

	return boothEventLoop();
}

void BoltEngine::closeBooth() {
	uint16 buttonState;

	_xp->readCursor(&buttonState, &_cursorY, &_cursorX);
	flushInput();
	_xp->fillDisplay(0, 0);
	unloadBooth();
}

void BoltEngine::playTour() {
	int16 playing = 1;

	_xp->hideCursor();
	_tourStep = 0;

	if (prepareAV(_rtfHandle, 4 - (_isDemo ? 3 : 0), _displayWidth, _displayHeight, _displayX, _displayY)) {
		while (playing && !shouldQuit()) {
			playing = maintainAV(0);
			if (!playing)
				break;

			uint32 eventData;
			int16 event = _xp->getEvent(etEmpty, &eventData);

			switch (event) {
			case etMouseDown:
				stopAV();
				playing = 0;
				break;

			case etSound:
				playing = maintainAV(1);
				break;

			case etTrigger:
				tourPaletteCycleStep();
				break;
			}
		}

		if (shouldQuit())
			stopAV();

		_xp->stopCycle();
		displayBooth(stFront);
	}

	_xp->showCursor();
}

void BoltEngine::finishPlayingHelp(int16 activeHotspot) {
	_helpPlaying = 0;

	if (_helpIsIdle != 0) {
		_helpIsIdle = 0;
	} else {
		resetInactivityState();
	}

	if (activeHotspot != 0)
		restoreColors(0);

	if (activeHotspot != 1)
		restoreColors(1);

	if (activeHotspot != 2 && _helpFlag == 0)
		restoreColors(2);

	_keyLeft = 0;

	if (_currentBoothScene == 9) {
		if (activeHotspot != 7)
			restoreColors(7);

		if (activeHotspot != 5)
			restoreColors(5);

		if (activeHotspot != 6)
			restoreColors(6);

		_keyRight = 0;
		_keyUp = 0;
		_keyDown = 0;
	} else {
		if (activeHotspot != 3)
			restoreColors(3);

		_keyEnter = 0;
	}
}

int16 BoltEngine::hotSpotActive(int16 hotspot) {
	switch (hotspot) {
	case 0:
	case 1:
		return _keyLeft;

	case 2:
		return (_helpPlaying || _helpFlag) ? 1 : 0;

	case 3:
		return _keyEnter;

	case 4:
		return 0;

	case 5:
		return _keyDown;

	case 6:
		return _keyUp;

	case 7:
		return _keyRight;

	default:
		return 0;
	}
}

void BoltEngine::hoverHotSpot() {
	int16 i = 0;

	while (i < _boothNumHotspots) {
		if (_boothHotspotDescs[i].contains(_cursorX, _cursorY))
			break;

		i++;
	}

	if (i == _hoveredHotspot)
		return;

	if (_hoveredHotspot < _boothNumHotspots) {
		if (!hotSpotActive(_hoveredHotspot))
			restoreColors(_hoveredHotspot);
	}

	_hoveredHotspot = i;

	if (i < _boothNumHotspots) {
		if (!hotSpotActive(i))
			setColors(_hoveredHotspot);
	}
}

int16 BoltEngine::boothEventLoop() {
	uint32 eventData;
	int16 exitCode = 0;

	_xp->showCursor();
	_hoveredHotspot = -1;
	hoverHotSpot();

	_xp->setInactivityTimer(30);

	while (exitCode == 0 && !shouldQuit()) {
		if (_helpPlaying != 0) {
			_helpPlaying = maintainAudioPlay(0);
			if (_helpPlaying == 0)
				finishPlayingHelp(_hoveredHotspot);
		}

		eventData = 0;
		switch (_xp->getEvent(etEmpty, &eventData)) {
		case etTimer:
			if (_keyLeft == 0 && _keyRight == 0 &&
				_keyUp == 0 && _keyDown == 0 &&
				_keyEnter == 0 && _helpFlag == 0)
				break;

			if (_keyReleased != 0) {
				// Key release: restore hotspot colors
				_keyReleased = 0;
				if (_keyLeft != 0) {
					restoreColors(0);
					restoreColors(1);
				} else if (_keyRight != 0) {
					restoreColors(7);
				} else if (_keyUp != 0) {
					restoreColors(6);
				} else if (_keyDown != 0) {
					restoreColors(5);
				} else if (_keyEnter != 0) {
					restoreColors(3);
				} else if (_helpFlag != 0) {
					restoreColors(2);

					if (_helpPlaying != 0) {
						if (_helpIsIdle == 0 || _currentBoothScene != 9)
							break;
					}

					restoreColors(8);
				}
			} else {
				// Key press: highlight hotspot colors
				_keyReleased = 1;
				if (_keyLeft != 0) {
					setColors(0);
					setColors(1);
				} else if (_keyRight != 0) {
					setColors(7);
				} else if (_keyUp != 0) {
					setColors(6);
				} else if (_keyDown != 0) {
					setColors(5);
				} else if (_keyEnter != 0) {
					setColors(3);
				} else if (_helpFlag != 0) {
					setColors(2);

					if (_helpPlaying != 0) {
						if (_helpIsIdle == 0 || _currentBoothScene != 9)
							break;
					}

					setColors(8);
				}
			}

			_helpTimer = _xp->startTimer(500);
			break;

		case etMouseMove:
			_cursorX = (int16)(eventData >> 16);
			_cursorY = (int16)eventData;
			hoverHotSpot();
			break;

		case etMouseDown:
		{
			int16 prevHelp = _helpPlaying;
			if (_helpPlaying != 0) {
				stopAnimation();
				finishPlayingHelp(_hoveredHotspot);
			}

			if (prevHelp != 0 && _hoveredHotspot == 2)
				break;

			if (_hoveredHotspot == -1)
				break;

			exitCode = handleButtonPress(_hoveredHotspot);
			if (exitCode == 0) {
				_hoveredHotspot = -1;
				hoverHotSpot();
			}

			break;
		}

		case etSound:
			if (_helpPlaying != 0) {
				_helpPlaying = maintainAudioPlay(1);
				if (_helpPlaying == 0)
					finishPlayingHelp(_hoveredHotspot);
			}

			break;

		case etInactivity:
			if (_helpPlaying != 0)
				break;

			if (_helpFlag == 0) {
				_helpFlag = 1;
				_helpTimer = _xp->startTimer(500);
				if (_idleHelpAudioAvailable != 0) {
					_xp->setInactivityTimer(30);
					continue;
				}
			} else {
				if (_idleHelpAudioAvailable == 0)
					break;

				if (startAnimation(_rtfHandle, _isDemo ? 0x16 : 0x19)) {
					_idleHelpAudioAvailable = 0;
					_helpPlaying = 1;
					_helpIsIdle = 1;
				}
			}

			break;

		case etTrigger:
			if (_helpPlaying == 0)
				break;

			if (_currentBoothScene == 9)
				mainEntranceHelpBlink();
			else
				boothHelpBlink();

			break;
		}
	}

	if (shouldQuit()) {
		return -1;
	}

	_xp->hideCursor();
	return _hoveredHotspot;
}

void BoltEngine::resetInactivityState() {
	if (_helpTimer != 0) {
		_xp->killTimer(_helpTimer);
		_helpTimer = 0;
	}

	_idleHelpAudioAvailable = 0;
	_xp->setInactivityTimer(30);

	if (_helpFlag != 0) {
		restoreColors(2);
		if (_currentBoothScene == 9)
			restoreColors(8);
	}

	_helpFlag = 0;
}

bool BoltEngine::handleButtonPress(int16 hotspot) {
	byte savedLeftDoor[3];
	byte savedRightDoor[3];

	savedLeftDoor[0] = _leftDoorNavTable[0];
	savedLeftDoor[1] = _leftDoorNavTable[1];
	savedLeftDoor[2] = _leftDoorNavTable[2];
	savedRightDoor[0] = _rightDoorNavTable[0];
	savedRightDoor[1] = _rightDoorNavTable[1];
	savedRightDoor[2] = _rightDoorNavTable[2];

	if (hotspot != -1)
		resetInactivityState();

	switch (hotspot) {
	case 0:
	case 1:
		return true;

	case 2: {
		int16 animId;
		switch (_currentBoothScene) {
		case 3:
			animId = _isDemo ? 0x0F : 0x12;
			break;
		case 4:
			animId = _isDemo ? 0x10 : 0x13;
			break;
		case 5:
			animId = _isDemo ? 0x11 : 0x14;
			break;
		case 6:
			animId = _isDemo ? 0x12 : 0x15;
			break;
		case 7:
			animId = _isDemo ? 0x13 : 0x16;
			break;
		case 8:
			animId = _isDemo ? 0x14 : 0x17;
			break;
		case 9:
			animId = _isDemo ? 0x15 : 0x18;
			break;
		default:
			animId = -1;
			break;
		}

		bool ok = false;
		if (animId != -1)
			ok = startAnimation(_rtfHandle, animId);

		if (ok) {
			_helpPlaying = 1;
			setColors(2);
			_helpFlag = 0;
			_tourStep = 0;
		}

		return false;
	}

	default:
		if (_currentBoothScene == 9) {
			switch (hotspot) {
			case 3:
			case 4:
			{
				byte *navTable = (hotspot == 3) ? savedLeftDoor : savedRightDoor;
				int8 boothOffset = (int8)navTable[_lettersWon % 3];
				playAVOverBooth(boothOffset + (_isDemo ? 9 : 12));
				return false;
			}

			case 5:
				if (_isDemo) {
					return true;
				}

				_xp->hideCursor();
				displayBooth(stBack);

				if (playAV(_rtfHandle, 3, _displayWidth, _displayHeight, _displayX, _displayY) == 0) {
					fadeToBlack(1);
					_xp->setTransparency(false);
					displayPic(getBOLTMember(_boothsBoltLib, _displayMode ? 0x1802 : 0x1801), _displayX, _displayY, 0);
					_xp->updateDisplay();
					displayColors(getBOLTMember(_boothsBoltLib, 0x1800), stFront, 0);
					_xp->updateDisplay();

					uint32 timer = _xp->startTimer(5000);
					if (timer != 0) {
						uint32 evtData;
						int16 evtType;
						do {
							evtType = _xp->getEvent(etEmpty, &evtData);
						} while ((evtType != etTimer || evtData != timer) && !shouldQuit());
					}

					fadeToBlack(32);
				}
				return true;

			case 6:
				_xp->hideCursor();

				vDelete("FredBC");
				vDelete("GeorgeBE");
				vDelete("Huck");
				vDelete("Scooby");
				vDelete("TopCatBF");
				vDelete("Yogi");

				_huckWins = 0;
				_fredWins = 0;
				_scoobyWins = 0;
				_yogiWins = 0;
				_georgeWins = 0;
				_topCatWins = 0;

				displayBooth(stBack);
				playAV(_rtfHandle, 5 - (_isDemo ? 3 : 0),
					   _displayWidth, _displayHeight,
					   _displayX, _displayY);

				if (_lettersWon != 0) {
					freeBOLTGroup(_boothsBoltLib, 0x800 + (_lettersWon << 8), 1);
					if (!getBOLTGroup(_boothsBoltLib, 0x800, 1)) {
						_hoveredHotspot = -1;
						return true;
					}

					boltPict2Pict(&_boothLetterSprite, memberAddr(_boothsBoltLib, _displayMode ? 0x801 : 0x800));
				}

				displayBooth(stFront);
				_lettersWon = 0;
				_xp->showCursor();
				return false;

			case 7:
				playTour();
				return false;
			}
		} else {
			switch (hotspot) {
			case 3:
				startCycle(_boothSceneDesc);
				_xp->startTimer(2000);
				return true;

			case 4:
				restoreColors(hotspot);
				playBoothAV();
				return false;
			}
		}

		return false;
	}
}

void BoltEngine::blastColors(byte **paletteTable, int16 index, int16 mode) {
	byte localPalette[384];
	int16 si = 0;

	byte *entry = paletteTable[index];
	int16 startIndex = (int16)READ_UINT16(entry);
	int16 count = (int16)READ_UINT16(entry + 2);

	for (int16 i = 0; i < count; i++) {
		int16 off = i * 6;
		localPalette[si] = entry[4 + off];
		localPalette[si + 1] = entry[6 + off];
		localPalette[si + 2] = entry[8 + off];
		si += 3;
	}

	if (mode == 1) {
		_xp->setPalette(count, startIndex, localPalette);
	}
}

void BoltEngine::setColors(int16 index) {
	if (_currentBoothScene == 9) {
		switch (index) {
		case 0:
		case 1:
		case 2:
			blastColors(_boothAnimDescs, index, 1);
			break;

		case 3:
		case 4:
			return;

		case 5:
		case 6:
		case 7:
			blastColors(_boothAnimDescs, index - 2, 1);
			break;

		case 8:
			displayPic(_boothVisitSignOn, 0, 0, stFront);
			_xp->updateDisplay();
			break;
		}
	} else {
		switch (index) {
		case 0:
		case 1:
		case 2:
			blastColors(_boothAnimDescs, index, 1);
			break;

		case 3:
			if (_currentBoothScene == 9)
				break;

			for (int16 i = 0; i < 4; i++)
				blastColors(_boothHotPalDescs, i, 1);

			break;

		case 4:
			blastColors(_boothAnimDescs, index - 1, 1);
			break;
		}
	}
}

void BoltEngine::restoreColors(int16 index) {
	if (_currentBoothScene == 9) {
		switch (index) {
		case 0:
		case 1:
		case 2: {
			byte *palData = _savedPalettes[index];
			byte *animDesc = _boothAnimDescs[index];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 3:
		case 4:
			return;

		case 5:
		case 6:
		case 7: {
			byte *palData = _savedPalettes[index - 2];
			byte *animDesc = _boothAnimDescs[index - 2];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 8:
			displayPic(_boothVisitSignOff, 0, 0, stFront);
			_xp->updateDisplay();
			break;
		}
	} else {
		switch (index) {
		case 0:
		case 1:
		case 2: {
			byte *palData = _savedPalettes[index];
			byte *animDesc = _boothAnimDescs[index];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 3:
			for (int16 i = 0; i < 4; i++) {
				byte *palData = _savedHotPalettes[i];
				byte *palDesc = _boothHotPalDescs[i];
				_xp->setPalette((int16)READ_UINT16(palDesc + 2), (int16)READ_UINT16(palDesc), palData);
			}
			break;

		case 4:
		{
			byte *palData = _savedPalettes[index - 1];
			byte *animDesc = _boothAnimDescs[index - 1];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}
		}
	}
}

void BoltEngine::loadColors() {
	for (int16 i = 0; i < _boothNumAnimations; i++) {
		byte *animDesc = _boothAnimDescs[i];
		_xp->getPalette((int16)READ_UINT16(animDesc), (int16)READ_UINT16(animDesc + 2), _savedPalettes[i]);
	}

	if (_currentBoothScene == 9) {
		int16 last = _boothNumAnimations - 1;
		byte *animDesc = _boothAnimDescs[last];
		_xp->getPalette((int16)READ_UINT16(animDesc), (int16)READ_UINT16(animDesc + 2), _savedPalettes[last]);
	}

	for (int16 i = 0; i < 4; i++) {
		byte *palDesc = _boothHotPalDescs[i];
		_xp->getPalette((int16)READ_UINT16(palDesc), (int16)READ_UINT16(palDesc + 2), _savedHotPalettes[i]);
	}
}

void BoltEngine::shiftColorMap(byte *colorMap, int16 delta) {
	WRITE_BE_INT16(colorMap + 2, READ_BE_INT16(colorMap + 2) + delta);
	WRITE_BE_INT16(colorMap + 4, READ_BE_INT16(colorMap + 4) + delta);
}

void BoltEngine::playBoothAV() {
	switch (_currentBoothScene) {
	case 3:
		playAVOverBooth(6 - (_isDemo ? 3 : 0));
		break;
	case 4:
		playAVOverBooth(7 - (_isDemo ? 3 : 0));
		break;
	case 5:
		playAVOverBooth(8 - (_isDemo ? 3 : 0));
		break;
	case 6:
		playAVOverBooth(9 - (_isDemo ? 3 : 0));
		break;
	case 7:
		playAVOverBooth(10 - (_isDemo ? 3 : 0));
		break;
	case 8:
		playAVOverBooth(11 - (_isDemo ? 3 : 0));
		break;
	}
}

void BoltEngine::mainEntranceHelpBlink() {
	_tourStep++;

	switch (_tourStep) {
	case 1:
		setColors(0);
		setColors(1);
		_keyLeft = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 2:
		_keyLeft = 0;
		_keyReleased = 0;

		if (!_boothHotspotDescs[0].contains(_cursorX, _cursorY))
			restoreColors(0);

		if (!_boothHotspotDescs[1].contains(_cursorX, _cursorY))
			restoreColors(1);

		return;

	case 3:
		setColors(7);
		_keyRight = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 4:
		_keyRight = 0;
		_keyReleased = 0;

		if (!_boothHotspotDescs[7] .contains(_cursorX, _cursorY))
			restoreColors(7);
		return;

	case 5:
		setColors(6);
		_keyUp = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 6:
		_keyUp = 0;
		_keyReleased = 0;

		if (!_boothHotspotDescs[6].contains(_cursorX, _cursorY))
			restoreColors(6);
		return;

	case 7:
		setColors(5);
		_keyDown = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 8:
		_keyDown = 0;
		_keyReleased = 0;
		_helpTimer = 0;

		if (!_boothHotspotDescs[5].contains(_cursorX, _cursorY))
			restoreColors(5);
		return;
	}
}

void BoltEngine::boothHelpBlink() {
	_tourStep++;

	switch (_tourStep) {
	case 1:
		setColors(3);
		_keyEnter = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 2:
		_keyEnter = 0;
		_keyReleased = 0;

		if (!_boothHotspotDescs[3].contains(_cursorX, _cursorY))
			restoreColors(3);

		return;

	case 3:
		setColors(0);
		setColors(1);
		_keyLeft = 1;
		_keyReleased = 1;
		_helpTimer = _xp->startTimer(500);
		break;

	case 4:
		_keyLeft = 0;
		_keyReleased = 0;
		_helpTimer = 0;

		if (!_boothHotspotDescs[0].contains(_cursorX, _cursorY))
			restoreColors(0);

		if (!_boothHotspotDescs[1].contains(_cursorX, _cursorY))
			restoreColors(1);

		return;
	}
}

void BoltEngine::tourPaletteCycleStep() {
	static const int16 cycleResIds[] = {
		0x700, 0x719, 0x71E, 0x720, 0x725, 0x72A, 0x72F, 0x731,
		-1,    0x736, 0x73B, 0x73F, 0x743, -1
	};

	_tourStep++;

	if ((_tourStep & 1) == 0) {
		_xp->stopCycle();
		return;
	}

	int16 index = _tourStep >> 1;
	if (index > 13)
		return;

	int16 resId = cycleResIds[index];
	if (resId == -1)
		return;

	startCycle(memberAddr(_boothsBoltLib, resId));
}

void BoltEngine::fadeToBlack(int16 steps) {
	byte palette[768];
	XPPicDesc desc;

	desc.paletteStart = 1;
	desc.paletteCount = 0x7F;

	_xp->getPalette(0, 256, palette);
	_xp->setFrameRate(32);

	int16 scale = steps;
	int16 factor = steps - 1;

	while (scale > 0) {
		byte *ptr = palette;
		for (int16 i = 0; i < 256; i++) {
			ptr[0] = (byte)((uint16)ptr[0] * factor / scale);
			ptr[1] = (byte)((uint16)ptr[1] * factor / scale);
			ptr[2] = (byte)((uint16)ptr[2] * factor / scale);
			ptr += 3;
		}

		desc.palette = &palette[3];
		_xp->displayPic(&desc, 0, 0, stFront);

		desc.palette = &palette[384];
		_xp->displayPic(&desc, 0, 0, stBack);

		_xp->updateDisplay();
		factor--;
		scale--;
	}

	_xp->setFrameRate(0);
}

void BoltEngine::flushInput() {
	uint32 eventBuf;

	// Drain all mouse-down events
	while (_xp->getEvent(etMouseDown, &eventBuf) == etMouseDown);

	// Drain all mouse-up events
	while (_xp->getEvent(etMouseUp, &eventBuf) == etMouseUp);
}

int16 BoltEngine::winALetter(int16 prevBooth) {
	_lettersWon++;

	if (_isDemo && _lettersWon == 6)
		return 18;

	if (_lettersWon == 15) {
		if (_allLettersWonFlag) {
			_allLettersWonFlag = false;
			_lettersWon = 16;
		} else {
			_allLettersWonFlag = true;
		}
	}

	int16 avIndex = _isDemo ? 25 : 31;
	playAV(_rtfHandle, _lettersWon + avIndex, _displayWidth, _displayHeight, _displayX, _displayY);

	if (_lettersWon >= 15)
		_lettersWon = 0;

	// Return the booth front ID for the game just played
	switch (prevBooth) {
	case 10:
		if (_isDemo) {
			_huckWins++;
		}

		return 3; // HuckGame    -> HucksBooth
	case 11:
		if (_isDemo) {
			_fredWins++;
		}

		return 4; // FredGame    -> FredsBooth
	case 12:
		if (_isDemo) {
			_scoobyWins++;
		}

		return 5; // ScoobyGame  -> ScoobysBooth
	case 13:
		if (_isDemo) {
			_yogiWins++;
		}

		return 6; // YogiGame    -> YogisBooth
	case 14:
		if (_isDemo) {
			_georgeWins++;
		}

		return 7; // GeorgeGame  -> GeorgesBooth
	case 15:
		if (_isDemo) {
			_topCatWins++;
		}

		return 8; // TopCatGame  -> TopCatsBooth
	default:
		return 9; // MainEntrance
	}
}

int16 BoltEngine::endDemo(int16 prevBooth) {
	_xp->hideCursor();
	_xp->setTransparency(false);

	int16 baseId = (_displayMode != 0) ? 0x0801 : 0x0800;
	int16 memberId = baseId + (_lettersWon << 8);
	displayPic(getBOLTMember(_boothsBoltLib, memberId), _displayX, _displayY, stFront);
	displayColors(getBOLTMember(_boothsBoltLib, 0x718), stFront, 0);

	_xp->updateDisplay();

	fadeToBlack(32);

	return 0;
}

} // End of namespace Bolt
