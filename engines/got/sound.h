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

#ifndef GOT_SOUND_H
#define GOT_SOUND_H

#include "audio/mixer.h"
#include "got/data/defines.h"
#include "got/gfx/gfx_chunks.h"

namespace Got {

enum {
	OW,
	GULP,
	SWISH,
	YAH,
	ELECTRIC,
	THUNDER,
	DOOR,
	FALL,
	ANGEL,
	WOOP,
	DEAD,
	BRAAPP,
	WIND,
	PUNCH1,
	CLANG,
	EXPLODE,
	BOSS11,
	BOSS12,
	BOSS13
};

#define NUM_SOUNDS 19

class Sound {
private:
	byte *_soundData = nullptr;
	Header _digiSounds[NUM_SOUNDS];
	Audio::SoundHandle _soundHandle;
	int _currentPriority = 0;

	const char *_currentMusic = nullptr;
	Audio::SoundHandle _musicHandle;

	const char *getMusicName(int num) const;

public:
	~Sound() {
		delete[] _soundData;
	}
	void load();

	void playSound(int index, bool override);
	void playSound(const Gfx::GraphicChunk &src);
	bool soundPlaying() const;

	void musicPlay(const int num, const bool override) {
		musicPlay(getMusicName(num), override);
	}
	void musicPlay(const char *name, bool override);
	void musicPause();
	void musicResume();
	void musicStop();
	bool musicIsOn() const;
};

extern void playSound(int index, bool override);
extern void playSound(const Gfx::GraphicChunk &src);
extern bool soundPlaying();
extern void musicPlay(int num, bool override);
extern void musicPlay(const char *name, bool override);
extern void musicPause();
extern void musicResume();

} // namespace Got

#endif
