/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"

#include "scumm.h"
#include "sound.h"

int num_mix;

#define TICKS_PER_BEAT 480

#ifdef USE_ADLIB
#define TEMPO_BASE 0x1924E0
#define HARDWARE_TYPE 1
#else
#define TEMPO_BASE 0x400000
#define HARDWARE_TYPE 5
#endif
#define SYSEX_ID 0x7D
#define SPECIAL_CHANNEL 9

#define TRIGGER_ID 0
#define COMMAND_ID 1

#ifdef SAMNMAX
	#define MDHD_TAG "MDpg"
#else
      #define MDHD_TAG "MDhd"
#endif

int clamp(int val, int min, int max) {
	if (val<min)
		return min;
	if (val>max)
		return max;
	return val;
}

int transpose_clamp(int a, int b, int c) {
	if (b>a) a += (b - a + 11) / 12 * 12;
	if (c<a) a -= (a - c + 11) / 12 * 12;
	return a;
}

uint32 get_delta_time(byte **s) {
	byte *d = *s,b;
	uint32 time = 0;
	do {
		b = *d++;
		time = (time<<7) | (b&0x7F);
	} while (b&0x80);
	*s = d;
	return time;
}

uint read_word(byte *a) {
	return (a[0]<<8) + a[1];
}

void skip_midi_cmd(byte **song_ptr) {
	byte *s, code;

	const byte num_skip[] = {
		2,2,2,2,1,1,2
	};

	s = *song_ptr;

	code = *s++;

	if (code<0x80) {
		s = NULL;
	} else if (code<0xF0) {
		s += num_skip[(code&0x70)>>4];
	} else {
		if (code==0xF0 || code==0xF7 || code==0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
		} else {
			s = NULL;
		}
	}
	*song_ptr = s;
}

int is_note_cmd(byte **a, IsNoteCmdData *isnote) {
	byte *s = *a;
	byte code;

	code = *s++;
	
	switch(code>>4) {
	case 8: /* key off */
		isnote->chan = code&0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		return 1;
	case 9: /* key on */
		isnote->chan = code&0xF;
		isnote->note = *s++;
		isnote->vel = *s++;
		*a = s;
		if (isnote->vel)
			return 2;
		return 1;
	case 0xA:
	case 0xB:
	case 0xE:
		s++;
	case 0xC:
	case 0xD:
		s++;
		break;
	case 0xF:
		if (code==0xF0 || code==0xF7 || code==0xFF && *s++ != 0x2F) {
			s += get_delta_time(&s);
			break;
		}
		return -1;
	default:
		return -1;
	}
	*a = s;
	return 0;
}

/**********************************************************************/

void SoundEngine::lock() {
	_locked++;
}

void SoundEngine::unlock() {
	_locked--;
}

byte *SoundEngine::findTag(int sound, char *tag, int index) {
	byte *ptr = _base_sounds[sound];
	int32 size,pos;

	if (ptr==NULL) {
		debug(1, "SoundEngine::findTag completely failed finding sound %d", sound);
		return 0;
	}

	ptr += 8;
	size = READ_BE_UINT32_UNALIGNED(ptr);
	ptr += 4;

	pos = 0;
	while (pos < size) {
		if (!memcmp(ptr + pos, tag, 4) && !index--)
			return ptr + pos + 8;
		pos += READ_BE_UINT32_UNALIGNED(ptr + pos + 4) + 8;
	}
	debug(1, "SoundEngine::findTag failed finding sound %d", sound);
	return NULL;

}

bool SoundEngine::start_sound(int sound) {
	Player *player;
	void *mdhd;

        mdhd = findTag(sound, MDHD_TAG, 0);
        if (!mdhd)  {
		warning("SE::start_sound failed: Couldn't find %s", MDHD_TAG);
		return false;
	}
	player = allocate_player(128);
	if (!player)
		return false;
	
	player->clear();
	return player->start_sound(sound);
}


Player *SoundEngine::allocate_player(byte priority) {
	Player *player = _players, *best = NULL;
	int i;
	byte bestpri = 255;

	for (i=ARRAYSIZE(_players); i!=0; i--, player++) {
		if (!player->_active)
			return player;
		if (player->_priority < bestpri) {
			best = player;
			bestpri = player->_priority;
		}
	}

	if (bestpri < priority)
		return best;

	debug(1, "Denying player request");
	return NULL;
}

void SoundEngine::init_players() {
	Player *player = _players;
	int i;

	for (i=ARRAYSIZE(_players); i!=0; i--, player++) {
		player->_active = false;
		player->_se = this;		
	}
}

void SoundEngine::init_sustaining_notes() {
	SustainingNotes *next = NULL, *sn = _sustaining_notes;
	int i;

	_sustain_notes_used = NULL;
	_sustain_notes_head = NULL;

	for (i=ARRAYSIZE(_sustaining_notes);i!=0; i--,sn++) {
		sn->next = next;
		next = sn;
	}
	_sustain_notes_free = next;
}

void SoundEngine::init_volume_fader() {
	VolumeFader *vf = _volume_fader;
	int i;

	for (i=ARRAYSIZE(_volume_fader); i!=0; i--, vf++)
		vf->initialize();

	_active_volume_faders = false;
}

void SoundEngine::init_parts() {
	Part *part;
	int i;

	for (i=0,part=_parts; i!=ARRAYSIZE(_parts); i++, part++) {
		part->init(_driver);
		part->_slot = i;
	}
}

int SoundEngine::stop_sound(int sound) {
	Player *player = _players;
	int i;
	int r = -1;

	for (i=ARRAYSIZE(_players); i!=0; i--,player++) {
		if (player->_active && player->_id==sound) {
			player->clear();
			r = 0;
		}
	}
	return r;
}

int SoundEngine::stop_all_sounds() {
	Player *player = _players;
	int i;

	for (i=ARRAYSIZE(_players); i!=0; i--,player++) {
		if (player->_active)
			player->clear();
	}
	return 0;
}

void SoundEngine::on_timer() {
	if (_locked || _paused)
		return;
	
	lock();
	
	sequencer_timers();
	expire_sustain_notes();
	expire_volume_faders();
	_driver->on_timer();

	unlock();
}

void SoundEngine::sequencer_timers() {
	Player *player = _players;
	int i;

	for (i=ARRAYSIZE(_players); i!=0; i--,player++) {
		if (player->_active)
			player->sequencer_timer();
	}
}

void Player::sequencer_timer() {
	byte *mtrk;
	uint32 counter;
	byte *song_ptr;

	counter = _timer_counter + _timer_speed;
	_timer_counter = counter&0xFFFF;
	_cur_pos += counter>>16;
	_tick_index += counter>>16;

	if (_tick_index>=_ticks_per_beat) {
		_beat_index += _tick_index/_ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}
	if (_loop_counter && _beat_index >= _loop_from_beat && _tick_index >= _loop_from_tick) {
		_loop_counter--;
		jump(_track_index, _loop_to_beat, _loop_to_tick);
	}
	if (_next_pos <= _cur_pos) {
		mtrk = _se->findTag(_song_index, "MTrk", _track_index);
		if (!mtrk) {
			warning("Sound %d was unloaded while active", _song_index);
			clear();
		} else {
			song_ptr = mtrk + _song_offset;
			_abort = false;

			while (_next_pos <= _cur_pos) {
				song_ptr = parse_midi(song_ptr);
				if (!song_ptr || _abort)
					return;
				_next_pos += get_delta_time(&song_ptr);
			}
			_song_offset = song_ptr - mtrk;
		}
	}
}

void SoundEngine::handle_marker(uint id, byte data) {
	uint16 *p;
	uint pos;	
	
	pos = _queue_end;
	if (pos == _queue_pos)
		return;
	
	if (_queue_adding && _queue_sound==id && data==_queue_marker)
		return;

	p = _cmd_queue[pos].array;

	if (p[0] != TRIGGER_ID || p[1] != id || p[2] != data)
		return;

	_trigger_count--;
	_queue_cleared = false;
	do {
		pos = (pos+1) & (ARRAYSIZE(_cmd_queue)-1);
		if (_queue_pos == pos)
			break;
		p = _cmd_queue[pos].array;
		if (*p++ != COMMAND_ID)
			break;
		_queue_end = pos;

		do_command(p[0],p[1],p[2],p[3],p[4],p[5],p[6],0);

		if (_queue_cleared)
			return;
		pos = _queue_end;
	} while (1);

	_queue_end = pos;
}

int SoundEngine::get_channel_volume(uint a) {
	if (a<8)
		return _channel_volume_eff[a];
	return _master_volume;
}

Part *SoundEngine::allocate_part(byte pri) {
	Part *part,*best=NULL;
	int i;

	for (i=ARRAYSIZE(_parts),part=_parts; i!=0; i--,part++) {
		if (!part->_player)
			return part;
		if (pri >= part->_pri_eff) {
			pri = part->_pri_eff;
			best = part;
		}
	}

	if (best)
		best->uninit();
	else
		debug(1, "Denying part request");
	return best;
}

void SoundEngine::expire_sustain_notes() {
	SustainingNotes *sn,*next;
	Player *player;
	uint32 counter;

	for(sn=_sustain_notes_head; sn; sn = next) {
		next = sn->next;
		player = sn->player;

		counter = sn->counter + player->_timer_speed;
		sn->pos += counter>>16;
		sn->counter = (unsigned short)counter & 0xFFFF;

		if (sn->pos >= sn->off_pos) {
			player->key_off(sn->chan, sn->note);

			/* Unlink the node */
			if (next)
				next->prev = sn->prev;
			if (sn->prev)
				sn->prev->next = next;
			else
				_sustain_notes_head = next;

			/* And put it in the free list */
			sn->next = _sustain_notes_free;
			_sustain_notes_free = sn;
		}
	}
}

void SoundEngine::expire_volume_faders() {
	VolumeFader *vf;
	int i;

	if (++_volume_fader_counter & 7)
		return;

	if (!_active_volume_faders)
		return;

	_active_volume_faders = false;
	vf = _volume_fader;
	for (i=ARRAYSIZE(_volume_fader); i!=0; i--,vf++) {
		if (vf->active) {
			_active_volume_faders = true;
			vf->on_timer();
		}
	}
}

void VolumeFader::on_timer() {
	byte newvol;
	
	newvol = curvol + speed_hi;
	speed_lo_counter += speed_lo;

	if (speed_lo_counter >= speed_lo_max) {
		speed_lo_counter -= speed_lo_max;
		newvol += direction;
	}
	
	if (curvol!=newvol) {
		if (!newvol) {
			player->clear();
			active = false;
			return;
		}
		curvol = newvol;
		player->set_vol(newvol);
	}
	
	if (!--num_steps) {
		active = false;
	}
}

int SoundEngine::get_sound_status(int sound) {
	int i;
	Player *player;

	for (i=ARRAYSIZE(_players),player=_players; i!=0; i--,player++) {
		if (player->_active && player->_id==(uint16)sound)
			return 1;
	}
	return get_queue_sound_status(sound);
}

int SoundEngine::get_queue_sound_status(int sound) {
	uint16 *a;
	int i,j;
	
	j = _queue_pos;
	i = _queue_end;

	while (i!=j) {
		a = _cmd_queue[i].array;
		if (a[0] == COMMAND_ID && a[1] == 8 && a[2] == (uint16)sound)
			return 2;
		i = (i+1) & (ARRAYSIZE(_cmd_queue)-1);
	}
	return 0;
}

int SoundEngine::set_volchan(int sound, int volchan) {
	int r;
	int i;
	int num;
	Player *player,*best,*sameid;

	r = get_volchan_entry(volchan);
	if (r==-1)
		return -1;

	if (r >= 8) {
		for(i=ARRAYSIZE(_players),player=_players; i!=0; i--,player++) {
			if (player->_active && player->_id==(uint16)sound && player->_vol_chan!=(uint16)volchan) {
				player->_vol_chan = volchan;
				player->set_vol(player->_volume);
				return 0;
			}
		}
		return -1;
	} else {
		best = NULL;
		num = 0;
		sameid = NULL;
		for(i=ARRAYSIZE(_players),player=_players; i!=0; i--,player++) {
			if (player->_active) {
				if (player->_vol_chan==(uint16)volchan) {
					num++;
					if (!best || player->_priority <= best->_priority)
						best = player;
				} else if (player->_id == (uint16)sound) {
					sameid = player;
				}
			}
		}
		if (sameid==NULL)
			return -1;
		if (num >= r)
			best->clear();
		player->_vol_chan = volchan;
		player->set_vol(player->_volume);
		return 0;
	}
}

int SoundEngine::clear_queue() {
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int SoundEngine::enqueue_command(int a, int b, int c, int d, int e, int f, int g) {
	uint16 *p;
	uint i;

	i = _queue_pos;

	if (i == _queue_end)
		return -1;

	if (a==-1) {
		_queue_adding = false;
		_trigger_count++;
		return 0;
	}

	p = _cmd_queue[_queue_pos].array;
	p[0] = COMMAND_ID;
	p[1] = a;
	p[2] = b;
	p[3] = c;
	p[4] = d;
	p[5] = e;
	p[6] = f;
	p[7] = g;

	i = (i+1) & (ARRAYSIZE(_cmd_queue)-1);

	if (_queue_end!=i) {
		_queue_pos = i;
		return 0;
	} else {
		_queue_pos = (i-1) & (ARRAYSIZE(_cmd_queue)-1);
		return -1;
	}
}

int SoundEngine::query_queue(int param) {
	switch(param) {
	case 0: /* get trigger count */
		return _trigger_count;
	case 1: /* get trigger type */
		if (_queue_end==_queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2: /* get trigger sound */
		if (_queue_end==_queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int SoundEngine::get_music_volume() {
	return _music_volume;
}

int SoundEngine::set_music_volume(uint vol) {
	if (vol > 100) 
		vol = 100;

	if (vol < 1)
		vol = 1;
	
	_music_volume = vol;
	return 0;
}

int SoundEngine::set_master_volume(uint vol) {
	int i;
	if (vol > 127)
		return -1;

	if (_music_volume > 0)
		vol = vol / (100 / _music_volume);

	_master_volume = vol;
	for (i=0; i!=8; i++)
		_channel_volume_eff[i] = (_channel_volume[i]+1) * vol >> 7;
	update_volumes();
	return 0;
}

int SoundEngine::get_master_volume() {
	return _master_volume;
}

int SoundEngine::terminate() {
	return 0;
	/* not implemented */
}


int SoundEngine::enqueue_trigger(int sound, int marker) {
	uint16 *p;
	uint pos;

	pos = _queue_pos;

	p = _cmd_queue[pos].array;
	p[0] = TRIGGER_ID;
	p[1] = sound;
	p[2] = marker;

	pos = (pos+1) & (ARRAYSIZE(_cmd_queue)-1);
	if (_queue_end==pos) {
		_queue_pos = (pos-1) & (ARRAYSIZE(_cmd_queue)-1);
		return -1;
	}
	
	_queue_pos = pos;
	_queue_adding = true;
	_queue_sound = sound;
	_queue_marker = marker;
	return 0;
}

int32 SoundEngine::do_command(int a, int b, int c, int d, int e, int f, int g, int h) {
	byte cmd = a&0xFF;
	byte param = a>>8;
	Player *player;

	if (!_initialized && (cmd || param))
		return -1;

	if (param==0) {
		switch(cmd) {
		case 6:
			return set_master_volume(b);
		case 7:
			return get_master_volume();
		case 8:
			return start_sound(b) ? 0 : -1;
		case 9:
			return stop_sound(b);
		case 11:
			return stop_all_sounds();
		case 13:
			return get_sound_status(b);
		case 16:
			return set_volchan(b,c);
		case 17:
			return set_channel_volume(b,c);
		case 18:
			return set_volchan_entry(b,c);
		default:
			warning("SoundEngine::do_command invalid command %d", cmd);
		}
	} else if (param==1) {
		
		if ( (1<<cmd) & (0x783FFF)) {
			player = get_player_byid(b);
			if (!player)
				return -1;
			if ( (1<<cmd) & (1<<11 | 1<<22) ) {
				assert(c>=0 && c<=15);
				player = (Player*)player->get_part(c);
				if(!player)
					return -1;
			}
		}
	
		switch(cmd) {
		case 0:
			return player->get_param(c, d);
		case 1:
			player->set_priority(c);
			return 0;
		case 2:
			return player->set_vol(c);
		case 3:
			player->set_pan(c);
			return 0;
		case 4:
			return player->set_transpose(c, d);
		case 5:
			player->set_detune(c);
			return 0;
		case 6:
			player->set_speed(c);
			return 0;
		case 7:
			return player->jump(c,d,e) ? 0 : -1;
		case 8:
			return player->scan(c,d,e);
		case 9:
			return player->set_loop(c,d,e,f,g) ? 0 : -1;
		case 10:
			player->clear_loop();
			return 0;
		case 11:
			((Part*)player)->set_onoff(d!=0);
			return 0;
		case 12:
			return player->_hook.set(c, d, e);
		case 13:
			return player->fade_vol(c,d);
		case 14:
			return enqueue_trigger(b,c);
		case 15:
			return enqueue_command(b,c,d,e,f,g,h);
		case 16:
			return clear_queue();
		case 19:
			return player->get_param(c,d);
		case 20:
			return player->_hook.set(c,d,e);
		case 21:
			return -1;
		case 22:
			((Part*)player)->set_vol(d);
			return 0;
		case 23:
			return query_queue(b);
		case 24:
			return 0;
		default:
			warning("SoundEngine::do_command default midi command %d", cmd);
			return -1;
		}
	}

	return -1;
}

int SoundEngine::set_channel_volume(uint chan, uint vol) {
	if (chan>=8 || vol>127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * (vol+1) >> 7;
	update_volumes();
	return 0;
}

void SoundEngine::update_volumes() {
	Player *player;
	int i;

	for(i=ARRAYSIZE(_players),player=_players; i!=0; i--,player++) {
		if (player->_active)
			player->set_vol(player->_volume);
	}
}

int SoundEngine::set_volchan_entry(uint a, uint b) {
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan) {
	switch(param) {
	case 18:
		return _jump;
	case 19:
		return _transpose;
	case 20:
		return _part_onoff[chan];
	case 21:
		return _part_volume[chan];
	case 22:
		return _part_program[chan];
	case 23:
		return _part_transpose[chan];
	default:
		return -1;
	}
}

int HookDatas::set(byte cls, byte value, byte chan) {
	switch(cls) {
	case 0:
		_jump = value;
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan<16)
			_part_onoff[chan] = value;
		else if (chan==16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan<16)
			_part_volume[chan] = value;
		else if (chan==16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan<16)
			_part_program[chan] = value;
		else if (chan==16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan<16)
			_part_transpose[chan] = value;
		else if (chan==16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}


VolumeFader *SoundEngine::allocate_volume_fader() {
	VolumeFader *vf;
	int i;

	vf = _volume_fader;
	for(i=ARRAYSIZE(_volume_fader); vf->active; ) {
		vf++;
		if (!--i)
			return NULL;
	}

	vf->active = true;
	_active_volume_faders = true;
	return vf;
}

Player *SoundEngine::get_player_byid(int id) {
	int i;
	Player *player,*found=NULL;

	for(i=ARRAYSIZE(_players),player=_players; i!=0; i--,player++) {
		if (player->_active && player->_id==(uint16)id) {
			if(found)
				return NULL;
			found = player;
		}
	}
	return found;
}

int SoundEngine::get_volchan_entry(uint a) {
	if (a<8)
		return _volchan_table[a];
	return -1;
}

int SoundEngine::initialize(Scumm *scumm, SoundDriver *driver) {
	int i;
	if (_initialized)
		return -1;

	scumm->_soundEngine = this;
	_s = scumm;

	_driver = (SOUND_DRIVER_TYPE*)driver;

	_master_volume = 127;
	if (_music_volume < 1) _music_volume = 60;

	for (i=0; i!=8; i++)
		_channel_volume[i] = _channel_volume_eff[i] = _volchan_table[i] = 127;

	init_players();
	init_sustaining_notes();
	init_volume_fader();
	init_queue();
	init_parts();

	_driver->init(this);

	_initialized = true;
	
	return 0;
}

void SoundEngine::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void SoundEngine::pause(bool paused) {
	lock();

#if 0
	int i;
	Part *part;
	MidiChannel *mc;

	for (i=ARRAYSIZE(_parts),part=_parts; i!=0; i--, part++) {
		if (part->_player) {
			if (paused) {
				part->_vol_eff = 0;
			} else {
				part->set_vol(part->_vol);
			}
			part->vol_changed();
		}
	}
#endif

	_paused = paused;

	unlock();
}


/*************************************************************************/

int Player::fade_vol(byte vol, int time) {
	VolumeFader *vf;
	int i;

	cancel_volume_fade();
	if (time==0) {
		set_vol(vol);
		return 0;
	}
	
	vf = _se->allocate_volume_fader();
	if (vf==NULL)
		return -1;

	vf->player = this;
	vf->num_steps = vf->speed_lo_max = time;
	vf->curvol = _volume;
	i = (vol - vf->curvol);
	vf->speed_hi = i / time;
	if (i<0) {
		i = -i;
		vf->direction = -1;
	} else {
		vf->direction = 1;
	}
	vf->speed_lo = i % time;
	vf->speed_lo_counter = 0;
	return 0;
}

void Player::clear() {
	uninit_seq();
	cancel_volume_fade();
	uninit_parts();
	_active = false;
	_ticks_per_beat = TICKS_PER_BEAT;
}

bool Player::start_sound(int sound) {
	void *mdhd;
	
	mdhd = _se->findTag(sound, MDHD_TAG, 0);
	if (mdhd==NULL) {
		warning("P::start_sound failed: Couldn't find %s", MDHD_TAG);
		return false;
	}
	_parts = NULL;
	_active = true;
	_id = sound;
	_priority = 0x80;
	_volume = 0x7F;
	_vol_chan = 0xFFFF;

	_vol_eff = (_se->get_channel_volume(0xFFFF)<<7)>>7;

	_pan = 0;
	_transpose = 0;
	_detune = 0;

	hook_clear();
	if (start_seq_sound(sound) != 0) {
		_active = false;
		return false;
	}
	return true;
}

void Player::hook_clear() {
	memset(&_hook, 0, sizeof(_hook));
}

int Player::start_seq_sound(int sound) {
	byte *ptr, *track_ptr;

	_song_index = sound;
	_timer_counter = 0;
	_loop_to_beat = 1;
	_loop_from_beat = 1;
	_track_index = 0;
	_loop_counter = 0;
	_loop_to_tick = 0;
	_loop_from_tick = 0;

	set_tempo(500000);
	set_speed(128);
	ptr = _se->findTag(sound, "MTrk", _track_index);
	if (ptr==NULL)
		return -1;
	
	track_ptr = ptr;
	_cur_pos = _next_pos = get_delta_time(&track_ptr);
	_song_offset = track_ptr - ptr;
	
	_tick_index = _cur_pos;
	_beat_index = 1;

	if (_tick_index >= _ticks_per_beat) {
		_beat_index += _tick_index/_ticks_per_beat;
		_tick_index %= _ticks_per_beat;
	}

	return 0;
}

void Player::set_tempo(uint32 b) {
	uint32 i,j;

        if (_se->_s->_gameTempo < 1000)
                i = TEMPO_BASE;
        else
                i = _se->_s->_gameTempo;

	j = _tempo = b;

	while (i&0xFFFF0000 || j&0xFFFF0000) { i>>=1; j>>=1; }

	_tempo_eff = (i<<16) / j;

	set_speed(_speed);
}

void Player::cancel_volume_fade() {
	VolumeFader *vf = _se->_volume_fader;
	int i;

	for (i=0; i<8; i++,vf++) {
		if (vf->active && vf->player==this)
			vf->active = false;
	}
}

void Player::uninit_parts() {
	if (_parts && _parts->_player != this)
		error("asd");
	while(_parts)
		_parts->uninit();
}

void Player::uninit_seq() {
	_abort = true;
}

void Player::set_speed(byte speed) {
	_speed = speed;
	_timer_speed = (_tempo_eff * speed >> 7);
}

byte *Player::parse_midi(byte *s) {
	byte cmd,chan,note,velocity,control;
	uint value;
	Part *part;
	
	cmd = *s++;

	chan = cmd&0xF;

	switch(cmd>>4) {
	case 0x8: /* key off */
		note = *s++;
		if (!_scanning) {
			key_off(chan, note);
		} else {
			clear_active_note(chan, note);
		}
		s++; /* skip velocity */
		break;

	case 0x9: /* key on */
		note = *s++;
		velocity = *s++;
		if (velocity) {
			if (!_scanning)
				key_on(chan, note,velocity);
			else
				set_active_note(chan,note);
		} else {
			if (!_scanning)
				key_off(chan, note);
			else
				clear_active_note(chan,note);
		}
		break;

	case 0xA: /* aftertouch */
		s += 2;
		break;

	case 0xB: /* control change */
		control = *s++;
		value = *s++;
		part = get_part(chan);
		if (!part)
			break;
		
		switch(control) {
		case 1:  /* modulation wheel */
			part->set_modwheel(value);
			break;
		case 7:  /* volume */
			part->set_vol(value);
			break;
		case 10: /* pan position */
			part->set_pan(value - 0x40);
			break;
		case 16: /* pitchbend factor */
			part->set_pitchbend_factor(value);
			break;
		case 17: /* gp slider 2 */
			part->set_detune(value - 0x40);
			break;
		case 18: /* gp slider 3 */
			part->set_pri(value - 0x40);
			_se->_driver->update_pris();
			break;
		case 64: /* hold pedal */
			part->set_pedal(value!=0);
			break;
		case 91: /* effects level */
			part->set_effect_level(value);
			break;
		case 93: /* chorus */
			part->set_chorus(value);
			break;
		default:
			warning("parse_midi: invalid control %d", control);
		}
		break;

	case 0xC: /* program change */
		value = *s++;
		part = get_part(chan);
		if (part)
			part->set_program(value);
		break;

	case 0xD: /* channel pressure */
		s++;
		break;

	case 0xE: /* pitch bend */
		part = get_part(chan);
		if (part)
			part->set_pitchbend(((s[1]-0x40)<<7)|s[0]);
		s+=2;
		break;

	case 0xF:
		if (chan==0) {
			uint size = get_delta_time(&s);
			if (*s==SYSEX_ID)
				parse_sysex(s,size);
			s += size;
		} else if (chan==0xF) {
			cmd = *s++;
			if (cmd==47)
				goto Error; /* end of song */
			if (cmd==81) {
				set_tempo((s[1]<<16) | (s[2]<<8) | s[3]);
				s+=4;
				break;
			}
			s += get_delta_time(&s);
		} else if (chan==0x7) {
			s += get_delta_time(&s);
		} else {
			goto Error;
		}
		break;

	default:
Error:;
		if(!_scanning)
			clear();
		return NULL;
	}
	return s;
}

void Player::parse_sysex(byte *p, uint len) {
	byte code;
	byte a;
	uint b;
	byte buf[128];
	Part *part;

	/* too big? */
	if (len>=sizeof(buf)*2)
		return;

	/* skip sysex manufacturer */
	p++;
	len -= 2;
	
	switch(code=*p++) {
	case 16: /* set instrument in part */
		a = *p++ & 0x0F;
		if (HARDWARE_TYPE != *p++)
			break;
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if(part)
			part->set_instrument((Instrument*)buf);
		break;

	case 17: /* set global instrument */
		p++;
		if (HARDWARE_TYPE != *p++)
			break;
		a=*p++;
		decode_sysex_bytes(p, buf, len - 4);
		_se->_driver->set_instrument(a, buf);
		break;

	case 33: /* param adjust */
		a = *p++ & 0x0F;
		if (HARDWARE_TYPE != *p++)
			break;
		decode_sysex_bytes(p, buf, len - 3);
		part = get_part(a);
		if (part)
			part->set_param(read_word(buf),read_word(buf+2));
		break;

	case 48: /* hook - jump */
		if (_scanning)
			break;
		decode_sysex_bytes(p+1,buf,len-2);
		maybe_jump(buf);
		break;

	case 49: /* hook - global transpose */
		decode_sysex_bytes(p+1,buf,len-2);
		maybe_set_transpose(buf);
		break;

	case 50: /* hook - part on/off */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p,buf+1,len-2);
		maybe_part_onoff(buf);
		break;

	case 51: /* hook - set volume */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p,buf+1,len-2);
		maybe_set_volume(buf);
		break;

	case 52: /* hook - set program */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p,buf+1,len-2);
		maybe_set_program(buf);
		break;
	
	case 53: /* hook - set transpose */
		buf[0] = *p++ & 0x0F;
		decode_sysex_bytes(p,buf+1,len-2);
		maybe_set_transpose_part(buf);
		break;

	case 64: /* marker */
		p++;
		len -= 2;
		while (len--) {
			_se->handle_marker(_id, *p++);
		}
		break;

	case 80: /* loop */
		decode_sysex_bytes(p+1,buf,len-2);
		set_loop( 
			read_word(buf), 
			read_word(buf+2), 
			read_word(buf+4),
			read_word(buf+6),
			read_word(buf+8)
		);
		break;

	case 81: /* end loop */
		clear_loop();
		break;

	case 96: /* set instrument */
		part = get_part(p[0] & 0x0F);
		b = (p[1]&0x0F)<<12 | (p[2]&0x0F)<<8 | (p[4]&0x0F)<<4 | (p[4]&0x0F);
		if(part)
			part->set_instrument(b);
		break;

	default:
		debug(6,"unknown sysex %d", code);
	}
}

void Player::decode_sysex_bytes(byte *src, byte *dst, int len) {
	while(len>=0) {
		*dst++ = (src[0]<<4)|(src[1]&0xF);
		src += 2;
		len -= 2;
	}
}

void Player::maybe_jump(byte *data) {
	byte cmd;

	cmd = data[0];

	/* is this the hook i'm waiting for? */
	if (cmd && _hook._jump!=cmd)
		return;

	/* reset hook? */
	if(cmd!=0 && cmd<0x80)
		_hook._jump = 0;

	jump(read_word(data+1), read_word(data+3), read_word(data+5));
}

void Player::maybe_set_transpose(byte *data) {
	byte cmd;

	cmd = data[0];

	/* is this the hook i'm waiting for? */
	if (cmd && _hook._transpose!=cmd)
		return;

	/* reset hook? */
	if(cmd!=0 && cmd<0x80)
		_hook._transpose = 0;

	set_transpose(data[1], (int8)data[2]);
}

void Player::maybe_part_onoff(byte *data) {
	byte cmd,*p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_onoff[chan];

	/* is this the hook i'm waiting for? */
	if (cmd && *p!=cmd)
		return;

	if (cmd!=0 && cmd<0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_onoff(data[2]!=0);
}

void Player::maybe_set_volume(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	p = &_hook._part_volume[chan];

	/* is this the hook i'm waiting for? */
	if (cmd && *p!=cmd)
		return;

	/* reset hook? */
	if (cmd!=0 && cmd<0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_vol(data[2]);
}

void Player::maybe_set_program(byte *data) {
	byte cmd;
	byte *p;
	uint chan;
	Part *part;

	cmd = data[1];
	chan = data[0];

	/* is this the hook i'm waiting for? */
	p = &_hook._part_program[chan];

	if (cmd && *p!=cmd)
		return;

	if (cmd!=0 && cmd<0x80)
		*p = 0;

	part = get_part(chan);
	if (part)
		part->set_program(data[2]);
}

void Player::maybe_set_transpose_part(byte *data) {
	byte cmd;
	byte *p;
	uint chan;

	cmd = data[1];
	chan = data[0];

	/* is this the hook i'm waiting for? */
	p = &_hook._part_transpose[chan];

	if (cmd && *p!=cmd)
		return;

	/* reset hook? */
	if (cmd!=0 && cmd<0x80)
		*p = 0;

	part_set_transpose(chan, data[2], (int8)data[3]);
}

int Player::set_transpose(byte relative, int b) {
	Part *part;

	if (b>24 || b<-24 || relative>1)
		return -1;
	if (relative)
		b = transpose_clamp(_transpose + b, -7, 7);

	_transpose = b;

	for(part=_parts; part; part=part->_next) {
		part->set_transpose(part->_transpose);
	}

	return 0;
}

void Player::clear_active_notes() {
	memset(_se->_active_notes, 0, sizeof(_se->_active_notes));
}

void Player::clear_active_note(int chan, byte note) {
	_se->_active_notes[note] &= ~(1<<chan);
}

void Player::set_active_note(int chan, byte note) {
	_se->_active_notes[note] |= (1<<chan);
}

void Player::part_set_transpose(uint8 chan, byte relative, int8 b) {
	Part *part;

	if (b>24 || b<-24)
		return;

	part = get_part(chan);
	if (!part)
		return;
	if (relative)
		b = transpose_clamp(b + part->_transpose, -7, 7);
	part->set_transpose(b);
}

void Player::key_on(uint8 chan, uint8 note, uint8 velocity) {
	Part *part;

	part = get_part(chan);
	if (!part || !part->_on)
		return;

	part->key_on(note, velocity);
}

void Player::key_off(uint8 chan, uint8 note) {
	Part *part;

	for(part=_parts; part; part = part->_next) {
		if (part->_chan==(byte)chan && part->_on)
			part->key_off(note);
	}
}

bool Player::jump(uint track, uint beat, uint tick) {
	byte *mtrk, *cur_mtrk, *scanpos;
	uint32 topos,curpos,track_offs;

	if (!_active)
		return false;

	mtrk = _se->findTag(_song_index, "MTrk", track);
	if (!mtrk)
		return false;

	cur_mtrk = _se->findTag(_song_index,"MTrk", _track_index);
	if (!cur_mtrk)
		return false;

	_se->lock();

	if (beat==0)
		beat=1;

	topos = (beat-1) * _ticks_per_beat + tick;

	if (track == _track_index && topos >= _cur_pos) {
		scanpos = _song_offset + mtrk;
		curpos = _next_pos;
	} else {
		scanpos = mtrk;
		curpos = get_delta_time(&scanpos);
	}

	while (curpos < topos) {
		skip_midi_cmd(&scanpos);
		if (!scanpos) {
			_se->unlock();
			return false;
		}
		curpos += get_delta_time(&scanpos);
	}

	track_offs = scanpos - mtrk;

	turn_off_pedals();

	find_sustaining_notes(cur_mtrk+_song_offset, mtrk+track_offs, curpos-topos);

	_beat_index = beat;
	_tick_index = tick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = track_offs;
	if (track != _track_index) {
		_track_index = track;
		_loop_counter = 0;
	}
	_abort = true;
	_se->unlock();
	return true;
}

bool Player::set_loop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick) {
	if (tobeat+1 >= frombeat)
		return false;

	if (tobeat==0)
		tobeat=1;

	_loop_counter = 0; /* because of possible interrupts */
	_loop_to_beat = tobeat;
	_loop_to_tick = totick;
	_loop_from_beat = frombeat;
	_loop_from_tick = fromtick;
	_loop_counter = count;

	return true;
}

void Player::clear_loop() {
	_loop_counter = 0;
}

void Player::turn_off_pedals() {
	Part *part;

	for(part=_parts; part; part = part->_next) {
		if (part->_pedal)
			part->set_pedal(false);
	}
}

void Player::find_sustaining_notes(byte *a, byte *b, uint32 l) {
	uint32 pos;
	uint16 mask;
	uint16 *bitlist_ptr;
	SustainingNotes *sn,*next;
	IsNoteCmdData isnote;
	int j;
	uint num_active;
	uint max_off_pos;

	num_active = update_actives();

	/* pos contains number of ticks since current position */
	pos = _next_pos - _cur_pos;
	if ((int32)pos<0)
		pos = 0;

	/* locate the positions where the notes are turned off.
	 * remember each note that was turned off
	 */
	while (num_active != 0) {
		/* is note off? */
		j = is_note_cmd(&a,&isnote);
		if (j==-1)
			break;
		if (j == 1) {
			mask = 1<<isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;
			if (*bitlist_ptr & mask) {
				*bitlist_ptr &= ~mask;
				num_active--;
				/* Get a node from the free list */
				if ((sn=_se->_sustain_notes_free) == NULL)
					return;
				_se->_sustain_notes_free = sn->next;
				
				/* Insert it in the beginning of the used list */
				sn->next = _se->_sustain_notes_used;
				_se->_sustain_notes_used = sn;
				sn->prev = NULL;
				if (sn->next)
					sn->next->prev = sn;

				sn->note = isnote.note;
				sn->chan = isnote.chan;
				sn->player = this;
				sn->off_pos = pos;
				sn->pos = 0;
				sn->counter = 0;
			}
		}
		pos += get_delta_time(&a);
	}

	/* find the maximum position where a note was turned off */
	max_off_pos = 0;
	for(sn=_se->_sustain_notes_used; sn; sn = sn->next) {
		_se->_active_notes[sn->note] |= (1<<sn->chan);
		if (sn->off_pos > max_off_pos) {
			max_off_pos = sn->off_pos;
		}
	}

	/* locate positions where notes are turned on */
	pos = l;
	while (pos < max_off_pos) {
		j = is_note_cmd(&b,&isnote);
		if (j==-1)
			break;
		if (j == 2) {
			mask = 1<<isnote.chan;
			bitlist_ptr = _se->_active_notes + isnote.note;

			if (*bitlist_ptr&mask) {
				sn = _se->_sustain_notes_used;
				while (sn) {
					next = sn->next;
					if (sn->note==isnote.note && sn->chan==isnote.chan && pos < sn->off_pos) {
						*bitlist_ptr &= ~mask;
						/* Unlink from the sustain list */
						if (next)
							next->prev = sn->prev;
						if (sn->prev)
							sn->prev->next = next;
						else
							_se->_sustain_notes_used = next;
						/* Insert into the free list */
						sn->next = _se->_sustain_notes_free;
						_se->_sustain_notes_free = sn;
					}
					sn = next;
				}
			}
		}
		pos += get_delta_time(&b);
	}

	/* Concatenate head and used list */
	if (!_se->_sustain_notes_head) {
		_se->_sustain_notes_head = _se->_sustain_notes_used;
		_se->_sustain_notes_used = NULL;
		return;
	}
	sn = _se->_sustain_notes_head;
	while (sn->next) sn = sn->next;
	sn->next = _se->_sustain_notes_used;
	_se->_sustain_notes_used = NULL;
	if (sn->next)
		sn->next->prev = sn;
}

Part *Player::get_part(uint8 chan) {
	Part *part;

	part = _parts;
	while (part) {
		if (part->_chan == chan)
			return part;
		part = part->_next;
	}

	part = _se->allocate_part(_priority);
	if (!part) {
		warning("no parts available");
		return NULL;
	}
	
	part->_chan = chan;
	part->setup(this);

	return part;
}

uint Player::update_actives() {
	Part *part;
	uint16 *active;
	int count = 0;
	
	clear_active_notes();	
	active = _se->_active_notes;
	for(part=_parts; part; part = part->_next) {
		if (part->_mc)
			count += part->update_actives(active);
	}
	return count;
}

void Player::set_priority(int pri) {
	Part *part;

	_priority = pri;
	for(part=_parts; part; part = part->_next) {
		part->set_pri(part->_pri);
	}
	_se->_driver->update_pris();
}

void Player::set_pan(int pan) {
	Part *part;

	_pan = pan;
	for(part=_parts; part; part = part->_next) {
		part->set_pan(part->_pan);
	}
}

void Player::set_detune(int detune) {
	Part *part;

	_detune = detune;
	for(part=_parts; part; part = part->_next) {
		part->set_detune(part->_detune);
	}
}

int Player::scan(uint totrack, uint tobeat, uint totick) {
	byte *mtrk,*scanptr;
	uint32 curpos,topos;
	uint32 pos;

	assert(totrack>=0 && tobeat>=0 && totick>=0);

	if (!_active)
		return -1;

	mtrk = _se->findTag(_song_index, "MTrk", totrack);
	if (!mtrk)
		return -1;

	_se->lock();
	if (tobeat==0)
		tobeat++;

	turn_off_parts();
	clear_active_notes();
	scanptr = mtrk;
	curpos = get_delta_time(&scanptr);
	_scanning=true;

	topos = (tobeat-1) * _ticks_per_beat + totick;

	while (curpos < topos) {
		scanptr = parse_midi(scanptr);
		if (!scanptr) {
			_scanning=false;
			_se->unlock();
			return -1;
		}
		curpos += get_delta_time(&scanptr);
	}
	pos = scanptr - mtrk;

	_scanning=false;
	_se->driver()->update_pris();
	play_active_notes();
	_beat_index = tobeat;
	_tick_index = totick;
	_cur_pos = topos;
	_next_pos = curpos;
	_timer_counter = 0;
	_song_offset = pos;
	if (_track_index != totrack) {
		_track_index = totrack;
		_loop_counter = 0;
	}
	_se->unlock();
	return 0;
}

void Player::turn_off_parts() {
	Part *part;

	for(part=_parts; part; part = part->_next)
		part->off();
}

void Player::play_active_notes() {
	int i,j;
	uint mask;

	for (i=0; i!=128; i++) {
		mask = _se->_active_notes[i];
		for (j=0; j!=16; j++,mask>>=1) {
			if (mask&1) {
				key_on(j, i, 80);
			}
		}
	}
}

int Player::set_vol(byte vol) {
	Part *part;

	if (vol > 127)
		return -1;

	_volume = vol;
	_vol_eff = _se->get_channel_volume(_vol_chan) * (vol + 1) >> 7;

	for(part=_parts; part; part=part->_next) {
		part->set_vol(part->_vol);
	}

	return 0;
}

int Player::get_param(int param, byte chan) {
	switch(param) {
	case 0:
		return (byte)_priority;
	case 1:
		return (byte)_volume;
	case 2:
		return (byte)_pan;
	case 3:
		return (byte)_transpose;
	case 4:
		return (byte)_detune;
	case 5:
		return _speed;
	case 6:
		return _track_index;
	case 7:
		return _beat_index;
	case 8:
		return _tick_index;
	case 9:
		return _loop_counter;
	case 10:
		return _loop_to_beat;
	case 11:
		return _loop_to_tick;
	case 12:
		return _loop_from_beat;
	case 13:
		return _loop_from_tick;
	case 14: case 15: case 16: case 17:
		return query_part_param(param, chan);
	case 18: case 19: case 20: case 21: case 22: case 23:
		return _hook.query_param(param, chan);
	default:
		return -1;
	}
}

int Player::query_part_param(int param, byte chan) {
	Part *part;

	part = _parts;
	while (part) {
		if(part->_chan==chan) {
			switch(param) {
			case 14:
				return part->_on;
			case 15:
				return part->_vol;
			case 16:
				return part->_program;
			case 17:
				return part->_transpose;
			default:
				return -1;
			}
		}
		part = part->_next;
	}
	return 129;
}

/*******************************************************************/

#define OFFS(type,item) ((int)(&((type*)0)->item))
#define SIZE(type,item) sizeof(((type*)0)->item)
#define MKLINE(type,item,saveas) {OFFS(type,item),saveas,SIZE(type,item)}
#define MKARRAY(type,item,saveas,num) {OFFS(type,item),128|saveas,SIZE(type,item)}, {num,0,0}
#define MKEND() {0xFFFF,0xFF,0xFF}

#define MKREF(type,item,refid) {OFFS(type,item),refid,0xFF}

enum {
	TYPE_PART = 1,
	TYPE_PLAYER = 2,
};

int SoundEngine::saveReference(SoundEngine *me, byte type, void*ref) {
	switch(type) {
	case TYPE_PART: return (Part*)ref - me->_parts;
	case TYPE_PLAYER: return (Player*)ref - me->_players;
	default:
		error("saveReference: invalid type");
	}
}

void *SoundEngine::loadReference(SoundEngine *me, byte type, int ref) {
	switch(type) {
	case TYPE_PART: return &me->_parts[ref];
	case TYPE_PLAYER: return &me->_players[ref];
	default:
		error("loadReference: invalid type");
	}
}

int SoundEngine::save_or_load(Serializer *ser) {
	const SaveLoadEntry mainEntries[] = {
		MKLINE(SoundEngine,_queue_end, sleUint8),
		MKLINE(SoundEngine,_queue_pos, sleUint8),
		MKLINE(SoundEngine,_queue_sound, sleUint16),
		MKLINE(SoundEngine,_queue_adding, sleByte),
		MKLINE(SoundEngine,_queue_marker, sleByte),
		MKLINE(SoundEngine,_queue_cleared, sleByte),
		MKLINE(SoundEngine,_master_volume, sleByte),
		MKLINE(SoundEngine,_trigger_count, sleUint16),
		MKARRAY(SoundEngine,_channel_volume[0], sleUint16, 8),
		MKARRAY(SoundEngine,_volchan_table[0], sleUint16, 8),
		MKEND()
	};
	
	const SaveLoadEntry playerEntries[] = {
		MKREF(Player,_parts,TYPE_PART),
		MKLINE(Player,_active,sleByte),
		MKLINE(Player,_id,sleUint16),
		MKLINE(Player,_priority,sleByte),
		MKLINE(Player,_volume,sleByte),
		MKLINE(Player,_pan,sleInt8),
		MKLINE(Player,_transpose,sleByte),
		MKLINE(Player,_detune,sleInt8),
		MKLINE(Player,_vol_chan,sleUint16),
		MKLINE(Player,_vol_eff,sleByte),
		MKLINE(Player,_speed,sleByte),
		MKLINE(Player,_song_index,sleUint16),
		MKLINE(Player,_track_index,sleUint16),
		MKLINE(Player,_timer_counter,sleUint16),
		MKLINE(Player,_loop_to_beat,sleUint16),
		MKLINE(Player,_loop_from_beat,sleUint16),
		MKLINE(Player,_loop_counter,sleUint16),
		MKLINE(Player,_loop_to_tick,sleUint16),
		MKLINE(Player,_loop_from_tick,sleUint16),
		MKLINE(Player,_tempo,sleUint32),
		MKLINE(Player,_cur_pos,sleUint32),
		MKLINE(Player,_next_pos,sleUint32),
		MKLINE(Player,_song_offset,sleUint32),
		MKLINE(Player,_tick_index,sleUint16),
		MKLINE(Player,_beat_index,sleUint16),
		MKLINE(Player,_ticks_per_beat,sleUint16),
		MKLINE(Player,_hook._jump,sleByte),
		MKLINE(Player,_hook._transpose,sleByte),
		MKARRAY(Player,_hook._part_onoff[0],sleByte,16),
		MKARRAY(Player,_hook._part_volume[0],sleByte,16),
		MKARRAY(Player,_hook._part_program[0],sleByte,16),
		MKARRAY(Player,_hook._part_transpose[0],sleByte,16),
		MKEND()
	};

	const SaveLoadEntry volumeFaderEntries[] = {
		MKREF(VolumeFader,player,TYPE_PLAYER),
		MKLINE(VolumeFader,active,sleUint8),
		MKLINE(VolumeFader,curvol,sleUint8),
		MKLINE(VolumeFader,speed_lo_max,sleUint16),
		MKLINE(VolumeFader,num_steps,sleUint16),
		MKLINE(VolumeFader,speed_hi,sleInt8),
		MKLINE(VolumeFader,direction,sleInt8),
		MKLINE(VolumeFader,speed_lo,sleInt8),
		MKLINE(VolumeFader,speed_lo_counter,sleUint16),
		MKEND()
	};

	const SaveLoadEntry partEntries[] = {
		MKREF(Part,_next,TYPE_PART),
		MKREF(Part,_prev,TYPE_PART),
		MKREF(Part,_player,TYPE_PLAYER),
		MKLINE(Part,_pitchbend,sleInt16),
		MKLINE(Part,_pitchbend_factor,sleUint8),
		MKLINE(Part,_transpose,sleInt8),
		MKLINE(Part,_vol,sleUint8),
		MKLINE(Part,_detune,sleInt8),
		MKLINE(Part,_pan,sleInt8),
		MKLINE(Part,_on,sleUint8),
		MKLINE(Part,_modwheel,sleUint8),
		MKLINE(Part,_pedal,sleUint8),
		MKLINE(Part,_program,sleUint8),
		MKLINE(Part,_pri,sleUint8),
		MKLINE(Part,_chan,sleUint8),
		MKLINE(Part,_effect_level,sleUint8),
		MKLINE(Part,_chorus,sleUint8),
		MKLINE(Part,_percussion,sleUint8),
		MKLINE(Part,_bank,sleUint8),
		MKEND()
	};

	if (!ser->isSaving()) {
		stop_all_sounds();
	}

	ser->_ref_me = this;
	ser->_saveload_ref = ser->isSaving() ? ((void*)&saveReference) : ((void*)&loadReference);

	ser->saveLoadEntries(this, mainEntries);
	ser->saveLoadArrayOf(_players, ARRAYSIZE(_players), sizeof(_players[0]), playerEntries);
	ser->saveLoadArrayOf(_parts, ARRAYSIZE(_parts),sizeof(_parts[0]), partEntries);
	ser->saveLoadArrayOf(_volume_fader,ARRAYSIZE(_volume_fader),
		sizeof(_volume_fader[0]), volumeFaderEntries);

	if (!ser->isSaving()) {
		/* Load all sounds that we need */
		fix_players_after_load();
		init_sustaining_notes();
		_active_volume_faders = true;
		fix_parts_after_load();
		_driver->update_pris();
	}

	return 0;
}

#undef MKLINE
#undef MKEND

void SoundEngine::fix_parts_after_load() {
	Part *part;
	int i;

	for (i=ARRAYSIZE(_parts),part=_parts; i!=0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

/* Only call this routine from the main thread,
 * since it uses getResourceAddress */
void SoundEngine::fix_players_after_load() {
	Player *player = _players;
	int i;

	for (i=ARRAYSIZE(_players); i!=0; i--, player++) {
		if (player->_active) {
			player->set_tempo(player->_tempo);
			_s->getResourceAddress(rtSound, player->_id);
		}
	}
}

void Part::set_detune(int8 detune) {
	_detune_eff = clamp((_detune=detune) + _player->_detune, -128, 127);
	changed(SoundDriver::pcMod);
}

void Part::set_pitchbend(int value) {
	_pitchbend = value * _pitchbend_factor >> 6;
	changed(SoundDriver::pcMod);
}

void Part::set_vol(uint8 vol) {
	_vol_eff = ((_vol=vol)+1)*_player->_vol_eff >> 7;
	changed(SoundDriver::pcVolume);
}

void Part::set_pri(int8 pri) {
	_pri_eff = clamp((_pri=pri) + _player->_priority, 0, 255);
}

void Part::set_pan(int8 pan) {
	_pan_eff = clamp((_pan=pan) + _player->_pan, -64, 63);
	changed(SoundDriver::pcPan);
}

void Part::set_transpose(int8 transpose) {
	_transpose_eff = transpose_clamp((_transpose=transpose) +
			_player->_transpose, -12, 12);
	changed(SoundDriver::pcMod);
}

void Part::set_pedal(bool value) {
	_pedal = value;
	changed(SoundDriver::pcPedal);
}

void Part::set_modwheel(uint value) {
	_modwheel = value;
	changed(SoundDriver::pcModwheel);
}

void Part::set_chorus(uint chorus) {
	_chorus = chorus;
	changed(SoundDriver::pcChorus);
}

void Part::set_effect_level(uint level) {
	_effect_level = level;
	changed(SoundDriver::pcEffectLevel);
}

void Part::fix_after_load() {
	set_transpose(_transpose);
	set_vol(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);
}

void Part::set_pitchbend_factor(uint8 value) {
	if (value > 12)
		return;
	set_pitchbend(0);
	_pitchbend_factor = value;
}

void Part::set_onoff(bool on) {
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			update_pris();
	}
}

void Part::set_instrument(Instrument *data) {
	_drv->part_set_instrument(this, data);
}

void Part::key_on(byte note, byte velocity) {
	_drv->part_key_on(this, note, velocity);
}

void Part::key_off(byte note) {
	_drv->part_key_off(this, note);
}

void Part::init(SoundDriver *driver) {
	_drv = (SOUND_DRIVER_TYPE*)driver;
	_player = NULL;
	_next = NULL;
	_prev = NULL;
	_mc = NULL;
}

void Part::setup(Player *player) {
	_player = player;
	
	/* Insert first into player's list */
	_prev = NULL;
		_next = player->_parts;
	if (player->_parts)
		player->_parts->_prev = this;
	player->_parts = this;

	_percussion = true;
	_on = true;
	_pri_eff = player->_priority;
	_pri = 0;
	_vol = 127;
	_vol_eff = player->_vol_eff;
	_pan = clamp(player->_pan, -64, 63);
	_transpose_eff = player->_transpose;
	_transpose = 0;
	_detune = 0;
	_detune_eff = player->_detune;
	_pitchbend_factor = 2;
	_pitchbend = 0;
	_effect_level = 64;
	_program = 255;
	_chorus = 0;
	_modwheel = 0;
	_bank = 0;
	_pedal = false;
	_mc = NULL;
}

void Part::uninit() {
	if (!_player)
		return;
	off();
	
	/* unlink */
	if (_next)
		_next->_prev = _prev;
	if (_prev)
		_prev->_next = _next;
	else
		_player->_parts = _next;
	_player = NULL;
	_next = NULL;
	_prev = NULL;
}

void Part::off() {
	_drv->part_off(this);
}

void Part::changed(byte what) {
	_drv->part_changed(this, what);
}

void Part::set_param(byte param, int value) {
	_drv->part_set_param(this,param,value);
}

void Part::update_pris() {
	_drv->update_pris();
}

int Part::update_actives(uint16 *active) {
	return _drv->part_update_active(this, active);
}

void Part::set_program(byte program) {
	if (_program!=program || _bank!=0) {
		_program = program;
		_bank = 0;
		changed(SoundDriver::pcProgram);
	}
}

void Part::set_instrument(uint b) {
	_bank = (byte)(b>>8);
	_program = (byte)b;
	changed(SoundDriver::pcProgram);
}

