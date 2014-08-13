/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_SPRITE_H
#define GRIM_SPRITE_H

#include "math/matrix4.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace Grim {

class Material;
class CMap;
class EMICostume;

class Sprite {
public:
	enum SpriteFlags1 {
		BlendAdditive = 0x400
	};
	enum SpriteFlags2 {
		DepthTest = 0x100,
		AlphaTest = 0x20000
	};

	Sprite();
	void draw() const;
	void loadBinary(Common::SeekableReadStream *, EMICostume *costume);
	void loadGrim(const Common::String &name, const char *comma, CMap *cmap);

	Math::Vector3d _pos;
	float _width;
	float _height;
	bool _visible;
	Material *_material;
	Sprite *_next;
	uint32 _flags1;
	int _red[4];
	int _green[4];
	int _blue[4];
	int _alpha[4];
	float _texCoordX[4];
	float _texCoordY[4];
	uint32 _flags2;
};

} // end of namespace Grim

#endif
