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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
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

extern byte kEGADefaultPalette[16][3];
extern Common::MemoryReadStream *unpackEXE(Common::File &ms);

void CastleEngine::loadDOSFonts(Common::SeekableReadStream *file, int pos) {
	file->seek(pos);
	byte *bufferPlane1 = (byte *)malloc(sizeof(byte) * 59 * 8);
	byte *bufferPlane2 = (byte *)malloc(sizeof(byte) * 59 * 8);
	byte *bufferPlane3 = (byte *)malloc(sizeof(byte) * 59 * 8);

	for (int i = 0; i < 59 * 8; i++) {
		//debug("%lx", file->pos());
		for (int j = 0; j < 4; j++) {
			uint16 c = readField(file, 16);
			assert(c < 256);
			if (j == 1) {
				bufferPlane1[i] = c;
			} else if (j == 2) {
				bufferPlane2[i] = c;
			} else if (j == 3) {
				bufferPlane3[i] = c;
			}
		}
		//debugN("\n");
	}
	//debug("%" PRIx64, file->pos());
	_fontPlane1.set_size(64 * 59);
	_fontPlane1.set_bits(bufferPlane1);

	_fontPlane2.set_size(64 * 59);
	_fontPlane2.set_bits(bufferPlane2);

	_fontPlane3.set_size(64 * 59);
	_fontPlane3.set_bits(bufferPlane3);
	_fontLoaded = true;
	free(bufferPlane1);
	free(bufferPlane2);
	free(bufferPlane3);
}

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
		Graphics::ManagedSurface *frame = loadFrameFromPlanes(file, widthBytes, height);
		frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
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

	Graphics::ManagedSurface *frame = loadFrameFromPlanes(file, widthBytes, height);
	frame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

	debug("header: %x %x, height: %d, mask: %x, widthBytes: %d, size: %d", header1, header2, height, mask, widthBytes, size);
	debug("pos: %lx", file->pos());
	return frame;
}

void CastleEngine::loadAssetsDOSFullGame() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CME.EXE");
		stream = unpackEXE(file);
		if (stream) {
			loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200);
			loadDOSFonts(stream, 0x29696);

			stream->seek(0x197c0);
			_endGameBackgroundFrame = loadFrameFromPlanes(stream, 112, 108);
			_endGameBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			_background = loadFrameFromPlanes(stream, 504, 18);
			_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
			debug("%lx", stream->pos());
			// Eye widget is next to 0x1f058

			stream->seek(0x1f4e3);
			for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%lx", stream->pos());
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

			stream->seek(0x20262);
			_strenghtBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_strenghtBarFrame = loadFrameWithHeaderDOS(stream);
			_strenghtWeightsFrames = loadFramesWithHeaderDOS(stream, 4);
			_spiritsMeterIndicatorBackgroundFrame = loadFrameWithHeaderDOS(stream);
			_spiritsMeterIndicatorFrame = loadFrameWithHeaderDOS(stream);
			loadFrameWithHeaderDOS(stream); // side
			loadFrameWithHeaderDOS(stream); // ???

			/*for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%lx", stream->pos());*/
			//assert(0);

			stream->seek(0x221ae);
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
			_thunderFrame = loadFrameFromPlanes(stream, 16, 128);
			_thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			// No header
			// Another thunder frame?
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

		switch (_language) {
			case Common::ES_ESP:
				stream = decryptFile("CMLS");
				loadRiddles(stream, 0xaae, 20);
				break;
			case Common::FR_FRA:
				stream = decryptFile("CMLF");
				break;
			case Common::DE_DEU:
				stream = decryptFile("CMLG");
				break;
			case Common::EN_ANY:
				stream = decryptFile("CMLE");
				loadRiddles(stream, 0xaae, 20);
				break;
			default:
				error("Invalid or unsupported language: %x", _language);
		}

		loadMessagesVariableSize(stream, 0x11, 164);
		delete stream;

		stream = decryptFile("CMEDF");
		load8bitBinary(stream, 0, 16);
		delete stream;
	} else
		error("Not implemented yet");

	addGhosts();
	// Discard the first three global conditions
	// It is unclear why they hide/unhide objects that formed the spirits
	for (int i = 0; i < 3; i++) {
		debugC(kFreescapeDebugParser, "Discarding condition %s", _conditionSources[1].c_str());
		_conditions.remove_at(1);
		_conditionSources.remove_at(1);
	}

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
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CMDE.EXE");
		stream = unpackEXE(file);
		if (stream) {
			loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200);
			loadDOSFonts(stream, 0x293f6);

			stream->seek(0x197c0 - 0x2a0);
			_endGameBackgroundFrame = loadFrameFromPlanes(stream, 112, 108);
			_endGameBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			_background = loadFrameFromPlanes(stream, 504, 18);
			_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			stream->seek(0x1f4e3 - 0x2a0);
			for (int i = 0; i < 6; i++)
				debug("i: %d -> %x", i, stream->readByte());
			debug("%lx", stream->pos());
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
			loadFrameWithHeaderDOS(stream); // side
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
			_thunderFrame = loadFrameFromPlanes(stream, 16, 128);
			_thunderFrame->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);
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
		loadRiddles(stream, 0xaae, 21);
		delete stream;

		stream = decryptFile("CDEDF");
		load8bitBinary(stream, 0, 16);
		delete stream;
	} else
		error("Not implemented yet");
}

void CastleEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = 10;
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);

	for (int k = 0; k < int(_keysCollected.size()); k++) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_keysBorderFrames[k], 76 - k * 3, 179, Common::Rect(0, 0, 6, 14), black);
	}

	drawEnergyMeter(surface, Common::Point(39, 157));
	int flagFrameIndex = (_ticks / 10) % 4;
	surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 285, 5, Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));

	surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorBackgroundFrame, 136, 162, Common::Rect(0, 0, _spiritsMeterIndicatorBackgroundFrame->w, _spiritsMeterIndicatorBackgroundFrame->h));
	surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 125 + _spiritsMeterPosition, 161, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h), black);
	//surface->copyRectToSurface(*_spiritsMeterIndicatorFrame, 100, 50, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h));
}

} // End of namespace Freescape
