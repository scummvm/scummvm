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

#ifndef CHEWY_AILCLASS_H
#define CHEWY_AILCLASS_H

#include "chewy/ngstypes.h"

void mod_irq();
void check_sample_end();
void DecodePatternLine();
void DecodeChannel(int16 ch);

class ailclass {
public:
	ailclass();
	~ailclass();

	int16 init(uint16 freq);
	int16 init(char *midi_drv_name);
	void exit1();

	void set_music_mastervol(int16 vol);
	void set_sound_mastervol(int16 vol);
	void set_stereo_pos(int16 channel, int16 pos);
	void set_channelvol(uint8 channel, uint8 vol);
	void disable_sound();

	void get_musik_info(musik_info *mi);
	void get_channel_info(channel_info *mi, int16 kanal);
	int16 music_playing();
	int16 get_sample_status(int16 kanal);

	void init_note_table(uint16 sfreq);
	void init_mix_mode();
	void exit_mix_mode();
	void play_mod(tmf_header *th);
	void stop_mod();
	void continue_music();
	void play_sequence(int16 startpos, int16 endpos);
	void play_pattern(int16 pattnr);
	void set_loopmode(int16 mode);
	void fade_in(uint16 delay);
	void fade_out(uint16 delay);

	void play_voc(char *anf_adr, int16 kanal, int16 vol,
	              int16 rep);
	void play_raw(int16 kanal, char *voc, uint32 len,
	              uint16 freqwert, int16 volume, int16 rep);
	void end_sound();
	void stop_sound();
	void continue_sound();
	void stop_sample(int16 kanal);
	void continue_sample(int16 kanal);
	void end_sample(int16 kanal);
	void init_double_buffer(char *b1, char *b2,
	                        uint32 len, int16 kanal);
	// FIXME : was FILE*
	void start_db_voc(void *voc, int16 kanal, int16 vol);
	void serve_db_samples();
private:
};
#endif
