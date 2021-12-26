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

// Music class

#ifndef SAGA_MUSIC_H
#define SAGA_MUSIC_H

#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"
#include "audio/mt32gm.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"

class TownsPC98_AudioDriver;

namespace Saga {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001
};

class Music {
private:
	static const uint8 MUSIC_SUNSPOT = 26;
	static const uint8 MT32_GOODBYE_MSG[MidiDriver_MT32GM::MT32_DISPLAY_NUM_CHARS];

public:
	Music(SagaEngine *vm, Audio::Mixer *mixer);
	~Music();
	void close();
	bool isPlaying();
	bool hasDigitalMusic() { return _digitalMusic; }

	void play(uint32 resourceId, MusicFlags flags = MUSIC_NORMAL);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }
	void resetVolume();
	bool isFading();

	bool isAdlib() const { return  _driverType == MT_ADLIB; }

	void syncSoundSettings();

	Common::Array<int32> _songTable;

private:
	SagaEngine *_vm;
	Audio::Mixer *_mixer;

	MidiParser *_parser;
	MidiDriver_Multisource *_driver;
	TownsPC98_AudioDriver *_driverPC98;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _userVolume;
	bool _userMute;
	int _targetVolume;
	int _currentVolume;
	int _currentVolumePercent;
	bool _digitalMusic;
	MusicType _musicType;
	MusicType _driverType;

	ResourceContext *_musicContext;
	ResourceContext *_digitalMusicContext;


	static void musicVolumeGaugeCallback(void *refCon);
	static void timerCallback(void *refCon);
	void onTimer();
	bool playDigital(uint32 resourceId, MusicFlags flags);
	void playQuickTime(uint32 resourceId, MusicFlags flags);
	void playMidi(uint32 resourceId, MusicFlags flags);
	void musicVolumeGauge();
	ByteArray *_currentMusicBuffer;
	ByteArray _musicBuffer[2];
};

} // End of namespace Saga

#endif
