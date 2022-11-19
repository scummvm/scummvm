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

#ifndef SCUMM_PLAYERS_PLAYER_V2BASE_H
#define SCUMM_PLAYERS_PLAYER_V2BASE_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/music.h"

namespace Scumm {

class ScummEngine;


#include "common/pack-start.h"	// START STRUCT PACKING

struct channel_data {
	uint16 time_left;          // 00
	uint16 next_cmd;           // 02
	uint16 base_freq;          // 04
	uint16 freq_delta;         // 06
	uint16 freq;               // 08
	uint16 volume;             // 10
	uint16 volume_delta;       // 12
	uint16 tempo;              // 14
	uint16 inter_note_pause;   // 16
	uint16 transpose;          // 18
	uint16 note_length;        // 20
	uint16 hull_curve;         // 22
	uint16 hull_offset;        // 24
	uint16 hull_counter;       // 26
	uint16 freqmod_table;      // 28
	uint16 freqmod_offset;     // 30
	uint16 freqmod_incr;       // 32
	uint16 freqmod_multiplier; // 34
	uint16 freqmod_modulo;     // 36
	uint16 unknown[4];         // 38 - 44
	uint16 music_timer;        // 46
	uint16 music_script_nr;    // 48
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

/**
 * Common base class for Player_V2 and Player_V2CMS.
 */
class Player_V2Base : public MusicEngine {
public:
	Player_V2Base(ScummEngine *scumm, bool pcjr);
	~Player_V2Base() override;

	// MusicEngine API
// 	virtual void setMusicVolume(int vol);
// 	virtual void startSound(int sound);
// 	virtual void stopSound(int sound);
// 	virtual void stopAllSounds();
 	int  getMusicTimer() override;
// 	virtual int  getSoundStatus(int sound) const;

protected:
	bool _isV3Game;
	ScummEngine *_vm;

	bool _pcjr;
	int _header_len;

	int   _current_nr;
	byte *_current_data;
	int   _next_nr;
	byte *_next_data;
	byte *_retaddr;

	union ChannelInfo {
		channel_data d;
		uint16 array[sizeof(channel_data)/2];
	};

	ChannelInfo _channels[5];

private:
	int _music_timer;
	int _music_timer_ctr;
	int _ticks_per_music_timer;

	const uint16 *_freqs_table;

protected:
	virtual void nextTick();
	virtual void clear_channel(int i);
	virtual void chainSound(int nr, byte *data);
	virtual void chainNextSound();

	void execute_cmd(ChannelInfo *channel);
	void next_freqs(ChannelInfo *channel);
};


} // End of namespace Scumm

#endif
