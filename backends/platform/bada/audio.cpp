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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio.h"
#include "system.h"

#include <FSysSettingInfo.h>

AudioThread::AudioThread() : 
  mixer(0),
  timer(0),
  audioOut(0),
  head(0),
  tail(0),
  ready(0),
  interval(TIMER_INTERVAL),
  playing(false) {
}

Audio::MixerImpl* AudioThread::Construct(OSystem* system) {
  logEntered();

  if (IsFailed(Thread::Construct(THREAD_TYPE_EVENT_DRIVEN))) {
    AppLog("Failed to create AudioThread");
    return null;
  }

  mixer = new Audio::MixerImpl(system, 44100);
  return mixer;
}

AudioThread::~AudioThread() {
  logEntered();
}

void AudioThread::setVolume(bool up) {
  if (audioOut) {
    int volume = audioOut->GetVolume();
    if (volume) {
      volume += up ? 10 : -10;
      if (volume > 0 && volume < 90) {
        audioOut->SetVolume(volume);
      }
    }
  }
}

bool AudioThread::OnStart(void) {
  logEntered();

  audioOut = new Osp::Media::AudioOut();
  if (!audioOut ||
      IsFailed(audioOut->Construct(*this))) {
    AppLog("Failed to create AudioOut");
    return false;
  }

  int sampleRate = mixer->getOutputRate();
  if (IsFailed(audioOut->Prepare(AUDIO_TYPE_PCM_S16_LE,
                                 AUDIO_CHANNEL_TYPE_STEREO,
                                 sampleRate))) {
    AppLog("Failed to prepare AudioOut %d", sampleRate);
    return false;
  }

  int bufferSize = audioOut->GetMinBufferSize();
  AppLog("bufferSize = %d", bufferSize);

  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
    if (IsFailed(audioBuffer[i].Construct(bufferSize))) {
      AppLog("Failed to create audio buffer");
      return false;
    }
  }

  timer = new Timer();
  if (!timer || IsFailed(timer->Construct(*this))) {
    AppLog("Failed to create audio timer");
    return false;
  }

  if (IsFailed(timer->Start(interval))) {
    AppLog("failed to start audio timer");
    return false;
  }

  bool silentMode;
  String key(L"SilentMode");
  Osp::System::SettingInfo::GetValue(key, silentMode);

  mixer->setReady(true);
  audioOut->SetVolume(silentMode ? 0 : 40);
  audioOut->Start();
  return true;
}

void AudioThread::OnStop(void) {
  logEntered();

  mixer->setReady(false);

  if (timer) {
    timer->Cancel();    
    delete timer;
  }

  if (audioOut) {
    audioOut->Reset();
    delete audioOut;
  }
}

void AudioThread::OnAudioOutErrorOccurred(Osp::Media::AudioOut& src, result r) {
  logEntered();
}

void AudioThread::OnAudioOutInterrupted(Osp::Media::AudioOut& src) {
  logEntered();
}

void AudioThread::OnAudioOutReleased(Osp::Media::AudioOut& src) {
  logEntered();
}

void AudioThread::OnAudioOutBufferEndReached(Osp::Media::AudioOut& src) {
  if (ready > 0) {
    playing = true;
    audioOut->WriteBuffer(audioBuffer[tail]);
    tail = (tail + 1 == NUM_AUDIO_BUFFERS ? 0 : tail + 1);
    ready--;
  }
  else {
    // audio buffer empty: decrease timer inverval
    playing = false;
    interval -= TIMER_INCREMENT;
    if (interval < MIN_TIMER_INTERVAL) {
      interval = MIN_TIMER_INTERVAL;
    }
  }
}

void AudioThread::OnTimerExpired(Timer& timer) {
  if (ready < NUM_AUDIO_BUFFERS) {
    uint len = audioBuffer[head].GetCapacity();
    int samples = mixer->mixCallback((byte *) audioBuffer[head].GetPointer(), len);
    if (samples) {
      head = (head + 1 == NUM_AUDIO_BUFFERS ? 0 : head + 1);
      ready++;
    }
  }
  else {
    // audio buffer full: increase timer inverval
    interval += TIMER_INCREMENT;
  }

  if (ready && !playing) {
    OnAudioOutBufferEndReached(*audioOut);
  }
  
  timer.Start(interval);
}

//
// end of audio.cpp 
//
