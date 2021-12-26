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

#ifndef SAGA2_MOUSEIMG_H
#define SAGA2_MOUSEIMG_H

namespace Saga2 {

enum {
	kMouseCloseBx1Image = 0,
	kMouseCloseBx2Image,
	kMouseUsePtrImage,
	kMouseXPointerImage,
	kMouseArrowImage,
	kMouseGrabPtrImage,
	kMouseAttakPtrImage,
	kMouseCenterActorIndicatorImage,
	kMousePgUpImage,
	kMousePgDownImage,
	kMousePgLeftImage,
	kMousePgRightImage,
	kMouseAutoWalkImage,
	kMouseGaugeImage,
	kMouseMax
};

extern gStaticImage *mouseCursors[kMouseMax];

//  Set a new image for the mouse pointer
void setMouseImage(int id, int16 x, int16 y);

//  Set a new image for the mouse pointer
void setMouseImage(gPixelMap &img, int16 x, int16 y);

//  Set new text for the mouse pointer
void setMouseText(char *text);

//  Set the gauge value for the mouse pointer (automatically turns
//  gauge on if necessary ).
void setMouseGauge(int numerator, int denominator);

//  Turn off the gauge on the mouse pointer
void clearMouseGauge();

void initCursors();
void freeCursors();

} // end of namespace Saga2

#endif
