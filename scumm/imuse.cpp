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

#include "stdafx.h"

#include "base/gameDetector.h"	// For kDefaultMasterVolume etc.
#include "common/util.h"

#include "scumm/imuse.h"
#include "scumm/imuse_internal.h"
#include "scumm/instrument.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"

#include "sound/mididrv.h"



////////////////////////////////////////
//
//  IMuseInternal implementation
//
////////////////////////////////////////

IMuseInternal::IMuseInternal() :
_old_adlib_instruments(false),
_enable_multi_midi(false),
_midi_adlib(0),
_midi_native(0),
_base_sounds(0),
_paused(false),
_initialized(false),
_tempoFactor(0),
_player_limit(ARRAYSIZE(_players)),
_recycle_players(false),
_queue_end(0),
_queue_pos(0),
_queue_sound(0),
_queue_adding(0),
_queue_marker(0),
_queue_cleared(0),
_master_volume(0),
_music_volume(0),
_trigger_count(0),
_snm_trigger_index(0)
{
	memset(_channel_volume,0,sizeof(_channel_volume));
	memset(_channel_volume_eff,0,sizeof(_channel_volume_eff));
	memset(_volchan_table,0,sizeof(_volchan_table));
	memset(_active_notes,0,sizeof(_active_notes));
}

byte *IMuseInternal::findStartOfSound(int sound) {
	byte *ptr = NULL;
	int32 size, pos;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL) {
		debug(1, "IMuseInternal::findStartOfSound(): Sound %d doesn't exist!", sound);
		return NULL;
	}

	// Check for old-style headers first, like 'RO'
	if (ptr[4] == 'R' && ptr[5] == 'O'&& ptr[6] != 'L')
		return ptr + 4;
	if (ptr[8] == 'S' && ptr[9] == 'O')
		return ptr + 8;

	ptr += 8;
	size = READ_BE_UINT32(ptr);
	ptr += 4;

	// Okay, we're looking for one of those things: either
	// an 'MThd' tag(for SMF), or a 'FORM' tag(for XMIDI).
	size = 48; // Arbitrary; we should find our tag within the first 48 bytes of the resource
	pos = 0;
	while (pos < size) {
		if (!memcmp(ptr + pos, "MThd", 4) || !memcmp(ptr + pos, "FORM", 4))
			return ptr + pos;
		++pos; // We could probably iterate more intelligently
	}

	debug(3, "IMuseInternal::findStartOfSound(): Failed to align on sound %d!", sound);
	return 0;
}

bool IMuseInternal::isMT32(int sound) {
	byte *ptr = NULL;
	uint32 tag;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL)
		return false;

	tag = *(((uint32 *)ptr) + 1);
	switch (tag) {
	case MKID('ADL '):
	case MKID('ASFX'): // Special AD class for old Adlib sound effects
		return false;
	case MKID('AMI '):
		return true;
	case MKID('ROL '):
		return true;
	case MKID('GMD '):
		return false;
	case MKID('MAC '):
		return true;
	case MKID('SPK '):
		return false;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[4] == 'R' && ptr[5] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	if (ptr[8] == 'S' && ptr[9] == 'O')
		return false;

	return false;
}

bool IMuseInternal::isGM(int sound) {
	byte *ptr = NULL;
	uint32 tag;

	if (_base_sounds)
		ptr = _base_sounds[sound];

	if (ptr == NULL)
		return false;

	tag = *(((uint32 *)ptr) + 1);
	switch (tag) {
	case MKID('ADL '):
	case MKID('ASFX'): // Special AD class for old Adlib sound effects
		return false;
	case MKID('AMI '):
		return true; // Yeah... for our purposes, this is GM
	case MKID('ROL '):
		return true; // Yeah... for our purposes, this is GM
	case MKID('GMD '):
		return true;
	case MKID('MIDI'):
		return true;
	case MKID('MAC '):
		return true; // I guess this one too, since it qualifies under isMT32()
	case MKID('SPK '):
		return false;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[4] == 'R' && ptr[5] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	// FIXME: Right now we're pretending it's GM.
	if (ptr[8] == 'S' && ptr[9] == 'O')
		return true;

	return false;
}

MidiDriver *IMuseInternal::getBestMidiDriver(int sound) {
	MidiDriver *driver = NULL;

	if (isGM(sound)) {
		if (_midi_native) {
			driver = _midi_native;
		} else {
			// Route it through Adlib anyway.
			if (!_midi_adlib) {
				_midi_adlib = MidiDriver_ADLIB_create(_mixer);
				initMidiDriver(_midi_adlib);
			}
			driver = _midi_adlib;
		}
	} else {
		if (!_midi_adlib &&(_enable_multi_midi || !_midi_native)) {
			_midi_adlib = MidiDriver_ADLIB_create(_mixer);
			initMidiDriver(_midi_adlib);
		}
		driver = _midi_adlib;
	}
	return driver;
}

bool IMuseInternal::startSound(int sound) {
	Player *player;
	void *ptr;

	// Do not start a sound if it is already set to
	// start on an ImTrigger event. This fixes carnival
	// music problems where a sound has been set to trigger
	// at the right time, but then is started up immediately
	// anyway, only to be restarted later when the trigger
	// occurs.
	int i;
	ImTrigger *trigger = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trigger) {
		if (trigger->sound && trigger->id && trigger->command[0] == 8 && trigger->command[1] == sound)
			return false;
	}

	ptr = findStartOfSound(sound);
	if (!ptr) {
		debug(2, "IMuseInternal::startSound(): Couldn't find sound %d!", sound);
		return false;
	}
	
	// Check which MIDI driver this track should use.
	// If it's NULL, it ain't something we can play.
	MidiDriver *driver = getBestMidiDriver(sound);
	if (!driver)
		return false;

	// If the requested sound is already playing, start it over
	// from scratch. This was originally a hack to prevent Sam & Max
	// iMuse messiness while upgrading the iMuse engine, but it
	// is apparently necessary to deal with fade-and-restart
	// race conditions that were observed in MI2. Reference
	// Bug #590511 and Patch #607175(which was reversed to fix
	// an FOA regression: Bug #622606).
	player = findActivePlayer(sound);
	if (!player)
		player = allocate_player(128);
	if (!player)
		return false;

	player->clear();
	return player->startSound(sound, driver);
}


Player *IMuseInternal::allocate_player(byte priority) {
	Player *player = _players, *best = NULL;
	int i;
	byte bestpri = 255;

	for (i = _player_limit; i != 0; i--, player++) {
		if (!player->isActive())
			return player;
		if (player->getPriority() < bestpri) {
			best = player;
			bestpri = player->getPriority();
		}
	}

	if (bestpri < priority || _recycle_players)
		return best;

	debug(1, "Denying player request");
	return NULL;
}

void IMuseInternal::init_players() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		player->_se = this;
		player->clear(); // Used to just set _active to false
	}
}

void IMuseInternal::init_parts() {
	Part *part;
	int i;

	for (i = 0, part = _parts; i != ARRAYSIZE(_parts); i++, part++) {
		part->init();
		part->_slot = i;
	}
}

int IMuseInternal::stopSound(int sound) {
	int r = -1;
	Player *player = findActivePlayer(sound);
	if (player) {
		player->clear();
		r = 0;
	}
	return r;
}

int IMuseInternal::stopAllSounds() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive())
			player->clear();
	}
	return 0;
}

void IMuseInternal::on_timer(MidiDriver *midi) {
	if (_paused || !_initialized)
		return;

	if (midi == _midi_native || !_midi_native)
		handleDeferredCommands(midi);
	sequencer_timers(midi);
}

int IMuseInternal::getMusicTimer() {
	int best_time = 0;
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive()) {
			int timer = player->getMusicTimer();
			if (timer > best_time)
				best_time = timer;
		}
	}
	return best_time;
}

void IMuseInternal::sequencer_timers(MidiDriver *midi) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getMidiDriver() == midi) {
			player->onTimer();
		}
	}
}

void IMuseInternal::handle_marker(uint id, byte data) {
	uint16 *p = 0;
	uint pos;

	if (_queue_adding && _queue_sound == id && data == _queue_marker)
		return;

	// Fix for bug #733401, revised for bug #761637:
	// It would seem that sometimes a marker is in the queue
	// but not at the head position. In the case of our bug,
	// this seems to be the result of commands in the queue
	// for songs that are no longer playing. So we skip
	// ahead to the appropriate marker, effectively chomping
	// anything in the queue before it. This fixes the FOA
	// end credits music, but needs to be tested for inappopriate
	// behavior elsewhere.
	pos = _queue_end;
	while (pos != _queue_pos) {
		p = _cmd_queue[pos].array;
		if (p[0] == TRIGGER_ID && p[1] == id && p[2] == data)
			break;
		pos = (pos + 1) &(ARRAYSIZE(_cmd_queue) - 1);
	}

	if (pos == _queue_pos)
		return;

	if (pos != _queue_end)
		warning("Skipping entries in iMuse command queue to reach marker");

	_trigger_count--;
	_queue_cleared = false;
	do {
		pos = (pos + 1) &(ARRAYSIZE(_cmd_queue) - 1);
		if (_queue_pos == pos)
			break;
		p = _cmd_queue[pos].array;
		if (*p++ != COMMAND_ID)
			break;
		_queue_end = pos;

		doCommand(p[0], p[1], p[2], p[3], p[4], p[5], p[6], 0);

		if (_queue_cleared)
			return;
		pos = _queue_end;
	} while (1);

	_queue_end = pos;
}

int IMuseInternal::get_channel_volume(uint a) {
	if (a < 8)
		return _channel_volume_eff[a];
	return (_master_volume * _music_volume / 255) >> 1;
}

Part *IMuseInternal::allocate_part(byte pri, MidiDriver *midi) {
	Part *part, *best = NULL;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (!part->_player)
			return part;
		if (pri >= part->_pri_eff) {
			pri = part->_pri_eff;
			best = part;
		}
	}

	if (best) {
		best->uninit();
		reallocateMidiChannels(midi);
	} else {
		debug(1, "Denying part request");
	}
	return best;
}

int IMuseInternal::getSoundStatus(int sound, bool ignoreFadeouts) const {
	int i;
	const Player *player = _players;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && (!ignoreFadeouts || !player->isFadingOut())) {
			if (sound == -1)
				return player->getID();
			else if (player->getID() == (uint16)sound)
				return 1;
		}
	}
	return (sound == -1) ? 0 : get_queue_sound_status(sound);
}

int IMuseInternal::get_queue_sound_status(int sound) const {
	const uint16 *a;
	int i, j;

	j = _queue_pos;
	i = _queue_end;

	while (i != j) {
		a = _cmd_queue[i].array;
		if (a[0] == COMMAND_ID && a[1] == 8 && a[2] == (uint16)sound)
			return 2;
		i = (i + 1) &(ARRAYSIZE(_cmd_queue) - 1);
	}

	for (i = 0; i < ARRAYSIZE (_deferredCommands); ++i) {
		if (_deferredCommands[i].time_left && _deferredCommands[i].a == 8 &&
		    _deferredCommands[i].b == sound)
		{
			return 2;
		}
	}

	return 0;
}

int IMuseInternal::set_volchan(int sound, int volchan) {
	int r;
	int i;
	int num;
	Player *player, *best, *sameid;

	r = get_volchan_entry(volchan);
	if (r == -1)
		return -1;

	if (r >= 8) {
		player = findActivePlayer(sound);
		if (player && player->_vol_chan != (uint16)volchan) {
			player->_vol_chan = volchan;
			player->setVolume(player->getVolume());
			return 0;
		}
		return -1;
	} else {
		best = NULL;
		num = 0;
		sameid = NULL;
		for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
			if (player->isActive()) {
				if (player->_vol_chan == (uint16)volchan) {
					num++;
					if (!best || player->getPriority() <= best->getPriority())
						best = player;
				} else if (player->getID() == (uint16)sound) {
					sameid = player;
				}
			}
		}
		if (sameid == NULL)
			return -1;
		if (num >= r)
			best->clear();
		player->_vol_chan = volchan;
		player->setVolume(player->getVolume());
		return 0;
	}
}

int IMuseInternal::clear_queue() {
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int IMuseInternal::enqueue_command(int a, int b, int c, int d, int e, int f, int g) {
	uint16 *p;
	uint i;

	i = _queue_pos;

	if (i == _queue_end)
		return -1;

	if (a == -1) {
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

	i = (i + 1) &(ARRAYSIZE(_cmd_queue) - 1);

	if (_queue_end != i) {
		_queue_pos = i;
		return 0;
	} else {
		_queue_pos = (i - 1) &(ARRAYSIZE(_cmd_queue) - 1);
		return -1;
	}
}

int IMuseInternal::query_queue(int param) {
	switch (param) {
	case 0: // Get trigger count
		return _trigger_count;
	case 1: // Get trigger type
		if (_queue_end == _queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2: // Get trigger sound
		if (_queue_end == _queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int IMuseInternal::get_music_volume() {
	return _music_volume;
}

int IMuseInternal::set_music_volume(uint vol) {
	if (vol > 255)
		vol = 255;
	if (_music_volume == vol)
		return 0;
	_music_volume = vol;
	vol = vol * _master_volume / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::setMasterVolume(uint vol) {
	if (vol > 255)
		vol = 255;
	if (_master_volume == vol)
		return 0;
	_master_volume = vol;
	vol = vol * _music_volume / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::get_master_volume() {
	return _master_volume;
}

int IMuseInternal::terminate1() {
	_initialized = false;
	stopAllSounds();
	return 0;
}

// This is the stuff that has to be done
// outside the monitor's mutex, otherwise
// a deadlock occurs.
int IMuseInternal::terminate2() {
	if (_midi_adlib) {
		_midi_adlib->close();
		delete _midi_adlib;
		_midi_adlib = 0;
	}

	if (_midi_native) {
		_midi_native->close();
		delete _midi_native;
		_midi_native = 0;
	}

	return 0;
}

int IMuseInternal::enqueue_trigger(int sound, int marker) {
	uint16 *p;
	uint pos;

	pos = _queue_pos;

	p = _cmd_queue[pos].array;
	p[0] = TRIGGER_ID;
	p[1] = sound;
	p[2] = marker;

	pos = (pos + 1) &(ARRAYSIZE(_cmd_queue) - 1);
	if (_queue_end == pos) {
		_queue_pos = (pos - 1) &(ARRAYSIZE(_cmd_queue) - 1);
		return -1;
	}

	_queue_pos = pos;
	_queue_adding = true;
	_queue_sound = sound;
	_queue_marker = marker;
	return 0;
}

int32 IMuseInternal::doCommand(int a, int b, int c, int d, int e, int f, int g, int h) {
	int args[8];
	args[0] = a;
	args[1] = b;
	args[2] = c;
	args[3] = d;
	args[4] = e;
	args[5] = f;
	args[6] = g;
	args[7] = h;
	return doCommand (8, args);
}

int32 IMuseInternal::doCommand (int numargs, int a[]) {
	int i;

	if (numargs < 1) return -1;
	byte cmd = a[0] & 0xFF;
	byte param = a[0] >> 8;
	Player *player = NULL;

	if (!_initialized &&(cmd || param))
		return -1;

#ifdef IMUSE_DEBUG
	{
		char string[128];
		sprintf (string, "doCommand - %d (%d/%d)", a[0], (int) param, (int) cmd);
		for (i = 1; i < numargs; ++i)
			sprintf (string + strlen(string), ", %d", a[i]);
		debug (0, string);
	}
#endif

	if (param == 0) {
		switch (cmd) {
		case 6:
			if (a[1] > 127)
				return -1;
			else
				return setMasterVolume((a[1] << 1) |(a[1] ? 0 : 1)); // Convert from 0-127 to 0-255
		case 7:
			return _master_volume >> 1; // Convert from 0-255 to 0-127
		case 8:
			return startSound(a[1]) ? 0 : -1;
		case 9:
			return stopSound(a[1]);
		case 10: // FIXME: Sam and Max - Not sure if this is correct
			return stopAllSounds();
		case 11:
			return stopAllSounds();
		case 12:
			// Sam & Max: Player-scope commands
			player = findActivePlayer(a[1]);
			if (!player)
				return -1;

			switch (a[3]) {
			case 6:
				// Set player volume.
				return player->setVolume(a[4]);
			default:
				warning("IMuseInternal::doCommand(12) unsupported sub-command %d", a[3]);
			}
			return -1;
		case 13:
			return getSoundStatus(a[1]);
		case 14:
			// Sam and Max: Parameter fade
			player = this->findActivePlayer(a[1]);
			if (player)
				return player->addParameterFader(a[3], a[4], a[5]);
			return -1;

		case 15:
			// Sam & Max: Set hook for a "maybe" jump
			player = findActivePlayer(a[1]);
			if (player) {
				player->setHook(0, a[3], 0);
				return 0;
			}
			return -1;
		case 16:
			return set_volchan(a[1], a[2]);
		case 17:
			if (g_scumm->_gameId != GID_SAMNMAX) {
				return set_channel_volume(a[1], a[2]);
			} else {
				if (a[4]) {
					int b[16];
					memset (b, 0, sizeof(b));
					for (i = 0; i < numargs; ++i)
						b[i] = a[i];
					return ImSetTrigger (b[1], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11]);
				} else {
					return ImClearTrigger(a[1], a[3]);
				}
			}
		case 18:
			if (g_scumm->_gameId != GID_SAMNMAX) {
				return set_volchan_entry(a[1], a[2]);
			} else {
				// Sam & Max: ImCheckTrigger.
				// According to Mike's notes to Ender,
				// this function returns the number of triggers
				// associated with a particular player ID and
				// trigger ID.
				a[0] = 0;
				for (i = 0; i < 16; ++i) {
					if (_snm_triggers [i].sound == a[1] && _snm_triggers [i].id &&
					   (a[3] == -1 || _snm_triggers [i].id == a[3]))
					{
						++a[0];
					}
				}
				return a[0];
			}
		case 19:
			// Sam & Max: ImClearTrigger
			// This should clear a trigger that's been set up
			// with ImSetTrigger(cmd == 17). Seems to work....
			return ImClearTrigger(a[1], a[3]);
		case 20:
			// Sam & Max: Deferred Command
			addDeferredCommand(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return 0;
		case 2:
		case 3:
			return 0;
		default:
			warning("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		}
	} else if (param == 1) {
		if ((1 << cmd) &(0x783FFF)) {
			player = findActivePlayer(a[1]);
			if (!player)
				return -1;
			if ((1 << cmd) &(1 << 11 | 1 << 22)) {
				assert(a[2] >= 0 && a[2] <= 15);
				player = (Player *) player->getPart(a[2]);
				if (!player)
					return -1;
			}
		}

		switch (cmd) {
		case 0:
			if (g_scumm->_gameId == GID_SAMNMAX) {
				if (a[3] == 1) // Measure number
					return ((player->getBeatIndex() - 1) >> 2) + 1;
				else if (a[3] == 2) // Beat number
					return player->getBeatIndex();
				return -1;
			} else {
				return player->getParam(a[2], a[3]);
			}
		case 1:
			if (g_scumm->_gameId == GID_SAMNMAX)
				player->jump(a[3] - 1, (a[4] - 1) * 4 + a[5], ((a[6] * player->getTicksPerBeat()) >> 2) + a[7]);
			else
				player->setPriority(a[2]);
			return 0;
		case 2:
			return player->setVolume(a[2]);
		case 3:
			player->setPan(a[2]);
			return 0;
		case 4:
			return player->setTranspose(a[2], a[3]);
		case 5:
			player->setDetune(a[2]);
			return 0;
		case 6:
			player->setSpeed(a[2]);
			return 0;
		case 7:
			return player->jump(a[2], a[3], a[4]) ? 0 : -1;
		case 8:
			return player->scan(a[2], a[3], a[4]);
		case 9:
			return player->setLoop(a[2], a[3], a[4], a[5], a[6]) ? 0 : -1;
		case 10:
			player->clearLoop();
			return 0;
		case 11:
			((Part *)player)->set_onoff(a[3] != 0);
			return 0;
		case 12:
			return player->setHook(a[2], a[3], a[4]);
		case 13:
			return player->addParameterFader (ParameterFader::pfVolume, a[2], a[3]);
		case 14:
			return enqueue_trigger(a[1], a[2]);
		case 15:
			return enqueue_command(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		case 16:
			return clear_queue();
		case 19:
			return player->getParam(a[2], a[3]);
		case 20:
			return player->setHook(a[2], a[3], a[4]);
		case 21:
			return -1;
		case 22:
			((Part *)player)->setVolume(a[3]);
			return 0;
		case 23:
			return query_queue(a[1]);
		case 24:
			return 0;
		default:
			warning("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return -1;
		}
	}

	return -1;
}

int32 IMuseInternal::ImSetTrigger (int sound, int id, int a, int b, int c, int d, int e, int f, int g, int h) {
	// Sam & Max: ImSetTrigger.
	// Sets a trigger for a particular player and
	// marker ID, along with doCommand parameters
	// to invoke at the marker. The marker is
	// represented by MIDI SysEx block 00 xx(F7)
	// where "xx" is the marker ID.
	uint16 oldest_trigger = 0;
	ImTrigger *oldest_ptr = NULL;

	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if (!trig->id)
			break;
		if (trig->id == id && trig->sound == sound)
			break;

		uint16 diff;
		if (trig->expire <= _snm_trigger_index)
			diff = _snm_trigger_index - trig->expire;
		else
			diff = 0x10000 - trig->expire + _snm_trigger_index;

		if (!oldest_ptr || oldest_trigger < diff) {
			oldest_ptr = trig;
			oldest_trigger = diff;
		}
	}

	// If we didn't find a trigger, see if we can expire one.
	if (!i) {
		if (!oldest_ptr)
			return -1;
		trig = oldest_ptr;
	}

	trig->id = id;
	trig->sound = sound;
	trig->expire = (++_snm_trigger_index & 0xFFFF);
	trig->command [0] = a;
	trig->command [1] = b;
	trig->command [2] = c;
	trig->command [3] = d;
	trig->command [4] = e;
	trig->command [5] = f;
	trig->command [6] = g;
	trig->command [7] = h;

	// If the command is to start a sound, stop that sound if it's already playing.
	// This fixes some carnival music problems.
	// NOTE: We ONLY do this if the sound that will trigger the command is actually
	// playing. Otherwise, there's a problem when exiting and re-entering the
	// Bumpusville mansion. Ref Bug #780918.
	if (trig->command [0] == 8 && getSoundStatus(trig->command [1]) && getSoundStatus (sound))
		stopSound(trig->command [1]);
	return 0;
}

int32 IMuseInternal::ImClearTrigger(int sound, int id) {
	int count = 0;
	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if ((sound == -1 || trig->sound == sound) &&
		    trig->id && (id == -1 || trig->id == id))
		{
			trig->sound = trig->id = 0;
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int32 IMuseInternal::ImFireAllTriggers(int sound) {
	if (!sound) return 0;
	int count = 0;
	int i;
	for (i = 0; i < 16; ++i) {
		if (_snm_triggers [i].sound == sound)
		{
			_snm_triggers [i].sound = _snm_triggers [i].id = 0;
			doCommand (8, _snm_triggers[i].command);
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int IMuseInternal::set_channel_volume(uint chan, uint vol)
{
	if (chan >= 8 || vol > 127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * _music_volume * vol / 255 / 255;
	update_volumes();
	return 0;
}

void IMuseInternal::update_volumes() {
	Player *player;
	int i;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->isActive())
			player->setVolume(player->getVolume());
	}
}

int IMuseInternal::set_volchan_entry(uint a, uint b) {
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan) {
	switch (param) {
	case 18:
		return _jump[0];
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
	switch (cls) {
	case 0:
		if (value != _jump[0]) {
			_jump[1] = _jump[0];
			_jump[0] = value;
		}
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan < 16)
			_part_onoff[chan] = value;
		else if (chan == 16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan < 16)
			_part_volume[chan] = value;
		else if (chan == 16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan < 16)
			_part_program[chan] = value;
		else if (chan == 16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan < 16)
			_part_transpose[chan] = value;
		else if (chan == 16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}

Player *IMuseInternal::findActivePlayer(int id) {
	int i;
	Player *player = _players;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getID() == (uint16)id)
			return player;
	}
	return NULL;
}

int IMuseInternal::get_volchan_entry(uint a) {
	if (a < 8)
		return _volchan_table[a];
	return -1;
}

uint32 IMuseInternal::property(int prop, uint32 value) {
	switch (prop) {
	case IMuse::PROP_TEMPO_BASE:
		// This is a specified as a percentage of normal
		// music speed. The number must be an integer
		// ranging from 50 to 200(for 50% to 200% normal speed).
		if (value >= 50 && value <= 200)
			_tempoFactor = value;
		break;

	case IMuse::PROP_NATIVE_MT32:
		Instrument::nativeMT32(value > 0);
		break;

	case IMuse::PROP_MULTI_MIDI:
		_enable_multi_midi = (value > 0);
		if (!_enable_multi_midi && _midi_native && _midi_adlib) {
			MidiDriver *driver = _midi_adlib;
			_midi_adlib = NULL;
			int i;
			for (i = 0; i < ARRAYSIZE(_players); ++i) {
				if (_players[i].isActive() && _players[i].getMidiDriver() == driver)
					_players[i].clear();
			}
			driver->close();
			// FIXME: shouldn't we delete 'driver' here, too ?
		}
		break;

	case IMuse::PROP_OLD_ADLIB_INSTRUMENTS:
		_old_adlib_instruments = (value > 0);
		break;

	case IMuse::PROP_LIMIT_PLAYERS:
		if (value > 0 && value <= ARRAYSIZE(_players))
			_player_limit = (int) value;
		break;

	case IMuse::PROP_RECYCLE_PLAYERS:
		if (value > 0 && value <= ARRAYSIZE(_players))
			_recycle_players = (value != 0);
		break;
	}
	return 0;
}

void IMuseInternal::setBase(byte **base) {
	_base_sounds = base;
}

IMuseInternal *IMuseInternal::create(OSystem *syst, SoundMixer *mixer, MidiDriver *native_midi) {
	IMuseInternal *i = new IMuseInternal;
	i->initialize(syst, mixer, native_midi);
	return i;
}

int IMuseInternal::initialize(OSystem *syst, SoundMixer *mixer, MidiDriver *native_midi) {
	int i;

	_mixer = mixer;
	_midi_native = native_midi;
	_midi_adlib = NULL;
	if (native_midi)
		initMidiDriver(_midi_native);

	if (!_tempoFactor) _tempoFactor = 100;
	_master_volume = 255;
	if (_music_volume < 1)
		_music_volume = kDefaultMusicVolume;

	for (i = 0; i != 8; i++)
		_channel_volume[i] = _channel_volume_eff[i] = _volchan_table[i] = 127;

	init_players();
	init_queue();
	init_parts();

	_initialized = true;

	return 0;
}

void IMuseInternal::initMidiDriver(MidiDriver *midi) {
	// Open MIDI driver
	midi->property(MidiDriver::PROP_OLD_ADLIB, _old_adlib_instruments ? 1 : 0);

	int result = midi->open();
	if (result)
		error("IMuse initialization - %s", MidiDriver::getErrorName(result));

	// In case we have an MT-32 attached.
	initMT32(midi);

	// Connect to the driver's timer
	midi->setTimerCallback(midi, &IMuseInternal::midiTimerCallback);
}

void IMuseInternal::initMT32(MidiDriver *midi) {
	byte buffer[32] = "\x41\x10\x16\x12\x00\x00\x00                        ";
	char info[256] = "ScummVM ";
	int len;
	
	// Compute version string(truncated to 20 chars max.)
	strcat(info, gScummVMVersion);
	len = strlen(info);
	if (len > 20)
		len = 20;

	// Reset the MT-32
	memcpy(&buffer[4], "\x7f\x00\x00\x01\x00", 5);
	midi->sysEx(buffer, 9);

	// Display a welcome message on MT-32 displays.
	memcpy(&buffer[4], "\x20\x00\x00", 3);
	memcpy(&buffer[7], "                    ", 20);
	memcpy(buffer + 7 +(20 - len) / 2, info, len);
	byte checksum = 0;
	for (int i = 4; i < 27; ++i)
		checksum -= buffer[i];
	buffer[27] = checksum;
	midi->sysEx(buffer, 28);
}

void IMuseInternal::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void IMuseInternal::pause(bool paused) {
	int vol = _music_volume;
	if (paused)
		_music_volume = 0;
	update_volumes();
	_music_volume = vol;

	_paused = paused;
}

void IMuseInternal::handleDeferredCommands(MidiDriver *midi) {
	uint32 advance = midi->getBaseTempo();

	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			continue;
		if (ptr->time_left <= advance) {
			doCommand(ptr->a, ptr->b, ptr->c, ptr->d, ptr->e, ptr->f, 0, 0);
			ptr->time_left = advance;
		}
		ptr->time_left -= advance;
	}
}

// "time" is interpreted as hundredths of a second.
// FIXME: Is that correct?
// We convert it to microseconds before prceeding
void IMuseInternal::addDeferredCommand(int time, int a, int b, int c, int d, int e, int f) {
	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			break;
	}

	if (i) {
		ptr->time_left = time * 10000;
		ptr->a = a;
		ptr->b = b;
		ptr->c = c;
		ptr->d = d;
		ptr->e = e;
		ptr->f = f;
	}
}

////////////////////////////////////////////////////////////
//
// IMuseInternal load/save implementation
//
////////////////////////////////////////////////////////////

enum {
	TYPE_PART = 1,
	TYPE_PLAYER = 2
};

int IMuseInternal::saveReference(void *me_ref, byte type, void *ref) {
	IMuseInternal *me = (IMuseInternal *)me_ref;
	switch (type) {
	case TYPE_PART:
		return (Part *)ref - me->_parts;
	case TYPE_PLAYER:
		return (Player *)ref - me->_players;
	default:
		error("saveReference: invalid type");
	}
}

void *IMuseInternal::loadReference(void *me_ref, byte type, int ref) {
	IMuseInternal *me = (IMuseInternal *)me_ref;
	switch (type) {
	case TYPE_PART:
		return &me->_parts[ref];
	case TYPE_PLAYER:
		return &me->_players[ref];
	default:
		error("loadReference: invalid type");
	}
}

int IMuseInternal::save_or_load(Serializer *ser, Scumm *scumm) {
	const SaveLoadEntry mainEntries[] = {
		MKLINE(IMuseInternal, _queue_end, sleUint8, VER(8)),
		MKLINE(IMuseInternal, _queue_pos, sleUint8, VER(8)),
		MKLINE(IMuseInternal, _queue_sound, sleUint16, VER(8)),
		MKLINE(IMuseInternal, _queue_adding, sleByte, VER(8)),
		MKLINE(IMuseInternal, _queue_marker, sleByte, VER(8)),
		MKLINE(IMuseInternal, _queue_cleared, sleByte, VER(8)),
		MKLINE(IMuseInternal, _master_volume, sleByte, VER(8)),
		MKLINE(IMuseInternal, _trigger_count, sleUint16, VER(8)),
		MKARRAY(IMuseInternal, _channel_volume[0], sleUint16, 8, VER(8)),
		MKARRAY(IMuseInternal, _volchan_table[0], sleUint16, 8, VER(8)),
		MKEND()
	};

	const SaveLoadEntry cmdQueueEntries[] = {
		MKARRAY(CommandQueue, array[0], sleUint16, 8, VER(23)),
		MKEND()
	};

	// VolumeFader is obsolete.
	const SaveLoadEntry volumeFaderEntries[] = {
		MK_OBSOLETE_REF(VolumeFader, player, TYPE_PLAYER, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, active, sleUint8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, curvol, sleUint8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo_max, sleUint16, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, num_steps, sleUint16, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_hi, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, direction, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo_counter, sleUint16, VER(8), VER(16)),
		MKEND()
	};

	const SaveLoadEntry partEntries[] = {
		MKREF(Part, _next, TYPE_PART, VER(8)),
		MKREF(Part, _prev, TYPE_PART, VER(8)),
		MKREF(Part, _player, TYPE_PLAYER, VER(8)),
		MKLINE(Part, _pitchbend, sleInt16, VER(8)),
		MKLINE(Part, _pitchbend_factor, sleUint8, VER(8)),
		MKLINE(Part, _transpose, sleInt8, VER(8)),
		MKLINE(Part, _vol, sleUint8, VER(8)),
		MKLINE(Part, _detune, sleInt8, VER(8)),
		MKLINE(Part, _pan, sleInt8, VER(8)),
		MKLINE(Part, _on, sleUint8, VER(8)),
		MKLINE(Part, _modwheel, sleUint8, VER(8)),
		MKLINE(Part, _pedal, sleUint8, VER(8)),
		MK_OBSOLETE(Part, _program, sleUint8, VER(8), VER(16)),
		MKLINE(Part, _pri, sleUint8, VER(8)),
		MKLINE(Part, _chan, sleUint8, VER(8)),
		MKLINE(Part, _effect_level, sleUint8, VER(8)),
		MKLINE(Part, _chorus, sleUint8, VER(8)),
		MKLINE(Part, _percussion, sleUint8, VER(8)),
		MKLINE(Part, _bank, sleUint8, VER(8)),
		MKEND()
	};

	int i;

	ser->_ref_me = this;
	ser->_save_ref = saveReference;
	ser->_load_ref = loadReference;

	ser->saveLoadEntries(this, mainEntries);
	ser->saveLoadArrayOf (_cmd_queue, ARRAYSIZE(_cmd_queue), sizeof(_cmd_queue[0]), cmdQueueEntries);

	// The players
	for (i = 0; i < ARRAYSIZE(_players); ++i)
		_players[i].save_or_load(ser);
	ser->saveLoadArrayOf(_parts, ARRAYSIZE(_parts), sizeof(_parts[0]), partEntries);

	{ // Load/save the instrument definitions, which were revamped with V11.
		Part *part = &_parts[0];
		if (ser->getVersion() >= VER(11)) {
			for (i = ARRAYSIZE(_parts); i; --i, ++part) {
				part->_instrument.saveOrLoad(ser);
			}
		} else {
			for (i = ARRAYSIZE(_parts); i; --i, ++part)
				part->_instrument.clear();
		}
	}

	// VolumeFader has been replaced with the more generic ParameterFader.
	for (i = 0; i < 8; ++i)
		ser->saveLoadEntries(0, volumeFaderEntries);

	if (!ser->isSaving()) {
		// Load all sounds that we need
		fix_players_after_load(scumm);
		fix_parts_after_load();
		setMasterVolume(_master_volume);

		if (_midi_native)
			reallocateMidiChannels(_midi_native);
		if (_midi_adlib)
			reallocateMidiChannels(_midi_adlib);
	}

	return 0;
}

#undef MKLINE
#undef MKEND

void IMuseInternal::fix_parts_after_load() {
	Part *part;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

// Only call this routine from the main thread,
// since it uses getResourceAddress
void IMuseInternal::fix_players_after_load(Scumm *scumm) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive()) {
			scumm->getResourceAddress(rtSound, player->getID());
			player->fixAfterLoad();
		}
	}
}

void Part::set_detune(int8 detune) {
	_detune_eff = clamp((_detune = detune) + _player->getDetune(), -128, 127);
	if (_mc) {
		_mc->pitchBend(clamp(_pitchbend +
						(_detune_eff * 64 / 12) +
						(_transpose_eff * 8192 / 12), -8192, 8191));
	}
}

void Part::set_pitchbend(int value) {
	_pitchbend = value;
	if (_mc) {
		_mc->pitchBend(clamp(_pitchbend +
						(_detune_eff * 64 / 12) +
						(_transpose_eff * 8192 / 12), -8192, 8191));
	}
}

void Part::setVolume(uint8 vol) {
	_vol_eff = ((_vol = vol) + 1) * _player->getEffectiveVolume() >> 7;
	if (_mc)
		_mc->volume(_vol_eff);
}

void Part::set_pri(int8 pri) {
	_pri_eff = clamp((_pri = pri) + _player->getPriority(), 0, 255);
	if (_mc)
		_mc->priority(_pri_eff);
}

void Part::set_pan(int8 pan) {
	_pan_eff = clamp((_pan = pan) + _player->getPan(), -64, 63);
	if (_mc)
		_mc->panPosition(_pan_eff + 0x40);
}

void Part::set_transpose(int8 transpose) {
	_transpose_eff = transpose_clamp((_transpose = transpose) + _player->getTranspose(), -24, 24);
	if (_mc) {
		_mc->pitchBend(clamp(_pitchbend +
						(_detune_eff * /*64*/82 / _pitchbend_factor) +
						(_transpose_eff * 8192 / _pitchbend_factor), -8192, 8191));
	}
}

void Part::set_pedal(bool value) {
	_pedal = value;
	if (_mc)
		_mc->sustain(_pedal);
}

void Part::set_modwheel(uint value) {
	_modwheel = value;
	if (_mc)
		_mc->modulationWheel(_modwheel);
}

void Part::set_chorus(uint chorus) {
	_chorus = chorus;
	if (_mc)
		_mc->chorusLevel(_effect_level);
}

void Part::set_effect_level(uint level)
{
	_effect_level = level;
	if (_mc)
		_mc->effectLevel(_effect_level);
}

void Part::fix_after_load() {
	set_transpose(_transpose);
	setVolume(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);
	sendAll();
}

void Part::set_pitchbend_factor(uint8 value) {
	if (value > 12)
		return;
	set_pitchbend(0);
	_pitchbend_factor = value;
	if (_mc)
		_mc->pitchBendFactor(_pitchbend_factor);
}

void Part::set_onoff(bool on) {
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			_player->_se->reallocateMidiChannels(_player->getMidiDriver());
	}
}

void Part::set_instrument(byte * data) {
	_instrument.adlib(data);
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::load_global_instrument(byte slot) {
	_player->_se->copyGlobalAdlibInstrument(slot, &_instrument);
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::key_on(byte note, byte velocity) {
	MidiChannel *mc = _mc;
	_actives[note >> 4] |= (1 <<(note & 0xF));

	// DEBUG
	if (_unassigned_instrument && !_percussion) {
		_unassigned_instrument = false;
		if (!_instrument.isValid()) {
			warning("[%02d] No instrument specified", (int) _chan);
			return;
		}
	}

	if (mc && _instrument.isValid()) {
		mc->noteOn(note, velocity);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (!mc)
			return;
		mc->volume(_vol_eff);
//		mc->programChange(_bank);
		mc->noteOn(note, velocity);
	}
}

void Part::key_off(byte note) {
	MidiChannel *mc = _mc;
	_actives[note >> 4] &= ~(1 <<(note & 0xF));
	if (mc) {
		mc->noteOff(note);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (mc)
			mc->noteOff(note);
	}
}

void Part::init() {
	_player = NULL;
	_next = NULL;
	_prev = NULL;
	_mc = NULL;
	memset(_actives, 0, sizeof(_actives));
}

void Part::setup(Player *player) {
	_player = player;

	_percussion = (player->isGM() && _chan == 9); // true;
	_on = true;
	_pri_eff = player->getPriority();
	_pri = 0;
	_vol = 127;
	_vol_eff = player->getEffectiveVolume();
	_pan = clamp(player->getPan(), -64, 63);
	_transpose_eff = player->getTranspose();
	_transpose = 0;
	_detune = 0;
	_detune_eff = player->getDetune();
	_pitchbend_factor = 2;
	_pitchbend = 0;
	_effect_level = 64;
	_instrument.clear();
	_unassigned_instrument = true;
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
	_player->removePart(this);
	_player = NULL;
}

void Part::off() {
	if (_mc) {
		_mc->allNotesOff();
		_mc->release();
		_mc = NULL;
	}
	memset(_actives, 0, sizeof(_actives));
}

bool Part::clearToTransmit() {
	if (_mc) return true;
	if (_instrument.isValid()) _player->_se->reallocateMidiChannels(_player->getMidiDriver());
	return false;
}

void Part::sendAll() {
	if (!clearToTransmit()) return;
	_mc->pitchBendFactor(_pitchbend_factor);
	_mc->pitchBend(clamp(_pitchbend +
	               (_detune_eff * 64 / 12) +
	               (_transpose_eff * 8192 / 12), -8192, 8191));
	_mc->volume(_vol_eff);
	_mc->sustain(_pedal);
	_mc->modulationWheel(_modwheel);
	_mc->panPosition(_pan_eff + 0x40);
	_mc->effectLevel(_effect_level);
	if (_instrument.isValid())
		_instrument.send(_mc);
	_mc->chorusLevel(_effect_level);
	_mc->priority(_pri_eff);
}

int Part::update_actives(uint16 *active) {
	int i, j;
	uint16 *act, mask, bits;
	int count = 0;

	bits = 1 << _chan;
	act = _actives;

	for (i = 8; i; i--) {
		mask = *act++;
		if (mask) {
			for (j = 16; j; j--, mask >>= 1, active++) {
				if (mask & 1 && !(*active & bits)) {
					*active |= bits;
					count++;
				}
			}
		} else {
			active += 16;
		}
	}
	return count;
}

void Part::set_program(byte program) {
	_bank = 0;
	_instrument.program(program, _player->isMT32());
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::set_instrument(uint b) {
	_bank = (byte)(b >> 8);
	if (_bank)
		warning ("Non-zero instrument bank selection. Please report this");
	_instrument.program((byte) b, _player->isMT32());
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::silence() {
	if (!_mc)
		return;
	_mc->allNotesOff();
	memset(_actives, 0, sizeof(_actives));
}

////////////////////////////////////////
//
// Some more IMuseInternal stuff
//
////////////////////////////////////////

void IMuseInternal::midiTimerCallback(void *data) {
	MidiDriver *driver = (MidiDriver *) data;
	if (g_scumm->_imuse)
		g_scumm->_imuse->on_timer(driver);
}

void IMuseInternal::reallocateMidiChannels(MidiDriver *midi) {
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	Part *lopart;

	while (true) {
		hipri = 0;
		hipart = NULL;
		for (i = 32, part = _parts; i; i--, part++) {
			if (part->_player && part->_player->getMidiDriver() == midi &&
			    !part->_percussion && part->_on &&
				!part->_mc && part->_pri_eff >= hipri)
			{
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		if ((hipart->_mc = midi->allocateChannel()) == NULL) {
			lopri = 255;
			lopart = NULL;
			for (i = 32, part = _parts; i; i--, part++) {
				if (part->_mc && part->_mc->device() == midi && part->_pri_eff <= lopri) {
					lopri = part->_pri_eff;
					lopart = part;
				}
			}

			if (lopart == NULL || lopri >= hipri)
				return;
			lopart->off();

			if ((hipart->_mc = midi->allocateChannel()) == NULL)
				return;
		}
		hipart->sendAll();
	}
}

void IMuseInternal::setGlobalAdlibInstrument(byte slot, byte *data) {
	if (slot < 32) {
		_global_adlib_instruments[slot].adlib(data);
	}
}

void IMuseInternal::copyGlobalAdlibInstrument(byte slot, Instrument *dest) {
	if (slot >= 32)
		return;
	_global_adlib_instruments[slot].copy_to(dest);
}

////////////////////////////////////////////////////////////
//
// IMuse implementation
//
// IMuse actually serves as a concurency monitor front-end
// to IMuseInternal and ensures that only one thread
// accesses the object at a time. This is necessary to
// prevent scripts and the MIDI parser from yanking objects
// out from underneath each other.
//
////////////////////////////////////////////////////////////

IMuse::IMuse(OSystem *system, IMuseInternal *target) : _system(system), _target(target) { _mutex = system->create_mutex(); }
IMuse::~IMuse() { if (_mutex) _system->delete_mutex(_mutex); if (_target) delete _target; }
inline void IMuse::in() const { _system->lock_mutex(_mutex); }
inline void IMuse::out() const { _system->unlock_mutex(_mutex); }

void IMuse::on_timer(MidiDriver *midi) { in(); _target->on_timer(midi); out(); }
void IMuse::pause(bool paused) { in(); _target->pause(paused); out(); }
int IMuse::save_or_load(Serializer *ser, Scumm *scumm) { in(); int ret = _target->save_or_load(ser, scumm); out(); return ret; }
int IMuse::set_music_volume(uint vol) { in(); int ret = _target->set_music_volume(vol); out(); return ret; }
int IMuse::get_music_volume() { in(); int ret = _target->get_music_volume(); out(); return ret; }
void IMuse::setMasterVolume(int vol) { in(); _target->setMasterVolume(vol); out(); }
int IMuse::get_master_volume() { in(); int ret = _target->get_master_volume(); out(); return ret; }
void IMuse::startSound(int sound) { in(); _target->startSound(sound); out(); }
void IMuse::stopSound(int sound) { in(); _target->stopSound(sound); out(); }
void IMuse::stopAllSounds() { in(); _target->stopAllSounds(); out(); }
int IMuse::getSoundStatus(int sound) const { in(); int ret = _target->getSoundStatus(sound, true); out(); return ret; }
bool IMuse::get_sound_active(int sound) const { in(); bool ret = _target->getSoundStatus(sound, false) ? 1 : 0; out(); return ret; }
int IMuse::getMusicTimer() { in(); int ret = _target->getMusicTimer(); out(); return ret; }
int32 IMuse::doCommand (int a, int b, int c, int d, int e, int f, int g, int h) { in(); int32 ret = _target->doCommand(a,b,c,d,e,f,g,h); out(); return ret; }
int32 IMuse::doCommand (int numargs, int args[]) { in(); int32 ret = _target->doCommand (numargs, args); out(); return ret; }
int IMuse::clear_queue() { in(); int ret = _target->clear_queue(); out(); return ret; }
void IMuse::setBase(byte **base) { in(); _target->setBase(base); out(); }
uint32 IMuse::property(int prop, uint32 value) { in(); uint32 ret = _target->property(prop, value); out(); return ret; }
void IMuse::terminate() { in(); _target->terminate1(); out(); _target->terminate2(); }

// The IMuse::create method provides a front-end factory
// for creating IMuseInternal without exposing that class
// to the client.
IMuse *IMuse::create(OSystem *syst, SoundMixer *mixer, MidiDriver *midi) {
	IMuseInternal *engine = IMuseInternal::create(syst, mixer, midi);
	return new IMuse(syst, engine);
}
