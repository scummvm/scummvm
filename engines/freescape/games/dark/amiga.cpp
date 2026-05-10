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
#include "common/system.h"

#include "graphics/palette.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

namespace {

const int kAmigaGemdosHeaderSize = 0x1C;

int amigaProgToFile(int address) {
	return address + kAmigaGemdosHeaderSize;
}

int wrapCompassPhase(int phase, int frameCount) {
	if (frameCount <= 0)
		return 0;

	phase %= frameCount;
	if (phase < 0)
		phase += frameCount;
	return phase;
}

int darkAmigaForcedCompassStep(int areaId) {
	switch (areaId) {
	case 1:
	case 27:
	case 28:
		return 1;
	case 18:
		return -1;
	default:
		return 0;
	}
}

int yawToCompassPhase(float yaw, int frameCount) {
	while (yaw < 0.0f)
		yaw += 360.0f;
	while (yaw >= 360.0f)
		yaw -= 360.0f;

	return wrapCompassPhase((int)(yaw / 5.0f), frameCount);
}

int stepCompassPhaseTowardTarget(int current, int target, int frameCount) {
	if (frameCount <= 0 || current == target)
		return 0;

	int forwardDistance = wrapCompassPhase(target - current, frameCount);
	int backwardDistance = wrapCompassPhase(current - target, frameCount);
	return (forwardDistance < backwardDistance) ? 1 : -1;
}

void decodeMaskedAmigaSprite(Common::SeekableReadStream *file, Graphics::ManagedSurface *surf,
		int dataOffset, int widthWords, int height, const uint16 *maskWords,
		const Graphics::PixelFormat &pixelFormat, const byte *palette) {
	for (int y = 0; y < height; y++) {
		for (int col = 0; col < widthWords; col++) {
			int off = dataOffset + (y * widthWords + col) * 8;
			file->seek(off);
			uint16 p0 = file->readUint16BE();
			uint16 p1 = file->readUint16BE();
			uint16 p2 = file->readUint16BE();
			uint16 p3 = file->readUint16BE();
			for (int bit = 0; bit < 16; bit++) {
				if (maskWords[col] & (0x8000 >> bit))
					continue;

				byte colorIdx = 0;
				if (p0 & (0x8000 >> bit)) colorIdx |= 1;
				if (p1 & (0x8000 >> bit)) colorIdx |= 2;
				if (p2 & (0x8000 >> bit)) colorIdx |= 4;
				if (p3 & (0x8000 >> bit)) colorIdx |= 8;
				if (colorIdx == 0)
					continue;

				uint32 color = pixelFormat.ARGBToColor(0xFF,
					palette[colorIdx * 3], palette[colorIdx * 3 + 1], palette[colorIdx * 3 + 2]);
				surf->setPixel(col * 16 + bit, y, color);
			}
		}
	}
}

} // namespace

void DarkEngine::loadAssetsAmigaFullGame() {
	Common::File file;
	file.open("0.drk");
	// Load title image: Amiga non-interleaved bitplanes with Atari ST palette
	// Palette: 16 words at file offset 0x9934, Atari ST 3-bit $0RGB format
	file.seek(0x9934);
	Graphics::Palette pal(16);
	for (int i = 0; i < 16; i++) {
		byte v1 = file.readByte();
		byte v2 = file.readByte();
		byte r = floor((v1 & 0x07) * 255.0 / 7.0);
		byte g = floor((v2 & 0x70) * 255.0 / 7.0 / 16.0);
		byte b = floor((v2 & 0x07) * 255.0 / 7.0);
		pal.set(i, r, g, b);
	}

	// Bitplanes: 4 planes x 8000 bytes at file offset 0x99B0, non-interleaved
	file.seek(0x99B0);
	Graphics::ManagedSurface *titleSurface = new Graphics::ManagedSurface();
	titleSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	titleSurface->fillRect(Common::Rect(0, 0, 320, 200), 0);
	for (int plane = 0; plane < 4; plane++) {
		for (int y = 0; y < 200; y++) {
			for (int x = 0; x < 40; x++) {
				byte b = file.readByte();
				for (int n = 0; n < 8; n++) {
					int px = x * 8 + (7 - n);
					int bit = ((b >> n) & 0x01) << plane;
					int sample = titleSurface->getPixel(px, y) | bit;
					titleSurface->setPixel(px, y, sample);
				}
			}
		}
	}
	titleSurface->convertToInPlace(_gfx->_texturePixelFormat, pal.data(), pal.size());
	_title = titleSurface;

	// Dark Side: COLOR5 palette cycling from assembly interrupt handler at $10E4.
	// Cycles $DFF18A (COLOR5) every 2 frames through 30 entries.
	{
		const uint16 kDarkSideCyclingTable[] = {
			0x000, 0xE6D, 0x600, 0x900, 0xC00, 0xF00, 0xF30, 0xF60,
			0xF90, 0xFC0, 0xFF0, 0xAF0, 0x5F0, 0x6F8, 0x7FD, 0x7EF,
			0xBDF, 0xDDF, 0xBCF, 0x9BF, 0x7BF, 0x6BF, 0x5AF, 0x4AF,
			0x29F, 0x18F, 0x07F, 0x04C, 0x02A, 0x007
		};
		for (int i = 0; i < 30; i++)
			_gfx->_colorCyclingTable.push_back(kDarkSideCyclingTable[i]);
	}
	_gfx->_colorCyclingPaletteIndex = 5;
	_gfx->_colorCyclingSpeed = 1;
	_gfx->_colorCyclingTimer = 0; // always active

	file.close();

	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.drk", "0.drk", 798);
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0x1b762);
	load8bitBinary(stream, 0x2e96a, 16);
	loadPalettes(stream, 0x2e528);
	loadGlobalObjects(stream, 0x30f0 - 50, 24);
	loadMessagesVariableSize(stream, 0x3d37, 66);
	loadSoundsFx(stream, 0x34738 + 2, 11);

	// Load HDSMUSIC.AM music data (Wally Beben custom engine)
	// HDSMUSIC.AM is an embedded GEMDOS executable at stream offset $BA64
	const uint32 kHdsMusicOffset = 0xBA64;
	const uint32 kGemdosHeaderSize = 0x1C;
	const uint32 kHdsMusicTextSize = 0xF4BC;

	stream->seek(kHdsMusicOffset + kGemdosHeaderSize);
	_musicData.resize(kHdsMusicTextSize);
	stream->read(_musicData.data(), kHdsMusicTextSize);

	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getCharsAmigaAtariInternal(8, 8, - 7 - 8, 16, 16, stream, 0x1b0bc, 85);
	_fontBig = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 8, 0, 10, 8, stream, 0x1b0bc + 0x430, 85);
	_fontMedium = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 5, - 7 - 8, 10, 16, stream, 0x1b0bc + 0x430, 85);
	_fontSmall = Font(chars);
	_fontSmall.setCharWidth(4);

	_fontLoaded = true;

	byte *palette = getPaletteFromNeoImage(stream, 0x1b762);
	loadAmigaCompass(stream, palette);
	loadAmigaIndicatorSprites(stream, palette);
	loadJetpackRawFrames(stream);
	free(palette);

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 26);
	}
}

void DarkEngine::loadAmigaIndicatorSprites(Common::SeekableReadStream *file, byte *palette) {
	if (!palette)
		return;

	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);

	_amigaCompassNeedleFrames.clear();
	for (int frame = 0; frame < 13; frame++) {
		auto *surf = new Graphics::ManagedSurface();
		surf->create(32, 3, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 32, 3), transparent);
		decodeAmigaSprite(file, surf, amigaProgToFile(0x2784E) + frame * 0x30, 2, 3, palette);
		_amigaCompassNeedleFrames.push_back(surf);
	}

	const uint16 kLeftMasks[2] = { 0xE000, 0x01FF };
	const uint16 kRightMasks[2] = { 0xFF80, 0x003F };
	const int kSideFrameOrder[4] = { 3, 2, 1, 0 };

	_amigaCompassLeftFrames.clear();
	for (int phase = 0; phase < 4; phase++) {
		int frameIndex = kSideFrameOrder[phase];
		auto *surf = new Graphics::ManagedSurface();
		surf->create(32, 21, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 32, 21), transparent);
		decodeMaskedAmigaSprite(file, surf, amigaProgToFile(0x29B34) + frameIndex * 0x150, 2, 21,
			kLeftMasks, _gfx->_texturePixelFormat, palette);
		_amigaCompassLeftFrames.push_back(surf);
	}

	_amigaCompassRightFrames.clear();
	for (int phase = 0; phase < 4; phase++) {
		int frameIndex = kSideFrameOrder[phase];
		auto *surf = new Graphics::ManagedSurface();
		surf->create(32, 21, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 32, 21), transparent);
		decodeMaskedAmigaSprite(file, surf, amigaProgToFile(0x2A07E) + frameIndex * 0x150, 2, 21,
			kRightMasks, _gfx->_texturePixelFormat, palette);
		_amigaCompassRightFrames.push_back(surf);
	}
}

void DarkEngine::loadAmigaCompass(Common::SeekableReadStream *file, byte *palette) {
	if (!palette)
		return;

	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);

	Graphics::ManagedSurface base;
	base.create(32, 5, _gfx->_texturePixelFormat);
	base.fillRect(Common::Rect(0, 0, 32, 5), transparent);
	decodeAmigaSprite(file, &base, amigaProgToFile(0x238B4), 2, 5, palette);

	_amigaCompassYawFrames.clear();
	file->seek(amigaProgToFile(0x234CC));
	uint32 cursorMaskBase = file->readUint32BE();
	for (int pos = 0; pos < 72; pos++) {
		auto *surf = new Graphics::ManagedSurface();
		surf->create(32, 5, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 32, 5), transparent);
		surf->copyRectToSurface(base, 0, 0, Common::Rect(base.w, base.h));

		int rowOffset = amigaProgToFile(0x234D0) + ((pos >> 3) & 0xFFFE);
		int shift = pos & 0xF;
		for (int row = 0; row < 5; row++) {
			file->seek(rowOffset + row * 14);
			uint32 longVal = file->readUint32BE();
			uint16 wordVal = file->readUint16BE();
			uint32 mask = ((longVal << shift) & 0xFFFFFFFF);
			mask |= (((uint32)wordVal << shift) >> 16) & 0xFFFF;
			mask |= cursorMaskBase;

			for (int bit = 0; bit < 32; bit++) {
				if ((mask & (0x80000000 >> bit)) == 0 && base.getPixel(bit, row) != transparent)
					surf->setPixel(bit, row, black);
			}
		}

		_amigaCompassYawFrames.push_back(surf);
	}

	_amigaCompassPitchMarker = new Graphics::ManagedSurface();
	_amigaCompassPitchMarker->create(16, 9, _gfx->_texturePixelFormat);
	_amigaCompassPitchMarker->fillRect(Common::Rect(0, 0, 16, 9), transparent);
	decodeAmigaSprite(file, _amigaCompassPitchMarker, amigaProgToFile(0x27AC6), 1, 9, palette);
}

void DarkEngine::loadJetpackRawFrames(Common::SeekableReadStream *file) {
	// The executable stream still includes the 0x1C-byte GEMDOS header, so the
	// original program addresses need to be converted back to file offsets here.
	// Original Amiga layout:
	// - transition strip at prog 0x23B9E, 9 frames, stride 0x160
	// - crouch frame at prog 0x2481E
	const int kTransitionBaseOffset = 0x23B9E + kAmigaGemdosHeaderSize;
	const int kTransitionFrameCount = 9;
	const int kCrouchFrameOffset = 0x2481E + kAmigaGemdosHeaderSize;
	const int kFrameSize = 0x160; // 2 word columns * 22 rows * 8 bytes/row
	_jetpackTransitionFrames.clear();
	for (int i = 0; i < kTransitionFrameCount; i++) {
		file->seek(kTransitionBaseOffset + i * kFrameSize);
		Common::Array<byte> raw(kFrameSize);
		file->read(raw.data(), kFrameSize);
		_jetpackTransitionFrames.push_back(raw);
	}

	file->seek(kCrouchFrameOffset);
	_jetpackCrouchFrame.resize(kFrameSize);
	file->read(_jetpackCrouchFrame.data(), kFrameSize);

	_jetpackIndicatorStateInitialized = false;
	_jetpackIndicatorTransitionDirection = 0;
}

void DarkEngine::drawJetpackIndicator(Graphics::Surface *surface) {
	const int kTransitionFrameCount = 9;
	const uint32 kFrameDelayMs = 60;
	const int kVisibleLeftX = 109;
	const int kSourceLeftPadding = 13;
	const int kDrawBaseX = kVisibleLeftX - kSourceLeftPadding;
	const int kHeight = 22;
	const int kWidthWords = 2;
	const int kDrawY = 175;
	const uint16 kMaskWords[kWidthWords] = { 0xFFF8, 0x00FF };
	const int kFlyingBaseFrame = 0;
	const int kGroundStandingFrame = 8;
	const uint16 kJetpackColors[16] = {
		0x000, 0x222, 0x000, 0x000,
		0x000, 0x000, 0x444, 0x666,
		0x000, 0x800, 0xA00, 0xF00,
		0xF80, 0xFD0, 0x000, 0x000
	};

	if (_jetpackTransitionFrames.size() != kTransitionFrameCount || _jetpackCrouchFrame.empty())
		return;

	if (!_jetpackIndicatorStateInitialized) {
		_jetpackIndicatorStateInitialized = true;
		_jetpackIndicatorLastFlyMode = _flyMode;
		_jetpackIndicatorTransitionFrame = _flyMode ? 0 : kTransitionFrameCount - 1;
		_jetpackIndicatorTransitionDirection = 0;
		_jetpackIndicatorNextFrameMillis = 0;
	} else if (_jetpackIndicatorLastFlyMode != _flyMode) {
		// The original routines at 0x89F4 and 0x8ACC play 8->0 on enable
		// and 0->8 on disable via $D18.
		_jetpackIndicatorLastFlyMode = _flyMode;
		_jetpackIndicatorTransitionFrame = _flyMode ? kTransitionFrameCount - 1 : 0;
		_jetpackIndicatorTransitionDirection = _flyMode ? -1 : 1;
		_jetpackIndicatorNextFrameMillis = g_system->getMillis() + kFrameDelayMs;
	}

	if (_jetpackIndicatorTransitionDirection != 0) {
		uint32 now = g_system->getMillis();
		while (now >= _jetpackIndicatorNextFrameMillis) {
			int nextFrame = _jetpackIndicatorTransitionFrame + _jetpackIndicatorTransitionDirection;
			if (nextFrame < 0 || nextFrame >= kTransitionFrameCount) {
				_jetpackIndicatorTransitionDirection = 0;
				break;
			}
			_jetpackIndicatorTransitionFrame = nextFrame;
			_jetpackIndicatorNextFrameMillis += kFrameDelayMs;
		}
	}

	const byte *raw = nullptr;
	if (_jetpackIndicatorTransitionDirection != 0) {
		raw = _jetpackTransitionFrames[_jetpackIndicatorTransitionFrame].data();
	} else if (_flyMode) {
		// 0x89F4 leaves the grounded strip on frame 0 after the 8->0 startup
		// transition. The later 0x24988 path in FUN_106A is an incremental
		// overlay, so the redraw-from-scratch UI still needs the base frame.
		raw = _jetpackTransitionFrames[kFlyingBaseFrame].data();
	} else {
		// Dark uses two grounded stances. The engine-side stance maps to the
		// same standing/crouch split used by the other Dark ports.
		raw = (_playerHeightNumber == 0) ? _jetpackCrouchFrame.data() : _jetpackTransitionFrames[kGroundStandingFrame].data();
	}

	for (int y = 0; y < kHeight; y++) {
		for (int col = 0; col < kWidthWords; col++) {
			int off = (y * kWidthWords + col) * 8;
			uint16 srcPlanes[4] = {
				(uint16)((raw[off] << 8) | raw[off + 1]),
				(uint16)((raw[off + 2] << 8) | raw[off + 3]),
				(uint16)((raw[off + 4] << 8) | raw[off + 5]),
				(uint16)((raw[off + 6] << 8) | raw[off + 7])
			};
			for (int bit = 0; bit < 16; bit++) {
				if (kMaskWords[col] & (0x8000 >> bit))
					continue;

				int px = col * 16 + bit;
				byte colorIdx = 0;
				if (srcPlanes[0] & (0x8000 >> bit)) colorIdx |= 1;
				if (srcPlanes[1] & (0x8000 >> bit)) colorIdx |= 2;
				if (srcPlanes[2] & (0x8000 >> bit)) colorIdx |= 4;
				if (srcPlanes[3] & (0x8000 >> bit)) colorIdx |= 8;
				uint16 colorWord = kJetpackColors[colorIdx];
				uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
					((colorWord >> 8) & 0xF) * 17,
					((colorWord >> 4) & 0xF) * 17,
					(colorWord & 0xF) * 17);
				surface->setPixel(kDrawBaseX + px, kDrawY + y, color);
			}
		}
	}
}

void DarkEngine::drawAmigaCompass(Graphics::Surface *surface) {
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);

	if (!_amigaCompassYawFrames.empty()) {
		const int frameCount = _amigaCompassYawFrames.size();
		const int targetPhase = yawToCompassPhase(_yaw, frameCount);
		if (!_amigaCompassYawPhaseInitialized) {
			_amigaCompassYawPhaseInitialized = true;
			_amigaCompassYawPhase = targetPhase;
			_amigaCompassYawLastUpdateTick = _ticks;
		} else if (_amigaCompassYawLastUpdateTick != _ticks) {
			int step = 0;
			if (_currentArea)
				step = darkAmigaForcedCompassStep(_currentArea->getAreaID());
			if (step == 0)
				step = stepCompassPhaseTowardTarget(_amigaCompassYawPhase, targetPhase, frameCount);

			_amigaCompassYawPhase = wrapCompassPhase(_amigaCompassYawPhase + step, frameCount);
			_amigaCompassYawLastUpdateTick = _ticks;
		}

		const int frame = wrapCompassPhase(_amigaCompassYawPhase, frameCount);
		surface->copyRectToSurfaceWithKey(*_amigaCompassYawFrames[frame], 48, 15,
			Common::Rect(_amigaCompassYawFrames[frame]->w, _amigaCompassYawFrames[frame]->h), transparent);
	}

	if (_amigaCompassPitchMarker) {
		int pos = CLIP<int>((int)(_pitch / 1.65f), -36, 36);
		surface->copyRectToSurfaceWithKey(*_amigaCompassPitchMarker, 304, 94 + pos,
			Common::Rect(_amigaCompassPitchMarker->w, _amigaCompassPitchMarker->h), transparent);
	}
}

void DarkEngine::drawAmigaAmbientIndicators(Graphics::Surface *surface) {
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);

	if (!_amigaCompassNeedleFrames.empty()) {
		int frame = (_ticks / 4) % _amigaCompassNeedleFrames.size();
		surface->copyRectToSurfaceWithKey(*_amigaCompassNeedleFrames[frame], 208, 21,
			Common::Rect(_amigaCompassNeedleFrames[frame]->w, _amigaCompassNeedleFrames[frame]->h), transparent);
	}

	if (!_amigaCompassLeftFrames.empty()) {
		int frame = (_ticks / 5) % _amigaCompassLeftFrames.size();
		surface->copyRectToSurfaceWithKey(*_amigaCompassLeftFrames[frame], 0, 143,
			Common::Rect(_amigaCompassLeftFrames[frame]->w, _amigaCompassLeftFrames[frame]->h), transparent);
	}

	if (!_amigaCompassRightFrames.empty()) {
		int frame = (_ticks / 5) % _amigaCompassRightFrames.size();
		surface->copyRectToSurfaceWithKey(*_amigaCompassRightFrames[frame], 288, 143,
			Common::Rect(_amigaCompassRightFrames[frame]->w, _amigaCompassRightFrames[frame]->h), transparent);
	}
}

void DarkEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0xCC, 0x00);
	uint32 orange = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x88, 0x00);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x00, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 grey = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x60, 0x60);

	uint32 grey8 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x88, 0x88, 0x88);
	uint32 greyA = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.x())), 19, 178, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.z())), 19, 184, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.y())), 19, 190, red, red, black, surface);

	drawString(kDarkFontBig, Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 73, 178, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d", _playerSteps[_playerStepIndex]), 73, 186, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%07d", score), 93, 16, orange, yellow, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d%%", ecds), 181, 16, orange, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawString(kDarkFontSmall, message, 32, 157, grey8, greyA, transparent, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	}

	drawString(kDarkFontSmall, _currentArea->_name, 32, 151, grey8, greyA, transparent, surface);
	drawBinaryClock(surface, 6, 110, white, grey);

	drawAmigaCompass(surface);
	drawAmigaAmbientIndicators(surface);
	drawJetpackIndicator(surface);

	int x = 229;
	int y = 180;
	for (int i = 0; i < _maxShield / 2; i++) {
		if (i < _gameStateVars[k8bitVariableShield] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}

	x = 229;
	y = 188;
	for (int i = 0; i < _maxEnergy / 2; i++) {
		if (i < _gameStateVars[k8bitVariableEnergy] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}
}

void DarkEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 33, 287, 130);
}

void DarkEngine::drawString(const DarkFontSize size, const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;

	Font *font = nullptr;

	if (size == kDarkFontBig) {
		font = &_fontBig;
	} else if (size == kDarkFontMedium) {
		font = &_fontMedium;
	} else if (size == kDarkFontSmall) {
		font = &_fontSmall;
	} else {
		error("Invalid font size %d", size);
		return;
	}

	Common::String ustr = str;
	ustr.toUppercase();
	font->setBackground(backColor);
	font->setSecondaryColor(secondaryColor);
	font->drawString(surface, ustr, x, y, _screenW, primaryColor);
}

} // End of namespace Freescape
