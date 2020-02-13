/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/sound.h"
#include "glk/glk.h"
#include "glk/events.h"
#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"

namespace Glk {

Sounds::~Sounds() {
	for (int idx = (int)_sounds.size() - 1; idx >= 0; --idx)
		delete _sounds[idx];
}

void Sounds::removeSound(schanid_t snd) {
	for (uint idx = 0; idx < _sounds.size(); ++idx) {
		if (_sounds[idx] == snd) {
			_sounds.remove_at(idx);
			break;
		}
	}
}

schanid_t Sounds::create(uint rock, uint volume) {
	schanid_t snd = new SoundChannel(this, volume);
	_sounds.push_back(snd);
	return snd;
}

schanid_t Sounds::iterate(schanid_t chan, uint *rockptr) {
	for (int idx = 0; idx < (int)_sounds.size() - 1; ++idx) {
		if (_sounds[idx] == chan) {
			schanid_t next = _sounds[idx + 1];
			if (*rockptr)
				*rockptr = next->_rock;

			return next;
		}
	}

	return nullptr;
}

void Sounds::poll() {
	for (uint idx = 0; idx < _sounds.size(); ++idx)
		_sounds[idx]->poll();
}

/*--------------------------------------------------------------------------*/

SoundChannel::SoundChannel(Sounds *owner, uint volume) : _owner(owner),
		_soundNum(0), _rock(0), _notify(0) {
	_defaultVolume = MIN(volume, (uint)GLK_MAXVOLUME);

	if (g_vm->gli_register_obj)
		_dispRock = (*g_vm->gli_register_obj)(this, gidisp_Class_Schannel);
}

SoundChannel::~SoundChannel() {
	stop();
	_owner->removeSound(this);

	if (g_vm->gli_unregister_obj)
		(*g_vm->gli_unregister_obj)(this, gidisp_Class_Schannel, _dispRock);
}

uint SoundChannel::play(uint soundNum, uint repeats, uint notify) {
	stop();
	if (repeats == 0)
		return 1;

	// Find a sound of the given name
	Audio::AudioStream *stream;
	Common::File f;
	Common::String nameSnd = Common::String::format("sound%u.snd", soundNum);
	Common::String nameWav = Common::String::format("sound%u.wav", soundNum);
	Common::String nameAiff = Common::String::format("sound%u.aiff", soundNum);
#ifdef USE_MAD
	Common::String nameMp3 = Common::String::format("sound%u.mp3", soundNum);
#endif

	if (f.exists(nameSnd) && f.open(nameSnd)) {
		if (f.readUint16BE() != (f.size() - 2))
			error("Invalid sound filesize");
		byte headerRepeats = f.readByte();
		if (headerRepeats > 0)
			repeats = headerRepeats;
		f.skip(1);
		uint freq = f.readUint16BE();
		f.skip(2);
		uint size = f.readUint16BE();

		Common::SeekableReadStream *s = f.readStream(size);
		stream = Audio::makeRawStream(s, freq, Audio::FLAG_UNSIGNED);

#ifdef USE_MAD
	} else if (f.exists(nameMp3) && f.open(nameMp3)) {
		Common::SeekableReadStream *s = f.readStream(f.size());
		stream = Audio::makeMP3Stream(s, DisposeAfterUse::YES);
#endif
	} else if (f.exists(nameWav) && f.open(nameWav)) {
		Common::SeekableReadStream *s = f.readStream(f.size());
		stream = Audio::makeWAVStream(s, DisposeAfterUse::YES);

	} else if (f.exists(nameAiff) && f.open(nameAiff)) {
		Common::SeekableReadStream *s = f.readStream(f.size());
		stream = Audio::makeAIFFStream(s, DisposeAfterUse::YES);

	} else {
		warning("Could not find sound %u", soundNum);
		return 1;
	}

	_soundNum = soundNum;
	_notify = notify;

	// Set up a repeat if multiple repeats are specified
	if (repeats > 1) {
		Audio::RewindableAudioStream *rwStream = dynamic_cast<Audio::RewindableAudioStream *>(stream);
		assert(rwStream);
		stream = new Audio::LoopingAudioStream(rwStream, repeats, DisposeAfterUse::YES);
	}

	// Start playing the audio
	g_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, stream, -1,
		_defaultVolume * 255 / GLK_MAXVOLUME);
	return 0;
}

void SoundChannel::stop() {
	g_vm->_mixer->stopHandle(_handle);
}

void SoundChannel::poll() {
	if (!g_vm->_mixer->isSoundHandleActive(_handle) && _notify != 0) {
		uint notify = _notify;
		_notify = 0;
		g_vm->_events->store(evtype_SoundNotify, nullptr, _soundNum, notify);
	}
}

void SoundChannel::setVolume(uint volume, uint duration, uint notify) {
	uint newVol = volume * 255 / GLK_MAXVOLUME;
	g_vm->_mixer->setChannelVolume(_handle, newVol);

	if (notify) {
		warning("TODO: Gradual volume change");
		g_vm->_events->store(evtype_VolumeNotify, nullptr, 0, notify);	
	}
}

void SoundChannel::pause() {
	g_vm->_mixer->pauseHandle(_handle, true);
}

void SoundChannel::unpause() {
	g_vm->_mixer->pauseHandle(_handle, false);
}

} // End of namespace Glk
