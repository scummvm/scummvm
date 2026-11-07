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

#ifndef HAROLD_BCB_CINEMA_H
#define HAROLD_BCB_CINEMA_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/span.h"
#include "common/ustr.h"
#include "graphics/dirtyrects.h"
#include "graphics/managed_surface.h"
#include "harold/bcb/cinema_tables.h"

namespace Common {
class Archive;
class SeekableReadStream;
}
namespace Graphics {
class Font;
class Screen;
}
namespace Video {
class AVIDecoder;
}

namespace BCB {

class BCBEngine;

class CinemaViewer {
public:
	explicit CinemaViewer(BCBEngine &engine);
	~CinemaViewer();
	bool load();
	void run();
	void pause(bool paused);
	static Common::SeekableReadStream *openPackage(BCBEngine &engine);
	static Common::Rect titleButtonRect();
	static void drawTitleButton(BCBEngine &engine, Graphics::Surface &surface);
	static void pressTitleButton(BCBEngine &engine);

private:
	enum Page { kBackground, kPlaylist, kPlayback };
	class Canvas : public Graphics::ManagedSurface {
	public:
		void addDirtyRect(const Common::Rect &rect) override;
		void clearDirtyRects() override { _dirtyRects.clear(); }
		void presentTo(Graphics::Screen &screen);
	private:
		Graphics::DirtyRectList _dirtyRects;
	};
	struct Caption {
		int frame;
		Common::U32String lines[2];
	};

	bool loadLabels();
	bool loadPicture(uint index, Graphics::ManagedSurface &surface);
	bool loadPlaylist();
	void loadCaptions(uint movie);
	void drawSelection(bool reveal = false);
	void drawRows();
	void drawButton(int index);
	void drawControls();
	void drawScrollbar();
	Common::Rect thumbRect() const;
	void drawCaptions();
	void clearCaptions();
	void showPlayback();
	bool startMovie();
	void stopPlayback();
	void seekMovie(int entry);
	void updatePlayback();
	void click(Common::Point mouse);
	void release();
	void back();
	void togglePause();
	void syncMoviePause();
	void scroll(int delta);
	void present();
	bool pumpEvents(bool interactive = true);
	void wait(uint millis);
	void revealBackground();
	Common::Point dialogOrigin() const;
	Common::Point movieOrigin() const;
	Common::U32String translate(const Common::String &key, const Common::U32String &original) const;

	BCBEngine &_engine;
	Common::ScopedPtr<Common::Archive> _archive;
	Graphics::ManagedSurface _sheets[3];
	Graphics::ManagedSurface _background;
	Canvas _canvas;
	Common::ScopedPtr<Graphics::Font> _listFont;
	Common::ScopedPtr<Graphics::Font> _captionFont;
	Common::ScopedPtr<Video::AVIDecoder> _video;
	Common::Array<Common::U32String> _labels;
	Common::Span<const CinemaMovie> _movies;
	Common::Array<Caption> _captions;
	Page _page = kBackground;
	int _backgroundIndex = 0;
	int _scrollOffset = 0;
	int _selectedPlaylist = 0;
	int _selectedRow = -1;
	int _entry = 0;
	uint _nextCaption = 0;
	int _captionSize = 12;
	int _pressedButton = -1;
	int _releaseAction = 0;
	int _heldScroll = 0;
	uint32 _repeatTime = 0;
	uint32 _playlistEndTime = 0;
	bool _running = false;
	bool _paused = false;
	bool _enginePaused = false;
	bool _dragging = false;
	int _seekEntry = 0;
	int _thumbOffset = 0;
};

} // namespace BCB

#endif
