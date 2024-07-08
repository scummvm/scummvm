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

#include "common/language.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/translation.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/game/tra_file.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void close_translation() {
	_GP(transtree).clear();
	_GP(trans) = Translation();
	_G(trans_name) = "";
	_G(trans_filename) = "";

	// Return back to default game's encoding
	if (_GP(game).options[OPT_GAMETEXTENCODING] == 65001) // utf-8 codepage number
		set_uformat(U_UTF8);
	else
		set_uformat(U_ASCII);
}

bool init_translation(const String &lang, const String &fallback_lang) {
	if (lang.IsEmpty())
		return false;
	_G(trans_name) = lang;
	_G(trans_filename) = String::FromFormat("%s.tra", lang.GetCStr());

	std::unique_ptr<Stream> in(_GP(AssetMgr)->OpenAsset(_G(trans_filename)));
	if (in == nullptr) {
		Debug::Printf(kDbgMsg_Error, "Cannot open translation: %s", _G(trans_filename).GetCStr());
		return false;
	}

	_GP(trans) = Translation();

	// First test if the translation is meant for this game
	HError err = TestTraGameID(_GP(game).uniqueid, _GP(game).gamename, in.get());
	if (err) {
		// If successful, then read translation data fully
		in.reset(_GP(AssetMgr)->OpenAsset(_G(trans_filename)));
		err = ReadTraData(_GP(trans), in.get());
	}

	// Process errors
	if (!err) {
		close_translation();
		Debug::Printf(kDbgMsg_Error, "Failed to read translation file: %s:\n%s",
			_G(trans_filename).GetCStr(),
			err->FullMessage().GetCStr());
		if (!fallback_lang.IsEmpty()) {
			Debug::Printf("Fallback to translation: %s", fallback_lang.GetCStr());
			init_translation(fallback_lang, "");
		}
		return false;
	}

	// Translation read successfully
	Debug::Printf("Translation loaded: %s", _G(trans_filename).GetCStr());
	// Configure new game settings
	if (_GP(trans).NormalFont >= 0)
		SetNormalFont(_GP(trans).NormalFont);
	if (_GP(trans).SpeechFont >= 0)
		SetSpeechFont(_GP(trans).SpeechFont);
	if (_GP(trans).RightToLeft == 1) {
		_GP(play).text_align = kHAlignLeft;
		_GP(game).options[OPT_RIGHTLEFTWRITE] = 0;
	} else if (_GP(trans).RightToLeft == 2) {
		_GP(play).text_align = kHAlignRight;
		_GP(game).options[OPT_RIGHTLEFTWRITE] = 1;
	}

	// Setup a text encoding mode depending on the translation data hint
	String encoding = _GP(trans).StrOptions["encoding"];
	if (encoding.CompareNoCase("utf-8") == 0)
		set_uformat(U_UTF8);
	else
		set_uformat(U_ASCII);
	String encoding_msg = !encoding.IsEmpty() ? encoding : "presume ASCII";
	Debug::Printf("Translation's encoding: %s", encoding_msg.GetCStr());

	// Mixed encoding support: 
	// original text unfortunately may contain extended ASCII chars (> 127);
	// if translation is UTF-8 but game is extended ASCII, then the translation
	// dictionary keys won't match. With that assumption we must convert
	// dictionary keys into ASCII using provided locale hint.
	int game_codepage = _GP(game).options[OPT_GAMETEXTENCODING];
	if ((get_uformat() == U_UTF8) && (game_codepage != 65001)) {
		String key_enc = (game_codepage > 0) ?
			String::FromFormat(".%d", game_codepage) :
			_GP(trans).StrOptions["gameencoding"];
		Debug::Printf("Game's source encoding hint: own: %d, from TRA: %s", game_codepage, _GP(trans).StrOptions["gameencoding"].GetCStr());
		if (!key_enc.IsEmpty()) {
			StringMap conv_map;
			std::vector<char> ascii; // ascii buffer
			Debug::Printf("Converting UTF-8 TRA keys to the game's encoding (%s)", key_enc.GetCStr());
			for (const auto &item : _GP(trans).Dict) {
				ascii.resize(item._key.GetLength() + 1); // ascii len will be <= utf-8 len
				StrUtil::ConvertUtf8ToAscii(item._key.GetCStr(), key_enc.GetCStr(), &ascii[0], ascii.size());
				conv_map.insert(std::make_pair(String(&ascii[0]), item._value));
			}
			_GP(trans).Dict = conv_map;
		}
		else {
			Debug::Printf(kDbgMsg_Warn, "WARNING: UTF-8 translation in the ASCII/ANSI game, but no encoding hint for TRA keys conversion");
		}
	}

	Debug::Printf(kDbgMsg_Info, "Translation initialized: %s (format: %s)", _G(trans_name).GetCStr(), encoding_msg.GetCStr());
	return true;
}

String get_translation_name() {
	return _G(trans_name);
}

String get_translation_path() {
	return _G(trans_filename);
}

const StringMap &get_translation_tree() {
	return _GP(trans).Dict;
}

} // namespace AGS3
