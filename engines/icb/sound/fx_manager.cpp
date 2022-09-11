/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/p4.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/sound/sound_common.h"
#include "engines/icb/icb.h"
#include "engines/icb/global_objects.h"

#include "common/textconsole.h"

#include "audio/decoders/wave.h"

namespace ICB {

// externals
extern _stub stub;

bool8 noSoundEngine = FALSE8;

// Nearest integer macro
#define NEAREST_INT(X) (X) > 0.0 ? int((X) + 0.5) : int((X)-0.5)

FxManager::FxManager() {
	for (int32 id = 0; id < MAX_FX; id++) {
		memset(m_effects[id].name, 0, SAMPLE_NAME_LENGTH);
		m_effects[id].flags = Effect::EMPTY;
		m_effects[id].delay = 0;
		m_effects[id].looped = 0;
		m_effects[id].pitch = 0;
		m_effects[id].pan = 0;
		m_effects[id].volume = 0;
		m_effects[id]._stream = nullptr;
	}
}

FxManager::~FxManager() {
	// Remove any memory resident sounds
	UnregisterAll();
}

bool8 FxManager::Poll() {
	if (noSoundEngine)
		return TRUE8;

	// Update all buffers
	for (int32 id = 0; id < MAX_FX; id++) {
		switch (m_effects[id].flags) {
		// If it's playing check it hasn't finished
		case Effect::PLAYING:
			// Apply current settings
			if (g_icb->_mixer->isSoundHandleActive(m_effects[id]._handle)) {
				float volumeConversion = Audio::Mixer::kMaxChannelVolume / 128.0f;
				g_icb->_mixer->setChannelVolume(m_effects[id]._handle, m_effects[id].volume * volumeConversion);
				g_icb->_mixer->setChannelBalance(m_effects[id]._handle, m_effects[id].pan);

				// FIXME correct pitch control
				//uint32 frequency = (m_effects[id].pitch * g_stub->cycle_speed) / 100;
				//m_effects[id].buffer->SetFrequency( frequency ) ;
				//alSourcef(m_effects[id].alStream.source, AL_PITCH, 1.0f);
			}

			if (!g_icb->_mixer->isSoundHandleActive(m_effects[id]._handle)) {
				// Finished playing so ready to go again
				m_effects[id].flags = Effect::READY;
			}
			break;

		// It's currently delayed
		case Effect::DELAYED:

			m_effects[id].delay--;
			if (m_effects[id].delay != 0)
				break;
			// falls through

		// It's waiting to play
		case Effect::QUEUED: {

			// Apply current settings
			// FIXME correct pitch control
			//uint32 frequency = (m_effects[id].pitch * g_stub->cycle_speed) / 100;
			//m_effects[id].buffer->SetFrequency( frequency ) ;
			//alSourcef(m_effects[id].alStream.source, AL_PITCH, 1.0f);

			// So play it
			float volumeConversion = Audio::Mixer::kMaxChannelVolume / 128.0f;
			g_icb->_mixer->playStream(Audio::Mixer::kSFXSoundType, &m_effects[id]._handle,
			                          makeLoopingAudioStream(m_effects[id]._stream, (m_effects[id].looped != 0) ? 0 : 1),
			                          -1, m_effects[id].volume * volumeConversion, m_effects[id].pan, DisposeAfterUse::NO);
			m_effects[id].flags = Effect::PLAYING;
			}
			break;
		default:
			break;
		}
	}
	return TRUE8;
}

int32 FxManager::Register(const int32 id, const char *name, const int32 delay, uint32 byteOffsetInCluster) {
	if (noSoundEngine)
		return 0;

	// Create the sound buffers
	if (!Load(id, name, byteOffsetInCluster)) {
		warning("sounds.txt: can't load \"%s\"", name);
		return -1;
	}

	// Record the samples name so we know it is currently loaded in memory
	strcpy(m_effects[id].name, name);

	// Setup the delay if there is one
	m_effects[id].delay = delay;

	if (delay)
		m_effects[id].flags = Effect::DELAYED;
	else
		m_effects[id].flags = Effect::READY;

	// Ok were done
	return id;
}

void FxManager::Unregister(int32 id) {
	if (noSoundEngine)
		return;

	if (g_icb->_mixer->isSoundHandleActive(m_effects[id]._handle)) {
		g_icb->_mixer->stopHandle(m_effects[id]._handle);
	}

	delete m_effects[id]._stream;
	m_effects[id]._stream = nullptr;

	memset(m_effects[id].name, 0, SAMPLE_NAME_LENGTH);
	m_effects[id].flags = Effect::EMPTY;
}

void FxManager::Play(int32 id) {
	if (noSoundEngine)
		return;

	if (m_effects[id].flags == Effect::READY) {
		// Queue the sound to be played
		m_effects[id].flags = Effect::QUEUED;
	}
}

void FxManager::SetVolume(int32 id, int32 vol) {
	if (noSoundEngine)
		return;

	m_effects[id].volume = vol;
}

void FxManager::SetPan(int32 id, int32 pan) {
	if (noSoundEngine)
		return;

	m_effects[id].pan = pan;
}

void FxManager::SetPitch(int32 id, int32 pitch) {
	if (noSoundEngine)
		return;

	m_effects[id].pitch = pitch;
}

void FxManager::Stop(int32 id) {
	if (noSoundEngine)
		return;

	if (m_effects[id].flags == Effect::PLAYING) {
		// Halt playback and reset position
		g_icb->_mixer->stopHandle(m_effects[id]._handle);
		m_effects[id]._stream->rewind();
		m_effects[id].flags = Effect::READY;
	}
}

void FxManager::StopAll() {
	if (noSoundEngine)
		return;

	for (int32 id = 0; id < MAX_FX; id++) {
		if (m_effects[id].flags == Effect::PLAYING) {
			// Halt playback and reset position
			g_icb->_mixer->stopHandle(m_effects[id]._handle);
			m_effects[id]._stream->rewind();
			m_effects[id].flags = Effect::READY;
		}
	}
}

void FxManager::UnregisterAll() {
	if (noSoundEngine)
		return;

	for (int32 id = 0; id < MAX_FX; id++) {
		Unregister(id);

		delete m_effects[id]._stream;
		m_effects[id]._stream = nullptr;
	}
}

int32 FxManager::GetDefaultRate(const char *name, uint32 byteOffsetInCluster) {
	int32 rateToFind = 0;
	int32 id;

	for (id = 0; id < MAX_FX; id++) {
		// Do we have this sample loaded in memory
		if (strcmp(m_effects[id].name, name) == 0)
			break;
	}

	if (id == MAX_FX)
		rateToFind = GetDefaultRateByName(name, byteOffsetInCluster);
	else
		rateToFind = GetDefaultRateByID(id);

	return rateToFind;
}

int32 FxManager::GetDefaultRateByName(const char * /*name*/, uint32 byteOffsetInCluster) {
	_wavHeader header;

	// Open the cluster file and seek to the start of the sample
	const char *clusterName = (const char *)pxVString("g\\samples.clu");

	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(clusterName);
	if (!stream)
		return 0;

	stream->seek(byteOffsetInCluster, SEEK_SET);

	// Read in the wave header
	if (stream->read(&header, sizeof(_wavHeader)) != sizeof(_wavHeader)) {
		delete stream;
		return 0;
	}

	// Close the file
	delete stream;

	// Return the wavs sampling rate
	return (FROM_LE_32(header.samplesPerSec));
}

bool8 FxManager::Load(int32 id, const char * /*name*/, uint32 byteOffsetInCluster) {
	_wavHeader header;
	uint32 length;
	int32 lengthInCycles;

	// Open the cluster file and seek to the start of the sample
	const char *clusterName = (const char *)pxVString("g\\samples.clu");

	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(clusterName);
	if (!stream)
		return FALSE8;

	stream->seek(byteOffsetInCluster, SEEK_SET);

	if (openWav(stream, header, length, byteOffsetInCluster, lengthInCycles) != TRUE8) {
		delete stream;
		return FALSE8;
	}

	// Straighten out the block align. (someties it's per second and sometime per sample)
	if (FROM_LE_16(header.blockAlign) > 16)
		header.blockAlign = TO_LE_16((uint16)(FROM_LE_16(header.channels) * FROM_LE_16(header.bitsPerSample) / 8));

	// Store buffer sampling rate for easy access later
	m_effects[id].rate = FROM_LE_32(header.samplesPerSec);
	m_effects[id]._stream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	if (m_effects[id].rate != 0)
		m_effects[id].length = 500 * length / m_effects[id].rate;
	else
		m_effects[id].length = 0;

	return TRUE8;
}

} // End of namespace ICB
