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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "common/endian.h"
#include "common/translation.h"
#include "image/scr.h"

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

void Kit8Engine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	if (!isSpectrum())
		return;
	for (Common::Action *action : engineKeyMap->getActions()) {
		switch (action->event.customType) {
		case kActionShoot: action->addDefaultInputMapping("b"); break;
		case kActionMoveLeft: action->addDefaultInputMapping("z"); break;
		case kActionMoveRight: action->addDefaultInputMapping("x"); break;
		case kActionInfoMenu: action->description = _("Center view"); break;
		default: break;
		}
	}
	const struct {
		const char *id, *label, *key;
		uint action;
	} controls[] = {
		{"ROTL", _s("Rotate left"), "q", kActionRotateLeft},
		{"ROTR", _s("Rotate right"), "w", kActionRotateRight},
		{"ACTIVATE", _s("Activate"), "a", kActionActivate},
		{"RISE", _s("Stand up"), "r", kActionRiseOrFlyUp},
		{"LOWER", _s("Crouch"), "f", kActionLowerOrFlyDown}
	};
	for (const auto &control : controls) {
		Common::Action *action = new Common::Action(control.id, _(control.label));
		action->setCustomEngineActionEvent(control.action);
		action->addDefaultInputMapping(control.key);
		engineKeyMap->addAction(action);
	}
	Common::Action *act = new Common::Action("BREAK", _("Restart"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_BREAK, 27));
	act->addDefaultInputMapping("S+SPACE");
	act->addDefaultInputMapping("BREAK");
	engineKeyMap->addAction(act);
}

void Kit8Engine::loadPresentationZX() {
	Common::File file;
	Image::ScrDecoder decoder;
	memcpy(_zxPalette, decoder.getPalette().data(), sizeof(_zxPalette));
	if (file.open("3dkit.zx.title")) {
		if (file.size() != 6912 || !decoder.loadStream(file))
			error("Invalid 3D Construction Kit Spectrum title");
		_title = new Graphics::ManagedSurface;
		_title->copyFrom(*decoder.getSurface());
		_title->convertToInPlace(_gfx->_texturePixelFormat, _zxPalette, 16);
		file.close();
	}
	if (!file.open("3dkit.zx.border") || file.size() != 6912)
		error("Missing 3D Construction Kit Spectrum border");
	byte bitmap[6144];
	if (file.read(bitmap, sizeof(bitmap)) != sizeof(bitmap) ||
			file.read(_borderAttributes, sizeof(_borderAttributes)) != sizeof(_borderAttributes))
		error("Truncated 3D Construction Kit Spectrum border");
	memcpy(_attributes, _borderAttributes, sizeof(_attributes));
	for (int y = 0; y < _screenH; ++y) {
		for (int x = 0; x < _screenW; ++x) {
			uint address = ((y & 0xc0) << 5) | ((y & 7) << 8) | ((y & 0x38) << 2) | (x >> 3);
			_borderSurface.setPixel(x, y, (bitmap[address] >> (7 - (x & 7))) & 1);
		}
	}
	_borderSurface.fillRect(_viewArea, 255);
	file.close();

	if (!file.open("3dkit.zx.code") || file.size() < 0x5eb7)
		error("Missing 3D Construction Kit Spectrum runner");
	file.seek(0x5e7b);
	if (file.read(_colorPatterns, sizeof(_colorPatterns)) != sizeof(_colorPatterns))
		error("Truncated 3D Construction Kit Spectrum shades");
	file.seek(0x1690);
	uint16 font = file.readUint16LE();
	if (font >= 0xa000 && font - 0xa000 + sizeof(_fontData) <= uint32(file.size())) {
		file.seek(font - 0xa000);
		_hasFont = file.read(_fontData, sizeof(_fontData)) == sizeof(_fontData);
	} else {
		// Tapes omit the ROM font; the runner also has a compact character set.
		file.seek(0x220);
		for (uint chr = 0; chr < 64; ++chr) {
			for (uint row = 1; row <= 6; ++row)
				_fontData[chr][row] = file.readByte() >> 1;
		}
		for (uint chr = 64; chr < 96; ++chr)
			memcpy(_fontData[chr], _fontData[chr - 32], 8);
		_hasFont = !file.err();
	}
}

void Kit8Engine::setAttributesZX(const Common::Rect &rect, byte color) {
	for (int y = rect.top / 8; y < (rect.bottom + 7) / 8; ++y) {
		for (int x = rect.left / 8; x < (rect.right + 7) / 8; ++x)
			_attributes[y * 32 + x] = color;
	}
}

void Kit8Engine::applyPaletteZX() {
	_gfx->_palette = _zxPalette;
	_gfx->_inkColor = _palette[0] | (_palette[2] << 3);
	_gfx->_paperColor = _palette[1] | (_palette[2] << 3);
	_currentArea->_usualBackgroundColor = 1;
	_currentArea->_skyColor = 1;
	_currentArea->_underFireBackgroundColor = 2;
	setAttributesZX(_viewArea, _palette[0] | (_palette[1] << 3) | (_palette[2] << 6));
}

} // namespace Freescape
