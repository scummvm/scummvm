/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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


#include "engines/grim/gfx_base.h"
#include "engines/grim/primitives.h"
#include "engines/grim/savegame.h"
#include "engines/grim/grim.h"
#include "engines/grim/color.h"

namespace Grim {

PrimitiveObject::PrimitiveObject() :
		_filled(false), _type(InvalidType) {
}

PrimitiveObject::~PrimitiveObject() {
}

void PrimitiveObject::saveState(SaveGame *savedState) const {
	savedState->writeLESint32((int32)_type);

	savedState->writeColor(_color);

	savedState->writeLEUint32(_filled);

	savedState->writeLEUint16(_p1.x);
	savedState->writeLEUint16(_p1.y);
	savedState->writeLEUint16(_p2.x);
	savedState->writeLEUint16(_p2.y);
	savedState->writeLEUint16(_p3.x);
	savedState->writeLEUint16(_p3.y);
	savedState->writeLEUint16(_p4.x);
	savedState->writeLEUint16(_p4.y);
}

bool PrimitiveObject::restoreState(SaveGame *savedState) {
	_type = (PrimType)savedState->readLESint32();

	_color = savedState->readColor();

	_filled = savedState->readLEUint32();

	_p1.x = savedState->readLEUint16();
	_p1.y = savedState->readLEUint16();
	_p2.x = savedState->readLEUint16();
	_p2.y = savedState->readLEUint16();
	_p3.x = savedState->readLEUint16();
	_p3.y = savedState->readLEUint16();
	_p4.x = savedState->readLEUint16();
	_p4.y = savedState->readLEUint16();

	return true;
}

void PrimitiveObject::createRectangle(const Common::Point &p1, const Common::Point &p2, const Color &color, bool filled) {
	_type = RectangleType;
	_p1 = p1;
	_p2 = p2;
	_color = color;
	_filled = filled;
}

void PrimitiveObject::createLine(const Common::Point &p1, const Common::Point &p2, const Color &color) {
	_type = LineType;
	_p1 = p1;
	_p2 = p2;
	_color = color;
}

void PrimitiveObject::createPolygon(const Common::Point &p1, const Common::Point &p2, const Common::Point &p3, const Common::Point &p4, const Color &color) {
	_type = PolygonType;
	_p1 = p1;
	_p2 = p2;
	_p3 = p3;
	_p4 = p4;
	_color = color;
}

void PrimitiveObject::draw() const {
	assert(_type != InvalidType);

	if (_type == RectangleType) {
		g_driver->drawRectangle(this);
	} else if (_type == LineType) {
		g_driver->drawLine(this);
	} else if (_type == PolygonType) {
		g_driver->drawPolygon(this);
	}
}

void PrimitiveObject::setPos(int x, int y) {
	if (x != -1) {
		int dx = x - _p1.x;
		_p1.x += dx;
		if (_type == RectangleType || _type == LineType || _type == PolygonType) {
			_p2.x += dx;
		}
		if (_type == PolygonType) {
			_p3.x += dx;
			_p4.x += dx;
		}
	}
	if (y != -1) {
		int dy = y - _p1.y;
		_p1.y += dy;
		if (_type == RectangleType || _type == LineType || _type == PolygonType)
			_p2.y += dy;
		if (_type == PolygonType) {
			_p3.y += dy;
			_p4.y += dy;
		}
	}
}

void PrimitiveObject::setEndpoint(int x, int y) {
	assert(_type == LineType);

	_p2.x = x;
	_p2.y = y;
}

} // end of namespace Grim
