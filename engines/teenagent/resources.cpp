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

#include "teenagent/resources.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

Resources::Resources() {}

Resources * Resources::instance() {
	static Resources i;
	return &i;
}

void Resources::deinit() {
	varia.close();
	off.close();
	on.close();
	ons.close();
	lan000.close();
	lan500.close();
	mmm.close();
	sam_mmm.close();
	sam_sam.close();
}

void Resources::loadArchives(const GameDescription * gd) {
	off.open("off.res");
	varia.open("varia.res");
	on.open("on.res");
	ons.open("ons.res");
	lan000.open("lan_000.res");
	lan500.open("lan_500.res");
	mmm.open("mmm.res");
	sam_mmm.open("sam_mmm.res");
	sam_sam.open("sam_sam.res");

	font7.load(7);
	
	Common::File exe;
	if (!exe.open("Teenagnt.exe")) {
		error("cannot open exe file");
		return;
	}
	exe.seek(gd->offsets.cseg_offset);
	cseg.read(&exe, 0xb3b0); //code
	
	exe.seek(gd->offsets.dseg_offset);
	dseg.read(&exe, 0xe790); //data

	exe.seek(gd->offsets.eseg_offset);
	eseg.read(&exe, 0x8be2);

	exe.close();
}

void Resources::loadOff(Graphics::Surface &surface, byte * palette, int id) {
	uint32 size = off.get_size(id);
	if (size == 0) {
		error("invalid background %d", id);
		return;
	}
	byte buf[64768];
	off.read(id, buf, sizeof(buf));

	byte * src = buf;
	byte * dst = (byte *)surface.pixels;
	memcpy(dst, src, 64000);
	memcpy(palette, buf + 64000, 768);
}

Common::SeekableReadStream * Resources::loadLan(uint32 id) const {
	return id <= 500? loadLan000(id): lan500.getStream(id - 500);
}

Common::SeekableReadStream * Resources::loadLan000(uint32 id) const {
	switch(id) {

	case 81:
		if (dseg.get_byte(0xDBAD)) 
			return lan500.getStream(160);
		break;

	case 137:
		if (dseg.get_byte(0xDBC5) == 1) {
			if (dseg.get_byte(0xDBC6) == 1)
				return lan500.getStream(203);
			else 
				return lan500.getStream(202);
		}
		break;

	case 25: 
		if (dseg.get_byte(0xDBDF) == 2) {
			return lan500.getStream(332);
		}
		break;

	case 37: 
		if (dseg.get_byte(0xdbe2) == 1) {
			return lan500.getStream(351);
		} else if (dseg.get_byte(0xdbe2) == 2) {
			return lan500.getStream(364);
		}
		break;
	
	case 29:
		if (dseg.get_byte(0xDBE7) == 1) {
			return lan500.getStream(380);
		}

	case 30:
		if (dseg.get_byte(0xDBE7) == 1) {
			return lan500.getStream(381);
		}

	case 42:
		if (dseg.get_byte(0xDBEC) == 1) {
			return lan500.getStream(400);
		}
	}
	return lan000.getStream(id);
}

} // End of namespace TeenAgent
