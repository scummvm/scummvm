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
 */

#include "ripper/toolbar.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/input.h"

namespace Ripper {

static const uint kToolbarActionCount = 9;
static const int kToolbarActivationHeight = 50;
static const int kToolbarRightEdge = 630;
static const int kToolbarActionGap = 5;
static const uint32 kDosTickMillis = 55;
static const uint32 kPreviewDelayMillis = 27 * kDosTickMillis;
static const byte kToolbarBlack = 4;
static const byte kToolbarWhite = 253;

static const char *const kToolbarHandlerNames[kToolbarActionCount] = {
	"RunTake2IniSliderSetupMenu",
	"HandleSceneSelectionAction",
	"RunUnlockGatedSelectionMenu",
	"RunWacFrontEndLoop",
	"RunSaveRestoreSlotMenu(save)",
	"RunSaveRestoreSlotMenu(restore)",
	"RunOptionsMenu",
	"RunModalSelectionTableDialogWithRestore",
	"RunBinaryPromptChooser"
};

ToolbarManager::ToolbarManager() : _sessionStartMillis(0), _lastFrameMillis(0), _hoveredAction(-1),
		_pressedAction(-1), _active(false), _previewEnabled(false) {
}

bool ToolbarManager::initialize(ResourceManager &resources) {
	Common::Array<Common::String> gameText;
	if (!resources.loadGameText(gameText) || gameText.size() < kToolbarActionCount ||
		!resources.loadInterfaceBitmapFont("7pt_font.fnt", _font))
		return false;

	_actions.clear();
	_actions.resize(kToolbarActionCount);
	for (uint i = 0; i < kToolbarActionCount; ++i) {
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("toolbar%u.pl", i + 1), _actions[i].sequence) ||
			_actions[i].sequence.frames.size() != 10) {
			warning("Ripper: invalid toolbar action asset %u", i + 1);
			return false;
		}
		_actions[i].label = gameText[i];
	}

	// RunFrontEndActionMenu at 0x18b3a lays controls out from right to left,
	// subtracting each bitmap width and a five-pixel gap from x=630.
	int x = kToolbarRightEdge;
	for (int i = kToolbarActionCount - 1; i >= 0; --i) {
		const BitmapAssetFrame &frame = _actions[i].sequence.frames[0];
		x -= frame.width + kToolbarActionGap;
		_actions[i].bounds = Common::Rect(x, 0, x + frame.width, frame.height);
		for (uint frameIndex = 1; frameIndex < _actions[i].sequence.frames.size(); ++frameIndex) {
			const BitmapAssetFrame &candidate = _actions[i].sequence.frames[frameIndex];
			if (candidate.width != frame.width || candidate.height != frame.height)
				return false;
		}
		debugC(2, kDebugResources,
			"Ripper: toolbar action=%u label='%s' rect=%d,%d,%d,%d frames=%u",
			i + 1, _actions[i].label.c_str(), _actions[i].bounds.left,
			_actions[i].bounds.top, _actions[i].bounds.width(), _actions[i].bounds.height(),
			_actions[i].sequence.frames.size());
	}

	debugC(1, kDebugScene,
		"Ripper: initialized front-end toolbar actions=%u activationHeight=%d previewTicks=27",
		_actions.size(), kToolbarActivationHeight);
	return true;
}

void ToolbarManager::enter(uint32 now) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w < 640 || screen->h < 480) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	_topBacking.resize(640 * kToolbarActivationHeight);
	for (int y = 0; y < kToolbarActivationHeight; ++y)
		memcpy(_topBacking.data() + y * 640, screen->getBasePtr(0, y), 640);
	g_system->unlockScreen();

	_active = true;
	_previewEnabled = false;
	_hoveredAction = -1;
	_pressedAction = -1;
	_sessionStartMillis = now;
	_lastFrameMillis = now;
	drawIcons();
	debugC(2, kDebugScene, "Ripper: entered toolbar input band y<%d", kToolbarActivationHeight);
}

void ToolbarManager::leave() {
	if (!_active)
		return;
	removeTooltip();
	if (!_topBacking.empty())
		g_system->copyRectToScreen(_topBacking.data(), 640, 0, 0, 640, kToolbarActivationHeight);
	_topBacking.clear();
	_active = false;
	_previewEnabled = false;
	_hoveredAction = -1;
	_pressedAction = -1;
	debugC(2, kDebugScene, "Ripper: left toolbar input band");
}

int ToolbarManager::findAction(const Common::Point &point) const {
	for (uint i = 0; i < _actions.size(); ++i) {
		if (_actions[i].bounds.contains(point))
			return i;
	}
	return -1;
}

void ToolbarManager::drawIcons() {
	if (!_active || _topBacking.empty())
		return;

	removeTooltip();
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (uint i = 0; i < _actions.size(); ++i) {
		const Action &action = _actions[i];
		for (int y = action.bounds.top; y < action.bounds.bottom; ++y) {
			memcpy(screen->getBasePtr(action.bounds.left, y),
				_topBacking.data() + y * 640 + action.bounds.left, action.bounds.width());
		}

		const BitmapAssetFrame &frame = action.sequence.frames[action.frameIndex];
		for (uint y = 0; y < frame.height; ++y) {
			byte *destination = (byte *)screen->getBasePtr(action.bounds.left,
				action.bounds.top + y);
			const byte *source = frame.pixels.data() + y * frame.width;
			for (uint x = 0; x < frame.width; ++x) {
				if (source[x] != frame.transparentColor)
					destination[x] = source[x];
			}
		}
	}
	g_system->unlockScreen();
}

uint ToolbarManager::measureText(const Common::String &text) const {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			width += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
			character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		width += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
	return width;
}

void ToolbarManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text) const {
	int drawX = x;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			drawX += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
			character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte pixel = _font.pixels[glyph.pixelOffset + glyphY * glyph.width + glyphX];
				if (pixel != _font.transparentColor)
					screen[(y + glyph.yOffset + glyphY) * pitch +
						drawX + glyph.xOffset + glyphX] = kToolbarBlack;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void ToolbarManager::removeTooltip() {
	if (_tooltipBacking.empty())
		return;
	g_system->copyRectToScreen(_tooltipBacking.data(), _tooltipBounds.width(),
		_tooltipBounds.left, _tooltipBounds.top, _tooltipBounds.width(), _tooltipBounds.height());
	_tooltipBacking.clear();
}

void ToolbarManager::drawTooltip(const Common::Point &point) {
	removeTooltip();
	if (!_previewEnabled || _hoveredAction < 0)
		return;

	const Common::String &label = _actions[_hoveredAction].label;
	const int width = measureText(label) + 4;
	const int height = _font.lineHeight + 4;
	int x = point.x + 20;
	int y = point.y;
	if (x + width > 640)
		x = 640 - width;
	if (y + height > 400)
		y = 400 - height;
	_tooltipBounds = Common::Rect(x, y, x + width, y + height);

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	_tooltipBacking.resize(width * height);
	for (int row = 0; row < height; ++row) {
		memcpy(_tooltipBacking.data() + row * width, screen->getBasePtr(x, y + row), width);
		memset(screen->getBasePtr(x, y + row), kToolbarWhite, width);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (int column = 0; column < width; ++column) {
		pixels[y * screen->pitch + x + column] = kToolbarBlack;
		pixels[(y + height - 1) * screen->pitch + x + column] = kToolbarBlack;
	}
	for (int row = 0; row < height; ++row) {
		pixels[(y + row) * screen->pitch + x] = kToolbarBlack;
		pixels[(y + row) * screen->pitch + x + width - 1] = kToolbarBlack;
	}
	drawText(pixels, screen->pitch, x + 2, y + 2, label);
	g_system->unlockScreen();
}

void ToolbarManager::dispatchAction(uint actionIndex) {
	debugC(1, kDebugScene,
		"Ripper: toolbar action=%u id=0x%x label='%s' handler=%s is stubbed",
		actionIndex + 1, actionIndex + 0x514, _actions[actionIndex].label.c_str(),
		kToolbarHandlerNames[actionIndex]);
}

bool ToolbarManager::service(const MouseState &mouse) {
	if (mouse.position.y >= kToolbarActivationHeight) {
		leave();
		return false;
	}

	const uint32 now = g_system->getMillis();
	if (!_active)
		enter(now);
	if (!_active)
		return true;

	const int hoveredAction = findAction(mouse.position);
	if (hoveredAction != _hoveredAction) {
		removeTooltip();
		_hoveredAction = hoveredAction;
		_lastFrameMillis = now;
		debugC(2, kDebugInput, "Ripper: toolbar hover action=%d point=%d,%d",
			_hoveredAction < 0 ? 0 : _hoveredAction + 1, mouse.position.x, mouse.position.y);
	}

	if (!_previewEnabled && now - _sessionStartMillis >= kPreviewDelayMillis) {
		_previewEnabled = true;
		debugC(3, kDebugScene, "Ripper: toolbar previews enabled after 27 DOS ticks");
	}

	bool redraw = false;
	if (_hoveredAction >= 0 && now - _lastFrameMillis >= kDosTickMillis) {
		Action &action = _actions[_hoveredAction];
		action.frameIndex = (action.frameIndex + 1) % action.sequence.frames.size();
		_lastFrameMillis = now;
		redraw = true;
		debugC(11, kDebugScene, "Ripper: toolbar action=%u frame=%u",
			_hoveredAction + 1, action.frameIndex);
	}
	if (redraw)
		drawIcons();
	if (_previewEnabled && _hoveredAction >= 0)
		drawTooltip(mouse.position);
	else
		removeTooltip();

	if ((mouse.pressed & kMouseButtonLeft) != 0 && _hoveredAction >= 0) {
		_pressedAction = _hoveredAction;
		debugC(3, kDebugInput, "Ripper: toolbar primary press action=%u point=%d,%d",
			_pressedAction + 1, mouse.position.x, mouse.position.y);
	}
	if ((mouse.released & kMouseButtonLeft) != 0 && _pressedAction >= 0) {
		const uint action = _pressedAction;
		dispatchAction(action);
		leave();
	}
	return true;
}

} // End of namespace Ripper
