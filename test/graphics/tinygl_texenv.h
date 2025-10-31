#include <cxxtest/TestSuite.h>

#ifdef USE_TINYGL

#include "graphics/tinygl/tinygl.h"

// every test sets up some texture environment
// then draws a single pixel and checks the resulting output pixel

class TinyGLTexEnvTestSuite : public CxxTest::TestSuite {
    TinyGL::ContextHandle *_context = nullptr;
public:
    void setUp() {
        _context = TinyGL::createContext(2, 2, Graphics::PixelFormat::createFormatARGB32(), 2, false, false);
        TinyGL::setContext(_context);

        tglEnable(TGL_TEXTURE_2D);
        tglDisable(TGL_BLEND);
        tglDisable(TGL_DEPTH_TEST);
        tglMatrixMode(TGL_PROJECTION);
        tglLoadIdentity();
        tglMatrixMode(TGL_MODELVIEW);
        tglLoadIdentity();
		tglViewport(0, 0, 2, 2);
    }

    void tearDown() {
        if (_context != nullptr) {
            TinyGL::destroyContext(_context);
            _context = nullptr;
        }
    }

	// these three functions use RGBA order instead of ARGB to make it consistent with tglColor4ub which we also call

    void setConstant(byte r, byte g, byte b, byte a) {
        const float values[] = { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        tglTexEnvfv(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_COLOR, values);
    }

    void drawPixel(byte texR, byte texG, byte texB, byte texA) {
		const float S = 10.0f;
        const byte texData[] = { texR, texG, texB, texA };
        TGLuint texture;
        tglGenTextures(1, &texture);
        tglBindTexture(TGL_TEXTURE_2D, texture);
        tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP);
        tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP);
        tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, 1, 1, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, &texData);
        tglBegin(TGL_TRIANGLES);
        tglTexCoord2f(0.0f, 0.0f); tglVertex2f(-S, -S);
        tglTexCoord2f(1.0f, 0.0f); tglVertex2f(+S, -S);
        tglTexCoord2f(0.5f, 1.0f); tglVertex2f(0, +S);
        tglEnd();
    }

    void checkOutput(byte expR, byte expG, byte expB, byte expA) {
        byte actA, actR, actG, actB;
        Graphics::Surface surface;
        TinyGL::presentBuffer();
        TinyGL::getSurfaceRef(surface);
        surface.format.colorToARGB(surface.getPixel(0, 0), actA, actR, actG, actB);

        TS_ASSERT_EQUALS(expA, actA);
        TS_ASSERT_EQUALS(expR, actR);
        TS_ASSERT_EQUALS(expG, actG);
        TS_ASSERT_EQUALS(expB, actB);
    }

    void testModulate() {
        // no tglTexEnvi setup because TGL_MODULATE should be the default
        tglColor4ub(255, 255, 255, 127);
        drawPixel(255, 127, 0, 255);
        checkOutput(255, 127, 0, 127);
    }

    void testReplace() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_REPLACE);
        tglColor4ub(255, 255, 255, 127);
        drawPixel(255, 127, 0, 255);
        checkOutput(255, 127, 0, 255);
    }

    void testDecal() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
        tglColor4ub(100, 200, 255, 123);
        drawPixel(200, 100, 0, 192);
        checkOutput(176, 125, 63, 123);
    }

    void testAdd() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_ADD);
        tglColor4ub(50, 100, 150, 127);
        drawPixel(25, 50, 150, 200);
        checkOutput(75, 150, 255, 100);
        // attention: TGL_ADD still modulates alpha
    }

    void testBlend() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_BLEND);
        tglColor4ub(210, 210, 200, 127);
        setConstant(123, 123, 100, 42);
        drawPixel(0, 255, 128, 200);
        checkOutput(211, 123, 150, 100);
		// the 211 is an unfortunate rounding problem
    }

    void setCombineMode(TGLuint combineRGB, TGLuint combineAlpha) {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_COMBINE);
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_RGB, combineRGB);
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_ALPHA, combineAlpha);
    }

    void setCombineArg(int arg, TGLuint rgbSource, TGLuint rgbOperand, TGLuint alphaSource, TGLuint alphaOperand) {
        assert(arg >= 0 && arg <= 1);
        tglTexEnvi(TGL_TEXTURE_ENV, arg ? TGL_SOURCE1_RGB : TGL_SOURCE0_RGB, rgbSource);
        tglTexEnvi(TGL_TEXTURE_ENV, arg ? TGL_SOURCE1_ALPHA : TGL_SOURCE0_ALPHA, alphaSource);
        tglTexEnvi(TGL_TEXTURE_ENV, arg ? TGL_OPERAND1_RGB : TGL_OPERAND0_RGB, rgbOperand);
        tglTexEnvi(TGL_TEXTURE_ENV, arg ? TGL_OPERAND1_ALPHA : TGL_OPERAND0_ALPHA, alphaOperand);
    }

    void testCombineArgTexture() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_TEXTURE, TGL_SRC_COLOR, TGL_TEXTURE, TGL_SRC_ALPHA);
        drawPixel(13, 37, 42, 24);
        checkOutput(13, 37, 42, 24);
    }

    void testCombineArgPrimaryColor() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_PRIMARY_COLOR, TGL_SRC_COLOR, TGL_PRIMARY_COLOR, TGL_SRC_ALPHA);
        tglColor4ub(12, 34, 56, 78);
        drawPixel(13, 37, 42, 24);
        checkOutput(12, 34, 56, 78);
    }

    void testCombineArgConstant() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_CONSTANT, TGL_SRC_COLOR, TGL_CONSTANT, TGL_SRC_ALPHA);
        setConstant(12, 34, 56, 78);
        drawPixel(13, 37, 42, 24);
        checkOutput(12, 34, 56, 78);
    }

	void testCombineArgMixed() {
		setCombineMode(TGL_REPLACE, TGL_REPLACE);
		setCombineArg(0, TGL_PRIMARY_COLOR, TGL_SRC_ALPHA, TGL_TEXTURE, TGL_SRC_ALPHA);
		tglColor4ub(12, 34, 56, 78);
		drawPixel(13, 37, 42, 24);
		checkOutput(78, 78, 78, 24);
	}

    void testCombineOpOneMinus() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_TEXTURE, TGL_ONE_MINUS_SRC_COLOR, TGL_TEXTURE, TGL_ONE_MINUS_SRC_ALPHA);
        drawPixel(13, 37, 42, 24);
        checkOutput(242, 218, 213, 231);
    }

    void testCombineOpSrcAlpha() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_TEXTURE, TGL_SRC_ALPHA, TGL_TEXTURE, TGL_ONE_MINUS_SRC_ALPHA);
        drawPixel(13, 37, 42, 24);
        checkOutput(24, 24, 24, 231);
    }

    void testCombineReplace() {
        setCombineMode(TGL_REPLACE, TGL_REPLACE);
        setCombineArg(0, TGL_TEXTURE, TGL_SRC_COLOR, TGL_TEXTURE, TGL_SRC_ALPHA);
        setCombineArg(1, TGL_PRIMARY_COLOR, TGL_SRC_COLOR, TGL_PRIMARY_COLOR, TGL_SRC_ALPHA);
        tglColor4ub(12, 34, 56, 78); // just to confuse the implementation
        drawPixel(13, 37, 42, 24);
        checkOutput(13, 37, 42, 24);
    }

    void testCombineModulate() {
        setCombineMode(TGL_MODULATE, TGL_MODULATE);
        setCombineArg(0, TGL_TEXTURE, TGL_SRC_COLOR, TGL_TEXTURE, TGL_SRC_ALPHA);
        setCombineArg(1, TGL_PRIMARY_COLOR, TGL_SRC_COLOR, TGL_PRIMARY_COLOR, TGL_SRC_ALPHA);
        tglColor4ub(255, 255, 255, 127);
        drawPixel(255, 127, 0, 255);
        checkOutput(255, 127, 0, 127);
    }

    void testCombineAdd() {
        setCombineMode(TGL_ADD, TGL_ADD);
        setCombineArg(0, TGL_TEXTURE, TGL_SRC_COLOR, TGL_TEXTURE, TGL_SRC_ALPHA);
        setCombineArg(1, TGL_PRIMARY_COLOR, TGL_SRC_COLOR, TGL_PRIMARY_COLOR, TGL_SRC_ALPHA);
        tglColor4ub(50, 100, 150, 100);
        drawPixel(25, 50, 150, 200);
        checkOutput(75, 150, 255, 255);
        // attention: TGL_COMBINE TGL_ADD does *not* modulate alpha
    }

	void testSaveTexEnvState() {
		tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_REPLACE);
		tglColor4ub(0, 0, 0, 0);
		drawPixel(255, 127, 0, 255);
		tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_MODULATE); // before executing the drawcall
		checkOutput(255, 127, 0, 255);
	}
};

#endif
