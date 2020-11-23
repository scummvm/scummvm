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

//include <string.h>
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/display.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/ac/global_translation.h"
#include "ags/shared/ac/string.h"
#include "ags/shared/ac/tree_map.h"
#include "ags/shared/platform/base/agsplatformdriver.h"
#include "ags/shared/plugin/agsplugin.h"
#include "ags/shared/plugin/plugin_engine.h"
#include "ags/shared/util/memory.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

using namespace AGS::Shared::Memory;

extern GameState play;
extern AGSPlatformDriver *platform;
extern TreeMap *transtree;
extern char transFileName[MAX_PATH];

const char *get_translation(const char *text) {
	if (text == nullptr)
		quit("!Null string supplied to CheckForTranslations");

	source_text_length = GetTextDisplayLength(text);

#if AGS_PLATFORM_64BIT
	// check if a plugin wants to translate it - if so, return that
	// TODO: plugin API is currently strictly 32-bit, so this may break on 64-bit systems
	char *plResult = Int32ToPtr<char>(pl_run_plugin_hooks(AGSE_TRANSLATETEXT, PtrToInt32(text)));
	if (plResult) {
		return plResult;
	}
#endif

	if (transtree != nullptr) {
		// translate the text using the translation file
		char *transl = transtree->findValue(text);
		if (transl != nullptr)
			return transl;
	}
	// return the original text
	return text;
}

int IsTranslationAvailable() {
	if (transtree != nullptr)
		return 1;
	return 0;
}

int GetTranslationName(char *buffer) {
	VALIDATE_STRING(buffer);
	const char *copyFrom = transFileName;

	while (strchr(copyFrom, '\\') != nullptr) {
		copyFrom = strchr(copyFrom, '\\') + 1;
	}
	while (strchr(copyFrom, '/') != nullptr) {
		copyFrom = strchr(copyFrom, '/') + 1;
	}

	strcpy(buffer, copyFrom);
	// remove the ".tra" from the end of the filename
	if (strstr(buffer, ".tra") != nullptr)
		strstr(buffer, ".tra")[0] = 0;

	return IsTranslationAvailable();
}

} // namespace AGS3
