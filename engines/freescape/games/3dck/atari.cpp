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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"
#include "image/neo.h"

#include "freescape/games/3dck/3dck.h"

namespace Freescape {

void KitEngine::loadPaletteAtari(Common::SeekableReadStream &file, byte *palette) {
	if (file.pos() > file.size() || file.size() - file.pos() < 32)
		error("Truncated 3D Construction Kit Atari ST palette");
	for (int i = 0; i < 16; i++) {
		uint16 color = file.readUint16BE();
		// The runner converts RGB444 to the ST's three bits per component.
		for (int c = 0; c < 3; c++)
			palette[3 * i + c] = ((color >> (9 - 4 * c)) & 7) * 255 / 7;
	}
}

void KitEngine::loadAssetsAtari() {
	Common::File file;
	if (!file.open(_gameDescription->filesDescriptions[0].fileName))
		error("Unable to open 3D Construction Kit Atari ST data");
	if (file.size() < 512 || (file.size() & 1))
		error("Invalid 3D Construction Kit Atari ST data size");
	Common::Array<byte> data;
	data.resize(file.size());
	if (file.read(data.data(), data.size()) != data.size())
		error("Truncated 3D Construction Kit Atari ST data");
	file.close();

	// Standalone runners XOR each word with their export key.
	const byte key[] = {byte(data[0] ^ 'A'), byte(data[1] ^ 'M')};
	for (uint i = 0; i < data.size(); i++)
		data[i] ^= key[i & 1];
	Common::MemoryReadStream world(data.data(), data.size());
	loadWorld(world);

	world.seek(world.size() - 8);
	uint32 paletteOffset = world.readUint32BE();
	uint32 paletteEnd = world.size() - 12;
	if (paletteOffset < 500 || paletteOffset > paletteEnd ||
			paletteEnd - paletteOffset != 34 * _areaData.size())
		error("Invalid 3D Construction Kit Atari ST palette table");
	Common::HashMap<uint16, bool> loadedPalettes;
	world.seek(paletteOffset);
	while (world.pos() < paletteEnd) {
		uint16 area = world.readUint16BE();
		if (!_areaData.contains(area) || loadedPalettes.contains(area))
			error("Invalid 3D Construction Kit Atari ST palette area %u", area);
		loadPaletteAtari(world, _areaData[area].palette);
		loadedPalettes[area] = true;
	}

	if (!file.open(_gameDescription->filesDescriptions[1].fileName))
		error("Unable to open 3D Construction Kit Atari ST border");
	if (file.size() != 32128)
		error("Invalid 3D Construction Kit Atari ST border size");
	file.seek(4);
	loadPaletteAtari(file, _palette);
	file.seek(0);
	Image::NeoDecoder decoder;
	if (!decoder.loadStream(file) || file.err())
		error("Unable to decode 3D Construction Kit Atari ST border");
	_borderPixels.copyFrom(*decoder.getSurface());
	_border = new Graphics::ManagedSurface();
	_border->copyFrom(*decoder.getSurface());
	_border->convertToInPlace(_gfx->_texturePixelFormat, _palette, 16);
}

void KitEngine::updateBorderAtari() {
	// Palette changes apply to whole scanlines, including the side panels.
	Graphics::Surface band = _borderPixels.getSubArea(Common::Rect(0, _viewArea.top, _screenW, _viewArea.bottom));
	Graphics::Surface *colored = band.convertTo(_gfx->_texturePixelFormat, _gfx->_palette, 16);
	_border->blitFrom(*colored, Common::Point(0, _viewArea.top));
	colored->free();
	delete colored;
	processBorder();
}

void KitEngine::splitColorAtari(byte &color, byte &extraColor) {
	extraColor = color & 0xf;
	color >>= 4;
	// A zero extra colour means solid fill to the renderer.
	if (!extraColor)
		SWAP(color, extraColor);
}

void KitEngine::drawBackground() {
	if (!isAtariST()) {
		FreescapeEngine::drawBackground();
		return;
	}
	clearBackground();
	byte first = _currentArea->_skyColor;
	byte second = _areaData[_currentArea->getAreaID()].skyExtraColor;
	byte r1, g1, b1, r2, g2, b2;
	_gfx->readFromPalette(first, r1, g1, b1);
	_gfx->readFromPalette(second, r2, g2, b2);
	_gfx->clear(r1, g1, b1);
	if (first != second)
		_gfx->fillViewportStippled(r1, g1, b1, r2, g2, b2, _gfx->_defaultStippleArray);
}

} // namespace Freescape
