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

class VQAPlayer {
	BladeRunnerEngine           *_vm;
	Common::SeekableReadStream  *_s;
	VQADecoder                   _decoder;
	Graphics::Surface           *_surface;
	Audio::QueuingAudioStream   *_audioStream;

	int _curFrame;
	int _curLoop;
	int _loopSpecial;
	int _loopDefault;

	uint32 _nextFrameTime;
	bool   _audioStarted;
	Audio::SoundHandle _soundHandle;

public:

	VQAPlayer(BladeRunnerEngine *vm, Graphics::Surface *surface)
		: _vm(vm),
		  _s(nullptr),
		  _surface(surface),
		  _audioStream(nullptr),
		  _curFrame(-1),
		  _curLoop(-1),
		  _loopSpecial(-1),
		  _loopDefault(-1),
		  _nextFrameTime(0),
		  _audioStarted(false)
	{}

	bool open(const Common::String &name);
	int  update();

	void setLoopSpecial(int loop, bool wait);
	void setLoopDefault(int loop);
};

}; // End of namespace BladeRunner

#endif
