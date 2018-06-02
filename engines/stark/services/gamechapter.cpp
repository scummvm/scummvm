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

#include "engines/stark/services/gamechapter.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/global.h"

#include "common/archive.h"
#include "common/stream.h"
#include "common/tokenizer.h"
#include "common/debug.h"

namespace Stark {

GameChapter::GameChapter() {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("chapters.ini");
	if (!stream) {
		error("Opening 'chapters.ini' failed");
		return;
	}

	Common::String line, title, subtitle;

	// Assume that the formats of all chapters.ini are the same
	stream->readLine();
	while (!stream->eos()) {
		_chapterEntries.push_back(ChapterEntry());

		line = stream->readLine();
		Common::StringTokenizer tokens(line, "=:");

		tokens.nextToken();
		_chapterEntries.back().title = tokens.nextToken();
		_chapterEntries.back().title.trim();
		_chapterEntries.back().subtitle = tokens.nextToken();
		_chapterEntries.back().subtitle.trim();
	}
}

int GameChapter::getActualCurrentChapter() {
	return StarkGlobal->getCurrentChapter() / 10;
}

} // End of namespace Stark
