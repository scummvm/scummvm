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

#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/gfx/avi_player.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
#include "video/avi_decoder.h"

namespace Ultima {
namespace Ultima8 {

AVIPlayer::AVIPlayer(Common::SeekableReadStream *rs, int width, int height, const byte *overridePal, bool noScale)
	: MoviePlayer(), _playing(false), _width(width), _height(height),
	  _doubleSize(false), _pausedMusic(false), _overridePal(overridePal) {
	_decoder = new Video::AVIDecoder();
	_decoder->loadStream(rs);
	uint32 vidWidth = _decoder->getWidth();
	uint32 vidHeight = _decoder->getHeight();
	if (vidWidth <= _width / 2 && vidHeight <= _height / 2 && !noScale) {
		_doubleSize = true;
		vidHeight *= 2;
		vidWidth *= 2;
	}
	_xoff = _width / 2 - (vidWidth / 2);
	_yoff = _height / 2 - (vidHeight / 2);
	_currentFrame.create(vidWidth, vidHeight, _decoder->getPixelFormat());
	_currentFrame.fillRect(Common::Rect(0, 0, vidWidth, vidHeight),
						   _decoder->getPixelFormat().RGBToColor(0, 0, 0));
	if (_currentFrame.format.bytesPerPixel == 1)
		_currentFrame.setTransparentColor(0);
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
		if (!frame || _decoder->getCurFrame() < 0) {
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
			// TODO: Add support for multiple bytes per pixel
			assert(_currentFrame.w == frame->w * 2 && _currentFrame.h == frame->h * 2);
			const int bpp = frame->format.bytesPerPixel;
			for (int y = 0; y < frame->h; y++) {
				const uint8 *srcPixel = static_cast<const uint8 *>(frame->getPixels()) + frame->pitch * y;
				uint8 *dstPixels = static_cast<uint8 *>(_currentFrame.getPixels()) + _currentFrame.pitch * y * 2;
				for (int x = 0; x < frame->w; x++) {
					for (int i = 0; i < bpp; i++) {
						dstPixels[x * 2 * bpp + i] = *srcPixel;
						dstPixels[x * 2 * bpp + i + bpp] = *srcPixel;
						srcPixel++;
					}
				}
			}
		} else {
			_currentFrame.blitFrom(*frame);
		}
	}

	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	surf->fill32(color, _xoff, _yoff, _currentFrame.w, _currentFrame.h);
	Common::Rect srcRect(_currentFrame.w, _currentFrame.h);
	surf->Blit(_currentFrame, srcRect, _xoff, _yoff);
}

void AVIPlayer::run() {
	if (_decoder->endOfVideo()) {
		_playing = false;
	}
}

int AVIPlayer::getFrameNo() const {
	return _decoder->getCurFrame();
}

void AVIPlayer::setOffset(int xoff, int yoff) {
	_xoff = xoff;
	_yoff = yoff;
}

} // End of namespace Ultima8
} // End of namespace Ultima
