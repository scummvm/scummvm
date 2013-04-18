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

#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "toon/audio.h"
#include "toon/movie.h"
#include "toon/toon.h"

namespace Toon {

void ToonstruckSmackerDecoder::handleAudioTrack(byte track, uint32 chunkSize, uint32 unpackedSize) {
	debugC(6, kDebugMovie, "handleAudioTrack(%d, %d, %d)", track, chunkSize, unpackedSize);

	if (track == 1 && chunkSize == 4) {
		/* uint16 width = */ _fileStream->readUint16LE();
		uint16 height = _fileStream->readUint16LE();
		_lowRes = (height == getHeight() / 2);
	} else
		Video::SmackerDecoder::handleAudioTrack(track, chunkSize, unpackedSize);
}

bool ToonstruckSmackerDecoder::loadFile(const Common::String &filename) {
	debugC(1, kDebugMovie, "loadFile(%s)", filename.c_str());

	_lowRes = false;

	if (Video::SmackerDecoder::loadFile(filename)) {
		if (_surface->h == 200) {
			if (_surface) {
				_surface->free();
				delete _surface;
			}
			_surface = new Graphics::Surface();
			_surface->create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
			_header.flags = 4;
		}

		return true;
	}
	return false;
}

ToonstruckSmackerDecoder::ToonstruckSmackerDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : Video::SmackerDecoder(mixer, soundType) {
	_lowRes = false;
}

// decoder is deallocated with Movie destruction i.e. new ToonstruckSmackerDecoder is needed
Movie::Movie(ToonEngine *vm , ToonstruckSmackerDecoder *decoder) {
	_vm = vm;
	_playing = false;
	_decoder = decoder;
}

Movie::~Movie() {
	delete _decoder;
}

void Movie::init() const {
}

void Movie::play(Common::String video, int32 flags) {
	debugC(1, kDebugMovie, "play(%s, %d)", video.c_str(), flags);
	bool isFirstIntroVideo = false;
	if (video == "209_1M.SMK")
		isFirstIntroVideo = true;

	_playing = true;
	if (flags & 1)
		_vm->getAudioManager()->setMusicVolume(0);
	_decoder->loadFile(video.c_str());
	playVideo(isFirstIntroVideo);
	_vm->flushPalette(true);
	if (flags & 1)
		_vm->getAudioManager()->setMusicVolume(_vm->getAudioManager()->isMusicMuted() ? 0 : 255);
	_decoder->close();
	_playing = false;
}

bool Movie::playVideo(bool isFirstIntroVideo) {
	debugC(1, kDebugMovie, "playVideo(isFirstIntroVideo: %d)", isFirstIntroVideo);
	while (!_vm->shouldQuit() && !_decoder->endOfVideo()) {
		if (_decoder->needsUpdate()) {
			const Graphics::Surface *frame = _decoder->decodeNextFrame();
			if (frame) {
				if (_decoder->isLowRes()) {
					// handle manually 2x scaling here
					Graphics::Surface* surf = _vm->getSystem()->lockScreen();
					for (int y = 0; y < frame->h / 2; y++) {
						memcpy(surf->getBasePtr(0, y * 2 + 0), frame->getBasePtr(0, y), frame->pitch);
						memcpy(surf->getBasePtr(0, y * 2 + 1), frame->getBasePtr(0, y), frame->pitch);
					}
					_vm->getSystem()->unlockScreen();
				} else {
					_vm->getSystem()->copyRectToScreen((byte *)frame->pixels, frame->pitch, 0, 0, frame->w, frame->h);

					// WORKAROUND: There is an encoding glitch in the first intro video. This hides this using the adjacent pixels.
					if (isFirstIntroVideo) {
						int32 currentFrame = _decoder->getCurFrame();
						if (currentFrame >= 956 && currentFrame <= 1038) {
							debugC(1, kDebugMovie, "Triggered workaround for glitch in first intro video...");
							_vm->getSystem()->copyRectToScreen((const byte *)frame->getBasePtr(frame->w-188, 123), frame->pitch, frame->w-188, 124, 188, 1);
							_vm->getSystem()->copyRectToScreen((const byte *)frame->getBasePtr(frame->w-188, 126), frame->pitch, frame->w-188, 125, 188, 1);
							_vm->getSystem()->copyRectToScreen((const byte *)frame->getBasePtr(0, 125), frame->pitch, 0, 126, 64, 1);
							_vm->getSystem()->copyRectToScreen((const byte *)frame->getBasePtr(0, 128), frame->pitch, 0, 127, 64, 1);
						}
					}
				}
			}
			_decoder->setSystemPalette();
			_vm->getSystem()->updateScreen();
		}

		Common::Event event;
		while (_vm->getSystem()->getEventManager()->pollEvent(event))
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
				_vm->dirtyAllScreen();
				return false;
			}

		_vm->getSystem()->delayMillis(10);
	}
	_vm->dirtyAllScreen();
	return !_vm->shouldQuit();
}

} // End of namespace Toon
