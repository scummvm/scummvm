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

/* This is a simple colored area widget -- rectangular or round  */

#ifndef NUVIE_GUI_GUI_AREA_H
#define NUVIE_GUI_GUI_AREA_H

#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/screen/screen.h"

namespace Ultima {
namespace Nuvie {

#define AREA_ROUND 1
#define AREA_ANGULAR 2

class GUI_Area : public GUI_Widget {

public:
	/* Passed the area, color and shape */
	GUI_Area(int x, int y, int w, int h, uint8 r, uint8 g, uint8 b, int aShape = AREA_ANGULAR);

	/* Passed the area, color, frame color, frame thickness and shape */
	GUI_Area(int x, int y, int w, int h, uint8 r, uint8 g, uint8 b,
	         uint8 fr, uint8 fg, uint8 fb, int fthick, int aShape = AREA_ANGULAR);

	/* Map the color to the display */
	void SetDisplay(Screen *s) override;

	/* Show the widget  */
	void Display(bool full_redraw) override;


protected:
	uint8 R, G, B;
	uint32 color;

	/* flag */
	int useFrame;

	/* frame color values */
	uint8 fR, fG, fB;
	uint32 frameColor;

	/* remember me */
	int frameThickness;
	int shape;

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
