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

#include "common/file.h"
#include "common/config-manager.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/games/castle/opl.music.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Common::SeekableReadStream *CastleEngine::decryptFile(const Common::Path &filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	int seed = 24;
	for (int i = 0; i < size; i++) {
		if (i > 1)
			encryptedBuffer[i] ^= seed;
		seed = (seed + 1) & 0xff;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

extern Common::MemoryReadStream *unpackEXE(Common::File &ms);

byte kEGARiddleFontPalette[16][3] = {
	{0x00, 0x00, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00},
	{0xaa, 0x55, 0x00}
};

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanes(Common::SeekableReadStream *file, int widthInBytes, int height) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(widthInBytes * 8 / 4, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, widthInBytes * 8 / 4, height), 0);
	loadFrameFromPlanesInternal(file, surface, widthInBytes, height);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameFromPlanesInternal(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height) {
	byte *colors = (byte *)malloc(sizeof(byte) * height * width);
	file->read(colors, height * width);

	for (int p = 0; p < 4; p++) {
		for (int i = 0; i < height * width; i++) {
			byte color = colors[i];
			for (int n = 0; n < 8; n++) {
				int y = i / width;
				int x = (i % width) * 8 + (7 - n);
				// Check that we are in the right plane
				if (x < width * (8 / 4) * p || x >= width * (8 / 4) * (p + 1))
					continue;

				int bit = ((color >> n) & 0x01) << p;
				int sample = surface->getPixel(x % (width * 8 / 4), y) | bit;
				assert(sample < 16);
				surface->setPixel(x % (width * 8 / 4), y, sample);
			}
		}
	}
	return surface;
}

// CGA frames are not planar: each byte holds four 2 bit pixels, high bits first
Graphics::ManagedSurface *CastleEngine::loadFrameFromPackedPixels(Common::SeekableReadStream *file, int widthInBytes, int height) {
	int width = widthInBytes * 4;
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, width, height), 0);

	byte *pixels = (byte *)malloc(sizeof(byte) * height * widthInBytes);
	file->read(pixels, height * widthInBytes);

	for (int y = 0; y < height; y++) {
		for (int i = 0; i < widthInBytes; i++) {
			byte packed = pixels[y * widthInBytes + i];
			for (int p = 0; p < 4; p++)
				surface->setPixel(i * 4 + p, y, (packed >> (6 - 2 * p)) & 0x3);
		}
	}

	free(pixels);
	return surface;
}

Graphics::ManagedSurface *CastleEngine::loadFrameDOS(Common::SeekableReadStream *file, int widthInBytes, int height) {
	if (_renderMode == Common::kRenderCGA)
		return loadFrameFromPackedPixels(file, widthInBytes, height);

	return loadFrameFromPlanes(file, widthInBytes, height);
}

void CastleEngine::convertFrameDOS(Graphics::ManagedSurface *frame) {
	if (_renderMode == Common::kRenderCGA)
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kCGAPalettePinkBlueBright, 4);
	else
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
}

Common::Array <Graphics::ManagedSurface *>CastleEngine::loadFramesWithHeaderDOS(Common::SeekableReadStream *file, int numFrames) {
	uint8 header1 = file->readByte();
	uint8 header2 = file->readByte();
	int height = file->readByte();
	uint8 mask = file->readByte();
	int size = file->readUint16LE();

	assert(size % height == 0);
	int widthBytes = (size / height);

	Common::Array<Graphics::ManagedSurface *> frames;
	for (int i = 0; i < numFrames; i++) {
		Graphics::ManagedSurface *frame = loadFrameDOS(file, widthBytes, height);
		convertFrameDOS(frame);
		frames.push_back(frame);
	}

	debug("header: %x %x, height: %d, mask: %x, widthBytes: %d, size: %d", header1, header2, height, mask, widthBytes, size);
	return frames;
}

Graphics::ManagedSurface *CastleEngine::loadFrameWithHeaderDOS(Common::SeekableReadStream *file) {
	uint8 header1 = file->readByte();
	uint8 header2 = file->readByte();
	int height = file->readByte();
	uint8 mask = file->readByte();
	int size = file->readUint16LE();

	assert(size % height == 0);
	int widthBytes = (size / height);

	Graphics::ManagedSurface *frame = loadFrameDOS(file, widthBytes, height);
	convertFrameDOS(frame);

	debug("header: %x %x, height: %d, mask: %x, widthBytes: %d, size: %d", header1, header2, height, mask, widthBytes, size);
	debug("pos: %x", (int32)file->pos());
	return frame;
}

// As above, but keeping the indexed surface so the sprite can be recolored later
Graphics::ManagedSurface *CastleEngine::loadFrameWithHeaderDOSIndexed(Common::SeekableReadStream *file) {
	file->readByte();
	file->readByte();
	int height = file->readByte();
	file->readByte();
	int size = file->readUint16LE();

	assert(size % height == 0);
	return loadFrameDOS(file, size / height, height);
}

Common::Array<Graphics::ManagedSurface *> CastleEngine::loadFramesWithHeaderDOSIndexed(Common::SeekableReadStream *file, int numFrames) {
	file->readByte();
	file->readByte();
	int height = file->readByte();
	file->readByte();
	int size = file->readUint16LE();

	assert(size % height == 0);
	int widthBytes = (size / height);

	Common::Array<Graphics::ManagedSurface *> frames;
	for (int i = 0; i < numFrames; i++)
		frames.push_back(loadFrameDOS(file, widthBytes, height));

	return frames;
}

void CastleEngine::initDOS() {
	_viewArea = Common::Rect(40, 33 - 2, 280, 152);
}

void CastleEngine::loadAssetsDOSFullGame() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		// Every block below is byte identical in CRE.EXE and CME.EXE, but sits
		// 0x30a0 earlier since CM2 has a smaller code section. The speaker
		// tables are at the same offset in both.
		const int delta = isCastleMaster2() ? -0x30a0 : 0;

		file.open(isCastleMaster2() ? "CRE.EXE" : "CME.EXE");
		stream = unpackEXE(file);
		if (stream) {
			_sound = loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200, 30);

			stream->seek(0x197c0 + delta);
			// Blank in CM2, but still parsed to reach the background after it
			_endGameBackgroundFrame = loadFrameFromPlanes(stream, 112, 108);
			_endGameBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			_background = loadFrameFromPlanes(stream, 504, 18);
			_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			debug("%x", (int32)stream->pos());
			// Eye widget is next to 0x1f058

			stream->seek(0x1f4e3 + delta);
			for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%x", (int32)stream->pos());
			debug("extra: %x", stream->readByte());

			for (int i = 0; i < 10; i++) {
				Graphics::ManagedSurface *frame = loadFrameFromPlanes(stream, 8, 14);
				frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
				_keysBorderFrames.push_back(frame);
			}

			for (int i = 0; i < 10; i++) {
				Graphics::ManagedSurface *frame = loadFrameFromPlanes(stream, 8, 14);
				frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
				_keysMenuFrames.push_back(frame);
			}

			//for (int i = 0; i < 6; i++)
			//	debug("i: %d -> %x", i, stream->readByte());

			//loadFrameWithHeaderDOS(stream);
			//debug("%lx", stream->pos());
			//assert(0);

			stream->seek(0x20262 + delta);
			_strenghtBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_strenghtBarFrame = loadFrameWithHeaderDOS(stream);
			_strenghtWeightsFrames = loadFramesWithHeaderDOS(stream, 4);
			_spiritsMeterIndicatorBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_spiritsMeterIndicatorFrame = loadFrameWithHeaderDOS(stream);
			_spiritsMeterIndicatorSideFrame = loadFrameWithHeaderDOS(stream); // side
			loadFrameWithHeaderDOS(stream); // ???

			/*for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%lx", stream->pos());*/
			//assert(0);

			stream->seek(0x221ae + delta);
			// No header?
			_menu = loadFrameFromPlanes(stream, 112, 115);
			_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			Common::Array <Graphics::ManagedSurface *> menuFrames = loadFramesWithHeaderDOS(stream, 5);
			_menuCrawlIndicator = menuFrames[0];
			_menuWalkIndicator = menuFrames[1];
			_menuRunIndicator = menuFrames[2];
			_menuFxOffIndicator = menuFrames[3];
			_menuFxOnIndicator = menuFrames[4];

			_flagFrames = loadFramesWithHeaderDOS(stream, 4);
			// Unused by CM2, but shipped, and parsed to keep reading in order
			_riddleTopFrame = loadFrameWithHeaderDOS(stream);
			_riddleBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_riddleBottomFrame = loadFrameWithHeaderDOS(stream);
			_endGameThroneFrame = loadFrameWithHeaderDOS(stream);
			// No header
			Graphics::ManagedSurface *thunderFrame;

			thunderFrame = loadFrameFromPlanes(stream, 16, 112);
			thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			_thunderFrames.push_back(thunderFrame);

			stream->seek(-0x160,SEEK_CUR);
			thunderFrame = loadFrameFromPlanes(stream, 16, 112);
			thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			_thunderFrames.push_back(thunderFrame);

			stream->seek(0x29696 + delta);
			Common::Array<Graphics::ManagedSurface *> chars;
			Common::Array<Graphics::ManagedSurface *> charsRiddle;
			for (int i = 0; i < 90; i++) {
				Graphics::ManagedSurface *img = loadFrameFromPlanes(stream, 8, 8);
				Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
				imgRiddle->copyFrom(*img);

				chars.push_back(img);
				chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

				charsRiddle.push_back(imgRiddle);
				charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGARiddleFontPalette, 16);
			}
			_font = Font(chars);
			_font.setCharWidth(9);

			_fontRiddle = Font(charsRiddle);
			_fontRiddle.setCharWidth(9);
			_fontLoaded = true;
		}

		delete stream;
		file.close();

		file.open(isCastleMaster2() ? "CRLE.DAT" : "CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		// CM2 draws its configuration menu as text, so it ships no CMOE.DAT
		if (!isCastleMaster2()) {
			file.open("CMOE.DAT");
			_option = load8bitBinImage(&file, 0x0);
			_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
			file.close();
		}

		file.open(isCastleMaster2() ? "CRE.DAT" : "CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		if (isCastleMaster2()) {
			// 132 entries: game text, area names from 41, then front end strings
			stream = decryptFile("CRLE");
			loadMessagesCastleMaster2(stream, 0x10, 132);
			delete stream;

			stream = decryptFile("CREDF");
			load8bitBinary(stream, 0, 16);
			delete stream;
			return;
		}

		switch (_language) {
			case Common::ES_ESP:
				stream = decryptFile("CMLS");
				loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
				// Fixes for incorrect or wrong translations
				Common::replace(_riddleList[16]._lines[5]._text, "IN", "EN");
				break;
			case Common::FR_FRA:
				stream = decryptFile("CMLF");
				loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
				break;
			case Common::DE_DEU:
				stream = decryptFile("CMLG");
				loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
				break;
			case Common::EN_ANY:
				stream = decryptFile("CMLE");
				loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
				break;
			default:
				error("Invalid or unsupported language: %x", _language);
		}

		loadMessagesVariableSize(stream, 0x11, 164);
		delete stream;

		stream = decryptFile("CMEDF");
		load8bitBinary(stream, 0, 16);
		delete stream;

		if (ConfMan.getBool("opl_music"))
			_playerMusic = new CastleOPLMusicPlayer();
	} else
		error("Not implemented yet");


	// CPC
	// file = gameDir.createReadStreamForMember("cm.bin");
	// if (file == nullptr)
	//	error("Failed to open cm.bin");
	// load8bitBinary(file, 0x791a, 16);
}

void CastleEngine::loadAssetsDOSDemo() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		file.open("CMDE.EXE");
		stream = unpackEXE(file);
		if (stream) {
			_sound = loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200, 30);

			stream->seek(0x197c0 - 0x2a0);
			_endGameBackgroundFrame = loadFrameFromPlanes(stream, 112, 108);
			_endGameBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			_background = loadFrameFromPlanes(stream, 504, 18);
			_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			stream->seek(0x1f4e3 - 0x2a0);
			for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%x", (int32)stream->pos());
			debug("extra: %x", stream->readByte());

			for (int i = 0; i < 9; i++) {
				Graphics::ManagedSurface *frame = loadFrameFromPlanes(stream, 8, 14);
				frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
				_keysBorderFrames.push_back(frame);
			}

			for (int i = 0; i < 11; i++) {
				Graphics::ManagedSurface *frame = loadFrameFromPlanes(stream, 8, 14);
				frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
				_keysMenuFrames.push_back(frame);
			}

			stream->seek(0x20262 - 0x2a0);
			_strenghtBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_strenghtBarFrame = loadFrameWithHeaderDOS(stream);
			_strenghtWeightsFrames = loadFramesWithHeaderDOS(stream, 4);
			_spiritsMeterIndicatorBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_spiritsMeterIndicatorFrame = loadFrameWithHeaderDOS(stream);
			_spiritsMeterIndicatorSideFrame = loadFrameWithHeaderDOS(stream); // side
			loadFrameWithHeaderDOS(stream); // ???

			stream->seek(0x221ae - 0x2a0);
			// No header?
			_menu = loadFrameFromPlanes(stream, 112, 115);
			_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			Common::Array <Graphics::ManagedSurface *> menuFrames = loadFramesWithHeaderDOS(stream, 5);
			_menuCrawlIndicator = menuFrames[0];
			_menuWalkIndicator = menuFrames[1];
			_menuRunIndicator = menuFrames[2];
			_menuFxOffIndicator = menuFrames[3];
			_menuFxOnIndicator = menuFrames[4];

			_flagFrames = loadFramesWithHeaderDOS(stream, 4);
			_riddleTopFrame = loadFrameWithHeaderDOS(stream);
			_riddleBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_riddleBottomFrame = loadFrameWithHeaderDOS(stream);
			_endGameThroneFrame = loadFrameWithHeaderDOS(stream);
			// No header
			Graphics::ManagedSurface *thunderFrame;

			thunderFrame = loadFrameFromPlanes(stream, 16, 112);
			thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			_thunderFrames.push_back(thunderFrame);

			stream->seek(-0x160,SEEK_CUR);
			thunderFrame = loadFrameFromPlanes(stream, 16, 112);
			thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			_thunderFrames.push_back(thunderFrame);

			stream->seek(0x293f6); // TODO: check this
			Common::Array<Graphics::ManagedSurface *> chars;
			Common::Array<Graphics::ManagedSurface *> charsRiddle;
			for (int i = 0; i < 90; i++) {
				Graphics::ManagedSurface *img = loadFrameFromPlanes(stream, 8, 8);
				Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
				imgRiddle->copyFrom(*img);

				chars.push_back(img);
				chars[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

				charsRiddle.push_back(imgRiddle);
				charsRiddle[i]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGARiddleFontPalette, 16);
			}
			_font = Font(chars);
			_font.setCharWidth(9);

			_fontRiddle = Font(charsRiddle);
			_fontRiddle.setCharWidth(9);
			_fontLoaded = true;
		}

		delete stream;
		file.close();

		file.open("CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CMOE.DAT");
		_option = load8bitBinImage(&file, 0x0);
		_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		stream = decryptFile("CMLD"); // Only english
		loadMessagesVariableSize(stream, 0x11, 164);
		loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
		delete stream;

		stream = decryptFile("CDEDF");
		load8bitBinary(stream, 0, 16);
		delete stream;

		if (ConfMan.getBool("opl_music"))
			_playerMusic = new CastleOPLMusicPlayer();
	} else if (_renderMode == Common::kRenderCGA) {
		loadAssetsDOSDemoCGA();
	} else
		error("Not implemented yet");

}

void CastleEngine::loadAssetsDOSDemoCGA() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	// CMDC.EXE holds the same assets in the same order as the EGA build, but
	// redrawn for CGA, so none of its offsets apply. The graphics segment starts
	// at 0x188b0 in the unpacked executable, with the background at offset zero
	file.open("CMDC.EXE");
	stream = unpackEXE(file);
	if (stream) {
		// The PC speaker tables are the only part shared with CMDE.EXE
		_sound = loadSpeakerFxDOS(stream, 0x46fd + 0x200, 0x477d + 0x200, 30);

		stream->seek(0x17110);
		_endGameBackgroundFrame = loadFrameDOS(stream, 56, 108);
		convertFrameDOS(_endGameBackgroundFrame);

		// Drawn inside the view, so it follows the palette of the current area
		_backgroundCLUT8 = loadFrameDOS(stream, 252, 18);

		// Two groups of ten key sprites, each with its own header
		stream->seek(0x19eac);
		_keysBorderCLUT8Frames = loadFramesWithHeaderDOSIndexed(stream, 10);
		_keysMenuCLUT8Frames = loadFramesWithHeaderDOSIndexed(stream, 10);

		stream->seek(0x1a328);
		_strenghtBackgroundCLUT8 = loadFrameWithHeaderDOSIndexed(stream);
		_strenghtBarCLUT8 = loadFrameWithHeaderDOSIndexed(stream);
		_strenghtWeightsCLUT8 = loadFramesWithHeaderDOSIndexed(stream, 4);
		_spiritsMeterBgCLUT8 = loadFrameWithHeaderDOSIndexed(stream);
		_spiritsMeterIndCLUT8 = loadFrameWithHeaderDOSIndexed(stream);
		_spiritsMeterSideCLUT8 = loadFrameWithHeaderDOSIndexed(stream);

		stream->seek(0x1b9fe);
		_menu = loadFrameDOS(stream, 56, 115);
		convertFrameDOS(_menu);

		Common::Array <Graphics::ManagedSurface *> menuFrames = loadFramesWithHeaderDOS(stream, 5);
		_menuCrawlIndicator = menuFrames[0];
		_menuWalkIndicator = menuFrames[1];
		_menuRunIndicator = menuFrames[2];
		_menuFxOffIndicator = menuFrames[3];
		_menuFxOnIndicator = menuFrames[4];

		_flagCLUT8 = loadFramesWithHeaderDOSIndexed(stream, 4);

		_riddleTopFrame = loadFrameWithHeaderDOS(stream);
		_riddleBackgroundFrame = loadFrameWithHeaderDOS(stream);
		_riddleBottomFrame = loadFrameWithHeaderDOS(stream);
		_endGameThroneFrame = loadFrameWithHeaderDOS(stream);
		// No lightning frames are shipped; updateThunder() skips an empty array

		stream->seek(0x1f05c);
		Common::Array<Graphics::ManagedSurface *> chars;
		Common::Array<Graphics::ManagedSurface *> charsRiddle;
		// 95 glyphs of 3x8 bytes after the usual header. They are left indexed so
		// Font::drawChar() recolors them with what updateCGAPalette() selected
		stream->skip(6);
		for (int i = 0; i < 90; i++) {
			Graphics::ManagedSurface *img = loadFrameDOS(stream, 3, 8);
			Graphics::ManagedSurface *imgRiddle = new Graphics::ManagedSurface();
			imgRiddle->copyFrom(*img);

			chars.push_back(img);
			charsRiddle.push_back(imgRiddle);
		}
		_font = Font(chars);
		_font.setCharWidth(9);

		_fontRiddle = Font(charsRiddle);
		_fontRiddle.setCharWidth(9);
		_fontLoaded = true;
	}

	delete stream;
	file.close();

	file.open("CMLC.DAT");
	_title = load8bitBinImage(&file, 0x0);
	_title->setPalette((byte *)&kCGAPalettePinkBlueBright, 0, 4);
	file.close();

	file.open("CMOC.DAT");
	_option = load8bitBinImage(&file, 0x0);
	_option->setPalette((byte *)&kCGAPalettePinkBlueBright, 0, 4);
	file.close();

	file.open("CMC.DAT");
	_border = load8bitBinImage(&file, 0x0);
	_border->setPalette((byte *)&kCGAPalettePinkBlueBright, 0, 4);
	file.close();

	stream = decryptFile("CMLD"); // Only english
	loadMessagesVariableSize(stream, 0x11, 164);
	loadRiddles(stream, 0xaae - 2 - 22 * 2, 22);
	delete stream;

	stream = decryptFile("CDCDF");
	load8bitBinary(stream, 0, 4);
	delete stream;

	// Build the sprite surfaces and font colors; swapPalette() redoes this per area
	updateCGAPalette((byte *)&kCGAPalettePinkBlueBright);
}

void CastleEngine::drawDOSUI(Graphics::Surface *surface) {
	// EGA picks a bright color from its 16 entry palette; CGA only has four
	uint32 color = _renderMode == Common::kRenderCGA ? 3 : 10;
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint8 r, g, b;
	drawLiftingGate(surface);
	drawDroppingGate(surface);

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateControl != kFreescapeGameStateEnd) {
			if (ghostInArea())
				drawStringInSurface(_ghostInAreaMessage, 97, 182, front, back, surface);
			else
				drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);
		}
	}

	for (int k = 0; k < int(_keysCollected.size()); k++) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_keysBorderFrames[k], 76 - k * 3, 179, Common::Rect(0, 0, 6, 14), black);
	}

	drawEnergyMeter(surface, Common::Point(38, 158));
	int flagFrameIndex = (_ticks / 10) % 4;
	// The CGA flag is 20 pixels wide instead of 32, with its pole further left
	int flagX = _renderMode == Common::kRenderCGA ? 288 : 285;
	surface->copyRectToSurface(*_flagFrames[flagFrameIndex], flagX, 5, Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));

	surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorBackgroundFrame, 136, 162, Common::Rect(0, 0, _spiritsMeterIndicatorBackgroundFrame->w, _spiritsMeterIndicatorBackgroundFrame->h));
	surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 125 + 6 + _spiritsMeterPosition, 161, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h), black);
	// The EGA sprite holds two copies of the cap, the CGA one only holds a single
	int sideWidth = _spiritsMeterIndicatorSideFrame->w;
	if (_renderMode != Common::kRenderCGA)
		sideWidth /= 2;
	surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorSideFrame, 122 + 5 + 1, 157 + 5 - 1, Common::Rect(0, 0, sideWidth, _spiritsMeterIndicatorSideFrame->h));
	//surface->copyRectToSurface(*_spiritsMeterIndicatorFrame, 100, 50, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h));
}

} // End of namespace Freescape
