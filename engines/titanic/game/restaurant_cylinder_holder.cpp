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

#include "titanic/game/restaurant_cylinder_holder.h"

namespace Titanic {

CRestaurantCylinderHolder::CRestaurantCylinderHolder() : CDropTarget(),
	_field118(0), _field11C(0), _field12C(0), _field130(0),
	_string6("z#61.wav"), _field140(1) {
}

void CRestaurantCylinderHolder::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeQuotedLine(_string5, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field140, indent);

	CDropTarget::save(file, indent);
}

void CRestaurantCylinderHolder::load(SimpleFile *file) {
	file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_string5 = file->readString();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_string6 = file->readString();
	_field140 = file->readNumber();

	CDropTarget::load(file);
}

} // End of namespace Titanic
