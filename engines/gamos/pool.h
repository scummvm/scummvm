/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GAMOS_POOL_H
#define GAMOS_POOL_H

#include "common/array.h"

namespace Gamos {

template<class T, int shift = 6>
class Pool {
public:
	typedef uint size_type; /*!< Size type of the array. */

	const size_type _blockSize = (1 << shift);
	const size_type _blockMask = _blockSize - 1;

public:

	constexpr Pool() : _size(0) {}

	explicit Pool(size_type count) : _size(count) {
		alloc(count);

		size_type n = _size;
		for (T * blk : _blocks) {
			for (size_type i = 0; n > 0 && i < _blockSize; i++) {
				n--;
				new (blk + i) T();
			}
		}
	}

	~Pool() {
		free();
	}

	template<class... TArgs>
	void emplace_back(TArgs &&...args) {
		alloc(_size + 1);

		const size_type blid = _size >> shift;
		const size_type elid = _size & _blockMask;

		new (_blocks[blid] + elid)T(Common::forward<TArgs>(args)...);

		_size++;
	}

	void push_back(const T &element) {
		emplace_back(element);
	}

	void push_back(T &&element) {
		emplace_back(Common::move(element));
	}

	T &operator[](size_type idx) {
		assert(idx < _size);

		const size_type blid = idx >> shift;
		const size_type elid = idx & _blockMask;

		return _blocks[blid][elid];
	}

	const T &operator[](size_type idx) const {
		assert(idx < _size);

		const size_type blid = idx >> shift;
		const size_type elid = idx & _blockMask;

		return _blocks[blid][elid];
	}

	size_type size() const {
		return _size;
	}

	bool empty() const {
		return (_size == 0);
	}

	void clear() {
		free();
	}

	T &front() {
		assert(_size > 0);
		return _blocks[0][0];
	}

	const T &front() const {
		assert(_size > 0);
		return _blocks[0][0];
	}

	T &back() {
		assert(_size > 0);
		return operator[](_size - 1);
	}

	const T &back() const {
		assert(_size > 0);
		return operator[](_size - 1);
	}

protected:
	void alloc(size_type count) {
		size_type blockCount = (count + _blockSize - 1) >> shift;
		if (_blocks.size() < blockCount) {
			size_type oldsz = _blocks.size();
			_blocks.resize(blockCount);
			for (size_type i = oldsz; i < blockCount; i++) {
				_blocks[i] = (T*)malloc(sizeof(T) * _blockSize);
			}
		}
	}

	void free() {
		size_type n = _size;
		for (T * blk : _blocks) {
			for (size_type i = 0; n > 0 && i < _blockSize; i++) {
				n--;
				blk[i].~T();
			}
			::free(blk);
		}
		_blocks.clear();

		_size = 0;
	}

protected:
	Common::Array<T *> _blocks;
	size_type _size = 0;
};

}

#endif
