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

#ifndef STARK_SERVICES_DIARY_H
#define STARK_SERVICES_DIARY_H

#include "common/str.h"
#include "common/str-array.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Stark {

class ResourceSerializer;

/**
 * Diary state storage
 *
 * Stores references to all the unlocked data available from the diary menu
 */
class Diary {
public:
	struct ConversationLogLine {
		Common::String line;
		int32 characterId;

		ConversationLogLine();
	};

	struct ConversationLog {
		Common::String title;
		Common::String characterName;
		int32 characterId;
		int32 chapter;
		bool dialogActive;
		Common::Array<ConversationLogLine> lines;

		ConversationLog();
	};

	Diary();
	virtual ~Diary();

	/** Does the player have the diary in their inventory? */
	bool isEnabled() const;

	/** Does the diary contain entries that have not been read yet? */
	bool hasUnreadEntries() const;

	/** Mark all the diary entries read */
	void setDiaryAllRead() { _hasUnreadEntries = false; }

	/** Add an entry to the list of available diary pages */
	void addDiaryEntry(const Common::String &name);

	/** Get and set the current diary page index */
	uint32 getPageIndex() const { return _pageIndex; };
	void setPageIndex(uint32 pageIndex) { _pageIndex = pageIndex; }

	/** Add a FMV entry to the list of movies available to play from the diary */
	void addFMVEntry(const Common::String &filename, const Common::String &title, int gameDisc);

	/** Get info of added FMV entries */
	uint countFMV() const { return _fmvEntries.size(); }
	const Common::String &getFMVFilename(uint index) const { return _fmvEntries[index].filename; }
	const Common::String &getFMVTitle(uint index) const { return _fmvEntries[index].title; }

	/** Get info of added Diary entries */
	uint countDiary() const { return _diaryEntries.size(); }
	const Common::String &getDiary(uint index) const { return _diaryEntries[index]; }

	/** Get added Dialog entries */
	uint countDialog() const { return _conversationEntries.size(); }
	const ConversationLog &getDialog(uint index) const { return _conversationEntries[index]; }

	/** Start recording speech lines for a dialog */
	void openDialog(const Common::String &title, const Common::String &characterName, int32 characterId);

	/** Record a speech line for the previously opened dialog */
	void logSpeech(const Common::String &line, int32 characterId);

	/** Close the currently active dialog */
	void closeDialog();

	/** Reset all the game state data */
	void clear();

	/** Replace the current state by that read from the stream */
	void readStateFromStream(Common::SeekableReadStream *stream, uint32 version);

	/** Write the state to a stream */
	void writeStateToStream(Common::WriteStream *stream);

private:
	struct FMVEntry {
		Common::String filename;
		Common::String title;
		int gameDisc;
	};

	bool hasFMVEntry(const Common::String &filename) const;
	void saveLoad(ResourceSerializer *serializer);

	Common::Array<Common::String> _diaryEntries;
	Common::Array<FMVEntry> _fmvEntries;
	Common::Array<ConversationLog> _conversationEntries;

	bool _hasUnreadEntries;
	uint32 _pageIndex;
};

} // End of namespace Stark

#endif // STARK_SERVICES_DIARY_H
