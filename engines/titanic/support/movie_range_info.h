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

#ifndef TITANIC_MOVIE_RANGE_INFO_H
#define TITANIC_MOVIE_RANGE_INFO_H

#include "video/video_decoder.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"
#include "titanic/support/movie_event.h"

namespace Titanic {

class CMovieRangeInfo : public ListItem {
public:
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
	int _field1C;
	CMovieEventList _events;
public:
	CMovieRangeInfo();
	CMovieRangeInfo(const CMovieRangeInfo *src);
	virtual ~CMovieRangeInfo() {}

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Adds an event to the events list
	 */
	void add(CMovieEvent *movieEvent) { _events.push_back(movieEvent); }
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_RANGE_INFO_H */
