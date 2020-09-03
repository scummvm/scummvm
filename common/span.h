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

#ifndef COMMON_SPAN_H
#define COMMON_SPAN_H

#include "common/file.h"
#include "common/memstream.h"
#include "common/safe-bool.h"
#include "common/scummsys.h"
#include "common/type-traits.h"

namespace Common {

#define COMMON_SPAN_TYPEDEFS \
	typedef typename super_type::value_type value_type; \
	typedef typename super_type::difference_type difference_type; \
	typedef typename super_type::index_type index_type; \
	typedef typename super_type::size_type size_type; \
	typedef typename super_type::const_iterator const_iterator; \
	typedef typename super_type::iterator iterator; \
	typedef typename super_type::pointer pointer; \
	typedef typename super_type::const_pointer const_pointer; \
	typedef typename super_type::reference reference; \
	typedef typename super_type::const_reference const_reference;

enum {
	kSpanMaxSize = 0xFFFFFFFF,
	kSpanKeepOffset = 0xFFFFFFFF
};

#pragma mark -
#pragma mark SpanValidationMode

enum SpanValidationMode {
	kValidateRead,
	kValidateWrite,
	kValidateSeek
};

namespace SpanInternal {
#pragma mark -
#pragma mark SpanIterator
	/**
	 * Bounds-checked iteration over a span of memory.
	 */
	template <typename Span, bool IsConst>
	class SpanIterator {
		typedef typename Span::value_type span_value_type;
		typedef typename Conditional<IsConst, const Span, Span>::type span_type;

	public:
		typedef typename Span::difference_type difference_type;
		typedef typename RemoveConst<span_value_type>::type value_type;
		typedef typename Conditional<IsConst, const span_value_type, span_value_type>::type *pointer;
		typedef typename Conditional<IsConst, const span_value_type, span_value_type>::type &reference;

		inline SpanIterator() : _span(nullptr), _index(0) {}

		inline SpanIterator(span_type *const span, const difference_type index) :
			_span(span),
			_index(index) {
			if (span != nullptr) {
				span->validate(index, 0, kValidateSeek);
			}
		}

		inline SpanIterator(const SpanIterator &other) :
			_span(other._span),
			_index(other._index) {}

		inline SpanIterator &operator=(const SpanIterator &other) {
			_span = other._span;
			_index = other._index;
			return *this;
		}

#pragma mark -
#pragma mark SpanIterator - Dereferencing operations

	public:
		inline reference operator*() const {
			// validation is in Span::operator[]
			return (*_span)[_index];
		}

		inline pointer operator->() const {
			return &operator*();
		}

		inline reference operator[](const difference_type index) const {
			// validation is in SpanIterator::operator+=
			return *(*this + index);
		}

#pragma mark -
#pragma mark SpanIterator - Arithmetic operations

	public:
		inline SpanIterator &operator+=(const difference_type delta) {
			assert(_span != nullptr);
			_span->validate(_index, delta, kValidateSeek);
			_index += delta;
			return *this;
		}

		inline SpanIterator &operator-=(const difference_type delta) {
			return operator+=(-delta);
		}

		inline SpanIterator &operator++() {
			return operator+=(1);
		}

		inline SpanIterator operator++(int) {
			SpanIterator old(*this);
			operator+=(1);
			return old;
		}

		inline SpanIterator &operator--() {
			return operator+=(-1);
		}

		inline SpanIterator operator--(int) {
			SpanIterator old(*this);
			operator+=(-1);
			return old;
		}

		inline SpanIterator operator+(const difference_type delta) const {
			SpanIterator it(*this);
			return it += delta;
		}

		inline SpanIterator operator-(const difference_type delta) const {
			return operator+(-delta);
		}

		inline difference_type operator-(const SpanIterator &other) const {
			assert(_span == other._span);
			return _index - other._index;
		}

#pragma mark -
#pragma mark SpanIterator - Comparison operations

	public:
		inline bool operator==(const SpanIterator& other) const {
			return _span == other._span && _index == other._index;
		}

		inline bool operator!=(const SpanIterator& other) const {
			return !operator==(other);
		}

		inline bool operator<(const SpanIterator& other) const {
			assert(_span == other._span);
			return _index < other._index;
		}

		inline bool operator<=(const SpanIterator& other) const {
			return !other.operator<(*this);
		}

		inline bool operator>(const SpanIterator& other) const {
			return other.operator<(*this);
		}

		inline bool operator>=(const SpanIterator& other) const {
			return !operator<(other);
		}

#pragma mark -
#pragma mark SpanIterator - Data access convenience functions

	public:
		inline int8 getInt8() const {
			return _span->getInt8At(_index);
		}

		inline uint8 getUint8() const {
			return _span->getUint8At(_index);
		}

		inline int16 getInt16BE() const {
			return _span->getInt16BEAt(_index);
		}

		inline int16 getInt16LE() const {
			return _span->getInt16LEAt(_index);
		}

		inline uint16 getUint16BE() const {
			return _span->getUint16BEAt(_index);
		}

		inline uint16 getUint16LE() const {
			return _span->getUint16LEAt(_index);
		}

		inline uint32 getUint24LE() const {
			return _span->getUint24LEAt(_index);
		}

		inline uint32 getUint32() const {
			return _span->getUint32At(_index);
		}

		inline int32 getInt32BE() const {
			return _span->getInt32BEAt(_index);
		}

		inline int32 getInt32LE() const {
			return _span->getInt32LEAt(_index);
		}

		inline uint32 getUint32BE() const {
			return _span->getUint32BEAt(_index);
		}

		inline uint32 getUint32LE() const {
			return _span->getUint32LEAt(_index);
		}

	protected:
		span_type *_span;
		difference_type _index;
	};
} // End of namespace SpanInternal

#pragma mark -
#pragma mark SpanBase

template <typename ValueType, template <typename> class Derived>
class SpanBase : public SafeBool<Derived<ValueType> > {
	typedef Derived<ValueType> derived_type;
	typedef typename AddConst<derived_type>::type const_derived_type;
	typedef typename RemoveConst<derived_type>::type mutable_derived_type;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T, bool U> friend class SpanInternal::SpanIterator;
	template <typename T, template <typename> class U> friend class SpanBase;
	template <typename T, typename U> friend struct SafeBool;
#endif
#ifdef CXXTEST_RUNNING
	friend class ::SpanTestSuite;
#endif

public:
	typedef ValueType value_type;
	typedef int32 difference_type;
	typedef uint32 index_type;
	typedef uint32 size_type;
	typedef SpanInternal::SpanIterator<derived_type, true> const_iterator;
	typedef SpanInternal::SpanIterator<derived_type, false> iterator;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type &reference;
	typedef const value_type &const_reference;

	inline size_type byteSize() const { return impl().size() * sizeof(value_type); }

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	inline SpanBase() {}
	inline SpanBase(const SpanBase &) {}
	inline SpanBase &operator=(const SpanBase &) { return this->impl(); }
	inline ~SpanBase() {}

	inline const_derived_type &impl() const { return static_cast<const_derived_type &>(*this); }
	inline mutable_derived_type &impl() { return static_cast<mutable_derived_type &>(*this); }

#pragma mark -
#pragma mark SpanBase - Interface

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	inline void clear();

	inline size_type size() const;

	inline const_iterator cbegin() const;
	inline const_iterator cend() const;
	inline const_iterator begin() const;
	inline const_iterator end() const;
	inline iterator begin();
	inline iterator end();

	inline pointer data() const;

#pragma mark -
#pragma mark SpanBase - Data access functions

public:
	inline const_reference operator[](const index_type index) const {
		impl().validate(index, sizeof(value_type));
		return impl().data()[index];
	}

	inline reference operator[](const index_type index) {
		impl().validate(index, sizeof(value_type));
		return impl().data()[index];
	}

	inline int8 getInt8At(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) == sizeof(uint8), int8_can_only_be_read_from_byte_or_char_spans);
		return (int8)getUint8At(index);
	}

	inline uint8 getUint8At(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) == sizeof(uint8), uint8_can_only_be_read_from_byte_or_char_spans);
		impl().validate(index, sizeof(uint8));
		return (uint8)impl().data()[index];
	}

	inline int16 getInt16BEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint16), int16_can_only_be_read_from_int16_or_smaller_spans);
		return (int16)impl().getUint16BEAt(index);
	}

	inline int16 getInt16LEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint16), int16_can_only_be_read_from_int16_or_smaller_spans);
		return (int16)impl().getUint16LEAt(index);
	}

	inline uint16 getUint16BEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint16), uint16_can_only_be_read_from_int16_or_smaller_spans);
		impl().validate(index, sizeof(uint16));
		return READ_BE_UINT16(impl().data() + index);
	}

	inline uint16 getUint16LEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint16), uint16_can_only_be_read_from_int16_or_smaller_spans);
		impl().validate(index, sizeof(uint16));
		return READ_LE_UINT16(impl().data() + index);
	}

	inline uint32 getUint24LEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= 3, uint24_can_only_be_read_from_int24_or_smaller_spans);
		impl().validate(index, 3);
		return READ_LE_UINT24(impl().data() + index);
	}

	inline uint32 getUint32At(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint32), uint32_can_only_be_read_from_int32_or_smaller_spans);
		impl().validate(index, sizeof(uint32));
		return READ_UINT32(impl().data() + index);
	}

	inline int32 getInt32BEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint32), int32_can_only_be_read_from_int32_or_smaller_spans);
		return (int32)impl().getUint32BEAt(index);
	}

	inline int32 getInt32LEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint32), int32_can_only_be_read_from_int32_or_smaller_spans);
		return (int32)impl().getUint32LEAt(index);
	}

	inline uint32 getUint32BEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint32), uint32_can_only_be_read_from_int32_or_smaller_spans);
		impl().validate(index, sizeof(uint32));
		return READ_BE_UINT32(impl().data() + index);
	}

	inline uint32 getUint32LEAt(const index_type index) const {
		STATIC_ASSERT(sizeof(value_type) <= sizeof(uint32), uint32_can_only_be_read_from_int32_or_smaller_spans);
		impl().validate(index, sizeof(uint32));
		return READ_LE_UINT32(impl().data() + index);
	}

	inline String getStringAt(const index_type index, size_type numEntries = kSpanMaxSize) const {
		STATIC_ASSERT(sizeof(value_type) == sizeof(char), strings_can_only_be_read_from_byte_or_char_spans);
		const char *string = (const char *)impl().data() + index;

		if (numEntries == kSpanMaxSize) {
			numEntries = strnlen(string, impl().size() - index);
		}

		impl().validate(index, numEntries);
		return String(string, numEntries);
	}

	/**
	 * Returns a raw pointer to memory after validating the given index and
	 * size. Use this only in performance-critical code, like processing pixel
	 * data in a loop, where validating each read independently would introduce
	 * unnecessary overhead.
	 */
	inline const_pointer getUnsafeDataAt(const index_type index, size_type numEntries = kSpanMaxSize) const {
		if (numEntries == kSpanMaxSize) {
			numEntries = impl().size() - index;
		}

		impl().validate(index, numEntries * sizeof(value_type));
		return impl().data() + index;
	}

	inline pointer getUnsafeDataAt(const index_type index, size_type numEntries = kSpanMaxSize) {
		if (numEntries == kSpanMaxSize) {
			numEntries = impl().size() - index;
		}

		impl().validate(index, numEntries * sizeof(value_type));
		return impl().data() + index;
	}

	inline MemoryReadStream toStream(const index_type index = 0, size_type numEntries = kSpanMaxSize) const {
		if (numEntries == kSpanMaxSize) {
			numEntries = impl().size() - index;
		}

		impl().validate(index, numEntries * sizeof(value_type));
		return MemoryReadStream(impl().data() + index, numEntries * sizeof(value_type), DisposeAfterUse::NO);
	}

#pragma mark -
#pragma mark SpanBase - Operators

public:
	template <typename Other>
	inline bool operator==(const Other &other) const {
		return impl().data() == other.impl().data() && impl().size() == other.impl().size();
	}

	template <typename Other>
	inline bool operator!=(const Other &other) const {
		return !operator==(other);
	}

	template <typename Other>
	inline difference_type operator-(const Other &other) const {
		return impl().data() - other.impl().data();
	}

	template <typename Other>
	inline bool operator<(const Other &other) const {
		return impl().data() < other.impl().data();
	}

	template <typename Other>
	inline bool operator<=(const Other &other) const {
		return !other.operator<(*this);
	}

	template <typename Other>
	inline bool operator>(const Other &other) const {
		return other.operator<(*this);
	}

	template <typename Other>
	inline bool operator>=(const Other &other) const {
		return !operator<(other);
	}

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	inline bool operator_bool() const { return impl().data() != nullptr; }

#pragma mark -
#pragma mark SpanBase - Copying

public:
	/**
	 * Copies data from this span to a raw pointer. To only copy a portion of
	 * the span, call subspan first.
	 */
	inline void unsafeCopyDataTo(void *target) const {
		memcpy(target, impl().data(), impl().byteSize());
	}

	/**
	 * Copies the data from this span to the given target span. To only copy a
	 * portion of the span, call subspan first.
	 */
	template <typename Other>
	inline void copyDataTo(Other &target) const {
		assert((impl().byteSize() % sizeof(typename Other::value_type)) == 0);
		target.impl().validate(0, impl().byteSize(), kValidateWrite);
		memcpy(target.impl().data(), impl().data(), impl().byteSize());
	}

#pragma mark -
#pragma mark SpanBase - Validation

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	/**
	 * @returns true if bounds are invalid.
	 */
	inline bool checkInvalidBounds(const index_type index, const difference_type deltaInBytes) const {
		// There is a potential that large bogus values may cause arithmetic
		// overflow, so the individual operands are checked separately first.
		// Values that are not allowed to be negative are treated as unsigned to
		// reduce the number of necessary comparisons
		const size_t maxByteOffset = index * (signed)sizeof(value_type) + deltaInBytes;
		return index > impl().size() || deltaInBytes > (difference_type)impl().byteSize() || maxByteOffset > impl().byteSize();
	}

	inline void validate(const index_type index, const difference_type deltaInBytes, const SpanValidationMode mode = kValidateRead) const {
		/* LCOV_EXCL_START */
		if (impl().checkInvalidBounds(index, deltaInBytes)) {
			error("%s", impl().getValidationMessage(index, deltaInBytes, mode).c_str());
		}
		/* LCOV_EXCL_STOP */
	}
};

#pragma mark -
#pragma mark SpanImpl

template <typename ValueType, template <typename> class Derived>
class SpanImpl : public SpanBase<ValueType, Derived> {
	typedef SpanBase<ValueType, Derived> super_type;
	typedef typename AddConst<Derived<ValueType> >::type const_derived_type;
	typedef typename RemoveConst<Derived<ValueType> >::type mutable_derived_type;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T, template <typename> class U> friend class SpanImpl;
#endif
#ifdef CXXTEST_RUNNING
	friend class ::SpanTestSuite;
#endif

public:
	COMMON_SPAN_TYPEDEFS

	inline SpanImpl() : super_type(), _data(nullptr), _size(0) {}

	inline SpanImpl(const pointer data_, const size_type size_) :
		super_type(),
		_data(data_),
		_size(size_) {}

	template <typename Other>
	inline SpanImpl(const Other &other) :
		super_type(),
		_data(other.data()),
		_size(other.size()) {}

	inline void clear() {
		_data = nullptr;
		_size = 0;
	}

	inline size_type size() const { return _size; }
	inline pointer data() const { return _data; }

	inline const_iterator cbegin() const { return const_iterator(&this->impl(), 0); }
	inline const_iterator cend() const { return const_iterator(&this->impl(), size()); }
	inline const_iterator begin() const { return const_iterator(&this->impl(), 0); }
	inline const_iterator end() const { return const_iterator(&this->impl(), size()); }
	inline iterator begin() { return iterator(&this->impl(), 0); }
	inline iterator end() { return iterator(&this->impl(), size()); }

	const String name() const { return String::format("%p", static_cast<const void *>(data())); }

	String getValidationMessage(const index_type index, const difference_type deltaInBytes, const SpanValidationMode mode) const {
		const char *modeName = "unknown";
		switch (mode) {
			case kValidateRead:
				modeName = "reading";
				break;
			case kValidateWrite:
				modeName = "writing";
				break;
			case kValidateSeek:
				modeName = "seeking";
				break;
			default:
				break;
		}

		return String::format("Access violation %s %s: %u + %d > %u",
							  modeName,
							  this->impl().name().c_str(),
							  index,
							  deltaInBytes / (int)sizeof(value_type),
							  size());
	}

#pragma mark -
#pragma mark SpanImpl - Subspan

public:
	template <typename NewValueType>
	inline const Derived<NewValueType> subspan(const index_type index, size_type numEntries = kSpanMaxSize) const {
		Derived<NewValueType> span;
		populateSubspan(span, index, numEntries);
		return span;
	}

	template <typename NewValueType>
	inline Derived<NewValueType> subspan(const index_type index, size_type numEntries = kSpanMaxSize) {
		Derived<NewValueType> span;
		populateSubspan(span, index, numEntries);
		return span;
	}

	inline const_derived_type subspan(const index_type index, const size_type numEntries = kSpanMaxSize) const {
		return subspan<value_type>(index, numEntries);
	}

	inline mutable_derived_type subspan(const index_type index, const size_type numEntries = kSpanMaxSize) {
		return subspan<value_type>(index, numEntries);
	}

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	template <typename NewValueType>
	void populateSubspan(Derived<NewValueType> &span, const index_type index, size_type numEntries) const {
		if (numEntries == kSpanMaxSize) {
			numEntries = CLIP<size_type>(size() - index, 0, size());
		}

		assert(numEntries * sizeof(value_type) % sizeof(NewValueType) == 0);
		this->validate(index, numEntries * sizeof(value_type), kValidateSeek);

		span._data = (NewValueType *)const_cast<mutable_value_type *>(_data + index);
		span._size = numEntries * sizeof(value_type) / sizeof(NewValueType);
	}

#pragma mark -
#pragma mark SpanImpl - Allocation

private:
	typedef typename RemoveConst<value_type>::type mutable_value_type;
	typedef Derived<mutable_value_type> mutable_value_derived_type;

public:
	mutable_value_derived_type &allocate(const size_type numEntries) {
		assert(_data == nullptr);
		assert(numEntries != kSpanMaxSize);
		_data = new mutable_value_type[numEntries];
		_size = numEntries;
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	template <typename Other>
	mutable_value_derived_type &allocateFromSpan(const Other &other) {
		assert(_data == nullptr);
		assert(sizeof(value_type) == sizeof(typename Other::value_type));
		_data = new mutable_value_type[other.size()];
		_size = other.size();
		copy(other.begin(), other.end(), const_cast<mutable_value_type *>(_data));
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	mutable_value_derived_type &allocateFromStream(SeekableReadStream &stream, size_type numEntries = kSpanMaxSize) {
		if (numEntries == kSpanMaxSize) {
			numEntries = (stream.size() - stream.pos()) / sizeof(value_type);
		}

		const uint32 bytesRequested = numEntries * sizeof(value_type);
		assert(stream.pos() + bytesRequested <= (uint)stream.size());
		allocate(numEntries);
		const uint32 bytesRead = stream.read((void *)const_cast<mutable_value_type *>(_data), bytesRequested);
		assert(bytesRead == bytesRequested);
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	value_type *_data;
	size_type _size;
};

#pragma mark -
#pragma mark Span

template <typename ValueType>
class Span : public SpanImpl<ValueType, Span> {
	typedef SpanImpl<ValueType, ::Common::Span> super_type;
	typedef typename AddConst<Span<ValueType> >::type const_derived_type;
	typedef typename RemoveConst<Span<ValueType> >::type mutable_derived_type;
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T> friend class Span;
#endif

public:
	COMMON_SPAN_TYPEDEFS

	inline Span() : super_type() {}

	inline Span(const pointer data_, const size_type size_) : super_type(data_, size_) {}

	// Allows unrelated sibling classes like NamedSpan to assign to superclass
	// siblings like Span
	template <typename Other>
	inline Span(const Other &other) : super_type(other) {}
};

#pragma mark -
#pragma mark NamedSpanImpl

template <typename ValueType, template <typename> class Derived>
class NamedSpanImpl : public SpanImpl<ValueType, Derived> {
	typedef SpanImpl<ValueType, Derived> super_type;
	typedef typename AddConst<Derived<ValueType> >::type const_derived_type;
	typedef typename RemoveConst<Derived<ValueType> >::type mutable_derived_type;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T, template <typename> class U> friend class NamedSpanImpl;
#endif
#ifdef CXXTEST_RUNNING
	friend class ::SpanTestSuite;
#endif

public:
	COMMON_SPAN_TYPEDEFS

	inline NamedSpanImpl() : super_type(), _name(), _sourceByteOffset(0) {}

	inline NamedSpanImpl(const pointer data_,
						 const size_type size_,
						 const String &name_ = String(),
						 const size_type sourceByteOffset_ = 0) :
		super_type(data_, size_),
		_name(name_),
		_sourceByteOffset(sourceByteOffset_) {}

	template <typename Other>
	inline NamedSpanImpl(const Other &other) :
		super_type(other),
		_name(other.name()),
		_sourceByteOffset(other.sourceByteOffset()) {}

	inline void clear() {
		super_type::clear();
		_name.clear();
		_sourceByteOffset = 0;
	}

	const String &name() const { return _name; }
	String &name() { return _name; }

	const size_type &sourceByteOffset() const { return _sourceByteOffset; }
	size_type &sourceByteOffset() { return _sourceByteOffset; }

private:
	String _name;
	size_type _sourceByteOffset;

#pragma mark -
#pragma mark NamedSpanImpl - Subspan

public:
	template <typename NewValueType>
	inline const Derived<NewValueType> subspan(const index_type index, const size_type numEntries = kSpanMaxSize, const String &name_ = String(), const size_type sourceByteOffset_ = kSpanKeepOffset) const {
		Derived<NewValueType> span;
		populateSubspan(span, index, numEntries, name_, sourceByteOffset_);
		return span;
	}

	template <typename NewValueType>
	inline Derived<NewValueType> subspan(const index_type index, const size_type numEntries = kSpanMaxSize, const String &name_ = String(), const size_type sourceByteOffset_ = kSpanKeepOffset) {
		Derived<NewValueType> span;
		populateSubspan(span, index, numEntries, name_, sourceByteOffset_);
		return span;
	}

	inline const_derived_type subspan(const index_type index, const size_type numEntries = kSpanMaxSize, const String &name_ = String(), const size_type sourceByteOffset_ = kSpanKeepOffset) const {
		return subspan<value_type>(index, numEntries, name_, sourceByteOffset_);
	}

	inline mutable_derived_type subspan(const index_type index, const size_type numEntries = kSpanMaxSize, const String &name_ = String(), const size_type sourceByteOffset_ = kSpanKeepOffset) {
		return subspan<value_type>(index, numEntries, name_, sourceByteOffset_);
	}

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	template <typename NewValueType>
	void populateSubspan(Derived<NewValueType> &span, const index_type index, size_type numEntries, const String &name_, const size_type sourceByteOffset_ = kSpanKeepOffset) const {
		super_type::template populateSubspan<NewValueType>(span, index, numEntries);

		if (name_.empty()) {
			span._name = _name;
		} else {
			span._name = name_;
		}

		if (sourceByteOffset_ == kSpanKeepOffset) {
			span._sourceByteOffset = _sourceByteOffset + index * sizeof(value_type);
		} else {
			span._sourceByteOffset = sourceByteOffset_;
		}
	}

#pragma mark -
#pragma mark NamedSpanImpl - Validation

public:
	String getValidationMessage(const index_type index, const difference_type deltaInBytes, const SpanValidationMode mode) const {
		const index_type indexInBytes = index * sizeof(value_type);
		const size_type maxSizeInBytes = this->impl().byteSize();

		return super_type::getValidationMessage(index, deltaInBytes, mode) +
			String::format(" (abs: %u + %d > %u)",
						   this->impl().sourceByteOffset() + indexInBytes,
						   deltaInBytes,
						   this->impl().sourceByteOffset() + maxSizeInBytes);
	}

#pragma mark -
#pragma mark NamedSpanImpl - Allocation

private:
	typedef typename RemoveConst<value_type>::type mutable_value_type;
	typedef Derived<mutable_value_type> mutable_value_derived_type;

public:
	mutable_value_derived_type &allocate(const size_type numEntries, const String &name_ = String()) {
		super_type::allocate(numEntries);
		_name = name_;
		_sourceByteOffset = 0;
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	template <typename OtherValueType>
	mutable_value_derived_type &allocateFromSpan(const NamedSpanImpl<OtherValueType, Derived> &other) {
		super_type::allocateFromSpan(other);
		_name = other.name();
		_sourceByteOffset = other.sourceByteOffset();
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	template <typename OtherValueType, template <typename> class OtherDerived>
	mutable_value_derived_type &allocateFromSpan(const SpanImpl<OtherValueType, OtherDerived> &other) {
		super_type::allocateFromSpan(other);
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	mutable_value_derived_type &allocateFromStream(SeekableReadStream &stream, size_type numEntries = kSpanMaxSize, const String &name_ = String()) {
		super_type::allocateFromStream(stream, numEntries);
		_name = name_;
		_sourceByteOffset = 0;
		return (mutable_value_derived_type &)const_cast<Derived<value_type> &>(this->impl());
	}

	mutable_value_derived_type &allocateFromStream(File &file, const size_type numEntries = kSpanMaxSize) {
		return allocateFromStream(file, numEntries, file.getName());
	}
};

#pragma mark -
#pragma mark NamedSpan

template <typename ValueType>
class NamedSpan : public NamedSpanImpl<ValueType, NamedSpan> {
	typedef NamedSpanImpl<ValueType, ::Common::NamedSpan> super_type;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T> friend class NamedSpan;
#endif

public:
	COMMON_SPAN_TYPEDEFS

	inline NamedSpan() : super_type() {}

	inline NamedSpan(const pointer data_,
					 const size_type size_,
					 const String &name_ = String(),
					 const size_type sourceByteOffset_ = 0) :
		super_type(data_, size_, name_, sourceByteOffset_) {}

	template <typename Other>
	inline NamedSpan(const Other &other) : super_type(other) {}
};

#pragma mark -
#pragma mark SpanOwner

/**
 * Similar to ScopedPtr, but allows holding and disposing pointers inside Spans
 * without requiring an additional pointer to data, and with copyability.
 */
template <typename OwnedSpan>
class SpanOwner : public SafeBool<SpanOwner<OwnedSpan> > {
	typedef typename OwnedSpan::value_type value_type;
	typedef typename OwnedSpan::size_type size_type;
	typedef typename OwnedSpan::index_type index_type;
	typedef typename OwnedSpan::pointer pointer;
	typedef typename OwnedSpan::reference reference;
	typedef typename OwnedSpan::const_reference const_reference;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T, typename U> friend struct SafeBool;
#endif

public:
	inline SpanOwner() : _span() {}

	inline SpanOwner(const OwnedSpan &span) : _span(span) {}

	/**
	 * Creates a new owned copy of the memory from the other SpanOwner.
	 */
	inline SpanOwner(const SpanOwner &other) {
		// Allocating memory when copy-constructing from an unallocated owner
		// will break the new owner by making it appear allocated even though
		// it doesn't (and shouldn't) contain data
		if (!other) {
			SpanOwner();
			return;
		}

		_span.allocateFromSpan(other._span);
	}

	inline SpanOwner &operator=(const SpanOwner &other) {
		if (this == &other) {
			return *this;
		}

		delete[] const_cast<typename RemoveConst<value_type>::type *>(_span.data());
		_span.clear();

		// Allocating memory when copy-assigning from an unallocated owner
		// will break the new owner by making it appear allocated even though
		// it doesn't (and shouldn't) contain data
		if (other) {
			_span.allocateFromSpan(other._span);
		}

		return *this;
	}

	inline ~SpanOwner() {
		delete[] const_cast<typename RemoveConst<value_type>::type *>(_span.data());
	}

	/**
	 * Transfers ownership of the Span from the other owner to this owner.
	 */
	inline SpanOwner &moveFrom(SpanOwner &other) {
		if (this == &other) {
			return *this;
		}

		delete[] const_cast<typename RemoveConst<value_type>::type *>(_span.data());
		_span = other._span;
		other.release();
		return *this;
	}

	/**
	 * Releases the memory owned by this SpanOwner to the caller.
	 */
	inline pointer release() {
		pointer data = _span.data();
		_span.clear();
		return data;
	}

	/**
	 * Destroys the memory owned by this owner.
	 */
	inline void clear() {
		delete[] const_cast<typename RemoveConst<value_type>::type *>(_span.data());
		_span.clear();
	}

#if !defined(_MSC_VER)
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
protected:
#endif
#endif
	inline bool operator_bool() const { return _span; }

private:
	OwnedSpan _span;

#pragma mark -
#pragma mark SpanOwner - Data access

public:
	inline const OwnedSpan &operator*() const { return _span; }
	inline OwnedSpan &operator*() { return _span; }

	inline const OwnedSpan *operator->() const { return &_span; }
	inline OwnedSpan *operator->() { return &_span; }

	inline const_reference operator[](const index_type index) const { return _span[index]; }
	inline reference operator[](const index_type index) { return _span[index]; }
};

} // End of namespace Common

#endif
