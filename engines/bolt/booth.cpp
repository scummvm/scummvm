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
	_xp->displayPic(&g_boothLetterSprite, g_displayX, g_displayY, page);
	displayColors(g_boothPalCycleData, page, 0);

	if (page != stFront)
		_xp->fillDisplay(0, stFront);

	_xp->updateDisplay();
}

void BoltEngine::playAVOverBooth(int16 animIndex) {
	_xp->hideCursor();
	displayBooth(stBack);
	playAV(g_rtfHandle, animIndex, g_displayWidth, g_displayHeight, g_displayX, g_displayY);
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
		return 10; // play game
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
		return 11;
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
		return 15;
	default:
		return 15;
	}
}

int16 BoltEngine::mainEntrance(int16 prevBooth) {
	int16 result = openBooth(9);
	closeBooth();

	switch (result) {
	case -1:
		return 0; // exit
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
		return 0; // exit
	default:
		return 9;
	}
}

bool BoltEngine::loadBooth(int16 boothId) {
	switch (boothId) {
	case 3:
		if (g_boothLoadedMask & 0x01)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x100, 1))
			return false;
		g_boothLoadedMask |= 0x01;
		break;

	case 4:
		if (g_boothLoadedMask & 0x02)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x200, 1))
			return false;
		g_boothLoadedMask |= 0x02;
		break;

	case 5:
		if (g_boothLoadedMask & 0x04)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x300, 1))
			return false;
		g_boothLoadedMask |= 0x04;
		break;

	case 6:
		if (g_boothLoadedMask & 0x08)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x400, 1))
			return false;
		g_boothLoadedMask |= 0x08;
		break;

	case 7:
		if (g_boothLoadedMask & 0x10)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x500, 1))
			return false;
		g_boothLoadedMask |= 0x10;
		break;

	case 8:
		if (g_boothLoadedMask & 0x20)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x600, 1))
			return false;
		g_boothLoadedMask |= 0x20;
		break;

	case 9:
		if (g_boothLoadedMask & 0x40)
			break;
		if (!getBOLTGroup(g_boothsBoltLib, 0x700, 1))
			return false;
		if (!getBOLTGroup(g_boothsBoltLib, 0x800 + (g_lettersWon << 8), 1))
			return false;
		g_boothLoadedMask |= 0x40;
		break;
	}

	return true;
}

void BoltEngine::unloadBooth() {
	if (g_boothLoadedMask & 0x01)
		freeBOLTGroup(g_boothsBoltLib, 0x100, 1);

	if (g_boothLoadedMask & 0x02)
		freeBOLTGroup(g_boothsBoltLib, 0x200, 1);

	if (g_boothLoadedMask & 0x04)
		freeBOLTGroup(g_boothsBoltLib, 0x300, 1);

	if (g_boothLoadedMask & 0x08)
		freeBOLTGroup(g_boothsBoltLib, 0x400, 1);

	if (g_boothLoadedMask & 0x10)
		freeBOLTGroup(g_boothsBoltLib, 0x500, 1);

	if (g_boothLoadedMask & 0x20)
		freeBOLTGroup(g_boothsBoltLib, 0x600, 1);

	if (g_boothLoadedMask & 0x40) {
		freeBOLTGroup(g_boothsBoltLib, 0x800 + (g_lettersWon << 8), 1);
		freeBOLTGroup(g_boothsBoltLib, 0x700, 1);
	}

	g_boothLoadedMask = 0;
}

int16 BoltEngine::openBooth(int16 sceneId) {
	int16 baseResId;
	int16 resId;

	g_currentBoothScene = sceneId;

	if (!loadBooth(sceneId))
		return -1;

	baseResId = (sceneId - 3) << 8;

	// Main entrance has more hotspots/animations than regular booths
	if (sceneId == 9) {
		g_boothNumHotspots = 8;
		g_boothNumAnimations = 7;
	} else {
		g_boothNumHotspots = 5;
		g_boothNumAnimations = 4;
	}

	// Background scene descriptor
	g_boothSceneDesc = memberAddr(g_boothsBoltLib, baseResId + 0x100);

	// Load 4 palette descriptors (at DS:0x13AC, members +0x105..+0x108)
	resId = baseResId + 0x105;
	for (int16 i = 0; i < 4; i++) {
		g_boothHotPalDescs[i] = memberAddr(g_boothsBoltLib, resId);
		resId++;
	}

	// Load animation descriptors
	if (sceneId == 9) {
		for (int16 i = 0; i < g_boothNumAnimations; i++)
			g_boothAnimDescs[i] = memberAddr(g_boothsBoltLib, 0x709 + i);
	} else {
		resId = baseResId + 0x109;
		for (int16 i = 0; i < g_boothNumAnimations; i++) {
			g_boothAnimDescs[i] = memberAddr(g_boothsBoltLib, resId);
			resId++;
		}
	}

	// Load hotspot descriptors
	if (sceneId == 9) {
		for (int16 i = 0; i < g_boothNumHotspots; i++)
			g_boothHotspotDescs[i] = memberToRect(memberAddr(g_boothsBoltLib, 0x710 + i));
	} else {
		resId = baseResId + 0x10D;
		for (int16 i = 0; i < g_boothNumHotspots; i++) {
			g_boothHotspotDescs[i] = memberToRect(memberAddr(g_boothsBoltLib, resId));
			resId++;
		}
	}

	if (sceneId == 9) {
		// Main entrance palette cycling data
		g_boothPalCycleData = memberAddr(g_boothsBoltLib, 0x718);

		// Letter marquee sprite (display mode selects variant)
		byte *sprite = memberAddr(g_boothsBoltLib, (g_displayMode != 0 ? 0x801 : 0x800) + (g_lettersWon << 8));
		boltPict2Pict(&g_boothLetterSprite, sprite);

		// Exit sign sprites
		g_boothExitLeft = memberAddr(g_boothsBoltLib, 0x747);
		g_boothExitRight = memberAddr(g_boothsBoltLib, 0x748);
	} else {
		g_boothPalCycleData = memberAddr(g_boothsBoltLib, baseResId + 0x112);

		byte *sprite = memberAddr(g_boothsBoltLib, baseResId + (g_displayMode != 0 ? 0x114 : 0x113));
		boltPict2Pict(&g_boothLetterSprite, sprite);
	}

	displayBooth(stFront);
	loadColors();
	_xp->setCursorColor(255, 255, 255);

	// First-time cursor position from resource data
	if (g_needInitCursorPos) {
		g_cursorX = (int16)READ_UINT16(memberAddr(g_boothsBoltLib, 1));
		g_cursorY = (int16)READ_UINT16(memberAddr(g_boothsBoltLib, 2));
		g_needInitCursorPos = false;
	}

	_xp->setCursorPos(g_cursorX, g_cursorY);

	flushInput();
	return boothEventLoop();
}

void BoltEngine::closeBooth() {
	uint16 buttonState;

	_xp->readCursor(&buttonState, &g_cursorY, &g_cursorX);
	flushInput();
	_xp->fillDisplay(0, 0);
	unloadBooth();
}

void BoltEngine::playTour() {
	int16 playing = 1;

	_xp->hideCursor();
	g_screensaverStep = 0;

	if (prepareAV(g_rtfHandle, 4, g_displayWidth, g_displayHeight, g_displayX, g_displayY)) {
		while (playing) {
			playing = maintainAV(0);
			if (!playing)
				break;

			uint32 eventData;
			int16 event = _xp->getEvent(etEmpty, &eventData);

			switch (event) {
			case etMouseDown: // Abort tour
				stopAV();
				playing = 0;
				break;

			case etSound: // Sound marker
				playing = maintainAV(1);
				break;

			case etTrigger: // Trigger marker from RTF stream
				screensaverStep();
				break;
			}
		}

		_xp->stopCycle();
		displayBooth(stFront);
	}

	_xp->showCursor();
}

void BoltEngine::finishPlayingHelp(int16 activeHotspot) {
	g_helpPlaying = 0;

	if (g_helpIsIdle != 0) {
		g_helpIsIdle = 0;
	} else {
		resetInactivityState();
	}

	if (activeHotspot != 0)
		restoreColors(0);

	if (activeHotspot != 1)
		restoreColors(1);

	if (activeHotspot != 2 && g_keyHelp == 0)
		restoreColors(2);

	g_keyLeft = 0;

	if (g_currentBoothScene == 9) {
		if (activeHotspot != 7)
			restoreColors(7);

		if (activeHotspot != 5)
			restoreColors(5);

		if (activeHotspot != 6)
			restoreColors(6);

		g_keyRight = 0;
		g_keyUp = 0;
		g_keyDown = 0;
	} else {
		if (activeHotspot != 3)
			restoreColors(3);

		g_keyEnter = 0;
	}
}

int16 BoltEngine::hotSpotActive(int16 hotspot) {
	switch (hotspot) {
	case 0:
	case 1:
		return g_keyLeft;

	case 2:
		return (g_helpPlaying || g_keyHelp) ? 1 : 0;

	case 3:
		return g_keyEnter;

	case 4:
		return 0;

	case 5:
		return g_keyDown;

	case 6:
		return g_keyUp;

	case 7:
		return g_keyRight;

	default:
		return 0;
	}
}

void BoltEngine::hoverHotSpot() {
	int16 i = 0;

	while (i < g_boothNumHotspots) {
		if (g_boothHotspotDescs[i].contains(g_cursorX, g_cursorY))
			break;

		i++;
	}

	if (i == g_hoveredHotspot)
		return;

	if (g_hoveredHotspot < g_boothNumHotspots) {
		if (!hotSpotActive(g_hoveredHotspot))
			restoreColors(g_hoveredHotspot);
	}

	g_hoveredHotspot = i;

	if (i < g_boothNumHotspots) {
		if (!hotSpotActive(i))
			setColors(g_hoveredHotspot);
	}
}

int16 BoltEngine::boothEventLoop() {
	uint32 eventData;
	int16 exitCode = 0;

	_xp->showCursor();
	g_hoveredHotspot = -1;
	hoverHotSpot();

	while (exitCode == 0 && !shouldQuit()) {
		_xp->setInactivityTimer(30);

		if (g_helpPlaying != 0) {
			g_helpPlaying = maintainAudioPlay(0);
			if (g_helpPlaying == 0)
				finishPlayingHelp(g_hoveredHotspot);
		}

		eventData = 0;
		switch (_xp->getEvent(etEmpty, &eventData)) {
		case etTimer:
			if (g_keyLeft == 0 && g_keyRight == 0 &&
				g_keyUp == 0 && g_keyDown == 0 &&
				g_keyEnter == 0 && g_keyHelp == 0)
				break;

			if (g_keyReleased != 0) {
				// Key release: restore hotspot colors
				g_keyReleased = 0;
				if (g_keyLeft != 0) {
					restoreColors(0);
					restoreColors(1);
				} else if (g_keyRight != 0) {
					restoreColors(7);
				} else if (g_keyUp != 0) {
					restoreColors(6);
				} else if (g_keyDown != 0) {
					restoreColors(5);
				} else if (g_keyEnter != 0) {
					restoreColors(3);
				} else if (g_keyHelp != 0) {
					restoreColors(2);

					if (g_helpPlaying != 0) {
						if (g_helpIsIdle == 0 || g_currentBoothScene != 9)
							break;
					}

					restoreColors(8);
				}
			} else {
				// Key press: highlight hotspot colors
				g_keyReleased = 1;
				if (g_keyLeft != 0) {
					setColors(0);
					setColors(1);
				} else if (g_keyRight != 0) {
					setColors(7);
				} else if (g_keyUp != 0) {
					setColors(6);
				} else if (g_keyDown != 0) {
					setColors(5);
				} else if (g_keyEnter != 0) {
					setColors(3);
				} else if (g_keyHelp != 0) {
					setColors(2);

					if (g_helpPlaying != 0) {
						if (g_helpIsIdle == 0 || g_currentBoothScene != 9)
							break;
					}

					setColors(8);
				}
			}

			g_keyTimer = _xp->startTimer(500);
			break;

		case etMouseMove:
			g_cursorX = (int16)(eventData >> 16);
			g_cursorY = (int16)eventData;
			hoverHotSpot();
			break;

		case etMouseDown:
		{
			int16 prevHelp = g_helpPlaying;
			if (g_helpPlaying != 0) {
				stopAnimation();
				finishPlayingHelp(g_hoveredHotspot);
			}

			if (prevHelp != 0 && g_hoveredHotspot == 2)
				break;

			if (g_hoveredHotspot == -1)
				break;

			exitCode = handleButtonPress(g_hoveredHotspot);
			if (exitCode == 0) {
				g_hoveredHotspot = -1;
				hoverHotSpot();
			}

			break;
		}

		case etSound:
			if (g_helpPlaying != 0) {
				g_helpPlaying = maintainAudioPlay(1);
				if (g_helpPlaying == 0)
					finishPlayingHelp(g_hoveredHotspot);
			}

			break;

		case etInactivity:
			if (g_helpPlaying != 0)
				break;

			if (g_keyHelp == 0) {
				g_keyHelp = 1;
				g_keyTimer = _xp->startTimer(500);
				if (g_idleHelpAvailable != 0)
					continue;
			} else {
				if (g_idleHelpAvailable == 0)
					break;

				if (startAnimation(g_rtfHandle, 0x19)) {
					g_idleHelpAvailable = 0;
					g_helpPlaying = 1;
					g_helpIsIdle = 1;
				}
			}

			break;

		case etTrigger:
			if (g_helpPlaying == 0)
				break;

			if (g_currentBoothScene == 9)
				mapIdleAnimation();
			else
				boothIdleAnimation();

			break;
		}
	}

	if (shouldQuit()) {
		return -1;
	}

	_xp->hideCursor();
	return g_hoveredHotspot;
}

void BoltEngine::resetInactivityState() {
	if (g_keyTimer != 0) {
		_xp->killTimer(g_keyTimer);
		g_keyTimer = 0;
	}

	g_idleHelpAvailable = 0;
	_xp->setInactivityTimer(30);

	if (g_keyHelp != 0) {
		restoreColors(2);
		if (g_currentBoothScene == 9)
			restoreColors(8);
	}

	g_keyHelp = 0;
}

bool BoltEngine::handleButtonPress(int16 hotspot) {
	byte savedLeftDoor[3];
	byte savedRightDoor[3];

	savedLeftDoor[0] = g_leftDoorNavTable[0];
	savedLeftDoor[1] = g_leftDoorNavTable[1];
	savedLeftDoor[2] = g_leftDoorNavTable[2];
	savedRightDoor[0] = g_rightDoorNavTable[0];
	savedRightDoor[1] = g_rightDoorNavTable[1];
	savedRightDoor[2] = g_rightDoorNavTable[2];

	if (hotspot != -1)
		resetInactivityState();

	switch (hotspot) {
	case 0:
	case 1:
		return true;

	case 2: {
		int16 animId;
		switch (g_currentBoothScene) {
		case 3:
			animId = 0x18;
			break;
		case 4:
			animId = 0x12;
			break;
		case 5:
			animId = 0x13;
			break;
		case 6:
			animId = 0x14;
			break;
		case 7:
			animId = 0x15;
			break;
		case 8:
			animId = 0x16;
			break;
		case 9:
			animId = 0x17;
			break;
		default:
			animId = -1;
			break;
		}

		bool ok = false;
		if (animId != -1)
			ok = startAnimation(g_rtfHandle, animId);

		if (ok) {
			g_helpPlaying = 1;
			setColors(2);
			g_keyHelp = 0;
			g_screensaverStep = 0;
		}

		return false;
	}

	default:
		if (g_currentBoothScene == 9) {
			switch (hotspot) {
			case 3:
			case 4:
			{
				byte *navTable = (hotspot == 3) ? savedLeftDoor : savedRightDoor;
				int8 boothOffset = (int8)navTable[g_lettersWon % 3];
				playAVOverBooth(boothOffset + 12);
				return false;
			}

			case 5:
				_xp->hideCursor();
				displayBooth(stBack);

				if (playAV(g_rtfHandle, 3, g_displayWidth, g_displayHeight, g_displayX, g_displayY) == 0) {
					fadeToBlack(1);
					_xp->setTransparency(false);
					displayPic(getBOLTMember(g_boothsBoltLib, g_displayMode ? 0x1802 : 0x1801), g_displayX, g_displayY, 0);
					_xp->updateDisplay();
					displayColors(getBOLTMember(g_boothsBoltLib, 0x1800), stFront, 0);
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

				displayBooth(stBack);
				playAV(g_rtfHandle, 5,
					   g_displayWidth, g_displayHeight,
					   g_displayX, g_displayY);

				if (g_lettersWon != 0) {
					freeBOLTGroup(g_boothsBoltLib, 0x800 + (g_lettersWon << 8), 1);
					if (!getBOLTGroup(g_boothsBoltLib, 0x800, 1)) {
						g_hoveredHotspot = -1;
						return true;
					}

					boltPict2Pict(&g_boothLetterSprite, memberAddr(g_boothsBoltLib, g_displayMode ? 0x801 : 0x800));
				}

				displayBooth(stFront);
				g_lettersWon = 0;
				_xp->showCursor();
				return false;

			case 7:
				playTour();
				return false;
			}
		} else {
			switch (hotspot) {
			case 3:
				startCycle(g_boothPalCycleData);
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

	if (mode == 1)
		_xp->setPalette(count, startIndex, localPalette);
}

void BoltEngine::setColors(int16 index) {
	if (g_currentBoothScene == 9) {
		switch (index) {
		case 0:
		case 1:
		case 2:
			blastColors(g_boothAnimDescs, index, 1);
			break;

		case 3:
		case 4:
			return;

		case 5:
		case 6:
		case 7:
			blastColors(g_boothAnimDescs, index - 2, 1);
			break;

		case 8:
			displayPic(g_boothExitLeft, 0, 0, 0);
			_xp->updateDisplay();
			break;
		}
	} else {
		switch (index) {
		case 0:
		case 1:
		case 2:
			blastColors(g_boothAnimDescs, index, 1);
			break;

		case 3:
			if (g_currentBoothScene == 9)
				break;

			for (int16 i = 0; i < 4; i++)
				blastColors(g_boothHotPalDescs, i, 1);

			break;

		case 4:
			blastColors(g_boothAnimDescs, index - 1, 1);
			break;
		}
	}
}

void BoltEngine::restoreColors(int16 index) {
	if (g_currentBoothScene == 9) {
		switch (index) {
		case 0:
		case 1:
		case 2: {
			byte *palData = g_savedPalettes[index];
			byte *animDesc = g_boothAnimDescs[index];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 3:
		case 4:
			return;

		case 5:
		case 6:
		case 7: {
			byte *palData = g_savedPalettes[index - 2];
			byte *animDesc = g_boothAnimDescs[index - 2];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 8:
			displayPic(g_boothExitRight, 0, 0, 0);
			_xp->updateDisplay();
			break;
		}
	} else {
		switch (index) {
		case 0:
		case 1:
		case 2: {
			byte *palData = g_savedPalettes[index];
			byte *animDesc = g_boothAnimDescs[index];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}

		case 3:
			for (int16 i = 0; i < 4; i++) {
				byte *palData = g_savedHotPalettes[i];
				byte *palDesc = g_boothHotPalDescs[i];
				_xp->setPalette((int16)READ_UINT16(palDesc + 2), (int16)READ_UINT16(palDesc), palData);
			}
			break;

		case 4:
		{
			byte *palData = g_savedPalettes[index - 1];
			byte *animDesc = g_boothAnimDescs[index - 1];
			_xp->setPalette((int16)READ_UINT16(animDesc + 2), (int16)READ_UINT16(animDesc), palData);
			break;
		}
		}
	}
}

void BoltEngine::loadColors() {
	for (int16 i = 0; i < g_boothNumAnimations; i++) {
		byte *animDesc = g_boothAnimDescs[i];
		_xp->getPalette((int16)READ_UINT16(animDesc), (int16)READ_UINT16(animDesc + 2), g_savedPalettes[i]);
	}

	if (g_currentBoothScene == 9) {
		int16 last = g_boothNumAnimations - 1;
		byte *animDesc = g_boothAnimDescs[last];
		_xp->getPalette((int16)READ_UINT16(animDesc), (int16)READ_UINT16(animDesc + 2), g_savedPalettes[last]);
	}

	for (int16 i = 0; i < 4; i++) {
		byte *palDesc = g_boothHotPalDescs[i];
		_xp->getPalette((int16)READ_UINT16(palDesc), (int16)READ_UINT16(palDesc + 2), g_savedHotPalettes[i]);
	}
}

void BoltEngine::shiftColorMap(byte *colorMap, int16 delta) {
	WRITE_BE_INT16(colorMap + 2, READ_BE_INT16(colorMap + 2) + delta);
	WRITE_BE_INT16(colorMap + 4, READ_BE_INT16(colorMap + 4) + delta);
}

void BoltEngine::playBoothAV() {
	switch (g_currentBoothScene) {
	case 3:
		playAVOverBooth(6);
		break;
	case 4:
		playAVOverBooth(7);
		break;
	case 5:
		playAVOverBooth(8);
		break;
	case 6:
		playAVOverBooth(9);
		break;
	case 7:
		playAVOverBooth(10);
		break;
	case 8:
		playAVOverBooth(11);
		break;
	}
}

void BoltEngine::mapIdleAnimation() {
	g_screensaverStep++;

	switch (g_screensaverStep) {
	case 1:
		setColors(0);
		setColors(1);
		g_keyLeft = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 2:
		g_keyLeft = 0;
		g_keyReleased = 0;

		if (!g_boothHotspotDescs[0].contains(g_cursorX, g_cursorY))
			restoreColors(0);

		if (!g_boothHotspotDescs[1].contains(g_cursorX, g_cursorY))
			restoreColors(1);

		return;

	case 3:
		setColors(7);
		g_keyRight = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 4:
		g_keyRight = 0;
		g_keyReleased = 0;

		if (!g_boothHotspotDescs[7] .contains(g_cursorX, g_cursorY))
			restoreColors(7);
		return;

	case 5:
		setColors(6);
		g_keyUp = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 6:
		g_keyUp = 0;
		g_keyReleased = 0;

		if (!g_boothHotspotDescs[6].contains(g_cursorX, g_cursorY))
			restoreColors(6);
		return;

	case 7:
		setColors(5);
		g_keyDown = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 8:
		g_keyDown = 0;
		g_keyReleased = 0;
		g_keyTimer = 0;

		if (!g_boothHotspotDescs[5].contains(g_cursorX, g_cursorY))
			restoreColors(5);
		return;
	}
}

void BoltEngine::boothIdleAnimation() {
	g_screensaverStep++;

	switch (g_screensaverStep) {
	case 1:
		setColors(3);
		g_keyEnter = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 2:
		g_keyEnter = 0;
		g_keyReleased = 0;

		if (!g_boothHotspotDescs[3].contains(g_cursorX, g_cursorY))
			restoreColors(3);

		return;

	case 3:
		setColors(0);
		setColors(1);
		g_keyLeft = 1;
		g_keyReleased = 1;
		g_keyTimer = _xp->startTimer(500);
		break;

	case 4:
		g_keyLeft = 0;
		g_keyReleased = 0;
		g_keyTimer = 0;

		if (!g_boothHotspotDescs[0].contains(g_cursorX, g_cursorY))
			restoreColors(0);

		if (!g_boothHotspotDescs[1].contains(g_cursorX, g_cursorY))
			restoreColors(1);

		return;
	}
}

void BoltEngine::screensaverStep() {
	static const int16 cycleResIds[] = {
		0x700, 0x719, 0x71E, 0x720, 0x725, 0x72A, 0x72F, 0x731,
		-1,    0x736, 0x73B, 0x73F, 0x743, -1
	};

	g_screensaverStep++;

	if (!(g_screensaverStep & 1)) {
		_xp->stopCycle();
		return;
	}

	int16 index = g_screensaverStep >> 1;
	if (index > 13)
		return;

	int16 resId = cycleResIds[index];
	if (resId == -1)
		return;

	startCycle(memberAddr(g_boothsBoltLib, resId));
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
	g_lettersWon++;

	if (g_lettersWon == 15) {
		if (g_allLettersWonFlag) {
			g_allLettersWonFlag = false;
			g_lettersWon = 16;
		} else {
			g_allLettersWonFlag = true;
		}
	}

	playAV(g_rtfHandle, g_lettersWon + 31, g_displayWidth, g_displayHeight, g_displayX, g_displayY);

	if (g_lettersWon >= 15)
		g_lettersWon = 0;

	// Return the booth front ID for the game just played
	switch (prevBooth) {
	case 10:
		return 3; // HuckGame    -> HucksBooth
	case 11:
		return 4; // FredGame    -> FredsBooth
	case 12:
		return 5; // ScoobyGame  -> ScoobysBooth
	case 13:
		return 6; // YogiGame    -> YogisBooth
	case 14:
		return 7; // GeorgeGame  -> GeorgesBooth
	case 15:
		return 8; // TopCatGame  -> TopCatsBooth
	default:
		return 9; // MainEntrance
	}
}

} // End of namespace Bolt
