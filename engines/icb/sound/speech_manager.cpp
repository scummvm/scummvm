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

#include "engines/icb/sound/speech_manager.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/sound/fx_manager.h"
#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/debug.h"
#include "engines/icb/p4.h"
#include "engines/icb/sound/sound_common.h"
#include "engines/icb/icb.h"

#include "common/textconsole.h"

#include "audio/mixer.h"
#include "audio/decoders/wave.h"

namespace ICB {

SpeechManager::SpeechManager() {
	m_wavByteOffsetInCluster = 0;
	m_wavDataSize = 0;

	m_paused = FALSE8;
	m_lengthInCycles = 0;
	_audioStream = NULL;
	m_speechVol = 0;
}

SpeechManager::~SpeechManager() { KillBuffer(); }

bool8 SpeechManager::IsPlaying() {
	if (noSoundEngine)
		return FALSE8;

	if (g_icb->_mixer->isSoundHandleActive(_handle)) {
		return TRUE8;
	}
	return FALSE8;
}

bool8 SpeechManager::StartSpeech(const char *fileName, uint32 byteOffsetToWav, int32 vol) {
	if (noSoundEngine)
		return FALSE8;

	if (fileName[0] == '\0')
		return FALSE8;

	// Stops and fills the buffer with silence
	KillBuffer();

	// A new speech file overrides paused status
	m_paused = FALSE8;

	// Open the cluster file
	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(fileName);
	if (stream == NULL)
		return FALSE8;

	// Need to seek to the correct postion in the cluster
	stream->seek(byteOffsetToWav, SEEK_SET);

	// Read in header information and make buffer
	if (!OpenSpeech(stream)) {
		warning("SpeechManager::OpenStream(%s, header) failed", fileName);
		return FALSE8;
	}

	// Record & Set the volume
	SetVolume(vol);

	g_icb->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_handle, _audioStream);

	return TRUE8;
}

bool8 SpeechManager::UpdateSpeech() {
	if (noSoundEngine)
		return TRUE8;

	// Don't do anything if we're paused or buffer invalid
	if (m_paused == FALSE8) {
		if (IsPlaying() == FALSE8)
			KillBuffer();
	}

	return TRUE8;
}

void SpeechManager::StopSpeech() {
	if (noSoundEngine)
		return;

	KillBuffer();
}

void SpeechManager::PauseSpeech() {
	if (noSoundEngine)
		return;

	if (m_paused || !IsPlaying())
		return;

	g_icb->_mixer->pauseHandle(_handle, true);
	m_paused = TRUE8;
}

void SpeechManager::ResumeSpeech() {
	if (noSoundEngine)
		return;

	if (m_paused) {
		m_paused = FALSE8;
		g_icb->_mixer->pauseHandle(_handle, false);
	}
}

void SpeechManager::SetSpeechVolume(int volume) {
	if (noSoundEngine)
		return;

	SetVolume(volume);
}

void SpeechManager::SetVolume(int volume) {
	float volumeConversion = Audio::Mixer::kMaxChannelVolume / 128.0f;
	g_icb->_mixer->setChannelVolume(_handle, volume * volumeConversion);
}

bool8 SpeechManager::OpenSpeech(Common::SeekableReadStream *stream) {
	_wavHeader header;

	// Get the length etc.
	if (openWav(stream, header, m_wavDataSize, m_wavByteOffsetInCluster, m_lengthInCycles) != TRUE8) {
		return FALSE8;
	}

	// _audioStream contains the latest Opened WAV, which should be played immediately after, and
	// thus cleaned by the mixer when done.
	_audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	return TRUE8;
}

void SpeechManager::KillBuffer() {
	if (noSoundEngine)
		return;
	if (g_icb->_mixer->isSoundHandleActive(_handle))
		g_icb->_mixer->stopHandle(_handle);
	// No need to free _audioStream as it is auto-freed by the mixer.
}

} // End of namespace ICB
