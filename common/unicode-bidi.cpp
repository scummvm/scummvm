/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "common/ustr.h"
#include "common/unicode-bidi.h"
#include "common/textconsole.h"

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>
#endif

namespace Common {

UnicodeBiDiText::UnicodeBiDiText(const Common::U32String &str) : logical(str), _log_to_vis_index(NULL), _vis_to_log_index(NULL) {
	initWithU32String(str);
}

UnicodeBiDiText::UnicodeBiDiText(const Common::String &str, const Common::CodePage page) : logical(str), _log_to_vis_index(NULL), _vis_to_log_index(NULL) {
	initWithU32String(str.decode(page));
}

UnicodeBiDiText::~UnicodeBiDiText() {
	delete[] _log_to_vis_index;
	delete[] _vis_to_log_index;
}

uint32 UnicodeBiDiText::getVisualPosition(uint32 logicalPos) const { 
	if (NULL != _log_to_vis_index && logicalPos < size()) {
		return _log_to_vis_index[logicalPos];
	}
	return logicalPos;
}
uint32 UnicodeBiDiText::getLogicalPosition(uint32 visualPos) const {
	if (NULL != _log_to_vis_index && visualPos < size()) {
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
	FriBidiParType pbase_dir = FRIBIDI_PAR_ON;

	if (!fribidi_log2vis(
		/* input */
		(const FriBidiChar *)input.c_str(),
		input_size,
		&pbase_dir,
		/* output */
		visual_str,
		(FriBidiStrIndex *)_log_to_vis_index,	// position_L_to_V_list,
		(FriBidiStrIndex *)_vis_to_log_index,	// position_V_to_L_list,
		NULL									// embedding_level_list
	)) {
		warning("initWithU32String: calling fribidi_log2vis failed");
		delete[] visual_str;
		delete[] _log_to_vis_index;
		delete[] _vis_to_log_index;
		visual = input;
		_log_to_vis_index = NULL;
		_vis_to_log_index = NULL;
	} else {
		visual = U32String((uint32 *)visual_str, input.size());
		delete[] visual_str;
	}
#else
	static bool fribidiWarning = true;
	if (fribidiWarning) {
		warning("initWithU32String: Fribidi not available, will use input strings as fallback.");
		fribidiWarning = false;
	}
	visual = input;
#endif

}

String convertBiDiString(const String &input, const Common::Language lang) {
	if (lang != Common::HE_ISR)		//TODO: modify when we'll support other RTL languages, such as Arabic and Farsi
		return input;

	return Common::convertBiDiString(input, kWindows1255);
}

String convertBiDiString(const String &input, const Common::CodePage page) {
	return convertBiDiU32String(input.decode(page)).visual.encode(page);
}

UnicodeBiDiText convertBiDiU32String(const U32String &input) {
	return UnicodeBiDiText(input);
}

} // End of namespace Common
