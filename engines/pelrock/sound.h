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

#ifndef PELROCK_SOUND_H
#define PELROCK_SOUND_H

#include "audio/mixer.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Pelrock {

typedef struct {
	Common::String filename;
	uint32_t offset;
	uint32_t size;
	unsigned char *data;
} SonidoFile;

static const char *SOUND_FILENAMES[] = {
	"NO_SOUND.SMP", // 0 - Silence/disabled
	"BUHO_ZZZ.SMP", // 1 - Owl
	"BIRD_1_1.SMP", // 2 - Bird variant 1
	"BIRD_1_2.SMP", // 3 - Bird variant 2
	"BIRD_1_3.SMP", // 4 - Bird variant 3
	"DESPERZZ.SMP", // 5 - Yawn/stretch
	"HORN_5ZZ.SMP", // 6 - Car horn 5
	"HORN_6ZZ.SMP", // 7 - Car horn 6
	"HORN_8ZZ.SMP", // 8 - Car horn 8
	"SUZIPASS.SMP", // 9 - Suzi passing
	"CAT_1ZZZ.SMP", // 10 - Cat
	"DOG_01ZZ.SMP", // 11 - Dog bark 1
	"DOG_02ZZ.SMP", // 12 - Dog bark 2
	"DOG_04ZZ.SMP", // 13 - Dog bark 4
	"DOG_05ZZ.SMP", // 14 - Dog bark 5
	"DOG_06ZZ.SMP", // 15 - Dog bark 6
	"DOG_07ZZ.SMP", // 16 - Dog bark 7
	"DOG_09ZZ.SMP", // 17 - Dog bark 9
	"ALARMZZZ.SMP", // 18 - Alarm
	"AMBULAN1.SMP", // 19 - Ambulance
	"FOUNTAIN.SMP", // 20 - Fountain
	"GRILLOSZ.SMP", // 21 - Crickets
	"HOJASZZZ.SMP", // 22 - Leaves rustling
	"FLASHZZZ.SMP", // 23 - Flash/camera
	"CUCHI1ZZ.SMP", // 24 - Knife 1
	"KNRRRRRZ.SMP", // 25 - Snoring
	"PHONE_02.SMP", // 26 - Phone ring 2
	"PHONE_03.SMP", // 27 - Phone ring 3
	"SSSHTZZZ.SMP", // 28 - Shush/quiet
	"BURGUER1.SMP", // 29 - Burger sizzle
	"FLIES_2Z.SMP", // 30 - Flies buzzing
	"PARRILLA.SMP", // 31 - Grill
	"WATER_2Z.SMP", // 32 - Water
	"XIQUETZZ.SMP", // 33 - Whistle
	"RONQUIZZ.SMP", // 34 - Snoring
	"MOCO1ZZZ.SMP", // 35 - Snot/mucus 1
	"MOCO2ZZZ.SMP", // 36 - Snot/mucus 2
	"SPRINGZZ.SMP", // 37 - Spring bounce
	"MARUJASZ.SMP", // 38 - Gossip/chatter
	"ELECTROZ.SMP", // 39 - Electric shock
	"GLASS1ZZ.SMP", // 40 - Glass clink
	"OPDOORZZ.SMP", // 41 - Door open
	"CLDOORZZ.SMP", // 42 - Door close
	"FXH2ZZZZ.SMP", // 43 - Effect 2
	"BOTEZZZZ.SMP", // 44 - Bottle
	"ELEC3ZZZ.SMP", // 45 - Electric 3
	"AJARLZZZ.SMP", // 46 - Ajar/creak
	"BELCHZZZ.SMP", // 47 - Belch/burp
	"64ZZZZZZ.SMP", // 48 - Sound effect 64
	"BIRDOWL2.SMP", // 49 - Bird/owl 2
	"BUBBLE2Z.SMP", // 50 - Bubbles
	"BURGUER1.SMP", // 51 - Burger (duplicate)
	"CACKLEZZ.SMP", // 52 - Cackle/laugh
	"CERAMIC1.SMP", // 53 - Ceramic break
	"CLANG5ZZ.SMP", // 54 - Metal clang
	"CUCHI2ZZ.SMP", // 55 - Knife 2
	"CUCHI3ZZ.SMP", // 56 - Knife 3
	"ELEC3ZZZ.SMP", // 57 - Electric 3 (duplicate)
	"HOJASZZZ.SMP", // 58 - Leaves (duplicate)
	"LIMA1ZZZ.SMP", // 59 - File/rasp
	"MOROSZZZ.SMP", // 60 - Moors/crowd
	"MOROZZZZ.SMP", // 61 - Moor/crowd
	"MUD1ZZZZ.SMP", // 62 - Mud squelch
	"PICOZZZZ.SMP", // 63 - Pickaxe
	"PICO1XZZ.SMP", // 64 - Pickaxe 1
	"PICO2XZZ.SMP", // 65 - Pickaxe 2
	"PICO3XZZ.SMP", // 66 - Pickaxe 3
	"RIMSHOTZ.SMP", // 67 - Rimshot drum
	"RONCOZZZ.SMP", // 68 - Snoring
	"SORBOZZZ.SMP", // 69 - Slurp/sip
	"VIENTO1Z.SMP", // 70 - Wind
	"2ZZZZZZZ.SMP", // 71 - Sound 2
	"20ZZZZZZ.SMP", // 72 - Sound 20
	"21ZZZZZZ.SMP", // 73 - Sound 21
	"23ZZZZZZ.SMP", // 74 - Sound 23
	"107ZZZZZ.SMP", // 75 - Sound 107
	"39ZZZZZZ.SMP", // 76 - Sound 39
	"81ZZZZZZ.SMP", // 77 - Sound 81
	"88ZZZZZZ.SMP", // 78 - Sound 88
	"92ZZZZZZ.SMP", // 79 - Sound 92
	"SAW_2ZZZ.SMP", // 80 - Saw
	"QUAKE2ZZ.SMP", // 81 - Earthquake
	"ROCKSZZZ.SMP", // 82 - Rocks falling
	"IN_FIREZ.SMP", // 83 - Fire
	"BEAMZZZZ.SMP", // 84 - Beam/ray
	"GLISSDWN.SMP", // 85 - Glissando down
	"REMATERL.SMP", // 86 - Rematerialize
	"FXH1ZZZZ.SMP", // 87 - Effect 1
	"FXH3ZZZZ.SMP", // 88 - Effect 3
	"FXH4ZZZZ.SMP", // 89 - Effect 4
	"MATCHZZZ.SMP", // 90 - Match strike
	"SURF_01Z.SMP", // 91 - Surf wave 1
	"SURF_02Z.SMP", // 92 - Surf wave 2
	"SURF_04Z.SMP", // 93 - Surf wave 4
	"TWANGZZZ.SMP", // 94 - Twang
	"LANDCRAS.SMP", // 95 - Crash landing
};

enum SoundFormat {
	SOUND_FORMAT_RAWPCM,
	SOUND_FORMAT_MILES,
	SOUND_FORMAT_MILES2,
	SOUND_FORMAT_RIFF,
	SOUND_FORMAT_INVALID
};

struct SoundData {
	SoundFormat format;
	int sampleRate;
	byte *data;
	uint32 size;
};

static const uint COUNTER_MASK = 0x1F;

const int kMaxChannels = 15;

class GameRNG {

private:
	uint32_t _state;

public:
	// LCG constants (from JUEGO.EXE @ 0x0002b12f)
	static constexpr uint32_t MULTIPLIER = 0x41C64E6D; // 1103515245
	static constexpr uint32_t INCREMENT = 0x3039;      // 12345

	GameRNG(uint32_t seed = 0) {
		_state = seed & 0xFFFFFFFF;
	}

	// Generate next random number (0-32767)
	uint16_t nextRandom() {
		_state = (_state * MULTIPLIER + INCREMENT) & 0xFFFFFFFF;
		return static_cast<uint16_t>((_state >> 16) & 0x7FFF);
	}

	uint32_t getState() const {
		return _state;
	}

	void setState(uint32_t state) {
		_state = state & 0xFFFFFFFF;
	}
};

class SoundManager {
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();
	void playSound(byte index, int volume = 255);
	void stopAllSounds();
	void stopSound(int channel);
	void stopMusic();
	void setVolume(int volume);
	bool isPlaying() const;
	bool isPlaying(int channel) const;
	void playMusicTrack(int trackNumber);
	bool isMusicPlaying() const {
		return _isMusicPlaying;
	}
	void loadSoundIndex();

	int tick(uint32 frameCount);

private:
	void playSound(SonidoFile sound, int volume = 255);
	SoundFormat detectFormat(byte *data, uint32 size);
	int getSampleRate(byte *data, SoundFormat format);
	int findFreeChannel();

private:
	Audio::Mixer *_mixer;
	bool _isMusicPlaying = false;
	int _currentVolume;
	Common::File *_musicFile;
	byte _currentMusicTrack = 0;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandles[kMaxChannels];
	Common::HashMap<Common::String, SonidoFile> _soundMap;
	GameRNG _rng = GameRNG(0);
};

} // End of namespace Pelrock

#endif // PELROCK_SOUND_H
