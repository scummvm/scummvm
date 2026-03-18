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

#ifndef HARVESTER_TEXT_H
#define HARVESTER_TEXT_H

#include "common/array.h"
#include "common/str.h"

namespace Harvester {

class ResourceManager;

struct DialogueIndexEntry {
	int wavId = 0;
	uint32 textOffset = 0;
	uint32 textLength = 0;
};

struct CftFontResource {
	Common::String path;
	Common::String name;
	Common::Array<byte> header;
	uint32 atlasWidth = 0;
	uint32 atlasHeight = 0;
	Common::Array<byte> atlasPixels;
};

class Text {
public:
	bool load(ResourceManager &resources);

	const Common::Array<byte> &getDialogueData() const { return _dialogueData; }
	const Common::Array<DialogueIndexEntry> &getDialogueEntries() const { return _dialogueEntries; }
	const Common::Array<Common::String> &getDialogueResponseLines() const { return _dialogueResponseLines; }
	const Common::Array<CftFontResource> &getFonts() const { return _fonts; }
	bool resolveDialogueSubtitle(int wavId, Common::String &text) const;
	Common::String getDialogueResponseLine(int zeroBasedIndex) const;

private:
	bool loadDialogueIndex(ResourceManager &resources);
	bool loadDialogueResponses(ResourceManager &resources);
	bool loadFont(ResourceManager &resources, const Common::String &path);
	void decodeXorText(Common::Array<byte> &data) const;

	Common::Array<byte> _dialogueData;
	Common::Array<DialogueIndexEntry> _dialogueEntries;
	Common::Array<Common::String> _dialogueResponseLines;
	Common::Array<CftFontResource> _fonts;
};

} // End of namespace Harvester

#endif // HARVESTER_TEXT_H
