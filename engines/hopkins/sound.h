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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_SOUND_H
#define HOPKINS_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

class SoundManager {
public:
	HopkinsEngine *_vm;

	int SPECIAL_SOUND;
	int SOUNDVOL;
	int VOICEVOL;
	int MUSICVOL;
	int OLD_SOUNDVOL;
	int OLD_MUSICVOL;
	int OLD_VOICEVOL;
	bool SOUNDOFF;
	bool MUSICOFF;
	bool VOICEOFF;
	bool TEXTOFF;
public:
	void setParent(HopkinsEngine *vm);

	void WSOUND_INIT();
	void VERIF_SOUND();
	void LOAD_ANM_SOUND();
	void PLAY_ANM_SOUND(int soundNumber);
	void WSOUND(int soundNumber);
	void VOICE_MIX(int a1, int a2);
	void DEL_SAMPLE(int soundNumber);
	void PLAY_SOUND(const Common::String &file);
	void MODSetSampleVolume();
	void MODSetVoiceVolume();
	void MODSetMusicVolume(int volume);
};

} // End of namespace Hopkins

#endif /* HOPKINS_SOUND_H */
