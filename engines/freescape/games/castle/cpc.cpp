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

void CastleEngine::initCPC() {
	_viewArea = Common::Rect(40, 33 - 2, 280, 152);
	_soundIndexShoot = 5;
	_soundIndexCollide = -1;
	_soundIndexFall = -1;
	_soundIndexClimb = -1;
	_soundIndexMenu = -1;
	_soundIndexStart = 6;
	_soundIndexAreaChange = 7;
}


byte kCPCPaletteCastleTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xff},
	{0xff, 0xff, 0x00},
	{0xff, 0x00, 0x00},
};

byte kCPCPaletteCastleBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x80, 0x80, 0x80},
	{0x00, 0x80, 0x00},
	{0xff, 0xff, 0xff},
};

// Data for the mountains background. This is not included in the original game for some reason
// but all the other releases have it. This is coming from the ZX Spectrum version.
byte mountainsData[288] {
	0x06, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b,
	0x00, 0x00, 0x38, 0x01, 0xa0, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x17,
	0x80, 0x00, 0x7c, 0x03, 0xd0, 0x00, 0x00, 0x01,
	0x00, 0x3f, 0x5c, 0x80, 0x0, 0x18, 0x0, 0x23,
	0xc0, 0x00, 0xfe, 0x0d, 0xfe, 0x6, 0x00, 0x02,
	0x80, 0x7e, 0xbe, 0xc0, 0x0, 0x3c, 0x0, 0x47,
	0xe0, 0x1, 0x57, 0x56, 0xff, 0xf, 0x80, 0x5,
	0x40, 0xf5, 0xfb, 0x40, 0x0, 0x76, 0x0, 0x93,
	0xd0, 0xa, 0xab, 0xab, 0xff, 0xaf, 0xc3, 0x2a,
	0xa1, 0xeb, 0xfe, 0xa8, 0x0, 0xde, 0x0, 0x21,
	0xa8, 0x75, 0x55, 0x41, 0xff, 0xd6, 0xef, 0xd5,
	0x53, 0x57, 0xfc, 0x14, 0x1, 0xb7, 0x7, 0x42,
	0xd5, 0xea, 0xaa, 0x92, 0xfb, 0xeb, 0xab, 0xea,
	0xaa, 0xae, 0xfa, 0x4a, 0x82, 0xea, 0xbe, 0x97,
	0xab, 0xd5, 0x55, 0x25, 0xdd, 0x75, 0x45, 0xf5,
	0x55, 0x7d, 0xdd, 0x25, 0x55, 0xd5, 0x54, 0x2f,
	0xf7, 0xaa, 0xaa, 0x53, 0xea, 0xa8, 0x13, 0xfa,
	0xaa, 0xea, 0xbe, 0x42, 0xab, 0xaa, 0xa9, 0x5f,
	0xdd, 0xd5, 0x55, 0x7, 0x55, 0x2, 0x45, 0xfd,
	0x51, 0x55, 0x57, 0x15, 0x57, 0xd5, 0x52, 0xaf,
	0xee, 0xfa, 0xaa, 0x2b, 0xaa, 0x80, 0x8b, 0xfe,
	0xaa, 0xaa, 0xae, 0xaa, 0xbe, 0xaa, 0xa4, 0x5a,
	0xb5, 0x5d, 0x5c, 0x56, 0xd5, 0x29, 0x1f, 0xff,
	0x55, 0x55, 0x5b, 0x55, 0x7d, 0x55, 0x9, 0xb5,
	0x5a, 0xaf, 0xba, 0xad, 0xaa, 0x92, 0x3e, 0xbf,
	0xea, 0xaa, 0xaf, 0xab, 0xea, 0xaa, 0x2, 0x5a,
	0xf5, 0x55, 0xfd, 0x57, 0x55, 0x5, 0x5f, 0x57,
	0xfd, 0x55, 0x55, 0x57, 0x55, 0x50, 0x15, 0xaf,
	0xba, 0xaa, 0xfe, 0xae, 0xfa, 0xaa, 0xbe, 0xaa,
	0xbf, 0xaa, 0xaa, 0xaa, 0xaa, 0x82, 0xaa, 0x55,
	0x55, 0x55, 0x5f, 0xd5, 0xfd, 0x55, 0x55, 0x55,
	0x5f, 0xfd, 0x55, 0x55, 0x55, 0x55, 0x55, 0xea,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

extern Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file, bool mode0);

void CastleEngine::loadAssetsCPCFullGame() {
	Common::File file;
	uint8 r, g, b;
	Common::Array<Graphics::ManagedSurface *> chars;

    file.open("CMLOAD.BIN");

	if (!file.isOpen())
		error("Failed to open CMLOAD.BIN");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteCastleTitleData, 0, 4);

    file.close();
	file.open("CMSCR.BIN");

	if (!file.isOpen())
		error("Failed to open CMSCR.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteCastleBorderData, 0, 4);

    file.close();
	file.open("CM.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	loadMessagesVariableSize(&file, 0x16c6, 71);
	switch (_language) {
		/*case Common::ES_ESP:
			loadRiddles(&file, 0x1470 - 4 - 2 - 9 * 2, 9);
			loadMessagesVariableSize(&file, 0xf3d, 71);
			load8bitBinary(&file, 0x6aab - 2, 16);
			loadSpeakerFxZX(&file, 0xca0, 0xcdc);

			file.seek(0x1218 + 16);
			for (int i = 0; i < 90; i++) {
				Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
				surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
				chars.push_back(loadFrame(&file, surface, 1, 8, 1));
			}
			_font = Font(chars);
			_font.setCharWidth(9);
			_fontLoaded = true;

			break;*/
		case Common::EN_ANY:
			loadRiddles(&file, 0x1b75 - 2 - 9 * 2, 9);
			load8bitBinary(&file, 0x791a, 16);

			file.seek(0x2724);
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

	loadColorPalette();

	int backgroundWidth = 16;
	int backgroundHeight = 18;
	Graphics::ManagedSurface *background = new Graphics::ManagedSurface();
	background->create(backgroundWidth * 8, backgroundHeight, _gfx->_texturePixelFormat);
	background->fillRect(Common::Rect(0, 0, backgroundWidth * 8, backgroundHeight), 0);
	Common::MemoryReadStream mountainsStream(mountainsData, sizeof(mountainsData));

	_gfx->readFromPalette(11, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_background = loadFrame(&mountainsStream, background, backgroundWidth, backgroundHeight, front);
	/*_gfx->readFromPalette(2, r, g, b);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(7, r, g, b);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_keysBorderFrames.push_back(loadFrameWithHeader(&file, _language == Common::ES_ESP ? 0xe06 : 0xdf7, red, white));

	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0xff, 0);
	_spiritsMeterIndicatorFrame = loadFrameWithHeader(&file, _language == Common::ES_ESP ? 0xe5e : 0xe4f, green, white);

	_gfx->readFromPalette(4, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int backgroundWidth = 16;
	int backgroundHeight = 18;
	Graphics::ManagedSurface *background = new Graphics::ManagedSurface();
	background->create(backgroundWidth * 8, backgroundHeight, _gfx->_texturePixelFormat);
	background->fillRect(Common::Rect(0, 0, backgroundWidth * 8, backgroundHeight), 0);

	file.seek(_language == Common::ES_ESP ? 0xfd3 : 0xfc4);
	_background = loadFrame(&file, background, backgroundWidth, backgroundHeight, front);

	_gfx->readFromPalette(6, r, g, b);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	_strenghtBackgroundFrame = loadFrameWithHeader(&file, _language == Common::ES_ESP ? 0xee6 : 0xed7, yellow, black);
	_strenghtBarFrame = loadFrameWithHeader(&file, _language == Common::ES_ESP ? 0xf72 : 0xf63, yellow, black);

	Graphics::ManagedSurface *bar = new Graphics::ManagedSurface();
	bar->create(_strenghtBarFrame->w - 4, _strenghtBarFrame->h, _gfx->_texturePixelFormat);
	_strenghtBarFrame->copyRectToSurface(*bar, 4, 0, Common::Rect(4, 0, _strenghtBarFrame->w - 4, _strenghtBarFrame->h));
	_strenghtBarFrame->free();
	delete _strenghtBarFrame;
	_strenghtBarFrame = bar;

	_strenghtWeightsFrames = loadFramesWithHeader(&file, _language == Common::ES_ESP ? 0xf92 : 0xf83, 4, yellow, black);

	_flagFrames = loadFramesWithHeader(&file, (_language == Common::ES_ESP ? 0x10e4 + 15 : 0x10e4), 4, green, black);

	int thunderWidth = 4;
	int thunderHeight = 43;
	_thunderFrame = new Graphics::ManagedSurface();
	_thunderFrame->create(thunderWidth * 8, thunderHeight, _gfx->_texturePixelFormat);
	_thunderFrame->fillRect(Common::Rect(0, 0, thunderWidth * 8, thunderHeight), 0);
	_thunderFrame = loadFrame(&file, _thunderFrame, thunderWidth, thunderHeight, front);

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
	_riddleBottomFrame->convertToInPlace(_gfx->_texturePixelFormat);*/

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		it._value->addObjectFromArea(164, _areaMap[255]);
		it._value->addObjectFromArea(174, _areaMap[255]);
		it._value->addObjectFromArea(175, _areaMap[255]);
		for (int16 id = 136; id < 140; id++) {
			it._value->addObjectFromArea(id, _areaMap[255]);
		}

		it._value->addObjectFromArea(195, _areaMap[255]);
		for (int16 id = 214; id < 228; id++) {
			it._value->addObjectFromArea(id, _areaMap[255]);
		}
	}
	// Discard some global conditions
	// It is unclear why they hide/unhide objects that formed the spirits
	for (int i = 0; i < 3; i++) {
		debugC(kFreescapeDebugParser, "Discarding condition %s", _conditionSources[0].c_str());
		_conditions.remove_at(0);
		_conditionSources.remove_at(0);
	}
}

void CastleEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _gfx->_paperColor;
	//uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 1;

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
		if (_gameStateControl == kFreescapeGameStatePlaying) {
				drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);
		}
	}

	/*uint32 color = 5;
	uint8 r, g, b;

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
		if (_gameStateControl == kFreescapeGameStatePlaying) {
			drawStringInSurface(_currentArea->_name, 120, 179, front, black, surface);
		}
	}

	for (int k = 0; k < int(_keysCollected.size()); k++) {
		surface->copyRectToSurface((const Graphics::Surface)*_keysBorderFrames[0], 99 - k * 4, 177, Common::Rect(0, 0, 6, 11));
	}

	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0xff, 0);

	surface->fillRect(Common::Rect(152, 156, 216, 164), green);
	surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 140 + _spiritsMeterPosition, 156, Common::Rect(0, 0, 15, 8));
	drawEnergyMeter(surface, Common::Point(63, 154));

	int ticks = g_system->getMillis() / 20;
	int flagFrameIndex = (ticks / 10) % 4;
	surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 264, 9, Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));*/
}

} // End of namespace Freescape
