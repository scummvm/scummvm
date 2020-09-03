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
		Common::Array<char> nameData(33);
		stream->read(nameData.data(), 32);

		AnimationDescription animation;
		animation.filename = Common::String(nameData.data());
		debug("animation %s, frames: %d, format: %d", animation.filename.c_str(), frames, format);
		while(frames--) {
			int x = stream->readSint16LE();
			int y = stream->readSint16LE();
			int w = stream->readUint32LE();
			int h = stream->readUint32LE();
			AnimationDescription::Frame frame = { x, y, w, h };
			animation.frames.push_back(frame);
			debug("frame %d, %d, %dx%d", x, y, w, h);
			stream->skip(50);
		}
		_animations.push_back(animation);
	}

	delete stream;
}

}
