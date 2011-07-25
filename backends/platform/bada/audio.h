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

#ifndef AUDIO_H
#define AUDIO_H

#include <FBase.h>
#include <FMedia.h>
#include <FIo.h>
#include <FBaseByteBuffer.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "audio/mixer_intern.h"

using namespace Osp::Base;
using namespace Osp::Base::Collection;
using namespace Osp::Base::Runtime;
using namespace Osp::Media;
using namespace Osp::Io;

#define NUM_AUDIO_BUFFERS 3

class AudioThread: public Osp::Media::IAudioOutEventListener,
                   public Osp::Base::Runtime::ITimerEventListener, 
                   public Osp::Base::Runtime::Thread {
public:
  AudioThread(void);
  ~AudioThread(void);

  Audio::MixerImpl* Construct(OSystem* system);
  bool isSilentMode();
  void setMute(bool on);
  int setVolume(bool up, bool minMax, int range);

  bool OnStart(void);
  void OnStop(void);
  void OnAudioOutErrorOccurred(Osp::Media::AudioOut& src, result r);
  void OnAudioOutInterrupted(Osp::Media::AudioOut& src);
  void OnAudioOutReleased(Osp::Media::AudioOut& src);
  void OnAudioOutBufferEndReached(Osp::Media::AudioOut& src);
  void OnTimerExpired(Timer& timer);
  
 private:
  Audio::MixerImpl* mixer;
  Osp::Base::Runtime::Timer* timer;
	Osp::Media::AudioOut* audioOut;
	Osp::Base::ByteBuffer audioBuffer[NUM_AUDIO_BUFFERS];
  int head, tail, ready, interval, muteVol;
  bool playing;
};

#endif
