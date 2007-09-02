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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
 
#include "kyra/sound.h"
#include "kyra/kyra_v1.h"

namespace Kyra {

bool KyraEngine_v1::speechEnabled() {
	return _flags.isTalkie && (_configVoice == 1 || _configVoice == 2);
}

bool KyraEngine_v1::textEnabled() {
	return !_flags.isTalkie || (_configVoice == 0 || _configVoice == 2);
}

void KyraEngine_v1::snd_playTheme(int file, int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playTheme(%d)", file);
	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(_curMusicTheme);
	_sound->playTrack(track);
}

void KyraEngine_v1::snd_playSoundEffect(int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playSoundEffect(%d)", track);
	if (_flags.platform == Common::kPlatformFMTowns && track == 49) {
		snd_playWanderScoreViaMap(56, 1);
		return;
	}
	_sound->playSoundEffect(track);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playWanderScoreViaMap(%d, %d)", command, restart);
	if (restart)
		_lastMusicCommand = -1;

	if (_flags.platform == Common::kPlatformFMTowns) {
		if (command == 1) {
			_sound->beginFadeOut();
		} else if (command >= 35 && command <= 38) {
			snd_playSoundEffect(command-20);
		} else if (command >= 2) {
			if (_lastMusicCommand != command) {
				// the original does -2 here we handle this inside _sound->playTrack()
				_sound->playTrack(command);
			}
		} else {
			_sound->haltTrack();
		}
	} else {
		static const int8 soundTable[] = {
			-1,   0,  -1,   1,   0,   3,   0,   2,
			 0,   4,   1,   2,   1,   3,   1,   4,
			 1,  92,   1,   6,   1,   7,   2,   2,
			 2,   3,   2,   4,   2,   5,   2,   6,
			 2,   7,   3,   3,   3,   4,   1,   8,
			 1,   9,   4,   2,   4,   3,   4,   4,
			 4,   5,   4,   6,   4,   7,   4,   8,
			 1,  11,   1,  12,   1,  14,   1,  13,
			 4,   9,   5,  12,   6,   2,   6,   6,
			 6,   7,   6,   8,   6,   9,   6,   3,
			 6,   4,   6,   5,   7,   2,   7,   3,
			 7,   4,   7,   5,   7,   6,   7,   7,
			 7,   8,   7,   9,   8,   2,   8,   3,
			 8,   4,   8,   5,   6,  11,   5,  11
		};
		//if (!_disableSound) {
		//	XXX
		//}
		assert(command*2+1 < ARRAYSIZE(soundTable));
		if (_curMusicTheme != soundTable[command*2]+1) {
			if (soundTable[command*2] != -1)
				snd_playTheme(soundTable[command*2]+1);
		}
	
		if (command != 1) {
			if (_lastMusicCommand != command) {
				_sound->haltTrack();
				_sound->playTrack(soundTable[command*2+1]);
			}
		} else {
			_sound->beginFadeOut();
		}
	}

	_lastMusicCommand = command;
}

void KyraEngine_v1::snd_playVoiceFile(int id) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playVoiceFile(%d)", id);
	char vocFile[9];
	assert(id >= 0 && id < 9999);
	sprintf(vocFile, "%03d", id);
	_sound->voicePlay(vocFile);
}

void KyraEngine_v1::snd_voiceWaitForFinish(bool ingame) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_voiceWaitForFinish(%d)", ingame);
	while (_sound->voiceIsPlaying() && !_skipFlag) {
		if (ingame)
			delay(10, true);
		else
			_system->delayMillis(10);
	}
}

void KyraEngine_v1::snd_stopVoice() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_stopVoice()");
	_sound->voiceStop();
}

bool KyraEngine_v1::snd_voiceIsPlaying() {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_voiceIsPlaying()");
	return _sound->voiceIsPlaying();
}

} // end of namespace Kyra
