/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "common/util.h"
#include "smush_mixer.h"
#include "channel.h"
#include "sound/mixer.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"

SmushMixer::SmushMixer(SoundMixer *m) :
	_mixer(m),
	_nextIndex(0),
	_soundFrequency(22050) {
	for (int32 i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		_channels[i].id = -1;
		_channels[i].chan = NULL;
		_channels[i].mixer_index = -1;
	}
}

SmushMixer::~SmushMixer() {
	for (int32 i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].mixer_index != -1)
			_mixer->stop(_channels[i].mixer_index);
	}
}

SmushChannel *SmushMixer::findChannel(int32 track) {
	debug(9, "SmushMixer::findChannel(%d)", track);
	for (int32 i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			return _channels[i].chan;
	}
	return NULL;
}

bool SmushMixer::addChannel(SmushChannel *c) {
	int32 track = c->getTrackIdentifier();
	int i;

	debug(9, "SmushMixer::addChannel(%d)", track);

	for (i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			warning("SmushMixer::addChannel(%d) : channel already exists", track);
	}
	if (_nextIndex >= SoundMixer::NUM_CHANNELS)
		_nextIndex = 0;

	for (i = _nextIndex; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].chan == NULL || _channels[i].id == -1) {
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].mixer_index = -1;
			_nextIndex = i + 1;
			return true;
		}
	}

	for (i = 0; i < _nextIndex; i++) {
		if (_channels[i].chan == NULL || _channels[i].id == -1)	{
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].mixer_index = -1;
			_nextIndex = i + 1;
			return true;
		}
	}

	warning("_nextIndex == %d", _nextIndex);

	for (i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		warning("channel %d : %p(%d, %d) %d %d", i, (void *)_channels[i].chan, 
			_channels[i].chan ? _channels[i].chan->getTrackIdentifier() : -1, 
			_channels[i].chan ? _channels[i].chan->isTerminated() : 1, 
			_channels[i].mixer_index);
	}

	error("SmushMixer::add_channel() : no more channel available");
	return false;
}

bool SmushMixer::handleFrame() {
	debug(9, "SmushMixer::handleFrame()");
	for (int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			if (_channels[i].chan->isTerminated()) {
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = NULL;
			} else {
				int32 rate;
				bool stereo, is_short;
				void *data;

				_channels[i].chan->getParameters(rate, stereo, is_short);
				int32 size = _channels[i].chan->availableSoundData();
				int32 flags = stereo ? SoundMixer::FLAG_STEREO : 0;

				if (is_short) {
					data = malloc(size * (stereo ? 2 : 1) * 4);
					_channels[i].chan->getSoundData((int16 *)data, size);
					size *= stereo ? 4 : 2;

					flags |= SoundMixer::FLAG_16BITS;

				} else {
					data = malloc(size * (stereo ? 2 : 1) * 2);
					_channels[i].chan->getSoundData((int8 *)data, size);
					size *= stereo ? 2 : 1;

					flags |= SoundMixer::FLAG_UNSIGNED;
				}

				if (_silentMixer == false) {
					if (_channels[i].mixer_index == -1) {
						_channels[i].mixer_index = _mixer->playStream(data, size, rate, flags, 2000000);
					} else {
						_mixer->append(_channels[i].mixer_index, data, size);
					}
				}
				free(data);
			}
		}
	}
	return true;
}

bool SmushMixer::stop() {
	debug(9, "SmushMixer::stop()");
	for (int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			delete _channels[i].chan;
			_channels[i].id = -1;
			_channels[i].chan = NULL;
		}
	}
	return true;
}
