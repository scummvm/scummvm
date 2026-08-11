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

#ifndef M4_SUBTITLES_H
#define M4_SUBTITLES_H

#include "common/hashmap.h"
#include "video/subtitles.h"

namespace Common {
class TranslationManager;
} // namespace Common

namespace M4 {

class M4Subtitles : Video::Subtitles {
public:
	M4Subtitles();
	~M4Subtitles();

	void drawSubtitle(const Common::String &audioFile) const;
	void clearSubtitle() const override;
	void updateSubtitleOverlay() const override;
	bool shouldShowSubtitle() const override;
	void load();

private:
	void setupSubtitles();
	Common::String getSubtitle(const Common::String &audioFile) const;
	int16 nudgeSubtitle() const;

	Common::HashMap<Common::String, uint32> _subtitleIndices;
	bool _subtitlesEnabled;
	mutable int16 _subtitleYOffset = 0;

	// Font sizing constants (as percentage of screen height)
	const int MIN_FONT_SIZE = 8;
	const float BASE_FONT_SIZE_PERCENT = 0.023f; // ~50px at 2160p

#ifdef USE_TRANSLATION
	Common::TranslationManager *_transMan = nullptr;
#endif
};

} // namespace M4

#endif /* M4_SUBTITLES_H */
