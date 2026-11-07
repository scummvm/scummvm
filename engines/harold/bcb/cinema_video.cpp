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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/cinema.h"
#include "video/avi_decoder.h"

namespace BCB {

void CinemaViewer::revealBackground() {
	const uint32 start = g_system->getMillis();
	for (int step = 0; step < 160 && _running && !_engine.shouldQuit(); ++step) {
		const uint32 elapsed = g_system->getMillis() - start;
		const uint32 deadline = (step + 1) * 1000 / 60;
		if (elapsed < deadline)
			wait(deadline - elapsed);
		const Common::Rect right(320 + step * 2, 0, 322 + step * 2, 480);
		const Common::Rect left(318 - step * 2, 0, 320 - step * 2, 480);
		_canvas.blitFrom(_background, right, Common::Point(right.left, 0));
		_canvas.blitFrom(_background, left, Common::Point(left.left, 0));
		present();
	}
}

void CinemaViewer::showPlayback() {
	if (!loadPlaylist())
		return;
	_heldScroll = 0;
	_pressedButton = -1;
	_releaseAction = 0;
	_page = kPlayback;
	_entry = 0;
	_captionSize = 12;
	_playlistEndTime = 0;
	_canvas.clear(0);
	present();
	if (_backgroundIndex)
		revealBackground();
	if (!_running || _engine.shouldQuit())
		return;
	drawControls();
	if (!startMovie())
		stopPlayback();
}

bool CinemaViewer::startMovie() {
	_video.reset();
	_paused = false;
	if (_entry >= static_cast<int>(_movies.size())) {
		_playlistEndTime = g_system->getMillis() + 1000;
		drawControls();
		present();
		return true;
	}
	const CinemaMovie &movie = _movies[_entry];
	const Common::String path = Common::String::format("movie/mov%04u.avi", movie.number);
	Common::SeekableReadStream *stream = _engine.createDiscReadStream(movie.disc, path.c_str());
	if (!stream) {
		warning("BCB Cinema: Missing disc %u file %s", movie.disc, path.c_str());
		return false;
	}
	_video.reset(new Video::AVIDecoder());
	_video->setSoundType(Audio::Mixer::kSFXSoundType);
	if (!_video->loadStream(stream)) {
		warning("BCB Cinema: Unable to decode disc %u file %s", movie.disc, path.c_str());
		_video.reset();
		return false;
	}
	_video->setOutputPixelFormat(_canvas.format);
	loadCaptions(movie.number);
	drawControls();
	_video->start();
	syncMoviePause();
	present();
	return true;
}

void CinemaViewer::stopPlayback() {
	_video.reset();
	_playlistEndTime = 0;
	_dragging = false;
	_paused = false;
	_pressedButton = -1;
	_releaseAction = 0;
	_page = kPlaylist;
	_entry = 0;
	drawSelection();
}

void CinemaViewer::seekMovie(int entry) {
	if (_page != kPlayback)
		return;
	clearCaptions();
	_dragging = false;
	_playlistEndTime = 0;
	_entry = CLIP<int>(entry, 0, _movies.size());
	if (!startMovie())
		stopPlayback();
}

void CinemaViewer::togglePause() {
	if (!_video)
		return;
	_paused = !_paused;
	syncMoviePause();
	drawButton(_paused ? 23 : 22);
	present();
}

void CinemaViewer::syncMoviePause() {
	const bool paused = _paused || _enginePaused || _dragging || _releaseAction == 2;
	if (_video && _video->isPaused() != paused)
		_video->pauseVideo(paused);
}

Common::Rect CinemaViewer::thumbRect() const {
	const int entry = _dragging ? _seekEntry : _entry;
	const int x = 189 + 16 + (_movies.size() == 0 ? 0 : 214 * entry / _movies.size());
	return Common::Rect(x, 440, x + 16, 456);
}

void CinemaViewer::drawScrollbar() {
	const uint32 black = _canvas.format.RGBToColor(0, 0, 0);
	const uint32 white = _canvas.format.RGBToColor(255, 255, 255);
	const Common::Rect track(189, 440, 451, 456);
	_canvas.fillRect(track, _canvas.format.RGBToColor(190, 190, 190));
	_canvas.frameRect(track, black);
	const Common::Rect left(189, 440, 205, 456), right(435, 440, 451, 456);
	_canvas.fillRect(left, white);
	_canvas.fillRect(right, white);
	_canvas.frameRect(left, black);
	_canvas.frameRect(right, black);
	for (int i = 0; i < 4; ++i) {
		_canvas.vLine(194 + i, 448 - i, 448 + i, black);
		_canvas.vLine(445 - i, 448 - i, 448 + i, black);
	}
	const Common::Rect thumb = thumbRect();
	_canvas.fillRect(thumb, white);
	_canvas.frameRect(thumb, black);
	for (int i = 4; i <= 10; i += 3)
		_canvas.vLine(thumb.left + i, thumb.top + 4, thumb.bottom - 5, black);
}

void CinemaViewer::drawControls() {
	_canvas.blitFrom(_sheets[2], Common::Rect(0, 120, 262, 160), dialogOrigin());
	drawButton(20);
	drawButton(_paused ? 23 : 22);
	drawButton(24);
	drawScrollbar();
}

void CinemaViewer::clearCaptions() {
	const Common::Point movie = movieOrigin();
	const int width = 18 * _captionSize;
	const int x = movie.x + (400 - width) / 2;
	const Common::Rect area(x, movie.y + 208, x + width, movie.y + 208 + 2 * (_captionSize + 2) + 4);
	if (_backgroundIndex)
		_canvas.blitFrom(_background, area, Common::Point(area.left, area.top));
	else
		_canvas.fillRect(area, _canvas.format.RGBToColor(0, 0, 0));
}

void CinemaViewer::drawCaptions() {
	if (!_video || _nextCaption >= _captions.size())
		return;
	const int frame = static_cast<int>(_video->getTime() * 15 / 1000) - 1;
	if (_captions[_nextCaption].frame > frame)
		return;
	clearCaptions();
	const int lineStep = _captionSize + 2;
	_captionSize = 14;
	const Caption &caption = _captions[_nextCaption++];
	const Graphics::Font *font = _captionFont ? _captionFont.get() : _engine._captionTextFont;
	if (!font)
		return;
	const Common::Point movie = movieOrigin();
	const int left = movie.x + (400 - 252) / 2 - 1;
	const int top = movie.y + 210 + 14 - 1 - font->getFontAscent();
	const uint32 color = _canvas.format.RGBToColor(255, 255, 255);
	font->drawString(&_canvas, caption.lines[0], left, top, 252, color, Graphics::kTextAlignCenter);
	font->drawString(&_canvas, caption.lines[1], left, top + lineStep, 252, color, Graphics::kTextAlignCenter);
}

void CinemaViewer::updatePlayback() {
	if (_playlistEndTime) {
		if (static_cast<int32>(g_system->getMillis() - _playlistEndTime) >= 0)
			stopPlayback();
		return;
	}
	if (!_video || _paused || _enginePaused || _dragging || _releaseAction == 2)
		return;
	if (_video->endOfVideo()) {
		seekMovie(_entry + 1);
		return;
	}
	bool changed = false;
	if (_video->needsUpdate()) {
		const Graphics::Surface *frame = _video->decodeNextFrame();
		if (frame) {
			const Common::Point origin = movieOrigin();
			_canvas.blitFrom(*frame, Common::Rect(frame->w, frame->h),
				Common::Rect(origin.x, origin.y, origin.x + 400, origin.y + 200));
			changed = true;
		}
	}
	const uint previousCaption = _nextCaption;
	drawCaptions();
	if (changed || previousCaption != _nextCaption)
		present();
}

} // namespace BCB
