#include <cxxtest/TestSuite.h>

#include "test/common/str-helper.h"

#include "common/path.h"
#include "common/hashmap.h"

static const char *TEST_PATH = "parent/dir/file.txt";
static const char *TEST_ESCAPED1_PATH = "|parent/dir/file.txt";
static const char *TEST_ESCAPED2_PATH = "par/ent\\dir\\file.txt";
static const char *TEST_BS_PATH = "parent\\dir\\file.txt";

class PathTestSuite : public CxxTest::TestSuite
{
	public:
	void test_Path() {
		Common::Path p;
		TS_ASSERT_EQUALS(p.toString(), "");
		TS_ASSERT_EQUALS(p.empty(), true);

		Common::Path p2(TEST_PATH);
		TS_ASSERT_EQUALS(p2.toString(), TEST_PATH);
		TS_ASSERT_EQUALS(p2.toString('|'), "parent|dir|file.txt");
		TS_ASSERT_EQUALS(p2.toString('\\'), "parent\\dir\\file.txt");

		Common::Path p3(TEST_ESCAPED1_PATH);
		TS_ASSERT_EQUALS(p3.toString(), TEST_ESCAPED1_PATH);
		TS_ASSERT_EQUALS(p3.toString('\\'), "|parent\\dir\\file.txt");

		Common::Path p4(TEST_ESCAPED2_PATH, '\\');
		TS_ASSERT_EQUALS(p4.toString('\\'), TEST_ESCAPED2_PATH);

		Common::Path p5(TEST_BS_PATH, '\\');
		TS_ASSERT_EQUALS(p5.toString('\\'), TEST_BS_PATH);

#ifndef RELEASE_BUILD
		Common::Path::_shownSeparatorCollisionWarning = false;
		TS_ASSERT_EQUALS(p3.toString('|'), "|parent|dir|file.txt");
		TS_ASSERT_EQUALS(Common::Path::_shownSeparatorCollisionWarning, true);

		Common::Path::_shownSeparatorCollisionWarning = false;
		TS_ASSERT_EQUALS(p3.toString('i'), "|parentidirifile.txt");
		TS_ASSERT_EQUALS(Common::Path::_shownSeparatorCollisionWarning, true);

		Common::Path::_shownSeparatorCollisionWarning = false;
		TS_ASSERT_EQUALS(p4.toString('/'), "par/ent/dir/file.txt");
		TS_ASSERT_EQUALS(Common::Path::_shownSeparatorCollisionWarning, true);

		Common::Path::_shownSeparatorCollisionWarning = false;
		TS_ASSERT_EQUALS(p5.toString('i'), "parentidirifile.txt");
		TS_ASSERT_EQUALS(Common::Path::_shownSeparatorCollisionWarning, true);
#endif
	}

	void test_clear() {
		Common::Path p(TEST_PATH);
		TS_ASSERT_EQUALS(p.empty(), false);

		p.clear();
		TS_ASSERT_EQUALS(p.empty(), true);

		TS_ASSERT(p.equals(Common::Path()));
		TS_ASSERT(p != Common::Path(TEST_PATH));
	}

	void test_getLastComponent() {
		Common::Path p;
		TS_ASSERT_EQUALS(p.getLastComponent().toString(), "");
		TS_ASSERT_EQUALS(p.getLastComponent(), Common::Path());

		Common::Path p2(TEST_PATH);
		TS_ASSERT_EQUALS(p2.getLastComponent().toString(), "file.txt");
		TS_ASSERT_EQUALS(p2.getLastComponent(), Common::Path("file.txt"));

		Common::Path p3("parent/dir/|file.txt");
		TS_ASSERT_EQUALS(p3.getLastComponent().toString(), "|file.txt");
		TS_ASSERT_EQUALS(p3.getLastComponent(), Common::Path("|file.txt"));
	}

	void test_baseName() {
		Common::Path p;
		TS_ASSERT_EQUALS(p.baseName(), "");

		Common::Path p2(TEST_PATH);
		TS_ASSERT_EQUALS(p2.baseName(), "file.txt");

		Common::Path p3("parent\\dir\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p3.baseName(), "fi/le.txt");

		Common::Path p4("parent/dir/file.txt/");
		TS_ASSERT_EQUALS(p4.baseName(), "file.txt");
	}

	void test_getParent() {
		Common::Path p;
		TS_ASSERT_EQUALS(p.getParent().toString(), "");

		Common::Path p2(TEST_PATH);
		TS_ASSERT_EQUALS(p2.getParent().toString(), "parent/dir/");
		// TODO: should this work?
		TS_ASSERT_EQUALS(p2.getParent().getLastComponent().toString(), "dir/");

		Common::Path p3(TEST_ESCAPED1_PATH);
		TS_ASSERT_EQUALS(p3.getParent().toString(), "|parent/dir/");
		// TODO: should this work?
		TS_ASSERT_EQUALS(p3.getParent().getLastComponent().toString(), "dir/");

		Common::Path p4(TEST_ESCAPED2_PATH, '\\');
		TS_ASSERT_EQUALS(p4.getParent().toString('\\'), "par/ent\\dir\\");
		// TODO: should this work?
		TS_ASSERT_EQUALS(p4.getParent().getLastComponent().toString('\\'), "dir\\");
	}

	void test_join() {
		Common::Path p("dir");
		Common::Path p2 = p.join("file.txt");
		TS_ASSERT_EQUALS(p2.toString(), "dir/file.txt");
		p2 = p.join("");
		TS_ASSERT_EQUALS(p2.toString(), "dir");
		p2 = p.join(Common::Path());
		TS_ASSERT_EQUALS(p2.toString(), "dir");

		Common::Path p3;
		Common::Path p4 = p3.join("file.txt");
		TS_ASSERT_EQUALS(p4.toString(), "file.txt");
		p4 = p3.join(Common::String("file.txt"));
		TS_ASSERT_EQUALS(p4.toString(), "file.txt");
		p4 = p3.join(Common::Path("file.txt"));
		TS_ASSERT_EQUALS(p4.toString(), "file.txt");

		Common::Path p5(TEST_PATH);
		Common::Path p6 = p5.getParent().join("other.txt");
		TS_ASSERT_EQUALS(p6.toString(), "parent/dir/other.txt");
		p6 = p5.getParent().join("|child\\other.txt", '\\');
		TS_ASSERT_EQUALS(p6.toString(), "parent/dir/|child/other.txt");
		p6 = p5.getParent().join("/child\\other.txt", '\\');
		TS_ASSERT_EQUALS(p6.toString('|'), "parent|dir|/child|other.txt");
		p6 = p5.getParent().join(Common::Path("|other.txt"));
		TS_ASSERT_EQUALS(p6.toString(), "parent/dir/|other.txt");
		p6 = p5.getParent().join(Common::Path("oth/er.txt", '\\'));
		TS_ASSERT_EQUALS(p6.toString('\\'), "parent\\dir\\oth/er.txt");

		Common::Path p7(TEST_ESCAPED1_PATH);
		Common::Path p8 = p7.getParent().join("other.txt");
		TS_ASSERT_EQUALS(p8.toString(), "|parent/dir/other.txt");
		p8 = p7.getParent().join(Common::Path("other.txt"));
		TS_ASSERT_EQUALS(p8.toString(), "|parent/dir/other.txt");
		p8 = p7.getParent().join(Common::Path("|other.txt"));
		TS_ASSERT_EQUALS(p8.toString(), "|parent/dir/|other.txt");
	}

	// Ensure we can joinInPlace correctly with leading or trailing separators
	void test_joinInPlace() {
		Common::Path p("abc/def");
		p.joinInPlace("file.txt");
		TS_ASSERT_EQUALS(p.toString(), "abc/def/file.txt");

		Common::Path p2("xyz/def");
		p2.joinInPlace(Common::Path("file.txt"));
		TS_ASSERT_EQUALS(p2.toString(), "xyz/def/file.txt");

		Common::Path p3("ghi/def/");
		p3.joinInPlace(Common::Path("file.txt"));
		TS_ASSERT_EQUALS(p3.toString(), "ghi/def/file.txt");

		Common::Path p4("123/def");
		p4.joinInPlace(Common::Path("/file4.txt"));
		TS_ASSERT_EQUALS(p4.toString(), "123/def/file4.txt");

		Common::Path p5("abc/def");
		p5.joinInPlace(Common::String("file.txt"));
		TS_ASSERT_EQUALS(p5.toString(), "abc/def/file.txt");
	}

	void test_append() {
		Common::Path p("abc/def");
		p.appendInPlace("");
		TS_ASSERT_EQUALS(p.toString(), "abc/def");

		Common::Path p2;
		p2.appendInPlace("file.txt");
		TS_ASSERT_EQUALS(p2.toString(), "file.txt");

		Common::Path p3("abc/def");
		p3.appendInPlace(Common::Path());
		TS_ASSERT_EQUALS(p3.toString(), "abc/def");

		Common::Path p4;
		p4.appendInPlace(Common::Path("file.txt"));
		TS_ASSERT_EQUALS(p4.toString(), "file.txt");

		TS_ASSERT_EQUALS(p4.append("a|b", '|').toString(), "file.txta/b");
		TS_ASSERT_EQUALS(p4.append(Common::String("a|b"), '|').toString('/'), "file.txta/b");
		TS_ASSERT_EQUALS(p4.append(p3).toString(), "file.txtabc/def");
	}

	void test_appendComponent() {
		Common::Path p("abc/def");
		Common::Path p2 = p.appendComponent("");
		TS_ASSERT_EQUALS(p2.toString(), "abc/def");

		Common::Path p3;
		Common::Path p4 = p3.appendComponent("file.txt");
		TS_ASSERT_EQUALS(p4.toString(), "file.txt");

		p2 = p.appendComponent("file.txt");
		TS_ASSERT_EQUALS(p2.toString(), "abc/def/file.txt");

		p2 = p.appendComponent("fi/le.txt");
		TS_ASSERT_EQUALS(p2.toString('\\'), "abc\\def\\fi/le.txt");

		Common::Path p5("abc\\de/f", '\\');
		Common::Path p6 = p5.appendComponent(Common::String("fi/le.txt"));
		TS_ASSERT_EQUALS(p6.toString('\\'), "abc\\de/f\\fi/le.txt");
	}

	void test_separator() {
		Common::Path p(TEST_PATH, '\\');
		TS_ASSERT_EQUALS(p.getLastComponent().toString(), TEST_PATH);
		TS_ASSERT_EQUALS(p.getParent().toString(), "");

		Common::Path p2(TEST_PATH, 'e');
		TS_ASSERT_EQUALS(p2.getLastComponent().toString(), ".txt");
		TS_ASSERT_EQUALS(p2.getParent().toString('#'), "par#nt/dir/fil#");
		TS_ASSERT_EQUALS(p2.getParent().getParent().toString('#'), "par#");
	}

	void test_splitComponents() {
		Common::Path p(TEST_PATH);
		Common::StringArray array = p.splitComponents();

		TS_ASSERT_EQUALS(array.size(), 3u);

		Common::StringArray::iterator iter = array.begin();

		TS_ASSERT_EQUALS(*iter, "parent");
		++iter;
		TS_ASSERT_EQUALS(*iter, "dir");
		++iter;
		TS_ASSERT_EQUALS(*iter, "file.txt");

		Common::Path p2;
		Common::StringArray array2 = p2.splitComponents();
		TS_ASSERT_EQUALS(array2.size(), 1u);
		Common::StringArray::iterator iter2 = array2.begin();
		TS_ASSERT_EQUALS(*iter2, "");

		Common::Path p3(TEST_ESCAPED1_PATH);
		Common::StringArray array3 = p3.splitComponents();

		TS_ASSERT_EQUALS(array3.size(), 3u);

		Common::StringArray::iterator iter3 = array3.begin();

		TS_ASSERT_EQUALS(*iter3, "|parent");
		++iter3;
		TS_ASSERT_EQUALS(*iter3, "dir");
		++iter3;
		TS_ASSERT_EQUALS(*iter3, "file.txt");

	}

	void test_joinComponents() {
		Common::StringArray array;
		Common::Path p = Common::Path::joinComponents(array);
		TS_ASSERT_EQUALS(p.toString(), "");

		array.push_back("");
		p = Common::Path::joinComponents(array);
		TS_ASSERT_EQUALS(p.toString(), "");

		Common::StringArray array2;
		array2.push_back("par/ent");
		array2.push_back("dir");
		array2.push_back("file.txt");
		p = Common::Path::joinComponents(array2);
		TS_ASSERT_EQUALS(p.toString('\\'), TEST_ESCAPED2_PATH);
	}

	void test_removeTrailingSeparators() {
		Common::Path p;
		p.removeTrailingSeparators();
		TS_ASSERT_EQUALS(p.toString(), "");

		Common::Path p2(TEST_PATH);
		p2.removeTrailingSeparators();
		TS_ASSERT_EQUALS(p2.toString(), TEST_PATH);

		Common::Path p3("parent/dir/file.txt///");
		p3.removeTrailingSeparators();
		TS_ASSERT_EQUALS(p3.toString(), "parent/dir/file.txt");

		Common::Path p4("//");
		p4.removeTrailingSeparators();
		TS_ASSERT_EQUALS(p4.toString(), "/");
	}

	void test_isRelativeTo() {
		Common::Path p, p1(TEST_PATH), p2(TEST_ESCAPED1_PATH);

		// Everything is relative to empty
		TS_ASSERT_EQUALS(p1.isRelativeTo(p), true);
		TS_ASSERT_EQUALS(p2.isRelativeTo(p), true);
		// Everything is relative to itself
		TS_ASSERT_EQUALS(p1.isRelativeTo(p1), true);
		TS_ASSERT_EQUALS(p2.isRelativeTo(p2), true);

		// A path is not relative to empty one
		TS_ASSERT_EQUALS(p.isRelativeTo(p1), false);

		TS_ASSERT_EQUALS(p1.isRelativeTo(Common::Path("parent/dir")), true);
		TS_ASSERT_EQUALS(p1.isRelativeTo(Common::Path("parent/dir/")), true);
		TS_ASSERT_EQUALS(p1.isRelativeTo(Common::Path("parent/dir/fi")), false);

		TS_ASSERT_EQUALS(p1.isRelativeTo(Common::Path("|parent/dir")), false);

		Common::Path p3("parent\\dir\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p3.isRelativeTo(Common::Path("parent/dir")), true);
		TS_ASSERT_EQUALS(p3.isRelativeTo(Common::Path("parent/dir/")), true);
		TS_ASSERT_EQUALS(p3.isRelativeTo(Common::Path("parent/dir/fi")), false);
		TS_ASSERT_EQUALS(p3.isRelativeTo(Common::Path("parent/dir/fa")), false);

		Common::Path p4("par|ent\\dir\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p4.isRelativeTo(Common::Path("par|ent/dir")), true);
	}

	void test_relativeTo() {
		Common::Path p, p1(TEST_PATH), p2(TEST_ESCAPED1_PATH);

		// Everything is relative to empty
		TS_ASSERT_EQUALS(p1.relativeTo(p).toString(), TEST_PATH);
		TS_ASSERT_EQUALS(p2.relativeTo(p).toString(), TEST_ESCAPED1_PATH);
		// Everything is relative to itself
		TS_ASSERT_EQUALS(p1.relativeTo(p1), p);
		TS_ASSERT_EQUALS(p2.relativeTo(p2), p);

		// A path is not relative to empty one
		TS_ASSERT_EQUALS(p.relativeTo(p1), p);

		TS_ASSERT_EQUALS(p1.relativeTo(Common::Path("parent/dir")).toString(), "file.txt");
		TS_ASSERT_EQUALS(p1.relativeTo(Common::Path("parent/dir/")).toString(), "file.txt");
		TS_ASSERT_EQUALS(p1.relativeTo(Common::Path("parent/dir/fi")), p1);

		TS_ASSERT_EQUALS(p1.relativeTo(Common::Path("|parent/dir")), p1);

		Common::Path p3("parent\\dir\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p3.relativeTo(Common::Path("parent/dir")).toString('\\'), "fi/le.txt");
		TS_ASSERT_EQUALS(p3.relativeTo(Common::Path("parent/dir/")).toString('\\'), "fi/le.txt");
		TS_ASSERT_EQUALS(p3.relativeTo(Common::Path("parent/dir/fi")), p3);
		TS_ASSERT_EQUALS(p3.relativeTo(Common::Path("parent/dir/fa")), p3);

		Common::Path p4("par|ent\\dir\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p4.relativeTo(Common::Path("par|ent/dir")).toString('\\'), "fi/le.txt");

		Common::Path p5("par|ent\\dir\\\\\\fi/le.txt", '\\');
		TS_ASSERT_EQUALS(p5.relativeTo(Common::Path("par|ent/dir")).toString('\\'), "fi/le.txt");
	}

	void test_normalize() {
		TS_ASSERT_EQUALS(Common::Path("/", '/').normalize().toString(), "/");
		TS_ASSERT_EQUALS(Common::Path("///", '/').normalize().toString(), "/");
		TS_ASSERT_EQUALS(Common::Path("/foo/bar", '/').normalize().toString(), "/foo/bar");
		TS_ASSERT_EQUALS(Common::Path("/foo//bar/", '/').normalize().toString(), "/foo/bar");
		TS_ASSERT_EQUALS(Common::Path("/foo/./bar", '/').normalize().toString(), "/foo/bar");
		TS_ASSERT_EQUALS(Common::Path("/foo//./bar//", '/').normalize().toString(), "/foo/bar");
		TS_ASSERT_EQUALS(Common::Path("/foo//.bar//", '/').normalize().toString(), "/foo/.bar");

		TS_ASSERT_EQUALS(Common::Path("", '/').normalize().toString(), "");
		TS_ASSERT_EQUALS(Common::Path("foo/bar", '/').normalize().toString(), "foo/bar");
		TS_ASSERT_EQUALS(Common::Path("foo//bar/", '/').normalize().toString(), "foo/bar");
		TS_ASSERT_EQUALS(Common::Path("foo/./bar", '/').normalize().toString(), "foo/bar");
		TS_ASSERT_EQUALS(Common::Path("foo//./bar//", '/').normalize().toString(), "foo/bar");
		TS_ASSERT_EQUALS(Common::Path("foo//.bar//", '/').normalize().toString(), "foo/.bar");

		TS_ASSERT_EQUALS(Common::Path("..", '/').normalize().toString(), "..");
		TS_ASSERT_EQUALS(Common::Path("../", '/').normalize().toString(), "..");
		TS_ASSERT_EQUALS(Common::Path("/..", '/').normalize().toString(), "/..");
		TS_ASSERT_EQUALS(Common::Path("../bar", '/').normalize().toString(), "../bar");
		TS_ASSERT_EQUALS(Common::Path("foo//../", '/').normalize().toString(), "");
		TS_ASSERT_EQUALS(Common::Path("foo/../bar", '/').normalize().toString(), "bar");
		TS_ASSERT_EQUALS(Common::Path("foo//../bar//", '/').normalize().toString(), "bar");
		TS_ASSERT_EQUALS(Common::Path("foo//..bar//", '/').normalize().toString(), "foo/..bar");

		TS_ASSERT_EQUALS(Common::Path("foo/../../bar//", '/').normalize().toString(), "../bar");
		TS_ASSERT_EQUALS(Common::Path("../foo/../bar", '/').normalize().toString(), "../bar");
		TS_ASSERT_EQUALS(Common::Path("../../foo/bar/", '/').normalize().toString(), "../../foo/bar");

		TS_ASSERT_EQUALS(Common::Path("foo/../|bar", '/').normalize().toString(), "|bar");
		TS_ASSERT_EQUALS(Common::Path("foo/^..^bar", '^').normalize().toString('^'), "bar");
		TS_ASSERT_EQUALS(Common::Path("foo^..^bar/", '^').normalize().toString('^'), "bar/");
	}

	void test_punycode() {
		Common::Path p;
		Common::Path p2 = p.punycodeDecode();
		TS_ASSERT_EQUALS(p.toString(), "");

		Common::Path p3("parent/dir/xn--Sound Manager 3.1  SoundLib-lba84k/Sound");
		Common::Path p4 = p3.punycodeDecode();

		TS_ASSERT_EQUALS(p4.toString(':'), "parent:dir:Sound Manager 3.1 / SoundLib:Sound");
		Common::Path p5 = p4.punycodeEncode();
		TS_ASSERT_EQUALS(p5.toString('/'), "parent/dir/xn--Sound Manager 3.1  SoundLib-lba84k/Sound");

		typedef Common::HashMap<Common::Path, bool,
				Common::Path::IgnoreCaseAndMac_Hash, Common::Path::IgnoreCaseAndMac_EqualTo> TestPathMap;
		TestPathMap map;

		map.setVal(p3, false);
		TS_ASSERT_EQUALS(map.size(), 1u);
		map.setVal(p4, false);
		TS_ASSERT_EQUALS(map.size(), 1u);
		map.setVal(p5, false);
		TS_ASSERT_EQUALS(map.size(), 1u);

		map.setVal(p, false);
		TS_ASSERT_EQUALS(map.size(), 2u);
	}

	void test_lowerupper() {
		Common::Path p2(TEST_PATH);
		p2.toUppercase();
		TS_ASSERT_EQUALS(p2.toString('/'), "PARENT/DIR/FILE.TXT");

		p2 = TEST_PATH;
		p2.toLowercase();
		TS_ASSERT_EQUALS(p2.toString('/'), TEST_PATH);

		Common::Path p3(TEST_ESCAPED1_PATH);
		p3.toUppercase();
		TS_ASSERT_EQUALS(p3.toString('/'), "|PARENT/DIR/FILE.TXT");

		Common::String s3(TEST_ESCAPED1_PATH);
		s3.toUppercase();
		p3 = s3;
		p3.toLowercase();
		TS_ASSERT_EQUALS(p3.toString('/'), TEST_ESCAPED1_PATH);
	}

	void test_caseinsensitive() {
		Common::Path p;
		Common::Path p2("parent:dir:Sound Manager 3.1 / SoundLib:Sound", ':');
		Common::Path p3("parent:dir:sound manager 3.1 / soundlib:sound", ':');
		Common::Path p4("parent/dir/xn--Sound Manager 3.1  SoundLib-lba84k/Sound");

		typedef Common::HashMap<Common::Path, bool,
				Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> TestPathMap;
		TestPathMap map;

		map.setVal(p2, false);
		TS_ASSERT_EQUALS(map.size(), 1u);
		map.setVal(p3, false);
		TS_ASSERT_EQUALS(map.size(), 1u);
		map.setVal(p4, false);
		TS_ASSERT_EQUALS(map.size(), 2u);

		map.setVal(p, false);
		TS_ASSERT_EQUALS(map.size(), 3u);
	}

	void test_casesensitive() {
		Common::Path p2("parent:dir:Sound Manager 3.1 / SoundLib:Sound", ':');
		Common::Path p3("parent:dir:sound manager 3.1 / soundlib:sound", ':');
		Common::Path p4("parent/dir/xn--Sound Manager 3.1  SoundLib-lba84k/Sound");

		TS_ASSERT_DIFFERS(p2.hash(), p3.hash());
		TS_ASSERT_DIFFERS(p2.hash(), p4.hash());
		TS_ASSERT_DIFFERS(p3.hash(), p4.hash());
	}

	void test_matchString() {
		TS_ASSERT(Common::Path("").matchPattern(""));
		TS_ASSERT(Common::Path("a").matchPattern("*"));
		TS_ASSERT(Common::Path("monkey.s01").matchPattern("*"));

		TS_ASSERT(!Common::Path("").matchPattern("?"));
		TS_ASSERT(Common::Path("a").matchPattern("?"));
		TS_ASSERT(!Common::Path("monkey.s01").matchPattern("?"));

		TS_ASSERT(Common::Path("monkey.s01").matchPattern("monkey.s??"));
		TS_ASSERT(Common::Path("monkey.s99").matchPattern("monkey.s??"));
		TS_ASSERT(!Common::Path("monkey.s101").matchPattern("monkey.s??"));

		TS_ASSERT(Common::Path("monkey.s01").matchPattern("monkey.s?1"));
		TS_ASSERT(!Common::Path("monkey.s99").matchPattern("monkey.s?1"));
		TS_ASSERT(!Common::Path("monkey.s101").matchPattern("monkey.s?1"));

		TS_ASSERT(Common::Path("monkey.s01").matchPattern("monkey.s*"));
		TS_ASSERT(Common::Path("monkey.s99").matchPattern("monkey.s*"));
		TS_ASSERT(Common::Path("monkey.s101").matchPattern("monkey.s*"));

		TS_ASSERT(Common::Path("monkey.s01").matchPattern("monkey.s*1"));
		TS_ASSERT(!Common::Path("monkey.s99").matchPattern("monkey.s*1"));
		TS_ASSERT(Common::Path("monkey.s101").matchPattern("monkey.s*1"));

		TS_ASSERT(Common::Path("monkey.s01").matchPattern("monkey.s##"));
		TS_ASSERT(!Common::Path("monkey.s01").matchPattern("monkey.###"));

		TS_ASSERT(Common::Path("monkey.s0#").matchPattern("monkey.s0\\#"));
		TS_ASSERT(!Common::Path("monkey.s0#").matchPattern("monkey.s0#"));
		TS_ASSERT(!Common::Path("monkey.s01").matchPattern("monkey.s0\\#"));

		TS_ASSERT(Common::Path("test/monkey.s01").matchPattern("*/*"));
		TS_ASSERT(!Common::Path("test/monkey.s01").matchPattern("|test/*"));
		TS_ASSERT(Common::Path("|test/monkey.s01").matchPattern("*/*"));
		TS_ASSERT(!Common::Path("test/monkey.s01").matchPattern("*"));
		TS_ASSERT(!Common::Path("test/monkey.s01").matchPattern(Common::Path("test\\fi/le.txt", '\\')));
	}

	// These tests are here to exercise cases currently not used
	// This allow to reach 100% code coverage (except unescape error)
	void test_findLastSeparator() {
		Common::Path p(TEST_PATH);
		TS_ASSERT_EQUALS(p.findLastSeparator(), 10u);

		Common::Path p2("file.txt");
		TS_ASSERT_EQUALS(p2.findLastSeparator(), Common::String::npos);
	}

	void test_extract() {
		Common::Path p(TEST_PATH);
		Common::Path p2 = p.extract(p._str.c_str(), p._str.c_str());
		TS_ASSERT_EQUALS(p2, Common::Path());

		Common::Path p3("file.txt");
		TS_ASSERT_EQUALS(p3.extract(p3._str.c_str()), p3);
	}

	void test_canUnescape() {
		TS_ASSERT(Common::Path::canUnescape(true, true, ""));
	}
};
