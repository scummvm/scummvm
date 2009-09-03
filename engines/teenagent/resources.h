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


#ifndef TEENAGENT_RESOURCES_H__
#define TEENAGENT_RESOURCES_H__

#include "pack.h"
#include "segment.h"
#include "font.h"
#include "graphics/surface.h"

namespace TeenAgent {

class GameDescription;

class Resources {
protected:
	Resources();
public: 
	static Resources * instance();
	void loadArchives(const GameDescription * gd);
	void deinit();
	void loadOff(Graphics::Surface &surface, byte *palette, int id);
	Common::SeekableReadStream * loadLan(uint32 id) const;
	Common::SeekableReadStream * loadLan000(uint32 id) const;
	//void loadOn(Graphics::Surface &surface, int id, uint16 &dst, uint16 * flags);
	//void loadOns(Graphics::Surface &surface, int id, uint16 &dst);

	Pack varia, off, on, ons, lan000, lan500, mmm, sam_mmm, sam_sam;
	Segment cseg, dseg, eseg;
	Font font7;
};

}

#endif
