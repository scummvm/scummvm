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
 */

#ifndef DREAMGEN_SEGMENT_H
#define DREAMGEN_SEGMENT_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/hashmap.h"
#include "common/list.h"

namespace DreamGen {

class WordRef {
	uint8		*_data;
	unsigned	_index;
	uint16		_value;

public:
	inline WordRef(Common::Array<uint8> &data, unsigned index) : _data(data.begin() + index), _index(index) {
		assert(index + 1 < data.size());
		_value = _data[0] | (_data[1] << 8);
	}

	inline WordRef& operator=(const WordRef &ref) {
		_value = ref._value;
		return *this;
	}

	inline WordRef& operator=(uint16 v) {
		_value = v;
		return *this;
	}

	inline operator uint16&() {
		return _value;
	}

	inline ~WordRef() {
		// FIXME: This is _too late_ to write back the
		// value. Example: in the call
		// printDirect(data.word(kLastxpos), .....)
		// the destructor isn't called until after printDirect returns. This
		// destroys the modifications to lastXPos that printDirect makes.
		_data[0] = _value & 0xff;
		_data[1] = _value >> 8;
		_value = _data[0] | (_data[1] << 8);
	}
};

class Segment {
	Common::Array<uint8> data;

public:
	Segment(uint size = 0) {
		if (size > 0)
			data.resize(size);
	}

	inline void assign(const uint8 *b, const uint8 *e) {
		data.assign(b, e);
	}

	inline uint8 &byte(unsigned index) {
		assert(index < data.size());
		return data[index];
	}

	inline WordRef word(unsigned index) {
		return WordRef(data, index);
	}

	inline uint8 *ptr(unsigned index, unsigned size) {
		assert(index + size <= data.size());
		return data.begin() + index;
	}
};

typedef Common::SharedPtr<Segment> SegmentPtr;

class SegmentRef {
	uint16		_value;
	SegmentPtr	_segment;

public:
	SegmentRef(uint16 value = 0, SegmentPtr segment = SegmentPtr())
	: _value(value), _segment(segment) {
	}

	inline operator uint16() const {
		return _value;
	}

	SegmentPtr getSegmentPtr() const {
		return _segment;
	}

	inline uint8 &byte(unsigned index) {
		assert(_segment != 0);
		return _segment->byte(index);
	}

	inline WordRef word(unsigned index) {
		//debug(1, "getting word ref for %04x:%d", _value, index);
		assert(_segment != 0);
		return _segment->word(index);
	}

	inline void assign(const uint8 *b, const uint8 *e) {
		assert(_segment != 0);
		_segment->assign(b, e);
	}

	inline uint8 *ptr(unsigned index, unsigned size) {
		assert(_segment != 0);
		return _segment->ptr(index, size);
	}

protected:
	SegmentRef &operator=(const SegmentRef &seg) {
		_value = seg._value;
		_segment = seg._segment;
		return *this;
	}

};

class SegmentManager;

class MutableSegmentRef : public SegmentRef {
protected:
	SegmentManager		*_segMan;

public:
	MutableSegmentRef(SegmentManager *segMan, uint16 value = 0, SegmentPtr segment = SegmentPtr())
	: _segMan(segMan), SegmentRef(value, segment) {
	}

	MutableSegmentRef(SegmentManager *segMan, SegmentRef seg)
	: _segMan(segMan), SegmentRef(seg) {
	}

	inline MutableSegmentRef& operator=(const uint16 id);

};


class SegmentManager {
private:
	typedef Common::HashMap<uint16, SegmentPtr> SegmentMap;
	SegmentMap _segments;

	typedef Common::List<uint16> FreeSegmentList;
	FreeSegmentList _freeSegments;

	enum { kDefaultDataSegment = 0x1000 };

public:

	SegmentPtr _realData;	///< the primary data segment, points to a huge blob of binary data
	SegmentRef data;	///< fake segment register always pointing to data segment

public:
	SegmentManager() :
		_realData(new Segment()),
		data(kDefaultDataSegment, _realData) {

		_segments[kDefaultDataSegment] = data.getSegmentPtr();
	}

	SegmentRef getSegment(uint16 value) {
		SegmentMap::iterator i = _segments.find(value);
		if (i != _segments.end())
			return SegmentRef(value, i->_value);
		else
			return SegmentRef(value);
	}

	SegmentRef allocateSegment(uint size) {
		unsigned id;
		if (_freeSegments.empty())
			id = kDefaultDataSegment + _segments.size();
		else {
			id = _freeSegments.front();
			_freeSegments.pop_front();
		}
		assert(!_segments.contains(id));
		SegmentPtr seg(new Segment(size));
		_segments[id] = seg;
		return SegmentRef(id, seg);
	}

	void deallocateSegment(uint16 id) {
		SegmentMap::iterator i = _segments.find(id);
		if(i != _segments.end()) {
			_segments.erase(i);
			_freeSegments.push_back(id);
		} else {
			debug("Deallocating non existent segment! Client code should be fixed.");
		}
	}

};


inline MutableSegmentRef& MutableSegmentRef::operator=(const uint16 id) {
	SegmentRef::operator=(_segMan->getSegment(id));
	return *this;
}

} // End of namespace DreamGen

#endif
