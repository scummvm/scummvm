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

#include "common/system.h"
#include "graphics/paletteman.h"

#include "dgds/game_palettes.h"
#include "dgds/includes.h"
#include "dgds/resource.h"

namespace Dgds {

static const byte EGA_COLORS[16][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xAA },
	{ 0x00, 0xAA, 0x00 },
	{ 0x00, 0xAA, 0xAA },
	{ 0xAA, 0x00, 0x00 },
	{ 0xAA, 0x00, 0xAA },
	{ 0xAA, 0x55, 0x00 },
	{ 0xAA, 0xAA, 0xAA },
	{ 0x55, 0x55, 0x55 },
	{ 0x55, 0x55, 0xFF },
	{ 0x55, 0xFF, 0x55 },
	{ 0x55, 0xFF, 0xFF },
	{ 0xFF, 0x55, 0x55 },
	{ 0xFF, 0x55, 0xFF },
	{ 0xFF, 0xFF, 0x55 },
	{ 0xFF, 0xFF, 0xFF },
};

DgdsPal::DgdsPal() : Palette(256) {
}

GamePalettes::GamePalettes(ResourceManager *resourceMan, Decompressor *decompressor) : _curPalNum(0),
_resourceMan(resourceMan), _decompressor(decompressor) {
}

void GamePalettes::reset() {
	_palettes.resize(1);
	selectPalNum(0);
}

int GamePalettes::loadPalette(const Common::String &filename) {
	Common::SeekableReadStream *fileStream = _resourceMan->getResource(filename);
	if (!fileStream) {
		// Happens in the Amiga version of Dragon
		warning("Couldn't load palette resource %s", filename.c_str());
		return 0;
	}

	_palettes.resize(_palettes.size() + 1);

	DgdsPal &pal = _palettes.back();

	DgdsChunkReader chunk(fileStream);
	while (chunk.readNextHeader(EX_PAL, filename)) {
		chunk.readContent(_decompressor);
		Common::SeekableReadStream *chunkStream = chunk.getContent();
		if (chunk.isSection(ID_PAL)) {
			assert(chunk.isContainer());
		} else if (chunk.isSection(ID_VGA)) {
			for (uint k = 0; k < 256; k++) {
				byte r = chunkStream->readByte() << 2;
				byte g = chunkStream->readByte() << 2;
				byte b = chunkStream->readByte() << 2;
				pal.set(k, r, g, b);
			}
			break;
		} else if (chunk.isSection(ID_EGA)) {
			if (chunk.getSize() > 20) {
				for (uint k = 0; k < chunk.getSize() / 2; k++) {
					byte egaCol = (chunkStream->readUint16LE() & 0xF);
					byte r = EGA_COLORS[egaCol][0];
					byte g = EGA_COLORS[egaCol][1];
					byte b = EGA_COLORS[egaCol][2];
					pal.set(k, r, g, b);
				}
			} else {
				for (uint k = 0; k < chunk.getSize(); k++) {
					byte egaCol = (chunkStream->readByte());
					byte r = EGA_COLORS[egaCol][0];
					byte g = EGA_COLORS[egaCol][1];
					byte b = EGA_COLORS[egaCol][2];
					pal.set(k, r, g, b);
				}
			}
			break;
		} else if (chunk.isSection(ID_CGA)) {
			warning("Skipping CGA palette data");
		} else {
			error("Unknown Palette chunk in %s: %s size %d", filename.c_str(), chunk.getIdStr(), chunk.getSize());
		}
	}
	pal.setName(filename);

	delete fileStream;
	selectPalNum(_palettes.size() - 1);

	return _palettes.size() - 1;
}

void GamePalettes::selectPalNum(int num) {
	_curPalNum = num;
	setPalette();
}

void GamePalettes::setPalette() {
	if (_curPalNum >= _palettes.size())
		error("request to set pal %d but only have %d pals", _curPalNum, _palettes.size());

	_curPal = _palettes[_curPalNum];
	g_system->getPaletteManager()->setPalette(_curPal.data(), 0, 256);
}

void GamePalettes::clearPalette() {
	_curPal = DgdsPal();
	g_system->getPaletteManager()->setPalette(_curPal.data(), 0, 256);
}

void GamePalettes::setFade(int col, int ncols, int targetcol, int fade) {
	if (_curPalNum >= _palettes.size())
		error("GamePalettes::setFade: invalid curPalNum %d, only have %d pals", _curPalNum, _palettes.size());

	if (col + ncols > 256)
		error("GamePalettes::setFade: request to fade past the end of the palette");

	const DgdsPal &pal = _palettes[_curPalNum];

	byte r2, b2, g2;
	pal.get(targetcol, r2, g2, b2);

	for (int c = col; c < col + ncols; c++) {
		byte r, g, b;
		pal.get(c, r, g, b);

		_curPal.set(c,
			(r2 * fade + r * (255 - fade)) / 255,
			(g2 * fade + g * (255 - fade)) / 255,
			(b2 * fade + b * (255 - fade)) / 255);
	}
	g_system->getPaletteManager()->setPalette(_curPal.data(), 0, 256);
}

Common::Error GamePalettes::syncState(Common::Serializer &s) {
	s.syncAsUint16LE(_curPalNum);
	uint npals = _palettes.size();
	s.syncAsUint16LE(npals);

	if (s.isLoading()) {
		if (npals > 100)
			error("Too many palettes to load, save is probably corrupt");
		for (uint i = 0; i < npals; i++) {
			Common::String name;
			s.syncString(name);
			loadPalette(name);
		}
		if (_curPalNum >= _palettes.size())
			error("Current palette number %d greater than available palettes %d",
					_curPalNum, _palettes.size());

		setPalette();
	} else {
		for (uint i = 0; i < npals; i++) {
			Common::String name = _palettes[i].getName();
			s.syncString(name);
		}
	}

	return Common::kNoError;
}

} // end namespace Dgds
