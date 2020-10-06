/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/sound/sound_common.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/sound.h"
#include "engines/icb/p4.h"
#include "engines/icb/icb.h"

#include "audio/decoders/wave.h"

namespace ICB {

extern _stub stub;

MusicManager::MusicManager() {
	m_bufferLength = 0;
	m_wavByteOffsetInCluster = 0;
	m_wavDataSize = 0;

	m_paused = FALSE8;
	m_lengthInCycles = 0;

	m_identifier = 0;

	m_fading = 0;
	m_adjustFadeVol = 0;
#ifdef ENABLE_OPENAL
	alGenBuffers(1, &alStream.buffer);
	alGenSources(1, &alStream.source);
#endif
}

MusicManager::~MusicManager() {
	KillBuffer();
#ifdef ENABLE_OPENAL
	alDeleteSources(1, &alStream.source);
	alDeleteBuffers(1, &alStream.buffer);
#endif
}

bool8 MusicManager::UpdateMusic() {
	if (noSoundEngine)
		return TRUE8;

	// Don't do anything if we're paused or buffer invalid
	if (m_paused == FALSE8) {
		// Do fade out here

		// Has music been stopped?
		if (m_adjustFadeVol != 0) {
			// Have we faded down
			if (m_fading == 0) {
#ifdef ENABLE_OPENAL
				// Stop playback and reset cursor to start
				alSourceStop(alStream.source);
				alSourceRewind(alStream.source);
#endif
				m_adjustFadeVol = 0;
			} else {
				AdjustVolume(m_adjustFadeVol * -1);
				m_fading--;
			}
		}
	}

	return TRUE8;
}

bool8 MusicManager::LoadMusic(const char *clusterName, uint32 byteOffsetToWav, int32 vol) {
	if (noSoundEngine)
		return FALSE8;

	if (clusterName[0] == '\0')
		return FALSE8;

	// Check to see if we have this piece of music already loaded
	if (m_identifier == byteOffsetToWav) {
		// Record & Set the volume
		SetVolume(vol);
		return TRUE8;
	}

	// Stops and fills the buffer with silence
	KillBuffer();

	// A new music file overrides paused status
	m_paused = FALSE8;

	// Open the cluster file
	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(clusterName);
	if (stream == NULL)
		return FALSE8;

	// Need to seek to the correct postion in the cluster
	stream->seek(byteOffsetToWav, SEEK_SET);

	// Read in header information and make buffer
	if (!OpenMusic(stream)) {
		return FALSE8;
	}

	// Record & Set the volume
	SetVolume(vol);

	// As we're only dealing with one global music cluster, then this number
	// can be used safely to idenity all different music in the entire game
	m_identifier = byteOffsetToWav;

	return TRUE8;
}

bool8 MusicManager::StartMusic(const char *clusterName, uint32 byteOffsetToWav, int32 vol) {
	if (LoadMusic(clusterName, byteOffsetToWav, vol) == TRUE8) {
		// Regular playback
		m_adjustFadeVol = 0;

		if (PlayMusic() == TRUE8)
			return TRUE8;
	}

	return FALSE8;
}

bool8 MusicManager::PlayMusic() {
	if (noSoundEngine)
		return FALSE8;

	// Regular playback
	m_adjustFadeVol = 0;

	// Play the sound buffer
	g_icb->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, _audioStream);
	// Autofreed for now

	return TRUE8;
}

void MusicManager::StopMusic() {
	if (noSoundEngine)
		return;

	// Do nothing if we're already fading
	if (m_fading == 0 && IsPlaying()) {
		// Set the fade active
		m_fading = 10;
		m_adjustFadeVol = (int)(GetMusicVolume() / m_fading) + 1;
		if (m_adjustFadeVol == 0)
			m_adjustFadeVol = 1;
	}
}

void MusicManager::SetMusicPausedStatus(bool8 p) {
	if (noSoundEngine)
		return;

	if (p) {
		// Already paused
		if (!IsPlaying() && m_paused)
			return;

		// Stop playback leaving cursors alone
		g_icb->_mixer->pauseHandle(_handle, true);

		m_paused = TRUE8;
	} else {
		if (m_paused) {
			m_paused = FALSE8;

			// Play from where we left off
			g_icb->_mixer->pauseHandle(_handle, false);
		}
	}
}

void MusicManager::SetMusicVolume(int volume) {
	if (noSoundEngine)
		return;

	SetVolume(volume);
}

bool8 MusicManager::IsPlaying() {
	if (noSoundEngine)
		return FALSE8;

	if (g_icb->_mixer->isSoundHandleActive(_handle)) {
		return TRUE8;
	}

	return FALSE8;
}

void MusicManager::SetVolume(int volume) {
	// Store and set correct volume
	m_musicVol = volume;
#ifdef ENABLE_OPENAL
	alSourcef(alStream.source, AL_GAIN, ((float)volume) / 128.f);
#endif
}

void MusicManager::AdjustVolume(int amount) {
	// FIXME: proper calculations
#ifdef ENABLE_OPENAL
	alSourcef(alStream.source, AL_GAIN, ((float)amount) / 128.f);
#endif
}

bool8 MusicManager::OpenMusic(Common::SeekableReadStream *stream) {
	_wavHeader header;

	if (openWav(stream, header, m_wavDataSize, m_wavByteOffsetInCluster, m_lengthInCycles) == FALSE8) {
		return FALSE8;
	}
	_audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
#ifdef ENABLE_OPENAL
	// FIXME: (header.samplesPerSec * stub.cycle_speed)/100;
#endif
	return TRUE8;
}

void MusicManager::KillBuffer() {
	if (noSoundEngine)
		return;

	if (g_icb->_mixer->isSoundHandleActive(_handle)) {
		g_icb->_mixer->stopHandle(_handle);
	}

	m_identifier = 0;
}

} // End of namespace ICB
