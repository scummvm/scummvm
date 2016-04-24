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

#ifndef TITANIC_PET_ROOMS_GLYPHS_H
#define TITANIC_PET_ROOMS_GLYPHS_H

#include "titanic/pet_control/pet_glyphs.h"

namespace Titanic {

class CPetRoomsGlyph : public CPetGlyph {
protected:
	int _field34;
	int _field38;
	int _field3C;
	CGameObject *_field40;
	CGameObject *_field44;
	CGameObject *_field48;
	CGameObject *_field4C;
	CGameObject *_field50;
	CGameObject *_field54;
	CGameObject *_field58;
	CGameObject *_field5C;
public:
	CPetRoomsGlyph();

	void set34(int val);

	void set38(int val);

	/**
	 * Setup the glyph
	 */
	virtual bool setup(CPetControl *petControl, CPetGlyphs *owner);

	/**
	 * Draw the glyph at a specified position
	 */
	virtual void drawAt(CScreenManager *screenManager, const Point &pt);

	/**
	 * Handles any secondary drawing of the glyph
	 */
	virtual void draw2(CScreenManager *screenManager) {}

	/**
	 * Handles mouse button down messages
	 */
	virtual void MouseButtonDownMsg(const Point &pt);

	virtual int proc29(const Point &pt);
	virtual void proc32();
	virtual int proc33();
	virtual void proc39();
};

class CPetRoomsGlyphs : public CPetGlyphs {
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ROOMS_GLYPHS_H */
