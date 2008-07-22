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

#ifndef COMMON_HASHMAP_H
#define COMMON_HASHMAP_H

#include "common/func.h"
#include "common/str.h"
#include "common/util.h"

// FIXME: Since this define is very system dependant, 
// it should be moved to the appropriate H file instead.
// Portdefs might be a good location for example
#if !defined(__SYMBIAN32__)
#define USE_HASHMAP_MEMORY_POOL
#endif

#ifdef USE_HASHMAP_MEMORY_POOL
#include "common/memorypool.h"
// FIXME: we sadly can't assume standard C++ to be present
// on every system we support, so we should get rid of this.
// The solution should be to write a simple placement new
// on our own.
#include <new>
#endif

namespace Common {

// The table sizes ideally are primes. We use a helper function to find
// suitable table sizes.
uint nextTableSize(uint x);


// Enable the following #define if you want to check how many collisions the
// code produces (many collisions indicate either a bad hash function, or a
// hash table that is too small).
//#define DEBUG_HASH_COLLISIONS


/**
 * HashMap<Key,Val> maps objects of type Key to objects of type Val.
 * For each used Key type, we need an "uint hashit(Key,uint)" function
 * that computes a hash for the given Key object and returns it as an
 * an integer from 0 to hashsize-1, and also an "equality functor".
 * that returns true if if its two arguments are to be considered
 * equal. Also, we assume that "=" works on Val objects for assignment.
 *
 * If aa is an HashMap<Key,Val>, then space is allocated each time aa[key] is
 * referenced, for a new key. If the object is const, then an assertion is
 * triggered instead. Hence if you are not sure whether a key is contained in
 * the map, use contains() first to check for its presence.
 */
template<class Key, class Val, class HashFunc = Hash<Key>, class EqualFunc = EqualTo<Key> >
class HashMap {
private:
#if defined (PALMOS_MODE)
public:
#endif

	typedef HashMap<Key, Val, HashFunc, EqualFunc> HM_t;

	struct Node {
		const Key _key;
		Val _value;
		Node(const Key &key) : _key(key), _value() {}
	};


#ifdef USE_HASHMAP_MEMORY_POOL
	MemoryPool _nodePool;

	Node *allocNode(const Key &key) {
		void* mem = _nodePool.malloc();
		return new (mem) Node(key);
	} 

	void freeNode(Node *node) {
		node->~Node();
		_nodePool.free(node);
	}
#else
	Node* allocNode(const Key &key) {
		return new Node(key);
	} 

	void freeNode(Node *node) {
		delete node;
	}
#endif

	Node **_arr;	// hashtable of size arrsize.
	uint _arrsize, _nele;

	HashFunc _hash;
	EqualFunc _equal;

	// Default value, returned by the const getVal.
	const Val _defaultVal;

#ifdef DEBUG_HASH_COLLISIONS
	mutable int _collisions, _lookups;
#endif

	void assign(const HM_t &map);
	int lookup(const Key &key) const;
	int lookupAndCreateIfMissing(const Key &key);
	void expand_array(uint newsize);

	template<class T> friend class IteratorImpl;

	/**
	 * Simple HashMap iterator implementation.
	 */
	template<class NodeType>
	class IteratorImpl {
		friend class HashMap;
		template<class T> friend class IteratorImpl;
	protected:
		typedef const HashMap hashmap_t;

		uint _idx;
		hashmap_t *_hashmap;

	protected:
		IteratorImpl(uint idx, hashmap_t *hashmap) : _idx(idx), _hashmap(hashmap) {}

		NodeType *deref() const {
			assert(_hashmap != 0);
			assert(_idx < _hashmap->_arrsize);
			Node *node = _hashmap->_arr[_idx];
			assert(node != 0);
			return node;
		}

	public:
		IteratorImpl() : _idx(0), _hashmap(0) {}
		template<class T>
		IteratorImpl(const IteratorImpl<T> &c) : _idx(c._idx), _hashmap(c._hashmap) {}

		NodeType &operator*() const { return *deref(); }
		NodeType *operator->() const { return deref(); }

		bool operator==(const IteratorImpl &iter) const { return _idx == iter._idx && _hashmap == iter._hashmap; }
		bool operator!=(const IteratorImpl &iter) const { return !(*this == iter); }

		IteratorImpl &operator++() {
			assert(_hashmap);
			do {
				_idx++;
			} while (_idx < _hashmap->_arrsize && _hashmap->_arr[_idx] == 0);
			if (_idx >= _hashmap->_arrsize)
				_idx = (uint)-1;

			return *this;
		}

		IteratorImpl operator++(int) {
			IteratorImpl old = *this;
			operator ++();
			return old;
		}
	};

public:
	typedef IteratorImpl<Node> iterator;
	typedef IteratorImpl<const Node> const_iterator;

	HashMap();
	HashMap(const HM_t &map);
	~HashMap();

	HM_t &operator=(const HM_t &map) {
		if (this == &map)
			return *this;

		// Remove the previous content and ...
		clear();
		delete[] _arr;
		// ... copy the new stuff.
		assign(map);
		return *this;
	}

	bool contains(const Key &key) const;

	Val &operator[](const Key &key);
	const Val &operator[](const Key &key) const;

	Val &getVal(const Key &key);
	const Val &getVal(const Key &key) const;
	void setVal(const Key &key, const Val &val);

	void clear(bool shrinkArray = 0);

	void erase(const Key &key);

	uint size() const { return _nele; }

	iterator	begin() {
		// Find and return the _key non-empty entry
		for (uint ctr = 0; ctr < _arrsize; ++ctr) {
			if (_arr[ctr])
				return iterator(ctr, this);
		}
		return end();
	}
	iterator	end() {
		return iterator((uint)-1, this);
	}

	const_iterator	begin() const {
		// Find and return the first non-empty entry
		for (uint ctr = 0; ctr < _arrsize; ++ctr) {
			if (_arr[ctr])
				return const_iterator(ctr, this);
		}
		return end();
	}
	const_iterator	end() const {
		return const_iterator((uint)-1, this);
	}

	iterator	find(const Key &key) {
		uint ctr = lookup(key);
		if (_arr[ctr])
			return iterator(ctr, this);
		return end();
	}

	const_iterator	find(const Key &key) const {
		uint ctr = lookup(key);
		if (_arr[ctr])
			return const_iterator(ctr, this);
		return end();
	}

	// TODO: insert() method?

	bool empty() const {
		return (_nele == 0);
	}
};

//-------------------------------------------------------
// HashMap functions

/**
 * Base constructor, creates an empty hashmap.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::HashMap() :
#ifdef USE_HASHMAP_MEMORY_POOL
	_nodePool(sizeof(Node)),
#endif
	_defaultVal() {
	_arrsize = nextTableSize(0);
	_arr = new Node *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(Node *));

	_nele = 0;

#ifdef DEBUG_HASH_COLLISIONS
	_collisions = 0;
	_lookups = 0;
#endif
}

/**
 * Copy constructor, creates a full copy of the given hashmap.
 * We must provide a custom copy constructor as we use pointers
 * to heap buffers for the internal storage.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::HashMap(const HM_t &map) : 
#ifdef USE_HASHMAP_MEMORY_POOL
	_nodePool(sizeof(Node)),
#endif
	_defaultVal()  {
	assign(map);
}

/**
 * Destructor, frees all used memory.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::~HashMap() {
	for (uint ctr = 0; ctr < _arrsize; ++ctr)
		if (_arr[ctr] != NULL)
		  freeNode(_arr[ctr]);

	delete[] _arr;
}

/**
 * Internal method for assigning the content of another HashMap
 * to this one.
 *
 * @note We do *not* deallocate the previous storage here -- the caller is
 *       responsible for doing that!
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::assign(const HM_t &map) {
	_arrsize = map._arrsize;
	_arr = new Node *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(Node *));

	// Simply clone the map given to us, one by one.
	_nele = 0;
	for (uint ctr = 0; ctr < _arrsize; ++ctr) {
		if (map._arr[ctr] != NULL) {
			_arr[ctr] = allocNode(map._arr[ctr]->_key);
			_arr[ctr]->_value = map._arr[ctr]->_value;
			_nele++;
		}
	}
	// Perform a sanity check (to help track down hashmap corruption)
	assert(_nele == map._nele);
}


template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::clear(bool shrinkArray) {
	for (uint ctr = 0; ctr < _arrsize; ++ctr) {
		if (_arr[ctr] != NULL) {
			freeNode(_arr[ctr]);
			_arr[ctr] = NULL;
		}
	}

	if (shrinkArray && _arrsize > nextTableSize(0)) {
		delete[] _arr;

		_arrsize = nextTableSize(0);
		_arr = new Node *[_arrsize];
		assert(_arr != NULL);
		memset(_arr, 0, _arrsize * sizeof(Node *));
	}

	_nele = 0;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::expand_array(uint newsize) {
	assert(newsize > _arrsize);
	uint ctr, dex;

	const uint old_nele = _nele;
	const uint old_arrsize = _arrsize;
	Node **old_arr = _arr;

	// allocate a new array
	_nele = 0;
	_arrsize = newsize;
	_arr = new Node *[_arrsize];
	assert(_arr != NULL);
	memset(_arr, 0, _arrsize * sizeof(Node *));

	// rehash all the old elements
	for (ctr = 0; ctr < old_arrsize; ++ctr) {
		if (old_arr[ctr] == NULL)
			continue;

		// Insert the element from the old table into the new table.
		// Since we know that no key exists twice in the old table, we
		// can do this slightly better than by calling lookup, since we
		// don't have to call _equal().
		dex = _hash(old_arr[ctr]->_key) % _arrsize;
		while (_arr[dex] != NULL) {
			dex = (dex + 1) % _arrsize;
		}

		_arr[dex] = old_arr[ctr];
		_nele++;
	}

	// Perform a sanity check: Old number of elements should match the new one!
	// This check will fail if some previous operation corrupted this hashmap.
	assert(_nele == old_nele);

	delete[] old_arr;

	return;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
int HashMap<Key, Val, HashFunc, EqualFunc>::lookup(const Key &key) const {
	uint ctr = _hash(key) % _arrsize;

	while (_arr[ctr] != NULL && !_equal(_arr[ctr]->_key, key)) {
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

template<class Key, class Val, class HashFunc, class EqualFunc>
int HashMap<Key, Val, HashFunc, EqualFunc>::lookupAndCreateIfMissing(const Key &key) {
	uint ctr = lookup(key);

	if (_arr[ctr] == NULL) {
		_arr[ctr] = allocNode(key);
		_nele++;

		// Keep the load factor below 75%.
		if (_nele > _arrsize * 75 / 100) {
			expand_array(nextTableSize(_arrsize));
			ctr = lookup(key);
		}
	}

	return ctr;
}


template<class Key, class Val, class HashFunc, class EqualFunc>
bool HashMap<Key, Val, HashFunc, EqualFunc>::contains(const Key &key) const {
	uint ctr = lookup(key);
	return (_arr[ctr] != NULL);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator[](const Key &key) {
	return getVal(key);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator[](const Key &key) const {
	return getVal(key);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
Val &HashMap<Key, Val, HashFunc, EqualFunc>::getVal(const Key &key) {
	uint ctr = lookupAndCreateIfMissing(key);
	assert(_arr[ctr] != NULL);
	return _arr[ctr]->_value;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::getVal(const Key &key) const {
	uint ctr = lookup(key);
	if (_arr[ctr] != NULL)
		return _arr[ctr]->_value;
	else
		return _defaultVal;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::setVal(const Key &key, const Val &val) {
	uint ctr = lookupAndCreateIfMissing(key);
	assert(_arr[ctr] != NULL);
	_arr[ctr]->_value = val;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::erase(const Key &key) {
	// This is based on code in the Wikipedia article on Hash tables.
	uint i = lookup(key);
	if (_arr[i] == NULL)
		return; // key wasn't present, so no work has to be done
	// If we remove a key, we must check all subsequent keys and possibly
	// reinsert them.
	uint j = i;
	freeNode(_arr[i]);
	_arr[i] = NULL;
	while (true) {
		// Look at the next table slot
		j = (j + 1) % _arrsize;
		// If the next slot is empty, we are done
		if (_arr[j] == NULL)
			break;
		// Compute the slot where the content of the next slot should normally be,
		// assuming an empty table, and check whether we have to move it.
		uint k = _hash(_arr[j]->_key) % _arrsize;
		if ((j > i && (k <= i || k > j)) ||
		    (j < i && (k <= i && k > j)) ) {
			_arr[i] = _arr[j];
			i = j;
		}
	}
	_arr[i] = NULL;
	_nele--;
	return;
}

}	// End of namespace Common

#endif
