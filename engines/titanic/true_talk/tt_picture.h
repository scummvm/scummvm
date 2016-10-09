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

#ifndef TITANIC_TT_PICTURE_H
#define TITANIC_TT_PICTURE_H

#include "titanic/true_talk/tt_major_word.h"

namespace Titanic {

class TTpicture : public TTmajorWord {
private:
	static bool _staticFlag;
protected:
	int _field30;
	uint _tag;
	int _field38;
	int _field3C;
public:
	TTpicture(const TTstring &str, WordClass wordClass, int val2, int val3, int val4, int val5, int val6);
	TTpicture(const TTpicture *src);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);

	/**
	 * Creates a copy of the word
	 */
	virtual TTword *copy() const;

	/**
	 * Checks whether the word's tag is a known type
	 */
	virtual bool checkTag() const;

	/**
	 * Compare the word's tag to a given tag value
	 */
	virtual bool compareTagTo(uint tag) const;

	/**
	 * Return the tag associated with the word
	 */
	virtual uint getTag() const;

	virtual bool proc9(int val) const;
	virtual int proc10() const;

};

} // End of namespace Titanic

#endif /* TITANIC_TT_PICTURE_H */
