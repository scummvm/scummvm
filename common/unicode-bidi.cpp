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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/ustr.h"
#include "common/unicode-bidi.h"
#include "common/textconsole.h"

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>
#endif

namespace Common {

uint32 GetFriBiDiParType(BiDiParagraph dir) {
#ifdef USE_FRIBIDI
	if (dir == BIDI_PAR_ON) {
		return FRIBIDI_PAR_ON;
	} else if (dir == BIDI_PAR_RTL) {
		return FRIBIDI_PAR_RTL;
	} else if (dir == BIDI_PAR_LTR) {
		return FRIBIDI_PAR_LTR;
	}
#endif
	return 0;
}

UnicodeBiDiText::UnicodeBiDiText(const Common::U32String &str, BiDiParagraph dir) :
	logical(str), _pbase_dir(GetFriBiDiParType(dir)),
	_log_to_vis_index(nullptr), _vis_to_log_index(nullptr) {
	initWithU32String(str);
}

UnicodeBiDiText::UnicodeBiDiText(const Common::String &str, const Common::CodePage page,
		uint32 *pbase_dir) : logical(str), _log_to_vis_index(nullptr), _vis_to_log_index(nullptr) {
	_pbase_dir = *pbase_dir;
	initWithU32String(str.decode(page));
	*pbase_dir = _pbase_dir;
}

UnicodeBiDiText::~UnicodeBiDiText() {
	delete[] _log_to_vis_index;
	delete[] _vis_to_log_index;
}

uint32 UnicodeBiDiText::getVisualPosition(uint32 logicalPos) const {
	if (nullptr != _log_to_vis_index && logicalPos < size()) {
		return _log_to_vis_index[logicalPos];
	}
	return logicalPos;
}
uint32 UnicodeBiDiText::getLogicalPosition(uint32 visualPos) const {
	if (nullptr != _log_to_vis_index && visualPos < size()) {
		return _vis_to_log_index[visualPos];
	}
	return visualPos;
}

void UnicodeBiDiText::initWithU32String(const U32String &input) {

#ifdef USE_FRIBIDI
	uint32 input_size = input.size();
	uint32 buff_length = (input_size + 2) * 2;		// it's more than enough, but it's better to be on the safe side
	FriBidiChar *visual_str = new FriBidiChar[buff_length * sizeof(FriBidiChar)];
	_log_to_vis_index = new uint32[input_size];
	_vis_to_log_index = new uint32[input_size];

	FriBidiParType pbase_dir = FriBidiParType(_pbase_dir);
	if (!fribidi_log2vis(
		/* input */
		(const FriBidiChar *)input.c_str(),
		input_size,
		&pbase_dir,
		/* output */
		visual_str,
		(FriBidiStrIndex *)_log_to_vis_index,	// position_L_to_V_list,
		(FriBidiStrIndex *)_vis_to_log_index,	// position_V_to_L_list,
		nullptr									// embedding_level_list
	)) {
		warning("initWithU32String: calling fribidi_log2vis failed");
		delete[] visual_str;
		delete[] _log_to_vis_index;
		delete[] _vis_to_log_index;
		visual = input;
		_log_to_vis_index = nullptr;
		_vis_to_log_index = nullptr;
	} else {
		visual = U32String((Common::u32char_type_t *)visual_str, input.size());
		delete[] visual_str;
	}
	_pbase_dir = pbase_dir;
#else
	static bool fribidiWarning = true;
	if (fribidiWarning) {
		warning("initWithU32String: Fribidi not available, will use input strings as fallback.");
		fribidiWarning = false;
	}
	visual = input;
#endif

}

Common::String bidiByLineHelper(Common::String line, va_list args) {
	/* Common::CodePage is int
	 * GCC warns that using Common::CodePage in va_arg would abort program */
	Common::CodePage page = (Common::CodePage) va_arg(args, int);
	uint32 *pbase_dir = va_arg(args, uint32*);
	return UnicodeBiDiText(line, page, pbase_dir).visual.encode(page);
}

String convertBiDiStringByLines(const String &input, const Common::CodePage page, BiDiParagraph dir) {
	uint32 pbase_dir = GetFriBiDiParType(dir);
	return input.forEachLine(bidiByLineHelper, page, &pbase_dir);
}

String convertBiDiString(const String &input, const Common::Language lang, BiDiParagraph dir) {
	if (lang == Common::HE_ISR) {
		return Common::convertBiDiString(input, kWindows1255, dir);
	} else if (lang == Common::FA_IRN) {
		return Common::convertBiDiString(input, kWindows1256, dir);
	} else {
		return input;
	}
}

String convertBiDiString(const String &input, const Common::CodePage page, BiDiParagraph dir) {
	return convertBiDiU32String(input.decode(page), dir).visual.encode(page);
}

UnicodeBiDiText convertBiDiU32String(const U32String &input, BiDiParagraph dir) {
	return UnicodeBiDiText(input, dir);
}

} // End of namespace Common
