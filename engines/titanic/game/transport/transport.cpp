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

#include "titanic/game/transport/transport.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CTransport, CMobile);

CTransport::CTransport() : CMobile(), _string1("*.*.*") {
}

void CTransport::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);

	CMobile::save(file, indent);
}

void CTransport::load(SimpleFile *file) {
	file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();

	CMobile::load(file);
}

} // End of namespace Titanic
