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

#include "harvester/startup_text.h"

#include "common/debug.h"
#include "common/endian.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const char *const kStartupFontPaths[] = {
	"GRAPHIC/FONT/HARVFONT.CFT",
	"GRAPHIC/FONT/HARVFNT2.CFT",
	"GRAPHIC/FONT/TEXTFONT.CFT",
	"GRAPHIC/FONT/TEXTFNT2.CFT",
	"GRAPHIC/FONT/MEDFONT1.CFT",
	"GRAPHIC/FONT/MEDFONT2.CFT"
};
static const char *const kDialogueResponsePath = "DIALOG.RSP";

static const uint32 kCftHeaderSize = 0x448;
static const uint32 kCftBitmapHeaderSize = kCftHeaderSize + 12;

static bool isDialogueDelimiter(byte value) {
	return value == 0 || value == '\n' || value == '\r' || value == '\f';
}

} // End of anonymous namespace

bool StartupText::load(ResourceManager &resources) {
	_dialogueData.clear();
	_dialogueEntries.clear();
	_dialogueResponseLines.clear();
	_fonts.clear();

	if (!loadDialogueIndex(resources) || !loadDialogueResponses(resources))
		return false;

	for (const char *path : kStartupFontPaths) {
		if (!loadFont(resources, path))
			return false;
	}

	debug(1, "Harvester: loaded %u dialogue entries and %u startup fonts",
		(uint)_dialogueEntries.size(), (uint)_fonts.size());
	return true;
}

bool StartupText::loadDialogueIndex(ResourceManager &resources) {
	if (!resources.loadFile("DIALOGUE.IDX", _dialogueData) || _dialogueData.empty()) {
		warning("Harvester: unable to load DIALOGUE.IDX");
		return false;
	}

	decodeXorText(_dialogueData);

	uint32 cursor = 0;
	while (cursor < _dialogueData.size()) {
		while (cursor < _dialogueData.size() && isDialogueDelimiter(_dialogueData[cursor]))
			++cursor;
		if (cursor >= _dialogueData.size())
			break;

		const uint32 wavIdStart = cursor;
		while (cursor < _dialogueData.size() && !isDialogueDelimiter(_dialogueData[cursor]))
			++cursor;

		const Common::String wavIdString((const char *)_dialogueData.data() + wavIdStart, cursor - wavIdStart);
		const int wavId = atoi(wavIdString.c_str());

		while (cursor < _dialogueData.size() && isDialogueDelimiter(_dialogueData[cursor]))
			++cursor;
		if (cursor >= _dialogueData.size())
			break;

		DialogueIndexEntry entry;
		entry.wavId = wavId;
		entry.textOffset = cursor;
		while (cursor < _dialogueData.size() && !isDialogueDelimiter(_dialogueData[cursor]))
			++cursor;
		entry.textLength = MIN<uint32>(cursor - entry.textOffset, 0x19a);

		if (entry.wavId > 0 && entry.textLength != 0)
			_dialogueEntries.push_back(entry);
	}

	return !_dialogueEntries.empty();
}

bool StartupText::loadDialogueResponses(ResourceManager &resources) {
	Common::Array<byte> responseData;
	if (!resources.loadFile(kDialogueResponsePath, responseData) || responseData.empty()) {
		warning("Harvester: unable to load %s", kDialogueResponsePath);
		return false;
	}

	Common::String line;
	for (byte value : responseData) {
		if (value == '\r')
			continue;
		if (value == '\n') {
			_dialogueResponseLines.push_back(line);
			line.clear();
			continue;
		}

		line += (char)value;
	}

	if (!line.empty())
		_dialogueResponseLines.push_back(line);

	return !_dialogueResponseLines.empty();
}

bool StartupText::loadFont(ResourceManager &resources, const Common::String &path) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < kCftBitmapHeaderSize) {
		warning("Harvester: unable to load font '%s'", path.c_str());
		return false;
	}

	CftFontResource font;
	font.path = path;
	font.header.resize(kCftHeaderSize);
	memcpy(font.header.data(), data.data(), kCftHeaderSize);

	for (uint i = 0; i < 0x40 && i < data.size(); ++i) {
		if (data[i] == 0)
			break;
		font.name += (char)data[i];
	}

	font.atlasWidth = READ_LE_UINT32(data.data() + kCftHeaderSize);
	font.atlasHeight = READ_LE_UINT32(data.data() + kCftHeaderSize + 4);
	const uint32 pixelCount = font.atlasWidth * font.atlasHeight;
	if (font.atlasWidth == 0 || font.atlasHeight == 0 || data.size() < kCftBitmapHeaderSize + pixelCount) {
		warning("Harvester: invalid font '%s'", path.c_str());
		return false;
	}

	font.atlasPixels.resize(pixelCount);
	memcpy(font.atlasPixels.data(), data.data() + kCftBitmapHeaderSize, pixelCount);

	_fonts.push_back(font);
	return true;
}

void StartupText::decodeXorText(Common::Array<byte> &data) const {
	for (byte &value : data) {
		if (value == '\n' || value == '\r')
			continue;
		value ^= 0xaa;
	}
}

bool StartupText::resolveDialogueSubtitle(int wavId, Common::String &text) const {
	text.clear();
	if (wavId <= 0 || _dialogueEntries.empty() || _dialogueData.empty())
		return false;

	int left = 0;
	int right = (int)_dialogueEntries.size() - 1;
	while (left <= right) {
		const int middle = left + (right - left) / 2;
		const DialogueIndexEntry &entry = _dialogueEntries[(uint)middle];
		if (entry.wavId == wavId) {
			if (entry.textOffset + entry.textLength > _dialogueData.size())
				return false;

			text = Common::String((const char *)_dialogueData.data() + entry.textOffset, entry.textLength);
			return !text.empty();
		}
		if (entry.wavId < wavId)
			left = middle + 1;
		else
			right = middle - 1;
	}

	return false;
}

Common::String StartupText::getDialogueResponseLine(int zeroBasedIndex) const {
	if (zeroBasedIndex < 0 || (uint)zeroBasedIndex >= _dialogueResponseLines.size())
		return Common::String();

	return _dialogueResponseLines[(uint)zeroBasedIndex];
}

} // End of namespace Harvester
