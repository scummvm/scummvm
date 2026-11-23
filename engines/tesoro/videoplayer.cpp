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
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

#include "tesoro/tesoro.h"
#include "tesoro/videoplayer.h"

namespace Tesoro {

void VideoPlayer::playVideo(int number) {
	// The movies are plain AVI files (Indeo 5) in the disc's bin folder, next to the game files.
	Common::File *file = new Common::File();
	const Common::String name = Common::String::format("bin/data%02d.bin", number);
	if (!file->open(Common::Path(name, '/'))) {
		warning("Tesoro: the movie %s is not there", name.c_str());
		delete file;
		return;
	}

	Video::AVIDecoder decoder;
	if (!decoder.loadStream(file)) {
		warning("Tesoro: the movie %s cannot be played", name.c_str());
		return;
	}
	decoder.start();

	// It plays until it ends or the skip key (Escape) is let go.
	bool skipped = false;
	while (!skipped && !decoder.endOfVideo() && !_engine.shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			_engine.handleEvent(event);
			if (event.type == Common::EVENT_KEYUP && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				skipped = true;
		}
		if (decoder.needsUpdate()) {
			if (const Graphics::Surface *frame = decoder.decodeNextFrame()) {
				Graphics::Surface *converted = frame->convertTo(_engine._screen->format);
				_engine._screen->clear();
				_engine._screen->copyRectToSurface(*converted, (640 - converted->w) / 2, (480 - converted->h) / 2, Common::Rect(converted->w, converted->h));
				converted->free();
				delete converted;
				_engine._screen->update();
			}
		}
		g_system->delayMillis(10);
	}
	decoder.stop();

	releaseInput();
}

void VideoPlayer::pumpEvents(uint32 milliseconds) {
	const uint32 end = g_system->getMillis() + milliseconds;
	do {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event))
			_engine.handleEvent(event);
		g_system->delayMillis(10);
	} while (g_system->getMillis() < end && !_engine.shouldQuit());
}

void VideoPlayer::releaseInput() {
	// The movie took the events: the skip key is let go, the mouse buttons come up, and then half a second passes.
	_engine._escape = 0;
	while ((_engine._buttonsDown[0] || _engine._buttonsDown[1] || _engine._buttonsDown[2]) && !_engine.shouldQuit())
		pumpEvents(10);
	pumpEvents(500);
}

} // End of namespace Tesoro
