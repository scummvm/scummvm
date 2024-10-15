/**
 * @file
 */

#pragma once

#include "common/util.h"
#include <stddef.h>

namespace TwinE {

/**
 * @brief Non allocating buffer class holds a maximum of given entries and allows an endless insert
 * by overrinding previous elements in the buffer
 */
template<typename TYPE, size_t SIZE = 64u>
class RingBuffer {
protected:
	size_t _size;
	size_t _front;
	size_t _back;
	TYPE _buffer[SIZE];

public:
	using value_type = TYPE;

	RingBuffer() : _size(0u), _front(0u), _back(SIZE - 1u) {
	}

	class iterator {
	private:
		RingBuffer *_ringBuffer;
		size_t _idx;

	public:
		iterator(RingBuffer *ringBuffer, size_t idx) : _ringBuffer(ringBuffer), _idx(idx) {
		}

		inline const TYPE &operator*() const {
			return _ringBuffer->_buffer[_idx % _ringBuffer->capacity()];
		}

		inline TYPE &operator*() {
			return _ringBuffer->_buffer[_idx % _ringBuffer->capacity()];
		}

		inline const TYPE &operator()() const {
			return _ringBuffer->_buffer[_idx % _ringBuffer->capacity()];
		}

		inline TYPE &operator()() {
			return _ringBuffer->_buffer[_idx % _ringBuffer->capacity()];
		}

		iterator &operator++() {
			++_idx;
			return *this;
		}

		inline bool operator!=(const iterator &rhs) const {
			return _ringBuffer != rhs._ringBuffer || _idx != rhs._idx;
		}

		inline bool operator==(const iterator &rhs) const {
			return _ringBuffer == rhs._ringBuffer && _idx == rhs._idx;
		}

		inline const TYPE *operator->() const {
			return &_ringBuffer->_buffer[_idx % _ringBuffer->capacity()];
		}
	};

	iterator begin() {
		return iterator(this, _front);
	}

	iterator end() {
		return iterator(this, _front + _size);
	}

	iterator begin() const {
		return iterator(const_cast<RingBuffer *>(this), _front);
	}

	iterator end() const {
		return iterator(const_cast<RingBuffer *>(this), _front + _size);
	}

	inline size_t size() const {
		return _size;
	}

	constexpr size_t capacity() const {
		return SIZE;
	}

	inline bool empty() const {
		return _size == 0;
	}

	/**
	 * @brief Access to the first element in the buffer
	 * @note This is not the same as accessing the index 0 element
	 */
	const TYPE &front() const {
		return _buffer[_front];
	}

	/**
	 * @brief Access to the first element in the buffer
	 * @note This is not the same as accessing the index 0 element
	 */
	TYPE &front() {
		return _buffer[_front];
	}

	/**
	 * @brief Access to the last element in the buffer
	 * @note This is not the same as accessing the index @c size-1 element
	 */
	const TYPE &back() const {
		return _buffer[_back];
	}

	/**
	 * @brief Access to the last element in the buffer
	 * @note This is not the same as accessing the index @c size-1 element
	 */
	TYPE &back() {
		return _buffer[_back];
	}

	/**
	 * @brief Clears the whole ring buffer
	 * @note Does not call any destructors - they are called when the buffer itself gets destroyed
	 */
	void clear() {
		_front = 0u;
		_back = SIZE - 1u;
		_size = 0u;
	}

	/**
	 * @brief Pushes an element to the end of the buffer
	 */
	void push_back(const TYPE &x) {
		_back = (_back + 1u) % SIZE;
		if (_size == SIZE) {
			_front = (_front + 1u) % SIZE;
		} else {
			++_size;
		}
		_buffer[_back] = x;
	}

	/**
	 * @brief Pushes an elements to the end of the buffer
	 * @note Performs a move operation
	 */
	template<typename... _Args>
	void emplace_back(_Args &&...args) {
		_back = (_back + 1u) % SIZE;
		if (_size == SIZE) {
			_front = (_front + 1u) % SIZE;
		} else {
			++_size;
		}
		_buffer[_back] = Common::move(TYPE{Common::forward<_Args>(args)...});
	}

	/**
	 * @brief Removes element from the beginning of the buffer
	 */
	void pop() {
		if (_size == 0) {
			return;
		}
		--_size;
		_front = (_front + 1u) % SIZE;
	}

	/**
	 * @brief Erase the given amount of elements from the end of the buffer
	 */
	void erase_back(const size_t n) {
		if (n >= _size) {
			clear();
			return;
		}
		_size -= n;
		_back = (_front + _size - 1u) % SIZE;
	}

	/**
	 * @brief Erase the given amount of elements from the beginning of the buffer
	 */
	void erase_front(const size_t n) {
		if (n >= _size) {
			clear();
			return;
		}
		_front = (_front + n) % SIZE;
		_size -= n;
	}

	inline TYPE &operator[](size_t i) {
		return _buffer[(_front + i) % SIZE];
	}

	inline const TYPE &operator[](size_t i) const {
		return _buffer[(_front + i) % SIZE];
	}
};

} // namespace TwinE
