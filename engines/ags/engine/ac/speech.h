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

#ifndef AGS_ENGINE_AC_SPEECH_H
#define AGS_ENGINE_AC_SPEECH_H

#include "ags/shared/util/string.h"

namespace AGS3 {

enum SkipSpeechStyle {
	kSkipSpeechNone = -1,
	kSkipSpeechKeyMouseTime = 0,
	kSkipSpeechKeyTime = 1,
	kSkipSpeechTime = 2,
	kSkipSpeechKeyMouse = 3,
	kSkipSpeechMouseTime = 4,
	kSkipSpeechKey = 5,
	kSkipSpeechMouse = 6,

	kSkipSpeechFirst = kSkipSpeechNone,
	kSkipSpeechLast = kSkipSpeechMouse
};

enum SpeechMode {
	kSpeech_TextOnly = 0,
	kSpeech_VoiceText = 1,
	kSpeech_VoiceOnly = 2,

	kSpeech_First = kSpeech_TextOnly,
	kSpeech_Last = kSpeech_VoiceOnly
};

int user_to_internal_skip_speech(SkipSpeechStyle userval);
SkipSpeechStyle internal_skip_speech_to_user(int internal_val);
// Locates and initializes a voice pack of the given *name*, tells if successful;
// pass empty string for default voice pack.
bool init_voicepak(const AGS::Shared::String &name = "");
// Gets voice pack's ID name, that is a filename without "sp_" prefix and no extension.
AGS::Shared::String get_voicepak_name();
// Gets an asset's parent path for voice-over clips and data files
AGS::Shared::String get_voice_assetpath();

} // namespace AGS3

#endif
