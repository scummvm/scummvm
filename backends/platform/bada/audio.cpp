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
#include <FAppAppRegistry.h>

#define TIMER_INCREMENT    10
#define TIMER_INTERVAL     80
#define MIN_TIMER_INTERVAL 20
#define MAX_VOL            99
#define INIT_VOL           40
#define CONFIG_KEY         L"audiovol"

// sound level pre-sets
const int levels[] = {2, 20, INIT_VOL, 70, MAX_VOL};

AudioThread::AudioThread() : 
  mixer(0),
  timer(0),
  audioOut(0),
  head(0),
  tail(0),
  ready(0),
  interval(TIMER_INTERVAL),
  muteVol(0),
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

bool AudioThread::isSilentMode() {
  bool silentMode;
  String key(L"SilentMode");
  Osp::System::SettingInfo::GetValue(key, silentMode);
  return silentMode;
}

void AudioThread::setMute(bool on) {
  if (audioOut && !isSilentMode()) {
    if (on) {
      muteVol = audioOut->GetVolume();
      audioOut->SetVolume(0);
    }
    else {
      audioOut->SetVolume(muteVol);
    }
  }
}

int AudioThread::setVolume(bool up, bool minMax) {
  int level = -1;
  int numLevels = sizeof(levels);

  if (audioOut && !isSilentMode()) {
    int volume = audioOut->GetVolume();
    if (minMax) {
      level = up ? numLevels - 1 : 0;
      volume = levels[level];
    }
    else {
      // adjust volume to be in unit steps
      for (int i = 0; i < numLevels && level == -1; i++) {
        if (volume == levels[i]) {
          if (up) {
            if (i + 1 < numLevels) {
              level = i + 1;
            }
          }
          else if (i > 0) {
            level = i - 1;
          }
        }
      }
      
      if (level != -1) {
        volume = levels[level];
      }
    }
    if (volume <= MAX_VOL) {
      audioOut->SetVolume(volume);

      // remember the chosen setting
      AppRegistry* registry = Application::GetInstance()->GetAppRegistry();
      if (registry) {
        registry->Set(CONFIG_KEY, volume);
      }
    }
  }
  return level;
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

  // get the volume from the app-registry
  int volume = INIT_VOL;
  AppRegistry* registry = Application::GetInstance()->GetAppRegistry();
  if (registry) {
    if (E_KEY_NOT_FOUND == registry->Get(CONFIG_KEY, volume)) {
      registry->Add(CONFIG_KEY, volume);
      volume = INIT_VOL;
    }
    else {
      AppLog("Setting volume: %d", volume);
    }
  }

  mixer->setReady(true);
  audioOut->SetVolume(isSilentMode() ? 0 : volume);
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
