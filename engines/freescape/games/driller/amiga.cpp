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
#include "common/random.h"

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DrillerEngine::loadRigSprites(Common::SeekableReadStream *file, int sprigsOffset, byte *paletteOverride) {
	// SPRIGS: 2 word columns × 25 rows × 5 frames, stride=$1A0 (416 bytes)
	const int frameStride = 0x1A0;
	const int numFrames = 5;
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);

	// Get the console palette
	byte *palette = paletteOverride;
	bool ownsPalette = false;
	if (!palette) {
		if (_variant & GF_AMIGA_RETAIL)
			palette = getPaletteFromNeoImage(file, 0x137f4);
		else {
			Common::File neoFile;
			neoFile.open("console.neo");
			if (neoFile.isOpen())
				palette = getPaletteFromNeoImage(&neoFile, 0);
		}
		ownsPalette = true;
	}
	if (!palette)
		return;

	for (int f = 0; f < numFrames; f++) {
		auto *surf = new Graphics::ManagedSurface();
		surf->create(32, 25, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 32, 25), transparent);
		decodeAmigaSprite(file, surf, sprigsOffset + (f + 1) * frameStride, 2, 25, palette);
		_rigSprites.push_back(surf);
	}

	if (ownsPalette)
		free(palette);
}

void DrillerEngine::loadIndicatorSprites(Common::SeekableReadStream *file, byte *palette,
		int stepOffset, int angleOffset, int vehicleOffset, int quitOffset) {
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);

	// Step indicator: 1 word × 4 rows, 8 frames, stride=40
	if (stepOffset >= 0) {
		for (int f = 0; f < 8; f++) {
			auto *surf = new Graphics::ManagedSurface();
			surf->create(16, 4, _gfx->_texturePixelFormat);
			surf->fillRect(Common::Rect(0, 0, 16, 4), transparent);
			decodeAmigaSprite(file, surf, stepOffset + f * 40, 1, 4, palette);
			_stepSprites.push_back(surf);
		}
	}

	// Angle indicator: 1 word × 4 rows, 8 frames, stride=40
	if (angleOffset >= 0) {
		for (int f = 0; f < 8; f++) {
			auto *surf = new Graphics::ManagedSurface();
			surf->create(16, 4, _gfx->_texturePixelFormat);
			surf->fillRect(Common::Rect(0, 0, 16, 4), transparent);
			decodeAmigaSprite(file, surf, angleOffset + f * 40, 1, 4, palette);
			_angleSprites.push_back(surf);
		}
	}

	// Vehicle indicator: 4 words × 43 rows, 5 frames, stride=1408 ($580)
	// Frame 0=fly, frames 1-4=tank heights 0-3
	if (vehicleOffset >= 0) {
		uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
		for (int f = 0; f < 5; f++) {
			auto *surf = new Graphics::ManagedSurface();
			surf->create(64, 43, _gfx->_texturePixelFormat);
			surf->fillRect(Common::Rect(0, 0, 64, 43), black);
			decodeAmigaSprite(file, surf, vehicleOffset + f * 0x580, 4, 43, palette);
			_vehicleSprites.push_back(surf);
		}
	}

	// Quit/abort indicator: 2 words × 8 rows, 11 frames, stride=$90=144
	// Frames 0-6: shutter animation, 7-10: confirmation squares filling in
	if (quitOffset >= 0) {
		uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
		for (int f = 0; f < 11; f++) {
			auto *surf = new Graphics::ManagedSurface();
			surf->create(32, 8, _gfx->_texturePixelFormat);
			surf->fillRect(Common::Rect(0, 0, 32, 8), black);
			decodeAmigaSprite(file, surf, quitOffset + f * 0x90, 2, 8, palette);
			_quitSprites.push_back(surf);
		}
	}
}

void DrillerEngine::loadEarthquakeSprites(Common::SeekableReadStream *file, byte *palette, int earthquakeOffset) {
	// Seismograph monitor: 2 word columns (32px) × 11 rows, decoded from overlapping
	// frames in a continuous sprite buffer. SPREQW={1,10,$200} means 2 columns, 11 rows
	// (dbra counts). SPREQM={$8000,$07FF} masks out pixel 0 and pixels 21-31, leaving
	// a visible area of 20×11 pixels. The original picks random byte offsets in steps
	// of 32 from two ranges:
	//   Sound ON:  2048..2528 (step 32) → 16 frames of dense seismic activity
	//   Sound OFF: 0..480 (step 32) → 16 frames of sparse activity
	// We precompute all 32 frames, storing only the 20×11 visible region.
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	static const int offsets[] = {
		0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480,
		2048, 2080, 2112, 2144, 2176, 2208, 2240, 2272, 2304, 2336, 2368, 2400, 2432, 2464, 2496, 2528
	};

	for (int i = 0; i < 32; i++) {
		// Decode the full 32×11 sprite, then extract the 20-pixel visible region
		Graphics::ManagedSurface full;
		full.create(32, 11, _gfx->_texturePixelFormat);
		full.fillRect(Common::Rect(0, 0, 32, 11), black);
		decodeAmigaSprite(file, &full, earthquakeOffset + offsets[i], 2, 11, palette);

		auto *surf = new Graphics::ManagedSurface();
		surf->create(20, 11, _gfx->_texturePixelFormat);
		surf->fillRect(Common::Rect(0, 0, 20, 11), black);
		surf->copyRectToSurface(full, 0, 0, Common::Rect(1, 0, 21, 11));
		_earthquakeSprites.push_back(surf);
	}
}

void DrillerEngine::loadCompassStrips(Common::SeekableReadStream *file, byte *palette,
		int pitchStripOffset, int yawCogOffset) {
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);

	// Pitch strip (SPRATT): 32px wide, 144+29=173 rows of continuous data.
	// stride=16 bytes per row, 2 word columns × 4 planes = 16 bytes/row.
	// 144 start positions, 29 visible rows at a time → need 173 total rows.
	{
		int totalRows = 144 + 29;
		_compassPitchStrip = new Graphics::ManagedSurface();
		_compassPitchStrip->create(32, totalRows, _gfx->_texturePixelFormat);
		_compassPitchStrip->fillRect(Common::Rect(0, 0, 32, totalRows), black);
		decodeAmigaSprite(file, _compassPitchStrip, pitchStripOffset, 2, totalRows, palette);
	}

	// Yaw compass (SPRCOG): pre-render all 72 rotation frames.
	// The original uses 70 bytes of pre-computed needle data, accessed at different
	// byte offsets and bit-shifted to produce 72 unique 30×5 pixel frames.
	// Each frame: read long(4)+word(2) per row, shift left, mask with $3FFFFE00.
	// The needle is written to bitplane 1 only (green in Amiga palette).
	{
		byte cogData[70];
		file->seek(yawCogOffset);
		file->read(cogData, 70);

		uint32 needleColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
			palette[2 * 3], palette[2 * 3 + 1], palette[2 * 3 + 2]); // bitplane 1 = color 2

		uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0, 0, 0);
		for (int rot = 0; rot < 72; rot++) {
			auto *surf = new Graphics::ManagedSurface();
			surf->create(30, 5, _gfx->_texturePixelFormat);
			surf->fillRect(Common::Rect(0, 0, 30, 5), transparent);

			int wordOff = (rot >> 3) & ~1;
			int bitShift = rot & 15;
			int a1 = wordOff;

			for (int row = 0; row < 5; row++) {
				uint32 longVal = ((uint32)cogData[a1] << 24) | ((uint32)cogData[a1+1] << 16) |
				                 ((uint32)cogData[a1+2] << 8) | cogData[a1+3];
				uint16 wordVal = ((uint16)cogData[a1+4] << 8) | cogData[a1+5];

				longVal = (longVal << bitShift);
				uint32 wordExt = ((uint32)wordVal << bitShift) >> 16;
				uint32 result = (longVal | wordExt) & 0x3FFFFE00;

				for (int b = 0; b < 30; b++) {
					if (result & (0x40000000 >> b))
						surf->setPixel(b, row, needleColor);
				}

				a1 += 14; // 6 bytes data + 8 bytes skip per row
			}

			_compassYawFrames.push_back(surf);
		}
	}
}

void DrillerEngine::loadAssetsAmigaFullGame() {
	Common::File file;
	if (_variant & GF_AMIGA_RETAIL) {
		file.open("driller");

		if (!file.isOpen())
			error("Failed to open 'driller' executable for Amiga");

		_border = loadAndConvertNeoImage(&file, 0x137f4);
		_title = loadAndConvertNeoImage(&file, 0xce);

		loadFonts(&file, 0x8940);
		Common::Array<Graphics::ManagedSurface *> chars;
		chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, &file, 0x8940 + 112 * 33 + 1, 100);
		_fontSmall = Font(chars);
		_fontSmall.setCharWidth(5);

		loadMessagesFixedSize(&file, 0xc66e, 14, 20);
		loadGlobalObjects(&file, 0xbd62, 8);
		load8bitBinary(&file, 0x29c16, 16);
		loadPalettes(&file, 0x297d4);
		loadSoundsFx(&file, 0x30e80, 25);

		byte *palette = getPaletteFromNeoImage(&file, 0x137f4);
		loadRigSprites(&file, 0x2407A);
		loadIndicatorSprites(&file, palette, 0x26F9A, 0x27222, 0x24D88, 0x26912);
		loadCompassStrips(&file, palette, 0x23316, 0x26F4C);
		loadEarthquakeSprites(&file, palette, 0x27560);
		free(palette);
	} else if (_variant & GF_AMIGA_BUDGET) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("driller");
		if (!file.isOpen())
			error("Failed to open 'driller' executable for Amiga");

		loadFonts(&file, 0xa62);
		Common::Array<Graphics::ManagedSurface *> chars;
		chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, &file, 0xa62 + 112 * 33 + 1, 100);
		_fontSmall = Font(chars);
		_fontSmall.setCharWidth(5);

		loadMessagesFixedSize(&file, 0x499a, 14, 20);
		loadGlobalObjects(&file, 0x4098, 8);
		load8bitBinary(&file, 0x21a3e, 16);
		loadPalettes(&file, 0x215fc);

		byte *palette = nullptr;
		Common::File neoFile;
		neoFile.open("console.neo");
		if (neoFile.isOpen())
			palette = getPaletteFromNeoImage(&neoFile, 0);
		loadRigSprites(&file, 0x1B8C8);
		if (palette) {
			loadIndicatorSprites(&file, palette, 0x1E288, 0x1E510, 0x1C5D6, 0x1DC00);
			loadCompassStrips(&file, palette, 0x1AB64, 0x1E23A);
			loadEarthquakeSprites(&file, palette, 0x1E84E);
		}
		free(palette);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for Amiga");

		loadSoundsFx(&file, 0, 25);
	} else
		error("Invalid or unknown Amiga release");


	for (auto &area : _areaMap) {
			// Center and pad each area name so we do not have to do it at each frame
			area._value->_name = centerAndPadString(area._value->_name, 14);
	}

	_indicators.push_back(loadBundledImage("driller_tank_indicator_0"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_1"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_2"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_3"));
	_indicators.push_back(loadBundledImage("driller_ship_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

void DrillerEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("lift.neo");
	if (!file.isOpen())
		error("Failed to open 'lift.neo' file");

	_title = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("console.neo");
	if (!file.isOpen())
		error("Failed to open 'console.neo' file");

	_border = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("demo.cmd");
	if (!file.isOpen())
		error("Failed to open 'demo.cmd' file");

	loadDemoData(&file, 0, 0x1000);

	file.close();
	file.open("driller");
	if (!file.isOpen())
		error("Failed to open 'driller' file");

	if (_variant & GF_AMIGA_MAGAZINE_DEMO) {
		loadMessagesFixedSize(&file, 0x3df0, 14, 20);
		loadGlobalObjects(&file, 0x3ba6, 8);
		_demoMode = false;

		loadFonts(&file, 0xa62);
		Common::Array<Graphics::ManagedSurface *> chars;
		chars = getCharsAmigaAtariInternal(8, 8, -3, 33, 32, &file, 0xa62 + 112 * 33 + 1, 100);
		_fontSmall = Font(chars);
		_fontSmall.setCharWidth(5);
	} else {
		loadFonts(&file, 0xa30);
		loadMessagesFixedSize(&file, 0x3960, 14, 20);
		loadGlobalObjects(&file, 0x3716, 8);

		byte *palette = nullptr;
		Common::File neoFile;
		neoFile.open("console.neo");
		if (neoFile.isOpen())
			palette = getPaletteFromNeoImage(&neoFile, 0);

		loadRigSprites(&file, 0x1A960);
		if (palette) {
			// The rolling demo matches the retail executable for these indicator blocks,
			// but its vehicle sprite set differs, so keep the bundled fallback for that one.
			loadIndicatorSprites(&file, palette, 0x1D320, 0x1D5A8, -1, 0x1CC98);
			loadCompassStrips(&file, palette, 0x19BFC, 0x1D2D2);
			loadEarthquakeSprites(&file, palette, 0x1D8E6);
		}
		free(palette);
	}

	file.close();
	file.open("data");
	if (!file.isOpen())
		error("Failed to open 'data' file");

	load8bitBinary(&file, 0x442, 16);
	loadPalettes(&file, 0x0);

	file.close();
	file.open("soundfx");
	if (!file.isOpen())
		error("Failed to open 'soundfx' executable for Amiga");

	loadSoundsFx(&file, 0, 25);

	_indicators.push_back(loadBundledImage("driller_tank_indicator_0"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_1"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_2"));
	_indicators.push_back(loadBundledImage("driller_tank_indicator_3"));
	_indicators.push_back(loadBundledImage("driller_ship_indicator"));

	for (auto &it : _indicators)
		it->convertToInPlace(_gfx->_texturePixelFormat);
}

/*
The following function contains specific UI code for both Amiga and AtariST
*/

void DrillerEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 brownish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x9E, 0x80, 0x20);
	uint32 brown = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x7E, 0x60, 0x19);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xE0, 0x00, 0x00);
	uint32 redish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xE0, 0x60, 0x20);
	uint32 primaryFontColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA0, 0x80, 0x00);
	uint32 secondaryFontColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x40, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	Common::String coords;

	// It seems that some demos will not include the complete font
	if (_currentArea->getAreaID() != _endArea && (!isDemo() || (_variant & GF_AMIGA_MAGAZINE_DEMO) || (_variant & GF_ATARI_MAGAZINE_DEMO))) {

		drawString(kDrillerFontSmall, ":", 38, 18, white, white, transparent, surface); // ":" is the next character to "9" representing "x"
		coords = Common::String::format("%04d", 2 * int(_position.x()));
		drawString(kDrillerFontSmall, coords, 47, 18, white, transparent, transparent, surface);

		drawString(kDrillerFontSmall, ";", 37, 26, white, white, transparent, surface); // ";" is the next character to ":" representing "y"
		coords = Common::String::format("%04d", 2 * int(_position.z())); // Coords y and z are swapped!
		drawString(kDrillerFontSmall, coords, 47, 26, white, transparent, transparent, surface);

		drawString(kDrillerFontSmall, "<", 37, 34, white, white, transparent, surface); // "<" is the next character to ";" representing "z"
		coords = Common::String::format("%04d", 2 * int(_position.y())); // Coords y and z are swapped!
		drawString(kDrillerFontSmall, coords, 47, 34, white, transparent, transparent, surface);
	}

	drawStringInSurface(_currentArea->_name, 189, 185, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 241, 129, primaryFontColor, secondaryFontColor, black, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d:", hours), 210, 7, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%02d:", minutes), 230, 7, primaryFontColor, secondaryFontColor, black, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 7, primaryFontColor, secondaryFontColor, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 188, 177, yellow, secondaryFontColor, black, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateVars[32] == 18)
			message = _messagesList[19];
		else if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 188, 177, primaryFontColor, secondaryFontColor, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(11, 178, 74 - (_maxShield - shield), 184);
		surface->fillRect(shieldBar, brown);

		if (shield > 11)
			shieldBar = Common::Rect(11, 178, 25, 184);
		else
			shieldBar = Common::Rect(11, 178, 74 - (_maxShield - shield), 184);
		surface->fillRect(shieldBar, red);

		shieldBar = Common::Rect(11, 179, 74 - (_maxShield - shield), 183);
		surface->fillRect(shieldBar, brownish);

		if (shield > 11)
			shieldBar = Common::Rect(11, 179, 25, 183);
		else
			shieldBar = Common::Rect(11, 179, 74 - (_maxShield - shield), 183);
		surface->fillRect(shieldBar, redish);

		shieldBar = Common::Rect(11, 180, 74 - (_maxShield - shield), 182);
		surface->fillRect(shieldBar, yellow);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(11, 186, 74 - (_maxEnergy - energy), 192);
		surface->fillRect(energyBar, brown);

		if (energy > 11)
			energyBar = Common::Rect(11, 186, 24, 192);
		else
			energyBar = Common::Rect(11, 186, 74 - (_maxEnergy - energy), 192);
		surface->fillRect(energyBar, red);

		energyBar = Common::Rect(11, 187, 74 - (_maxEnergy - energy), 191);
		surface->fillRect(energyBar, brownish);

		if (energy > 11)
			energyBar = Common::Rect(11, 187, 24, 191);
		else
			energyBar = Common::Rect(11, 187, 74 - (_maxEnergy - energy), 191);
		surface->fillRect(energyBar, redish);

		energyBar = Common::Rect(11, 188, 74 - (_maxEnergy - energy), 190);
		surface->fillRect(energyBar, yellow);
	}

	if (!_vehicleSprites.empty()) {
		int frame = _flyMode ? 0 : (_playerHeightNumber + 1);
		frame = CLIP(frame, 0, (int)_vehicleSprites.size() - 1);
		// Mask $FF00,$0000,$0000,$0007: 8 bits transparent left, 3 bits transparent right
		// Visible pixels: x=8 to x=60 within the 64px sprite
		surface->copyRectToSurface(*_vehicleSprites[frame], 104, 126,
			Common::Rect(8, 0, 61, _vehicleSprites[frame]->h));
	} else if (_indicators.size() > 0) {
		// Fallback to bundled images
		if (_flyMode)
			surface->copyRectToSurface(*_indicators[4], 106, 128, Common::Rect(_indicators[1]->w, _indicators[1]->h));
		else
			surface->copyRectToSurface(*_indicators[_playerHeightNumber], 106, 128, Common::Rect(_indicators[1]->w, _indicators[1]->h));
	}

	// Step indicator: shows current step size (0-7)
	if (!_stepSprites.empty()) {
		int frame = _playerStepIndex % _stepSprites.size();
		surface->copyRectToSurfaceWithKey(*_stepSprites[frame], 48, 160,
			Common::Rect(_stepSprites[frame]->w, _stepSprites[frame]->h), transparent);
	}

	// Angle/compass indicator: shows current rotation angle setting (0-7)
	if (!_angleSprites.empty()) {
		int frame = _angleRotationIndex % _angleSprites.size();
		surface->copyRectToSurfaceWithKey(*_angleSprites[frame], 64, 160,
			Common::Rect(_angleSprites[frame]->w, _angleSprites[frame]->h), transparent);
	}

	// Pitch compass (SPRATT): vertically scrolling strip at x=$4E=78, y=$89=137
	// Mask $FFFC,$0078 means only 14 pixels visible: 2 from column 0 right + 12 from column 1
	// Visible pixel range: x=14 to x=27 within the 32px strip (bits 0-1 of col0 + bits 0-2,7-15 of col1)
	if (_compassPitchStrip) {
		int pos = ((int)(_pitch * 0.4f) + 144) % 144;
		Common::Rect srcRect(14, pos, 28, pos + 29);
		surface->copyRectToSurface(*_compassPitchStrip, 78, 138, srcRect);
	}

	// Yaw compass: purple gradient background (SPRCBG) drawn first,
	// then scrolling N/E/S/W needle (SPRCOG) drawn on top one line below.
	// Background at x=$32→48, y=$8E=142. Needle at y=$8E+1=143.
	if (!_compassYawFrames.empty()) {
		float yaw = _yaw;
		if (yaw < 0) yaw += 360;
		if (yaw >= 360) yaw -= 360;
		int rot = ((int)(yaw / 5.0f)) % 72;
		surface->copyRectToSurfaceWithKey(*_compassYawFrames[rot], 49, 143,
			Common::Rect(_compassYawFrames[rot]->w, _compassYawFrames[rot]->h), transparent);
	}

	// Seismograph monitor (SPREQL): animated noise at x=50, y=1 (20×11 visible pixels).
	// The original updates every 4th tick picking a random frame.
	// Sound-on frames (indices 16-31) show dense activity; sound-off (0-15) sparse.
	if (!_earthquakeSprites.empty()) {
		if ((_ticks & 3) == 0)
			_earthquakeLastFrame = 16 + _rnd->getRandomNumber(15);
		surface->copyRectToSurface(*_earthquakeSprites[_earthquakeLastFrame], 50, 1,
			Common::Rect(0, 0, 20, 11));
	}

	// Quit indicator (ABORTSQ): shows on the console when quit is initiated.
	// First click: shutter rolls down (frames 0-6), then shows 3 empty squares (frame 7).
	// Clicks 2-4: squares fill in (frames 8-10). Fourth click = quit confirmed.
	// Mask $0000,$0FFF: 20 visible pixels.
	// Quit sequence from assembly (ABORTSQ):
	// Click 1: shutter rolls down (frames 0-6), settles on frame 7 (3 empty lights)
	// Click 2: frame 8 (first light on)
	// Click 3: frame 9 (second light on)
	// Click 4: frame 10 (third light on, bar turns green) → next click quits
	if (!_quitSprites.empty() && _quitConfirmCounter > 0) {
		int frame;
		if (_quitConfirmCounter == 1) {
			// Shutter intro: animate frames 0-6, then hold frame 7
			int shutterFrame = (_ticks - _quitStartTicks) / 2;
			frame = (shutterFrame >= 7) ? 7 : shutterFrame;
		} else {
			// Counter 2→frame 8, 3→frame 9, 4→frame 10
			frame = 6 + _quitConfirmCounter;
		}
		frame = CLIP(frame, 0, (int)_quitSprites.size() - 1);
		surface->copyRectToSurface(*_quitSprites[frame], 176, 5,
			Common::Rect(0, 0, 20, _quitSprites[frame]->h));
	}

	// Drilling rig animation: cycles through 5 frames when rig is placed
	if (!_rigSprites.empty() && _drillStatusByArea[_currentArea->getAreaID()] == 1) {
		int frame = (_ticks / 7) % _rigSprites.size();
		surface->copyRectToSurfaceWithKey(*_rigSprites[frame], 272, 143,
			Common::Rect(_rigSprites[frame]->w, _rigSprites[frame]->h), transparent);
	}
}

void DrillerEngine::drawString(const DrillerFontSize size, const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;

	Font *font = nullptr;

	if (size == kDrillerFontNormal) {
		font = &_font;
	} else if (size == kDrillerFontSmall) {
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

void DrillerEngine::initAmigaAtari() {
	_viewArea = Common::Rect(36, 16, 284, 118);

	_moveFowardArea = Common::Rect(184, 125, 199, 144);
	_moveLeftArea = Common::Rect(161, 145, 174, 164);
	_moveRightArea = Common::Rect(207, 145, 222, 164);
	_moveBackArea = Common::Rect(184, 152, 199, 171);
	_moveUpArea = Common::Rect(231, 145, 246, 164);
	_moveDownArea = Common::Rect(254, 145, 269, 164);
	_deployDrillArea = Common::Rect(284, 145, 299, 166);
	_infoScreenArea = Common::Rect(125, 172, 152, 197);
	_saveGameArea = Common::Rect(9, 145, 39, 154);
	_loadGameArea = Common::Rect(9, 156, 39, 164);

	_borderExtra = nullptr;
	_compassPitchStrip = nullptr;
	_quitConfirmCounter = 0;
	_quitStartTicks = 0;
	_earthquakeLastFrame = 16;
	_quitArea = Common::Rect(188, 5, 208, 13);
	_borderExtraTexture = nullptr;

	_soundIndexShoot = 1;
	_soundIndexCollide = 19;
	_soundIndexStepDown = 19;
	_soundIndexStepUp = 19;	
	_soundIndexAreaChange = 5;
	_soundIndexHit = 2;
	_soundIndexFall = 25;
	_soundIndexFallen = 11;
	_soundIndexForceEndGame = 11;
	_soundIndexNoShield = 11;
	_soundIndexNoEnergy = 11;
	_soundIndexTimeout = 11;
	_soundIndexCrushed = 11;
}

} // End of namespace Freescape
