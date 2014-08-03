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

#include "common/endian.h"

#include "engines/grim/sprite.h"
#include "engines/grim/resource.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/emi/costumeemi.h"


namespace Grim {

Sprite::Sprite() :
		_width(0), _height(0), _visible(false), _material(nullptr), _next(nullptr), _blendMode(BlendNormal),
		_writeDepth(true), _alphaTest(true) {
}


void Sprite::draw() const {
	if (!_visible)
		return;

	_material->select();
	g_driver->drawSprite(this);
}

void Sprite::loadGrim(const Common::String &name, const char *comma, CMap *cmap) {
	int width, height, x, y, z;
	sscanf(comma, ",%d,%d,%d,%d,%d", &width, &height, &x, &y, &z);
	_material = g_resourceloader->loadMaterial(name, cmap, true);
	_width = (float)width / 100.0f;
	_height = (float)height / 100.0f;
	_pos.set((float)x / 100.0f, (float)y / 100.0f, (float)z / 100.0f);
}

void Sprite::loadBinary(Common::SeekableReadStream *stream, EMICostume *costume) {
	if (!stream)
		return;

	uint32 namelength = stream->readUint32LE();
	char *name = new char[namelength];
	stream->read(name, namelength);

	stream->seek(40, SEEK_CUR);
	uint32 texnamelength = stream->readUint32LE();
	char *texname = new char[texnamelength];
	stream->read(texname, texnamelength);
	stream->readByte(); // Unknown
	byte blendMode = stream->readByte();
	if (blendMode == 4)
		_blendMode = BlendAdditive;
	else if (blendMode != 0)
		warning("Unknown blend mode value %d for sprite %s", blendMode, name);
	stream->skip(2); // Unknown
	float width, height;
	float offX, offY;
	char data[16];
	stream->read(data, sizeof(data));
	width = get_float(data);
	height = get_float(data + 4);
	offX = get_float(data + 8);
	offY = get_float(data + 12);
	stream->skip(4);//Unknown
	for (int i = 0; i < 4; ++i) {
		_alpha[i] = stream->readSint32LE();
		_red[i] = stream->readSint32LE();
		_green[i] = stream->readSint32LE();
		_blue[i] = stream->readSint32LE();
	}
	for (int i = 0; i < 4; ++i) {
		char f[4];
		stream->read(f, 4);
		_texCoordX[i] = get_float(f);
		stream->read(f, 4);
		_texCoordY[i] = get_float(f);
	}
	stream->readByte(); // Unknown (seems to always be 4)
	if (stream->readByte() == 2)
		_writeDepth = false;
	if (stream->readByte() < 2)
		_alphaTest = false;

	_material = costume->loadMaterial(texname, true);
	_width = width;
	_height = height;
	_next = nullptr;
	_visible = true;
	_pos.set(offX, offY, 0);

	delete[] name;
	delete[] texname;
}

} // end of namespace Grim
