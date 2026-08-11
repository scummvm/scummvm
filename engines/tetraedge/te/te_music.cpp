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

#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/vorbis.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_core.h"

namespace Tetraedge {

TeMusic::TeMusic() : _repeat(true), _isPlaying(false), _currentData(0),
_volume(1.0), _isPaused(false), _channelName("music"), _sndHandleValid(false),
_retain(false) {
	g_engine->getSoundManager()->musics().push_back(this);
}

TeMusic::~TeMusic() {
	close();
	Common::Array<TeMusic *> &m = g_engine->getSoundManager()->musics();
	for (uint i = 0; i < m.size(); i++) {
		if (m[i] == this) {
			m.remove_at(i);
			break;
		}
	}
}

void TeMusic::pause() {
	_mutex.lock();
	_isPaused = true;
	if (_isPlaying) {
		Audio::Mixer *mixer = g_system->getMixer();
		mixer->pauseHandle(_sndHandle, true);
	}
	_mutex.unlock();
}

bool TeMusic::play() {
	if (isPlaying())
		return true;
	if (!_fileNode.exists())
		return false;

	Common::SeekableReadStream *streamfile = _fileNode.createReadStream();
	if (!streamfile) {
		return false;
	}
	Audio::AudioStream *stream = Audio::makeVorbisStream(streamfile, DisposeAfterUse::YES);
	byte vol = round(_volume * 255.0);
	//int channelId = _channelName.hash();

	Audio::Mixer *mixer = g_system->getMixer();
	Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType;
	if (_channelName == "sfx") {
		soundType = Audio::Mixer::kSFXSoundType;
	} else if (_channelName == "dialog") {
		soundType = Audio::Mixer::kSpeechSoundType;
	} else if (_channelName == "music") {
		soundType = Audio::Mixer::kMusicSoundType;
	}

	//debug("playing %s on channel %s at vol %d", _fileNode.toString().c_str(), _channelName.c_str(), vol);
	mixer->playStream(soundType, &_sndHandle, stream, -1, vol);
	_sndHandleValid = true;
	_isPaused = false;
	_isPlaying = true;
	if (_repeat)
		mixer->loopChannel(_sndHandle);
	return true;
}

bool TeMusic::repeat() {
	_mutex.lock();
	bool retval = _repeat;
	_mutex.unlock();
	return retval;
}

void TeMusic::repeat(bool val) {
	_mutex.lock();
	if (_repeat && !val && _sndHandleValid) {
		error("TODO: Implement clearing of loop - not supported by ScummVM mixer");
	}
	_repeat = val;
	if (_sndHandleValid) {
		Audio::Mixer *mixer = g_system->getMixer();
		mixer->loopChannel(_sndHandle);
	}
	_mutex.unlock();
}

void TeMusic::resume() {
	_mutex.lock();
	_isPaused = true;
	if (_isPlaying) {
		Audio::Mixer *mixer = g_system->getMixer();
		mixer->pauseHandle(_sndHandle, false);
	}
	_mutex.unlock();
}

void TeMusic::stop() {
	_mutex.lock();
	_isPlaying = false;
	_isPaused = false;
	_mutex.unlock();
	/* original does this here.. probably not needed as mixer
	 does it for us.
	_thread->waitForTerminate();
	_soundStreamed->close(); */
	if (_sndHandleValid) {
		Audio::Mixer *mixer = g_system->getMixer();
		mixer->stopHandle(_sndHandle);
		_sndHandleValid = false;
		_sndHandle = Audio::SoundHandle();
		_onStopSignal.call();
	}
	return;
}

byte TeMusic::currentData() {
	_mutex.lock();
	byte retval = _currentData;
	_mutex.unlock();
	return retval;
}


// This is probably not needed - it's the thread function
// which is handled by the mixer in ScummVM
void TeMusic::entry() {
	error("TODO: Implement TeMusic::entry");
}

bool TeMusic::isPlaying() {
	_mutex.lock();
	bool retval = _isPlaying && !_isPaused;
	_mutex.unlock();
	return retval;
}

bool TeMusic::load(const Common::Path &path) {
	if (path.empty())
		return false;

	if (path != _rawPath)
		setFilePath(path);

	return true;
}

bool TeMusic::onSoundManagerVolumeChanged() {
	// Note: Not needed in ScummVM as it recalcalates the sound
	// volume given new global volume - but that's handled by the mixer.
	return false;
}

Common::Path TeMusic::path() {
	_mutex.lock();
	Common::Path retval = _rawPath;
	_mutex.unlock();
	return retval;
}

void TeMusic::setFilePath(const Common::Path &name) {
	stop();
	setAccessName(name);
	_rawPath = name;
	TeCore *core = g_engine->getCore();
	// Note: original search logic here abstracted away in our version..
	_fileNode = core->findFile(name);
}

void TeMusic::update() {
	/* Do callback and update flags when sounds stop */
	bool hasStopped = false;
	_mutex.lock();
	if (_isPlaying && !_isPaused && _sndHandleValid && !g_system->getMixer()->isSoundHandleActive(_sndHandle))
		hasStopped = true;

	if (hasStopped) {
		g_system->getMixer()->stopHandle(_sndHandle);
		_sndHandle = Audio::SoundHandle();
		_isPaused = false;
		_isPlaying = false;
		_sndHandleValid = false;
	}
	_mutex.unlock();

	if (hasStopped) {
		_onStopSignal.call();
	}
}

void TeMusic::volume(float vol) {
	_mutex.lock();
	_volume = vol;
	if (_sndHandleValid) {
		Audio::Mixer *mixer = g_system->getMixer();
		mixer->setChannelVolume(_sndHandle, round(_volume * 255.0));
	}
	_mutex.unlock();
}

float TeMusic::volume() {
	_mutex.lock();
	float retval = _volume;
	_mutex.unlock();
	return retval;
}

} // end namespace Tetraedge
