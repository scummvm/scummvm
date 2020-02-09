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

#ifndef TITANIC_MOVIE_CLIP_H
#define TITANIC_MOVIE_CLIP_H

#include "titanic/core/list.h"

namespace Titanic {

enum ClipFlag {
	CLIPFLAG_HAS_END_FRAME = 1,
	CLIPFLAG_4 = 4,
	CLIPFLAG_HAS_START_FRAME = 8,
	CLIPFLAG_PLAY = 0x10
};

class CGameObject;

/**
 * Movie clip
 */
class CMovieClip : public ListItem {
private:
	Common::List<void *> _items;
	CString _string2;
	CString _string3;
public:
	CString _name;
	int _startFrame;
	int _endFrame;
public:
	CLASSDEF;
	CMovieClip();
	CMovieClip(const CString &name, int startFrame, int endFrame);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

/**
 * Movie clip list
 */
class CMovieClipList: public List<CMovieClip> {
public:
	/**
	 * Finds and returns a movie clip in the list by name
	 */
	CMovieClip *findByName(const Common::String &name) const;

	/**
	 * Returns true if a clip exists in the list with a given name
	 * and starting frame number
	 */
	bool existsByStart(const CString &name, int startFrame = 0) const;

	/**
	 * Returns true if a clip exists in the list with a given name
	 * and starting frame number
	 */
	bool existsByEnd(const CString &name, int endFrame = 0) const;
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_CLIP_H */
