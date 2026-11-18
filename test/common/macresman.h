#include <cxxtest/TestSuite.h>
#include "common/macresman.h"

/**
 * Test suite for the file name handling in common/macresman.h
 */
class MacResManagerTestSuite : public CxxTest::TestSuite {
	public:

	void checkName(const Common::Path &name, const Common::Path &expected, bool expectedDecorated) {
		bool decorated = false;
		Common::Path result = Common::MacResManager::disassembleName(name, &decorated);

		TS_ASSERT_EQUALS(result, expected);
		TS_ASSERT_EQUALS(decorated, expectedDecorated);
	}

	void test_disassembleName() {
		// Plain names pass through untouched
		checkName("JMP PP Resources", "JMP PP Resources", false);
		checkName("data.dat", "data.dat", false);

		// MacBinary ".bin" extension, case-insensitive (bug #17026)
		checkName("JMP PP Resources.bin", "JMP PP Resources", true);
		checkName("Foo.BIN", "Foo", true);

		// Raw resource fork / AppleDouble ".rsrc" extension
		checkName("JMP PP Resources.rsrc", "JMP PP Resources", true);

		// AppleDouble "._" prefix
		checkName("._JMP PP Resources", "JMP PP Resources", true);

		// Decorations on the last component of a longer path
		checkName("Data/._Movie", "Data/Movie", true);
		checkName("Data/Movie.bin", "Data/Movie", true);

		// Both prefix and extension are stripped
		checkName("._Foo.rsrc", "Foo", true);

		// Substrings that are not decorations are kept
		checkName("binfile", "binfile", false);
		checkName("x._y", "x._y", false);
	}
};
