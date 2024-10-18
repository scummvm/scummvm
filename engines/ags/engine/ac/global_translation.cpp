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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/translation.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin_evts.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/util/memory.h"
#include "ags/engine/ac/string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared::Memory;

const char *get_translation(const char *text) {
	if (text == nullptr)
		quit("!Null string supplied to CheckForTranslations");

	_G(source_text_length) = GetTextDisplayLength(text);

#if AGS_PLATFORM_64BIT
	// check if a plugin wants to translate it - if so, return that
	// TODO: plugin API is currently strictly 32-bit, so this may break on 64-bit systems
	char *plResult = Int32ToPtr<char>(pl_run_plugin_hooks(AGSE_TRANSLATETEXT, PtrToInt32(text)));
	if (plResult) {
		return plResult;
	}
#endif

	const auto &transtree = get_translation_tree();
	const auto it = transtree.find(String::Wrapper(text));
	if (it != transtree.end())
		return it->_value.GetCStr();

	// return the original text
	return text;
}

int IsTranslationAvailable() {
	if (get_translation_tree().size() > 0)
		return 1;
	return 0;
}

int GetTranslationName(char *buffer) {
	VALIDATE_STRING(buffer);
	snprintf(buffer, MAX_MAXSTRLEN, "%s", get_translation_name().GetCStr());
	return IsTranslationAvailable();
}

} // namespace AGS3
