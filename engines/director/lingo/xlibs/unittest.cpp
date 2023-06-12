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

/*************************************
 *
 * ScummVM Unit Testing framework.
 * Used in the Director Test suite - https://github.com/scummvm/director-tests
 *
 *************************************/

#include "director/director.h"
#include "director/archive.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/unittest.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

namespace Director {

const char *UnitTest::xlibName = "UnitTest";
const char *UnitTest::fileNames[] = {
	"UnitTest",
	0
};

static BuiltinProto builtins[] = {
	{ "UTScreenshot", UnitTest::m_UTScreenshot, 0, 1, 400, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void UnitTest::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void UnitTest::close(int type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void UnitTest::m_UTScreenshot(int nargs) {
	Common::String filenameBase = g_director->getCurrentMovie()->getArchive()->getFileName();
	if (filenameBase.hasSuffixIgnoreCase(".dir"))
		filenameBase = filenameBase.substr(0, filenameBase.size() - 4);

	if (nargs > 1) {
		g_lingo->dropStack(nargs - 1);
	}
	if (nargs == 1) {
		Datum name = g_lingo->pop();
		if (name.type == STRING) {
			filenameBase = *name.u.s;
		} else if (name.type != VOID) {
			warning("UnitTest::b_UTScreenshot(): expected string for arg 1, ignoring");
		}
	}

	Common::FSNode gameDataDir = g_director->_gameDataDir;
	Common::FSNode screenDir = gameDataDir.getChild("utscreen");
	if (!screenDir.exists()) {
		screenDir.createDirectory();
	}

	// force a full screen redraw before taking the screenshot
	Score *score = g_director->getCurrentMovie()->getScore();
	score->renderSprites(score->getCurrentFrameNum(), kRenderForceUpdate);
	Window *window = g_director->getCurrentWindow();
	window->render();
	Graphics::ManagedSurface *windowSurface = window->getSurface();

#ifdef USE_PNG
	Common::FSNode file = screenDir.getChild(Common::String::format("%s.png", filenameBase.c_str()));
#else
	Common::FSNode file = screenDir.getChild(Common::String::format("%s.bmp", filenameBase.c_str()));
#endif

	Common::SeekableWriteStream *stream = file.createWriteStream();
	if (!stream) {
		warning("UnitTest::b_UTScreenshot(): could not open file %s", file.getPath().c_str());
		return;
	}

	bool success = false;
#ifdef USE_PNG
	if (windowSurface->format.bytesPerPixel == 1) {
		success = Image::writePNG(*stream, *windowSurface, g_director->getPalette());
	} else {
		success = Image::writePNG(*stream, *windowSurface);
	}
#else
	success = Image::writeBMP(*stream, *windowSurface);
#endif
	if (!success) {
		warning("UnitTest::b_UTScreenshot(): error writing screenshot data to file %s", file.getPath().c_str());
	}
	stream->finalize();
	delete stream;
}

} // End of namespace Director
