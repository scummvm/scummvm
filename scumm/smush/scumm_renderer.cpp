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
#include "scumm_renderer.h"
#include "channel.h"

class scumm_mixer : public Mixer {
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
	scumm_mixer(SoundMixer *);
	virtual ~scumm_mixer();
	bool init();
	_Channel * findChannel(int track);
	bool addChannel(_Channel * c);
	bool handleFrame();
	bool stop();
	bool update();
};

scumm_mixer::scumm_mixer(SoundMixer * m) : _mixer(m), _nextIndex(0) {
	for(int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		_channels[i].id = -1;
		_channels[i].chan = 0;
		_channels[i].first = true;
	}
}

scumm_mixer::~scumm_mixer() {
}

bool scumm_mixer::init() {
	debug(9, "scumm_mixer::init()");
	return true;
}

_Channel * scumm_mixer::findChannel(int track) {
	debug(9, "scumm_mixer::findChannel(%d)", track);
	for(int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id == track)
			return _channels[i].chan;
	}
	return 0;
}

bool scumm_mixer::addChannel(_Channel * c) {
	int track = c->getTrackIdentifier();
	int i;

	debug(9, "scumm_mixer::addChannel(%d)", track);

	for(i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id == track)
			warning("mixer::addChannel(%d) : channel already exist !", track);
	}
	if(_nextIndex >= SoundMixer::NUM_CHANNELS) _nextIndex = 0;

	for(i = _nextIndex; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].chan == 0 || _channels[i].id == -1) {
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].first = true;
			_nextIndex = i + 1;
			return true;
		}
	}

	for(i = 0; i < _nextIndex; i++) {
		if(_channels[i].chan == 0 || _channels[i].id == -1)	{
			_channels[i].chan = c;
			_channels[i].id = track;
			_channels[i].first = true;
			_nextIndex = i + 1;
			return true;
		}
	}

	fprintf(stderr, "_nextIndex == %d\n", _nextIndex);

	for(i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		fprintf(stderr, "channel %d : %p(%d, %d) %d %d\n", i, _channels[i].chan, 
			_channels[i].chan ? _channels[i].chan->getTrackIdentifier() : -1, 
			_channels[i].chan ? _channels[i].chan->isTerminated() : 1, 
			_channels[i].first, _channels[i].mixer_index);
	}

	error("mixer::add_channel() : no more channel available");
	return false;
}

bool scumm_mixer::handleFrame() {
	debug(9, "scumm_mixer::handleFrame()");
	for(int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id != -1) {
			debug(9, "updating channel %d (%p)", _channels[i].id, _channels[i].chan);
			if(_channels[i].chan->isTerminated()) {
				debug(9, "channel %d has terminated (%p)", _channels[i].id, _channels[i].chan);
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = 0;
			} else {
				int rate;
				bool stereo, is_short;

				_channels[i].chan->getParameters(rate, stereo, is_short);
				int size = _channels[i].chan->availableSoundData();
				debug(9, "channel %d : %d, %s, %d bits, %d", _channels[i].id, rate, stereo ? "stereo" : "mono", is_short ? 16 : 8, size);
				int flags = stereo ? SoundMixer::FLAG_STEREO : 0;

				if(is_short) {
					// FIXME this is one more data copy... we could get rid of it...
					short * data = new short[size * (stereo ? 2 : 1)];
					_channels[i].chan->getSoundData(data, size);
					size *= stereo ? 4 : 2;

					// append to _sound
					if(_channels[i].first) {
						_channels[i].mixer_index = _mixer->playStream(NULL, -1, data, size, rate, flags | SoundMixer::FLAG_16BITS);
						debug(5, "channel %d bound to mixer_index %d", _channels[i].id, _channels[i].mixer_index);
						_channels[i].first = false;
					} else {
						_mixer->append(_channels[i].mixer_index, data, size, rate, flags | SoundMixer::FLAG_16BITS);
					}

					delete []data;
				} else {
					char * data = new char[size*(stereo ? 2 : 1)];
					_channels[i].chan->getSoundData(data, size);
					size *= stereo ? 2 : 1;

					// append to _sound
					if(_channels[i].first) {
						_channels[i].mixer_index = _mixer->playStream(NULL, -1, data, size, rate, flags | SoundMixer::FLAG_UNSIGNED);
						_channels[i].first = false;
					} else {
						_mixer->append(_channels[i].mixer_index, data, size, rate, flags | SoundMixer::FLAG_UNSIGNED);
					}

					delete []data;
				}
			}
		}
	}
	return true;
}

bool scumm_mixer::stop() {
	debug(9, "scumm_mixer::stop()");
	for(int i = 0; i < SoundMixer::NUM_CHANNELS; i++) {
		if(_channels[i].id != -1) {
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = 0;
		}
	}
	//~ _mixer->stopAll();
	return true;
}

ScummRenderer::ScummRenderer(Scumm * scumm) : _scumm(scumm), _smixer(0) {
}

static ScummRenderer * s_renderer;

static void smush_handler(Scumm * scumm) {
	s_renderer->update();
}

Mixer * ScummRenderer::getMixer() {
	if(_smixer == 0) {
		_scumm->_sound->pauseBundleMusic(true);
		_smixer = new scumm_mixer(_scumm->_mixer);
		if(!_smixer) error("unable to allocate a smush mixer");
		s_renderer = this;
		_scumm->_timer->installProcedure(&smush_handler, 75);
	}
	return _smixer;
}

ScummRenderer::~ScummRenderer() {
	_scumm->_insaneState = 0;
	_scumm->exitCutscene();
	if(_smixer) {
		_scumm->_timer->releaseProcedure(&smush_handler);
		delete _smixer;
		_smixer = 0;
	}
	_scumm->_sound->pauseBundleMusic(false);
}

bool ScummRenderer::wait(int ms) {
	while(_wait) {
		_scumm->waitForTimer(1);
	}
	return true; 
}

bool ScummRenderer::startDecode(const char * fname, int version, int nbframes) {
	_scumm->_sound->pauseBundleMusic(true);
	_scumm->videoFinished = 0;
	_scumm->_insaneState = 1;
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

void ScummRenderer::save(int frame) {
	int width = min(getWidth(), _scumm->_realWidth); 
	int height = min(getHeight(), _scumm->_realHeight);
	
	_scumm->_system->copy_rect((const byte *)data(), getWidth(), 0, 0, width, height);
	_scumm->_system->update_screen();
	_scumm->processKbd();
	_wait = true;
}

bool ScummRenderer::prematureClose() { 
	return _scumm->videoFinished; 
}

bool ScummRenderer::update() {
	_wait = false;
	return true;
}

