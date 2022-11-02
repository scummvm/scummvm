#include <cxxtest/TestSuite.h>
#include "common/rendermode.h"
#include "common/gui_options.h"
#include "common/str.h"

class RenderModeTestSuite : public CxxTest::TestSuite {
	public:
	void test_parse_render_mode_good() {
		/*
		 * Tests for parseRenderMode.
		 * It takes a code (case-insensitive) and spits a RenderMode back at you.
		 * These cases should work - the inputs are standard, there's just some
		 * fun with caps being had in here.
		 */
		TS_ASSERT_EQUALS(Common::parseRenderMode("fMTOwNs"), Common::kRenderFMTowns);
		TS_ASSERT_EQUALS(Common::parseRenderMode("hercGrEen"), Common::kRenderHercG);
		TS_ASSERT_EQUALS(Common::parseRenderMode("hercAmbeR"), Common::kRenderHercA);
		TS_ASSERT_EQUALS(Common::parseRenderMode("CgA"), Common::kRenderCGA);
		TS_ASSERT_EQUALS(Common::parseRenderMode("ega"), Common::kRenderEGA);
		TS_ASSERT_EQUALS(Common::parseRenderMode("Vga"), Common::kRenderVGA);
		TS_ASSERT_EQUALS(Common::parseRenderMode("AmigA"), Common::kRenderAmiga);
		TS_ASSERT_EQUALS(Common::parseRenderMode("pc9821"), Common::kRenderPC9821);
		TS_ASSERT_EQUALS(Common::parseRenderMode("PC9801"), Common::kRenderPC9801);
		TS_ASSERT_EQUALS(Common::parseRenderMode("0"), Common::kRenderDefault);
	}


	void test_parse_render_mode_bad() {
		/*
		 * These cases, according to the specification, should return the default.
		 * It is only mentioned that the function must be case insensitive.
		 * Whitespaces, in particular, should not be automatically trimmed.
		 */
		TS_ASSERT_EQUALS(Common::parseRenderMode("fmtowns "), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode("FM-TOWNS "), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode(" cga"), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode("\tC g A"), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode("\t"), Common::kRenderDefault);
		// This is the only interesting bit: if the function was really, really
		// broken it could be tempted to test for +-0x20.
		TS_ASSERT_EQUALS(Common::parseRenderMode("pc Y8 21 "), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode(" PC\t9801 "), Common::kRenderDefault);
		TS_ASSERT_EQUALS(Common::parseRenderMode("0"), Common::kRenderDefault);
	}

	void test_get_render_mode_code_back_and_forth() {
		/*
		 * What does getRenderModeCode return?
		 * Notably, the output should not be in mixed case.
		 */
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("FMTOWNS")), "fmtowns", 7);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("CGA")), "cga", 3);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("vga")), "vga", 3);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("Ega")), "ega", 3);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("AmiGa")), "amiga", 5);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("PC9821")), "pc9821", 6);
		TS_ASSERT_SAME_DATA(Common::getRenderModeCode(Common::parseRenderMode("PC9801")), "pc9801", 6);
		// Slightly more interesting:
		// Make sure that we get a null pointer for 0 (and not the "0" string or stuff)
		char *null_p = 0;
		TS_ASSERT_EQUALS(Common::getRenderModeCode(Common::kRenderDefault), null_p);
	}

	void test_render_2_guio() {
		/*
		 * Verify that a rendermode is taken and the corresponding
		 * GUIO_xxxxx is returned.
		 */
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderHercG), GUIO_RENDERHERCGREEN);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderHercA), GUIO_RENDERHERCAMBER);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderCGA), GUIO_RENDERCGA);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderEGA), GUIO_RENDEREGA);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderVGA), GUIO_RENDERVGA);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderAmiga), GUIO_RENDERAMIGA);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderFMTowns), GUIO_RENDERFMTOWNS);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderPC9821), GUIO_RENDERPC9821);
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderPC9801), GUIO_RENDERPC9801);
		// renderMode2GUIO is supposed to return an empty string
		// if given kRenderDefault as an argument
		Common::String empty;
		TS_ASSERT_EQUALS(Common::renderMode2GUIO(Common::kRenderDefault), empty);
	}
};
