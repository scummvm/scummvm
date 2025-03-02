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

#include "common/events.h"
#include "common/file.h"
#include "engines/util.h"
#include "video/qt_decoder.h"
#include "video/qt_data.h"

#include "testbed/testbed.h"
#include "testbed/video.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "gui/browser.h"

#include "video/qt_data.h"

namespace Testbed {

Common::Error Videotests::videoTest(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (!file->open(path)) {
		warning("Cannot open file %s", path.toString(Common::Path::kNativeSeparator).c_str());
		delete file;
		return Common::kNoGameDataFoundError;
	}
	return videoTest(file, path.toString(Common::Path::kNativeSeparator));
}

Common::Error Videotests::videoTest(const Common::FSNode &node) {
	Common::SeekableReadStream *stream = node.createReadStream();
	if (!stream) {
		warning("Cannot open file %s", node.getName().c_str());
		return Common::kNoGameDataFoundError;
	}

	return videoTest(stream, node.getName());
}

Common::Error Videotests::videoTest(Common::SeekableReadStream *stream, const Common::String &name) {
	Video::QuickTimeDecoder *video = new Video::QuickTimeDecoder();
	if (!video->loadStream(stream)) {
		warning("Cannot open video %s", name.c_str());
		delete video;
		return Common::kReadingFailed;
	}

	video->setTargetSize(400, 300);

	warning("Video size: %d x %d", video->getWidth(), video->getHeight());

	Common::List<Graphics::PixelFormat> supportedFormatsList = g_system->getSupportedFormats();
	Graphics::PixelFormat pixelformat = supportedFormatsList.front();
	warning("Best pixel format: %s", pixelformat.toString().c_str());
	warning("Video pixel format: %s", video->getPixelFormat().toString().c_str());

	if (video->getPixelFormat().isCLUT8()) {
		pixelformat = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		if (pixelformat.isCLUT8() && video->setDitheringPalette(Video::quickTimeDefaultPalette256)) {
			pixelformat = Graphics::PixelFormat::createFormatCLUT8();
		} else if (video->setOutputPixelFormats(supportedFormatsList)) {
			pixelformat = video->getPixelFormat();
		} else {
			pixelformat = supportedFormatsList.front();
		}
	}

	warning("Actual pixel format: %s", pixelformat.toString().c_str());

#ifdef __DS__
	int w = 256, h = 192;
#elif defined(__3DS__)
	int w = 320, h = 240;
#elif defined(USE_HIGHRES)
	int w = 640, h = 480;
#else
	int w = 320, h = 200;
#endif
	if (w < video->getWidth() || h < video->getHeight()) {
		w = video->getWidth();
		h = video->getHeight();
	}

	initGraphics(w, h, &pixelformat);

	video->start();

	Common::Point mouse;

	while (!video->endOfVideo()) {
		if (video->needsUpdate()) {
			uint32 pos = video->getTime();
			debug(5, "video time: %d", pos);

			if (pixelformat.isCLUT8() && video->hasDirtyPalette()) {
				g_system->getPaletteManager()->setPalette(video->getPalette(), 0, 256);
			}

			const Graphics::Surface *frame = video->decodeNextFrame();
			int x = 0, y = 0;
			int mw = 0, mh = 0;

			if (frame) {
				const Graphics::Surface *surf = frame;
				Graphics::Surface *conv = nullptr;

				if (frame->format != pixelformat) {
					surf = conv = frame->convertTo(pixelformat, Video::quickTimeDefaultPalette256);
				}

				mw = surf->w;
				mh = surf->h;

				if (surf->w < w)
					x = (w - surf->w) >> 1;
				if (surf->h < h)
					y = (h - surf->h) >> 1;

				g_system->copyRectToScreen(surf->getPixels(), surf->pitch, x, y, MIN<uint16>(surf->w, w), MIN<uint16>(surf->h, h));

				if (conv) {
					conv->free();
					delete conv;
				}
			}

			Common::Event event;

			while (g_system->getEventManager()->pollEvent(event)) {
				if (Common::isMouseEvent(event))
					mouse = event.mouse;

				if (mouse.x >= x && mouse.x < x + mw &&
						mouse.y >= y && mouse.y < y + mh) {
					switch (event.type) {
					case Common::EVENT_LBUTTONDOWN:
						((Video::QuickTimeDecoder *)video)->handleMouseButton(true, event.mouse.x - x, event.mouse.y - y);
						break;
					case Common::EVENT_LBUTTONUP:
						((Video::QuickTimeDecoder *)video)->handleMouseButton(false, event.mouse.x - x, event.mouse.y - y);
						break;
					case Common::EVENT_MOUSEMOVE:
						((Video::QuickTimeDecoder *)video)->handleMouseMove(event.mouse.x - x, event.mouse.y - y);
						break;
					case Common::EVENT_KEYUP:
					case Common::EVENT_KEYDOWN:
						((Video::QuickTimeDecoder *)video)->handleKey(event.kbd, event.type == Common::EVENT_KEYDOWN);
						break;
					default:
						break;
					}
				} else {
					CursorMan.showMouse(false);
				}

				if (Engine::shouldQuit()) {
					video->close();
					delete video;
					return Common::kNoError;
				}
			}
			g_system->updateScreen();
			video->delayMillis(10);
		}
	}
	video->close();
	delete video;

	return Common::kNoError;
}

TestExitStatus Videotests::testPlayback() {
	Testsuite::clearScreen();
	Common::String info = "Video playback test. A QuickTime video should be selected using the file browser, and it'll be played on the screen.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing video playback", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : testPlayback\n");
		return kTestSkipped;
	}

	GUI::BrowserDialog browser(Common::U32String("Select video file"), false);

	if (browser.runModal() <= 0) {
		Testsuite::logPrintf("Info! Skipping test : testPlayback\n");
		return kTestSkipped;
	}

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	Common::Error error = videoTest(browser.getResult());

	initGraphics(320, 200);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	if (error.getCode() != Common::kNoError) {
		Testsuite::logDetailedPrintf("Video playback failed: %s\n", error.getDesc().c_str());
		return kTestFailed;
	}

	Common::String prompt = "Did the video display correctly?";
	if (!Testsuite::handleInteractiveInput(prompt, "Yes", "No", kOptionLeft)) {
		Testsuite::logDetailedPrintf("Video playback failed\n");
		return kTestFailed;
	}

	return kTestPassed;
}

VideoDecoderTestSuite::VideoDecoderTestSuite() {
	_isTsEnabled = false;
	addTest("testPlayback", &Videotests::testPlayback, true);
}

} // End of namespace Testbed
