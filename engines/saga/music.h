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

// Music class

#ifndef SAGA_MUSIC_H
#define SAGA_MUSIC_H

#include "sound/audiocd.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/decoders/mp3.h"
#include "sound/decoders/vorbis.h"
#include "sound/decoders/flac.h"
#include "common/mutex.h"

namespace Saga {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001,
	MUSIC_DEFAULT = 0xffff
};

class MusicDriver : public MidiDriver {
public:
	MusicDriver();
	~MusicDriver();

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	bool isAdlib() { return _driverType == MD_ADLIB; }
	bool isMT32() { return _driverType == MD_MT32 || _nativeMT32; }
	void setGM(bool isGM) { _isGM = isGM; }

	//MidiDriver interface implementation
	int open() { return _driver->open(); }
	void close() { _driver->close(); }
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length) {}

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { _driver->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo()	{ return _driver->getBaseTempo(); }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	Common::Mutex _mutex;

protected:

	static void onTimer(void *data);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MidiDriverType _driverType;
	byte _channelVolume[16];
	bool _isGM;
	bool _nativeMT32;

	byte _masterVolume;

	byte *_musicData;
	uint16 *_buf;
	size_t _musicDataSize;
};

class Music {
public:

	Music(SagaEngine *vm, Audio::Mixer *mixer);
	~Music();
	bool isPlaying();
	bool hasDigitalMusic() { return _digitalMusic; }

	void play(uint32 resourceId, MusicFlags flags = MUSIC_DEFAULT);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }

	int32 *_songTable;
	int _songTableLen;

private:
	SagaEngine *_vm;
	Audio::Mixer *_mixer;

	MusicDriver *_driver;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _targetVolume;
	int _currentVolume;
	int _currentVolumePercent;
	bool _digitalMusic;

	ResourceContext *_musicContext;
	ResourceContext *_digitalMusicContext;
	MidiParser *_parser;

	byte *_midiMusicData;

	static void musicVolumeGaugeCallback(void *refCon);
	static void onTimer(void *refCon);
	void musicVolumeGauge();
};

} // End of namespace Saga

#endif
