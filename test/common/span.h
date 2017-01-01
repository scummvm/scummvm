#include <cxxtest/TestSuite.h>

class SpanTestSuite;

#include "common/span.h"
#include "common/str.h"

class SpanTestSuite : public CxxTest::TestSuite {
	struct Foo {
		int a;
	};

public:
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
			owner2.release();
			TS_ASSERT_EQUALS((bool)owner2, false);
		}

		{
			Common::SpanOwner<Common::Span<byte> > owner2;
			TS_ASSERT_EQUALS((bool)owner, true);
			void *dataPtr = owner->data();
			owner2 = owner;
			TS_ASSERT_EQUALS((bool)owner, false);
			TS_ASSERT(owner->data() == nullptr);
			TS_ASSERT_EQUALS(owner2->data(), dataPtr);
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
			void *dataPtr = owner->data();
			TS_ASSERT_EQUALS(owner.release(), dataPtr);
			TS_ASSERT_EQUALS((bool)owner, false);
		}
	}

	void test_span_byte() {
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

#ifdef SCUMM_LITTLE_ENDIAN
		TS_ASSERT_EQUALS(span.getUint32At(3), 4278256131U);
#elif SCUMM_BIG_ENDIAN
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
		span[3] = '\0';
		TS_ASSERT(span.getStringAt(0) == "hel");
	}

	void test_span_unsafe_data() {
		char data[] = "hello";
		Common::Span<char> span(data, sizeof(data));
		TS_ASSERT_EQUALS(span.getUnsafeDataAt(0, 6), data);
	}

	void test_span_subspan() {
		byte data[] = { 1, 2, 3, 4, 5, 6 };
		Common::Span<byte> span(data, sizeof(data));

		TS_ASSERT_EQUALS(span.subspan(0).size(), sizeof(data) - 0);
		TS_ASSERT_EQUALS(span.subspan(2).size(), sizeof(data) - 2);
		TS_ASSERT_EQUALS(span.subspan(2, 2).size(), 2U);
		TS_ASSERT_EQUALS(span.subspan<uint16>(0).size(), sizeof(data) / 2);
		TS_ASSERT_EQUALS(span.subspan<uint16>(0).byteSize(), sizeof(data));
		TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).size(), 1U);
		TS_ASSERT_EQUALS(span.subspan<uint16>(0, 2).byteSize(), 2U);

#ifdef SCUMM_LITTLE_ENDIAN
		TS_ASSERT_EQUALS(span.subspan<uint16>(0)[1], 4 << 8 | 3);
#elif SCUMM_BIG_ENDIAN
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

	void test_span_to_stream() {
		const byte data[] = { 0, 1, 2, 3, 4, 5 };
		Common::Span<const byte> span(data, sizeof(data));

		Common::MemoryReadStream stream(span.toStream(1, 2));
		byte out;
		TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
		TS_ASSERT_EQUALS(out, 1);
		TS_ASSERT_EQUALS(stream.read(&out, 1), 1U);
		TS_ASSERT_EQUALS(out, 2);
		TS_ASSERT_EQUALS(stream.read(&out, 1), 0U);
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
		TS_ASSERT(span.checkInvalidBounds(6, 1)); // positive overflow (+7)
		TS_ASSERT(span.checkInvalidBounds(2, -4)); // negative overflow (-2)

		const ptrdiff_t big = 1L << (8 * sizeof(ptrdiff_t) - 1);
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

		actual = span.getValidationMessage(-34, -56, Common::kValidateSeek);
		expected = Common::String::format("Access violation seeking %s: -34 + -56 > 1", source.c_str());
		TS_ASSERT_EQUALS(actual, expected);
	}

	void test_span_comparators() {
		byte data[2];
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

	void test_namedspan() {
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
			expected = "Access violation reading new.data: -34 + -56 > 4 (abs: -32 + -56 > 6)";
			actual = subspan.getValidationMessage(-34, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		{
			Common::NamedSpan<byte> subspan = span.subspan(2, Common::kSpanMaxSize, "new.data", 0);
			expected = "Access violation reading new.data: -34 + -56 > 4 (abs: -34 + -56 > 4)";
			actual = subspan.getValidationMessage(-34, -56, Common::kValidateRead);
			TS_ASSERT_EQUALS(actual, expected);
		}

		Common::Span<byte> superclassInstance;
		superclassInstance = span;
	}
};
