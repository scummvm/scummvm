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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Music class

#ifndef SAGA2_MUSIC_H
#define SAGA2_MUSIC_H

#include "audio/mididrv.h"
#include "audio/midiplayer.h"
#include "audio/midiparser.h"
#include "audio/mixer.h"

namespace Saga2 {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001
};

class MusicDriver : public Audio::MidiPlayer {
public:
	MusicDriver();

	void play(byte *data, uint32 size, bool loop);
	void pause() override;
	void resume() override;

	bool isAdlib() const { return _driverType == MT_ADLIB; }

	// FIXME
	bool isPlaying() const { return _parser && _parser->isPlaying(); }

	// MidiDriver_BASE interface implementation
	void send(uint32 b) override;
	void metaEvent(byte type, byte *data, uint16 length) override;

protected:
	MusicType _driverType;
	bool _milesAudioMode;
};

class Music {
public:

	Music(hResContext *musicRes, Audio::Mixer *mixer);
	~Music();
	bool isPlaying();

	void play(uint32 resourceId, MusicFlags flags = MUSIC_NORMAL);
	void pause();
	void resume();
	void stop();

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }

	bool isAdlib() const { return _player->isAdlib(); }

private:
	Saga2Engine *_vm;
	Audio::Mixer *_mixer;

	MusicDriver *_player;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _currentVolume;

	hResContext *_musicContext;

	byte *_currentMusicBuffer;
};

} // End of namespace Saga2

#endif
