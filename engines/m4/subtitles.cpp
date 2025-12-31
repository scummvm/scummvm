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
#include "common/translation.h"

#include "m4/m4.h"
#include "m4/subtitles.h"
#include "m4/platform/sound/digi.h"

namespace M4 {

M4Subtitles::M4Subtitles() {
	_subtitlesEnabled = ConfMan.getBool("subtitles");
}

M4Subtitles::~M4Subtitles() {
	clearSubtitle();
}

void M4Subtitles::load() {
	Common::StringArray soundFiles;
	uint32 index = 0;

#ifdef USE_TRANSLATION
	_transMan = new Common::TranslationManager(IS_RIDDLE ? "riddle_translations.dat" : "burger_translations.dat");
	_transMan->setLanguage(TransMan.getCurrentLanguage());
	soundFiles = _transMan->getContexts();

	for (const auto &soundFile : soundFiles) {
		Common::String key = soundFile;
		key.toUppercase();
		_subtitleIndices[key] = index++;
	}
#endif

	_loaded = !soundFiles.empty();
	if (_loaded)
		setupSubtitles();
}

void M4Subtitles::setupSubtitles() {
	// Subtitle positioning constants (as percentages of screen height)
	const int HORIZONTAL_MARGIN = 20;
	const float BOTTOM_MARGIN_PERCENT = 0.009f; // ~20px at 2160p
	const float DEFAULT_HEIGHT_PERCENT = IS_RIDDLE ? 0.2f : 0.26f; // ~432px at 2160p

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

Common::String M4Subtitles::getSubtitle(const Common::String &audioFile) const {
#ifdef USE_TRANSLATION
	Common::String key = audioFile;
	key.toUppercase();

	if (_subtitleIndices.contains(key)) {
		return _transMan->getTranslation(_subtitleIndices[key]).encode();
	}
#endif

	return "";
}

void M4Subtitles::drawSubtitle(const Common::String &audioFile) const {
	if (!_loaded || !_subtitlesEnabled)
		return;

	Common::String subtitle = getSubtitle(audioFile);
	if (subtitle.empty())
		return;

	Common::Array<Video::SubtitlePart> parts;
	parts.push_back(Video::SubtitlePart(subtitle, ""));
	_parts = &parts;

	recalculateBoundingBox();
	renderSubtitle();
	_subtitleYOffset = nudgeSubtitle();
	_parts = nullptr;
}

int16 M4Subtitles::nudgeSubtitle() const {
	// Nudge subtitle text box upwards, depending on the lines to draw
	int16 h = g_system->getOverlayHeight();
	int fontSize = MAX(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));

	return _splitPartCount > 0 ? static_cast<int16>((_splitPartCount - 1) * fontSize) : 0;
}

bool M4Subtitles::shouldShowSubtitle() const {
	return digi_play_state(1) || digi_play_state(2);
}

void M4Subtitles::updateSubtitleOverlay() const {
	if (!_subtitlesEnabled)
		return;

	translateBBox(0, -_subtitleYOffset);
	Subtitles::updateSubtitleOverlay();
	translateBBox(0, _subtitleYOffset);
}

void M4Subtitles::clearSubtitle() const {
	if (!_subtitlesEnabled)
		return;

	Subtitles::clearSubtitle();
}

} // namespace M4
