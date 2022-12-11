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

#include "engines/stark/services/gamechapter.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/global.h"

#include "common/tokenizer.h"
#include "common/formats/ini-file.h"

namespace Stark {

GameChapter::GameChapter() : _errorText("Unknown Chapter") {
	Common::INIFile file;
	if (!file.loadFromFile("chapters.ini")) {
		error("Opening file 'chapters.ini' failed");
		return;
	}

	Common::String section = file.getSections().front().name;

	int index = 0;
	Common::String key = Common::String::format("%02d", index);
	Common::String value;

	while (file.hasKey(key, section)) {
		file.getKey(key, section, value);
		_chapterEntries.push_back(ChapterEntry());

		Common::StringTokenizer tokens(value, ":");
		_chapterEntries.back().title = tokens.nextToken();
		_chapterEntries.back().title.trim();
		_chapterEntries.back().subtitle = tokens.nextToken();
		_chapterEntries.back().subtitle.trim();

		++index;
		key = Common::String::format("%02d", index);
	}

	if (index < _numChapter) {
		error("File 'chapters.ini' is incomplete");
	}
}

const Common::String &GameChapter::getCurrentChapterTitle() const {
	return getChapterTitle(StarkGlobal->getCurrentChapter());
}

const Common::String &GameChapter::getCurrentChapterSubtitle() const {
	return getChapterSubtitle(StarkGlobal->getCurrentChapter());
}

} // End of namespace Stark
