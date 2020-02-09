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

#ifndef SCUMM_PLAYERS_PLAYER_HE_H
#define SCUMM_PLAYERS_PLAYER_HE_H

#include "scumm/music.h"
#include "audio/mixer.h"
#include "audio/mididrv.h"
#include "common/mutex.h"

class MidiParser;

#ifdef ENABLE_HE

namespace Scumm {
class ScummEngine;

class Player_HE : public MusicEngine, public MidiDriver {
public:
	Player_HE(ScummEngine *scumm);
	~Player_HE() override;
	void setMusicVolume(int vol) override;
	void startSound(int sound) override { startSoundWithTrackID(sound, 0); }
	void startSoundWithTrackID(int sound, int track) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int  getSoundStatus(int sound) const override;
	int  getMusicTimer() override;

	int open() override;
	bool isOpen() const override;
	void close() override;
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;
	uint32 getBaseTempo() override;
	MidiChannel *allocateChannel() override { return NULL; };
	MidiChannel *getPercussionChannel() override { return NULL; };
	void send(uint32 b) override;

private:
	ScummEngine *_vm;
	MidiParser *_parser;
	MidiDriver *_midi;
	Common::Mutex _mutex;
	byte *_bank;
	int _bankSize;
	int _currentMusic;
	int _masterVolume;
	byte _channelVolume[16];
	static void onTimer(void *data);
	void loadAdLibBank();
};
}

#endif

#endif
