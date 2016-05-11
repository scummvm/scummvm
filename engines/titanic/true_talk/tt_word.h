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

#ifndef TITANIC_TT_WORD_H
#define TITANIC_TT_WORD_H

#include "titanic/support/simple_file.h"
#include "titanic/true_talk/tt_string.h"
#include "titanic/true_talk/tt_string_node.h"

namespace Titanic {

class TTword {
protected:
	TTString _string;
	int _fieldC;
	TTsynonymNode *_synP;
	TTStringStatus _status;
	int _wordMode;
	int _field1C;
	int _field20;
	int _field24;
	int _field28;
protected:
	/**
	 * Read in a number
	 */
	uint readNumber(const char *str);

	bool testFileHandle(SimpleFile *file) const;
public:
	TTword(TTString &str, int mode, int val2);

	/**
	 * Read in a synonym for the given word
	 */
	int readSyn(SimpleFile *file);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file, int mode);
};

class TTword1 : public TTword {
protected:
	int _field2C;
public:
	TTword1(TTString &str, int val1, int val2, int val3);
};

class TTword2 : public TTword1 {
protected:
	int _field30;
public:
	TTword2(TTString &str, int val1, int val2, int val3, int val4);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);
};

class TTword3 : public TTword1 {
protected:
	int _field30;
	int _field34;
	int _field38;
	int _field3C;
public:
	TTword3(TTString &str, int val1, int val2, int val3, int val4, int val5, int val6);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);
};

class TTword4 : public TTword1 {
protected:
	int _field30;
public:
	TTword4(TTString &str, int val1, int val2, int val3, int val4);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);
};

class TTword5 : public TTword1 {
protected:
	int _field30;
public:
	TTword5(TTString &str, int val1, int val2, int val3, int val4);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_WORD_H */
