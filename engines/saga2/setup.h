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


//Sets Up Tile Map Area

const int   tileRectX           =   16 + 4,
            tileRectY           =   16 + 4,
            tileRectWidth       =   448 - 8,
            tileRectHeight      =   428 - 8;

//Sets Up Filenames based on PROJECT defined in make.

#define IMAGE_RESFILE   "FTAIMAGE.HRS"
#define OBJECT_RESFILE  "FTA.HRS"
#define AUX_RESFILE     "FTADATA.HRS"
#define SCRIPT_RESFILE  "SCRIPTS.HRS"
#define SOUND_RESFILE   "FTASOUND.HRS"
#define VOICE_RESFILE   "FTAVOICE.HRS"

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
	MWRightBorder3,
};

extern WindowDecoration mainWindowDecorations[];

const int   extraObjects  = 512,
            extraActors   = 64;

} // end of namespace Saga2

#endif
