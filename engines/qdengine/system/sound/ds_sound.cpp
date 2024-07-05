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

#include "qdengine/qd_precomp.h"
#include "qdengine/system/sound/ds_sound.h"
#include "qdengine/system/sound/wav_sound.h"

/* ---------------------------- INCLUDE SECTION ----------------------------- */


/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */
namespace QDEngine {

dsSound::dsSound(const sndSound &snd, LPDIRECTSOUND sound_device) : sndSound(snd),
	sound_device_(sound_device),
	sound_buffer_(NULL),
	flags_(0) {
}

dsSound::dsSound(const dsSound &snd) : sndSound(snd),
	sound_device_(snd.sound_device_),
	sound_buffer_(snd.sound_buffer_),
	flags_(snd.flags_) {
}

dsSound::~dsSound() {
	release_sound_buffer();
}

bool dsSound::create_sound_buffer() {
	if (!sound())
		return false;

	warning("STUB: dsSound::create_sound_buffer()");
#if 0
	WAVEFORMATEX wfx;

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = WORD(sound()->channels());
	wfx.nSamplesPerSec = sound()->samples_per_sec();
	wfx.wBitsPerSample = WORD(sound()->bits_per_sample());
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;

	DSBUFFERDESC desc;
	memset(&desc, 0, sizeof(DSBUFFERDESC));

	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	desc.dwBufferBytes = sound()->data_length();
	desc.lpwfxFormat = &wfx;

	release_sound_buffer();

	HRESULT res = sound_device_->CreateSoundBuffer(&desc, &sound_buffer_, NULL);
	if (FAILED(res))
		return false;

	LPVOID ptr_1 = NULL, ptr_2 = NULL;
	DWORD size_1, size_2;
	res = sound_buffer_->Lock(0, sound()->data_length(), &ptr_1, &size_1, &ptr_2, &size_2, 0L);
	if (FAILED(res) || ptr_1 == NULL)
		return false;

	memcpy(ptr_1, sound()->data(), sound()->data_length());

	res = sound_buffer_->Unlock(ptr_1, sound()->data_length(), NULL, 0L);
	if (FAILED(res))
		return false;

	sound_buffer_->SetCurrentPosition(0);
#endif

	return true;
}

bool dsSound::release_sound_buffer() {
	if (sound_buffer_) {
		if (!is_stopped())
			stop();

		warning("STUB: dsSound::release_sound_buffer()");
#if 0
		sound_buffer_->Release();
		sound_buffer_ = NULL;
#endif
	}

	return true;
}

bool dsSound::play() {
	flags_ &= ~SOUND_FLAG_PAUSED;

	if (!sound_buffer_) return false;
	warning("STUB: dsSound::play()");
#if 0
	DWORD flags = (flags_ & SOUND_FLAG_LOOPING) ? DSBPLAY_LOOPING : 0;
	sound_buffer_->Play(0, 0, flags);
#endif
	return true;
}

bool dsSound::stop() {
	if (!sound_buffer_) return false;
	warning("STUB: dsSound::stop()");
#if 0
	sound_buffer_->Stop();
#endif
	return true;
}

void dsSound::pause() {
	flags_ |= SOUND_FLAG_PAUSED;
	stop();
}

void dsSound::resume() {
	play();
}

sndSound::status_t dsSound::status() const {
	if (!sound_buffer_) return sndSound::SOUND_STOPPED;

	if (is_paused()) return sndSound::SOUND_PAUSED;
	warning("STUB: dsSound::status()");
#if 0
	DWORD st;
	sound_buffer_->GetStatus(&st);

	if (st & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)) return SOUND_PLAYING;
#endif
	return SOUND_STOPPED;
}

bool dsSound::is_stopped() const {
	switch (status()) {
	case SOUND_PLAYING:
	case SOUND_PAUSED:
		return false;
	default:
		return true;
	}
}

bool dsSound::set_volume(int vol) {
	if (!sound_buffer_) return false;
	warning("STUB: dsSound::set_volume()");
#if 0
	sound_buffer_->SetVolume(vol);
#endif
	return true;
}

bool dsSound::change_frequency(float coeff) {
	if (!sound_buffer_) return false;
	warning("STUB: dsSound::change_frequency()");
#if 0
	DWORD freq;
	if (sound_buffer_->GetFrequency(&freq) != DS_OK)
		return false;

	freq = round(float(freq) * coeff);
	if (freq > DSBFREQUENCY_MAX)
		freq = DSBFREQUENCY_MAX;
	else if (freq < DSBFREQUENCY_MIN)
		freq = DSBFREQUENCY_MIN;

	if (sound_buffer_->SetFrequency(freq) != DS_OK)
		return false;
#endif
	return true;
}

float dsSound::position() const {
	if (!sound_buffer_) return 0.0f;
	warning("STUB: dsSound::position()");
#if 0
	DWORD pos = 0;
	if (sound_buffer_->GetCurrentPosition(&pos, NULL) == DS_OK) {
		float norm_pos = float(pos) / float(sound()->data_length());

		if (norm_pos < 0.0f) norm_pos = 0.0f;
		if (norm_pos > 1.0f) norm_pos = 1.0f;

		return norm_pos;
	}
#endif
	return 0.0f;
}

bool dsSound::set_position(float pos) {
	if (sound_buffer_) {
		warning("STUB: dsSound::set_position()");
#if 0
		DWORD npos = DWORD(float(sound()->data_length() * pos));

		if (sound_buffer_->SetCurrentPosition(npos) == DS_OK)
			return true;
#endif
	}

	return false;
}
} // namespace QDEngine
