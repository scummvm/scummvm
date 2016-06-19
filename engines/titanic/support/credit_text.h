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

#ifndef TITANIC_CREDIT_TEXT_H
#define TITANIC_CREDIT_TEXT_H

#include "titanic/core/list.h"

namespace Titanic {

class CGameObject;
class CScreenManager;

class COverrideSubItem : public ListItem {

};
typedef List<COverrideSubItem> CCreditTextSubList;

class CCreditTextItem : public ListItem {

};
typedef List<CCreditTextItem> CCreditTextList;

class CCreditText {
private:
	/**
	 * Sets up needed data
	 */
	void setup();
public:
	CScreenManager *_screenManagerP;
	Rect _rect;
	int _field14;
	CCreditTextList _list;
	uint _ticks;
	uint _fontHeight;
	CGameObject *_objectP;
	int _field34;
	int _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	int _field58;
	int _field5C;
public:
	CCreditText();

	/**
	 * Clears the object
	 */
	void clear();

	/**
	 * Sets the game object this override is associated with
	 */
	void load(CGameObject *obj, CScreenManager *screenManager,
		const Rect &rect, int v = 0);

	/**
	 * Draw the item
	 */
	bool draw();
};

} // End of namespace Titanic

#endif /* TITANIC_CREDIT_TEXT_H */
