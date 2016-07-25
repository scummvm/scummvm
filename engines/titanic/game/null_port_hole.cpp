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

#include "titanic/game/null_port_hole.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CNullPortHole, CClickResponder);

CNullPortHole::CNullPortHole() : CClickResponder() {
	_string1 = "For a better view, why not visit the Promenade Deck?";
	_string2 = "b#48.wav";
}

void CNullPortHole::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string1, indent);

	CClickResponder::save(file, indent);
}

void CNullPortHole::load(SimpleFile *file) {
	file->readNumber();
	_string2 = file->readString();
	_string1 = file->readString();

	CClickResponder::load(file);
}

} // End of namespace Titanic
