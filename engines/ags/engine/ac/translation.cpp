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

//include <cstdio>
#include "ags/shared/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/ac/global_game.h"
#include "ags/shared/ac/runtime_defines.h"
#include "ags/shared/ac/translation.h"
#include "ags/shared/ac/tree_map.h"
#include "ags/shared/ac/wordsdictionary.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/misc.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/core/assetmanager.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetup usetup;
extern GameSetupStruct game;
extern GameState play;
extern char transFileName[MAX_PATH];


TreeMap *transtree = nullptr;
long lang_offs_start = 0;
char transFileName[MAX_PATH] = "\0";

void close_translation() {
	if (transtree != nullptr) {
		delete transtree;
		transtree = nullptr;
	}
}

bool parse_translation(Stream *language_file, String &parse_error);

bool init_translation(const String &lang, const String &fallback_lang, bool quit_on_error) {

	if (lang.IsEmpty())
		return false;
	sprintf(transFileName, "%s.tra", lang.GetCStr());

	Stream *language_file = find_open_asset(transFileName);
	if (language_file == nullptr) {
		Debug::Printf(kDbgMsg_Error, "Cannot open translation: %s", transFileName);
		return false;
	}
	// in case it's inside a library file, record the offset
	lang_offs_start = language_file->GetPosition();

	char transsig[16] = { 0 };
	language_file->Read(transsig, 15);
	if (strcmp(transsig, "AGSTranslation") != 0) {
		Debug::Printf(kDbgMsg_Error, "Translation signature mismatch: %s", transFileName);
		delete language_file;
		return false;
	}

	if (transtree != nullptr) {
		close_translation();
	}
	transtree = new TreeMap();

	String parse_error;
	bool result = parse_translation(language_file, parse_error);
	delete language_file;

	if (!result) {
		close_translation();
		parse_error.Prepend(String::FromFormat("Failed to read translation file: %s:\n", transFileName));
		if (quit_on_error) {
			parse_error.PrependChar('!');
			quit(parse_error);
		} else {
			Debug::Printf(kDbgMsg_Error, parse_error);
			if (!fallback_lang.IsEmpty()) {
				Debug::Printf("Fallback to translation: %s", fallback_lang.GetCStr());
				init_translation(fallback_lang, "", false);
			}
			return false;
		}
	}
	Debug::Printf("Translation initialized: %s", transFileName);
	return true;
}

bool parse_translation(Stream *language_file, String &parse_error) {
	while (!language_file->EOS()) {
		int blockType = language_file->ReadInt32();
		if (blockType == -1)
			break;
		// MACPORT FIX 9/6/5: remove warning
		/* int blockSize = */ language_file->ReadInt32();

		if (blockType == 1) {
			char original[STD_BUFFER_SIZE], translation[STD_BUFFER_SIZE];
			while (1) {
				read_string_decrypt(language_file, original, STD_BUFFER_SIZE);
				read_string_decrypt(language_file, translation, STD_BUFFER_SIZE);
				if ((strlen(original) < 1) && (strlen(translation) < 1))
					break;
				if (language_file->EOS()) {
					parse_error = "Translation file is corrupt";
					return false;
				}
				transtree->addText(original, translation);
			}

		} else if (blockType == 2) {
			int uidfrom;
			char wasgamename[100];
			uidfrom = language_file->ReadInt32();
			read_string_decrypt(language_file, wasgamename, sizeof(wasgamename));
			if ((uidfrom != game.uniqueid) || (strcmp(wasgamename, game.gamename) != 0)) {
				parse_error.Format("The translation file is not compatible with this game. The translation is designed for '%s'.",
					wasgamename);
				return false;
			}
		} else if (blockType == 3) {
			// game settings
			int temp = language_file->ReadInt32();
			// normal font
			if (temp >= 0)
				SetNormalFont(temp);
			temp = language_file->ReadInt32();
			// speech font
			if (temp >= 0)
				SetSpeechFont(temp);
			temp = language_file->ReadInt32();
			// text direction
			if (temp == 1) {
				play.text_align = kHAlignLeft;
				game.options[OPT_RIGHTLEFTWRITE] = 0;
			} else if (temp == 2) {
				play.text_align = kHAlignRight;
				game.options[OPT_RIGHTLEFTWRITE] = 1;
			}
		} else {
			parse_error.Format("Unknown block type in translation file (%d).", blockType);
			return false;
		}
	}

	if (transtree->text == nullptr) {
		parse_error = "The translation file was empty.";
		return false;
	}

	return true;
}

} // namespace AGS3
