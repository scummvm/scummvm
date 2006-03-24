/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Code is based on:

/* 
 * Copyright (c) 1998-2003 Massachusetts Institute of Technology. 
 * This code was developed as part of the Haystack research project 
 * (http://haystack.lcs.mit.edu/). Permission is hereby granted, 
 * free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to 
 * the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

/*************************************************

  assocarray.h - Associative arrays

  Andrew Y. Ng, 1996

**************************************************/

#include "common/stdafx.h"
#include "common/str.h"
#include "common/util.h"

#ifndef COMMON_ASSOCARRAY_H
#define COMMON_ASSOCARRAY_H

namespace Common {

#define INIT_SIZE 11

typedef Common::String String;

// If aa is an AssocArray<Key,Val>, then space is allocated each
// time aa[key] is referenced, for a new key. To "see" the value
// of aa[key] but without allocating new memory, use aa.queryVal(key)
// instead.
// 
// An AssocArray<Key,Val> Maps type Key to type Val. For each 
// Key, we need a int hashit(Key,int) that hashes Key and returns 
// an integer from 0 to hashsize-1, and a int data_eq(Key,Key) 
// that returns true if the 2 arguments it is passed are to
// be considered equal. Also, we assume that "=" works
// on Val's for assignment.

int hashit(int x, int hashsize);
int data_eq(int x, int y);
int hashit(double x, int hashsize);
int data_eq(double x, double y);
int hashit(const char *str, int hashsize);
int data_eq(const char *str1, const char *str2);
int hashit(const String &str, int hashsize);
int data_eq(const String &str1, const String &str2);

// data structure used by AssocArray internally to keep
// track of what's mapped to what.
template <class Key, class Val>
struct aa_ref_t {
	Key index;
	Val dat;
};

template <class Key, class Val>
class AssocArray {
private:
	aa_ref_t <Key, Val> **_arr;	// hashtable of size arrsize.
	Val _default_value;
	int _arrsize, _nele;

	inline void expand_array(void);
	inline Val &subscript_helper(Key &index);	// like [], but never expands array

	inline int nele_val(void) const { return _nele; }

public:

	inline Val &operator [](const Key &index);
//	inline AssocArray(Val default_value = Val());
	inline AssocArray(Val default_value);
	inline ~AssocArray();
	inline int contains(const Key &index) const;
	inline Key *new_all_keys(void) const;
	inline Val *new_all_values(void) const;
	inline Val queryVal(const Key &index) const;
	inline void clear(int shrinkArray = 0);
};

//-------------------------------------------------------
// AssocArray functions

template <class Key, class Val>
inline int AssocArray <Key, Val>::contains(const Key &index) const {
	int ctr;

	ctr = hashit(index, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->index, index)) {
		ctr++;

		if (ctr == _arrsize)
			ctr = 0;
	}

	if (_arr[ctr] == NULL)
		return 0;
	else
		return 1;
}

template <class Key, class Val>
inline Key *AssocArray <Key, Val>::new_all_keys(void) const {
	Key *all_keys;
	int ctr, dex;

	if (_nele == 0)
		return NULL;

	all_keys = new Key[_nele];

	assert(all_keys != NULL);

	dex = 0;
	for (ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] == NULL)
			continue;
		all_keys[dex++] = _arr[ctr]->index;

		assert(dex <= _nele);
	}

	assert(dex == _nele);

	return all_keys;
}

template <class Key, class Val>
inline Val *AssocArray <Key, Val>::new_all_values(void) const {
	Val *all_values;
	int ctr, dex;

	if (_nele == 0)
		return NULL;

	all_values = new Val[_nele];

	assert(all_values != NULL);

	dex = 0;
	for (ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] == NULL)
			continue;

		all_values[dex++] = _arr[ctr]->dat;

		assert(dex <= _nele);
	}

	assert(dex == _nele);

	return all_values;
}

template <class Key, class Val>
inline AssocArray <Key, Val>::AssocArray(Val default_value) : _default_value(default_value) {
//	int ctr;

	_arr = new aa_ref_t <Key, Val> *[INIT_SIZE];

//	for (ctr = 0; ctr < INIT_SIZE; ctr++)
//		_arr[ctr] = NULL;

//	assert(_arr != NULL);

	_arrsize = INIT_SIZE;
	_nele = 0;
}

template <class Key, class Val>
inline AssocArray <Key, Val>::~AssocArray() {
	int ctr;

	for (ctr = 0; ctr < _arrsize; ctr++)
		if (_arr[ctr] != NULL)
			delete _arr[ctr];

	delete[] _arr;
}

template <class Key, class Val>
inline void AssocArray <Key, Val>::clear(int shrinkArray) {
	for (int ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] != NULL) {
			delete _arr[ctr];
			_arr[ctr] = NULL;
		}
	}

	shrinkArray = 0;
	if (shrinkArray && _arrsize > INIT_SIZE) {
		delete _arr;

		_arr = new aa_ref_t <Key, Val> *[INIT_SIZE];
		_arrsize = INIT_SIZE;

		for (int ctr = 0; ctr < _arrsize; ctr++)
			_arr[ctr] = NULL;
	}

	_nele = 0;
}

static int is_odd(int x) {return x&1;}
static int is_even(int x) {return !is_odd(x);}

template <class Key, class Val>
inline void AssocArray <Key, Val>::expand_array(void) {
	aa_ref_t <Key, Val> **old_arr;
	int old_arrsize, old_nele, ctr, dex;

	old_nele = _nele;
	old_arr = _arr;
	old_arrsize = _arrsize;

    // GROWTH_FACTOR 1.531415936535
	// allocate a new array 
	_arrsize = 153 * old_arrsize / 100;

	if (is_even(_arrsize))
		_arrsize++;

	_arr = new aa_ref_t <Key, Val> *[_arrsize];

	assert(_arr != NULL);

	for (ctr = 0; ctr < _arrsize; ctr++)
		_arr[ctr] = NULL;

	_nele = 0;

	// rehash all the old elements
	for (ctr = 0; ctr < old_arrsize; ctr++) {
		if (old_arr[ctr] == NULL)
			continue;

//      (*this)[old_arr[ctr]->index] = old_arr[ctr]->dat;
		dex = hashit(old_arr[ctr]->index, _arrsize);

		while (_arr[dex] != NULL)
			if (++dex == _arrsize)
				dex = 0;

		_arr[dex] = old_arr[ctr];
		_nele++;
	}

	assert(_nele == old_nele);

	delete[] old_arr;

	return;
}

// like [], but never expands array.
// Precond: index is a key that is already in the hash table.
template <class Key, class Val>
inline Val &AssocArray <Key, Val>::subscript_helper(Key &index) {
	int ctr;

	ctr = hashit(index, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->index, index)) {
		ctr++;

		if (ctr == _arrsize)
			ctr = 0;
	}

//  if (_arr[ctr] == NULL)
//      {
//      _arr[ctr] = new aa_ref_t<Val,Key>;
//      _arr[ctr]->index = index;
//      _arr[ctr]->dat = _default_value;
//      _nele++;
//      }

	assert(_arr[ctr] != NULL);

	return _arr[ctr]->dat;
}

template <class Key, class Val>
inline Val &AssocArray <Key, Val>::operator [](const Key &index) {
	int ctr;

	ctr = hashit(index, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->index, index)) {
		ctr++;

		if (ctr == _arrsize)
			ctr = 0;
	}

	if (_arr[ctr] == NULL) {
		_arr[ctr] = new aa_ref_t <Key, Val>;
		_arr[ctr]->index = index;
		_arr[ctr]->dat = _default_value;
		_nele++;

		if (_nele > _arrsize / 2) {
			expand_array();

			return (*this)[index];
		} else {
			return _arr[ctr]->dat;
		}
	}

	return _arr[ctr]->dat;
}

template <class Key, class Val>
inline Val AssocArray <Key, Val>::queryVal(const Key &index) const {
	int ctr;

	ctr = hashit(index, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->index, index)) {
		ctr++;

		if (ctr == _arrsize)
			ctr = 0;
	}

	if (_arr[ctr] == NULL)
		return _default_value;
	else
		return _arr[ctr]->dat;
}

}	// End of namespace Common

#endif
