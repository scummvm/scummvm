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
 */

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"

#include "m4/m4.h"
#include "m4/subtitles.h"
#include "m4/platform/sound/digi.h"

namespace M4 {

M4Subtitles::M4Subtitles() {
	_loaded = loadSubtitles(IS_RIDDLE ? "riddle.pot" : "burger.pot");
	_subtitlesEnabled = ConfMan.getBool("subtitles");
	if (_loaded) {
		setupSubtitles();
		g_system->showOverlay(false);
	}
}

M4Subtitles::~M4Subtitles() {
	clearSubtitle();
}

void M4Subtitles::setupSubtitles() {
	// Subtitle positioning constants (as percentages of screen height)
	const int HORIZONTAL_MARGIN = 20;
	const float BOTTOM_MARGIN_PERCENT = 0.009f; // ~20px at 2160p
	const float DEFAULT_HEIGHT_PERCENT = IS_RIDDLE ? 0.2f : 0.26f; // ~432px at 2160p

	// Font sizing constants (as percentage of screen height)
	const int MIN_FONT_SIZE = 8;
	const float BASE_FONT_SIZE_PERCENT = 0.023f; // ~50px at 2160p

	int16 h = g_system->getOverlayHeight();
	int16 w = g_system->getOverlayWidth();
	int fontSize = MAX(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));

	int bottomMargin = int(h * BOTTOM_MARGIN_PERCENT);

	int topOffset = int(h * DEFAULT_HEIGHT_PERCENT);
	setBBox(Common::Rect(HORIZONTAL_MARGIN,
						 h - topOffset,
						 w - HORIZONTAL_MARGIN,
						 h - bottomMargin));

	setColor(0xff, 0xff, 0xff);
	setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
}

bool M4Subtitles::loadSubtitles(const Common::String &filename) {
	Common::File inFile;
	Common::String line;
	Common::String lastComment;

	if (!inFile.open(filename.c_str()))
		return false;

	while (!inFile.eos()) {
		line = inFile.readLine();

		if (line.hasPrefix("#: ")) {
			lastComment = line.substr(3);
		}

		if (line.hasPrefix("msgid ") && !lastComment.empty()) {
			_subtitles[lastComment] = line.substr(7, line.size() - 7 - 1); // Remove trailing "
		}
	}

	inFile.close();

	return true;
}

Common::String M4Subtitles::getSubtitle(const Common::String &audioFile) const {
	Common::String key = audioFile;
	key.toUppercase();
	return _subtitles.contains(key) ? _subtitles[key] : "";
}

bool M4Subtitles::drawSubtitle(const Common::String &audioFile) const {
	if (!_loaded || !_subtitlesEnabled)
		return false;

	Common::String subtitle = getSubtitle(audioFile);
	if (subtitle.empty())
		return false;

	Common::Array<Video::SubtitlePart> parts;
	parts.push_back(Video::SubtitlePart(subtitle, ""));
	_parts = &parts;

	recalculateBoundingBox();
	renderSubtitle();
	_parts = nullptr;

	g_system->showOverlay(false);
	updateSubtitleOverlay();

	return true;
}

void M4Subtitles::updateSubtitleOverlay() const {
	if (!_loaded || !_subtitlesEnabled)
		return;

	if (!digi_play_state(1) && !digi_play_state(2)) {
		// No subtitle to show
		g_system->hideOverlay();
		return;
	} else {
		if (!g_system->isOverlayVisible())
			g_system->showOverlay(false);
	}

	if (_overlayHasAlpha) {
		// When we have alpha, draw the whole surface without thinking it more
		g_system->copyRectToOverlay(_surface.getPixels(), _surface.pitch, _realBBox.left, _realBBox.top, _realBBox.width(), _realBBox.height());
	} else {
		// When overlay doesn't have alpha, showing it hides the underlying game screen
		// We force a copy of the game screen to the overlay by clearing it
		// We then draw the smallest possible surface to minimize black rectangle behind text
		g_system->clearOverlay();
		g_system->copyRectToOverlay((byte *)_surface.getPixels() + _drawRect.top * _surface.pitch + _drawRect.left * _surface.format.bytesPerPixel, _surface.pitch,
									_realBBox.left + _drawRect.left, _realBBox.top + _drawRect.top, _drawRect.width(), _drawRect.height());
	}
}

void M4Subtitles::clearSubtitle() const {
	if (!_loaded || !_subtitlesEnabled)
		return;

	g_system->hideOverlay();
	g_system->clearOverlay();
	_drawRect.setEmpty();
	_surface.fillRect(Common::Rect(0, 0, _surface.w, _surface.h), _transparentColor);
}

} // namespace M4
