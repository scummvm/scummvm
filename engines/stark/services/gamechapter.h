/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_SERVICES_GAME_CHAPTER_H
#define STARK_SERVICES_GAME_CHAPTER_H

#include "common/str.h"
#include "common/array.h"

namespace Stark {

/**
 * Game chapter services
 * 
 * Provide the game chapter's title and subtitle
 */
class GameChapter {
public:
	GameChapter();
	~GameChapter() {}

	const Common::String &getChapterTitle(uint chapter) const {
		if (chapter >= _numChapter * 10) {
			return  _errorText;
		} else {
			return _chapterEntries[chapter / 10].title;
		}
	}

	const Common::String &getCurrentChapterTitle() const;

	const Common::String &getChapterSubtitle(uint chapter) const {
		if (chapter >= _numChapter * 10) {
			return _errorText;
		} else {
			return _chapterEntries[chapter / 10].subtitle;
		}
	}

	const Common::String &getCurrentChapterSubtitle() const;

private:
	static const int _numChapter = 15;

	struct ChapterEntry {
		Common::String title;
		Common::String subtitle;
	};

	Common::Array<ChapterEntry> _chapterEntries;
	Common::String _errorText;
};

} // End of namespace Stark

#endif // STARK_SERVICES_GAME_CHAPTER_H
