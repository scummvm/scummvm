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

#include "common/archive.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/screen.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/cinema.h"
#include "harold/bcb/static_data.h"
#include "harold/bcb/translation.h"
#include "video/avi_decoder.h"

namespace BCB {

CinemaViewer::CinemaViewer(BCBEngine &engine) : _engine(engine) {
}

CinemaViewer::~CinemaViewer() {
	_video.reset();
}

Common::Rect CinemaViewer::titleButtonRect() {
	const ButtonControlRecord &button = kTitleButtonTable[4];
	return Common::Rect(24, button.y, 24 + button.width, button.y + button.height);
}

void CinemaViewer::drawTitleButton(BCBEngine &engine, Graphics::Surface &surface) {
	const Common::Rect rect = titleButtonRect();
	Graphics::Surface buttons;
	buttons.create(rect.width(), rect.height() * 2, surface.format);
	for (int state = 0; state < 2; ++state) {
		buttons.copyRectToSurface(surface, 0, state * rect.height(),
			Common::Rect(state * rect.width(), 736, (state + 1) * rect.width(), 736 + rect.height()));
		for (int y = 3; y < rect.height() - 3; ++y) {
			const int row = state * rect.height() + y;
			buttons.fillRect(Common::Rect(3, row, rect.width() - 3, row + 1), buttons.getPixel(3, row));
		}
	}
	if (!applyTranslatedCinemaTitleButton(buttons, engine._textTranslation)) {
		const Graphics::Font *font = engine._ownerDrawUiTextFont;
		if (!font)
			font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		const Common::U32String name = engine.translateSystemTextForDisplay("cinema_title",
			Common::String("\x83\x56\x83\x6c\x83\x7d" "DISK").decode(Common::kWindows932));
		for (int state = 0; state < 2; ++state) {
			const int y = state * rect.height() + (rect.height() - font->getFontHeight()) / 2;
			const uint32 ink = state ? surface.format.RGBToColor(136, 90, 155) : surface.format.RGBToColor(53, 55, 106);
			const uint32 highlight = state ? surface.format.RGBToColor(183, 169, 211) : surface.format.RGBToColor(142, 146, 232);
			font->drawString(&buttons, name, 5, y - 1, rect.width() - 8, highlight, Graphics::kTextAlignCenter);
			font->drawString(&buttons, name, 4, y, rect.width() - 8, ink, Graphics::kTextAlignCenter);
		}
	}
	surface.copyRectToSurface(buttons, rect.left, rect.top, Common::Rect(rect.width(), rect.height()));
	// Cache the pressed state in the unused right-hand title atlas area.
	surface.copyRectToSurface(buttons, 324, 736, Common::Rect(0, rect.height(), rect.width(), rect.height() * 2));
	buttons.free();
}

void CinemaViewer::pressTitleButton(BCBEngine &engine) {
	const Common::Rect rect = titleButtonRect();
	engine._screen->copyRectToSurface(engine._titleBitmap->getBasePtr(324, 736), engine._titleBitmap->pitch,
		rect.left, rect.top, rect.width(), rect.height());
	engine._screen->update();
}

void BCBEngine::runCinemaViewer() {
	if (!_cinemaMenu) {
		warning("BCB: Unable to load bcb-cinema.dat");
		restoreTitleScreenRegion(CinemaViewer::titleButtonRect());
		return;
	}
	const uint32 playTime = getTotalPlayTime();
	_cinemaViewer = _cinemaMenu;
	_titleButtonsActive = false;
	stopMenuBackgroundMusic();
	_cinemaMenu->run();
	_cinemaViewer = nullptr;
	setTotalPlayTime(playTime);
	_lastTimerTick = g_system->getMillis();
	if (!shouldQuit()) {
		showTitleScreen();
		startMenuBackgroundMusic();
	}
}

void BCBEngine::pauseEngineIntern(bool paused) {
	Engine::pauseEngineIntern(paused);
	if (_cinemaViewer)
		_cinemaViewer->pause(paused);
}

void CinemaViewer::pause(bool paused) {
	_enginePaused = paused;
	syncMoviePause();
}

Common::Point CinemaViewer::dialogOrigin() const {
	if (_page == kPlayback)
		return Common::Point(189, 400);
	// ScummVM uses the same 640x480 canvas in windowed and fullscreen modes.
	const uint16 *size = kCinemaDialogSizes[_page == kBackground ? 3 : 1];
	return Common::Point((640 - size[0]) / 2, (480 - size[1]) / 2);
}

Common::Point CinemaViewer::movieOrigin() const {
	return _backgroundIndex == 5 ? Common::Point(46, 51) : Common::Point(120, 72);
}

void CinemaViewer::drawButton(int index) {
	const CinemaButton &button = kCinemaButtons[index];
	const Graphics::ManagedSurface &sheet = _sheets[index < 12 ? 0 : index < 20 ? 1 : 2];
	_canvas.blitFrom(sheet, button.source, Common::Point(button.destinationX, button.destinationY) + dialogOrigin());
}

void CinemaViewer::drawSelection(bool reveal) {
	if (_engine._titleBitmap)
		_canvas.blitFrom(*_engine._titleBitmap, Common::Rect(640, 480), Common::Point(0, 0));
	else
		_canvas.clear(0);
	if (_canvas.format.bytesPerPixel == 4) {
		const uint32 keepMask = ~_canvas.format.ARGBToColor(0, 255, 255, 255);
		const uint32 halfMask = _canvas.format.ARGBToColor(0, 127, 127, 127);
		for (int y = 0; y < _canvas.h; ++y) {
			uint32 *row = static_cast<uint32 *>(_canvas.getBasePtr(0, y));
			for (int x = 0; x < _canvas.w; ++x)
				row[x] = (row[x] & keepMask) | ((row[x] >> 1) & halfMask);
		}
	} else {
		for (int y = 0; y < _canvas.h; ++y) {
			for (int x = 0; x < _canvas.w; ++x) {
				byte a, r, g, b;
				_canvas.format.colorToARGB(_canvas.getPixel(x, y), a, r, g, b);
				_canvas.setPixel(x, y, _canvas.format.ARGBToColor(a, r / 2, g / 2, b / 2));
			}
		}
	}
	const int sheet = _page == kBackground ? 0 : 1;
	const Common::Rect rect = _page == kBackground ? Common::Rect(420, 199) : Common::Rect(260, 192);
	_canvas.blitFrom(_sheets[sheet], rect, dialogOrigin());
	if (_page == kPlaylist)
		drawRows();
	if (reveal) {
		_engine.blitScreenWithLineReveal(_canvas);
		_canvas.clearDirtyRects();
	} else
		present();
}

void CinemaViewer::drawRows() {
	const Common::Point origin = dialogOrigin();
	const Graphics::Font *font = _listFont ? _listFont.get() : _engine._ownerDrawUiTextFont;
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	Common::Rect area(15, 15, 195, 127);
	area.translate(origin.x, origin.y);
	_canvas.fillRect(area, _canvas.format.RGBToColor(255, 255, 255));
	for (int row = 0; row < 4; ++row) {
		Common::Rect bounds(15, 15 + row * 28, 195, 43 + row * 28);
		bounds.translate(origin.x, origin.y);
		const bool selected = row == _selectedRow;
		if (selected)
			_canvas.fillRect(bounds, _canvas.format.RGBToColor(0, 0, 0));
		font->drawString(&_canvas, _labels[_scrollOffset + row], origin.x + 16,
			origin.y + 33 + row * 28 - font->getFontAscent(), 178,
			selected ? _canvas.format.RGBToColor(255, 255, 255) : _canvas.format.RGBToColor(0, 0, 0));
	}
}

void CinemaViewer::scroll(int delta) {
	_scrollOffset = CLIP<int>(_scrollOffset + delta, 0, ARRAYSIZE(kCinemaPlaylists) - 4);
	_selectedRow = -1;
	drawRows();
}

void CinemaViewer::Canvas::addDirtyRect(const Common::Rect &rect) {
	Common::Rect clipped = rect;
	clipped.clip(getBounds());
	if (!clipped.isEmpty())
		_dirtyRects.push_back(clipped);
}

void CinemaViewer::Canvas::presentTo(Graphics::Screen &screen) {
	_dirtyRects.merge();
	for (const Common::Rect &rect : _dirtyRects)
		screen.copyRectToSurface(getBasePtr(rect.left, rect.top), pitch, rect.left, rect.top, rect.width(), rect.height());
	_dirtyRects.clear();
	screen.update();
}

void CinemaViewer::present() {
	_canvas.presentTo(*_engine._screen);
}

void CinemaViewer::back() {
	_releaseAction = 0;
	_dragging = false;
	release();
	if (_page == kPlayback) {
		stopPlayback();
	} else if (_page == kPlaylist) {
		_page = kBackground;
		drawSelection(true);
	} else {
		_running = false;
	}
}

void CinemaViewer::release() {
	_heldScroll = 0;
	if (_dragging) {
		_dragging = false;
		seekMovie(_seekEntry);
	}
	if (_pressedButton >= 0) {
		drawButton(_pressedButton - 1);
		_pressedButton = -1;
		present();
	}
	const int action = _releaseAction;
	_releaseAction = 0;
	if (action == 1 && _selectedPlaylist)
		showPlayback();
	else if (action == 2)
		stopPlayback();
	else if (action == 3)
		_running = false;
}

void CinemaViewer::click(Common::Point mouse) {
	const Common::Point local = mouse - dialogOrigin();
	if (_page == kPlayback && Common::Rect(0, 40, 262, 56).contains(local)) {
		if (_playlistEndTime)
			return;
		const Common::Rect thumb = thumbRect();
		if (thumb.contains(mouse)) {
			_dragging = true;
			_seekEntry = _entry;
			_thumbOffset = mouse.x - thumb.left;
			syncMoviePause();
		} else {
			const int step = local.x < 16 ? -1 : local.x >= 246 ? 1 : mouse.x < thumb.left ? -4 : 4;
			seekMovie(_entry + step);
		}
		return;
	}
	const Common::Span<const StaticRect> items = _page == kBackground ?
		Common::Span<const StaticRect>(kCinemaBackgroundItems, ARRAYSIZE(kCinemaBackgroundItems) - 1) :
		_page == kPlaylist ? Common::Span<const StaticRect>(kCinemaPlaylistItems, ARRAYSIZE(kCinemaPlaylistItems)) :
		Common::Span<const StaticRect>(kCinemaPlaybackItems, ARRAYSIZE(kCinemaPlaybackItems));
	for (uint item = 0; item < items.size(); ++item) {
		if (!items[item].contains(local))
			continue;
		if (_page == kBackground) {
			if (item < 4) {
				drawButton(item * 2 + 1);
				present();
				const int background = item ? item + 2 : 0;
				if (background && !loadPicture(background, _background)) {
					drawSelection();
					return;
				}
				_backgroundIndex = background;
				_page = kPlaylist;
				drawSelection(true);
			} else if (item == 4) {
				_pressedButton = 9;
				_releaseAction = 3;
				drawButton(9);
				present();
			}
		} else if (_page == kPlaylist) {
			if (item < 2) {
				_heldScroll = item ? 1 : -1;
				_pressedButton = item ? 15 : 13;
				drawButton(_pressedButton);
				scroll(_heldScroll);
				_repeatTime = g_system->getMillis() + 117;
				present();
			} else if (item == 2) {
				_pressedButton = 17;
				_releaseAction = 1;
				drawButton(17);
				present();
			} else if (item == 3) {
				back();
			} else {
				_selectedRow = item - 4;
				_selectedPlaylist = _scrollOffset + _selectedRow + 1;
				drawRows();
				present();
			}
		} else if (!_playlistEndTime) {
			if (item == 0)
				seekMovie(_entry + 1);
			else if (item == 1)
				togglePause();
			else {
				_pressedButton = 25;
				_releaseAction = 2;
				syncMoviePause();
				drawButton(25);
				present();
			}
		}
		return;
	}
}

bool CinemaViewer::pumpEvents(bool interactive) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			_engine.quitGame();
			_running = false;
			return false;
		}
		if (!interactive)
			continue;
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_engine._mousePosition = event.mouse;
			if (_dragging) {
				const int x = CLIP<int>(event.mouse.x - 189 - 16 - _thumbOffset, 0, 214);
				_seekEntry = (x * _movies.size() + 107) / 214;
				drawScrollbar();
				present();
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			_engine._mousePosition = event.mouse;
			click(event.mouse);
			break;
		case Common::EVENT_LBUTTONUP:
			release();
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType == kBCBActionInteract)
				click(_engine._mousePosition);
			else if (event.customType == kBCBActionEscape)
				back();
			else if (event.customType == kBCBActionSkip && _page == kPlayback)
				seekMovie(_entry + 1);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			if (event.customType == kBCBActionInteract)
				release();
			break;
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			if (_page == kPlaylist) {
				scroll(event.type == Common::EVENT_WHEELUP ? -1 : 1);
				present();
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				back();
			else if (event.kbd.keycode == Common::KEYCODE_SPACE && _page == kPlayback)
				seekMovie(_entry + 1);
			else if (event.kbd.keycode == Common::KEYCODE_p && _page == kPlayback)
				togglePause();
			break;
		default:
			break;
		}
	}
	return _running && !_engine.shouldQuit();
}

void CinemaViewer::wait(uint millis) {
	const uint32 start = g_system->getMillis();
	while (_running && !_engine.shouldQuit() && g_system->getMillis() - start < millis) {
		pumpEvents(false);
		_engine._screen->update();
		g_system->delayMillis(5);
	}
}

void CinemaViewer::run() {
	_page = kBackground;
	_backgroundIndex = 0;
	_scrollOffset = 0;
	_selectedPlaylist = 0;
	_selectedRow = -1;
	_entry = 0;
	_nextCaption = 0;
	_pressedButton = -1;
	_releaseAction = 0;
	_heldScroll = 0;
	_playlistEndTime = 0;
	_paused = _enginePaused = _dragging = false;
	_running = true;
	drawSelection(true);
	while (pumpEvents()) {
		if (_heldScroll && static_cast<int32>(g_system->getMillis() - _repeatTime) >= 0) {
			scroll(_heldScroll);
			_repeatTime = g_system->getMillis() + 117;
			present();
		}
		if (_page == kPlayback)
			updatePlayback();
		_engine._screen->update();
		g_system->delayMillis(5);
	}
	_video.reset();
	_background.free();
	_movies.clear();
	_captions.clear();
}

} // namespace BCB
