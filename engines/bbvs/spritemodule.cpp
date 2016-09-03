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

#include "bbvs/spritemodule.h"

namespace Bbvs {

byte *Sprite::getRow(int y) {
	if (type == 1)
		return data + READ_LE_UINT32((data + offset) + y * 4);
	else
		return data + offset + y * width;
}

SpriteModule::SpriteModule()
	: _spritesCount(0), _paletteStart(0), _paletteCount(0), _spriteData(0), _spriteDataSize(0),
	_spriteTblOffs(0), _paletteOffs(0) {
}

SpriteModule::~SpriteModule() {
	unload();
}

void SpriteModule::load(const char *filename) {
	unload();

	Common::File fd;
	if (!fd.open(filename))
		error("SpriteModule::load() Could not open %s", filename);

	fd.readUint32LE(); // Skip magic
	fd.readUint32LE(); // Skip unused
	fd.readUint32LE(); // Skip filesize

	_paletteOffs = fd.readUint32LE();
	fd.readUint32LE(); // Skip unused flagsTbl1Ofs
	fd.readUint32LE(); // Skip unused flagsTbl2Ofs
	_spriteTblOffs = fd.readUint32LE();
	_paletteStart = fd.readUint32LE();
	_paletteCount = fd.readUint32LE();
	_spritesCount = fd.readUint32LE();

	debug(0, "_paletteOffs: %08X", _paletteOffs);
	debug(0, "_spriteTblOffs: %08X", _spriteTblOffs);
	debug(0, "_paletteStart: %d", _paletteStart);
	debug(0, "_paletteCount: %d", _paletteCount);
	debug(0, "_spritesCount: %d", _spritesCount);

	_spriteDataSize = fd.size();
	_spriteData = new byte[_spriteDataSize];
	fd.seek(0);
	fd.read(_spriteData, _spriteDataSize);

	// Convert palette
	byte *palette = _spriteData + _paletteOffs;
	for (int i = 0; i < _paletteCount; ++i) {
		palette[i * 3 + 0] <<= 2;
		palette[i * 3 + 1] <<= 2;
		palette[i * 3 + 2] <<= 2;
	}

}

Sprite SpriteModule::getSprite(int index) {
	Sprite sprite;
	uint32 spriteOffs = READ_LE_UINT32(_spriteData + _spriteTblOffs + index * 4);
	byte *info = _spriteData + spriteOffs;
	sprite.data = _spriteData;
	sprite.offset = READ_LE_UINT32(info + 0);
	sprite.type = READ_LE_UINT32(info + 4);
	sprite.width = READ_LE_UINT32(info + 8);
	sprite.height = READ_LE_UINT32(info + 12);
	sprite.xOffs = READ_LE_UINT32(info + 16);
	sprite.yOffs = READ_LE_UINT32(info + 20);
	return sprite;
}

Palette SpriteModule::getPalette() {
	Palette palette;
	palette.data = _spriteData + _paletteOffs;
	palette.start = _paletteStart;
	palette.count = _paletteCount;
	return palette;
}

void SpriteModule::unload() {
	_spritesCount = 0;
	_paletteStart = 0;
	_paletteCount = 0;
	delete[] _spriteData;
	_spriteData = 0;
}

} // End of namespace Bbvs
