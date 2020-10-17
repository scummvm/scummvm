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

#include "agds/character.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/resourceManager.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace AGDS {

void Character::load(Common::SeekableReadStream* stream) {
	debug("loading character...");
	stream->readUint32LE(); //unk
	uint16 magic = stream->readUint16LE();
	uint n;
	switch(magic) {
		case 0xdead:
			n = 16;
			break;
		case 0x8888:
			n = 8;
			break;
		default:
			error("invalid magic %04x", magic);
	}

	_animations.clear();
	while(stream->pos() < stream->size()) {
		uint size = stream->readUint32LE();
		uint index = stream->readUint16LE();
		debug("header size %u, index: %u", size, index);

		uint16 frames = stream->readUint16LE();
		uint16 format = stream->readUint16LE();
		Common::String filename = readString(stream);

		AnimationDescription animation;
		animation.filename = filename;
		debug("animation %s, frames: %d, format: %d", animation.filename.c_str(), frames, format);
		while(frames--) {
			int x = stream->readSint16LE();
			int y = stream->readSint16LE();
			int w = stream->readUint32LE();
			int h = stream->readUint32LE();
			AnimationDescription::Frame frame = { x, y, w, h };
			animation.frames.push_back(frame);
			debug("frame %d, %d, %dx%d", x, y, w, h);
			uint unk1 = stream->readUint32LE();
			uint unk2 = stream->readUint32LE();
			uint unk3 = stream->readUint32LE();
			uint unk4 = stream->readUint32LE(); //GRP file offset?
			uint unk5 = stream->readUint32LE();
			uint unk6 = stream->readByte();
			uint unk7 = stream->readUint32LE();
			uint unk8 = stream->readUint32LE();
			stream->readUint32LE(); //CDCDCDCD
			uint unk9 = stream->readUint32LE();
			uint unk10 = stream->readUint32LE();
			stream->readUint32LE(); //CDCDCDCD
			uint unk11 = stream->readByte();
			stream->readUint32LE(); //CDCDCDCD
			debug("unknown: %u %u %u 0x%08x - %u %u %u %u - %u %u %u",
				unk1, unk2, unk3, unk4,
				unk5, unk6, unk7, unk8,
				unk9, unk10, unk11
			);
		}
		_animations.push_back(animation);
	}

	delete stream;
}

void Character::setDirection(int dir) {
	debug("setDirection %d", dir);
	_direction = dir;
	_animation = _engine->loadAnimation(_animations[dir].filename);
	if (!_animation) {
		debug("no animation?");
		_phase = -1;
		_frames = 0;
	}
	_animation->loop(true);
}

void Character::moveTo(Common::Point dst, int frames) {
	debug("move to %d,%d", dst.x, dst.y);
	_dst = dst;
	_phase = 0;
	_frames = frames;
}

void Character::animate(Common::Point pos, int frames, int speed) {
	_animation = _engine->loadAnimation(_animations[_direction].filename);
	if (!_animation) {
		debug("no animation?");
		_phase = -1;
		_frames = 0;
	}
	_animation->loop(true);
	_phase = 0;
	_frames = frames;
	_pos = pos;
}

void Character::paint(Graphics::Surface & backbuffer) {
	if (!_enabled || !_visible || !_animation)
		return;

	Common::Point pos = _pos;
	if (_phase >= 0 && _phase < _frames) {
		_animation->tick(*_engine);
		if (_phase + 1 >= _frames) {
			_phase = -1;
			_frames = 0;
			pos = _pos = _dst;
		} else {
			float dx = _dst.x - _pos.x;
			float dy = _dst.y - _pos.y;
			float t = 1.0f * _phase / _frames;
			pos.x += dx * t;
			pos.y += dy * t;
			++_phase;
		}
	}

	_animation->paint(*_engine, backbuffer, pos);
}

}
