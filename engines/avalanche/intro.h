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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_INTRO_H
#define AVALANCHE_INTRO_H

namespace Avalanche {

class AvalancheEngine;

class Intro {
public:
	Intro(AvalancheEngine *vm);
	virtual ~Intro() {}

	void run();

private:
	void resetPlanes();
	void moveStars();
	void plotStar(uint8 plane, int x, int y);
	void plotStars(uint8 plane, int y);
	void combineAndDraw();

	AvalancheEngine *_vm;
	
	// Each plane is 320x200 (40 bytes per row)
	// Plane 0: Slow pace stars, Plane 1: Mid pace stars, Plane 2: Fast pace stars
	uint8 _planes[3][40*200];
	
	int _displayCounter;
};

} // End of namespace Avalanche

#endif // AVALANCHE_INTRO_H
