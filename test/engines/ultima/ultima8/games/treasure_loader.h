#include <cxxtest/TestSuite.h>
#include "engines/ultima/ultima8/games/treasure_loader.h"

/**
 * Test suite for the functions in engines/ultima/ultima8/games/treasure_loader.h
 *
 * TODO: We should test type= values, but they are loaded from the config file
 * 		 so we need a way to add those.
 *
 * That would also allow testing "type", "special", and "mult" values which
 * need defaults.
 */
class U8TreasureLoaderTestSuite : public CxxTest::TestSuite {
	public:
	U8TreasureLoaderTestSuite() {
	}
	Ultima::Ultima8::TreasureLoader loader;

	/* Parse nothing -> should return nothing */
	void test_parse_empty() {
		Common::Array<Ultima::Ultima8::TreasureInfo> t;
		bool result = loader.parse("", t);
		TS_ASSERT(result);
		TS_ASSERT(t.empty());
	}

	/* Parse a single treasure type */
	void test_parse_basic() {
		Common::Array<Ultima::Ultima8::TreasureInfo> t;

		bool result = loader.parse("shape=123,456 frame=2,3 count=4-20 map=23 chance=0.234", t);
		TS_ASSERT(result);

		TS_ASSERT_EQUALS(t.size(), 1u);
		const Ultima::Ultima8::TreasureInfo ti = t[0];
		TS_ASSERT_EQUALS(ti._shapes.size(), 2u);
		TS_ASSERT_EQUALS(ti._shapes[0], 123u);
		TS_ASSERT_EQUALS(ti._shapes[1], 456u);
		TS_ASSERT_EQUALS(ti._frames.size(), 2u);
		TS_ASSERT_EQUALS(ti._frames[0], 2u);
		TS_ASSERT_EQUALS(ti._frames[1], 3u);
		TS_ASSERT_EQUALS(ti._minCount, 4u);
		TS_ASSERT_EQUALS(ti._maxCount, 20u);
		TS_ASSERT_EQUALS(ti._special, "");
		TS_ASSERT_EQUALS(ti._map, 23);
		TS_ASSERT_EQUALS(ti._chance, 0.234);
	}

	/* Parse multiple treasure types */
	void test_parse_multi() {
		Common::Array<Ultima::Ultima8::TreasureInfo> t;

		bool result = loader.parse("shape=123;shape=456 frame=2-5;shape=888 map=-12", t);
		TS_ASSERT(result);

		TS_ASSERT_EQUALS(t.size(), 3u);

		TS_ASSERT_EQUALS(t[0]._shapes.size(), 1u);
		TS_ASSERT_EQUALS(t[0]._shapes[0], 123u);
		TS_ASSERT_EQUALS(t[0]._frames.size(), 0u);
		TS_ASSERT_EQUALS(t[0]._minCount, 1u);
		TS_ASSERT_EQUALS(t[0]._maxCount, 1u);
		TS_ASSERT_EQUALS(t[0]._special, "");
		TS_ASSERT_EQUALS(t[0]._map, 0);
		TS_ASSERT_EQUALS(t[0]._chance, 1u);

		TS_ASSERT_EQUALS(t[1]._shapes.size(), 1u);
		TS_ASSERT_EQUALS(t[1]._shapes[0], 456u);
		TS_ASSERT_EQUALS(t[1]._frames.size(), 4u);
		TS_ASSERT_EQUALS(t[1]._frames[0], 2u);
		TS_ASSERT_EQUALS(t[1]._frames[1], 3u);
		TS_ASSERT_EQUALS(t[1]._frames[2], 4u);
		TS_ASSERT_EQUALS(t[1]._frames[3], 5u);

		TS_ASSERT_EQUALS(t[2]._shapes.size(), 1u);
		TS_ASSERT_EQUALS(t[2]._shapes[0], 888u);
		TS_ASSERT_EQUALS(t[2]._map, -12);
	}

	/* Check that various invalid strings don't parse */
	void test_parse_invalid() {
		Common::Array<Ultima::Ultima8::TreasureInfo> t;

		bool result;

		result = loader.parse("shape=", t);
		TS_ASSERT(!result);

		result = loader.parse("what", t);
		TS_ASSERT(!result);

		result = loader.parse("shape=abc", t);
		TS_ASSERT(!result);

		result = loader.parse("shape=123,123456789", t);
		TS_ASSERT(!result);

		result = loader.parse("shape=-123", t);
		TS_ASSERT(!result);

		result = loader.parse("frame=-1,5", t);
		TS_ASSERT(!result);

		/* TODO: This case falls back to parsing the 10, not great.
		result = loader.parse("count=10-1", t);
		TS_ASSERT(!result);
		*/

		result = loader.parse("chance=-1", t);
		TS_ASSERT(!result);
	}
};
