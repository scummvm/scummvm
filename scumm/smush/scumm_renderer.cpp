/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdafx.h>
#include "common/util.h"
#include "scumm_renderer.h"
#include "channel.h"
#include "mixer.h"
#include "sound/mixer.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse.h"
#include "scumm/actor.h"

class ScummMixer : public Mixer {
private:
	SoundMixer * _mixer; //!< pointer to the SoundMixer instance
	struct {
		int id;
		_Channel * chan;
		bool first;
		int mixer_index;
	} _channels[SoundMixer::NUM_CHANNELS];		//!< The map of track and channels
	int _nextIndex;
public:
	ScummMixer(SoundMixer *);
	virtual ~ScummMixer();
	bool init();
	_Channel * findChannel(int32 track);
	bool addChannel(_Channel * c);
	bool handleFrame();
	bool stop();
	bool update();
	bool _silentMixer;
};

ScummMixer::ScummMixer(SoundMixer * m) : _mixer(m), _nextIndex(_mixer->_beginSlots) {
	for(int32 i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		_channels[i].id = -1;
		_channels[i].chan = 0;
		_channels[i].first = true;
	}
}

ScummMixer::~ScummMixer() {
}

bool ScummMixer::init() {
	debug(9, "ScummMixer::init()");
	return true;
}

_Channel * ScummMixer::findChannel(int32 track) {
	debug(9, "scumm_mixer::findChannel(%d)", track);
	for(int32 i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id == track)
			return _channels[i].chan;
	}
	return 0;
}

bool ScummMixer::addChannel(_Channel * c) {
	int32 track = c->getTrackIdentifier();
	int i;

	debug(9, "ScummMixer::addChannel(%d)", track);

	for(i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id == track)
			warning("mixer::addChannel(%d) : channel already exist !", track);
	}
	if(_nextIndex >= SoundMixer::NUM_CHANNELS) _nextIndex = _mixer->_beginSlots;

	for(i = _nextIndex; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].chan == 0 || _channels[i].id == -1) {
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].first = true;
			_nextIndex = i + 1;
			return true;
		}
	}

	for(i = _mixer->_beginSlots; i < _nextIndex; i++) {
		if(_channels[i].chan == 0 || _channels[i].id == -1)	{
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].first = true;
			_nextIndex = i + 1;
			return true;
		}
	}

	fprintf(stderr, "_nextIndex == %d\n", _nextIndex);

	for(i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		fprintf(stderr, "channel %d : %p(%d, %d) %d %d\n", i, (void *)_channels[i].chan, 
			_channels[i].chan ? _channels[i].chan->getTrackIdentifier() : -1, 
			_channels[i].chan ? _channels[i].chan->isTerminated() : 1, 
			_channels[i].first, _channels[i].mixer_index);
	}

	error("mixer::add_channel() : no more channel available");
	return false;
}

bool ScummMixer::handleFrame() {
	debug(9, "ScummMixer::handleFrame()");
	for(int i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id != -1) {
			debug(9, "updating channel %d (%p)", _channels[i].id, _channels[i].chan);
			if(_channels[i].chan->isTerminated()) {
				debug(9, "channel %d has terminated (%p)", _channels[i].id, _channels[i].chan);
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = 0;
			} else {
				int32 rate;
				bool stereo, is_short;

				_channels[i].chan->getParameters(rate, stereo, is_short);
				int32 size = _channels[i].chan->availableSoundData();
				debug(9, "channel %d : %d, %s, %d bits, %d", _channels[i].id, rate, stereo ? "stereo" : "mono", is_short ? 16 : 8, size);
				int32 flags = stereo ? SoundMixer::FLAG_STEREO : 0;

				if(is_short) {
					// FIXME this is one more data copy... we could get rid of it...
					short * data = new int16[size * (stereo ? 2 : 1) * 2];
					_channels[i].chan->getSoundData(data, size);
					if(_channels[i].chan->getRate() == 11025) size *= 2;
					size *= stereo ? 4 : 2;

					if(_silentMixer == false) {
						// append to _sound
						if(_channels[i].first) {
							_channels[i].mixer_index = _mixer->playStream(NULL, -1, data, size, rate, flags | SoundMixer::FLAG_16BITS);
							debug(5, "channel %d bound to mixer_index %d", _channels[i].id, _channels[i].mixer_index);
							_channels[i].first = false;
						} else {
							_mixer->append(_channels[i].mixer_index, data, size, rate, flags | SoundMixer::FLAG_16BITS);
						}
					}

					delete []data;
				} else {
					int8 * data = new int8[size * (stereo ? 2 : 1) * 2];
					_channels[i].chan->getSoundData(data, size);
					if(_channels[i].chan->getRate() == 11025) size *= 2;
					size *= stereo ? 2 : 1;

					if(_silentMixer == false) {
						// append to _sound
						if(_channels[i].first) {
							_channels[i].mixer_index = _mixer->playStream(NULL, -1, data, size, rate, flags | SoundMixer::FLAG_UNSIGNED);
							_channels[i].first = false;
						} else {
							_mixer->append(_channels[i].mixer_index, data, size, rate, flags | SoundMixer::FLAG_UNSIGNED);
						}
					}

					delete []data;
				}
			}
		}
	}
	return true;
}

bool ScummMixer::stop() {
	debug(9, "ScummMixer::stop()");
	for(int i = _mixer->_beginSlots; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id != -1) {
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = 0;
		}
	}
	return true;
}

ScummRenderer::ScummRenderer(Scumm * scumm, uint32 speed) :
	_scumm(scumm),
	_smixer(0),
	_insaneSpeed(speed),
	_pending_updates(0) {
}

static ScummRenderer * s_renderer;

static void smush_handler(void * engine) {
	s_renderer->update();
}

Mixer * ScummRenderer::getMixer() {
	if(_smixer == 0) {
		_smixer = new ScummMixer(_scumm->_mixer);
		if(!_smixer) error("unable to allocate a smush mixer");
		_smixer->_silentMixer = _scumm->_silentDigitalImuse;
		s_renderer = this;
		_scumm->_timer->installProcedure(&smush_handler, _insaneSpeed);
	}
	return _smixer;
}

ScummRenderer::~ScummRenderer() {
	_scumm->_insaneState = false;
	_scumm->exitCutscene();
	if(_smixer) {
		_scumm->_timer->releaseProcedure(&smush_handler);
		delete _smixer;
		_smixer = 0;
	}
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(false);
	}
	_scumm->_sound->pauseBundleMusic(false);
#if 0
	// FIXME - enabling this breaks the COMI demo. OTOH I am not aware
	// what disabling this breaks... if anybody knows of any regressions
	// turning this off causes, please tell me.
	_scumm->_fullRedraw = 1;
	_scumm->redrawBGAreas();
	for (int32 i = 0; i < _scumm->NUM_ACTORS; i++)
		_scumm->derefActor(i)->needRedraw = true;
	_scumm->processActors();
#endif
}

bool ScummRenderer::wait(int32 ms) {
	// Because waitForTimer() also is the function that checks for user
	// input we always want to call it at least once between frames, or
	// the user may become unable to interrupt the movie.
	do {
 		_scumm->waitForTimer(1);
	} while(_pending_updates <= 0);
	return true; 
}

bool ScummRenderer::startDecode(const char * fname, int32 version, int32 nbframes) {
	if (_scumm->_imuseDigital) {
		_scumm->_imuseDigital->pause(true);
	}
	_scumm->_sound->pauseBundleMusic(true);
	_scumm->_videoFinished = false;
	_scumm->_insaneState = true;
	return true;
}

bool ScummRenderer::setPalette(const Palette & pal) {
	int i;
	byte palette_colors[1024];
	byte *p = palette_colors;

	for (i = 0; i < 256; i++, p += 4) {
		p[0] = pal[i].red();
		p[1] = pal[i].green();
		p[2] = pal[i].blue();
		p[3] = 0;
	}

	_scumm->_system->set_palette(palette_colors, 0, 256);
	_scumm->setDirtyColors(0, 255);
	return BaseRenderer::setPalette(pal); // For compatibility with possible subclass...
}

void ScummRenderer::save(int32 frame) {
	int width = MIN(getWidth(), _scumm->_realWidth); 
	int height = MIN(getHeight(), _scumm->_realHeight);
	

	// In theory, this will always be true. In reality, there may be
	// several pending updates because the computer wasn't fast enough to
	// process them all. In that case, skip the frame to catch up.
	if (--_pending_updates <= 0) {
		_scumm->_system->copy_rect((const byte *)data(), getWidth(), 0, 0, width, height);
		_scumm->_system->update_screen();
	} else {
		warning("ScummRenderer: Skipping frame %d to catch up", getFrame());
	}
	_scumm->processKbd();
}

bool ScummRenderer::prematureClose() { 
	return _scumm->_videoFinished || _scumm->_saveLoadFlag;
}

bool ScummRenderer::update() {
	_pending_updates++;
	return true;
}

