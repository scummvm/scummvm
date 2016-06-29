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

#include "titanic/support/movie_range_info.h"
#include "titanic/support/movie_clip.h"
#include "titanic/core/game_object.h"

namespace Titanic {

CMovieRangeInfo::CMovieRangeInfo() : ListItem(), _startFrame(0), _endFrame(0) {
}

CMovieRangeInfo::~CMovieRangeInfo() {
	_events.destroyContents();
}

CMovieRangeInfo::CMovieRangeInfo(const CMovieRangeInfo *src) : ListItem() {
	_movieName = src->_movieName;
	_startFrame = src->_startFrame;
	_endFrame = src->_endFrame;

	// Duplicate the events list
	for (CMovieEventList::const_iterator i = _events.begin();
			i != _events.end(); ++i) {
		_events.push_back(new CMovieEvent(*i));
	}
}

void CMovieRangeInfo::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeQuotedLine(_movieName, indent + 1);
	file->writeNumberLine(_endFrame, indent + 1);
	file->writeNumberLine(_startFrame, indent + 1);
	_events.save(file, indent + 1);
}

void CMovieRangeInfo::load(SimpleFile *file) {
	int val = file->readNumber();
	if (!val) {
		_movieName = file->readString();
		_endFrame = file->readNumber();
		_startFrame = file->readNumber();
		_events.load(file);
	}
}

void CMovieRangeInfo::get1(CMovieEventList &list) {
	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_fieldC == 1)
			list.push_back(new CMovieEvent(movieEvent));
	}
}

void CMovieRangeInfo::get2(CMovieEventList &list, int val) {
	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_fieldC == 2 && movieEvent->_field1C == val)
			list.push_back(new CMovieEvent(movieEvent));
	}
}

void CMovieRangeInfo::process(CGameObject *owner) {
	int flags = 0;
	if (_endFrame)
		flags |= CLIPFLAG_HAS_END_FRAME;
	if (_startFrame)
		flags |= CLIPFLAG_HAS_START_FRAME;
	
	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (!movieEvent->_fieldC) {
			flags |= CLIPFLAG_PLAY;
			break;
		}
	}

	owner->checkPlayMovie(_movieName, flags);

	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (!movieEvent->_fieldC)
			owner->surface38(movieEvent->_field1C);
	}
}

} // End of namespace Titanic
