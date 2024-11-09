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

#include "twine/parser/text.h"
#include "common/debug.h"
#include "common/str-enc.h"
#include "common/util.h"
#include "common/translation.h"
#include "twine/resources/hqr.h"
#include "twine/shared.h"

namespace TwinE {

void TextData::initCustomTexts(TextBankId textBankId) {
	if (textBankId == TextBankId::Options_and_menus) {
		// TODO: add resource file for these custom strings to support other languages
		add(textBankId, TextEntry{Common::U32String("High resolution on").encode(Common::CodePage::kDos850), -1, TextId::kCustomHighResOptionOn});
		add(textBankId, TextEntry{Common::U32String("High resolution off").encode(Common::CodePage::kDos850), -1, TextId::kCustomHighResOptionOff});
		add(textBankId, TextEntry{Common::U32String("Wall collision on").encode(Common::CodePage::kDos850), -1, TextId::kCustomWallCollisionOn});
		add(textBankId, TextEntry{Common::U32String("Wall collision off").encode(Common::CodePage::kDos850), -1, TextId::kCustomWallCollisionOff});
		add(textBankId, TextEntry{Common::U32String("Language selection").encode(Common::CodePage::kDos850), -1, TextId::kCustomLanguageOption});
		add(textBankId, TextEntry{Common::U32String("Voices: None").encode(Common::CodePage::kDos850), -1, TextId::kCustomVoicesNone});
		add(textBankId, TextEntry{Common::U32String("Voices: English").encode(Common::CodePage::kDos850), -1, TextId::kCustomVoicesEnglish});
		add(textBankId, TextEntry{Common::U32String("Voices: French").encode(Common::CodePage::kDos850), -1, TextId::kCustomVoicesFrench});
		add(textBankId, TextEntry{Common::U32String("Voices: German").encode(Common::CodePage::kDos850), -1, TextId::kCustomVoicesGerman});
	}
}

bool TextData::loadFromHQR(const char *name, TextBankId textBankId, int language, bool lba1, int entryCount) {
	const int langIdx = (int)textBankId * 2 + (entryCount * language);
	Common::SeekableReadStream *indexStream = HQR::makeReadStream(name, langIdx + 0);
	Common::SeekableReadStream *offsetStream = HQR::makeReadStream(name, langIdx + 1);
	if (indexStream == nullptr || offsetStream == nullptr) {
		warning("Failed to load %s with index %i", name, langIdx);
		delete indexStream;
		delete offsetStream;
		return false;
	}

	_texts[(int)textBankId].clear();
	initCustomTexts(textBankId);

	const int numIdxEntries = (int)indexStream->size() / 2;
	_texts[(int)textBankId].reserve(numIdxEntries + _texts[(int)textBankId].size());

	for (int entry = 0; entry < numIdxEntries; ++entry) {
		const TextId textIdx = (TextId)indexStream->readUint16LE();
		uint16 start = offsetStream->readUint16LE();
		const int32 offsetPos = offsetStream->pos();
		const uint16 end = offsetStream->readUint16LE();

		if (!lba1) {
			++start;
		}
		offsetStream->seek(start);
		Common::String result;
		for (int16 i = start; i < end - 1; ++i) {
			const char c = (char)offsetStream->readByte();
			if (c == '\0') {
				break;
			}
			result += c;
		}
		add(textBankId, TextEntry{result, entry, textIdx});
		debugC(2, TwinE::kDebugResources, "index: %i (bank %i), text: %s", (int)textIdx, (int)textBankId, result.c_str());
		offsetStream->seek(offsetPos);
		if (end >= offsetStream->size()) {
			break;
		}
	}
	delete indexStream;
	delete offsetStream;

	return true;
}

const TextEntry *TextData::getText(TextBankId textBankId, TextId textIndex) const {
	const Common::Array<TextEntry> &entries = _texts[(int)textBankId];
	const int32 size = entries.size();
	for (int32 i = 0; i < size; ++i) {
		if (entries[i].textIndex == textIndex) {
			return &entries[i];
		}
	}
	debugC(1, TwinE::kDebugResources, "Failed to find text entry for bank id %i with text index %i", (int)textBankId, (int)textIndex);
	return nullptr;
}

} // End of namespace TwinE
