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

#include "teenagent/surface.h"
#include "teenagent/surface_list.h"

namespace TeenAgent {

SurfaceList::SurfaceList() : surfaces(NULL) {}

void SurfaceList::load(Common::SeekableReadStream *stream, Type type, int sub_hack) {
	free();
	
	byte fn = stream->readByte();
	if (stream->eos())
		return;
		
	surfaces_n = fn - sub_hack;
	debug(0, "loading %u surfaces from list (skip %d)", surfaces_n, sub_hack);

	if (surfaces_n == 0)
		return;
	
	surfaces = new Surface[surfaces_n];

	byte i;
	for (i = 0; i < surfaces_n; ++i) {
		uint offset = stream->readUint16LE();
		uint pos = stream->pos(); 
		stream->seek(offset);
		surfaces[i].load(stream, Surface::kTypeOns);
		stream->seek(pos);
	}
	//for(; i < fn; ++i)
	//	debug(0, "*hack* skipping flag %04x", stream->readUint16LE());
}

void SurfaceList::free() {
	delete[] surfaces;
	surfaces = NULL;
	surfaces_n = 0;
}

Common::Rect SurfaceList::render(Graphics::Surface *surface, int horizon, bool second_pass) const {
	Common::Rect dirty;
	for(uint i = 0; i < surfaces_n; ++i) {
		const Surface &s = surfaces[i];
		if (second_pass) {
			//debug(0, "%d %d", s.y + s.h, horizon);
			if (s.y + s.h > horizon)
				dirty.extend(s.render(surface));
		} else {
			if (s.y + s.h <= horizon)
				dirty.extend(s.render(surface));
		}
	}
	return dirty;
}

}
