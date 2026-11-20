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

#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {

void RunawayRoomScene::resumeFinaleOutdoorAudio(bool entering, uint16 windCue, byte windVolume) {
	_finaleAmbientTick = _now;
	if (!_audio.effectPlaying(2)) {
		_audio.playEffect(windCue, windVolume, 2, true);
	}
	if (entering || !_audio.musicPlaying()) {
		_musicCue = 12 + _random.getRandomNumber(4);
		_audio.playMusic(_musicCue, false);
	}
}

void RunawayRoomScene::updateFinaleOutdoorAudio(byte wildlifeVolume) {
	updateOutdoorMusic(5);
	if (wildlifeVolume && _now - _finaleAmbientTick >= 250) {
		_finaleAmbientTick = _now;
		if (!_audio.effectPlaying(0) && !_random.getRandomNumber(49)) {
			byte cue;
			do {
				cue = 74 + _random.getRandomNumber(4);
			} while (cue == _finaleLastAmbient);
			_finaleLastAmbient = cue;
			_audio.playEffect(cue, wildlifeVolume);
		}
	}
}

bool RunawayRoomScene::sayFinaleInventory(const RunawayInventory::Reply &reply) {
	typedef RunawaySequence S;
	static const S::Command map[] = {
		{S::kImage, 146}, {S::kSpeech, 301, 0, 0, 1}, {S::kWaitInput}, {S::kEnd}
	};
	if (reply.cue == 301 || reply.cue == 303) {
		if (reply.cue == 301) {
			_sequence.start(map, _text, _now, 'H');
		} else {
			_filmCommands.clear();
			_filmCommands.push_back({S::kImage, 147});
			const byte variation = _finale.moneyFound >= 3;
			_filmCommands.push_back({S::kSpeech, 303, variation, variation, 1});
			_filmCommands.push_back(S::Command(S::kWaitInput));
			_filmCommands.push_back(S::Command(S::kEnd));
			_sequence.start(_filmCommands.data(), _text, _now, 'H');
		}
		_inventorySequence = true;
		return true;
	}
	if (reply.cue == 200 || reply.cue == 256) {
		return sayHopiInventory(reply);
	}
	if (reply.cue == 253 && _finale.scalpelNeeded) {
		say(302, 0, true);
		return true;
	}
	if (reply.cue == 306) {
		say(306, 0, true);
		say(306, 1, true);
		return true;
	}
	if (reply.cue == 309) {
		say(309, _finale.recordings, true);
		return true;
	}
	if (reply.cue == 313 && reply.variation < 6) {
		say(313, reply.variation, true);
		_audio.playEffect(reply.variation == 4 ? 40 : 41);
		say(313, reply.variation + 1, true);
		return true;
	}
	return false;
}

void RunawayRoomScene::resumeFinaleJailAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(18, false);
	}
	_audio.setMusicVolume(191);
	_nextMusic = 0;
}

void RunawayRoomScene::updateFinaleJailAudio() {
	if (!_audio.musicPlaying()) {
		if (!_nextMusic) {
			static const uint delays[] = {0, 60000, 30000, 15000};
			_nextMusic = _now + delays[_random.getRandomNumber(ARRAYSIZE(delays) - 1)];
		}
		if ((int32)(_now - _nextMusic) >= 0) {
			_audio.playMusic(18, false);
			_audio.setMusicVolume(191);
			_nextMusic = 0;
		}
	}
}

} // End of namespace Hollywood
