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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_videoplayer.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "engines/util.h"

namespace Illusions {

// DuckmanVideoPlayer

DuckmanVideoPlayer::DuckmanVideoPlayer(IllusionsEngine_Duckman *vm)
	: _vm(vm), _videoDecoder(0) {
}

DuckmanVideoPlayer::~DuckmanVideoPlayer() {
	delete _videoDecoder;
}

void DuckmanVideoPlayer::start(uint32 videoId, uint32 callingThreadId) {
	debug(0, "DuckmanVideoPlayer::play(%08X, %08X)", videoId, callingThreadId);
	_callingThreadId = callingThreadId;
	_vm->_input->discardAllEvents();
	Common::String filename = Common::String::format("%08x.avi", videoId);
	_videoDecoder = new Video::AVIDecoder();
	if (!_videoDecoder->loadFile(filename)) {
		delete _videoDecoder;
		_videoDecoder = 0;
		warning("Unable to open video %s", filename.c_str());
		return;
	}
	_videoDecoder->start();
}

void DuckmanVideoPlayer::stop() {
	_vm->_input->discardAllEvents();
	delete _videoDecoder;
	_videoDecoder = 0;
	if (_callingThreadId != 0) {
		_vm->notifyThreadId(_callingThreadId);
		_callingThreadId = 0;
	}
}

void DuckmanVideoPlayer::update() {
	if (_vm->_input->pollEvent(kEventSkip) || _videoDecoder->endOfVideo()) {
		stop();
	} else if (_videoDecoder->needsUpdate()) {
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Graphics::Surface *backSurface = _vm->_screen->getBackSurface();
		if (frame && frame->format.bytesPerPixel == g_system->getScreenFormat().bytesPerPixel) {
			const int width = MIN(frame->w, backSurface->w);
			const int height = MIN(frame->h, backSurface->h);
			const byte *src = (const byte*)frame->getPixels();
			byte *dest = (byte*)backSurface->getPixels();
			for (int yc = 0; yc < height; ++yc) {
				memcpy(dest, src, width);
				src += frame->pitch;
				dest += backSurface->pitch;
			}
		}
		if (_videoDecoder->hasDirtyPalette()) {
			const byte *palette = _videoDecoder->getPalette();
			byte palette4[1024];
			for (uint i = 0; i < 256; ++i) {
				palette4[i * 4 + 0] = palette[i * 3 + 0];
				palette4[i * 4 + 1] = palette[i * 3 + 1];
				palette4[i * 4 + 2] = palette[i * 3 + 2];
			}
			_vm->_screenPalette->setPalette(palette4, 1, 256);
		}
	}
}

bool DuckmanVideoPlayer::isPlaying() const {
	return _videoDecoder != 0;
}

} // End of namespace Illusions
