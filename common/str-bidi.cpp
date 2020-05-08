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

#include "common/str-bidi.h"
#include "common/textconsole.h"

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>
#endif

namespace Common {

String convertBiDiString(const String &input, const Common::CodePage page) {
	return convertBiDiU32String(input.decode(page)).encode(page);
}

U32String convertBiDiU32String(const U32String &input) {

#ifdef USE_FRIBIDI
	int buff_length = (input.size() + 2) * 2;		// it's more than enough, but it's better to be on the safe side
	FriBidiChar *visual_str = (FriBidiChar *)malloc(buff_length * sizeof(FriBidiChar));

	FriBidiCharType pbase_dir = FRIBIDI_TYPE_ON;

	if (!fribidi_log2vis(
		/* input */
		(const FriBidiChar *)input.c_str(),
		input.size(),
		&pbase_dir,
		/* output */
		visual_str,
		NULL,			// position_L_to_V_list,
		NULL,			// position_V_to_L_list,
		NULL			// embedding_level_list
	)) {
		warning("convertBiDiU32String: calling fribidi_log2vis failed");
		free(visual_str);
		return input;
	}

	U32String result = U32String(visual_str, input.size());
	free(visual_str);

	return result;
#else
	warning("convertBiDiU32String: Fribidi not available, using input string as fallback");
	return input;
#endif

}

int getVisualPosition(const U32String &str, int pos) {
	Common::U32String suffix(str.c_str() + pos + 1);
	Common::U32String visual_str = Common::convertBiDiU32String(str);
	Common::U32String visual_prefix(visual_str.c_str(), suffix.size());
	Common::U32String visual_suffix = Common::convertBiDiU32String(suffix);
	return visual_prefix == visual_suffix ? suffix.size() : pos;
}

int getVisualPosition(const String &str, const Common::CodePage page, int logicalPos) {
	return getVisualPosition(str.decode(page), logicalPos);
}

} // End of namespace Common
