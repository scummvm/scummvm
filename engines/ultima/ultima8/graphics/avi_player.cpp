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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/avi_player.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/ultima8.h"
#include "graphics/surface.h"
#include "common/system.h"
#include "common/stream.h"
#include "video/avi_decoder.h"

namespace Ultima {
namespace Ultima8 {

AVIPlayer::AVIPlayer(Common::SeekableReadStream *rs, int width, int height, const byte *overridePal)
	: MoviePlayer(), _playing(false), _width(width), _height(height),
	  _doubleSize(false), _pausedMusic(false), _overridePal(overridePal) {
	_decoder = new Video::AVIDecoder();
	_decoder->loadStream(rs);
	uint32 vidWidth = _decoder->getWidth();
	uint32 vidHeight = _decoder->getHeight();
	if (vidWidth <= _width / 2 && vidHeight <= _height / 2) {
		_doubleSize = true;
		vidHeight *= 2;
		vidWidth *= 2;
	}
	_xoff = _width / 2 - (vidWidth / 2);
	_yoff = _height / 2 - (vidHeight / 2);
	_currentFrame.create(vidWidth, vidHeight, _decoder->getPixelFormat());
	_currentFrame.fillRect(Common::Rect(0, 0, vidWidth, vidHeight), 0);
}

AVIPlayer::~AVIPlayer() {
	delete _decoder;
}

void AVIPlayer::start() {
	MusicProcess *music = MusicProcess::get_instance();
	if (music && music->isPlaying()) {
		music->pauseMusic();
		_pausedMusic = true;
	}

	_playing = true;
	_decoder->start();
}

void AVIPlayer::stop() {
	MusicProcess *music = MusicProcess::get_instance();
	if (music && _pausedMusic) {
		music->unpauseMusic();
		_pausedMusic = false;
	}

	_playing = false;
	_decoder->stop();
}

void AVIPlayer::paint(RenderSurface *surf, int /*lerp*/) {
	if (_decoder->endOfVideo()) {
		_playing = false;
		return;
	}
	if (_decoder->needsUpdate())
	{
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		if (!frame) {
			// Some sort of decoding error?
			_playing = false;
			return;
		}
		if (frame->format.bytesPerPixel == 1) {
			const byte *pal;
			if (_overridePal)
				pal = _overridePal;
			else
				pal = _decoder->getPalette();

			_currentFrame.setPalette(pal, 0, 256);
		}
		if (_doubleSize) {
			assert(_currentFrame.w == frame->w * 2 && _currentFrame.h == frame->h * 2);
			for (int y = 0; y < frame->h; y++) {
				const uint8 *srcPixel = static_cast<const uint8 *>(frame->getPixels()) + frame->pitch * y;
				uint8 *dstPixels = static_cast<uint8 *>(_currentFrame.getPixels()) + _currentFrame.pitch * y * 2;
				for (int x = 0; x < frame->w; x++) {
					dstPixels[x * 2] = *srcPixel;
					dstPixels[x * 2 + 1] = *srcPixel;
					srcPixel++;
				}
			}
		} else {
			_currentFrame.blitFrom(*frame);
		}
	}

	surf->Blit(&_currentFrame, 0, 0, _currentFrame.w, _currentFrame.h,
			_xoff, _yoff);
}

void AVIPlayer::run() {
	if (_decoder->endOfVideo()) {
		_playing = false;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
