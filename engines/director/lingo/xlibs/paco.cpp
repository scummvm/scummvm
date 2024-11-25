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

#include "common/system.h"
#include "common/tokenizer.h"
#include "graphics/paletteman.h"
#include "video/paco_decoder.h"

#include "director/director.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/paco.h"

/**************************************************
 *
 * USED IN:
 * hellcab-win
 *
 **************************************************/

/*
-- PACow External Factory. 15Jul93 JMU
--PACo
SS     mNew, command      --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
SSS    mPACo, commands, results    --Plays Paco movies
 */

namespace Director {

const char *const PACoXObj::xlibName = "PACo";
const XlibFileDesc PACoXObj::fileNames[] = {
	{ "PACO",	nullptr },
	{ "PACOW",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PACoXObj::m_new,		 1, 1,	300 },
	{ "dispose",				PACoXObj::m_dispose,		 0, 0,	300 },
	{ "pACo",				PACoXObj::m_pACo,		 2, 2,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PACoXObject::PACoXObject(ObjectType ObjectType) :Object<PACoXObject>("PACo") {
	_objType = ObjectType;
}

void PACoXObj::open(ObjectType type, const Common::Path &path) {
    PACoXObject::initMethods(xlibMethods);
    PACoXObject *xobj = new PACoXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PACoXObj::close(ObjectType type) {
    PACoXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void callPacoPlay(const Common::String &cmd) {
	Common::StringTokenizer st(cmd);

	Common::String verb = st.nextToken();
	if (verb == "playfile") {

		Common::String videoPath = st.nextToken();

		int posX = 0;
		int posY = 0;

		while (!st.empty()) {
			Common::String token = st.nextToken();
			if (token == "-posx") {
				posX = atoi(st.nextToken().c_str());
			} else if (token == "-posy") {
				posY = atoi(st.nextToken().c_str());
			} else {
				warning("callPacoPlay: Unknown parameter %s %s", token.c_str(), st.nextToken().c_str());
			}
		}

		Video::PacoDecoder *video = new Video::PacoDecoder();
		bool result = video->loadFile(findPath(videoPath));
		if (!result) {
			warning("callPacoPlay: PACo video not loaded: %s", videoPath.c_str());
			delete video;
			return;
		}

		// save the current palette
		byte origPalette[256 * 3];
		uint16 origCount = g_director->getPaletteColorCount();

		if (origCount > 256) {
			warning("callPacoPlay: too big palette, %d > 256", origCount);
			origCount = 256;
		}

		memcpy(origPalette, g_director->getPalette(), origCount * 3);
		byte videoPalette[256 * 3];

		Graphics::Surface const *frame = nullptr;
		Common::Event event;
		bool keepPlaying = true;
		video->start();
		memcpy(videoPalette, video->getPalette(), 256 * 3);
		while (!video->endOfVideo()) {
			if (g_director->pollEvent(event)) {
				switch (event.type) {
					case Common::EVENT_QUIT:
						g_director->processEventQUIT();
						// fallthrough
					case Common::EVENT_KEYDOWN:
					case Common::EVENT_RBUTTONDOWN:
					case Common::EVENT_LBUTTONDOWN:
						keepPlaying = false;
						break;
					default:
						break;
				}
			}
			if (!keepPlaying)
				break;
			if (video->needsUpdate()) {
				frame = video->decodeNextFrame();
				// Palette info gets set after the frame is decoded
				if (video->hasDirtyPalette()) {
					byte *palette = const_cast<byte *>(video->getPalette());
					memcpy(videoPalette, palette, 256 * 3);
				}

				// Video palette order is going to be different to the screen, we need to untangle it
				Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, videoPalette, 256, origPalette, origCount, Graphics::kDitherNaive);
				int width = MIN(dither->w + posX, (int)g_system->getWidth()) - posX;
				int height = MIN(dither->h + posY, (int)g_system->getHeight()) - posY;
				g_system->copyRectToScreen(dither->getPixels(), dither->pitch, posX, posY, width, height);
				dither->free();
				delete dither;
			}
			g_system->updateScreen();
			g_director->delayMillis(10);
		}

		video->close();
		delete video;
	} else {
		warning("callPacoPlay: Unknown verb %s", verb.c_str());
	}


}

void PACoXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PACoXObj::m_new", nargs);
	if (nargs == 1) {
		Common::String cmd = g_lingo->pop().asString();
		callPacoPlay(cmd);
	} else {
		warning("PACoXObj::m_new: Invalid number of args %d", nargs);
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(PACoXObj::m_dispose)
XOBJSTUB(PACoXObj::m_pACo, "")

}
