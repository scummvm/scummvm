#include <cxxtest/TestSuite.h>

#include "common/ini-file.h"
#include "common/memstream.h"

class IniFileTestSuite : public CxxTest::TestSuite {
	public:
	void test_blank_ini_file() {
		Common::INIFile inifile;

		TS_ASSERT(!inifile.hasSection("abc"));

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 0);
	}

	void test_simple_ini_file() {
		const unsigned char data[] = "[s]\nabc=1\ndef=xyz";
		Common::MemoryReadStream ms(data, sizeof(data));

		Common::INIFile inifile;
		bool result = inifile.loadFromStream(ms);
		TS_ASSERT(result);

		TS_ASSERT(inifile.hasSection("s"));
		TS_ASSERT(inifile.hasKey("abc", "s"));

		Common::String val;
		TS_ASSERT(inifile.getKey("abc", "s", val));
		TS_ASSERT_EQUALS(val, "1");
		inifile.setKey("abc", "s", "newval");
		TS_ASSERT(inifile.getKey("abc", "s", val));
		TS_ASSERT_EQUALS(val, "newval");
	}
};
