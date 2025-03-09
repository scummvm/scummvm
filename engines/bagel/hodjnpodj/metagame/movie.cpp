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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/metagame/movie.h"
#include "bagel/metaengine.h"
#include "bagel/hodjnpodj/events.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define SCROLL_BITMAP "video/vscroll.bmp"

Movie::Movie() : View("Movie") {
}

bool Movie::msgOpen(const OpenMessage &msg) {
	GfxSurface s = getSurface();
	s.clear();

	// Get the movie playback palette
	Common::File f;
	Image::BitmapDecoder decoder;
	if (!f.open(SCROLL_BITMAP) || !decoder.loadStream(f))
		error("Could not open - %s", SCROLL_BITMAP);
	loadPalette(decoder.getPalette());

	_video.start();
	return true;
}

bool Movie::msgClose(const CloseMessage &msg) {
	_video.close();

	if (_movieId != MOVIE_ID_TITLE)
		g_events->showCursor(false);
	// TODO: Handling when different movies end
#ifdef TODO
	if (nMovieId == MOVIE_ID_LOGO) {
		::ShowCursor(TRUE);
		StartBackgroundMidi();
		::ShowCursor(FALSE);
		PostMessage(WM_COMMAND, IDC_PLAY_TITLE_MOVIE);

	} else if (nMovieId == MOVIE_ID_ENDING) {
		::ShowCursor(TRUE);
		ShowCredits();
		BlackScreen();
		PostMessage(WM_COMMAND, IDC_MAINDLG);

	} else {
		::ShowCursor(TRUE);
		PostMessage(WM_COMMAND, IDC_MAINDLG);
	}
#endif
	return true;
}

bool Movie::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE || msg._action == KEYBIND_SELECT) {
		close();
		return true;
	}

	return View::msgAction(msg);
}

bool Movie::msgGame(const GameMessage &msg) {
	if (msg._name == "MOVIE") {
		_movieId = msg._value;
		if (!_video.loadFile(Common::Path(msg._stringValue)))
			error("Could not load video - %s", msg._stringValue.c_str());

		addView();
		return true;
	}

	return false;
}

bool Movie::tick() {
	if (_video.isPlaying()) {
		if (_video.needsUpdate())
			redraw();
	} else {
		close();
	}

	return true;
}

void Movie::draw() {
	if (_video.isPlaying() && _video.needsUpdate()) {
		GfxSurface s = getSurface();
		const Graphics::Surface *frame = _video.decodeNextFrame();
		s.blitFrom(frame, Common::Rect(0, 0, frame->w, frame->h),
			Common::Rect(0, 0, s.w, s.h));
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
