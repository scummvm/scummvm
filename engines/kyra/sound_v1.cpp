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
 * $URL$
 * $Id$
 *
 */

#include "kyra/sound.h"
#include "kyra/kyra_v1.h"

namespace Kyra {

void KyraEngine_v1::snd_playSoundEffect(int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playSoundEffect(%d)", track);
	if ((_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) && track == 49) {
		snd_playWanderScoreViaMap(56, 1);
		return;
	}

	KyraEngine::snd_playSoundEffect(track);
}

void KyraEngine_v1::snd_playWanderScoreViaMap(int command, int restart) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playWanderScoreViaMap(%d, %d)", command, restart);
	if (restart)
		_lastMusicCommand = -1;

	if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
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
		KyraEngine::snd_playWanderScoreViaMap(command, restart);
	}
}

void KyraEngine_v1::snd_playVoiceFile(int id) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v1::snd_playVoiceFile(%d)", id);
	char vocFile[9];
	assert(id >= 0 && id < 9999);
	sprintf(vocFile, "%03d", id);
	_speechFile = vocFile;
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

} // end of namespace Kyra
