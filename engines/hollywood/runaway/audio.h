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

#ifndef HOLLYWOOD_RUNAWAY_AUDIO_H
#define HOLLYWOOD_RUNAWAY_AUDIO_H

#include "hollywood/sound.h"

namespace Common {
class RandomSource;
}

namespace Hollywood {

class RunawayAudio {
public:
	explicit RunawayAudio(Audio::Mixer *mixer) : _mixer(mixer) {}
	~RunawayAudio();
	void setMuseum(bool museum) { _museum = museum; }
	void setChapter(byte chapter);
	void playIntroMusic(uint16 cue = 2);
	// Script volumes are native percentages, before the user's mixer settings.
	uint32 playMusic(uint16 cue, bool loop = true, bool fullVolume = false);
	void updateBackgroundMusic(uint32 now, Common::RandomSource &random, byte firstCue, byte cueCount, byte volume = 100, int repeatDelay = -1);
	uint16 musicCue() const { return _musicCue; }
	bool musicLooping() const { return _musicLooping && musicPlaying(); }
	void playMusicEffect(uint16 cue, byte volume = 100, byte channel = 0, bool loop = false);
	void setMusicVolume(byte volume, int attenuation = 0);
	void setEffectVolume(byte channel, byte volume);
	void setListenerPosition(int x);
	int listenerPosition() const { return _listenerX; }
	uint32 playSpeech(uint16 sample, bool global = false);
	void setSpeechVolume(byte volume);
	void playBackgroundSpeech(uint16 sample, byte channel);
	void playEffect(uint16 cue, byte volume = 100, byte channel = 0, bool loop = false);
	void playEffectAt(uint16 cue, Common::Point position, byte volume = 100, byte channel = 0, bool loop = false);
	void playFootstep(byte surface, byte variant, byte volume = 100, int x = -1);
	void playPointer();
	void playInventoryIdle();
	void playMenuMusic();
	void playDemoSlideshowMusic();
	void stopMusic();
	void pauseMusic(bool paused);
	void stopSpeech();
	void stopEffect();
	void stopEffect(byte channel);
	void pauseEffect(byte channel, bool paused);
	bool musicPlaying() const;
	bool speechPlaying() const;
	bool effectPlaying(byte channel = 0) const;

private:
	enum {
		kMusicVolumeBase = 2800,
		kAmbientVolumeBase = 3000,
		kEffectVolumeBase = 3200,
		kSpeechVolumeBase = 4000,
		kFootstepVolumeBase = 2400
	};
	byte mixerVolume(byte volume, int base, int attenuation = 0) const;
	void playEffect(uint16 cue, byte volume, byte channel, bool loop, const Common::Point *position);
	uint32 play(const char *archive, uint16 cue, uint tableCount, bool pairedTables,
		byte flags, Audio::Mixer::SoundType type, SoundChannel &channel, byte volume, bool loop, uint rate,
		const Common::Point *position = nullptr);
	bool _museum = false;
	byte _chapter = 3;
	Audio::Mixer *_mixer;
	SoundChannel _music, _speech, _effects[3], _footsteps[14], _pointer;
	uint16 _musicCue = 0;
	int _musicVolumeBase = kMusicVolumeBase;
	int _effectVolumeBase[3] = {kEffectVolumeBase, kEffectVolumeBase, kEffectVolumeBase};
	bool _musicLooping = false;
	uint32 _nextMusic = 0;
	int _listenerX = 512;
};

} // End of namespace Hollywood

#endif
