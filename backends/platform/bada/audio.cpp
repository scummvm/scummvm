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

AudioThread::AudioThread() : mixer(0),
                             timer(0),
                             audioOut(0),
                             audioBufferIndex(0) {
}

Audio::MixerImpl* AudioThread::Construct(OSystem* system) {
  Osp::Base::Runtime::Thread::Construct(THREAD_TYPE_EVENT_DRIVEN);

  audioOut = new Osp::Media::AudioOut();
  if (!audioOut ||
      IsFailed(audioOut->Construct(*this))) {
    AppLog("Failed to create AudioOut");
    return null;
  }
  
  int bufferSize = audioOut->GetMinBufferSize();
  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
    if (IsFailed(audioBuffer[i].Construct(bufferSize))) {
      AppLog("Failed to create audio buffer");
      return null;
    }
  }

  timer = new Timer();
  if (!timer ||
      IsFailed(timer->Construct(*this)) ||
      IsFailed(timer->Start(TIMER_INTERVAL))) {
    AppLog("Failed to create audio timer");
    return null;
  }

  mixer = new Audio::MixerImpl(system, 44100);
  mixer->setReady(true);
}

AudioThread::~AudioThread() {
  if (audioOut) {
    delete audioOut;
  }
  if (timer) {
    delete timer;
  }
}

void AudioThread::OnAudioOutBufferEndReached(Osp::Media::AudioOut& src) {
  // TODO
  //r = src.WriteBuffer(*pWriteBuffer);
}

void AudioThread::OnAudioOutErrorOccurred(Osp::Media::AudioOut& src, result r) {
}

void AudioThread::OnAudioOutInterrupted(Osp::Media::AudioOut& src) {
}

void AudioThread::OnAudioOutReleased(Osp::Media::AudioOut& src) {
}

Osp::Base::Object* AudioThread::Run(void) {
  return null;
}

bool AudioThread::OnStart(void) {
  // TODO
  // result r = audioOut->Prepare(AUDIO_TYPE_PCM_U8, AUDIO_CHANNEL_TYPE_STEREO, 8000);
  return true;
}

void AudioThread::OnStop(void) {
  if (timer) {
    timer->Cancel();
  }
}

void AudioThread::OnTimerExpired(Timer& timer) {
  timer.Start(TIMER_INTERVAL);
  // TODO
  // r = __pAudioThread->WriteBuffer(__byteBuffer[0]);
}

