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
#include "bladerunner/game_constants.h"

#include "bladerunner/archive.h"
#include "bladerunner/savefile.h"
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
	assert(name.size() <= 12);
	if (loopId < 0) {
		warning("Overlays::play - loop id can't be a negative number!");
		return -1;
	}

	int32 hash = MIXArchive::getHash(name);
	int index = findByHash(hash);
	if (index < 0) {
		index = findEmpty();
		if (index < 0) {
			return index;
		}
		_videos[index].loaded = true;
		_videos[index].name = name;
		_videos[index].hash = hash;
		_videos[index].loopId = loopId;
		_videos[index].enqueuedLoopId = -1;
		_videos[index].loopForever = loopForever;
		_videos[index].vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceFront, Common::String::format("%s.VQA", name.c_str()));

		if (!_videos[index].vqaPlayer) {
			resetSingle(index);
			return -1;
		}
		// TODO? Removed as redundant
		// repeat forever
		//_videos[index].vqaPlayer->setBeginAndEndFrame(0, 0, -1, kLoopSetModeJustStart, nullptr, nullptr);
	}

	bool skipNewVQAPlayerOpen = false;
	if (_videos[index].vqaPlayer
	    && !startNow
	    && _videos[index].vqaPlayer->getFrameCount() > 0
	) {
		skipNewVQAPlayerOpen = true;
		_videos[index].enqueuedLoopId = loopId;
	}

	if (skipNewVQAPlayerOpen || _videos[index].vqaPlayer->open()) {
		_videos[index].vqaPlayer->setLoop(
			loopId,
			loopForever ? -1 : 0,
			startNow ? kLoopSetModeImmediate : kLoopSetModeEnqueue,
			nullptr, nullptr);
	} else {
		resetSingle(index);
		return -1;
	}
	return index;
}

void Overlays::resume(bool isLoadingGame) {

	for (int i = 0; i < kOverlayVideos; ++i) {
		if (_videos[i].loaded && isLoadingGame) {
			_videos[i].vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceFront, Common::String::format("%s.VQA", _videos[i].name.c_str()));
			if (!_videos[i].vqaPlayer) {
				resetSingle(i);
				continue;
			}

			_videos[i].vqaPlayer->open();
			_videos[i].vqaPlayer->setLoop(
				_videos[i].loopId,
				_videos[i].loopForever ? -1 : 0,
				kLoopSetModeImmediate,
				nullptr, nullptr);

			_videos[i].vqaPlayer->seekToFrame(_videos[i].frame);
			_videos[i].vqaPlayer->update(true);
		}
	}
}

void Overlays::remove(const Common::String &name) {
	int index = findByHash(MIXArchive::getHash(name));
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
			_videos[i].frame = _videos[i].vqaPlayer->update(true);
			if (_videos[i].frame < 0) {
				resetSingle(i);
			}
		}
	}
}

int Overlays::findByHash(int32 hash) const {
	for (int i = 0; i < kOverlayVideos; ++i) {
		if (_videos[i].loaded && _videos[i].hash == hash) {
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
	_videos[i].hash = 0;
	_videos[i].frame = -1;
	_videos[i].name.clear();
}

void Overlays::reset() {
	_videos.clear();
}

void Overlays::save(SaveFileWriteStream &f) {
	for (int i = 0; i < kOverlayVideos; ++i) {
		// 37 bytes per overlay
		Video &ov = _videos[i];

		f.writeBool(ov.loaded);
		f.writeInt(0); // vqaPlayer pointer
		f.writeStringSz(ov.name, 13);
		f.writeSint32LE(ov.hash);
		if (ov.enqueuedLoopId != -1) {
		// When there is an enqueued video, save that loop Id instead
			f.writeInt(ov.enqueuedLoopId);
		} else {
			f.writeInt(ov.loopId);
		}
		f.writeBool(ov.loopForever);
		f.writeInt(ov.frame);
	}
}

void Overlays::load(SaveFileReadStream &f) {
	for (int i = 0; i < kOverlayVideos; ++i) {
		// 37 bytes per overlay
		Video &ov = _videos[i];

		ov.loaded = f.readBool();
		f.skip(4); // vqaPlayer pointer
		ov.vqaPlayer = nullptr;
		ov.name = f.readStringSz(13);
		ov.hash = f.readSint32LE();
		ov.loopId = f.readInt();
		ov.loopForever = f.readBool();
		ov.frame = f.readInt();
	}
}

} // End of namespace BladeRunner
