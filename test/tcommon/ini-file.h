#include <cxxtest/TestSuite.h>

#include "common/formats/ini-file.h"
#include "common/memstream.h"


class IniFileTestSuite : public CxxTest::TestSuite {
	public:
	void test_blank_ini_file() {
		Common::INIFile inifile;

		TS_ASSERT(!inifile.hasSection("abc"));

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 0U);
	}

	void test_simple_ini_file() {
		static const unsigned char inistr[] = "#comment\n[s]\nabc=1\ndef=xyz";
		Common::MemoryReadStream ms(inistr, sizeof(inistr));
		Common::INIFile inifile;
		bool result = inifile.loadFromStream(ms);
		TS_ASSERT(result);

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 1U);

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
		static const unsigned char inistr[] = "[s]\nabc=1\ndef=xyz\n#comment=no\n[empty]\n\n[s2]\n abc = 2  \n ; comment=no";
		Common::MemoryReadStream ms(inistr, sizeof(inistr));
		Common::INIFile inifile;
		bool result = inifile.loadFromStream(ms);
		TS_ASSERT(result);

		Common::INIFile::SectionList sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 3U);

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

		inifile.clear();
		sections = inifile.getSections();
		TS_ASSERT_EQUALS(sections.size(), 0U);
		TS_ASSERT(!inifile.hasSection("s"));
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

	void test_name_validity() {
		Common::INIFile inifile;

		inifile.addSection("s*");
		TS_ASSERT(!inifile.hasSection("s*"));

		inifile.addSection("");
		TS_ASSERT(!inifile.hasSection(""));

		// Valid is alphanum plus [-_:. ]
		inifile.addSection("sEcT10N -_..Name:");
		TS_ASSERT(inifile.hasSection("sEcT10N -_..Name:"));

		const char invalids[] = "!\"#$%&'()=~[]()+?<>\r\t\n";
		for (uint i = 0; i < sizeof(invalids) - 1; i++) {
			char c = invalids[i];
			const Common::String s(c);
			inifile.addSection(s);
			TS_ASSERT(!inifile.hasSection(s));
		}

		inifile.clear();
		inifile.allowNonEnglishCharacters();
		for (uint i = 0; i < sizeof(invalids) - 1; i++) {
			char c = invalids[i];
			if (c == '[' || c == ']' || c == '#' || c == '=' || c == '\r' || c == '\n')
				continue;
			const Common::String s(c);
			inifile.addSection(s);
			TS_ASSERT(inifile.hasSection(s));
		}
	}

	void test_write_simple_ini_file() {
		byte buf[1024];
		Common::INIFile inifile;
		{
			static const unsigned char inistr[] = "#comment\n[s]\nabc=1\ndef=xyz";
			Common::MemoryReadStream mrs(inistr, sizeof(inistr));
			TS_ASSERT(inifile.loadFromStream(mrs));
		}

		// A too-small write buffer (should fail)
		{
			Common::MemoryWriteStream mws(buf, 10);
			TS_ASSERT(!inifile.saveToStream(mws));
		}

		// A good sized write buffer (should work)
		int len;
		{
			Common::MemoryWriteStream mws(buf, 1024);
			TS_ASSERT(inifile.saveToStream(mws));
			len = mws.pos();
		}

		{
			Common::MemoryReadStream mrs(buf, len - 1);
			Common::INIFile checkinifile;
			TS_ASSERT(checkinifile.loadFromStream(mrs));
			TS_ASSERT(checkinifile.hasSection("s"));

			const Common::INIFile::SectionList &sections = checkinifile.getSections();
			const Common::INIFile::Section &section = sections.front();
			TS_ASSERT_EQUALS(section.comment, "#comment\n");
			TS_ASSERT_EQUALS(section.name, "s");

			TS_ASSERT(checkinifile.hasKey("abc", "s"));
			TS_ASSERT(checkinifile.hasKey("def", "s"));

			Common::String val;
			TS_ASSERT(inifile.getKey("abc", "s", val));
			TS_ASSERT_EQUALS(val, "1");
			TS_ASSERT(inifile.getKey("def", "s", val));
			TS_ASSERT_EQUALS(val, "xyz");
		}

	}

};
