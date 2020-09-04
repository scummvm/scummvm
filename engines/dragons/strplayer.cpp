/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "video/psx_decoder.h"
#include "dragons/dragons.h"
#include "dragons/screen.h"
#include "dragons/strplayer.h"

namespace Dragons {

StrPlayer::StrPlayer(DragonsEngine *vm, Screen *screen) : _vm(vm), _screen(screen) {
	_decoder = new Video::PSXStreamDecoder(Video::PSXStreamDecoder::kCD2x);
}

void StrPlayer::playVideo(const Common::String &filename) {
	bool skipped = false;

	if (!_decoder->loadFile(filename)) {
		error("Error playing video from %s", filename.c_str());
	}
	_decoder->start();

	while (!_vm->shouldQuit() && !_decoder->endOfVideo() && !skipped) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *frame = _decoder->decodeNextFrame();
			if (frame) {
				_screen->clearScreen();
				_screen->copyRectToSurface(*frame, 0, 0, Common::Rect(frame->w, frame->h));
				_screen->updateScreen();
			}
		}

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END
				&& (event.customType == Dragons::kDragonsActionSelect || event.customType == Dragons::kDragonsActionEnter)) {
				skipped = true;
			}
		}

		_vm->_system->delayMillis(10);
	}
	_screen->clearScreen();
	_decoder->close();
}

StrPlayer::~StrPlayer() {
	delete _decoder;
}

} // End of namespace Dragons
