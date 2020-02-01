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

#ifndef SOUND_MIXER_INTERN_H
#define SOUND_MIXER_INTERN_H

//#include "common/scummsys.h"
#include "mutex.h"
#include "audio/mixer.h"

namespace Audio {

/**
 * The (default) implementation of the ScummVM audio mixing subsystem.
 *
 * Backends are responsible for allocating (and later releasing) an instance
 * of this class, which engines can access via OSystem::getMixer().
 *
 * Initialisation of instances of this class usually happens as follows:
 * 1) Creat a new Audio::MixerImpl instance.
 * 2) Set the hardware output sample rate via the setSampleRate() method.
 * 3) Hook up the mixCallback() in a suitable audio processing thread/callback.
 * 4) Change the mixer into ready mode via setReady(true).
 * 5) Start audio processing (e.g. by resuming the audio thread, if applicable).
 *
 * In the future, we might make it possible for backends to provide
 * (partial) alternative implementations of the mixer, e.g. to make
 * better use of native sound mixing support on low-end devices.
 *
 * @see OSystem::getMixer()
 */
class MixerImpl : public Mixer {
private:
	enum {
		NUM_CHANNELS = 16
	};

	//OSystem *_syst;
	Common::Mutex _mutex;

	const uint32 _sampleRate;
	bool _mixerReady;
	uint32 _handleSeed;

	int _volumeForSoundType[4];
	Channel *_channels[NUM_CHANNELS];


public:

	MixerImpl(uint32 sampleRate);
	~MixerImpl();

	virtual bool isReady() const {
		return _mixerReady;
	}

	virtual void playStream(
	    SoundType type,
	    SoundHandle *handle,
	    AudioStream *input,
	    int id, uint8 volume, sint8 balance,
	    DisposeAfterUse::Flag autofreeStream,
	    bool permanent,
	    bool reverseStereo);

	virtual void stopAll();
	virtual void stopID(int id);
	virtual void stopHandle(SoundHandle handle);

	virtual void pauseAll(bool paused);
	virtual void pauseID(int id, bool paused);
	virtual void pauseHandle(SoundHandle handle, bool paused);

	virtual bool isSoundIDActive(int id);
	virtual int getSoundID(SoundHandle handle);

	virtual bool isSoundHandleActive(SoundHandle handle);

	virtual void setChannelVolume(SoundHandle handle, uint8 volume);
	virtual void setChannelBalance(SoundHandle handle, sint8 balance);

	virtual uint32 getSoundElapsedTime(SoundHandle handle);
	virtual Timestamp getElapsedTime(SoundHandle handle);

	virtual bool hasActiveChannelOfType(SoundType type);

	virtual void setVolumeForSoundType(SoundType type, int volume);
	virtual int getVolumeForSoundType(SoundType type) const;

	virtual uint32 getOutputRate() const;

protected:
	void insertChannel(SoundHandle *handle, Channel *chan);

public:
	/**
	 * The mixer callback function, to be called at regular intervals by
	 * the backend (e.g. from an audio mixing thread). All the actual mixing
	 * work is done from here.
	 */
	void mixCallback(uint8 *samples, uint32 len);

	/**
	 * Set the internal 'is ready' flag of the mixer.
	 * Backends should invoke Mixer::setReady(true) once initialisation of
	 * their audio system has been completed.
	 */
	void setReady(bool ready);
};


} // End of namespace Audio

#endif
