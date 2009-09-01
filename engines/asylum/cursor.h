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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_CURSOR_H_
#define ASYLUM_CURSOR_H_

#include "asylum/sceneres.h"

namespace Asylum {

struct CommonResources;

class Cursor {
public:
	Cursor(ResourcePack *res);
	virtual ~Cursor();

	void show();
	void hide();
	/**
	 * Load a GraphicResource at the position specified by
	 * index from the _resPack ResourcePack
	 */
	void load(uint32 index);
	void set(byte *data, byte width, byte height);
	void set(int frame);
	void setCoords(uint32 mouseX, uint32 mouseY);
	void update();
	void update(CommonResources *cr, int currentAction);
	void animate();

	uint32 x() { return _mouseX; }
	uint32 y() { return _mouseY; }
	uint32 currentFrame() { return _curFrame; }

private:
	ResourcePack	*_resPack;
	GraphicResource *_cursorResource;
	bool   cursorLoaded;
	uint32 _curFrame;
	int32  _cursorStep;
	uint32 _mouseX;
	uint32 _mouseY;

}; // end of class Cursor

} // end of namespace Asylum

#endif
