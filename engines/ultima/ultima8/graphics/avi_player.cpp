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
	  _doubleSize(false), _overridePal(overridePal) {
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
}

AVIPlayer::~AVIPlayer() {
	delete _decoder;
}

void AVIPlayer::start() {
	_playing = true;
	_decoder->start();
}

void AVIPlayer::stop() {
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

			_currentFrame.loadSurface8Bit(frame, pal);
		} else {
			_currentFrame.loadSurface(frame);
		}
	}

	// TODO: Crusader has a CRT-like scaling which it uses in some
	// movies too (eg, T02 for the intro).  For now just point-scale.
	if (_doubleSize) {
		const Scaler *pointScaler = &Ultima8Engine::get_instance()->point_scaler;
		bool ok = surf->ScalerBlit(&_currentFrame, 0, 0, _currentFrame.w, _currentFrame.h,
								   _xoff, _yoff, _currentFrame.w * 2, _currentFrame.h * 2,
								   pointScaler, false);
		assert(ok);
	} else {
		surf->Blit(&_currentFrame, 0, 0, _currentFrame.w, _currentFrame.h,
				   _xoff, _yoff);
	}
}

void AVIPlayer::run() {
	if (_decoder->endOfVideo()) {
		_playing = false;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
