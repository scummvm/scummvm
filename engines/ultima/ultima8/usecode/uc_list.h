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

#ifndef ULTIMA8_USECODE_UCLIST_H
#define ULTIMA8_USECODE_UCLIST_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class IDataSource;
class ODataSource;

// stringlists: _elementSize = 2, each element is actually a stringref
// see for example the 0x0E opcode: there is no way to see if the
// created list is a stringlist or not
// the opcodes which do need a distinction have a operand for this.

// Question: how are unionList/substractList supposed to know what to do?
// their behaviour differs if this is a stringlist

// Question: does substractList remove _all_ occurences of elements or only 1?

class UCList {
	Std::vector<uint8> _elements;
	unsigned int _elementSize;
	unsigned int _size;

public:
	UCList(unsigned int elementSize, unsigned int capacity = 0) :
		_elementSize(elementSize), _size(0) {
		if (capacity > 0)
			_elements.reserve(_elementSize * capacity);
	}

	~UCList() {
		// Slight problem: we don't know if we're a stringlist.
		// So we need to hope something else has ensured any strings
		// are already freed.
		free();
	}

	const uint8 *operator[](uint32 index) {
		// check that index isn't out of bounds...
		return &(_elements[index * _elementSize]);
	}

	uint16 getuint16(uint32 index) {
		assert(_elementSize == 2);
		uint16 t = _elements[index * _elementSize];
		t += _elements[index * _elementSize + 1] << 8;
		return t;
	}

	void append(const uint8 *e) {
		_elements.resize((_size + 1) * _elementSize);
		for (unsigned int i = 0; i < _elementSize; i++)
			_elements[_size * _elementSize + i] = e[i];
		_size++;
	}

	void remove(const uint8 *e) {
		// do we need to erase all occurences of e or just the first one?
		// (deleting all, currently)
		for (unsigned int i = 0; i < _size; i++) {
			bool equal = true;
			for (unsigned int j = 0; j < _elementSize && equal; j++)
				equal = equal && (_elements[i * _elementSize + j] == e[j]);
			if (equal) {
				_elements.erase(_elements.begin() + i * _elementSize,
				               _elements.begin() + (i + 1)*_elementSize);
				_size--;
				i--; // back up a bit
			}
		}
	}

	bool inList(const uint8 *e) {
		for (unsigned int i = 0; i < _size; i++) {
			bool equal = true;
			for (unsigned int j = 0; j < _elementSize && equal; j++)
				equal = (_elements[i * _elementSize + j] == e[j]);
			if (equal)
				return true;
		}
		return false;
	}

	void appendList(UCList &l) {
		// need to check if elementsizes match...
		_elements.reserve(_elementSize * (_size + l._size));
		unsigned int lsize = l._size;
		for (unsigned int i = 0; i < lsize; i++)
			append(l[i]);
	}
	void unionList(UCList &l) { // like append, but remove duplicates
		// need to check if elementsizes match...
		_elements.reserve(_elementSize * (_size + l._size));
		for (unsigned int i = 0; i < l._size; i++)
			if (!inList(l[i]))
				append(l[i]);
	}
	void substractList(UCList &l) {
		for (unsigned int i = 0; i < l._size; i++)
			remove(l[i]);
	}

	void free() {
		_elements.clear();
		_size = 0;
	}
	uint32 getSize() const {
		return _size;
	}
	unsigned int getElementSize() const {
		return _elementSize;
	}

	void assign(uint32 index, const uint8 *e) {
		// need to check that index isn't out-of-bounds? (or grow list?)
		for (unsigned int i = 0; i < _elementSize; i++)
			_elements[index * _elementSize + i] = e[i];
	}

	void copyList(UCList &l) { // deep copy for list
		free();
		appendList(l);
	}

	void freeStrings();
	void copyStringList(UCList &l);
	void unionStringList(UCList &l);
	void substractStringList(UCList &l);
	bool stringInList(uint16 str);
	void assignString(uint32 index, uint16 str);
	void removeString(uint16 str, bool nodel = false);

	uint16 getStringIndex(uint32 index);

	void save(ODataSource *ods);
	bool load(IDataSource *ids, uint32 version);

private:
	Std::string &getString(uint32 index);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
