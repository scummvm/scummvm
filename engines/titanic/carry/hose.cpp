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

#include "titanic/carry/hose.h"

namespace Titanic {

CHoseStatics *CHose::_statics;

void CHose::init() {
	_statics = new CHoseStatics();
}

void CHose::deinit() {
	delete _statics;
}

CHose::CHose() : CCarry(),
	_string6("Succ-U-Bus auxiliary hose attachment incompatible with sliding glass cover.") {
}

void CHose::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_statics->_v1, indent);
	file->writeQuotedLine(_statics->_v2, indent);
	file->writeQuotedLine(_string6, indent);
	CCarry::save(file, indent);
}

void CHose::load(SimpleFile *file) {
	file->readNumber();
	_statics->_v1 = file->readNumber();
	_statics->_v2 = file->readString();
	_string6 = file->readString();
	CCarry::load(file);
}

} // End of namespace Titanic
