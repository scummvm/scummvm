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

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void CastleEngine::initZX() {
	_viewArea = Common::Rect(64, 36, 256, 148);
	_soundIndexShoot = 5;
	_soundIndexCollide = 3;
	_soundIndexStartFalling = -1;
	_soundIndexFallen = 1;
	_soundIndexFall = 6;
	_soundIndexStepUp = 12;
	_soundIndexStepDown = 12;
	_soundIndexMenu = 3;
	_soundIndexStart = 7;
	_soundIndexAreaChange = 7;
}

void CastleEngine::loadAssetsZXFullGame() {
	Common::File file;
	uint8 r, g, b;
	Common::Array<Graphics::ManagedSurface *> chars;

	Common::Path titleFile(isCastleMaster2() ? "castlemaster2.zx.title" : "castlemaster.zx.title");
	Common::Path borderFile(isCastleMaster2() ? "castlemaster2.zx.border" : "castlemaster.zx.border");
	Common::Path dataFile(isCastleMaster2() ? "castlemaster2.zx.data" : "castlemaster.zx.data");

	file.open(titleFile);
	if (file.isOpen()) {
		_title = loadAndConvertScrImage(&file);
	} else
		error("Unable to find %s", titleFile.toString().c_str());

	file.close();
	file.open(borderFile);
	if (file.isOpen()) {
		_border = loadAndConvertScrImage(&file);
	} else
		error("Unable to find %s", borderFile.toString().c_str());
	file.close();

	file.open(dataFile);
	if (!file.isOpen())
		error("Failed to open %s", dataFile.toString().c_str());

	if (isCastleMaster2()) {
		// CM2 game text (L6cb9_game_text) and area names (L6f49_area_names)
		// are both fixed-size: 16 bytes per entry (1-byte indent flag + 15
		// chars of text). The indent flag is used by Ld01c_draw_string for
		// display positioning but is not part of the text content.
		// Game text: 41 entries at offset 0x02B9.
		// Area names: 40 entries at offset 0x0549.
		// Both are loaded into _messagesList (game text at indices 0-40,
		// area names at indices 41-80).
		file.seek(0x02b9);
		for (int i = 0; i < 41; i++) {
			file.readByte(); // skip indent flag
			char buf[16];
			file.read(buf, 15);
			buf[15] = '\0';
			_messagesList.push_back(Common::String(buf));
		}

		// Area names follow immediately (L6f49_area_names, 40 entries).
		for (int i = 0; i < 40; i++) {
			file.readByte(); // skip indent flag
			char buf[16];
			file.read(buf, 15);
			buf[15] = '\0';
			_messagesList.push_back(Common::String(buf));
		}

		load8bitBinary(&file, 0x6682, 16);
		loadSpeakerFxZX(&file, 0x0bbf, 0x0bfb);

		file.seek(0x1147);
		for (int i = 0; i < 90; i++) {
			Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
			surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
			chars.push_back(loadFrame(&file, surface, 1, 8, 1));
		}
		_font = Font(chars);
		_font.setCharWidth(9);
		_fontLoaded = true;
	} else {
		loadMessagesVariableSize(&file, 0x4bd, 71);
		switch (_language) {
			case Common::ES_ESP:
				loadRiddles(&file, 0x1458, 9);
				load8bitBinary(&file, 0x6aa9, 16);
				loadSpeakerFxZX(&file, 0xca0, 0xcdc);

				file.seek(0x1228);
				for (int i = 0; i < 90; i++) {
					Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
					surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
					chars.push_back(loadFrame(&file, surface, 1, 8, 1));
				}
				_font = Font(chars);
				_font.setCharWidth(9);
				_fontLoaded = true;

				break;
			case Common::EN_ANY:
				if (_variant & GF_ZX_RETAIL) {
					loadRiddles(&file, 0x1448, 9);
					load8bitBinary(&file, 0x6a3b, 16);
					loadSpeakerFxZX(&file, 0xc91, 0xccd);
					file.seek(0x1219);
				} else if (_variant & GF_ZX_DISC) {
					loadRiddles(&file, 0x1457, 9);
					load8bitBinary(&file, 0x6a9b, 16);
					loadSpeakerFxZX(&file, 0xca0, 0xcdc);
					file.seek(0x1228);
				} else {
					error("Unknown Castle Master ZX variant");
				}
				for (int i = 0; i < 90; i++) {
					Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
					surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
					chars.push_back(loadFrame(&file, surface, 1, 8, 1));
				}
				_font = Font(chars);
				_font.setCharWidth(9);
				_fontLoaded = true;

				break;
			default:
				error("Language not supported");
				break;
		}
	}

	loadColorPalette();
	_gfx->readFromPalette(2, r, g, b);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(7, r, g, b);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	if (isCastleMaster2()) {
		_keysBorderFrames.push_back(loadFrameWithHeader(&file, 0x0d25, red, white));
	} else {
		_keysBorderFrames.push_back(loadFrameWithHeader(&file, _variant & GF_ZX_DISC ? 0xe06 : 0xdf7, red, white));
	}

	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0xff, 0);
	if (isCastleMaster2()) {
		_spiritsMeterIndicatorFrame = loadFrameWithHeader(&file, 0x0d7d, green, white);
	} else {
		_spiritsMeterIndicatorFrame = loadFrameWithHeader(&file, _variant & GF_ZX_DISC ? 0xe5e : 0xe4f, green, white);
	}

	_gfx->readFromPalette(4, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int backgroundWidth = 16;
	int backgroundHeight = 18;
	Graphics::ManagedSurface *background = new Graphics::ManagedSurface();
	background->create(backgroundWidth * 8, backgroundHeight, _gfx->_texturePixelFormat);
	background->fillRect(Common::Rect(0, 0, backgroundWidth * 8, backgroundHeight), 0);

	if (isCastleMaster2()) {
		file.seek(0x0ef2);
	} else {
		file.seek(_variant & GF_ZX_DISC ? 0xfd3 : 0xfc4);
	}
	_background = loadFrame(&file, background, backgroundWidth, backgroundHeight, front);

	_gfx->readFromPalette(6, r, g, b);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	if (isCastleMaster2()) {
		_strenghtBackgroundFrame = loadFrameWithHeader(&file, 0x0e05, yellow, black);
		_strenghtBarFrame = loadFrameWithHeader(&file, 0x0e91, yellow, black);
		_strenghtWeightsFrames = loadFramesWithHeader(&file, 0x0eb1, 4, yellow, black);
		_flagFrames = loadFramesWithHeader(&file, 0x1012, 4, green, black);
	} else {
		_strenghtBackgroundFrame = loadFrameWithHeader(&file, _variant & GF_ZX_DISC ? 0xee6 : 0xed7, yellow, black);
		_strenghtBarFrame = loadFrameWithHeader(&file, _variant & GF_ZX_DISC ? 0xf72 : 0xf63, yellow, black);
		_strenghtWeightsFrames = loadFramesWithHeader(&file, _variant & GF_ZX_DISC ? 0xf92 : 0xf83, 4, yellow, black);
		_flagFrames = loadFramesWithHeader(&file, (_variant & GF_ZX_DISC ? 0x10e4 + 15 : 0x10e4), 4, green, black);
	}

	file.skip(24);
	int thunderWidth = 4;
	int thunderHeight = 44;
	Graphics::ManagedSurface *thunderFrame = new Graphics::ManagedSurface();
	thunderFrame->create(thunderWidth * 8, thunderHeight, _gfx->_texturePixelFormat);
	thunderFrame->fillRect(Common::Rect(0, 0, thunderWidth * 8, thunderHeight), 0);
	thunderFrame = loadFrame(&file, thunderFrame, thunderWidth, thunderHeight, front);

	_thunderFrames.push_back(new Graphics::ManagedSurface);
	_thunderFrames.push_back(new Graphics::ManagedSurface);

	_thunderFrames[0]->create(thunderWidth * 8 / 2, thunderHeight, _gfx->_texturePixelFormat);
	_thunderFrames[1]->create(thunderWidth * 8 / 2, thunderHeight, _gfx->_texturePixelFormat);

	_thunderFrames[0]->copyRectToSurface(*thunderFrame, 0, 0, Common::Rect(0, 0, thunderWidth * 8 / 2, thunderHeight));
	_thunderFrames[1]->copyRectToSurface(*thunderFrame, 0, 0, Common::Rect(thunderWidth * 8 / 2, 0, thunderWidth * 8, thunderHeight));

	if (!isCastleMaster2()) {
		Graphics::Surface *tmp;
		tmp = loadBundledImage("castle_riddle_top_frame");
		_riddleTopFrame = new Graphics::ManagedSurface;
		_riddleTopFrame->copyFrom(*tmp);
		tmp->free();
		delete tmp;
		_riddleTopFrame->convertToInPlace(_gfx->_texturePixelFormat);

		tmp = loadBundledImage("castle_riddle_background_frame");
		_riddleBackgroundFrame = new Graphics::ManagedSurface();
		_riddleBackgroundFrame->copyFrom(*tmp);
		tmp->free();
		delete tmp;
		_riddleBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat);

		tmp = loadBundledImage("castle_riddle_bottom_frame");
		_riddleBottomFrame = new Graphics::ManagedSurface();
		_riddleBottomFrame->copyFrom(*tmp);
		tmp->free();
		delete tmp;
		_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat);
	}
}

void CastleEngine::drawZXUI(Graphics::Surface *surface) {
	uint32 color = 5;
	uint8 r, g, b;

	drawLiftingGate(surface);
	drawDroppingGate(surface);

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;
	_gfx->readFromPalette(color, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(123, 179, 242 + 5, 188);
	surface->fillRect(backRect, black);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		//debug("deadline: %d countdown: %d", deadline, _countdown);
		drawStringInSurface(message, 120, 179, front, black, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateControl != kFreescapeGameStateEnd) {
			if (getGameBit(31)) { // The final cutscene is playing but it is not ended yet
				drawStringInSurface(_messagesList[5], 120, 179, front, black, surface); // "You did it!"
			} else
				drawStringInSurface(_currentArea->_name, 120, 179, front, black, surface);
		}
	}

	for (int k = 0; k < int(_keysCollected.size()); k++) {
		surface->copyRectToSurface((const Graphics::Surface)*_keysBorderFrames[0], 99 - k * 4, 177, Common::Rect(0, 0, 6, 11));
	}

	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0xff, 0);

	surface->fillRect(Common::Rect(152, 156, 216, 164), green);
	surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 140 + _spiritsMeterPosition, 156, Common::Rect(0, 0, 15, 8));

	surface->fillRect(Common::Rect(64, 155, 64 + 72, 155 + 15), _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00));
	drawEnergyMeter(surface, Common::Point(64, 155));

	int ticks = g_system->getMillis() / 20;
	int flagFrameIndex = (ticks / 10) % 4;
	surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 264, 9, Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));
}

} // End of namespace Freescape
