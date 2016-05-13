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
#include "titanic/true_talk/tt_synonym.h"

namespace Titanic {

class TTword {
protected:
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

	bool testFileHandle(SimpleFile *file) const { return true; }
	bool testFileHandle(FileHandle resHandle) const;
public:
	TTword *_pNext;
	TTsynonym *_synP;
	TTString _string;
public:
	TTword(TTString &str, int mode, int val2);
	TTword(TTword *src);

	/**
	 * Delete any following words chained to the word
	 */
	void deleteSiblings();

	/**
	 * Read in a synonym for the given word
	 */
	int readSyn(SimpleFile *file);

	/**
	 * Either sets the first synonym for a word, or adds it to an existing one
	 */
	void appendNode(TTsynonym *node);

	/**
	 * Load the word
	 */
	int load(SimpleFile *file, int mode);

	TTword *scanCopy(const TTString &str, TTsynonym *node, int mode);

	const char *c_str() const { return _string.c_str(); }
	operator const char *() const { return c_str(); }

	/**
	 * Creates a copy of the word
	 */
	virtual TTword *copy();
	
	virtual int proc2() const { return 0; }
	virtual int proc3() const { return -1; }
	virtual void proc4() {}
	virtual void proc5() {}
	virtual int proc6() const { return 0; }
	virtual int proc7() const { return 0; }
	virtual int proc8() const { return 0; }
	virtual int proc9() const { return 0; }
	virtual int proc10() const { return 0; }
	virtual void proc11() {}
	virtual int proc12() const { return 0; }
	virtual int proc13() const { return 0; }
	virtual int proc14() const { return 0; }
	virtual int proc15() const { return -1; }
	virtual int proc16() const { return 0; }
	virtual int proc17() const { return 0; }
	virtual int proc18() const { return 0; }
	virtual int proc19() const { return 0; }
	virtual int proc20() const { return 0; }

	/**
	 * Returns the file associated with the word's first synonym
	 */
	virtual FileHandle getSynFile() const;

	/**
	 * Checks whether the file associated with the word's first
	 * synonym matches the specified file
	 */
	virtual bool checkSynFile(FileHandle file) const;

	/**
	 * Sets the file associated with a synonym
	 */
	virtual void setSynFile(FileHandle file);

	virtual int proc24() const { return 0; }
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
