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

#include "sci/tools.h"
#include "sci/sfx/player/new_player.h"
#include "sci/sfx/sequencer.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/core.h"

#include "common/system.h"

#include "sci/sfx/softseq/pcjr.h"
#include "sci/sfx/softseq/adlib.h"

namespace Sci {

NewPlayer::NewPlayer() {
	_mididrv = 0;

	_iterator = NULL;
	_pauseTimeDiff = 0;

	_paused = false;
	_iteratorIsDone = false;
	_tempo = 0;

	_mutex = 0;
	_volume = 15;
}

void NewPlayer::play_song(SongIterator *it) {
	while (_iterator && _wakeupTime.msecsDiff(_currentTime) <= 0) {
		int delay;
		byte buf[8];
		int result;

		switch ((delay = songit_next(&(_iterator),
		                             buf, &result,
		                             IT_READER_MASK_ALL
		                             | IT_READER_MAY_FREE
		                             | IT_READER_MAY_CLEAN))) {

		case SI_FINISHED:
			delete _iterator;
			_iterator = NULL;
			_iteratorIsDone = true;
			return;

		case SI_IGNORE:
		case SI_LOOP:
		case SI_RELATIVE_CUE:
		case SI_ABSOLUTE_CUE:
			break;

		case SI_PCM:
			sfx_play_iterator_pcm(_iterator, 0);
			break;

		case 0:
			static_cast<MidiDriver *>(_mididrv)->send(buf[0], buf[1], buf[2]);

			break;

		default:
			_wakeupTime = _wakeupTime.addFrames(delay);
		}
	}
}

void NewPlayer::tell_synth(int buf_nr, byte *buf) {
	byte op1 = (buf_nr < 2 ? 0 : buf[1]);
	byte op2 = (buf_nr < 3 ? 0 : buf[2]);

	static_cast<MidiDriver *>(_mididrv)->send(buf[0], op1, op2);
}

void NewPlayer::player_timer_callback(void *refCon) {
	NewPlayer *player = (NewPlayer *)refCon;
	assert(refCon);
	Common::StackLock lock(*player->_mutex);

	if (player->_iterator && !player->_iteratorIsDone && !player->_paused) {
		player->play_song(player->_iterator);
	}

	player->_currentTime = player->_currentTime.addFrames(1);
}

/* API implementation */

Common::Error NewPlayer::init(ResourceManager *resmgr, int expected_latency) {
	MidiDriverType musicDriver = MidiDriver::detectMusicDriver(MDT_PCSPK | MDT_ADLIB);

	switch(musicDriver) {
	case MD_ADLIB:
		_mididrv = new MidiPlayer_Adlib();
		break;
	case MD_PCJR:
		_mididrv = new MidiPlayer_PCJr();
		break;
	case MD_PCSPK:
		_mididrv = new MidiPlayer_PCSpeaker();
		break;
	default:
		break;
	}

	assert(_mididrv);

	_polyphony = _mididrv->getPolyphony();

	_tempo = _mididrv->getBaseTempo();
    uint32 time = g_system->getMillis();
	_currentTime = Audio::Timestamp(time, 1000000 / _tempo);
	_wakeupTime = Audio::Timestamp(time, SFX_TICKS_PER_SEC);

	_mutex = new Common::Mutex();

	_mididrv->setTimerCallback(this, player_timer_callback);
	_mididrv->open(resmgr);
	_mididrv->setVolume(_volume);

	return Common::kNoError;
}

Common::Error NewPlayer::add_iterator(SongIterator *it, uint32 start_time) {
	Common::StackLock lock(*_mutex);
	SIMSG_SEND(it, SIMSG_SET_PLAYMASK(_mididrv->getPlayMask()));
	SIMSG_SEND(it, SIMSG_SET_RHYTHM(_mididrv->hasRhythmChannel()));

	if (_iterator == NULL) {
		// Resync with clock
		_currentTime = Audio::Timestamp(g_system->getMillis(), 1000000 / _tempo);
		_wakeupTime = Audio::Timestamp(start_time, SFX_TICKS_PER_SEC);
	}

	_iterator = sfx_iterator_combine(_iterator, it);
	_iteratorIsDone = false;

	return Common::kNoError;
}

Common::Error NewPlayer::stop(void) {
	debug(3, "Player: Stopping song iterator %p", (void *)_iterator);
	Common::StackLock lock(*_mutex);
	delete _iterator;
	_iterator = NULL;
	for (int i = 0; i < MIDI_CHANNELS; i++)
		static_cast<MidiDriver *>(_mididrv)->send(0xb0 + i, SCI_MIDI_CHANNEL_NOTES_OFF, 0);

	return Common::kNoError;
}

Common::Error NewPlayer::iterator_message(const SongIterator::Message &msg) {
	Common::StackLock lock(*_mutex);
	if (!_iterator) {
		return Common::kUnknownError;
	}

	songit_handle_message(&_iterator, msg);

	return Common::kNoError;
}

Common::Error NewPlayer::pause(void) {
	Common::StackLock lock(*_mutex);

	_paused = true;
	_pauseTimeDiff = _wakeupTime.msecsDiff(_currentTime);

	_mididrv->playSwitch(false);

	return Common::kNoError;
}

Common::Error NewPlayer::resume(void) {
	Common::StackLock lock(*_mutex);

	_wakeupTime = Audio::Timestamp(_currentTime.msecs() + _pauseTimeDiff, SFX_TICKS_PER_SEC);
	_mididrv->playSwitch(true);
	_paused = false;

	return Common::kNoError;
}

Common::Error NewPlayer::exit(void) {
	_mididrv->close();
	delete _mididrv;
	delete _mutex;
	delete _iterator;
	_iterator = NULL;

	return Common::kNoError;
}

} // End of namespace Sci
