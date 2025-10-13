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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/x/xplaypacoxfcn.h"

/**************************************************
 *
 * USED IN:
 * jman
 *
 **************************************************/

#include "video/paco_decoder.h"

namespace Director {

struct PACoPlayer {
	Video::PacoDecoder *_video;
};

class XPlayPACoState : public Object<XPlayPACoState> {
public:
	XPlayPACoState();
	~XPlayPACoState();

	int openfile(Common::String &rawPath);
	int close(int playerId);
	Common::String play(int playerId, int posX, int posY);

	Common::HashMap<int, PACoPlayer> _players;

	int _nextId;
};

XPlayPACoState::XPlayPACoState() : Object<XPlayPACoState>("XPlayPACo") {
	_nextId = 1;
}

XPlayPACoState::~XPlayPACoState() {
	for (auto it : _players) {
		delete it._value._video;
		it._value._video = nullptr;
	}
}

int XPlayPACoState::openfile(Common::String &rawPath) {
	Common::Path path = findPath(rawPath);
	if (path.empty()) {
		warning("XPlayPACo::openfile: Could not resolve path %s", rawPath.c_str());
		return -1;
	}
	Video::PacoDecoder *dec = new Video::PacoDecoder();
	if (!dec->loadFile(path)) {
		delete dec;
		return -1;
	}
	int result = _nextId;
	_nextId++;
	_players[result] = PACoPlayer();
	_players[result]._video = dec;
	return result;
}

int XPlayPACoState::close(int playerId) {
	if (_players.contains(playerId)) {
		_players[playerId]._video->close();
		delete _players[playerId]._video;
		_players.erase(playerId);
	}
	return 0;
}

Common::String XPlayPACoState::play(int playerId, int posX, int posY) {
	if (_players.contains(playerId)) {
		Video::PacoDecoder *video = _players[playerId]._video;
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
	}
	return "0 0";
}

const char *XPlayPACoXFCN::xlibName = "XPlayPACo";
const XlibFileDesc XPlayPACoXFCN::fileNames[] = {
	{ "Presto.rsrc", "jman" },
	{ "XPlayPACo", nullptr },
	{ nullptr, nullptr }
};

static BuiltinProto builtins[] = {
	{ "XPlayPACo", XPlayPACoXFCN::m_XPlayPACo, -1, 0, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void XPlayPACoXFCN::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);

	if (!g_lingo->_openXLibsState.contains("XPlayPACo")) {
		XPlayPACoState *pacoState = new XPlayPACoState();
		g_lingo->_openXLibsState.setVal("XPlayPACo", pacoState);
	}
}

void XPlayPACoXFCN::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
	if (g_lingo->_openXLibsState.contains("XPlayPACo")) {
		AbstractObject *pacoState = g_lingo->_openXLibsState.getVal("XPlayPACo");
		delete pacoState;
		g_lingo->_openXLibsState.erase("XPlayPACo");
	}
}

void XPlayPACoXFCN::m_XPlayPACo(int nargs) {
	g_lingo->printSTUBWithArglist("m_XPlayPACo", nargs);
	XPlayPACoState *state = (XPlayPACoState *)g_lingo->_openXLibsState.getVal("XPlayPACo");
	Datum result;
	if (nargs == 0) {
		warning("XPlayPACoXFCN: no arguments");
	} else {
		Common::String cmd = g_lingo->peek(nargs - 1).asString();
		if (cmd.equalsIgnoreCase("openfile")) {
			if (nargs == 2) {
				Common::String filename = g_lingo->peek(0).asString();
				result = state->openfile(filename);
			} else {
				warning("XPlayPACoXFCN: expected 1 arg for openfile");
			}
		} else if (cmd.equals("set")) {

		} else if (cmd.equals("play")) {
			if (nargs >= 2) {
				int playerId = g_lingo->peek(nargs - 2).asInt();
				int posX = 0;
				int posY = 0;
				for (int i = nargs - 3; i >= 0; i -= 2) {
					Datum paramName = g_lingo->peek(i);
					Datum paramValue = g_lingo->peek(i - 1);
					if (paramName.asString().equalsIgnoreCase("posX")) {
						posX = paramValue.asInt();
					} else if (paramName.asString().equalsIgnoreCase("posY")) {
						posY = paramValue.asInt();
					}
				}
				result = state->play(playerId, posX, posY);
			} else {
				warning("XPlayPACoXFCN: expected at least 1 arg for play");
			}
		} else if (cmd.equals("close")) {
			if (nargs == 2) {
				int playerId = g_lingo->peek(nargs - 2).asInt();
				result = state->close(playerId);
			} else {
				warning("XPlayPACoXFCN: expected 1 arg for close");
			}
		} else {
			warning("XPlayPACoXFCN: Unknown command %s", cmd.c_str());
		}
	}
	g_lingo->dropStack(nargs);
	g_lingo->push(result);
}

}
