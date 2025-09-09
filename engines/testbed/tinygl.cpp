/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "engines/util.h"
#include "image/png.h"
#include "graphics/managed_surface.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/debug.h"
#include "graphics/tinygl/tinygl.h"

#include "testbed/tinygl.h"

namespace Testbed {

namespace TinyGLTests {
	struct TextureEnvironmentArg {
		GLuint _sourceRGB, _operandRGB;
		GLuint _sourceAlpha, _operandAlpha;

		TextureEnvironmentArg(
			GLuint sourceRGB = GL_TEXTURE,
			GLuint operandRGB = GL_SRC_COLOR,
			GLuint sourceAlpha = GL_TEXTURE,
			GLuint operandAlpha = GL_SRC_ALPHA)
			: _sourceRGB(sourceRGB)
			, _operandRGB(operandRGB)
			, _sourceAlpha(sourceAlpha)
			, _operandAlpha(operandAlpha) { }
	};

	struct TextureEnvironment {
		GLuint _mode, _combineRGB, _combineAlpha;
		TextureEnvironmentArg _arg0, _arg1;
		byte _constantR = 255, _constantG = 255, _constantB = 255, _constantA = 255;

		TextureEnvironment(
			GLuint mode = GL_REPLACE,
			GLuint combineRGB = GL_REPLACE,
			GLuint combineAlpha = GL_REPLACE)
			: _mode(mode)
			, _combineRGB(combineRGB)
			, _combineAlpha(combineAlpha) { }

		template<typename TexEnvFunci, typename TexEnvFuncfv>
		void apply(TexEnvFunci funci, TexEnvFuncfv funcfv) const {
			funci(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
			funci(GL_TEXTURE_ENV, GL_COMBINE_RGB, _combineRGB);
			funci(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, _combineAlpha);

			funci(GL_TEXTURE_ENV, GL_SOURCE0_RGB, _arg0._sourceRGB);
			funci(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, _arg0._sourceAlpha);
			funci(GL_TEXTURE_ENV, GL_SOURCE1_RGB, _arg1._sourceRGB);
			funci(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, _arg1._sourceAlpha);

			funci(GL_TEXTURE_ENV, GL_OPERAND0_RGB, _arg0._operandRGB);
			funci(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, _arg0._operandAlpha);
			funci(GL_TEXTURE_ENV, GL_OPERAND1_RGB, _arg1._operandRGB);
			funci(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, _arg1._operandAlpha);

			const float values[] = {
				_constantR / 255.0f,
				_constantG / 255.0f,
				_constantB / 255.0f,
				_constantA / 255.0f
			};
			funcfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, values);
		}
	};

	TestExitStatus runTexEnvTest(
		const char *testName,
		const TextureEnvironment &env,
		byte colorR, byte colorG, byte colorB, byte colorA);
	TestExitStatus testTexEnvReplace();
	TestExitStatus testTexEnvModulate();
	TestExitStatus testTexEnvDecal();
	TestExitStatus testTexEnvAdd();
	TestExitStatus testTexEnvBlend();
	TestExitStatus testTexEnvCombineOpNormal();
	TestExitStatus testTexEnvCombineOpInverse();
	TestExitStatus testTexEnvCombineOpAlphaToColor();
	TestExitStatus testTexEnvCombineMixedArgs();
	TestExitStatus testTexEnvCombineReplace();
	TestExitStatus testTexEnvCombineModulate();
	TestExitStatus testTexEnvCombineAdd();
}

TinyGLTestSuite::TinyGLTestSuite() {
	addTest("Replace", &TinyGLTests::testTexEnvReplace);
	addTest("Modulate", &TinyGLTests::testTexEnvModulate);
	addTest("Decal", &TinyGLTests::testTexEnvDecal);
	addTest("Add", &TinyGLTests::testTexEnvAdd);
	addTest("Blend", &TinyGLTests::testTexEnvBlend);
	addTest("CombineOpNormal", &TinyGLTests::testTexEnvCombineOpNormal);
	addTest("CombineOpInverse", &TinyGLTests::testTexEnvCombineOpInverse);
	addTest("CombineOpAlphaToColor", &TinyGLTests::testTexEnvCombineOpAlphaToColor);
	addTest("CombineMixedArgs", &TinyGLTests::testTexEnvCombineMixedArgs);
	addTest("CombineReplace", &TinyGLTests::testTexEnvCombineReplace);
	addTest("CombineModulate", &TinyGLTests::testTexEnvCombineModulate);
	addTest("CombineAdd", TinyGLTests::testTexEnvCombineAdd);
}

TestExitStatus TinyGLTests::testTexEnvModulate() {
	TextureEnvironment env(GL_MODULATE);
	return runTexEnvTest("Modulate", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvReplace() {
	TextureEnvironment env(GL_REPLACE);
	return runTexEnvTest("Replace", env, 255, 0, 255, 255);
}

TestExitStatus TinyGLTests::testTexEnvDecal() {
	TextureEnvironment env(GL_DECAL);
	return runTexEnvTest("Decal", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvAdd() {
	TextureEnvironment env(GL_ADD);
	return runTexEnvTest("Add", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvBlend() {
	TextureEnvironment env(GL_BLEND);
	env._constantR = 250;
	env._constantG = 150;
	env._constantB = 100;
	env._constantA = 50;
	return runTexEnvTest("Blend", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvCombineOpNormal() {
	TextureEnvironment env(GL_COMBINE, GL_REPLACE, GL_REPLACE);
	env._arg0 = { GL_TEXTURE, GL_SRC_COLOR, GL_TEXTURE, GL_SRC_ALPHA };
	return runTexEnvTest("CombineOpNormal", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvCombineOpInverse() {
	TextureEnvironment env(GL_COMBINE, GL_REPLACE, GL_REPLACE);
	env._arg0 = { GL_TEXTURE, GL_ONE_MINUS_SRC_COLOR, GL_TEXTURE, GL_ONE_MINUS_SRC_ALPHA };
	return runTexEnvTest("CombineOpInverse", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvCombineOpAlphaToColor() {
	TextureEnvironment env(GL_COMBINE, GL_REPLACE, GL_REPLACE);
	env._arg0 = { GL_TEXTURE, GL_SRC_ALPHA, GL_TEXTURE, GL_ONE_MINUS_SRC_ALPHA };
	return runTexEnvTest("CombineOpAlphaToColor", env, 50, 111, 222, 255);
}

TestExitStatus TinyGLTests::testTexEnvCombineMixedArgs() {
	TextureEnvironment env(GL_COMBINE, GL_REPLACE, GL_REPLACE);
	env._arg0 = { GL_PRIMARY_COLOR, GL_SRC_ALPHA, GL_TEXTURE, GL_SRC_ALPHA };
	return runTexEnvTest("CombineMixedArgs", env, 255, 0, 255, 50);
}

TestExitStatus TinyGLTests::testTexEnvCombineReplace() {
	TextureEnvironment env(GL_COMBINE, GL_REPLACE, GL_REPLACE);
	env._arg0 = { GL_TEXTURE, GL_SRC_COLOR, GL_PRIMARY_COLOR, GL_SRC_ALPHA };
	return runTexEnvTest("CombineReplace", env, 50, 11, 222, 127);
}

TestExitStatus TinyGLTests::testTexEnvCombineModulate() {
	TextureEnvironment env(GL_COMBINE, GL_MODULATE, GL_MODULATE);
	env._arg0 = { GL_TEXTURE, GL_SRC_COLOR, GL_TEXTURE, GL_SRC_ALPHA };
	env._arg1 = { GL_PRIMARY_COLOR, GL_SRC_COLOR, GL_PRIMARY_COLOR, GL_SRC_ALPHA };
	return runTexEnvTest("CombineModulate", env, 50, 11, 222, 127);
}

TestExitStatus TinyGLTests::testTexEnvCombineAdd() {
	TextureEnvironment env(GL_COMBINE, GL_ADD, GL_ADD);
	env._arg0 = { GL_TEXTURE, GL_SRC_COLOR, GL_TEXTURE, GL_SRC_ALPHA };
	env._arg1 = { GL_PRIMARY_COLOR, GL_SRC_COLOR, GL_PRIMARY_COLOR, GL_SRC_ALPHA };
	return runTexEnvTest("CombineAdd", env, 50, 11, 222, 127);
}

struct TinyGLContextDeleter {
	inline void operator()(TinyGL::ContextHandle *handle) {
		TinyGL::destroyContext(handle);
	}
};

// using cdecl calling convention so we can use a function pointer
static void classicGLTexEnvi(GLenum target, GLenum param, GLint value) {
	glTexEnvi(target, param, value);
}

static void classicGLTexEnvfv(GLenum target, GLenum param, const GLfloat *values) {
	glTexEnvfv(target, param, values);
}

static void copyAlphaIntoColorChannels(Graphics::ManagedSurface &surface) {
	byte a, r, g, b;
	for (int y = 0; y < surface.h; y++) {
		uint32 *pixel = (uint32*)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; x++, pixel++) {
			surface.format.colorToARGB(*pixel, a, r, g, b);
			r = g = b = a;
			a = 255;
			*pixel = surface.format.ARGBToColor(a, r, g, b);
		}
	}
}

static constexpr const int kScreenWidth = 800; // enough space for a 2x2 grid and some padding
static constexpr const int kScreenHeight = 600;
static constexpr const int kTextureSize = 256;

static void renderClassicQuad(GLuint texture, float x, float y, bool flipV = false) {
	const float tLow = flipV ? 1.0f : 0.0f;
	const float tHigh = flipV ? 0.0f : 1.0f;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f, tLow); glVertex2f(x, y);
	glTexCoord2f(1.0f, tLow); glVertex2f(x + kTextureSize, y);
	glTexCoord2f(0.0f, tHigh); glVertex2f(x, y + kTextureSize);
	glTexCoord2f(1.0f, tHigh); glVertex2f(x + kTextureSize, y + kTextureSize);
	glEnd();
}

static void renderClassicGridQuad(GLuint texture, int row, int column) {
	renderClassicQuad(
		texture,
		kScreenWidth / 4 * (row * 2 + 1) - kTextureSize / 2,
		kScreenHeight / 4 * (column * 2 + 1) - kTextureSize / 2,
		true); // by rendering and reading back we have flipped the image
}

TestExitStatus TinyGLTests::runTexEnvTest(
	const char *testName,
	const TextureEnvironment &env,
	byte colorR, byte colorG, byte colorB, byte colorA) {

	int oldW = g_system->getWidth();
	int oldH = g_system->getHeight();
	auto oldFormat = g_system->getScreenFormat();
	int oldGraphicsMode = g_system->getGraphicsMode();

	// load test image (crop and scale to square power-of-two)
	Graphics::ManagedSurface testImage;
	{
		constexpr const char *kImagePath = "image/pm5544-32bpp-grayalpha.png";
		Image::PNGDecoder pngDecoder;
		Common::File file;
		if (!file.open(kImagePath) ||
			!pngDecoder.loadStream(file)) {
			Testsuite::logDetailedPrintf("Error! Could not load test image: %s\n", kImagePath);
			return kTestFailed;
		}
		const auto *pngSurface = pngDecoder.getSurface();
		if (pngSurface->w < 240 || pngSurface->h < 240) {
			Testsuite::logDetailedPrintf("Error! Test image has unexpected size: %dx%d\n", pngSurface->w, pngSurface->h);
			return kTestFailed;
		}
		int16 pngSize = MIN(pngSurface->w, pngSurface->h);
		auto subRect = Common::Rect::center(pngSurface->w / 2, pngSurface->h / 2, pngSize, pngSize);

		Graphics::ManagedSurface converted;
		converted.convertFrom(*pngSurface, Graphics::PixelFormat::createFormatRGBA32());
		testImage.create(kTextureSize, kTextureSize, Graphics::PixelFormat::createFormatRGBA32());
		Graphics::scaleBlitBilinear(
			(byte *)testImage.getBasePtr(0, 0),
			(const byte *)converted.getBasePtr(subRect.left, subRect.top),
			testImage.pitch,
			converted.pitch,
			testImage.w, testImage.h,
			converted.w, converted.h,
			converted.format);
	}
	Graphics::ManagedSurface readBack(testImage.w, testImage.h, testImage.format);

	// Initialize classic opengl
	initGraphics3d(kScreenWidth, kScreenHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, kScreenWidth, 0, kScreenHeight, -1, 1);
	glViewport(0, 0, kScreenWidth, kScreenHeight);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 0);
	GLuint classicTextures[5]; // test texture + 2*classic result textures + 2*TinyGL result textures
	glGenTextures(5, classicTextures);
	GLuint classicTestTexture = classicTextures[0];
	glBindTexture(GL_TEXTURE_2D, classicTestTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, testImage.getPixels());

	// Initialize TinyGL
	Common::ScopedPtr<TinyGL::ContextHandle, TinyGLContextDeleter> tglContext(
		TinyGL::createContext(kTextureSize, kTextureSize, testImage.format, kTextureSize, false, false));
	TinyGL::setContext(tglContext.get());
	Graphics::Surface tinyglSurface;
	TinyGL::getSurfaceRef(tinyglSurface);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, kScreenWidth, 0, kScreenHeight, -1, 1);
	tglViewport(0, 0, kScreenWidth, kScreenHeight);
	tglDisable(TGL_BLEND);
	tglDisable(TGL_DEPTH_TEST);
	tglDisable(TGL_CULL_FACE);
	tglEnable(TGL_TEXTURE_2D);
	tglClearColor(0, 0, 0, 0);
	TGLuint tinyglTestTexture;
	tglGenTextures(1, &tinyglTestTexture);
	tglBindTexture(TGL_TEXTURE_2D, classicTestTexture);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, kTextureSize, kTextureSize, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, testImage.getPixels());

	// Render classic OpenGL
	env.apply(classicGLTexEnvi, classicGLTexEnvfv);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4ub(colorR, colorG, colorB, colorA);
	renderClassicQuad(classicTestTexture, 0, 0);
	glFlush();
	g_system->presentBuffer();
	readBack.clear();
	glReadPixels(0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE, readBack.getPixels());
	GLuint classicReadBackColorTex = classicTextures[1];
	glBindTexture(GL_TEXTURE_2D, classicReadBackColorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, readBack.getPixels());
	copyAlphaIntoColorChannels(readBack);
	GLuint classicReadBackAlphaTex = classicTextures[2];
	glBindTexture(GL_TEXTURE_2D, classicReadBackAlphaTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, readBack.getPixels());

	// Render TinyGL
	env.apply(tglTexEnvi, tglTexEnvfv);
	tglClear(TGL_COLOR_BUFFER_BIT);
	tglColor4ub(colorR, colorG, colorB, colorA);
	tglBegin(TGL_TRIANGLE_STRIP);
	tglTexCoord2f(0.0f, 1.0f); tglVertex2f(0.0f, 0.0f);
	tglTexCoord2f(1.0f, 1.0f); tglVertex2f(kTextureSize, 0.0f);
	tglTexCoord2f(0.0f, 0.0f); tglVertex2f(0.0f, kTextureSize);
	tglTexCoord2f(1.0f, 0.0f); tglVertex2f(kTextureSize, kTextureSize);
	tglEnd();
	TinyGL::presentBuffer();
	readBack.copyFrom(tinyglSurface);
	GLuint tinyglReadBackColorTex = classicTextures[3];
	glBindTexture(GL_TEXTURE_2D, tinyglReadBackColorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, readBack.getPixels());
	copyAlphaIntoColorChannels(readBack);
	GLuint tinyglReadBackAlphaTex = classicTextures[4];
	glBindTexture(GL_TEXTURE_2D, tinyglReadBackAlphaTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, readBack.getPixels());

	// Render comparison
	glClear(GL_COLOR_BUFFER_BIT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1, 1, 1, 1);
	renderClassicGridQuad(classicReadBackColorTex, 0, 0);
	renderClassicGridQuad(classicReadBackAlphaTex, 1, 0);
	renderClassicGridQuad(tinyglReadBackColorTex, 0, 1);
	renderClassicGridQuad(tinyglReadBackAlphaTex, 1, 1);
	glFlush();
	g_system->updateScreen();
	glDeleteTextures(5, classicTextures);

	g_system->delayMillis(1000);
	TestExitStatus status = kTestPassed;
	Common::String info = Common::String::format("Does the top row of images look like the bottom row?\n(Testing %s)", testName);
	if (Testsuite::handleInteractiveInput(info, "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! TinyGL and OpenGL have different texure environment behaviors for %s\n", testName);
		status = kTestFailed;
	}

	// Return to previous state
	g_system->beginGFXTransaction();
	g_system->setGraphicsMode(oldGraphicsMode);
	g_system->initSize(oldW, oldH, &oldFormat);
	g_system->endGFXTransaction();
	return status;
}

}
