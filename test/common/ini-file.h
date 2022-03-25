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
		static const unsigned char inistr[] = "#comment\n[s]\nabc=1\ndef=xyz";
		Common::MemoryReadStream ms(inistr, sizeof(inistr));
		Common::INIFile inifile;
		bool result = inifile.loadFromStream(ms);
		TS_ASSERT(result);

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 1);

		TS_ASSERT(inifile.hasSection("s"));
		TS_ASSERT(inifile.hasKey("abc", "s"));

		Common::String val;
		TS_ASSERT(inifile.getKey("abc", "s", val));
		TS_ASSERT_EQUALS(val, "1");
		TS_ASSERT(inifile.getKey("def", "s", val));
		TS_ASSERT_EQUALS(val, "xyz");
		inifile.setKey("abc", "s", "newval");
		TS_ASSERT(inifile.getKey("abc", "s", val));
		TS_ASSERT_EQUALS(val, "newval");
	}

	void test_multisection_ini_file() {
		static const unsigned char inistr[] = "[s]\nabc=1\ndef=xyz\n#comment=no\n[empty]\n\n[s2]\nabc=2";
		Common::MemoryReadStream ms(inistr, sizeof(inistr));
		Common::INIFile inifile;
		bool result = inifile.loadFromStream(ms);
		TS_ASSERT(result);

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 3);

		TS_ASSERT(inifile.hasSection("s"));
		TS_ASSERT(inifile.hasSection("empty"));
		TS_ASSERT(inifile.hasSection("s2"));
		TS_ASSERT(inifile.hasKey("abc", "s"));
		TS_ASSERT(inifile.hasKey("abc", "s2"));

		Common::String val;
		TS_ASSERT(inifile.getKey("abc", "s", val));
		TS_ASSERT_EQUALS(val, "1");
		TS_ASSERT(inifile.getKey("abc", "s2", val));
		TS_ASSERT_EQUALS(val, "2");
	}

	void test_modify_ini_file() {
		Common::INIFile inifile;

		TS_ASSERT(!inifile.hasSection("s"));
		inifile.addSection("s");
		TS_ASSERT(inifile.hasSection("s"));

		inifile.setKey("k", "s", "val");
		TS_ASSERT(inifile.hasKey("k", "s"));

		inifile.setKey("k2", "s", "val2");
		TS_ASSERT(inifile.hasKey("k2", "s"));
		inifile.removeKey("k2", "s");
		TS_ASSERT(!inifile.hasKey("k2", "s"));

		inifile.renameSection("s", "t");
		TS_ASSERT(!inifile.hasSection("s"));
		TS_ASSERT(inifile.hasSection("t"));
		TS_ASSERT(inifile.hasKey("k", "t"));

		inifile.removeSection("t");
		TS_ASSERT(!inifile.hasSection("t"));
	}
};
