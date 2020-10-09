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

#include "engines/stark/services/diary.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"

namespace Stark {

Diary::Diary() {
	clear();
}

Diary::~Diary() {}

void Diary::clear() {
	_diaryEntries.clear();
	_fmvEntries.clear();
	_conversationEntries.clear();
	_hasUnreadEntries = false;
	_pageIndex = 0;
}

void Diary::addDiaryEntry(const Common::String &name) {
	_diaryEntries.push_back(name);
	_hasUnreadEntries = true;
	StarkUserInterface->notifyDiaryEntryEnabled();
}

void Diary::addFMVEntry(const Common::String &filename, const Common::String &title, int gameDisc) {
	if (!hasFMVEntry(filename)) {
		FMVEntry entry;
		entry.filename = filename;
		entry.title = title;
		entry.gameDisc = gameDisc;
		_fmvEntries.push_back(entry);
	}
}

bool Diary::hasFMVEntry(const Common::String &filename) const {
	for (uint i = 0; i < _fmvEntries.size(); i++) {
		if (_fmvEntries[i].filename == filename) {
			return true;
		}
	}

	return false;
}

void Diary::readStateFromStream(Common::SeekableReadStream *stream, uint32 version) {
	clear();

	if (version <= 6) {
		return; //Early save versions did not persist the diary
	}

	ResourceSerializer serializer(stream, nullptr, version);
	saveLoad(&serializer);
}

void Diary::writeStateToStream(Common::WriteStream *stream) {
	ResourceSerializer serializer(nullptr, stream, StateProvider::kSaveVersion);
	saveLoad(&serializer);
}

void Diary::saveLoad(ResourceSerializer *serializer) {
	// Diary entries
	serializer->syncArraySize(_diaryEntries);
	for (uint i = 0; i < _diaryEntries.size(); i++) {
		serializer->syncAsString32(_diaryEntries[i]);
	}

	// FMV entries
	serializer->syncArraySize(_fmvEntries);
	for (uint i = 0; i < _fmvEntries.size(); i++) {
		serializer->syncAsString32(_fmvEntries[i].filename);
		serializer->syncAsString32(_fmvEntries[i].title);
		serializer->syncAsUint32LE(_fmvEntries[i].gameDisc);
	}

	// Conversations
	serializer->syncArraySize(_conversationEntries, 8);
	for (uint i = 0; i < _conversationEntries.size(); i++) {
		ConversationLog &entry = _conversationEntries[i];
		serializer->syncAsSint32LE(entry.chapter);
		serializer->syncAsSint32LE(entry.characterId);
		serializer->syncAsString32(entry.characterName);
		serializer->syncAsString32(entry.title);

		serializer->syncArraySize(entry.lines);
		for (uint j = 0; j < entry.lines.size(); j++) {
			ConversationLogLine &logLine = entry.lines[j];
			serializer->syncAsString32(logLine.line);
			serializer->syncAsSint32LE(logLine.characterId);
		}
	}

	// Misc
	serializer->syncAsByte(_hasUnreadEntries);
	serializer->syncAsUint32LE(_pageIndex);
}

void Diary::openDialog(const Common::String &title, const Common::String &characterName, int32 characterId) {
	// Reuse the previous dialog if it has the same title
	if (_conversationEntries.empty() || _conversationEntries.back().title != title) {
		ConversationLog conversation;
		conversation.title = title;
		conversation.characterName = characterName;
		conversation.characterId = characterId;
		conversation.chapter = StarkGlobal->getCurrentChapter();
		_conversationEntries.push_back(conversation);
	}

	_conversationEntries.back().dialogActive = true;
}

void Diary::closeDialog() {
	if (!_conversationEntries.empty()) {
		_conversationEntries.back().dialogActive = false;
	}
}

void Diary::logSpeech(const Common::String &line, int32 characterId) {
	ConversationLog &conversationLog = _conversationEntries.back();
	if (conversationLog.dialogActive) {
		ConversationLogLine logLine;
		logLine.line = line;
		logLine.characterId = characterId;

		conversationLog.lines.push_back(logLine);
	}
}

bool Diary::isEnabled() const {
	return StarkGlobal->getInventory() && StarkGlobal->hasInventoryItem("Diary");
}

bool Diary::hasUnreadEntries() const {
	return _hasUnreadEntries;
}

Diary::ConversationLog::ConversationLog() :
		dialogActive(false),
        chapter(0),
        characterId(0) {
}

Diary::ConversationLogLine::ConversationLogLine() :
		characterId(0) {
}

} // End of namespace Stark
