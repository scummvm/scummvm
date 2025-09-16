#include <cxxtest/TestSuite.h>

class SpanTestSuite;

#include "common/span.h"
#include "common/str.h"

class SpanTestSuite : public CxxTest::TestSuite {
	struct Foo {
		int a;
	};

	template <typename ValueType, template <typename> class Derived>
	class SiblingSpanImpl : public Common::SpanImpl<ValueType, Derived> {
		typedef Common::SpanImpl<ValueType, Derived> super_type;
	public:
		COMMON_SPAN_TYPEDEFS;
		SiblingSpanImpl() = default;
		SiblingSpanImpl(pointer data_, size_type size_) : super_type(data_, size_) {}
	};

	template <typename ValueType>
	class SiblingSpan : public SiblingSpanImpl<ValueType, SiblingSpan> {
		typedef SiblingSpanImpl<ValueType, ::SpanTestSuite::SiblingSpan> super_type;
	public:
		COMMON_SPAN_TYPEDEFS;
		SiblingSpan() = default;
		SiblingSpan(pointer data_, size_type size_) : super_type(data_, size_) {}
	};

	template <typename ValueType, template <typename> class Derived>
	class SubSpanImpl : public Common::NamedSpanImpl<ValueType, Derived> {
		typedef Common::NamedSpanImpl<ValueType, Derived> super_type;
	public:
		COMMON_SPAN_TYPEDEFS;
		SubSpanImpl() = default;
		SubSpanImpl(pointer data_,
					size_type size_,
					const Common::String &name_ = Common::String(),
					const size_type sourceByteOffset_ = 0) :
			super_type(data_, size_, name_, sourceByteOffset_) {}

		template <typename Other>
		SubSpanImpl(const Other &other) : super_type(other) {}
	};

	template <typename ValueType>
	class SubSpan : public SubSpanImpl<ValueType, SubSpan> {
		typedef SubSpanImpl<ValueType, ::SpanTestSuite::SubSpan> super_type;
	public:
		COMMON_SPAN_TYPEDEFS;
		SubSpan() = default;
		SubSpan(pointer data_,
				size_type size_,
				const Common::String &name_ = Common::String(),
				const size_type sourceByteOffset_ = 0) :
			super_type(data_, size_, name_, sourceByteOffset_) {}

		template <typename Other>
		SubSpan(const Other &other) : super_type(other) {}
	};

public:
	void test_sibling_span() {
		byte data[] = { 'h', 'e', 'l', 'l', 'o' };
		SiblingSpan<byte> ss(data, sizeof(data));
		Common::Span<byte> superInstance = ss;
		TS_ASSERT_EQUALS(ss.data(), data);
		TS_ASSERT_EQUALS(superInstance.data(), data);
	}

	void test_sub_span() {
		byte data[] = { 'h', 'e', 'l', 'l', 'o' };
		SubSpan<byte> ss(data, sizeof(data), "custom subspan");
		Common::NamedSpan<byte> namedSuper = ss;
		Common::Span<byte> unnamedSuper = ss;
		TS_ASSERT(ss.name() == "custom subspan");
		TS_ASSERT(namedSuper.name() == ss.name());
		TS_ASSERT(unnamedSuper.name() == Common::String::format("%p", (void *)data));
	}

	void test_span_iterator_const() {
		byte data[] = { 'h', 'e', 'l', 'l', 'o' };
		const Common::Span<byte> span(data, sizeof(data));

		Common::Span<byte>::const_iterator it = span.cbegin();

		Common::Span<byte>::const_iterator sameIt(it);

		TS_ASSERT_EQUALS(sameIt, it);

		uint i;
		for (i = 0; it != span.cend(); ++i, ++it) {
			TS_ASSERT_EQUALS(*it, data[i]);
			TS_ASSERT_LESS_THAN(i, sizeof(data));
		}
		TS_ASSERT_EQUALS(i, sizeof(data));

		it = span.cend() - 1;
		for (i = sizeof(data) - 1; it != span.cbegin(); --i, --it) {
			TS_ASSERT_EQUALS(data[i], *it);
		}
		TS_ASSERT_EQUALS(i, 0U);

		it = span.cbegin();

		it += 4;
		TS_ASSERT_EQUALS(data[4], *it);

		it -= 4;
		TS_ASSERT_EQUALS(data[0], *it);

		TS_ASSERT_EQUALS(data[0], *it++);

		TS_ASSERT_EQUALS(data[1], *it--);

		TS_ASSERT_EQUALS(span.cend() - span.cbegin(), 5);

		TS_ASSERT_EQUALS(*(span.cbegin() + 4), data[4]);

		TS_ASSERT_EQUALS(*(span.cend() - 4), data[1]);

		TS_ASSERT(span.cbegin() < span.cend());

		TS_ASSERT(span.cbegin() <= span.cend());
		TS_ASSERT(span.cbegin() <= span.cbegin());

		TS_ASSERT(span.cend() > span.cbegin());

		TS_ASSERT(span.cend() >= span.cbegin());
		TS_ASSERT(span.cend() >= span.cend());
	}

	void test_span_iterator() {
		byte data[] = { 'h', 'e', 'l', 'l', 'o' };
		Common::Span<byte> span(data, sizeof(data));

		// empty iterator should default construct OK
		Common::Span<byte>::iterator defaultIt;

		Common::Span<byte>::iterator it = span.begin();

		Common::Span<byte>::iterator sameIt(it);

		TS_ASSERT_EQUALS(sameIt, it);

		uint i;
		for (i = 0; it != span.end(); ++i, ++it) {
			TS_ASSERT_EQUALS(*it, data[i]);
			TS_ASSERT_LESS_THAN(i, sizeof(data));
		}
		TS_ASSERT_EQUALS(i, sizeof(data));

		it = span.end() - 1;
		for (i = sizeof(data) - 1; it != span.begin(); --i, --it) {
			TS_ASSERT_EQUALS(data[i], *it);
		}
		TS_ASSERT_EQUALS(i, 0U);

		it = span.begin();

		it += 4;
		TS_ASSERT_EQUALS(data[4], *it);

		it -= 4;
		TS_ASSERT_EQUALS(data[0], *it);

		TS_ASSERT_EQUALS(data[0], *it++);

		TS_ASSERT_EQUALS(data[1], *it--);

		TS_ASSERT_EQUALS(span.end() - span.begin(), 5);

		TS_ASSERT_EQUALS(*(span.begin() + 4), data[4]);

		TS_ASSERT_EQUALS(*(span.end() - 4), data[1]);

		TS_ASSERT(span.begin() < span.end());

		TS_ASSERT(span.begin() <= span.end());
		TS_ASSERT(span.begin() <= span.begin());

		TS_ASSERT(span.end() > span.begin());

		TS_ASSERT(span.end() >= span.begin());
		TS_ASSERT(span.end() >= span.end());

		it = span.begin();
		for (i = 0; it != span.end(); ++i, ++it) {
			*it = 'a' + i;
		}

		it = span.begin();
		for (i = 0; it != span.end(); ++i, ++it) {
			TS_ASSERT_EQUALS(*it, 'a' + i);
			TS_ASSERT_EQUALS(data[i], 'a' + i);
		}
	}

	void test_span_iterator_integers() {
		const byte data[] = { 0xFF, 1, 2, 3, 2, 1, 0xFF };
		Common::Span<const byte> span(data, sizeof(data));
		Common::Span<const byte>::const_iterator it = span.cbegin();

		TS_ASSERT_EQUALS(it.getInt8(), -1);
		TS_ASSERT_EQUALS(it.getUint8(), 255);
		TS_ASSERT_EQUALS(it.getInt16BE(), -255);
		TS_ASSERT_EQUALS(it.getUint16BE(), 65281U);
		TS_ASSERT_EQUALS((it + 5).getInt16LE(), -255);
		TS_ASSERT_EQUALS((it + 5).getUint16LE(), 65281U);
		TS_ASSERT_EQUALS(it.getUint24LE(), 131583U);
#if defined(SCUMM_LITTLE_ENDIAN)
		TS_ASSERT_EQUALS((it + 3).getUint32(), 4278256131U);
#elif defined(SCUMM_BIG_ENDIAN)
		TS_ASSERT_EQUALS(it.getUint32(), 4278256131U);
#else
#error No endianness detected
#endif
		TS_ASSERT_EQUALS(it.getInt32BE(), -16711165);
		TS_ASSERT_EQUALS(it.getUint32BE(), 4278256131U);
		TS_ASSERT_EQUALS((it + 3).getInt32LE(), -16711165);
		TS_ASSERT_EQUALS((it + 3).getUint32LE(), 4278256131U);
	}

	void test_span_iterator_ptr() {
		Foo foo[2];
		foo[0].a = 1;
		foo[1].a = 2;

		const Common::Span<Foo> span(foo, 2);
		Common::Span<Foo>::const_iterator it = span.cbegin();
		TS_ASSERT_EQUALS(it->a, 1);
		++it;
		TS_ASSERT_EQUALS(it->a, 2);

		TS_ASSERT_EQUALS(it[0].a, 2);
		TS_ASSERT_EQUALS(it[-1].a, 1);
		--it;
		TS_ASSERT_EQUALS(it[1].a, 2);
	}

	void test_span_owner() {
		Common::SpanOwner<Common::Span<byte> > owner;
		owner->allocate(3);
		owner[0] = 'a';
		owner[1] = 'b';
		owner[2] = 'c';
		for (int i = 0; i < 3; ++i) {
			TS_ASSERT_EQUALS(owner->getUint8At(i), 'a' + i);
			TS_ASSERT_EQUALS((*owner)[i], 'a' + i);
		}

		{
			Common::SpanOwner<Common::NamedSpan<byte> > owner2;
			TS_ASSERT(owner2->data() == nullptr);
			owner2->allocateFromSpan(*owner);
			TS_ASSERT(owner2->data() != nullptr);
			TS_ASSERT_DIFFERS(owner->data(), owner2->data());

			for (int i = 0; i < 3; ++i) {
				TS_ASSERT_EQUALS(owner2->getUint8At(i), 'a' + i);
				TS_ASSERT_EQUALS((*owner2)[i], 'a' + i);
			}

			TS_ASSERT_EQUALS((bool)owner2, true);
			owner2.clear();
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			Common::SpanOwner<Common::Span<byte> > owner2;
			TS_ASSERT(owner2->data() == nullptr);
			owner2 = owner;
			TS_ASSERT(owner2->data() != nullptr);
			TS_ASSERT_DIFFERS(owner->data(), owner2->data());

			for (int i = 0; i < 3; ++i) {
				TS_ASSERT_EQUALS(owner2->getUint8At(i), 'a' + i);
				TS_ASSERT_EQUALS((*owner2)[i], 'a' + i);
			}

			TS_ASSERT_EQUALS((bool)owner2, true);
			owner2.clear();
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			Common::SpanOwner<Common::Span<byte> > owner2;
			TS_ASSERT_EQUALS((bool)owner, true);
			void *dataPtr = owner->data();
			owner2.moveFrom(owner);
			TS_ASSERT_EQUALS((bool)owner, false);
			TS_ASSERT(owner->data() == nullptr);
			TS_ASSERT_EQUALS(owner2->data(), dataPtr);

			// tests destruction of held pointer by reassignment
			owner2 = owner;

			// tests nullipotence of assignment to self
			dataPtr = owner2->data();
			owner2 = owner2;
			TS_ASSERT(owner2->data() == dataPtr);
		}

		{
			char *data = new char[6];
			Common::strlcpy(data, "hello", 6);
			const Common::SpanOwner<Common::Span<const char> > constOwner(Common::Span<const char>(data, 6));
			TS_ASSERT_EQUALS((*constOwner)[0], 'h');
			TS_ASSERT_EQUALS(constOwner->getUint8At(1), 'e');
			TS_ASSERT_EQUALS(constOwner[2], 'l');
		}

		{
			TS_ASSERT_EQUALS((bool)owner, false);
			Common::SpanOwner<Common::Span<byte> > owner2(owner);
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			owner->allocate(1);
			TS_ASSERT_EQUALS((bool)owner, true);
			Common::SpanOwner<Common::Span<byte> > owner2(owner);
			TS_ASSERT_EQUALS((bool)owner2, true);
			TS_ASSERT_DIFFERS(owner->data(), owner2->data());
		}

		{
			TS_ASSERT_EQUALS((bool)owner, true);
			byte *dataPtr = owner->data();
			TS_ASSERT_EQUALS(owner.release(), dataPtr);
			TS_ASSERT_EQUALS((bool)owner, false);
			delete[] dataPtr;
		}
	}

	void test_span_owner_named_span() {
		Common::SpanOwner<Common::NamedSpan<byte> > owner;
		owner->allocate(3, "foo");
		owner[0] = 'a';
		owner[1] = 'b';
		owner[2] = 'c';
		for (int i = 0; i < 3; ++i) {
			TS_ASSERT_EQUALS(owner->getUint8At(i), 'a' + i);
			TS_ASSERT_EQUALS((*owner)[i], 'a' + i);
		}
		TS_ASSERT(owner->name() == "foo");

		{
			Common::SpanOwner<Common::NamedSpan<byte> > owner2;
			TS_ASSERT(owner2->data() == nullptr);
			owner2->allocateFromSpan(*owner);
			TS_ASSERT(owner2->data() != nullptr);
			TS_ASSERT_DIFFERS(owner->data(), owner2->data());
			TS_ASSERT(owner2->name() == "foo");

			for (int i = 0; i < 3; ++i) {
				TS_ASSERT_EQUALS(owner2->getUint8At(i), 'a' + i);
				TS_ASSERT_EQUALS((*owner2)[i], 'a' + i);
			}

			TS_ASSERT_EQUALS((bool)owner2, true);
			owner2.clear();
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			Common::SpanOwner<Common::NamedSpan<byte> > owner2;
			TS_ASSERT_EQUALS((bool)owner, true);
			void *dataPtr = owner->data();
			owner2.moveFrom(owner);
			TS_ASSERT_EQUALS((bool)owner, false);
			TS_ASSERT(owner->data() == nullptr);
			TS_ASSERT_EQUALS(owner2->data(), dataPtr);

			// tests destruction of held pointer by reassignment
			owner2 = owner;
		}

		{
			char *data = new char[6];
			Common::strlcpy(data, "hello", 6);
			const Common::SpanOwner<Common::NamedSpan<const char> > constOwner(Common::NamedSpan<const char>(data, 6));
			TS_ASSERT_EQUALS((*constOwner)[0], 'h');
			TS_ASSERT_EQUALS(constOwner->getUint8At(1), 'e');
			TS_ASSERT_EQUALS(constOwner[2], 'l');
		}

		{
			TS_ASSERT_EQUALS((bool)owner, false);
			Common::SpanOwner<Common::NamedSpan<byte> > owner2(owner);
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			owner->allocate(1);
			TS_ASSERT_EQUALS((bool)owner, true);
			Common::SpanOwner<Common::NamedSpan<byte> > owner2(owner);
			TS_ASSERT_EQUALS((bool)owner2, true);
			TS_ASSERT_DIFFERS(owner->data(), owner2->data());
		}

		{
			TS_ASSERT_EQUALS((bool)owner, true);
			byte *dataPtr = owner->data();
			TS_ASSERT_EQUALS(owner.release(), dataPtr);
			TS_ASSERT_EQUALS((bool)owner, false);
			delete[] dataPtr;
		}
	}

	void test_span_allocate_from_stream() {
		byte data[] = "hello";
		Common::MemoryReadStream stream(data, sizeof(data));
		Common::SpanOwner<Common::Span<byte> > owner;
		owner->allocateFromStream(stream, 2);
		TS_ASSERT(owner->data() != data);
		TS_ASSERT_EQUALS(owner->size(), 2U);
		TS_ASSERT_EQUALS(owner[0], 'h');
		TS_ASSERT_EQUALS(owner[1], 'e');
		owner.clear();
		TS_ASSERT(owner->data() == nullptr);
		stream.seek(0, SEEK_SET);

		owner->allocateFromStream(stream);
		TS_ASSERT(owner->data() != data);
		TS_ASSERT_EQUALS(owner->size(), sizeof(data));
		TS_ASSERT_EQUALS(owner[0], 'h');
		TS_ASSERT_EQUALS(owner[1], 'e');
		TS_ASSERT_EQUALS(owner[2], 'l');
		TS_ASSERT_EQUALS(owner[3], 'l');
		TS_ASSERT_EQUALS(owner[4], 'o');

		Common::SpanOwner<Common::NamedSpan<const byte> > owner2;
		stream.seek(0, SEEK_SET);
		owner2->allocateFromStream(stream, Common::kSpanMaxSize, "streamname");
		TS_ASSERT(owner2->data() != data);
		TS_ASSERT_EQUALS(owner2->size(), sizeof(data));
		TS_ASSERT_EQUALS(owner2[0], 'h');
		TS_ASSERT_EQUALS(owner2[1], 'e');
		TS_ASSERT_EQUALS(owner2[2], 'l');
		TS_ASSERT_EQUALS(owner2[3], 'l');
		TS_ASSERT_EQUALS(owner2[4], 'o');
		TS_ASSERT_EQUALS(owner2->name(), "streamname");
	}

	void test_span_byte() {
		{
			byte data[] = { 'h', 'e', 'l', 'l', 'o' };
			Common::Span<byte> span(data, sizeof(data));

			TS_ASSERT_EQUALS(span.size(), sizeof(data));
			TS_ASSERT_EQUALS(span.byteSize(), sizeof(data));

			Common::Span<byte> other(span);
			TS_ASSERT_EQUALS(span, other);
			other.clear();
			TS_ASSERT(span != other);

			TS_ASSERT_EQUALS(span[0], 'h');
			TS_ASSERT_EQUALS(span[1], 'e');
			span[1] = 'o';
			TS_ASSERT_EQUALS(span[1], 'o');

			TS_ASSERT((bool)span);
			span.clear();
			TS_ASSERT(!(bool)span);
		}

		{
			byte data[] = { 'h', 'e', 'l', 'l', 'o' };
			const Common::Span<const byte> span(data, sizeof(data));

			TS_ASSERT_EQUALS(span.size(), sizeof(data));
			TS_ASSERT_EQUALS(span.byteSize(), sizeof(data));

			const Common::Span<const byte> other(span);
			TS_ASSERT_EQUALS(span, other);

			TS_ASSERT_EQUALS(span[0], 'h');
			TS_ASSERT_EQUALS(span[1], 'e');
		}
	}

	void test_span_integers() {
		const byte data[] = { 0xFF, 1, 2, 3, 2, 1, 0xFF };
		Common::Span<const byte> span(data, sizeof(data));

		TS_ASSERT_EQUALS(span[0], 255);
		TS_ASSERT_EQUALS(span.getInt8At(0), -1);
		TS_ASSERT_EQUALS(span.getUint8At(0), 255U);
		TS_ASSERT_EQUALS(span.getInt16BEAt(0), -255);
		TS_ASSERT_EQUALS(span.getUint16BEAt(0), 65281U);
		TS_ASSERT_EQUALS(span.getInt16LEAt(5), -255);
		TS_ASSERT_EQUALS(span.getUint16LEAt(5), 65281U);
		TS_ASSERT_EQUALS(span.getUint24LEAt(0), 131583U);
		TS_ASSERT_EQUALS(span.getInt32BEAt(0), -16711165);
		TS_ASSERT_EQUALS(span.getUint32BEAt(0), 4278256131U);
		TS_ASSERT_EQUALS(span.getInt32LEAt(3), -16711165);
		TS_ASSERT_EQUALS(span.getUint32LEAt(3), 4278256131U);

#if defined(SCUMM_LITTLE_ENDIAN)
		TS_ASSERT_EQUALS(span.getUint32At(3), 4278256131U);
#elif defined(SCUMM_BIG_ENDIAN)
		TS_ASSERT_EQUALS(span.getUint32At(0), 4278256131U);
#else
#error No endianness detected
#endif
	}

	void test_span_string() {
		char data[] = "hello";
		Common::Span<char> span(data, sizeof(data));
		TS_ASSERT_EQUALS(span[sizeof(data) - 1], '\0');

		TS_ASSERT(span.getStringAt(0) == data);
		TS_ASSERT(span.getStringAt(0, 2) == "he");
		TS_ASSERT(span.getStringAt(2) == "llo");
		TS_ASSERT(span.getStringAt(2, 3) == "llo");
		span[3] = '\0';
		TS_ASSERT(span.getStringAt(0) == "hel");
	}

	void test_span_unsafe_data() {
		char data[] = "hello";
		Common::Span<char> span(data, sizeof(data));

		char *ptr = span.getUnsafeDataAt(0, 6);
		TS_ASSERT_EQUALS(ptr, data);
		ptr = span.getUnsafeDataAt(0);
		TS_ASSERT_EQUALS(ptr, data);

		const Common::Span<const char> span2(data, sizeof(data));
		const char *ptr2 = span2.getUnsafeDataAt(0, 6);
		TS_ASSERT_EQUALS(ptr2, data);
		ptr2 = span2.getUnsafeDataAt(0);
		TS_ASSERT_EQUALS(ptr2, data);
	}

	void test_span_subspan() {
		{
			byte data[] = { 1, 2, 3, 4, 5, 6 };
			Common::Span<byte> span(data, sizeof(data));

			TS_ASSERT_EQUALS(span.subspan(0).size(), sizeof(data) - 0);
			TS_ASSERT_EQUALS(span.subspan(2).size(), sizeof(data) - 2);
			TS_ASSERT_EQUALS(span.subspan(2, 2).size(), 2U);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0).size(), sizeof(data) / 2);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0).byteSize(), sizeof(data));
			TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).size(), 1U);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).byteSize(), 2U);

#if defined(SCUMM_LITTLE_ENDIAN)
			TS_ASSERT_EQUALS(span.subspan<uint16>(0)[1], 4 << 8 | 3);
#elif defined(SCUMM_BIG_ENDIAN)
			TS_ASSERT_EQUALS(span.subspan<uint16>(0)[1], 3 << 8 | 4);
#else
#error No endianness detected
#endif

			Common::Span<uint16> shortSpan = span.subspan<uint16>(0);
			TS_ASSERT_EQUALS(shortSpan.byteSize(), span.byteSize());
			TS_ASSERT(shortSpan.size() != span.size());
			shortSpan[1] = 0xFFFF;
			Common::Span<byte> byteSpan = shortSpan.subspan<byte>(1);
			TS_ASSERT_EQUALS(byteSpan.size(), sizeof(data) - 1 * sizeof(uint16));
			TS_ASSERT_EQUALS(byteSpan[0], 0xFF);
			TS_ASSERT_EQUALS(byteSpan[1], 0xFF);
		}

		{
			byte data[] = { 1, 2, 3, 4, 5, 6 };
			const Common::Span<const byte> span(data, sizeof(data));

			TS_ASSERT_EQUALS(span.subspan(0).size(), sizeof(data) - 0);
			TS_ASSERT_EQUALS(span.subspan(2).size(), sizeof(data) - 2);
			TS_ASSERT_EQUALS(span.subspan(2, 2).size(), 2U);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0).size(), sizeof(data) / 2);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0).byteSize(), sizeof(data));
			TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).size(), 1U);
			TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).byteSize(), 2U);

#if defined(SCUMM_LITTLE_ENDIAN)
			TS_ASSERT_EQUALS(span.subspan<uint16>(0)[1], 4 << 8 | 3);
#elif defined(SCUMM_BIG_ENDIAN)
			TS_ASSERT_EQUALS(span.subspan<uint16>(0)[1], 3 << 8 | 4);
#else
#error No endianness detected
#endif

			const Common::Span<uint16> shortSpan = span.subspan<uint16>(0);
			TS_ASSERT_EQUALS(shortSpan.byteSize(), span.byteSize());
			TS_ASSERT(shortSpan.size() != span.size());
			Common::Span<byte> byteSpan = shortSpan.subspan<byte>(1);
			TS_ASSERT_EQUALS(byteSpan.size(), sizeof(data) - 1 * sizeof(uint16));
			TS_ASSERT_EQUALS(byteSpan[0], 3);
			TS_ASSERT_EQUALS(byteSpan[1], 4);
		}
	}

	void test_span_to_stream() {
		const byte data[] = { 0, 1, 2, 3 };
		Common::Span<const byte> span(data, sizeof(data));

		{
			Common::MemoryReadStream stream(span.toStream(1, 2));
			byte out;
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 1);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 2);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 0U);
		}

		{
			Common::MemoryReadStream stream = span.toStream();
			byte out;
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 0);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 1);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 2);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
			TS_ASSERT_EQUALS(out, 3);
			TS_ASSERT_EQUALS(stream.read(&out, 1), 0U);
		}
	}

	void test_span_copying() {
		const byte data[] = { 0, 1, 2, 3, 4, 5 };
		Common::Span<const byte> span(data, sizeof(data));

		byte targetData[sizeof(data)] = {};
		Common::Span<byte> target(targetData, sizeof(targetData));
		span.copyDataTo(target);
		for (uint i = 0; i < sizeof(data); ++i) {
			TS_ASSERT_EQUALS(target[i], i);
		}

		byte out[sizeof(data)];
		span.unsafeCopyDataTo(out);
		for (uint i = 0; i < sizeof(data); ++i) {
			TS_ASSERT_EQUALS(out[i], i);
		}
	}

	void test_span_validation() {
		byte data[6];
		Common::Span<byte> span(data, sizeof(data));
		TS_ASSERT(!span.checkInvalidBounds(0, 0));
		TS_ASSERT(!span.checkInvalidBounds(0, 6));
		TS_ASSERT(!span.checkInvalidBounds(2, 4));
		TS_ASSERT(!span.checkInvalidBounds(4, 2));
		TS_ASSERT(!span.checkInvalidBounds(6, 0));
		TS_ASSERT(!span.checkInvalidBounds(2, -2));
		TS_ASSERT(span.checkInvalidBounds(-2, 2)); // negative index disallowed
		TS_ASSERT(span.checkInvalidBounds(6, 1)); // combined positive overflow (+7)
		TS_ASSERT(span.checkInvalidBounds(2, -4)); // negative overflow (-2)
		TS_ASSERT(span.checkInvalidBounds(0, 10)); // delta positive overflow

		const Common::Span<byte>::difference_type big = (Common::Span<byte>::difference_type)1 << (8 * sizeof(Common::Span<byte>::difference_type) - 1);
		TS_ASSERT(span.checkInvalidBounds(big, 0));
		TS_ASSERT(span.checkInvalidBounds(0, big));
		TS_ASSERT(span.checkInvalidBounds(big, big));
	}

	void test_span_validation_message() {
		byte data[1];
		Common::Span<byte> span(data, sizeof(data));

		Common::String source = span.name();
		Common::String actual;
		Common::String expected;

		actual = span.getValidationMessage(12, 34, Common::kValidateRead);
		expected = Common::String::format("Access violation reading %s: 12 + 34 > 1", source.c_str());
		TS_ASSERT_EQUALS(actual, expected);

		actual = span.getValidationMessage(23, 45, Common::kValidateWrite);
		expected = Common::String::format("Access violation writing %s: 23 + 45 > 1", source.c_str());
		TS_ASSERT_EQUALS(actual, expected);

		actual = span.getValidationMessage(0, -56, Common::kValidateSeek);
		expected = Common::String::format("Access violation seeking %s: 0 + -56 > 1", source.c_str());
		TS_ASSERT_EQUALS(actual, expected);
	}

	void test_span_comparators() {
		byte data[2] = { 0 };
		Common::Span<const byte> span0(data, sizeof(data));
		Common::Span<const byte> span1(data, sizeof(data));
		Common::Span<const byte> span2(data, sizeof(data) - 1);
		Common::Span<const byte> span3(data + 1, sizeof(data) - 1);
		Common::Span<const byte> span4(data + 2, sizeof(data) - 2);

		TS_ASSERT(span0 == span1);
		TS_ASSERT(span0 != span2);
		TS_ASSERT(span0 <= span1);
		TS_ASSERT(span0 <= span3);
		TS_ASSERT(span0 < span3);
		TS_ASSERT(span3 < span4);
		TS_ASSERT(span4 > span3);
		TS_ASSERT(span3 > span0);
		TS_ASSERT(span4 >= span4);
		TS_ASSERT(span0 >= span1);

		TS_ASSERT_EQUALS(span1 - span0, 0);
		TS_ASSERT_EQUALS(span3 - span0, 1);
		TS_ASSERT_EQUALS(span4 - span0, 2);
		TS_ASSERT_EQUALS(span0 - span1, 0);
		TS_ASSERT_EQUALS(span0 - span3, -1);
		TS_ASSERT_EQUALS(span0 - span4, -2);
	}

	void test_named_span() {
		byte data[6] = { 0, 1, 2, 3, 4, 5 };
		Common::NamedSpan<byte> span(data, sizeof(data), "foo.data");
		TS_ASSERT_EQUALS(span.name(), "foo.data");

		Common::String actual;
		Common::String expected;

		actual = span.getValidationMessage(12, 34, Common::kValidateRead);
		expected = "Access violation reading foo.data: 12 + 34 > 6 (abs: 12 + 34 > 6)";
		TS_ASSERT_EQUALS(actual, expected);

		{
			Common::NamedSpan<byte> subspan = span.subspan(2);

			expected = "Access violation reading foo.data: 23 + 45 > 4 (abs: 25 + 45 > 6)";
			actual = subspan.getValidationMessage(23, 45, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::NamedSpan<byte> subspan = span.subspan(2, Common::kSpanMaxSize, "new.data");
			expected = "Access violation reading new.data: 0 + -56 > 4 (abs: 2 + -56 > 6)";
			actual = subspan.getValidationMessage(0, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::NamedSpan<byte> subspan = span.subspan(2, Common::kSpanMaxSize, "new.data", 0);
			expected = "Access violation reading new.data: 0 + -56 > 4 (abs: 0 + -56 > 4)";
			actual = subspan.getValidationMessage(0, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		Common::NamedSpan<byte> span2;
		span = span2 = span;
		TS_ASSERT_EQUALS(span2, span);
		TS_ASSERT(span2.name() == span.name());
		TS_ASSERT(span2.sourceByteOffset() == span.sourceByteOffset());

		Common::Span<byte> superclassInstance;
		superclassInstance = span;
		TS_ASSERT_EQUALS(span, superclassInstance);

		Common::Span<byte> subclassInstance(superclassInstance);
		TS_ASSERT_EQUALS(subclassInstance, superclassInstance);

		const Common::NamedSpan<const byte> constSpan(span);

		{
			Common::NamedSpan<const byte> subspan = constSpan.subspan(2);

			expected = "Access violation reading foo.data: 23 + 45 > 4 (abs: 25 + 45 > 6)";
			actual = subspan.getValidationMessage(23, 45, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
			TS_ASSERT_EQUALS(subspan.sourceByteOffset(), 2U);
		}

		{
			Common::NamedSpan<const byte> subspan = constSpan.subspan(2, Common::kSpanMaxSize, "new.data");
			expected = "Access violation reading new.data: 0 + -56 > 4 (abs: 2 + -56 > 6)";
			actual = subspan.getValidationMessage(0, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::NamedSpan<const byte> subspan = constSpan.subspan(2, Common::kSpanMaxSize, "new.data", 0);
			expected = "Access violation reading new.data: 0 + -56 > 4 (abs: 0 + -56 > 4)";
			actual = subspan.getValidationMessage(0, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::NamedSpan<const byte> subspan = constSpan.subspan(2, Common::kSpanMaxSize, "new.data", 0);
			subspan.sourceByteOffset() = 2;
			expected = "Access violation reading new.data: 0 + -56 > 4 (abs: 2 + -56 > 6)";
			actual = subspan.getValidationMessage(0, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::MemoryReadStream *stream = new Common::MemoryReadStream(data, sizeof(data));
			Common::File file;
			file.open(stream, "test.txt");
			Common::SpanOwner<Common::NamedSpan<const byte> > fileOwner;
			fileOwner->allocateFromStream(file);
			TS_ASSERT_EQUALS(fileOwner->size(), (uint)file.size());
			file.close();
			TS_ASSERT(fileOwner->name() == "test.txt");
			for (uint i = 0; i < fileOwner->size(); ++i) {
				TS_ASSERT_EQUALS(fileOwner->getInt8At(i), data[i]);
			}
		}
	}
};
