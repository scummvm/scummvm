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

#include "common/endian.h"
#include "graphics/fonts/dosfont.h"
#include "math/utils.h"

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

static const byte kCPCInks[27] = {
	20, 4, 21, 28, 24, 29, 12, 5, 13, 22, 6, 23, 30, 0,
	31, 14, 7, 15, 18, 2, 19, 26, 25, 27, 10, 3, 11
};

void Kit8Engine::loadPresentation() {
	_gfx->_keyColor = 0;
	_scriptSurface.create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	_scriptSurface.fillRect(_fullscreenViewArea, 255);
	_overlaySurface.create(_screenW, _screenH, _gfx->_texturePixelFormat);
	_borderSurface.create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	_borderSurface.fillRect(_fullscreenViewArea, 0);
	_colorMap.resize(ARRAYSIZE(_colorPatterns));
	for (uint i = 0; i < _colorMap.size(); i++)
		_colorMap[i] = _colorPatterns[i];
	if (isSpectrum()) {
		loadPresentationZX();
		return;
	}
	if (isC64()) {
		loadPresentationC64();
		return;
	}
	// Shade 0 is transparent; the CPC runner indexes these patterns with shade - 1.
	static const byte patterns[15][4] = {
		{0x00, 0x00, 0x00, 0x00},
		{0x0f, 0x0f, 0x0f, 0x0f}, {0xf0, 0xf0, 0xf0, 0xf0}, {0xff, 0xff, 0xff, 0xff},
		{0x0a, 0x05, 0x0a, 0x05}, {0xa0, 0x50, 0xa0, 0x50}, {0xaa, 0x55, 0xaa, 0x55},
		{0xa5, 0x5a, 0xa5, 0x5a}, {0xaf, 0x5f, 0xaf, 0x5f}, {0xfa, 0xf5, 0xfa, 0xf5},
		{0x02, 0x08, 0x02, 0x08}, {0x20, 0x80, 0x20, 0x80}, {0x22, 0x88, 0x22, 0x88},
		{0x2d, 0x87, 0x2d, 0x87}, {0x2f, 0x8f, 0x2f, 0x8f}
	};
	memcpy(_colorPatterns, patterns, sizeof(_colorPatterns));

	Common::File file;
	if (file.open("BORDER.DAT")) {
		if (file.size() != 16384 && file.size() != 16512)
			error("Invalid 3D Construction Kit CPC border size");
		file.seek(file.size() - 16384);
		Common::Array<byte> screen;
		screen.resize(16384);
		file.read(screen.data(), screen.size());
		for (int y = 0; y < _screenH; y++) {
			for (int x = 0; x < _screenW; x++)
				_borderSurface.setPixel(x, y, getCPCPixelMode1(screen[(y & 7) * 2048 + (y >> 3) * 80 + (x >> 2)], x & 3));
		}
		file.close();
	}
	_borderSurface.fillRect(_viewArea, 255);
	// A saved editor data file omits the runner's font and border.
	const char *runner = _gameDescription->filesDescriptions[1].fileName;
	if (file.open(runner ? runner : "DISC.BIN") && file.size() == 25216) {
		byte header[128];
		file.read(header, sizeof(header));
		if (READ_LE_UINT16(header + 21) == 0x3e00 && READ_LE_UINT16(header + 24) == 25088) {
			file.seek(128 + 0x9c00 - 0x3e00);
			_hasFont = file.read(_fontData, sizeof(_fontData)) == sizeof(_fontData);
		}
	}
}

void Kit8Engine::applyPalette() {
	if (isSpectrum()) {
		applyPaletteZX();
		return;
	}
	if (isC64()) {
		applyPaletteC64();
		return;
	}
	_gfx->_fourColorBackground = kCPCInks[_palette[0]];
	_gfx->_underFireBackgroundColor = kCPCInks[_palette[2]];
	_gfx->_paperColor = kCPCInks[_palette[1]];
	_gfx->_inkColor = kCPCInks[_palette[3]];
	_currentArea->_usualBackgroundColor = encodeCPCDirectColor(kCPCInks[_palette[0]]);
	_currentArea->_skyColor = _currentArea->_usualBackgroundColor;
	_currentArea->_underFireBackgroundColor = encodeCPCDirectColor(kCPCInks[_palette[3]]);
}

void Kit8Engine::printText(const Common::String &text, byte x, byte y, byte color) {
	if (!_textOutputEnabled || x >= _screenW / 8 || y >= _screenH / 8)
		return;
	Graphics::DosFont font;
	byte foreground = isSpectrum() ? 1 : color & (isC64() ? 15 : 3);
	byte background = isSpectrum() ? 0 : isC64() ? color >> 4 : ((color >> 3) & 1) | ((color >> 1) & 2);
	for (uint i = 0; i < text.size() && x < _screenW / 8; i++, x++) {
		byte chr = text[i];
		if (isSpectrum())
			_attributes[y * 32 + x] = color;
		_scriptSurface.fillRect(Common::Rect(8 * x, 8 * y, 8 * x + 8, 8 * y + 8), background);
		if (_hasFont && chr >= 32 && chr < 128) {
			for (int row = 0; row < 8; row++) {
				for (int col = 0; col < 8; col++) {
					if (_fontData[chr - 32][row] & (0x80 >> col))
						_scriptSurface.setPixel(8 * x + col, 8 * y + row, foreground);
				}
			}
		} else
			font.drawChar(_scriptSurface.surfacePtr(), chr, 8 * x, 8 * y, foreground);
	}
}

void Kit8Engine::printMessage(byte id, byte x, byte y) {
	if (_kitMessages.contains(id))
		printText(_kitMessages[id], x, y, _textColor);
}

void Kit8Engine::updateInstruments() {
	for (const auto &instrument : _instruments) {
		byte type = instrument[0], x = instrument[1], y = instrument[2], length = instrument[3];
		byte variable = instrument[4] & 127, color = instrument[5];
		if (!type || type > 3 || x >= _screenW / 8 || y >= _screenH / 8 || !length)
			continue;
		uint16 value = _kitVariables[variable];
		if (type == 1) {
			if (length > 5 || x + length > _screenW / 8)
				continue;
			if (length > 3)
				value |= _kitVariables[(variable + 1) & 127] << 8;
			printText(Common::String::format("%0*u", length, value), x, y, color);
		} else {
			if ((type == 2 && x + length > _screenW / 8) || (type == 3 && y + length > _screenH / 8))
				continue;
			Common::Rect bar(8 * x, 8 * y, 8 * (x + (type == 2 ? length : 1)), 8 * (y + (type == 3 ? length : 1)));
			if (isSpectrum())
				setAttributesZX(bar, color);
			_scriptSurface.fillRect(bar, isSpectrum() ? 0 : isC64() ? color >> 4 : (color >> 2) & 3);
			int filled = MIN<int>(value, 8 * length);
			if (isC64() && type == 2)
				filled &= ~1;
			if (type == 2)
				bar.right = bar.left + filled;
			else
				bar.top = bar.bottom - filled;
			if (!bar.isEmpty())
				_scriptSurface.fillRect(bar, isSpectrum() ? 1 : color & (isC64() ? 15 : 3));
		}
	}
}

void Kit8Engine::drawUI() {
	uint32 colors[16];
	bool flash = (g_system->getMillis() / 320) & 1;
	for (uint i = 0; i < (isSpectrum() || isC64() ? 16 : 4); i++) {
		byte r, g, b;
		if (isSpectrum() || isC64())
			_gfx->readFromPalette(i, r, g, b);
		else
			_gfx->selectColorFromFourColorPalette(i, r, g, b);
		colors[i] = _overlaySurface.format.ARGBToColor(255, r, g, b);
	}
	for (int y = 0; y < _screenH; y++) {
		for (int x = 0; x < _screenW; x++) {
			byte pen = _scriptSurface.getPixel(x, y);
			if (pen == 255)
				pen = _borderSurface.getPixel(x, y);
			if (isC64() && pen == 16)
				pen = _palette[0];
			if (isSpectrum() && pen != 255) {
				byte attr = _attributes[(y / 8) * 32 + x / 8];
				if ((attr & 128) && flash)
					pen ^= 1;
				pen = ((attr >> 3) & 8) | (pen ? attr & 7 : (attr >> 3) & 7);
			}
			_overlaySurface.setPixel(x, y, pen == 255 ? 0 : colors[pen]);
		}
	}
	drawFullscreenSurface(_overlaySurface.surfacePtr());
	if (_crossVisible) {
		_gfx->setViewport(_fullscreenViewArea);
		_gfx->renderCrossair(_crossairPosition);
	}
	playPendingSound();
}

bool Kit8Engine::handleInput(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
		if (isSpectrum() && event.kbd.keycode == Common::KEYCODE_BREAK) {
			if (event.type == Common::EVENT_KEYDOWN)
				_gameStateControl = kFreescapeGameStateRestart;
			return true;
		}
		byte key = event.kbd.ascii < 128 ? event.kbd.ascii : 255;
		if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER)
			key = 13;
		if (key >= 'a' && key <= 'z')
			key -= 'a' - 'A';
		if (event.type == Common::EVENT_KEYDOWN)
			_currentKey = key;
		else if (_currentKey == key)
			_currentKey = 255;
		if (!_scriptFrameActive)
			_kitVariables[121] = _currentKey;
	} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START || event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
		if (isSpectrum() && (event.customType == kActionRiseOrFlyUp || event.customType == kActionLowerOrFlyDown)) {
			if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START && !_scriptFrameActive && !isPlayingSound())
				setMovementMode(event.customType == kActionRiseOrFlyUp ? 1 : 0);
			return true;
		}
		if (event.customType == kActionSkip || event.customType == kActionInfoMenu) {
			byte key = event.customType == kActionSkip ? ' ' : 'I';
			if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				_currentKey = key;
				if (isSpectrum() && event.customType == kActionInfoMenu && !_scriptFrameActive) {
					_pitch = _roll = 0;
					updateCamera();
				}
			} else if (_currentKey == key)
				_currentKey = 255;
			if (!_scriptFrameActive)
				_kitVariables[121] = _currentKey;
			return true;
		}
		if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END)
			return false;
		switch (event.customType) {
		case kActionShoot:
		case kActionActivate:
			if (!_scriptFrameActive)
				interact(event.customType == kActionShoot);
			return true;
		case kActionEscape:
		case kActionChangeMode:
			return false;
		default:
			break;
		}
		bool movement = event.customType == kActionMoveUp || event.customType == kActionMoveDown ||
			event.customType == kActionMoveLeft || event.customType == kActionMoveRight;
		return _scriptFrameActive && !movement;
	} else if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
		if (_shootMode)
			_crossairPosition = getNormalizedPosition(event.mouse);
		if (!_scriptFrameActive)
			interact(event.type == Common::EVENT_LBUTTONDOWN);
		return true;
	} else if (_scriptFrameActive && event.type == Common::EVENT_MOUSEMOVE)
		return true;
	return false;
}

void Kit8Engine::interact(bool shot) {
	if (!_viewArea.contains(_crossairPosition) || (shot && !_kitVariables[125]) || (isSpectrum() && isPlayingSound()))
		return;
	float x = 2.0f * (_crossairPosition.x - _viewArea.left) / _viewArea.width() - 1;
	float y = 1 - 2.0f * (_crossairPosition.y - _viewArea.top) / _viewArea.height();
	float projection = tan(Math::deg2rad(_fieldOfView) / 2);
	Math::Vector3d direction = directionToVector(_pitch + Math::rad2deg(atan(y * projection / _viewAspectRatio)),
		_yaw - Math::rad2deg(atan(x * projection)), false);
	Object *object = _currentArea->checkCollisionRay(Math::Ray(_position, direction), 8192, true);
	if (shot) {
		if (_kitVariables[125] != 255)
			_kitVariables[125]--;
		_shootingFrames = 3;
		playSound(3, true);
	}
	if (!object || !object->isGeometric())
		return;
	if (!shot) {
		Math::Vector3d diff = object->getOrigin() + object->getSize() / 2 - _position;
		if (ABS(diff.x()) + ABS(diff.y()) + ABS(diff.z()) > 32.0f * _activationRange / _currentArea->getScale())
			return;
	}
	executeObjectConditions(static_cast<GeometricObject *>(object), shot, false, !shot);
}

} // namespace Freescape
