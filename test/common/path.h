#include <cxxtest/TestSuite.h>

#include "common/path.h"

static const char *TEST_PATH = "parent/dir/file.txt";

class PathTestSuite : public CxxTest::TestSuite
{
	public:
	void test_Path() {
		Common::Path p;
		TS_ASSERT_EQUALS(p.toString(), Common::String());

		Common::Path p2(TEST_PATH);
		TS_ASSERT_EQUALS(p2.toString(), Common::String(TEST_PATH));
	}

	void test_getLastComponent() {
		Common::Path p(TEST_PATH);
		TS_ASSERT_EQUALS(p.getLastComponent().toString(), "file.txt");
	}

	void test_getParent() {
		Common::Path p(TEST_PATH);
		TS_ASSERT_EQUALS(p.getParent().toString(), "parent/dir/");
		// TODO: should this work?
		TS_ASSERT_EQUALS(p.getParent().getLastComponent().toString(), "dir/");
	}

	void test_join() {
		Common::Path p("dir");
		Common::Path p2 = p.join("file.txt");
		TS_ASSERT_EQUALS(p2.toString(), "dir/file.txt");

		Common::Path p3(TEST_PATH);
		Common::Path p4 = p3.getParent().join("other.txt");
		TS_ASSERT_EQUALS(p4.toString(), "parent/dir/other.txt");
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

	void test_normalize() {
		TS_ASSERT_EQUALS(Common::Path("/", '/').normalize().toString(), "/");
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
	}
};
