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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SETUP_H
#define SAGA2_SETUP_H

namespace Saga2 {

struct WindowDecoration;

//If Defined Then It Will Print Status
#define         WriteStatus

//Is Combat In Game Or Not

#define     Combat

//Sets Up Ready Container View

#define     hasReadyContainers

//If No Ready Containers Then Following Const's not needed

// enum for the three levels in the trio view
enum trioViews {
	top,
	mid,
	bot
};

struct ContainerInfo {
	int16   xPos;
	int16   yPos;
	int8    rows;
	int8    cols;
};


// these consts are defined in objects.cpp

// trio ready container consts
extern const ContainerInfo  trioReadyContInfo[];

// indiv ready container consts
extern const ContainerInfo  indivReadyContInfoTop;
extern const ContainerInfo  indivReadyContInfoBot;

//			char   *fileName = "FTA.HRS";
//			char   *scriptsName = "SCRIPTS.HRS";
//			char   *soundsName = "FTASOUND.HRS";

//  List of decorations for main window

enum borderIDs {
	MWBottomBorder = 0,
	MWTopBorder,
	MWLeftBorder,
	MWRightBorder1,
	MWRightBorder2,
	MWRightBorder3
};

const int   extraObjects  = 512,
            extraActors   = 64;

} // end of namespace Saga2

#endif
