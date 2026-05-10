
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
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern const byte kBorderPalette[16 * 3];
extern const int8 kAtariCompassPhaseToFrame[];
extern const int kAtariCompassPhaseCount;
extern const int kAtariCompassBaseFrames;
extern const int kAtariCompassTotalFrames;
extern const uint32 kAtariAreaRecordBase;
extern const uint32 kAtariAreaIndexBase;
extern const int kAtariAreaIndexCount;
extern const uint16 kAtariDarkAreaFlag;

Graphics::ManagedSurface *loadAtariSTRawSprite(Common::SeekableReadStream *stream,
		int pixelOffset, int cols, int rows);
Graphics::ManagedSurface *loadAtariSTSprite(Common::SeekableReadStream *stream,
		int maskOffset, int pixelOffset, int cols, int rows);
Graphics::ManagedSurface *loadAtariSTNeedleSprite(Common::SeekableReadStream *stream,
		int pixelOffset, const Graphics::PixelFormat &format);
void buildAtariSTCompassMirrorCache(Common::SeekableReadStream *stream, int pixelOffset,
		Common::Array<Graphics::ManagedSurface *> &sprites, const Graphics::PixelFormat &format);
bool containsAtariAreaID(const Common::Array<uint16> &areaIDs, uint16 areaID);

void EclipseEngine::loadAssetsAmigaFullGame() {
	// Decrypt 1.tec using 0.tec as the unpacker (unpack array at file offset $454).
	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.tec", "0.tec", 0x454);

	parseAmigaAtariHeader(stream);

	// The Amiga decrypted stream shares identical game data with the Atari ST
	// binary, but at a consistent offset delta of +0xC87A for most resources.
	// Messages and the eclipse fade palette are at different locations.
	static const int kAmigaDelta = 0xC87A;

	loadMessagesVariableSize(stream, 0x9864, 28);
	load8bitBinary(stream, 0x2a53c + kAmigaDelta, 16);

	_border = loadAndConvertNeoImage(stream, 0x139c8 + kAmigaDelta);
	loadPalettes(stream, 0x29e52 + kAmigaDelta);

	// Colors 0-5 from border palette (Copper list palette split, same as Atari ST Timer-B)
	for (auto &entry : _paletteByArea) {
		byte *pal = entry._value;
		memcpy(pal, kBorderPalette, 6 * 3);
	}

	loadSoundsFx(stream, 0x3030c + kAmigaDelta, 6);

	// Load TEMUSIC.AM — Wally Beben custom Paula engine (same family as Dark Side)
	// GEMDOS executable embedded at stream offset 0x10F5E, TEXT size 0xEB20
	{
		static const uint32 kTEMusicAmigaOffset = 0x10F5E;
		static const uint32 kGemdosHeaderSize = 0x1C;
		static const uint32 kTEMusicAmigaTextSize = 0xEB20;
		stream->seek(kTEMusicAmigaOffset + kGemdosHeaderSize);
		_musicData.resize(kTEMusicAmigaTextSize);
		stream->read(_musicData.data(), kTEMusicAmigaTextSize);
		debug(3, "TE-Amiga: Loaded TEMUSIC.AM TEXT segment (%d bytes)", kTEMusicAmigaTextSize);
	}

	// UI font (Font A): same 4-plane format as Atari ST
	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getChars4Plane(stream, 0x24C5A + kAmigaDelta, 85);
	_font = Font(chars);

	// Score font (Font B)
	Common::Array<Graphics::ManagedSurface *> scoreChars;
	scoreChars = getChars4Plane(stream, 0x249DA + kAmigaDelta, 11);
	_fontScore = Font(scoreChars);

	// Sprite offsets: Atari prog addr + 0x1C (GEMDOS hdr) + kAmigaDelta
	static const int kHdr = 0x1C + kAmigaDelta;

	_atariDarkAreas.clear();
	Common::Array<uint16> parsedAreas;
	uint32 streamSize = stream->size();
	for (int i = 0; i < kAtariAreaIndexCount; i++) {
		uint32 indexOffset = kAtariAreaIndexBase + kHdr + i * 4;
		if (indexOffset + 4 > streamSize)
			break;

		stream->seek(indexOffset);
		uint16 lowWord = stream->readUint16BE();
		uint16 highWord = stream->readUint16BE();
		uint32 recordWordOffset = lowWord | ((uint32)highWord << 8);
		uint32 recordOffset = kAtariAreaRecordBase + kHdr + recordWordOffset * 2;
		if (recordOffset + 6 > streamSize)
			continue;

		stream->seek(recordOffset);
		uint16 flags = stream->readUint16BE();
		stream->readUint16BE();
		uint16 areaID = stream->readUint16BE();
		if (!_areaMap.contains(areaID) || containsAtariAreaID(parsedAreas, areaID))
			continue;

		parsedAreas.push_back(areaID);
		if ((flags & kAtariDarkAreaFlag) != 0)
			_atariDarkAreas.push_back(areaID);
	}

	// Eclipse fade palettes for dark areas (at a different offset than the main delta)
	for (int level = 0; level < 6; level++) {
		stream->seek(0xFE66 + level * 32);
		byte *pal = loadPalette(stream);
		memcpy(_eclipseFadePalettes[level], pal, 16 * 3);
		delete[] pal;
	}

	for (uint i = 0; i < _atariDarkAreas.size(); i++)
		applyEclipseFadePalette(_atariDarkAreas[i], _lanternBatteryLevel);

	// Heart indicator sprites
	_eclipseSprites.resize(2);
	_eclipseSprites[0] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + kHdr, 1, 13);
	_eclipseSprites[0]->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);
	_eclipseSprites[1] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + 104 + kHdr, 1, 13);
	_eclipseSprites[1]->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	// Eclipse progress indicator: 16 frames
	_eclipseProgressSprites.resize(16);
	for (int i = 0; i < 16; i++) {
		_eclipseProgressSprites[i] = loadAtariSTSprite(stream, 0x1DA96 + kHdr, 0x1DA98 + kHdr + i * 128, 1, 16);
		_eclipseProgressSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Ankh indicator: 5 fade-in frames
	_ankhSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_ankhSprites[i] = loadAtariSTSprite(stream, 0x1B732 + kHdr, 0x1B734 + kHdr + i * 120, 1, 15);
		_ankhSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Compass
	_compassBackground = loadAtariSTRawSprite(stream, 0x20986 + kHdr, 2, 27);
	_compassBackground->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	{
		Common::copy(kAtariCompassPhaseToFrame, kAtariCompassPhaseToFrame + kAtariCompassPhaseCount, _compassLookup);

		_compassSprites.resize(kAtariCompassTotalFrames);
		for (int frame = 0; frame < kAtariCompassBaseFrames; frame++) {
			_compassSprites[frame] = loadAtariSTNeedleSprite(stream,
				0x20B36 + kHdr + frame * 432, _gfx->_texturePixelFormat);
		}
		buildAtariSTCompassMirrorCache(stream, 0x20B36 + kHdr, _compassSprites, _gfx->_texturePixelFormat);
	}

	// Lantern light animation: 6 frames
	_lanternLightSprites.resize(6);
	for (int i = 0; i < 6; i++) {
		_lanternLightSprites[i] = loadAtariSTRawSprite(stream, 0x2026A + kHdr + i * 96, 2, 6);
		_lanternLightSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Lantern switch: 2 frames
	_lanternSwitchSprites.resize(2);
	for (int i = 0; i < 2; i++) {
		_lanternSwitchSprites[i] = loadAtariSTRawSprite(stream, 0x204B4 + kHdr + i * 0x170, 2, 23);
		_lanternSwitchSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Sound toggle: 5 frames
	_soundToggleSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_soundToggleSprites[i] = loadAtariSTRawSprite(stream, 0x20794 + kHdr + i * 96, 1, 11);
		_soundToggleSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Water body bitmap
	_atariWaterBody = loadAtariSTRawSprite(stream, 0x2003C + kHdr, 2, 31);
	_atariWaterBody->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	stream->seek(0x2024C + kHdr);
	_atariWaterSurfacePixels.resize(8);
	for (int w = 0; w < 8; w++)
		_atariWaterSurfacePixels[w] = stream->readUint16BE();

	_atariWaterSurfaceMask.resize(2);
	stream->seek(0x2025C + kHdr);
	for (int w = 0; w < 2; w++)
		_atariWaterSurfaceMask[w] = stream->readUint16BE();

	// Shooting crosshair: 2 frames
	_shootSprites.resize(2);
	_shootSprites[0] = loadAtariSTSprite(stream, 0x1CC2C + kHdr, 0x1CC30 + kHdr, 2, 25);
	_shootSprites[0]->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);
	_shootSprites[1] = loadAtariSTSprite(stream, 0x1CDC6 + kHdr, 0x1CDCC + kHdr, 3, 25);
	_shootSprites[1]->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	delete stream;
}

} // End of namespace Freescape
