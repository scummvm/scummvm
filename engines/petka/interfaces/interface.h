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

#ifndef PETKA_INTERFACE_H
#define PETKA_INTERFACE_H

#include "common/ustr.h"
#include "common/rect.h"
#include "common/array.h"

namespace Petka {

class QVisibleObject;
class QText;

class Interface {
public:
	Interface();
	virtual ~Interface() {}

	virtual void start(int id) {};
	virtual void stop() {};

	virtual void onLeftButtonDown(const Common::Point p) {};
	virtual void onRightButtonDown(const Common::Point p) {};
	virtual void onMouseMove(const Common::Point p) {};

	void setText(const Common::U32String &text, uint16 textColor, uint16 outlineColor);
	void setTextPhrase(const Common::U32String &text, uint16 textColor, uint16 outlineColor);

	void removeTexts();

	QVisibleObject *findObject(int resourceId);

	void initCursor(int id, bool show, bool animate);

public:
	Common::Array<QVisibleObject *> _objs;
	QVisibleObject *_objUnderCursor;
	int _startIndex;
};

} // End of namespace Petka

#endif
