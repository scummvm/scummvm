/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRIM_OVERLAY_H
#define GRIM_OVERLAY_H

#include "common/endian.h"

#include "engines/grim/pool.h"

namespace Grim {

class Material;

class Overlay : public PoolObject<Overlay> {
public:
	Overlay(const Common::String &filename, Common::SeekableReadStream *data);
	~Overlay();

	void draw();
	void setPos(float x, float y) { _x = x; _y = y; }
	void setLayer(int layer) { _layer = layer; }
	int getWidth() const;
	int getHeight() const;

	static int32 getStaticTag() { return MKTAG('O','V','E','R'); }

//private:
	Material *_material;
	float _x;
	float _y;
	int _layer;
};

}

#endif
