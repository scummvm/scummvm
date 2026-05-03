
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


// Forward declaration (defined in atari.music.cpp)
MusicPlayer *makeEclipseAtariMusicPlayer(const byte *data, uint32 dataSize,
                                                  int songNum = 1);

extern const int kAtariCompassPhaseCount = 72;
extern const int kAtariCompassBaseFrames = 19;
extern const int kAtariCompassTotalFrames = 37;
const int kAtariClockCenterX = 106;
const int kAtariClockCenterY = 159;
const int kAtariCompassX = 176;
const int kAtariCompassY = 151;
const int kAtariWaterIndicatorX = 224;
const int kAtariWaterIndicatorY = 154;
const int kAtariWaterIndicatorMaxLevel = 29;
// The original 68K code at $07BA renders the dark room light hole using a
// pre-built bitplane mask. The fully-visible center spans 6 word-groups
// (96px) with 2 masked border groups (32px each) on either side, for a
// total visible width of ~160px in the 256px-wide viewport. The vertical
// extent is similarly masked per-scanline. This corresponds to a circle
// radius of approximately 80 pixels. The lantern battery (6 levels)
// scales this down proportionally.
const int kAtariDarkLightRadius = 80;
const int kAtariDarkLightRadiusStep = 10;
extern const uint32 kAtariAreaRecordBase = 0x2A520;
extern const uint32 kAtariAreaIndexBase = 0x2A6B0;
extern const int kAtariAreaIndexCount = 64;
extern const uint16 kAtariDarkAreaFlag = 0x8000;

void fillCircle(Graphics::Surface *surface, int x, int y, int radius, int color);

// Repaired ST phase-to-frame table from $1542. Six corrupt bytes in the dumped
// binary break the intended 37-frame sweep built from $20B36 and $22B46.
extern const int8 kAtariCompassPhaseToFrame[kAtariCompassPhaseCount] = {
	0, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
	36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
};

void EclipseEngine::applyEclipseFadePalette(uint16 areaID, int brightnessLevel) {
	if (!_paletteByArea.contains(areaID))
		return;
	brightnessLevel = CLIP(brightnessLevel, 0, 5);
	byte *pal = _paletteByArea[areaID];
	// Colors 0-5 stay as the border palette; overwrite 6-15 from the fade table
	memcpy(pal + 6 * 3, _eclipseFadePalettes[brightnessLevel] + 6 * 3, 10 * 3);
}

void EclipseEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 16, 288, 118);

	// On-screen control hotspots (from binary hotspot table at prog $869A)
	// Right-side arrow buttons
	_lookUpArea = Common::Rect(268, 133, 288, 153);
	_lookDownArea = Common::Rect(290, 133, 310, 153);
	_turnLeftArea = Common::Rect(268, 155, 288, 175);
	_turnRightArea = Common::Rect(290, 155, 310, 175);
	_uTurnArea = Common::Rect(268, 177, 288, 197);
	_faceForwardArea = Common::Rect(290, 177, 310, 197);

	// Left-side buttons
	_moveBackwardArea = Common::Rect(9, 133, 29, 153);
	_stepBackwardArea = Common::Rect(9, 155, 29, 175);
	_interactArea = Common::Rect(31, 155, 51, 175);
	_infoDisplayArea = Common::Rect(31, 133, 51, 153);

	// Center/functional areas
	_lanternArea = Common::Rect(57, 138, 75, 168);
	_restArea = Common::Rect(85, 140, 127, 177);

	// Status bar indicators
	_stepSizeArea = Common::Rect(74, 117, 86, 129);
	_heightArea = Common::Rect(222, 117, 234, 129);

	// Save/load (menu screen)
	_saveGameArea = Common::Rect(180, 36, 190, 47);
	_loadGameArea = Common::Rect(180, 50, 190, 60);
}

/*void EclipseEngine::loadAssetsCPCFullGame() {
	Common::File file;

	if (isEclipse2())
		file.open("TE2.BI1");
	else
		file.open("TESCR.SCR");

	if (!file.isOpen())
		error("Failed to open TESCR.SCR/TE2.BI1");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI3");
	else
		file.open("TECON.SCR");

	if (!file.isOpen())
		error("Failed to open TECON.SCR/TE2.BI3");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	if (isEclipse2())
		file.open("TE2.BI2");
	else
		file.open("TECODE.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	if (isEclipse2()) {
		loadFonts(&file, 0x60bc, _font);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x62b4, 16);
	} else {
		loadFonts(&file, 0x6076, _font);
		loadMessagesFixedSize(&file, 0x326, 16, 30);
		load8bitBinary(&file, 0x626e, 16);
	}

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		if (isEclipse2() && it._value->getAreaID() == 1)
			continue;

		if (isEclipse2() && it._value->getAreaID() == _startArea)
			continue;

		for (int16 id = 183; id < 207; id++)
			it._value->addObjectFromArea(id, _areaMap[255]);
	}
	loadColorPalette();
	swapPalette(1);

	_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void EclipseEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _currentArea->_underFireBackgroundColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_inkColor;

	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 135, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 135, back, front, surface);

	Common::String scoreStr = Common::String::format("%07d", score);
	drawStringInSurface(scoreStr, 136, 6, back, other, surface, 'Z' - '0' + 1);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	Common::String shieldStr = Common::String::format("%d", shield);
	drawStringInSurface(shieldStr, x, 162, back, other, surface);

	drawStringInSurface(Common::String('0' + _angleRotationIndex - 3), 79, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('3' - _playerStepIndex), 63, 135, back, front, surface, 'Z' - '$' + 1);
	drawStringInSurface(Common::String('7' - _playerHeightNumber), 240, 135, back, front, surface, 'Z' - '$' + 1);

	if (_shootingFrames > 0) {
		drawStringInSurface("4", 232, 135, back, front, surface, 'Z' - '$' + 1);
		drawStringInSurface("<", 240, 135, back, front, surface, 'Z' - '$' + 1);
	}
	drawAnalogClock(surface, 90, 172, back, other, front);
	drawIndicator(surface, 45, 4, 12);
	drawEclipseIndicator(surface, 228, 0, front, other);
}*/

// Border palette from CONSOLE.NEO (Atari ST 9-bit $0RGB, scaled to 8-bit).
// Used for font rendering and sprite conversion.
extern const byte kBorderPalette[16 * 3] = {
	0, 0, 0,        // 0: $000 black
	145, 72, 0,     // 1: $420 dark brown
	182, 109, 36,   // 2: $531 medium brown
	218, 145, 36,   // 3: $641 golden brown
	255, 182, 36,   // 4: $751 bright gold
	255, 218, 145,  // 5: $764
	218, 218, 218,  // 6: $666
	182, 182, 182,  // 7: $555
	145, 145, 145,  // 8: $444
	109, 109, 109,  // 9: $333
	72, 72, 72,     // 10: $222
	182, 36, 0,     // 11: $510 dark red
	255, 72, 0,     // 12: $720
	255, 109, 0,    // 13: $730
	255, 145, 0,    // 14: $740
	255, 255, 255,  // 15: $777 white
};

// Load raw 4-plane pixel data (no mask) from stream into a CLUT8 surface.
Graphics::ManagedSurface *loadAtariSTRawSprite(Common::SeekableReadStream *stream,
		int pixelOffset, int cols, int rows) {
	stream->seek(pixelOffset);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(cols * 16, rows, Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			uint16 p0 = stream->readUint16BE();
			uint16 p1 = stream->readUint16BE();
			uint16 p2 = stream->readUint16BE();
			uint16 p3 = stream->readUint16BE();
			for (int bit = 15; bit >= 0; bit--) {
				int x = col * 16 + (15 - bit);
				byte color = ((p0 >> bit) & 1)
				           | (((p1 >> bit) & 1) << 1)
				           | (((p2 >> bit) & 1) << 2)
				           | (((p3 >> bit) & 1) << 3);
				surface->setPixel(x, row, color);
			}
		}
	}
	return surface;
}

float atariCompassAbs(float value) {
	return value < 0.0f ? -value : value;
}

int atariCompassRoundToNearestInt(float value) {
	return value >= 0.0f ? (int)(value + 0.5f) : (int)(value - 0.5f);
}

int wrapAtariCompassPhase(int phase) {
	phase %= kAtariCompassPhaseCount;
	if (phase < 0)
		phase += kAtariCompassPhaseCount;
	return phase;
}

float normalizeAtariCompassYaw(float yaw) {
	while (yaw < 0.0f)
		yaw += 360.0f;
	while (yaw >= 360.0f)
		yaw -= 360.0f;
	return yaw;
}

int atariCompassPhaseToYaw(int phase) {
	int rotationY = wrapAtariCompassPhase(phase) * 5;
	int yaw = 0;
	if (rotationY < 90) {
		yaw = 90 - rotationY;
	} else if (rotationY <= 180) {
		yaw = 450 - rotationY;
	} else if (rotationY <= 225) {
		yaw = rotationY;
	} else if (rotationY < 270) {
		yaw = rotationY - 90;
	} else {
		yaw = 450 - rotationY;
	}

	while (yaw < 0)
		yaw += 360;
	while (yaw >= 360)
		yaw -= 360;
	return yaw;
}

float atariCompassAngularDistance(float a, float b) {
	float diff = atariCompassAbs(a - b);
	if (diff > 180.0f)
		diff = 360.0f - diff;
	return diff;
}

int stepAtariCompassPhaseTowardTarget(int currentPhase, int targetPhase) {
	if (currentPhase == targetPhase)
		return 0;

	int forwardDistance = wrapAtariCompassPhase(targetPhase - currentPhase);
	int backwardDistance = wrapAtariCompassPhase(currentPhase - targetPhase);
	return (forwardDistance < backwardDistance) ? 1 : -1;
}

bool atariCompassPhaseUsesVerticalFlip(int phase) {
	int wrappedPhase = wrapAtariCompassPhase(phase);
	return wrappedPhase < 18 || wrappedPhase > 54;
}

void atariCompassRoxl(uint32 &value, uint32 &extend) {
	uint32 nextExtend = (value >> 31) & 1;
	value = ((value << 1) | extend) & 0xFFFFFFFF;
	extend = nextExtend;
}

void atariCompassRoxr(uint32 &value, uint32 &extend) {
	uint32 nextExtend = value & 1;
	value = ((extend << 31) | (value >> 1)) & 0xFFFFFFFF;
	extend = nextExtend;
}

void decodeAtariSTNeedleRow(Graphics::ManagedSurface *surface, int row, const uint16 *rowWords,
		const Graphics::PixelFormat &pixelFormat) {
	for (int half = 0; half < 2; half++) {
		int offset = half * 4;
		for (int bit = 15; bit >= 0; bit--) {
			byte colorIndex = ((rowWords[offset + 0] >> bit) & 1)
			                | (((rowWords[offset + 1] >> bit) & 1) << 1)
			                | (((rowWords[offset + 2] >> bit) & 1) << 2)
			                | (((rowWords[offset + 3] >> bit) & 1) << 3);
			if (colorIndex == 0)
				continue;

			int x = half * 16 + (15 - bit);
			uint32 color = pixelFormat.ARGBToColor(0xFF,
				kBorderPalette[colorIndex * 3],
				kBorderPalette[colorIndex * 3 + 1],
				kBorderPalette[colorIndex * 3 + 2]);
			surface->setPixel(x, row, color);
		}
	}
}

Graphics::ManagedSurface *loadAtariSTNeedleSprite(Common::SeekableReadStream *stream,
		int pixelOffset, const Graphics::PixelFormat &pixelFormat) {
	stream->seek(pixelOffset);

	uint32 transparent = pixelFormat.ARGBToColor(0x00, 0, 0, 0);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(32, 27, pixelFormat);
	surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), transparent);

	for (int row = 0; row < 27; row++) {
		uint16 sourceWords[8];
		for (int word = 0; word < 8; word++)
			sourceWords[word] = stream->readUint16BE();
		decodeAtariSTNeedleRow(surface, row, sourceWords, pixelFormat);
	}

	return surface;
}

void buildAtariSTCompassMirrorCache(Common::SeekableReadStream *stream, int pixelOffset,
		Common::Array<Graphics::ManagedSurface *> &sprites, const Graphics::PixelFormat &pixelFormat) {
	stream->seek(pixelOffset + 432);

	uint32 extend = 0;
	uint32 transformed = 0;

	for (int frame = 1; frame < kAtariCompassBaseFrames; frame++) {
		uint32 transparent = pixelFormat.ARGBToColor(0x00, 0, 0, 0);
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(32, 27, pixelFormat);
		surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), transparent);

		for (int row = 0; row < 27; row++) {
			uint16 sourceWords[8];
			uint16 rowWords[8];
			for (int word = 0; word < 8; word++)
				sourceWords[word] = stream->readUint16BE();

			for (int plane = 0; plane < 4; plane++) {
				uint32 value = ((uint32)sourceWords[plane] << 16) | sourceWords[plane + 4];
				for (int bit = 0; bit <= 0x20; bit++) {
					atariCompassRoxl(value, extend);
					atariCompassRoxr(transformed, extend);
				}
				rowWords[plane] = transformed >> 16;
				rowWords[plane + 4] = transformed & 0xFFFF;
			}

			decodeAtariSTNeedleRow(surface, row, rowWords, pixelFormat);
		}

		sprites[18 + frame] = surface;
	}
}

Graphics::ManagedSurface *loadAtariSTSprite(Common::SeekableReadStream *stream,
		int maskOffset, int pixelOffset, int cols, int rows) {
	// Read per-column mask (1 word per column, same for all rows)
	stream->seek(maskOffset);
	Common::Array<uint16> mask(cols);
	for (int c = 0; c < cols; c++)
		mask[c] = stream->readUint16BE();

	// Read pixel data: sequential 4-plane words, cols per row
	stream->seek(pixelOffset);
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(cols * 16, rows, Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			uint16 p0 = stream->readUint16BE();
			uint16 p1 = stream->readUint16BE();
			uint16 p2 = stream->readUint16BE();
			uint16 p3 = stream->readUint16BE();
			for (int bit = 15; bit >= 0; bit--) {
				int x = col * 16 + (15 - bit);
				byte color = ((p0 >> bit) & 1)
				           | (((p1 >> bit) & 1) << 1)
				           | (((p2 >> bit) & 1) << 2)
				           | (((p3 >> bit) & 1) << 3);
				surface->setPixel(x, row, color);
			}
		}
	}
	return surface;
}

void drawAtariCompassNeedle(Graphics::Surface *surface, const Graphics::ManagedSurface *needle,
		int x, int y, bool flipVertically, uint32 transparent) {
	for (int destY = 0; destY < needle->h; destY++) {
		int srcY = flipVertically ? (needle->h - 1 - destY) : destY;
		for (int srcX = 0; srcX < needle->w; srcX++) {
			uint32 pixel = needle->getPixel(srcX, srcY);
			if (pixel != transparent)
				surface->setPixel(x + srcX, y + destY, pixel);
		}
	}
}

uint32 getAtariBorderColor(const Graphics::PixelFormat &pixelFormat, byte colorIndex) {
	return pixelFormat.ARGBToColor(0xFF,
		kBorderPalette[colorIndex * 3],
		kBorderPalette[colorIndex * 3 + 1],
		kBorderPalette[colorIndex * 3 + 2]);
}

bool containsAtariAreaID(const Common::Array<uint16> &areaIDs, uint16 areaID) {
	for (uint i = 0; i < areaIDs.size(); i++) {
		if (areaIDs[i] == areaID)
			return true;
	}
	return false;
}

int getAtariLanternHoleRadius(int lanternFrame) {
	if (lanternFrame < 0)
		return 0;

	int radius = kAtariDarkLightRadius - lanternFrame * kAtariDarkLightRadiusStep;
	if (radius < 0)
		radius = 0;
	return radius;
}

void advanceAtariLanternAnimation(EclipseEngine *engine) {
	if (engine->_atariLanternAnimationDirection == 0 || engine->_atariLanternLastUpdateTick == engine->_ticks)
		return;

	if (engine->_atariLanternAnimationDirection < 0) {
		if (engine->_atariLanternLightFrame > 0)
			engine->_atariLanternLightFrame--;
		else
			engine->_atariLanternAnimationDirection = 0;
	} else if (engine->_atariLanternLightFrame < 5) {
		engine->_atariLanternLightFrame++;
	} else {
		engine->_atariLanternLightFrame = -1;
		engine->_atariLanternAnimationDirection = 0;
	}

	engine->_atariLanternLastUpdateTick = engine->_ticks;
}

void drawAtariDarknessMask(Graphics::Surface *surface, const Common::Rect &viewArea,
		int holeX, int holeY, int holeRadius, const Graphics::PixelFormat &pixelFormat) {
	uint32 blackout = pixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	int transparent = pixelFormat.ARGBToColor(0x00, 0, 0, 0);

	surface->fillRect(viewArea, blackout);
	if (holeRadius <= 0)
		return;

	if (holeX < viewArea.left)
		holeX = viewArea.left;
	else if (holeX >= viewArea.right)
		holeX = viewArea.right - 1;

	if (holeY < viewArea.top)
		holeY = viewArea.top;
	else if (holeY >= viewArea.bottom)
		holeY = viewArea.bottom - 1;

	fillCircle(surface, holeX, holeY, holeRadius, transparent);
}

void drawAtariWaterSurfaceRow(Graphics::Surface *surface, int x, int y,
		const Common::Array<uint16> &maskWords, const Common::Array<uint16> &pixelWords,
		const Graphics::PixelFormat &pixelFormat) {
	if (maskWords.size() != 2 || pixelWords.size() != 8)
		return;

	for (uint col = 0; col < maskWords.size(); col++) {
		uint16 mask = maskWords[col];
		int offset = col * 4;
		for (int bit = 15; bit >= 0; bit--) {
			if ((mask >> bit) & 1)
				continue;

			byte colorIndex = ((pixelWords[offset + 0] >> bit) & 1)
			                | (((pixelWords[offset + 1] >> bit) & 1) << 1)
			                | (((pixelWords[offset + 2] >> bit) & 1) << 2)
			                | (((pixelWords[offset + 3] >> bit) & 1) << 3);
			surface->setPixel(x + col * 16 + (15 - bit), y,
				getAtariBorderColor(pixelFormat, colorIndex));
		}
	}
}

int EclipseEngine::atariCompassPhaseFromRotationY(float rotationY) const {
	return wrapAtariCompassPhase(atariCompassRoundToNearestInt(rotationY / 5.0f));
}

int EclipseEngine::atariCompassTargetPhaseFromYaw(float yaw, int referencePhase) const {
	float normalizedYaw = normalizeAtariCompassYaw(yaw);
	int wrappedReferencePhase = wrapAtariCompassPhase(referencePhase);
	int bestPhase = wrappedReferencePhase;
	float bestYawError = 361.0f;
	int bestPhaseDistance = kAtariCompassPhaseCount + 1;

	for (int phase = 0; phase < kAtariCompassPhaseCount; phase++) {
		float yawError = atariCompassAngularDistance(normalizedYaw, (float)atariCompassPhaseToYaw(phase));
		int phaseDistance = MIN(wrapAtariCompassPhase(phase - wrappedReferencePhase),
			wrapAtariCompassPhase(wrappedReferencePhase - phase));
		if (yawError + 0.001f < bestYawError ||
				(atariCompassAbs(yawError - bestYawError) <= 0.001f && phaseDistance < bestPhaseDistance)) {
			bestPhase = phase;
			bestYawError = yawError;
			bestPhaseDistance = phaseDistance;
		}
	}

	return bestPhase;
}

void EclipseEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	int lanternFrame = _atariLanternLightFrame;

	// Darkness radius based on battery level (5=full/bright → 0=dim → -1=dead).
	// ROM uses TeLanternBrightnessFrame ($7f6c) directly: frame 5 = largest radius,
	// frame 0 = smallest, frame -1 = no light.  ScummVM's getAtariLanternHoleRadius
	// maps frame 0→34px and frame 5→14px (inverted), so we pass (5 - battery).
	int lanternRadius = 0;
	if (_flashlightOn && _lanternBatteryLevel >= 0) {
		int effectiveFrame = 5 - _lanternBatteryLevel;
		lanternRadius = getAtariLanternHoleRadius(effectiveFrame);
	}

	if (_atariAreaDark)
		drawAtariDarknessMask(surface, _viewArea, _crossairPosition.x, _crossairPosition.y,
			lanternRadius, _gfx->_texturePixelFormat);

	// Border palette colors for the 4-plane font (from CONSOLE.NEO).
	// The Atari ST uses raster interrupts to switch palettes between
	// the 3D viewport (area palette) and the border/UI area (border palette).
	uint32 pal[5];
	pal[0] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	pal[1] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 145, 72, 0);
	pal[2] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 182, 109, 36);
	pal[3] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 218, 145, 36);
	pal[4] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 255, 182, 36);
	_font.setBackground(pal[0]);
	_font.setSecondaryColor(pal[2]);
	_font.setTertiaryColor(pal[3]);
	_font.setQuaternaryColor(pal[4]);

	// Score: Font B at x=$8B(139), y=$06(6) — from $11A66/$11A6E
	int score = _gameStateVars[k8bitVariableScore];
	drawScoreString(score, 139, 6, pal[1], pal[0], surface);

	// Room name / messages: $CE2 at x=$55(85), y=$77(119) — from $11BDC-$11C02
	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 85, 119, pal[1], pal[2], pal[0], surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 85, 119, pal[1], pal[2], pal[0], surface);

	// Atari ST water indicator from TeDrawWaterIndicator at $1EF0:
	// a fixed 32x31 body bitmap at $2003C and a highlighted top row at
	// $2024C/$2025C.
	int waterLevel = _gameStateVars[k8bitVariableEnergy];
	if (waterLevel < 0)
		waterLevel = 0;
	if (waterLevel > kAtariWaterIndicatorMaxLevel)
		waterLevel = kAtariWaterIndicatorMaxLevel;

	if (_atariWaterBody) {
		surface->copyRectToSurface(*_atariWaterBody, kAtariWaterIndicatorX, kAtariWaterIndicatorY,
			Common::Rect(_atariWaterBody->w, _atariWaterBody->h));
	}

	if (waterLevel > 0) {
		int topY = 183 - waterLevel;
		drawAtariWaterSurfaceRow(surface, kAtariWaterIndicatorX, topY,
			_atariWaterSurfaceMask, _atariWaterSurfacePixels, _gfx->_texturePixelFormat);
	}

	// Step indicator: $CDC at x=$4C(76), y=$77(119)
	// d7 = $42 + (2 - _playerStepIndex) * 2, drawChar chr = d7 + 31
	{
		int d7 = 0x42 + (2 - _playerStepIndex) * 2;
		int chr = d7 + 31;
		_font.drawChar(surface, chr, 76, 119, pal[1]);
	}

	// Height indicator: $CDC at x=$E0(224), y=$77(119)
	// d7 = $48 + (2 - _playerHeightNumber) * 2, drawChar chr = d7 + 31
	{
		int d7 = 0x48 + (2 - _playerHeightNumber) * 2;
		int chr = d7 + 31;
		_font.drawChar(surface, chr, 224, 119, pal[1]);
	}

	// Rotation/shooting indicator: $CDC at x=$E9(233), y=$77(119)
	// d7 = $4E normally, $50 when shooting
	if (_shootingFrames > 0) {
		int chr = 0x50 + 31;
		_font.drawChar(surface, chr, 233, 119, pal[1]);
	} else {
		int chr = 0x4E + 31;
		_font.drawChar(surface, chr, 233, 119, pal[1]);
	}

	// Heart indicator: sprite blit at x=$A0(160), y=$86(134)
	// Original heartbeat at $1E28-$1EAC: "lub-dub-pause" pattern.
	// Counter $125A loaded from shield ($7F68). Two quick beats (3 VBLs each)
	// then a long pause of shield-value VBLs. Lower shield = faster heartbeat.
	// Phase counter $126C tracks beat position (0->1->2->0): beats at 0,1 show
	// frame 0 (heart visible), phase 2 shows frame 1 (dimmed) during the pause.
	if (_eclipseSprites.size() >= 2) {
		int shield = _gameStateVars[k8bitVariableShield];
		int beatCycle = MAX(shield, 1) + 6;
		int phase = _ticks % beatCycle;
		int frame = (phase < 6) ? 0 : 1;
		surface->copyRectToSurface(*_eclipseSprites[frame], 160, 134,
			Common::Rect(_eclipseSprites[frame]->w, _eclipseSprites[frame]->h));
	}

	// Eclipse progress indicator: sprite blit at x=$80(128), y=$84(132) — from $11CA0-$11CE0
	// 16 frames showing sun being progressively eclipsed.
	// Frame = countdown-based: frame 0 = full sun, frame 15 = nearly fully eclipsed.
	if (_eclipseProgressSprites.size() >= 16) {
		// Frame 0 = fully eclipsed, frame 15 = full sun
		// progress: 1.0 at game start (full time left), 0.0 when time's up
		int frame = 0;
		if (_initialCountdown > 0 && _countdown > 0) {
			float progress = float(_countdown) / float(_initialCountdown);
			frame = (int)(15.0f * progress);
		}
		frame = CLIP(frame, 0, 15);
		surface->copyRectToSurface(*_eclipseProgressSprites[frame], 128, 132,
			Common::Rect(_eclipseProgressSprites[frame]->w, _eclipseProgressSprites[frame]->h));
	}

	// Ankh indicators at y=$B6(182), x = i*16 + 3 — from $11D88
	// Draw collected ankhs with transparency (skip black/color-0 pixels)
	if (_ankhSprites.size() >= 5) {
		uint32 transparentColor = pal[0]; // black
		Graphics::ManagedSurface *ankh = _ankhSprites[3]; // frame 3 = fully visible
		for (int i = 0; i < _gameStateVars[kVariableEclipseAnkhs] && i < 5; i++) {
			int destX = 3 + 16 * i;
			int destY = 182;
			for (int y = 0; y < ankh->h; y++) {
				for (int x = 0; x < ankh->w; x++) {
					uint32 pixel = ankh->getPixel(x, y);
					if (pixel != transparentColor)
						surface->setPixel(destX + x, destY + y, pixel);
				}
			}
		}
	}

	// Compass at x=$B0(176), y=$97(151): restore the ST background, then overlay
	// the animated needle.
	if (_compassBackground) {
		surface->copyRectToSurface(*_compassBackground, kAtariCompassX, kAtariCompassY,
			Common::Rect(_compassBackground->w, _compassBackground->h));
	}

	if (_compassSprites.size() >= kAtariCompassTotalFrames) {
		uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);
		if (!_atariCompassPhaseInitialized) {
			int targetPhase = atariCompassTargetPhaseFromYaw(_yaw, 0);
			_atariCompassPhase = targetPhase;
			_atariCompassTargetPhase = targetPhase;
			_atariCompassLastUpdateTick = _ticks;
			_atariCompassPhaseInitialized = true;
		}

		if (_atariCompassLastUpdateTick != _ticks) {
			_atariCompassPhase = wrapAtariCompassPhase(_atariCompassPhase +
				stepAtariCompassPhaseTowardTarget(_atariCompassPhase, _atariCompassTargetPhase));
			_atariCompassLastUpdateTick = _ticks;
		}

		int wrappedPhase = wrapAtariCompassPhase(_atariCompassPhase);
		int needleFrame = _compassLookup[wrappedPhase];
		if (needleFrame >= 0 && needleFrame < (int)_compassSprites.size()) {
			drawAtariCompassNeedle(surface, _compassSprites[needleFrame], kAtariCompassX, kAtariCompassY,
				atariCompassPhaseUsesVerticalFlip(wrappedPhase), transparent);
		}
	}

	// Lantern switch at x=$30(48), y=$91(145). The ST code always draws the
	// switch and picks frame 0/1 from the persistent lantern state.
	if (_lanternSwitchSprites.size() >= 2) {
		int switchFrame = _flashlightOn ? 0 : 1;
		surface->copyRectToSurface(*_lanternSwitchSprites[switchFrame], 48, 145,
			Common::Rect(_lanternSwitchSprites[switchFrame]->w, _lanternSwitchSprites[switchFrame]->h));
	}

	// Lantern light strip at (48, 139). During on/off animation use the animation
	// frame; once settled, show the battery level. The original 68K code at
	// $1C1C uses eclipse_brightness_level ($7F6C) directly as the sprite index:
	// level 5 = sprite 5 (brightest), level 0 = sprite 0 (dimmest).
	{
		int hudLanternFrame = lanternFrame;
		if (_flashlightOn && _atariLanternAnimationDirection == 0 && _lanternBatteryLevel >= 0)
			hudLanternFrame = _lanternBatteryLevel;
		if (hudLanternFrame >= 0 && hudLanternFrame < 6 && _lanternLightSprites.size() >= 6) {
			surface->copyRectToSurface(*_lanternLightSprites[hudLanternFrame], 48, 139,
				Common::Rect(_lanternLightSprites[hudLanternFrame]->w, _lanternLightSprites[hudLanternFrame]->h));
		}
	}

	// Shooting crosshair overlay at x=$80(128), y=$9F(159)
	if (_shootingFrames > 0 && _shootSprites.size() >= 2) {
		int shootFrame = (_shootingFrames > 5) ? 1 : 0;
		surface->copyRectToSurface(*_shootSprites[shootFrame], 128, 159,
			Common::Rect(_shootSprites[shootFrame]->w, _shootSprites[shootFrame]->h));
	}

	// The Atari border art uses a different clock pivot than the CPC-style UI.
	// These coordinates are the center of the bezel drawn in CONSOLE.NEO.
	uint8 r, g, b;
	uint32 color = _currentArea->_underFireBackgroundColor;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color))
		color = (*_gfx->_colorRemaps)[color];
	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_inkColor;
	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	drawAnalogClock(surface, kAtariClockCenterX, kAtariClockCenterY, back, other, front);
	advanceAtariLanternAnimation(this);
}

void EclipseEngine::loadAssetsAtariFullGame() {
	Common::File file;
	file.open("0.tec");
	Common::SeekableReadStream *stream = nullptr;
	if (!file.isOpen()) {
		stream = decryptFileAtariVirtualWorlds("1.tec");
	} else {
		_title = loadAndConvertNeoImage(&file, 0x17ac);
		file.close();

		stream = decryptFileAmigaAtari("1.tec", "0.tec", 0x1774 - 4 * 1024);
	}
	parseAmigaAtariHeader(stream);

	loadMessagesVariableSize(stream, 0x87a6, 28);
	load8bitBinary(stream, 0x2a53c, 16);

	_border = loadAndConvertNeoImage(stream, 0x139c8);
	// The palette table is split across two regions in the binary: areas 32-51
	// at prog $29E36, then areas 25-31, 127, 1-24 at prog $2A0DE. Load from
	// the start of the first region so all areas get palettes.
	loadPalettes(stream, 0x29e52);

	// The original game uses a Timer-B raster interrupt to split the hardware
	// palette mid-screen: colors 0-5 always come from the border (CONSOLE.NEO)
	// palette, while only colors 6-15 are swapped per area.
	for (auto &entry : _paletteByArea) {
		byte *pal = entry._value;
		memcpy(pal, kBorderPalette, 6 * 3);
	}

	loadSoundsFx(stream, 0x3030c, 6);

	// Load TEMUSIC.ST (GEMDOS executable at file offset $11F5A, skip $1C header, TEXT size $11E8)
	static const uint32 kTEMusicOffset = 0x11F5A;
	static const uint32 kGemdosHeaderSize = 0x1C;
	static const uint32 kTEMusicTextSize = 0x11E8;
	stream->seek(kTEMusicOffset + kGemdosHeaderSize);
	_musicData.resize(kTEMusicTextSize);
	stream->read(_musicData.data(), kTEMusicTextSize);
	_playerMusic = makeEclipseAtariMusicPlayer(_musicData.data(), _musicData.size());
	debug(3, "TE-Atari: Loaded TEMUSIC.ST TEXT segment (%d bytes)", kTEMusicTextSize);

	// UI font (Font A): 4-plane 16-color bordered font at prog $24C3E (file offset $24C5A)
	// 85 characters: ASCII text (32-116) plus special indicator glyphs (65-84)
	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getChars4Plane(stream, 0x24C5A, 85);
	_font = Font(chars);

	// Score font (Font B): 4-plane 10-glyph font at prog $249BE (file offset $249DA)
	// Dedicated score digits 0-9 with different bordered style
	Common::Array<Graphics::ManagedSurface *> scoreChars;
	scoreChars = getChars4Plane(stream, 0x249DA, 11);
	_fontScore = Font(scoreChars);

	// All sprite addresses below are program addresses from the 68K disassembly.
	// The decrypted stream includes a $1C-byte GEMDOS header, so add $1C to
	// convert program addresses to stream offsets.
	static const int kHdr = 0x1C;

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

	// Eclipse/dark areas use a fade palette table at prog $10EB6 instead of
	// the per-area palette table. 6 brightness levels (0=black, 5=brightest),
	// 16 words each. The 68K code at $10FDC applies LSR.L #1 before writing
	// to the raster interrupt buffer, but ScummVM reads normal area palettes
	// from the same table format without that shift (and they look correct),
	// so we load these the same way — via loadPalette, no shift.
	for (int level = 0; level < 6; level++) {
		stream->seek(0x10EB6 + 0x1C + level * 32);
		byte *pal = loadPalette(stream);
		memcpy(_eclipseFadePalettes[level], pal, 16 * 3);
		delete[] pal;
	}

	// Apply brightest fade palette to all dark areas at load time
	for (uint i = 0; i < _atariDarkAreas.size(); i++)
		applyEclipseFadePalette(_atariDarkAreas[i], _lanternBatteryLevel);

	// Heart indicator sprites: 2 frames, 16x13 pixels
	// Descriptor at prog $1D2B8 (1 col × 13 rows), mask at +6, pixels at +8
	// Frame 0 = heart visible, Frame 1 = heart hidden/dimmed
	_eclipseSprites.resize(2);
	_eclipseSprites[0] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + kHdr, 1, 13);
	_eclipseSprites[1] = loadAtariSTSprite(stream, 0x1D2BE + kHdr, 0x1D2C0 + 104 + kHdr, 1, 13);

	// Eclipse progress indicator: 16 frames, 16x16 pixels
	// Descriptor at prog $1DA90 (1 col × 16 rows), mask at +6, pixels at +8
	// Frame 0 = full sun, frame 15 = nearly fully eclipsed. Each frame = 128 bytes.
	_eclipseProgressSprites.resize(16);
	for (int i = 0; i < 16; i++)
		_eclipseProgressSprites[i] = loadAtariSTSprite(stream, 0x1DA96 + kHdr, 0x1DA98 + kHdr + i * 128, 1, 16);

	// Ankh indicator: 5 fade-in frames at prog $1B734, 16x15 (1 col, stride 120 bytes)
	// Mask at prog $1B732. Frame 3 = fully visible ankh.
	_ankhSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_ankhSprites[i] = loadAtariSTSprite(stream, 0x1B732 + kHdr, 0x1B734 + kHdr + i * 120, 1, 15);
		_ankhSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Compass background at $20986 and needle bank at $20B36.
	_compassBackground = loadAtariSTRawSprite(stream, 0x20986 + kHdr, 2, 27);
	_compassBackground->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	// The ST init code expands frames 1-18 into the second 18-frame bank in RAM.
	{
		Common::copy(kAtariCompassPhaseToFrame, kAtariCompassPhaseToFrame + kAtariCompassPhaseCount, _compassLookup);

		_compassSprites.resize(kAtariCompassTotalFrames);
		for (int frame = 0; frame < kAtariCompassBaseFrames; frame++) {
			_compassSprites[frame] = loadAtariSTNeedleSprite(stream,
				0x20B36 + kHdr + frame * 432, _gfx->_texturePixelFormat);
		}
		buildAtariSTCompassMirrorCache(stream, 0x20B36 + kHdr, _compassSprites, _gfx->_texturePixelFormat);
	}

	// Lantern light animation: 6 frames, 32x6, at prog $2026A, stride 96 bytes
	_lanternLightSprites.resize(6);
	for (int i = 0; i < 6; i++) {
		_lanternLightSprites[i] = loadAtariSTRawSprite(stream, 0x2026A + kHdr + i * 96, 2, 6);
		_lanternLightSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Lantern switch: 2 frames, 32x23, at prog $204B4, stride $170 bytes
	// Frame 0 = on, frame 1 = off (toggled with 'T' key)
	_lanternSwitchSprites.resize(2);
	_lanternSwitchSprites[0] = loadAtariSTRawSprite(stream, 0x204B4 + kHdr, 2, 23);
	_lanternSwitchSprites[1] = loadAtariSTRawSprite(stream, 0x204B4 + 0x170 + kHdr, 2, 23);
	for (auto &sprite : _lanternSwitchSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);

	// Sound ON/OFF toggle: 5 frames, 16x11, at prog $20794, stride 96 bytes
	// Drawn at (32, 138) when sound is toggled
	_soundToggleSprites.resize(5);
	for (int i = 0; i < 5; i++) {
		_soundToggleSprites[i] = loadAtariSTRawSprite(stream, 0x20794 + kHdr + i * 96, 1, 11);
		_soundToggleSprites[i]->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);
	}

	// Water indicator data from TeDrawWaterIndicator at $1EF0:
	// a 32x31 body bitmap at $2003C and a highlighted 32-pixel cap row at
	// $2024C/$2025C.
	_atariWaterBody = loadAtariSTRawSprite(stream, 0x2003C + kHdr, 2, 31);
	_atariWaterBody->convertToInPlace(_gfx->_texturePixelFormat,
		const_cast<byte *>(kBorderPalette), 16);

	_atariWaterSurfacePixels.resize(8);
	stream->seek(0x2024C + kHdr);
	for (uint i = 0; i < _atariWaterSurfacePixels.size(); i++)
		_atariWaterSurfacePixels[i] = stream->readUint16BE();

	_atariWaterSurfaceMask.resize(2);
	stream->seek(0x2025C + kHdr);
	for (uint i = 0; i < _atariWaterSurfaceMask.size(); i++)
		_atariWaterSurfaceMask[i] = stream->readUint16BE();

	// Shooting crosshair sprites: 2 frames with mask, at prog $1CC26 and $1CDC0
	// Frame 0: 32x25 (2 cols), frame 1: 48x25 (3 cols)
	_shootSprites.resize(2);
	_shootSprites[0] = loadAtariSTSprite(stream, 0x1CC2C + kHdr, 0x1CC30 + kHdr, 2, 25);
	_shootSprites[1] = loadAtariSTSprite(stream, 0x1CDC6 + kHdr, 0x1CDCC + kHdr, 3, 25);
	for (auto &sprite : _shootSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat,
			const_cast<byte *>(kBorderPalette), 16);

	// Convert heart and eclipse progress sprites from CLUT8 to target format using border palette
	for (auto &sprite : _eclipseSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat, const_cast<byte *>(kBorderPalette), 16);
	for (auto &sprite : _eclipseProgressSprites)
		sprite->convertToInPlace(_gfx->_texturePixelFormat, const_cast<byte *>(kBorderPalette), 16);

	_fontLoaded = true;
}

} // End of namespace Freescape
