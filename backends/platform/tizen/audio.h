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

#ifndef TIZEN_AUDIO_H
#define TIZEN_AUDIO_H

#include <FBase.h>
#include <FMedia.h>
#include <FIo.h>
#include <FBaseByteBuffer.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "audio/mixer_intern.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Media;
using namespace Tizen::Io;

#define NUM_AUDIO_BUFFERS 2

class AudioThread:
	public Tizen::Media::IAudioOutEventListener,
	public Tizen::Base::Runtime::ITimerEventListener,
	public Tizen::Base::Runtime::EventDrivenThread {

public:
	AudioThread(void);
	~AudioThread(void);

	Audio::MixerImpl *Construct();
	bool isSilentMode();
	void setMute(bool on);

private:
	bool OnStart(void);
	void OnStop(void);
	void OnAudioOutErrorOccurred(Tizen::Media::AudioOut &src, result r);
	void OnAudioOutInterrupted(Tizen::Media::AudioOut &src);
	void OnAudioOutReleased(Tizen::Media::AudioOut &src);
	void OnAudioOutBufferEndReached(Tizen::Media::AudioOut &src);
	void OnTimerExpired(Timer &timer);

	Audio::MixerImpl *_mixer;
	Tizen::Base::Runtime::Timer *_timer;
	Tizen::Media::AudioOut *_audioOut;
	Tizen::Base::ByteBuffer _audioBuffer[NUM_AUDIO_BUFFERS];
	int _head, _tail, _ready, _interval, _playing, _size;
	bool _muted;
};

#endif
