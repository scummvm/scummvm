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

#include "bladerunner/overlays.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/archive.h"
#include "bladerunner/vqa_player.h"

#include "graphics/surface.h"

namespace BladeRunner {

Overlays::Overlays(BladeRunnerEngine *vm) {
	_vm = vm;
}

bool Overlays::init() {
	reset();
	_videos.resize(kOverlayVideos);

	for (int i = 0; i < kOverlayVideos; ++i) {
		_videos[i].vqaPlayer = nullptr;
		resetSingle(i);
	}

	return true;
}

Overlays::~Overlays() {
	for (int i = 0; i < kOverlayVideos; ++i) {
		resetSingle(i);
	}
	_videos.clear();
	reset();
}

int Overlays::play(const Common::String &name, int loopId, bool loopForever, bool startNow, int a6) {
	int id = mix_id(name);
	int index = findById(id);
	if (index < 0) {
		index = findEmpty();
		if (index < 0) {
			return index;
		}
		_videos[index].id = id;
		_videos[index].vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceFront);

		// repeat forever
		_videos[index].vqaPlayer->setBeginAndEndFrame(0, 0, -1, kLoopSetModeJustStart, nullptr, nullptr);
		_videos[index].loaded = true;
	}

	Common::String resourceName = Common::String::format("%s.VQA", name.c_str());
	_videos[index].vqaPlayer->open(resourceName);
	_videos[index].vqaPlayer->setLoop(
		loopId,
		loopForever ? -1 : 0,
		startNow ? kLoopSetModeImmediate : kLoopSetModeEnqueue,
		nullptr, nullptr);

	return index;
}

void Overlays::remove(const Common::String &name) {
	int id = mix_id(name);
	int index = findById(id);
	if (index >= 0) {
		resetSingle(index);
	}
}

void Overlays::removeAll() {
	for (int i = 0; i < kOverlayVideos; ++i) {
		if (_videos[i].loaded) {
			resetSingle(i);
		}
	}
}

void Overlays::tick() {
	for (int i = 0; i < kOverlayVideos; ++i) {
		if (_videos[i].loaded) {
			int frame = _videos[i].vqaPlayer->update(true);
			if (frame < 0) {
				resetSingle(i);
			}
		}
	}
}

int Overlays::findById(int32 id) const {
	for (int i = 0; i < kOverlayVideos; ++i) {
		if (_videos[i].loaded && _videos[i].id == id) {
			return i;
		}
	}
	return -1;
}

int Overlays::findEmpty() const {
	for (int i = 0; i < kOverlayVideos; ++i) {
		if (!_videos[i].loaded) {
			return i;
		}
	}
	return -1;
}

void Overlays::resetSingle(int i) {
	assert(i >= 0 && i < (int)_videos.size());
	if (_videos[i].vqaPlayer) {
		delete _videos[i].vqaPlayer;
		_videos[i].vqaPlayer = nullptr;
	}
	_videos[i].loaded = false;
	_videos[i].id = 0;
	_videos[i].field2 = -1;
}

void Overlays::reset() {
	_videos.clear();
}

} // End of namespace BladeRunner
