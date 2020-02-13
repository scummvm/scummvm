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

#ifndef GLK_QUEST_LIMIT_STACK
#define GLK_QUEST_LIMIT_STACK

namespace Glk {
namespace Quest {

template <class T> class LimitStack {
	uint stack_size, cur_ptr, end_ptr;
	Common::Array<T> data;
	//bool last_push;


	uint dofwd(uint i) const {
		i ++;
		return i == stack_size ? 0 : i;
	}
	uint dobwd(uint i) const {
		return (i == 0 ? stack_size : i) - 1;
	}
	void fwd(uint &i) const {
		i = dofwd(i);
	}
	void bwd(uint &i) const {
		i = dobwd(i);
	}

	/*
	void fwd (uint &i) { i ++; if (i == stack_size) i = 0; }
	void bwd (uint &i) { i = (i == 0 ? stack_size : i) - 1; }
	uint dofwd (uint i) { uint rv = i; fwd(rv); return rv; }
	uint dobwd (uint i) { uint rv = i; bwd(rv); return rv; }
	*/

public:
	LimitStack(uint maxSize) : stack_size(maxSize), cur_ptr(0), end_ptr(maxSize - 1), data(Common::Array<T> (maxSize)) { }

	void push(T &item) {
		if (cur_ptr == end_ptr)
			fwd(end_ptr);
		data[cur_ptr] = item;
		fwd(cur_ptr);
	}

	T &pop() {
		assert(!is_empty());
		bwd(cur_ptr);
		return data[cur_ptr];
	}

	bool is_empty() {
		return dobwd(cur_ptr) == end_ptr;
	}

	uint size() {
		if (cur_ptr > end_ptr)
			return cur_ptr - end_ptr - 1;
		else
			return (cur_ptr + stack_size) - end_ptr - 1;
	}

	void dump(Common::WriteStream &o) {
		o << size() << ": < ";
		for (uint i = dobwd(cur_ptr); i != end_ptr; bwd(i))
			o << data[i] << " ";
		o << ">";
	}

	T &peek() {
		return data[dobwd(cur_ptr)];
	}
};

template<class T> Common::WriteStream &operator<< (Common::WriteStream &o, LimitStack<T> st) {
	st.dump(o);
	return o;
}

} // End of namespace Quest
} // End of namespace Glk

#endif
