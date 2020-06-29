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

#ifndef GRAPHICS_MACGUI_MACSHAPE_H
#define GRAPHICS_MACGUI_MACSHAPE_H

#include "graphics/macgui/macwidget.h"

namespace Graphics {

enum MacShapeType {
	kShapeRectangle,
	kShapeRoundRect,
	kShapeOval,
	kShapeLine
};

class MacShape : public MacWidget {
public:
	MacShape(MacShapeType shapeType, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, uint fgcolor, uint bgcolor, uint thickness, uint fillType);

	void setStyle(MacShapeType shapeType, uint fgcolor, uint bgcolor, uint thickness, uint fillType);
	void render();

private:
	MacShapeType _shapeType;
	int _thickness;
	uint _fillType;
};

} // end of namespace Graphics

#endif
