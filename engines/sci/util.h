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

#ifndef SCI_UTIL_H
#define SCI_UTIL_H

#include "common/span.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Sci {

// Wrappers for reading/writing 16-bit values in the endianness
// of the original game platform.
uint16 READ_SCIENDIAN_UINT16(const void *ptr);
void WRITE_SCIENDIAN_UINT16(void *ptr, uint16 val);

// Wrappers for reading integer values for SCI1.1+.
// Mac versions have big endian data for some fields.
uint16 READ_SCI11ENDIAN_UINT16(const void *ptr);
uint32 READ_SCI11ENDIAN_UINT32(const void *ptr);
void WRITE_SCI11ENDIAN_UINT16(void *ptr, uint16 val);
void WRITE_SCI11ENDIAN_UINT32(void *ptr, uint32 val);
// Wrappers for reading integer values in resources that are
// LE in SCI1.1 Mac, but BE in SCI32 Mac
uint16 READ_SCI32ENDIAN_UINT16(const void *ptr);

#pragma mark -
#pragma mark SciSpanImpl - SciSpanIterator

namespace SciSpanInternal {
	template <typename Span, bool IsConst>
	class SciSpanIterator : public Common::SpanInternal::SpanIterator<Span, IsConst> {
		typedef typename Common::SpanInternal::SpanIterator<Span, IsConst> super_type;
		typedef typename Span::value_type span_value_type;
		typedef typename Common::Conditional<IsConst, const Span, Span>::type span_type;

	public:
		typedef typename Span::difference_type difference_type;
		typedef typename Common::RemoveConst<span_value_type>::type value_type;
		typedef typename Common::Conditional<IsConst, const span_value_type, span_value_type>::type *pointer;
		typedef typename Common::Conditional<IsConst, const span_value_type, span_value_type>::type &reference;

		inline SciSpanIterator() : super_type() {}

		inline SciSpanIterator(span_type *span, const difference_type index) : super_type(span, index) {}

		inline SciSpanIterator(const SciSpanIterator &other) : super_type(other) {}

		inline SciSpanIterator &operator=(const SciSpanIterator &other) {
			super_type::operator=(other);
			return *this;
		}

		inline SciSpanIterator operator+(const difference_type delta) const {
			SciSpanIterator it(*this);
			it += delta;
			return it;
		}

		inline SciSpanIterator operator-(const difference_type delta) const {
			return operator+(-delta);
		}

		inline difference_type operator-(const SciSpanIterator &other) const {
			assert(this->_span == other._span);
			return this->_index - other._index;
		}

		inline int16 getInt16SE() const {
			return this->_span->getInt16SEAt(this->_index);
		}

		inline uint16 getUint16SE() const {
			return this->_span->getUint16SEAt(this->_index);
		}

		inline uint16 getUint16SE32() const {
			return this->_span->getUint16SE32At(this->_index);
		}

		inline int32 getInt32SE() const {
			return this->_span->getInt32SEAt(this->_index);
		}

		inline uint32 getUint32SE() const {
			return this->_span->getUint32SEAt(this->_index);
		}

		inline void setUint16SE(uint16 value) const {
			this->_span->setUint16SEAt(this->_index, value);
		}

#ifdef ENABLE_SCI32
		inline void setUint32SE(uint32 value) const {
			this->_span->setUint32SEAt(this->_index, value);
		}
#endif
	};
} // End of namespace SciSpanInternal

/**
 * Bounds-checked random access into a contiguous block of memory, with
 * extra methods that automatically read and write using the correct endianness
 * for the currently loaded game.
 */
template <typename ValueType, template <typename> class Derived>
class SciSpanImpl : public Common::NamedSpanImpl<ValueType, Derived> {
	typedef Common::NamedSpanImpl<ValueType, Derived> super_type;
	typedef Derived<ValueType> derived_type;

#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template <typename T, template <typename> class U> friend class SciSpanImpl;
#endif
#if CXXTEST_RUNNING
	friend class ::SpanTestSuite;
#endif

public:
	typedef typename super_type::value_type value_type;
	typedef typename super_type::difference_type difference_type;
	typedef typename super_type::index_type index_type;
	typedef typename super_type::size_type size_type;
	typedef SciSpanInternal::SciSpanIterator<derived_type, true> const_iterator;
	typedef SciSpanInternal::SciSpanIterator<derived_type, false> iterator;
	typedef typename super_type::pointer pointer;
	typedef typename super_type::const_pointer const_pointer;
	typedef typename super_type::reference reference;
	typedef typename super_type::const_reference const_reference;

	inline SciSpanImpl() : super_type() {}

	inline SciSpanImpl(const pointer data_,
					   const size_type size_,
					   const Common::String &name_ = Common::String(),
					   const size_type sourceByteOffset_ = 0) :
		super_type(data_, size_, name_, sourceByteOffset_) {}

	template <typename Other>
	inline SciSpanImpl(const Other &other) : super_type(other) {}

	inline const_iterator cbegin() const { return const_iterator(&this->impl(), 0); }
	inline const_iterator cend() const { return const_iterator(&this->impl(), this->size()); }
	inline const_iterator begin() const { return const_iterator(&this->impl(), 0); }
	inline const_iterator end() const { return const_iterator(&this->impl(), this->size()); }
	inline iterator begin() { return iterator(&this->impl(), 0); }
	inline iterator end() { return iterator(&this->impl(), this->size()); }

#pragma mark -
#pragma mark SciSpan - Data access

public:
	inline int16 getInt16SEAt(const size_type index) const {
		return (int16)getUint16SEAt(index);
	}

	inline uint16 getUint16SEAt(const size_type index) const {
		this->validate(index, sizeof(uint16));
		return READ_SCI11ENDIAN_UINT16(this->data() + index);
	}

	inline uint16 getUint16SE32At(const size_type index) const {
		this->validate(index, sizeof(uint16));
		return READ_SCI32ENDIAN_UINT16(this->data() + index);
	}

	inline int32 getInt32SEAt(const size_type index) const {
		return (int32)getUint32SEAt(index);
	}

	inline uint32 getUint32SEAt(const size_type index) const {
		this->validate(index, sizeof(uint32));
		return READ_SCI11ENDIAN_UINT32(this->data() + index);
	}

	inline void setUint16SEAt(const size_type index, uint16 value) {
		this->validate(index, sizeof(uint16), Common::kValidateWrite);
		WRITE_SCI11ENDIAN_UINT16(this->data() + index, value);
	}

	inline void setUint32SEAt(const size_type index, uint32 value) {
		this->validate(index, sizeof(uint32), Common::kValidateWrite);
		WRITE_SCI11ENDIAN_UINT32(this->data() + index, value);
	}

#pragma mark -
#pragma mark SciSpanImpl - ForwardIterator

// Spans that are used as ForwardIterators must not be allowed inside of
// SpanOwner, since this will result in the wrong pointer to memory to be
// deleted
private:
	typedef typename Common::RemoveConst<Derived<ValueType> >::type mutable_derived_type;

public:
	inline const_reference operator*() const {
		this->validate(0, sizeof(value_type));
		return *this->_data;
	}

	inline reference operator*() {
		this->validate(0, sizeof(value_type));
		return *this->_data;
	}

	inline mutable_derived_type &operator+=(const difference_type delta) {
		this->validate(0, delta * sizeof(value_type), Common::kValidateSeek);
		this->_data += delta;
		this->_size -= delta;
		return this->impl();
	}

	inline mutable_derived_type &operator++() {
		return operator+=(1);
	}

	inline mutable_derived_type operator++(int) {
		mutable_derived_type span(this->impl());
		operator+=(1);
		return span;
	}

	inline mutable_derived_type operator+(const difference_type delta) const {
		mutable_derived_type span(this->impl());
		span += delta;
		return span;
	}
};

template <typename ValueType>
class SciSpan : public SciSpanImpl<ValueType, SciSpan> {
	typedef SciSpanImpl<ValueType, ::Sci::SciSpan> super_type;

public:
	COMMON_SPAN_TYPEDEFS

	inline SciSpan() : super_type() {}

	inline SciSpan(const pointer data_,
				   const size_type size_,
				   const Common::String &name_ = Common::String(),
				   const size_type sourceByteOffset_ = 0) :
		super_type(data_, size_, name_, sourceByteOffset_) {}

	template <typename Other>
	inline SciSpan(const Other &other) : super_type(other) {}
};

} // End of namespace Sci

#endif
