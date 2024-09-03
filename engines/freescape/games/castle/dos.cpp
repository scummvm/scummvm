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

			stream->seek(0x1c700);
			_background = loadFrameFromPlanes(stream, 252, 42);
			_background->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			stream->seek(0x221ae);
			_menu = loadFrameFromPlanes(stream, 112, 114);
			_menu->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&kEGADefaultPalette, 16);

			//debug("%lx", stream->pos());
			// TODO: some space here from the menu image
			/*stream->seek(0x25414);
			_menuCrawlIndicator = loadFrameFromPlanes(stream, 16, 12, lightGray, lightGray, lightGray, darkGray);
			_menuWalkIndicator = loadFrameFromPlanes(stream, 16, 12, lightGray, lightGray, lightGray, darkGray);
			_menuRunIndicator = loadFrameFromPlanes(stream, 16, 12, lightGray, lightGray, lightGray, darkGray);
			_menuFxOffIndicator = loadFrameFromPlanes(stream, 16, 12, lightGray, lightGray, lightGray, darkGray);
			_menuFxOnIndicator = loadFrameFromPlanes(stream, 16, 12, lightGray, lightGray, lightGray, darkGray);*/

			// This end in 0x257d4??
			byte flagPalette[4][3] = {
				{0x00, 0x00, 0x00},
				{0x00, 0xaa, 0x00},
				{0x55, 0xff, 0x55},
				{0xff, 0xff, 0xff}
			};

			stream->seek(0x257cc);
			_flagFrames[0] = loadFrameFromPlanes(stream, 16, 11);
			_flagFrames[0]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&flagPalette, 4);
			_flagFrames[1] = loadFrameFromPlanes(stream, 16, 11);
			_flagFrames[1]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&flagPalette, 4);
			_flagFrames[2] = loadFrameFromPlanes(stream, 16, 11);
			_flagFrames[2]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&flagPalette, 4);
			_flagFrames[3] = loadFrameFromPlanes(stream, 16, 11);
			_flagFrames[3]->convertToInPlace(_gfx->_texturePixelFormat, (byte *)&flagPalette, 4);

			//debug("%lx", stream->pos());
			//stream->seek(0x25a90);
			// This has only two planes?
			//_riddleTopFrames[0] = loadFrameFromPlanes(stream, 30, ??, lightGreen, transparent, darkGreen, transparent);
			//_riddleBottomFrames[0] = loadFrameFromPlanes(stream, 30, ??, lightGreen, transparent, darkGreen, transparent);*/

			/*stream->seek(0x25a94 + 0xe00);
			byte *grayPalette = (byte *)malloc(16 * 3);
			for (int i = 0; i < 16; i++) { // gray scale palette
				grayPalette[i * 3 + 0] = i * (255 / 16);
				grayPalette[i * 3 + 1] = i * (255 / 16);
				grayPalette[i * 3 + 2] = i * (255 / 16);
			}

			_something = loadFrameFromPlanes(stream, 36, 82);
			_something->convertToInPlace(_gfx->_texturePixelFormat, grayPalette, 16);*/
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
				break;
			case Common::FR_FRA:
				stream = decryptFile("CMLF");
				break;
			case Common::DE_DEU:
				stream = decryptFile("CMLG");
				break;
			case Common::EN_ANY:
				stream = decryptFile("CMLE");
				loadRiddles(stream, 0xaae, 11);
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
			loadDOSFonts(stream, 0x29696);
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
		loadRiddles(stream, 0xaae, 10);
		delete stream;

		stream = decryptFile("CDEDF");
		load8bitBinary(stream, 0, 16);
		delete stream;
	} else
		error("Not implemented yet");
}

void CastleEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = 10;
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

	drawEnergyMeter(surface);
	int flagFrameIndex = (_ticks / 10) % 4;
	surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 282, 5, Common::Rect(10, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));
	//surface->copyRectToSurface(*_something, 100, 50, Common::Rect(0, 0, _something->w, _something->h));
}

} // End of namespace Freescape
