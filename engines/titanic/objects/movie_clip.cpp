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

#include "titanic/objects/movie_clip.h"

namespace Titanic {

CMovieClip::CMovieClip() {
}

void CMovieClip::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(2, indent);
	file->writeQuotedLine("Clip", indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_field18, indent);
	file->writeNumberLine(_field1C, indent);

	ListItem::save(file, indent);
}

void CMovieClip::load(SimpleFile *file) {
	int val = file->readNumber();

	switch (val) {
	case 1:
		_string1 = file->readString();
		_field18 = file->readNumber();
		_field1C = file->readNumber();
		_field20 = file->readNumber();
		_field24 = file->readNumber();
		_field28 = file->readNumber();
		_field2C = file->readNumber();
		_field30 = file->readNumber();
		break;

	case 2:
		file->readString();
		_string1 = file->readString();
		_field18 = file->readNumber();
		_field1C = file->readNumber();
		break;

	default:
		break;
	}

	ListItem::load(file);
}

} // End of namespace Titanic
