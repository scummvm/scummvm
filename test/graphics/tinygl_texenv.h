#include <cxxtest/TestSuite.h>

#ifdef USE_TINYGL

#include "graphics/tinygl/tinygl.h"

// every test sets up some texture environment
// then draws a single pixel and checks the resulting output pixel

class TinyGLTexEnvTestSuite : public CxxTest::TestSuite {
    TinyGL::ContextHandle *_context = nullptr;
public:
    void setUp() {
        _context = TinyGL::createContext(1, 1, Graphics::PixelFormat::createFormatARGB32(), 1, false, false);
        TinyGL::setContext(_context);

        tglEnable(TGL_TEXTURE_2D);
        tglDisable(TGL_BLEND);
        tglDisable(TGL_DEPTH_TEST);
        tglMatrixMode(TGL_PROJECTION);
        tglLoadIdentity();
        tglMatrixMode(TGL_MODELVIEW);
        tglLoadIdentity();
    }

    void tearDown() {
        if (_context != nullptr) {
            TinyGL::destroyContext(_context);
            _context = nullptr;
        }
    }

    void drawPixel(byte texA, byte texR, byte texG, byte texB) {
        const float S = 3.0f; ///< such that a single triangle will fill the entire clip space
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

    void checkOutput(byte expA, byte expR, byte expG, byte expB) {
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
        tglColor4ub(255, 255, 255, 128);
        drawPixel(255, 128, 0, 255);
        checkOutput(255, 128, 0, 128);
    }

    void testReplace() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_REPLACE);
        tglColor4ub(255, 255, 255, 128);
        drawPixel(255, 128, 0, 255);
        checkOutput(255, 128, 0, 255);
    }

    void testDecal() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
        tglColor4ub(100, 200, 255, 123);
        drawPixel(200, 100, 0, 192);
        checkOutput(175, 125, 67, 123);
    }

    void testAdd() {
        tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_ADD);
        tglColor4ub(50, 100, 150, 100);
        drawPixel(25, 50, 150, 200);
        checkOutput(75, 150, 255, (100 + 200) >> 8);
        // attention: TGL_ADD still modulates alpha
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
        setCombineArg(0, TGL_PRIMARY_COLOR, TGL_SRC_COLOR, TGL_PRIMARY_COLOR, TGL_SRC_COLOR);
        tglColor4ub(12, 34, 56, 78);
        drawPixel(13, 37, 42, 24);
        checkOutput(12, 34, 56, 78);
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
        tglColor4ub(255, 255, 255, 128);
        drawPixel(255, 128, 0, 255);
        checkOutput(255, 128, 0, 128);
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
};

#endif
