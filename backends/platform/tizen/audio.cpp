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

#include <FSysSettingInfo.h>
#include <FAppAppRegistry.h>

#include "backends/platform/tizen/audio.h"
#include "backends/platform/tizen/system.h"

#define TIMER_INTERVAL		10
#define VOLUME				96
#define MIN_TIMER_INTERVAL	5

AudioThread::AudioThread() :
	_mixer(0),
	_timer(0),
	_audioOut(0),
	_head(0),
	_tail(0),
	_ready(0),
	_interval(TIMER_INTERVAL),
	_playing(-1),
	_size(0),
	_muted(true) {
}

Audio::MixerImpl *AudioThread::Construct() {
	logEntered();

	if (IsFailed(EventDrivenThread::Construct(DEFAULT_STACK_SIZE, THREAD_PRIORITY_HIGH))) {
		AppLog("Failed to create AudioThread");
		return NULL;
	}

	_mixer = new Audio::MixerImpl(44100);
	return _mixer;
}

AudioThread::~AudioThread() {
	logEntered();
}

bool AudioThread::isSilentMode() {
	bool silentMode;
	String key(L"SilentMode");
	Tizen::System::SettingInfo::GetValue(key, silentMode);
	return silentMode;
}

void AudioThread::setMute(bool on) {
	if (_audioOut && _timer) {
		_muted = on;
		if (on) {
			_timer->Cancel();
		} else {
			_timer->Start(_interval);
		}
	}
}

bool AudioThread::OnStart(void) {
	logEntered();

	_audioOut = new Tizen::Media::AudioOut();
	if (!_audioOut || IsFailed(_audioOut->Construct(*this))) {
		AppLog("Failed to create AudioOut");
		return false;
	}

	int sampleRate = _mixer->getOutputRate();

	// ideally we would update _mixer with GetOptimizedSampleRate here
	if (_audioOut->GetOptimizedSampleRate() != sampleRate) {
		AppLog("Non optimal sample rate %d", _audioOut->GetOptimizedSampleRate());
	}

	if (IsFailed(_audioOut->Prepare(AUDIO_TYPE_PCM_S16_LE,
			AUDIO_CHANNEL_TYPE_STEREO, sampleRate))) {
		AppLog("Failed to prepare AudioOut %d", sampleRate);
		return false;
	}

	int bufferSize = _audioOut->GetMinBufferSize();
	for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
		if (IsFailed(_audioBuffer[i].Construct(bufferSize))) {
			AppLog("Failed to create audio buffer");
			return false;
		}
	}

	_size = _audioBuffer[0].GetCapacity();
	_timer = new Timer();
	if (!_timer || IsFailed(_timer->Construct(*this))) {
		AppLog("Failed to create audio timer");
		return false;
	}

	if (IsFailed(_timer->Start(_interval))) {
		AppLog("failed to start audio timer");
		return false;
	}

	_muted = false;
	_mixer->setReady(true);
	_audioOut->SetVolume(isSilentMode() ? 0 : VOLUME);
	_audioOut->Start();
	return true;
}

void AudioThread::OnStop(void) {
	logEntered();

	_mixer->setReady(false);

	if (_timer) {
		if (!_muted) {
			_timer->Cancel();
		}
		delete _timer;
	}

	if (_audioOut) {
		_audioOut->Reset();
		_audioOut->Unprepare();
		delete _audioOut;
	}
}

void AudioThread::OnAudioOutErrorOccurred(Tizen::Media::AudioOut &src, result r) {
	logEntered();
}

void AudioThread::OnAudioOutInterrupted(Tizen::Media::AudioOut &src) {
	logEntered();
}

void AudioThread::OnAudioOutReleased(Tizen::Media::AudioOut &src) {
	logEntered();
	_audioOut->Start();
}

void AudioThread::OnAudioOutBufferEndReached(Tizen::Media::AudioOut &src) {
	if (_ready > 0) {
		_playing = _tail;
		_audioOut->WriteBuffer(_audioBuffer[_tail]);
		_tail = (_tail + 1) % NUM_AUDIO_BUFFERS;
		_ready--;
	} else {
		// audio buffer empty: decrease timer interval
		_playing = -1;
		_interval -= 1;
		if (_interval < MIN_TIMER_INTERVAL) {
			_interval = MIN_TIMER_INTERVAL;
		}
	}

}

void AudioThread::OnTimerExpired(Timer &timer) {
	if (_ready < NUM_AUDIO_BUFFERS) {
		if (_playing != _head) {
			if (_mixer->mixCallback((byte *)_audioBuffer[_head].GetPointer(), _size)) {
				_head = (_head + 1) % NUM_AUDIO_BUFFERS;
				_ready++;
			}
		}
	} else {
		// audio buffer full: restore timer interval
		_interval = TIMER_INTERVAL;
	}

	if (_ready && _playing == -1) {
		OnAudioOutBufferEndReached(*_audioOut);
	}

	_timer->Start(_interval);
}

