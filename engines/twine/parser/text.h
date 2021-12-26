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

#ifndef TWINE_PARSER_TEXT_H
#define TWINE_PARSER_TEXT_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

class TextEntry {
public:
	Common::String string;	/**< The real string behind the text id */
	int index;				/**< The index in the text index hqr file. This is also the index in the corresponding vox hqr file */
	TextId textIndex;			/**< The text identifier */
};

class TextData {
private:
	// 30 is the max for lba2, lba1 uses 28
	Common::Array<TextEntry> _texts[30];
	void add(TextBankId textBankId, const TextEntry &entry) {
		_texts[(int)textBankId].push_back(entry);
	}

	// custom texts that are not included in the original game
	void initCustomTexts(TextBankId textBankId);
public:
	bool loadFromHQR(const char *name, TextBankId textBankId, int language, bool lba1, int entryCount);

	const TextEntry *getText(TextBankId textBankId, TextId textIndex) const;
};

} // End of namespace TwinE

#endif
