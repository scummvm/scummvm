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

class CCreditLine : public ListItem {
public:
	CString _line;
	uint _lineWidth;
public:
	CCreditLine() : _lineWidth(0) {}
	CCreditLine(const CString &line, uint lineWidth) : _line(line), _lineWidth(lineWidth) {}
};
typedef List<CCreditLine> CCreditLines;

class CCreditLineGroup : public ListItem {
public:
	CCreditLines _lines;
};
typedef List<CCreditLineGroup> CCreditLineGroups;

class CCreditText {
private:
	/**
	 * Sets up needed data
	 */
	void setup();

	/**
	 * Read in a text line from the passed stream
	 */
	CString readLine(Common::SeekableReadStream *stream);

	/**
	 * Handles a group where the .... sequence was encountered
	 */
	void handleDots(CCreditLineGroup *group);
public:
	CScreenManager *_screenManagerP;
	Rect _rect;
	CCreditLineGroups _groups;
	uint _ticks;
	int _fontHeight;
	CGameObject *_objectP;
	CCreditLineGroups::iterator _groupIt;
	CCreditLines::iterator _lineIt;
	int _yOffset;
	int _priorInc;
	int _textR, _textG, _textB;
	int _deltaR, _deltaG, _deltaB;
	int _counter;
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
		const Rect &rect);

	/**
	 * Draw the item
	 */
	bool draw();
};

} // End of namespace Titanic

#endif /* TITANIC_CREDIT_TEXT_H */
