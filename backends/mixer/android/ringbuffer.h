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

#ifndef _BACKENDS_MIXER_ANDROID_RINGBUFFER_H_
#define _BACKENDS_MIXER_ANDROID_RINGBUFFER_H_

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <new>
#include <atomic>

/**
 * A lock-free FIFO ring-buffer with contiguous buffers for production.
 */
template<typename T>
class RingBuffer {
public:
	RingBuffer(size_t n) : _size(n + 1), _buffer(new T[_size]), _pending_read(0), _pending_write(0), _read(0), _write(0), _last(0) { }
	RingBuffer(const RingBuffer<T> &) = delete;

	/*
	 * Contruct a new RingBuffer moving data from the old one
	 * The other RingBuffer must not be in use and destroyed afterwards.
	 *
	 * When the new RingBuffer is smaller than the previous one, the older samples are dropped.
	 */
	RingBuffer(size_t n, RingBuffer<T> &&o) : RingBuffer(n) {
		// First make the RingBuffer was in a valid state and make it invalid
		const size_t write = o._write.exchange(-1);
		const size_t read = o._read.exchange(-1);
		assert(o._pending_write == write);
		o._pending_write = 0;
		assert(o._pending_read == read);
		o._pending_read = -1;

		T const *buffer = o._buffer;
		o._buffer = nullptr;

		// From here, o is completely invalid

		if (read == write) {
			// Empty queue: nothing to move
			delete[] buffer;
			return;
		}
		if (read < write) {
			// Cap the kept data to our own buffer size
			size_t nread = read;
			if (nread + n < write) {
				nread = write - n;
			}
			_pending_write = write - nread;

			memcpy(&_buffer[0], &buffer[nread], _pending_write * sizeof(T));

			_last.store(_pending_write, std::memory_order_relaxed);
			_write.store(_pending_write, std::memory_order_release);
			delete[] buffer;
			return;
		}

		// read > write: the buffer is in two parts
		if (n <= write) {
			// Easy: we can take the last n samples in one shot
			_pending_write = n;
			memcpy(&_buffer[0], &buffer[write - n], n * sizeof(T));

			_last.store(_pending_write, std::memory_order_relaxed);
			_write.store(_pending_write, std::memory_order_release);

			delete[] buffer;
			return;
		}

		// n > write
		size_t last = o._last.load(std::memory_order_relaxed);
		size_t end_part_sz = last - read;
		if (end_part_sz > (n - write)) {
			end_part_sz = n - write;
		}

		// First, copy the end of the buffer up to last, then copy the whole beginning
		_pending_write = end_part_sz + write;
		memcpy(&_buffer[0], &buffer[last - end_part_sz], end_part_sz * sizeof(T));
		memcpy(&_buffer[end_part_sz], &buffer[0], write * sizeof(T));

		_last.store(_pending_write, std::memory_order_relaxed);
		_write.store(_pending_write, std::memory_order_release);

		delete[] buffer;
	}

	~RingBuffer() { delete[] _buffer; }

	/*
	 * Try to produce at least n elements.
	 * The ring-buffer will adjust n with the real element count
	 * which should be produced.
	 * In case of failure, nullptr is returned.
	 *
	 * When successful, n is guaranteed to be at least what has been queried.
	 * A pointer to the buffer to fill is returned.
	 */
	T *try_produce(size_t *n) {
		size_t real_n = *n;
		assert(real_n > 0);

		size_t write = _write.load(std::memory_order_relaxed);
		size_t read = _read.load(std::memory_order_acquire);
		assert(_pending_write == write);

		// Try to acquire at at least real_n records
		if (read <= write) {
			if (write + real_n <= _size) {
				real_n = _size - write;
				*n = real_n;
				_wraparound_write = false;
				_pending_write = write + real_n;
				return &_buffer[write];
			} else if (real_n < read) { // Don't go up to read: that would make believe it's empty
				real_n = read - 1;
				*n = real_n;
				_wraparound_write = true;
				_pending_write = real_n;
				return &_buffer[0];
			} else {
				return nullptr;
			}
		} else {
			if (write + real_n < read) { // Don't go up to read: that would make believe it's empty
				real_n = read - write - 1;
				*n = real_n;
				_wraparound_write = false;
				_pending_write = write + real_n;
				return &_buffer[write];
			} else {
				return nullptr;
			}
		}
	}

	/*
	 * Indicate that n samples have been produced.
	 * n must be less than or equal to what have been returned by try_produce.
	 */
	void produced(size_t n) {
		size_t write = _write.load(std::memory_order_relaxed);
		size_t pending_write;
		if (_wraparound_write) {
			pending_write = n;
			_last.store(write, std::memory_order_relaxed);
		} else {
			pending_write = write + n;
		}
		// Make sure we didn't overshoot
		assert(_pending_write >= pending_write);
		if (pending_write > _last.load(std::memory_order_relaxed)) {
			_last.store(pending_write, std::memory_order_relaxed);
		}
		_pending_write = pending_write;
		_write.store(pending_write, std::memory_order_release);
	}

	/*
	 * Try to consume at most n elements.
	 * If there is less than n elements (or if the buffer is not contiguous), adjusts n to the real count.
	 * If there is no element available, returns nullptr.
	 *
	 * Loop over try_consume until it returns nullptr to fetch all the expected elements.
	 */
	T *try_consume(size_t *n) {
		size_t real_n = *n;
		assert(real_n > 0);

		size_t read = _read.load(std::memory_order_relaxed);
		assert(_pending_read == read);

		// Try to acquire at most n records
		size_t write = _write.load(std::memory_order_acquire);

		if (read == write) {
			// Empty queue: nothing to return
			return nullptr;
		} else if (read < write) {
			if (read + real_n > write) {
				real_n = write - read;
			}
			*n = real_n;
			_pending_read = read + real_n;
			return &_buffer[read];
		} else {
			size_t last = _last.load(std::memory_order_relaxed);
			if (read == last) { // This happens when we read up to the end the last time: consider read as 0
				if (0 == write) {
					// Empty queue: nothing to return
					return nullptr;
				}
				if (real_n > write) {
					real_n = write;
				}
				*n = real_n;
				_pending_read = real_n;
				return &_buffer[0];
			} else if (read + real_n < last) {
				*n = real_n;
				_pending_read =  read + real_n;
				return &_buffer[read];
			} else {
				*n = last - read;
				_pending_read = 0;
				return &_buffer[read];
			}
		}
	}

	/*
	 * Indicate that the previous consume request has been done.
	 *
	 * Frees the buffer for more produced samples.
	 */
	void consumed() {
		_read.store(_pending_read, std::memory_order_release);
	}

private:
	const size_t _size;
	T *_buffer;

	size_t _pending_read;
	size_t _pending_write;
	bool   _wraparound_write;

	// LLVM between 8 and 10 wrongfully said they supported this
#if 0 && __cpp_lib_hardware_interference_size
	static constexpr size_t hardware_destructive_interference_size = std::hardware_destructive_interference_size;
#else
	// 64 is a good fit for most platforms
	static constexpr size_t hardware_destructive_interference_size = 64;
#endif

	alignas(hardware_destructive_interference_size) std::atomic<size_t> _read;
	alignas(hardware_destructive_interference_size) std::atomic<size_t> _write;
	alignas(hardware_destructive_interference_size) std::atomic<size_t> _last;
};

#endif
