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

#ifndef SAGA2_GBEVEL_H
#define SAGA2_GBEVEL_H

namespace Saga2 {

/* ===================================================================== *
                              Standard Pens
 * ===================================================================== */

//  Defines all the standard drawing pens for bevels

enum bevelPens {

	transpPen = 0,                          // transparent pen for mouse

	//  color pens

	blackPen,                               // black
	whitePen,                               // white

	//  Background pens

	backPen,                                // background fill
	backLtPen,                              // background highlight
	backDkPen,                              // background shadow

	//  Button pens

	buttonPen,                              // button fill color
	buttonLtPen,                            // button highlight
	buttonDkPen,                            // button shadow

	//  UI highlighting pens

	hilitePen,                              // text highlight color

	cStdPenCount
};

//  Pen array definition

extern gPen         mainPens[];         // the array of pens

#if 0
#define transpPen   mainPens[0]       // transparent pen for mouse

//  color pens

#define blackPen    mainPens[1]       // black
#define whitePen    mainPens[2]       // white

//  Background pens

#define backPen     mainPens[3]       // background fill
#define backLtPen   mainPens[4]       // background highlight
#define backDkPen   mainPens[5]       // background shadow

//  Button pens

#define buttonPen   mainPens[6]       // button fill color
#define buttonLtPen mainPens[7]       // button highlight
#define buttonDkPen mainPens[8]       // button shadow

#define transpPen   mainPens[0]       // transparent pen for mouse

//  color pens

#define blackPen    mainPens[1]       // black
#define whitePen    mainPens[2]       // white

//  Background pens

#define backPen     mainPens[3]       // background fill
#define backLtPen   mainPens[4]       // background highlight
#define backDkPen   mainPens[5]       // background shadow

//  Button pens

#define buttonPen   mainPens[6]       // button fill color
#define buttonLtPen mainPens[7]       // button highlight
#define buttonDkPen mainPens[8]       // button shadow

//  Text pens

#define hilitePen   mainPens[9]       // text highlight color
#endif

/* ===================================================================== *
                              Bevel Boxes
 * ===================================================================== */

enum bevelOptions {
	bevelRecessed   = (1 << 0),         // bevel is pushed in
	bevelFilled     = (1 << 1),         // bevel filled with approp. color
	bevelInverted   = (1 << 2)          // pushed in w/slightly diff. look
};

void DrawBevelFrame(gPort &port, const Rect16 &r, uint16 flags);
void DrawOutlineFrame(gPort &port, const Rect16 &r, int16 fillColor);
void DrawTitleBar(gPort &port, const Rect16 &extent, int16 selected, char *title);
void DrawTextFrame(gPort &port, const Rect16 &r);
void ThinBevelFrame(gPort &port, const Rect16 &r, gPen ul, gPen lr, gPen corner);
void DrawWell(gPort &port, const Rect16 &r, gPen fillColor, uint16 flags);

} // end of namespace Saga2

#endif
