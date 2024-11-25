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
 * USED IN:
 * Iron Helix
 *
 *************************************/

#include "video/paco_decoder.h"

#include "director/director.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/xplayanim.h"

namespace Director {

const char *const XPlayAnim::xlibName = "XPlayAnim";
const XlibFileDesc XPlayAnim::fileNames[] = {
	{ "XPlayAnim",	nullptr },
	{ nullptr,		nullptr },
};

static const BuiltinProto builtins[] = {
	{ "XPlayAnim",	XPlayAnim::b_xplayanim, 3, 3, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void XPlayAnim::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void XPlayAnim::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void XPlayAnim::b_xplayanim(int nargs) {
	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	Common::String filename = g_lingo->pop().asString();

	debug(5, "LB::b_xPlayAnim: x: %i y: %i", x, y);
	Video::PacoDecoder *video = new Video::PacoDecoder();
	bool result = video->loadFile(Common::Path(filename, g_director->_dirSeparator));
	if (!result) {
		warning("b_xPlayAnim: PACo video not loaded: %s", filename.c_str());
		delete video;
		return;
	}

	// save the current palette
	byte origPalette[256 * 3];
	uint16 origCount = g_director->getPaletteColorCount();

	if (origCount > 256) {
		warning("b_xPlayAnim: too big palette, %d > 256", origCount);
		origCount = 256;
	}

	memcpy(origPalette, g_director->getPalette(), origCount * 3);
	Graphics::Surface const *frame = nullptr;
	Common::Event event;
	bool keepPlaying = true;
	video->start();
	while (!video->endOfVideo()) {
		if (g_director->pollEvent(event)) {
			switch(event.type) {
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
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
		}
		if (video->hasDirtyPalette()) {
			byte *palette = const_cast<byte *>(video->getPalette());
			g_director->setPalette(palette, 256);
		}

		g_system->updateScreen();
		g_director->delayMillis(10);

	}
	if (frame != nullptr)
		// Display the last frame after the video is done
		g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
			frame->getPixels(), frame->pitch, x, y, frame->w, frame->h
		);

	video->close();
	delete video;
	// restore the palette
	g_director->setPalette(origPalette, origCount);
}

} // End of namespace Director
