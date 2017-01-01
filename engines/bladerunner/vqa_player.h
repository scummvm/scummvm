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

#ifndef BLADERUNNER_VQA_PLAYER_H
#define BLADERUNNER_VQA_PLAYER_H

#include "bladerunner/vqa_decoder.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

namespace BladeRunner {

class BladeRunnerEngine;
class View;
class Lights;

class VQAPlayer {
	BladeRunnerEngine           *_vm;
	Common::SeekableReadStream  *_s;
	VQADecoder                   _decoder;
	const Graphics::Surface     *_surface;
	const uint16                *_zBuffer;
	Audio::QueuingAudioStream   *_audioStream;

	int _curFrame;
	int _decodedFrame;
	int _curLoop;
	int _loopBegin;
	int _loopEnd;

	uint32 _nextFrameTime;
	bool   _hasAudio;
	bool   _audioStarted;
	Audio::SoundHandle _soundHandle;

	void (*_callbackLoopEnded)(void*, int frame, int loopId);
	void  *_callbackData;

public:

	VQAPlayer(BladeRunnerEngine *vm)
		: _vm(vm),
		  _s(nullptr),
		  _surface(nullptr),
		  _audioStream(nullptr),
		  _curFrame(-1),
		  _decodedFrame(-1),
		  _curLoop(-1),
		  _loopBegin(-1),
		  _loopEnd(-1),
		  _nextFrameTime(0),
		  _hasAudio(false),
		  _audioStarted(false),
		  _callbackLoopEnded(nullptr) {
	}

	~VQAPlayer() {
		close();
	}

	bool open(const Common::String &name);
	void close();

	int  update();
	const Graphics::Surface *getSurface() const;
	const uint16 *getZBuffer() const;
	void updateView(View *view);
	void updateLights(Lights *lights);

	bool setLoop(int loop, int unknown, int loopMode, void(*callback)(void*, int, int), void* callbackData);

	int getLoopBeginFrame(int loop);
	int getLoopEndFrame(int loop);

private:
	int calcNextFrame(int frame) const;
	void queueAudioFrame(Audio::AudioStream *audioStream);
};

} // End of namespace BladeRunner

#endif
