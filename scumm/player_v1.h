/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef PLAYER_V1_H
#define PLAYER_V1_H

#include "player_v2.h"

class Player_V1 : public Player_V2 {
public:
	Player_V1(Scumm *scumm);
	~Player_V1();

	void startSound(int nr, byte *data);
	int  getMusicTimer() const;

protected:
	void restartSound();
	void next_speaker_cmd(ChannelInfo *channel);
	void chainSound(int nr, byte *data);

	void do_mix (int16 *buf, uint len);

	void set_mplex(uint mplex);
	void parse_chunk();
	void next_speaker_cmd();
	void generateSpkSamples(int16 *data, uint len);

private:
	byte *_next_cmd;
	byte *_repeat_ptr;
	uint  _chunk_type;
	uint  _time_left;
	uint  _mplex;
	uint  _freq_start;
	uint  _freq_end;
	uint  _freq_delta;
	uint  _repeat_ctr;
	uint  _freq_current;
	uint  _forced_level;
	uint16 _random_lsr;
};

#endif
