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
	_startFrame = src->_startFrame;
	_endFrame = src->_endFrame;
	_initialFrame = src->_initialFrame;
	_isReversed = src->_isReversed;
	_isRepeat = src->_isRepeat;

	// Duplicate the events list
	for (CMovieEventList::const_iterator i = _events.begin();
			i != _events.end(); ++i) {
		_events.push_back(new CMovieEvent(*i));
	}
}

void CMovieRangeInfo::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_startFrame, indent + 1);
	file->writeNumberLine(_endFrame, indent + 1);
	file->writeNumberLine(_initialFrame, indent + 1);
	file->writeNumberLine(_isRepeat, indent + 1);
	file->writeNumberLine(_isReversed, indent + 1);
	_events.save(file, indent + 1);

	ListItem::save(file, indent);
}

void CMovieRangeInfo::load(SimpleFile *file) {
	int val = file->readNumber();
	if (!val) {
		_startFrame = file->readNumber();
		_endFrame = file->readNumber();
		_initialFrame = file->readNumber();
		_isRepeat = file->readNumber();
		_isReversed = file->readNumber();
		_events.load(file);
	}

	ListItem::load(file);
}

void CMovieRangeInfo::getMovieEnd(CMovieEventList &list) {
	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_type == MET_MOVIE_END)
			list.push_back(new CMovieEvent(movieEvent));
	}
}

void CMovieRangeInfo::getMovieFrame(CMovieEventList &list, int frameNumber) {
	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_type == MET_FRAME && movieEvent->_initialFrame == frameNumber)
			list.push_back(new CMovieEvent(movieEvent));
	}
}

void CMovieRangeInfo::process(CGameObject *owner) {
	int flags = 0;
	if (_isRepeat)
		flags |= MOVIE_REPEAT;
	if (_isReversed)
		flags |= MOVIE_REVERSE;

	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_type == MET_MOVIE_END) {
			flags |= CLIPFLAG_PLAY;
			break;
		}
	}

	owner->playMovie(_startFrame, _endFrame, _initialFrame, flags);

	for (CMovieEventList::iterator i = _events.begin(); i != _events.end(); ++i) {
		CMovieEvent *movieEvent = *i;
		if (movieEvent->_type == MET_PLAY)
			owner->movieEvent(movieEvent->_initialFrame);
	}
}

} // End of namespace Titanic
