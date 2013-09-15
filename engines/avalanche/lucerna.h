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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* LUCERNA		The screen, [keyboard] and mouse handler.*/

#ifndef AVALANCHE_LUCERNA2_H
#define AVALANCHE_LUCERNA2_H

#include "common/scummsys.h"
#include "common/file.h"

namespace Avalanche {
class AvalancheEngine;

class Clock {
public:
	Clock(AvalancheEngine *vm);

	void update();
	
private:
	static const int kCenterX = 510;
	static const int kCenterY = 183;

	AvalancheEngine *_vm;

	uint16 _hour, _minute, _second, _hourAngle, _oldHour, _oldMinute, _oldHourAngle;
	Common::Point _clockHandHour, _clockHandMinute;

	void calcHand(uint16 angle, uint16 length, Common::Point &endPoint, byte color);
	void drawHand(const Common::Point &endPoint, byte color);
	void plotHands();
	void chime();
};

class Lucerna {
public:
	bool _holdLeftMouse;
	Clock _clock;
	
	Lucerna(AvalancheEngine *vm);
	~Lucerna();

	void init();
	void callVerb(byte id);
	void drawAlsoLines();
	void loadRoom(byte num);
	void exitRoom(byte x);
	void enterRoom(byte room, byte ped);
	void thinkAbout(byte object, bool type); // Hey!!! Get it and put it!!!
	void loadDigits(); // Load the scoring digits & rwlites
	void drawToolbar();
	void drawScore();
	void incScore(byte num); // Add on no. of points
	void useCompass(const Common::Point &cursorPos); // Click on the compass on the toolbar to control Avvy's movement.
	void fxToggle();
	void refreshObjectList();
	void checkClick();
	void errorLed();
	void dusk();
	void dawn();
	void drawDirection(); // Draws the little icon at the left end of the text input field.
	void gameOver();
	uint16 bearing(byte whichPed); // Returns the bearing from ped 'whichped' to Avvy, in degrees.
	void fixFlashers();
	void loadAlso(byte num);

	// There are two kinds of redraw: Major and Minor. Minor is what happens when you load a game, etc. Major redraws EVERYTHING.
	void minorRedraw();
	void majorRedraw();

	void spriteRun();

private:
	AvalancheEngine *_vm;

	Common::File file;

	Common::String readAlsoStringFromFile();
	void scram(Common::String &str);
	void unScramble();

	void zoomOut(int16 x, int16 y); // Only used when entering the map.
	void enterNewTown();
	void findPeople(byte room);
	void putGeidaAt(byte whichPed, byte ped);
	void guideAvvy(Common::Point cursorPos);

	// Will be used in dusk() and dawn().
	bool _fxHidden;

	int8 fades(int8 x);
	void fadeOut(byte n);
	void fadeIn(byte n);

};

} // End of namespace Avalanche

#endif // AVALANCHE_LUCERNA2_H
