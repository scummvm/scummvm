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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

Sprite::Sprite() :_width(0), _height(0), _visible(false), _material(NULL), _next(NULL) {
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
	_material = g_resourceloader->loadMaterial(name, cmap);
	_width = (float)width / 100.0f;
	_height = (float)height / 100.0f;
	_pos.set((float)x / 100.0f, (float)y / 100.0f, (float)z / 100.0f);
}

void Sprite::loadBinary(Common::SeekableReadStream *stream, EMICostume *costume) {
	if (!stream)
		return;

	uint32 namelength = stream->readUint32LE();
	stream->skip(namelength);

	stream->seek(40, SEEK_CUR);
	uint32 texnamelength = stream->readUint32LE();
	char *texname = new char[texnamelength];
	stream->read(texname, texnamelength);
	/* unknown = */ stream->readUint32LE();
	float width, height;
	float offX, offY;
	char data[16];
	stream->read(data, sizeof(data));
	width = get_float(data);
	height = get_float(data + 4);
	offX = get_float(data + 8);
	offY = get_float(data + 12);

	_material = costume->findSharedMaterial(texname);
	_width = width;
	_height = height;
	_next = NULL;
	_visible = true;
	_pos.set(-offX, offY, 0);

	delete[] texname;
}

} // end of namespace Grim
