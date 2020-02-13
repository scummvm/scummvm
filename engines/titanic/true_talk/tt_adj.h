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

#ifndef TITANIC_TT_ADJ_H
#define TITANIC_TT_ADJ_H

#include "titanic/true_talk/tt_major_word.h"

namespace Titanic {

class TTadj : public TTmajorWord {
private:
	static bool _staticFlag;
protected:
	int _val;
public:
	TTadj(TTstring &str, WordClass wordClass, int val2, int val3, int val4);
	TTadj(const TTadj *src);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);

	int adjFn1(int val);

	/**
	 * Creates a copy of the word
	 */
	TTword *copy() const override;

	bool proc14(int val) const override { return _val == val; }
	int proc15() const override { return _val; }
	bool proc16() const override { return _val >= 7; }
	bool proc17() const override { return _val <= 3; }
	bool proc18() const override { return _val > 3 && _val < 7; }

	/**
	 * Dumps data associated with the word to file
	 */
	int save(SimpleFile *file) const override {
		return saveData(file, _val);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_TT_ADJ_H */
