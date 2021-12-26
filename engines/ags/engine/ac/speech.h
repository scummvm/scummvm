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

int user_to_internal_skip_speech(SkipSpeechStyle userval);
SkipSpeechStyle internal_skip_speech_to_user(int internal_val);

} // namespace AGS3

#endif
