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

#ifndef SCUMM_PLAYERS_PLAYER_V1_H
#define SCUMM_PLAYERS_PLAYER_V1_H

#include "scumm/players/player_v2.h"

namespace Scumm {

/**
 * Scumm V1 PC-Speaker player.
 */
class Player_V1 : public Player_V2 {
public:
	Player_V1(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr);
	~Player_V1() override;

	void startSound(int sound) override;
	void stopSound(int sound) override;
	void stopAllSounds() override;
	int  getMusicTimer() override;

protected:
	void nextTick() override;
	void clear_channel(int i) override;
	void chainSound(int nr, byte *data) override;

	void generateSpkSamples(int16 *data, uint len) override;
	void generatePCjrSamples(int16 *data, uint len) override;

	void set_mplex(uint mplex);
	void parseSpeakerChunk();
	void nextSpeakerCmd();
	void parsePCjrChunk();
	void nextPCjrCmd();

private:
	struct channel_data_v1 {
		uint freq;
		uint volume;
		byte *cmd_ptr;
		uint notelen;
		uint hull_counter;
		uint attack;
		uint decay;
		uint level;
		uint sustain_1;
		uint sustain_2;
		int  sustctr;
	};

	channel_data_v1 _channels[4];

	byte *_next_chunk;
	byte *_repeat_chunk;
	uint  _chunk_type;
	uint  _mplex_step;
	uint  _mplex;
	uint  _repeat_ctr;
	uint  _freq_current;
	int   _forced_level;
	uint16 _random_lsr;
	uint  *_value_ptr;
	uint  _time_left;
	uint  _start;
	uint  _end;
	int   _delta;
	uint  *_value_ptr_2;
	uint  _time_left_2;
	uint  _start_2;
	int   _delta_2;
};

} // End of namespace Scumm

#endif
