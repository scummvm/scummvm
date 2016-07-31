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

#include "titanic/game/cell_point_button.h"

namespace Titanic {

CCellPointButton::CCellPointButton() : CBackground() {
	_fieldE0 = 0;
	_fieldE4 = 0;
	_fieldE8 = 0;
	_fieldEC = 0;
	_fieldF0 = 0;
	_fieldF4 = 0;
	_fieldF8 = 0;
	_fieldFC = 0;
	_field100 = 0;
	_field104 = 0;
	_field108 = 1;
}

void CCellPointButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeNumberLine(_field100, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_field118, indent);

	CBackground::save(file, indent);
}

void CCellPointButton::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();
	_field100 = file->readNumber();
	_field104 = file->readNumber();
	_field108 = file->readNumber();
	_string3 = file->readString();
	_field118 = file->readNumber();

	CBackground::load(file);
}

} // End of namespace Titanic
