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
#include "common/config-manager.h"
#include "common/file.h"

#include "graphics/managed_surface.h"

#include "video/mpegps_decoder.h"

#include "qdengine/qdengine.h"
#include "qdengine/qdcore/util/WinVideo.h"

#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

bool winVideo::_is_initialized = false;

winVideo::winVideo() {
	_decoder = new Video::MPEGPSDecoder();
	_x = _y = 0;
	_vidWidth = _vidHeight = 0;
	_videostream = nullptr;
}

winVideo::~winVideo() {
	close_file();
	delete _decoder;
}

bool winVideo::init() {
	return true;
}

bool winVideo::done() {
	if (!_is_initialized)
		return false;

	_is_initialized = false;

	return true;
}

void winVideo::set_window(int x, int y, int xsize, int ysize) {
	_x = x;
	_y = y;

	_vidWidth = xsize;
	_vidHeight = ysize;

	delete _tempSurf;
	_tempSurf = nullptr;

	if (_vidWidth != _decoder->getWidth() || _vidHeight != _decoder->getHeight())
		_tempSurf = new Graphics::ManagedSurface(xsize, ysize, g_engine->_pixelformat);
}

bool winVideo::open_file(const Common::Path fname) {
	Common::String filename = (char *)transCyrillic(fname.toString());
	debugC(3, kDebugLoad, "winVideo::open_file(%s)", filename.c_str());

	_videostream = new Common::File();

	if (!_videostream->open(filename.c_str())) {
		warning("WinVideo::open: Failed to open file %s", filename.c_str());
		delete _videostream;
		_videostream = nullptr;
		return false;
	}

	if (!_decoder->loadStream(_videostream)) {
		warning("WinVideo::open: Failed to Load Stream");
		_videostream = nullptr;
		return false;
	}

	return true;
}

bool winVideo::play() {
	if (!_videostream) {
		warning("WinVideo::play: No video stream loaded");
		return false;
	}

	_decoder->setVolume(ConfMan.getInt("music_volume"));
	_decoder->start();
	return true;
}

bool winVideo::quant() {
	debugC(9, kDebugGraphics, "WinVideo::play: Video Playback loop");

	// Video Playback loop
	if (_decoder->needsUpdate()) {
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		int frameWidth = _decoder->getWidth();
		int frameHeight = _decoder->getHeight();

		const Graphics::Surface *surf = frame;

		if (frame) {
			if (_tempSurf) {
				const Common::Rect srcRect(0, 0, frameWidth, frameHeight);
				const Common::Rect destRect(0, 0, _vidWidth, _vidHeight);

				_tempSurf->blitFrom(*frame, srcRect, destRect);
				surf = _tempSurf->surfacePtr();
			}

			g_system->copyRectToScreen(surf->getPixels(), surf->pitch, _x, _y, _vidWidth, _vidHeight);
		}

		g_system->delayMillis(10);
		g_system->updateScreen();
	}

	return true;
}

bool winVideo::stop() {
	delete _tempSurf;
	_tempSurf = NULL;

	return true;
}

bool winVideo::is_playback_finished() {
	return _decoder->endOfVideo();
}

bool winVideo::get_movie_size(int &sx, int &sy) {
	if (!_decoder)
		return false;

	sx = _decoder->getWidth();
	sy = _decoder->getHeight();

	return true;
}

void winVideo::close_file() {
	if (_videostream)
		_videostream->close();
}

} // namespace QDEngine
