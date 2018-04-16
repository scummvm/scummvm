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

#include "titanic/support/movie_event.h"
#include "titanic/core/game_object.h"

namespace Titanic {

CMovieEvent::CMovieEvent() : ListItem(), _type(MET_PLAY), _startFrame(0),
	_endFrame(0), _initialFrame(0), _gameObject(nullptr) {
}

CMovieEvent::CMovieEvent(const CMovieEvent *src) {
	_type = src->_type;
	_startFrame = src->_startFrame;
	_endFrame = src->_endFrame;
	_initialFrame = src->_initialFrame;
	_gameObject = src->_gameObject;
}

void CMovieEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_startFrame, indent + 1);
	file->writeNumberLine(_endFrame, indent + 1);
	error("FIXME: Original save/loaded object pointer");
	//	file->writeNumberLine(_gameObject, indent + 1);
	file->writeNumberLine(_initialFrame, indent + 1);

	ListItem::save(file, indent);
}

void CMovieEvent::load(SimpleFile *file) {
	int val = file->readNumber();
	if (!val) {
		_startFrame = file->readNumber();
		_endFrame = file->readNumber();
		file->readNumber();
		error("FIXME: Original save/loaded object pointer");
		_initialFrame = file->readNumber();
	}

	ListItem::load(file);
}

} // End of namespace Titanic
