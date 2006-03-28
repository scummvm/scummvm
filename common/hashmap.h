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

// The hash map (associative array) implementation in this file is
// based on code by Andrew Y. Ng, 1996:

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

#include "common/stdafx.h"
#include "common/str.h"
#include "common/util.h"

#ifndef COMMON_HASHMAP_H
#define COMMON_HASHMAP_H

namespace Common {

typedef Common::String String;

// If aa is an HashMap<Key,Val>, then space is allocated each
// time aa[key] is referenced, for a new key. To "see" the value
// of aa[key] but without allocating new memory, use aa.queryVal(key)
// instead.
// 
// A HashMap<Key,Val> maps objects of type Key to objects of type Val.
// For each used Key type, we need an "uint hashit(Key,uint)" function
// that computes a hash for the given Key object and returns it as an
// an integer from 0 to hashsize-1, and also a "bool data_eq(Key,Key) "
// function that returns true if if its two arguments are to be considered
// equal. Also, we assume that "=" works on Val objects for assignment.

uint hashit(int x, uint hashsize);
bool data_eq(int x, int y);
uint hashit(double x, uint hashsize);
bool data_eq(double x, double y);
uint hashit(const char *str, uint hashsize);
bool data_eq(const char *str1, const char *str2);
uint hashit(const String &str, uint hashsize);
bool data_eq(const String &str1, const String &str2);


// The table sizes ideally are primes. We use a helper function to find
// suitable table sizes.
uint nextTableSize(uint x);


// Enable the following #define if you want to check how many collisions the
// code produces (many collisions indicate either a bad hash function, or a
// hash table that is too small).
//#define DEBUG_HASH_COLLISIONS

template <class Key, class Val>
class HashMap {
private:
	// data structure used by HashMap internally to keep
	// track of what's mapped to what.
	struct BaseNode {
		Key _key;
		Val _value;
		BaseNode() {}
		BaseNode(const Key &key) : _key(key) {}
	};
	
	BaseNode **_arr;	// hashtable of size arrsize.
	uint _arrsize, _nele;
	
#ifdef DEBUG_HASH_COLLISIONS
	mutable int _collisions, _lookups;
#endif

	int lookup(const Key &key) const;
	void expand_array(uint newsize);

public:

	HashMap();
	~HashMap();

	bool contains(const Key &key) const;

	Val &operator [](const Key &key);
	const Val &operator [](const Key &key) const;
	const Val &queryVal(const Key &key) const;

	void clear(bool shrinkArray = 0);

	// The following two methods are used to return a list of
	// all the keys / all the values (or rather, duplicates of them).
	// Currently they aren't used, and I think a better appraoch
	// is to add iterators, which allow the same in a more C++-style
	// fashion, do not require making duplicates, and finally
	// even allow in-place modifications of
	//const_iterator	begin() const
	//const_iterator	end() const
	
	// TODO: There is no "remove" method yet.
	//void remove(const Key &key);


	bool empty() const {
		return (_nele == 0);
	}
};

//-------------------------------------------------------
// HashMap functions

template <class Key, class Val>
HashMap<Key, Val>::HashMap() {
	_arrsize = nextTableSize(0);
	_arr = new BaseNode *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(BaseNode *));

	_nele = 0;
	
#ifdef DEBUG_HASH_COLLISIONS
	_collisions = 0;
	_lookups = 0;
#endif
}

template <class Key, class Val>
HashMap<Key, Val>::~HashMap() {
	uint ctr;

	for (ctr = 0; ctr < _arrsize; ctr++)
		if (_arr[ctr] != NULL)
			delete _arr[ctr];

	delete[] _arr;
}

template <class Key, class Val>
void HashMap<Key, Val>::clear(bool shrinkArray) {
	for (uint ctr = 0; ctr < _arrsize; ctr++) {
		if (_arr[ctr] != NULL) {
			delete _arr[ctr];
			_arr[ctr] = NULL;
		}
	}

	if (shrinkArray && _arrsize > nextTableSize(0)) {
		delete[] _arr;

		_arrsize = nextTableSize(0);
		_arr = new BaseNode *[_arrsize];
		assert(_arr != NULL);
		memset(_arr, 0, _arrsize * sizeof(BaseNode *));
	}

	_nele = 0;
}

template <class Key, class Val>
void HashMap<Key, Val>::expand_array(uint newsize) {
	assert(newsize > _arrsize);
	BaseNode **old_arr;
	uint old_arrsize, old_nele, ctr, dex;

	old_nele = _nele;
	old_arr = _arr;
	old_arrsize = _arrsize;

	// allocate a new array 
	_arrsize = newsize;
	_arr = new BaseNode *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(BaseNode *));

	_nele = 0;

	// rehash all the old elements
	for (ctr = 0; ctr < old_arrsize; ctr++) {
		if (old_arr[ctr] == NULL)
			continue;

		// Insert the element from the old table into the new table.
		// Since we know that no key exists twice the old table, we
		// can do this slightly better than by calling lookup, since we
		// don't have to call data_eq().
		dex = hashit(old_arr[ctr]->_key, _arrsize);
		while (_arr[dex] != NULL) {
			dex = (dex + 1) % _arrsize;
		}

		_arr[dex] = old_arr[ctr];
		_nele++;
	}

	// Perform a sanity check: Old number of elements should match the new one!
	assert(_nele == old_nele);

	delete[] old_arr;

	return;
}

template <class Key, class Val>
int HashMap<Key, Val>::lookup(const Key &key) const {
	uint ctr = hashit(key, _arrsize);

	while (_arr[ctr] != NULL && !data_eq(_arr[ctr]->_key, key)) {
		ctr = (ctr + 1) % _arrsize;

#ifdef DEBUG_HASH_COLLISIONS
		_collisions++;
#endif
	}
	
#ifdef DEBUG_HASH_COLLISIONS
	_lookups++;
	fprintf(stderr, "collisions %d, lookups %d, ratio %f in HashMap %p; size %d num elements %d\n",
		_collisions, _lookups, ((double) _collisions / (double)_lookups),
		(const void *)this, _arrsize, _nele);
#endif

	return ctr;
}

template <class Key, class Val>
bool HashMap<Key, Val>::contains(const Key &key) const {
	uint ctr = lookup(key);
	return (_arr[ctr] != NULL);
}

template <class Key, class Val>
Val &HashMap<Key, Val>::operator [](const Key &key) {
	uint ctr = lookup(key);

	if (_arr[ctr] == NULL) {
		_arr[ctr] = new BaseNode(key);
		_nele++;

		// Keep the load factor below 75%.
		if (_nele > _arrsize * 65 / 100) {
			expand_array(nextTableSize(_arrsize));
			ctr = lookup(key);
		}
	}

	return _arr[ctr]->_value;
}

template <class Key, class Val>
const Val &HashMap<Key, Val>::operator [](const Key &key) const {
	return queryVal(key);
}

template <class Key, class Val>
const Val &HashMap<Key, Val>::queryVal(const Key &key) const {
	uint ctr = lookup(key);
	assert(_arr[ctr] != NULL);
	return _arr[ctr]->_value;
}

}	// End of namespace Common

#endif
