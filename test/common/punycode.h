#include <cxxtest/TestSuite.h>
#include "common/punycode.h"

/**
 * Test suite for the functions in common/util.h
 */
static const char *strings[] = {
	"Icon\r", "xn--Icon-ja6e", "1",
	"ascii", "ascii", "0",
	"ends with dot .", "xn--ends with dot .-", "1",
	"ends with space ", "xn--ends with space -", "1",
	"バッドデイ(Power PC)", "xn--(Power PC)-jx4ilmwb1a7h", "1",
	"Hello*", "xn--Hello-la10a", "1",
	"File I/O", "xn--File IO-oa82b", "1",
	"HDにｺﾋﾟｰして下さい。G3", "xn--HDG3-rw3c5o2dpa9kzb2170dd4tzyda5j4k", "1",
	"Buried in Time™ Demo", "xn--Buried in Time Demo-eo0l", "1",
	"•Main Menu", "xn--Main Menu-zd0e", "1",
	"Spaceship Warlock™", "xn--Spaceship Warlock-306j", "1",
	"ワロビージャックの大冒険<デモ>", "xn--baa0pja0512dela6bueub9gshf1k1a1rt742c060a2x4u", "1",
	"Jönssonligan går på djupet.exe", "xn--Jnssonligan gr p djupet.exe-glcd70c", "1",
	"Jönssonligan.exe", "xn--Jnssonligan.exe-8sb", "1",
	"G3フォルダ", "xn--G3-3g4axdtexf", "1",
	"Big[test]", "Big[test]", "0",
	"Where \\ Do <you> Want / To: G* ? ;Unless=nowhere,or|\"(everything)/\":*|\\?%<>,;=", "xn--Where  Do you Want  To G  ;Unless=nowhere,or(everything),;=-5baedgdcbtamaaaaaaaaa99woa3wnnmb82aqb71ekb9g3c1f1cyb7bx6rfcv2pxa", "1",
	"Buried in Timeｪ Demo", "xn--Buried in Time Demo-yp97h", "1",
	0
};
class PunycodeTestSuite : public CxxTest::TestSuite {
	public:

	void test_punycode() {
		for (const char **a = strings; *a; a += 3) {
			Common::U32String string_in(a[0]);
			Common::String string_out(a[1]);
			bool need = (a[2][0] == '1');

			TS_ASSERT_EQUALS(punycode_decodefilename(string_out), string_in);
			TS_ASSERT_EQUALS(punycode_encodefilename(string_in), string_out);
			TS_ASSERT_EQUALS(punycode_needEncode(string_in), need);
		}
	}
};
