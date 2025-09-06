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
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480

#define STRLEN          129
#define SCROLL_BITMAP   "..\\video\\vscroll.bmp"

#pragma warning(disable: 4706)

//static int      videoID;
static bool     bPaintScroll;

bool CMovieWindow::BlowWindow(CWnd *pParent, bool bScroll,
		const char *AviMovee, int x, int y, int w, int h) {
	m_pParent = pParent;
	bPaintScroll = bScroll;

	MovieRect.SetRect(x, y, (x + w), (y + h));

	if (!pParent) {
		warning("null hwndParent");
		return false;
	}

	lpszAviMovie = AviMovee;

	return PlayMovie();
}

bool CMovieWindow::PlayMovie() {
	Video::AVIDecoder decoder;
	CWinApp *app = AfxGetApp();
	Graphics::Screen *screen = app->getScreen();
	Common::Rect destRect = MovieRect;
	CFile file;
	Common::Event event;

	if (!file.Open(lpszAviMovie))
		return false;

	if (!decoder.loadStream(file.detach()))
		return false;

	decoder.start();

	while (!decoder.endOfVideo()) {
		if (decoder.hasDirtyPalette()) {
			Graphics::Palette pal(decoder.getPalette(), 256);
			app->setPalette(pal);
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			screen->blitFrom(*frame, Common::Rect(0, 0, frame->w, frame->h), destRect);
			screen->update();
		}

		bool breakFlag = false;
		while (g_system->getEventManager()->pollEvent(event) && !breakFlag) {
			breakFlag = (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
				event.customType == KEYBIND_ESCAPE) ||
				(event.type == Common::EVENT_LBUTTONDOWN) ||
				(event.type == Common::EVENT_QUIT) ||
				(event.type == Common::EVENT_RETURN_TO_LAUNCHER);
		}
		if (breakFlag)
			break;
	}

	decoder.stop();

	if (event.type == Common::EVENT_QUIT ||
		event.type == Common::EVENT_RETURN_TO_LAUNCHER)
		app->quit();
	else
		m_pParent->PostMessage(WM_COMMAND, MOVIE_OVER);

	return true;
}

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
