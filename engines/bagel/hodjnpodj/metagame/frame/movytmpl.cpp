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

#include "video/avi_decoder.h"
#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/frame/movytmpl.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480

#define STRLEN          129
#define SCROLL_BITMAP   "..\\video\\vscroll.bmp"

#pragma warning(disable: 4706)

static int      videoID;
static BOOL     bPaintScroll;

BOOL CMovieWindow::BlowWindow(CWnd *pParent, BOOL bScroll,
		LPCSTR AviMovee, int x, int y, int w, int h) {
	m_pParent = pParent;
	bPaintScroll = bScroll;

	MovieRect.SetRect(x, y, (x + w), (y + h));

	if (!pParent) {
		warning("null hwndParent");
		return FALSE;
	}

	lpszAviMovie = (LPSTR) AviMovee;

	return PlayMovie();
}

BOOL CMovieWindow::PlayMovie() {
	Video::AVIDecoder decoder;
	auto *app = AfxGetApp();
	Graphics::Screen *screen = app->getScreen();
	Common::Rect destRect = MovieRect;
	CFile file;
	Common::Event event;

	if (!file.Open(lpszAviMovie))
		return false;

	if (!decoder.loadStream(file.detach()))
		return false;

	decoder.start();

	while (!app->shouldQuit() &&
			!decoder.endOfVideo()) {
		if (decoder.hasDirtyPalette()) {
			Graphics::Palette pal(decoder.getPalette(), 256);
			AfxGetApp()->setPalette(pal);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			screen->blitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h), destRect);
		}

		bool breakFlag = false;
		while (app->pollEvents(event) && !app->shouldQuit() && !breakFlag) {
			breakFlag = ((event.type == Common::EVENT_KEYDOWN &&
				event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				(event.type == Common::EVENT_LBUTTONDOWN));
		}
		if (breakFlag)
			break;
	}

	decoder.stop();
	m_pParent->PostMessage(MOVIE_OVER);

	return TRUE;
}

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
