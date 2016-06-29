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

#include "titanic/game/hammer_dispensor_button.h"

namespace Titanic {

CHammerDispensorButton::CHammerDispensorButton() : CStartAction(),
	_fieldF8(0), _fieldFC(0), _field100(0), _field104(56),
	_field108(6), _field10C(0), _field110(0) {
}

void CHammerDispensorButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeNumberLine(_field100, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field110, indent);

	CStartAction::save(file, indent);
}

void CHammerDispensorButton::load(SimpleFile *file) {
	file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();
	_field100 = file->readNumber();
	_field104 = file->readNumber();
	_field108 = file->readNumber();
	_field110 = file->readNumber();

	CStartAction::load(file);
}

} // End of namespace Titanic
