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
 * $URL$
 * $Id$
 */

#include "teenagent/animation.h"
#include "common/endian.h"

namespace TeenAgent {

Animation::Animation() : id(0), x(0), y(0), loop(true), data(0), data_size(0), frames_count(0), frames(0), index(0) {
}

Surface * Animation::firstFrame() const {
	return frames;
}

Surface * Animation::currentFrame(int dt) {
	if (frames == NULL || frames_count == 0)
		return NULL;
	
	Surface * r;
	
	if (data != NULL) {
		uint32 frame = 3 * index;
		//debug(0, "%u/%u", index, data_size / 3);
		index += dt;
		
		if (!loop && index >= data_size / 3) {
			return NULL;
		}
		
		if (data[frame] - 1 >= frames_count) {
			warning("invalid frame %u(0x%x) (max %u) index %u, mod %u", frame, frame, frames_count, index - 1, data_size / 3);
			return NULL;
		}
	
		r = frames + data[frame] - 1;
		uint16 pos = READ_LE_UINT16(data + frame + 1);
		index %= (data_size / 3);

		if (pos != 0) {
			x = r->x = pos % 320;
			y = r->y = pos / 320;
		}
	} else {
		//debug(0, "index %u", index);
		r = frames + index;
		index += dt;
		index %= frames_count;
	}

	return r;
}


void Animation::free() {
	id = 0;
	x = y = 0;
	loop = true;
	
	delete[] data;
	data = NULL;
	data_size = 0;	

	frames_count = 0;
	delete[] frames;
	frames = NULL;
	
	index = 0;
}

void Animation::load(Common::SeekableReadStream * s, Type type) {
	//fixme: do not reload the same animation each time
	free();
	
	if (s == NULL && s->size() <= 1) {
		debug(0, "empty animation");
		return;
	}

	uint16 pos = 0;
	int off = 0;
	switch(type) {
	case TypeLan:
		data_size = s->readUint16LE();
		if (s->eos()) {
			debug(0, "empty animation");
			return;
		}

		data_size -= 2;
		data = new byte[data_size];
		data_size = s->read(data, data_size);
/*		for (int i = 0; i < data_size; ++i) {
			debug(0, "%02x ", data[i]);
		}
		debug(0, ", %u frames", data_size / 3);
*/	
		frames_count = s->readByte();
		debug(0, "%u physical frames", frames_count);
		if (frames_count == 0)
			return;

		frames = new Surface[frames_count];
	
		s->skip(frames_count * 2 - 2); //sizes
		pos = s->readUint16LE();
		//debug(0, "pos?: %04x", pos);
			
		for (uint16 i = 0; i < frames_count; ++i) {
			frames[i].load(s, Surface::TypeLan);
			frames[i].x = 0;
			frames[i].y = 0;
		}
		break;

	case TypeInventory: {
		data_size = 3 * s->readByte();
		data = new byte[data_size];

		frames_count = 0;
		for (byte i = 0; i < data_size / 3; ++i) {
			int idx = i * 3;
			/* byte unk = */ s->readByte();
			data[idx] = s->readByte();
			if (data[idx] == 0)
				data[idx] = 1; //fixme: investigate
			if (data[idx] > frames_count)
				frames_count = data[idx];
			data[idx + 1] = 0;
			data[idx + 2] = 0;
			//debug(0, "frame #%u", data[idx]);
		}
		
		frames = new Surface[frames_count];
		
		for (uint16 i = 0; i < frames_count; ++i) {
			frames[i].load(s, Surface::TypeOns);
		}
	}
	break;
	
	case TypeVaria:
		frames_count = s->readByte();
		debug(0, "loading varia resource, %u physical frames", frames_count);
		uint16 offset[255];
		for (byte i = 0; i < frames_count; ++i) {
			offset[i] = s->readUint16LE();
			debug(0, "%u: %04x", i, offset[i]);
		}
		frames = new Surface[frames_count];
		for (uint16 i = 0; i < frames_count; ++i) {
			debug(0, "%04x", offset[i]);
			s->seek(offset[i] + off);
			frames[i].load(s, Surface::TypeOns);
			frames[i].x = 0;
			frames[i].y = 0;
		}
		
		break;
	}
	
	debug(0, "%u frames", data_size / 3);
}

Animation::~Animation() {
	free();
}

} // End of namespace TeenAgent
