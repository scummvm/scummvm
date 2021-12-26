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

#ifndef COMMON_STRING_BIDI_H
#define COMMON_STRING_BIDI_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/language.h"

namespace Common {

/**
 * List of paragraph directions
 */
enum BiDiParagraph {
	BIDI_PAR_ON = 0,
	BIDI_PAR_RTL = 1,
	BIDI_PAR_LTR = 2
};

class UnicodeBiDiText {
private:
	uint32 *_log_to_vis_index; // from fribidi conversion
	uint32 *_vis_to_log_index; // from fribidi conversion
	void initWithU32String(const Common::U32String &str);
	Common::String bidiByLine(Common::String line, va_list args);
public:
	const Common::U32String logical; // original string, ordered logically
	Common::U32String visual; // from fribidi conversion, ordered visually
	uint32 _pbase_dir;

	UnicodeBiDiText(const Common::U32String &str, BiDiParagraph dir = BIDI_PAR_ON);
	/* This constructor shouldn't be used outside of unicode-bidi.cpp file */
	UnicodeBiDiText(const Common::String &str, const Common::CodePage page, uint32 *pbase_dir);
	~UnicodeBiDiText();

	/**
	 * Implicit conversion to U32String to get the visual representation.
	 */
	operator const U32String &() const { return visual; }

	uint32 getVisualPosition(uint32 logicalPos) const;
	uint32 getLogicalPosition(uint32 visualPos) const;
	uint32 size() const { return logical.size(); }
};

/* just call the constructor for convenience */
UnicodeBiDiText convertBiDiU32String(const U32String &input, BiDiParagraph dir = BIDI_PAR_ON);
String convertBiDiString(const String &input, const Common::Language lang, BiDiParagraph dir = BIDI_PAR_ON);
String convertBiDiString(const String &input, const Common::CodePage page, BiDiParagraph dir = BIDI_PAR_ON);

// calls convertBiDiString for each line in isolation
String convertBiDiStringByLines(const String &input, const Common::CodePage page, BiDiParagraph dir = BIDI_PAR_ON);

} // End of namespace Common

#endif
