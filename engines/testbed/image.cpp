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

#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "graphics/pm5544.h"
#include "graphics/screen.h"
#include "image/bmp.h"
#include "image/gif.h"
#include "image/iff.h"
#include "image/jpeg.h"
#include "image/pcx.h"
#include "image/pict.h"
#include "image/png.h"
#include "image/tga.h"
#include "image/xbm.h"

#include "testbed/image.h"
#include "testbed/testsuite.h"

namespace Testbed {

namespace ImageTests {
TestExitStatus testRenderPM5544();
TestExitStatus testBitmapDecoder();
TestExitStatus testJPEGDecoder();
TestExitStatus testGIFDecoder();
TestExitStatus testPCXDecoder();
TestExitStatus testPICTDecoder();
TestExitStatus testPNGDecoder();
TestExitStatus testTGADecoder();
TestExitStatus testXBMDecoder();
bool testImageDecoder(Common::Path &filepath, Image::ImageDecoder &decoder);
}

ImageTestSuite::ImageTestSuite() {
	// Add tests here

	// Render base image
	addTest("testRenderPM5544", &ImageTests::testRenderPM5544);

	// Test image decoders
	addTest("testBitmapDecoder", &ImageTests::testBitmapDecoder);
	addTest("testJPEGDecoder", &ImageTests::testJPEGDecoder);
	addTest("testGIFDecoder", &ImageTests::testGIFDecoder);
	addTest("testPCXDecoder", &ImageTests::testPCXDecoder);
	addTest("testPICTDecoder", &ImageTests::testPICTDecoder);
	addTest("testPNGDecoder", &ImageTests::testPNGDecoder);
	addTest("testTGADecoder", &ImageTests::testTGADecoder);

	// External XBM files are not yet supported
	//addTest("testXBMDecoder", &ImageTests::testXBMDecoder);
}

TestExitStatus ImageTests::testRenderPM5544() {
	Common::String info = "Render test pattern?\n"
						  "This test pattern is the comparison reference for the other image decoders.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testRenderPM5544()\n");
		return kTestSkipped;
	}

	int xres = 320, yres = 240;
	g_system->beginGFXTransaction();
	g_system->initSize(xres, yres);
	g_system->endGFXTransaction();

	Graphics::ManagedSurface *pm5544 = Graphics::renderPM5544(xres, yres);
	// Clear the version string
	pm5544->fillRect(Common::Rect(112, 176, 208, 192), 0);

	byte palette[768];
	pm5544->grabPalette(palette, 0, 256);
	// Common::Path filename("image/pm5544.bmp");
	// Common::DumpFile dumpFile;
	// bool result = dumpFile.open(filename);
	// if (result) {
	// 	result = Image::writeBMP(dumpFile, pm5544->rawSurface(), palette);
	// }

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	g_system->copyRectToScreen(pm5544->surfacePtr()->getPixels(), pm5544->surfacePtr()->pitch, 0, 0, xres, yres);
	g_system->updateScreen();

	delete pm5544;

	return kTestPassed;
}

TestExitStatus ImageTests::testBitmapDecoder() {
	if (Testsuite::handleInteractiveInput("Test bitmap decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testBitmapDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.bmp";
	decoder.reset(new Image::BitmapDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp.bmp";
	decoder.reset(new Image::BitmapDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Bitmap decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testJPEGDecoder() {
	if (Testsuite::handleInteractiveInput("Test JPEG decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testJPEGDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.jpg";
	decoder.reset(new Image::JPEGDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("JPEG decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testGIFDecoder() {
#ifdef USE_GIF
	if (Testsuite::handleInteractiveInput("Test GIF decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testGIFDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-8bpp.gif";
	decoder.reset(new Image::GIFDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("GIF decoder is OK\n");
	return kTestPassed;
#else
	Testsuite::logDetailedPrintf("Info! Skipping test: GIF decoder is disabled.\n");
	return kTestSkipped;
#endif
}

TestExitStatus ImageTests::testPCXDecoder() {
	if (Testsuite::handleInteractiveInput("Test PCX decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testPCXDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.pcx";
	decoder.reset(new Image::PCXDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp.pcx";
	decoder.reset(new Image::PCXDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("PCX decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testPICTDecoder() {
	if (Testsuite::handleInteractiveInput("Test PICT decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testPICTDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.pict";
	decoder.reset(new Image::PICTDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp.pict";
	decoder.reset(new Image::PICTDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("PICT decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testPNGDecoder() {
	if (Testsuite::handleInteractiveInput("Test PNG decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testPNGDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.png";
	decoder.reset(new Image::PNGDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp.png";
	decoder.reset(new Image::PNGDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp-grey.png";
	decoder.reset(new Image::PNGDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("PNG decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testTGADecoder() {
	if (Testsuite::handleInteractiveInput("Test TGA decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testTGADecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-24bpp.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-24bpp-rle.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp-rle.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp-grey.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	filepath = "image/pm5544-8bpp-grey-rle.tga";
	decoder.reset(new Image::TGADecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("TGA decoder is OK\n");
	return kTestPassed;
}

TestExitStatus ImageTests::testXBMDecoder() {
	if (Testsuite::handleInteractiveInput("Test XBM decoder?", "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test: testXBMDecoder()\n");
		return kTestSkipped;
	}

	Common::Path filepath;
	Common::SharedPtr<Image::ImageDecoder> decoder;

	filepath = "image/pm5544-1bpp.xbm";
	decoder.reset(new Image::XBMDecoder());
	if (!testImageDecoder(filepath, *decoder)) {
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("XBM decoder is OK\n");
	return kTestPassed;
}

bool ImageTests::testImageDecoder(Common::Path &filepath, Image::ImageDecoder &decoder) {
	Common::File f;
	if (!f.open(filepath)) {
		Testsuite::logDetailedPrintf("Error! File could not be opened: %s\n", filepath.toString().c_str());
		return false;
	}

	if (!decoder.loadStream(f)) {
		Testsuite::logDetailedPrintf("Error! Image decoder failed: %s\n", filepath.toString().c_str());
		return false;
	}

	int oldW = g_system->getWidth();
	int oldH = g_system->getHeight();
	Graphics::PixelFormat oldFormat = g_system->getScreenFormat();

	const Graphics::Surface *pSurface = decoder.getSurface();
	Graphics::PixelFormat pf = g_system->getOverlayFormat();
	g_system->beginGFXTransaction();
	g_system->initSize(pSurface->w, pSurface->h, &pf);
	g_system->endGFXTransaction();

	Graphics::Screen screen;
	if (decoder.hasPalette()) {
		screen.simpleBlitFrom(*pSurface, Graphics::FLIP_NONE, false, 255, &decoder.getPalette());
	} else {
		screen.simpleBlitFrom(*pSurface);
	}

	screen.update();
	g_system->delayMillis(1000);

	bool result = true;
	Common::String info = "Did the image \"" + filepath.baseName() + "\" display as expected?";
	if (Testsuite::handleInteractiveInput(info, "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Image did not display as expected: %s\n", filepath.toString().c_str());
		result = false;
	}

	// Return to previous state
	g_system->beginGFXTransaction();
	g_system->initSize(oldW, oldH, &oldFormat);
	g_system->endGFXTransaction();
	return result;
}

} // End of namespace Testbed
