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

#ifndef GOB_HELPER_H
#define GOB_HELPER_H

namespace Gob {

/** A strncpy that forces the final \0. */
inline char *strncpy0(char *dest, const char *src, size_t n) {
	strncpy(dest, src, n);
	dest[n] = 0;
	return dest;
}

/** A "smart" reference counting templated class. */
template<typename T>
class ReferenceCounter {
public:
	class Ptr {
	public:
		bool operator==(const Ptr &p) const { return _p == p._p; }
		bool operator==(const ReferenceCounter *p) const { return _p == p; }

		T *operator-> () { return _p; }
		T &operator* () { return *_p; }
		operator T*() { return _p; }

		Ptr(T *p) : _p(p) { ++_p->_references; }
		Ptr() : _p(0) { }

		~Ptr() {
			if (_p && (--_p->_references == 0))
				delete _p;
		}

		Ptr(const Ptr &p) : _p(p._p) { ++_p->_references; }

		Ptr &operator= (const Ptr &p) {
			++p._p->_references;
			if (_p && (--_p->_references == 0))
				delete _p;
			_p = p._p;
			return *this;
		}
		Ptr *operator= (const Ptr *p) {
			if (p)
				++p->_p->_references;
			if (_p && (--_p->_references == 0))
				delete _p;

			_p = p ? p->_p : 0;
			return this;
		}

	private:
		T *_p;
	};

	ReferenceCounter() : _references(0) { }
	virtual ~ReferenceCounter() {}

private:
	unsigned _references;
	friend class Ptr;
};

} // End of namespace Gob

#endif // GOB_HELPER_H
