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

#include "ripper/remote_control.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const int kRemoteX = 212;
static const int kRemoteY = 126;
static const int kRemoteWidth = 216;
static const int kRemoteHeight = 148;
static const int kLabelX = 270;
static const int kLabelY = 152;
static const int kLabelWidth = 90;
static const int kLabelHeight = 7;
static const int kTickX = 278;
static const int kTickY = 171;
static const int kTickWidth = 7;
static const int kTickHeight = 2;
static const int kTickSpacing = 8;
static const byte kTextColor = 251;
static const byte kLabelBackgroundColor = 5;
static const byte kFilledTickColor = 254;
static const byte kEmptyTickColor = 252;
static const uint kDefaultCursor = 14;
static const uint kActiveCursor = 16;
static const uint kHelpResource = 0x19d;

struct ControlLayout {
	int x;
	int y;
	uint16 command;
};

// g_take2IniSliderSetupUiLayoutTemplate at 0x1860a stores coordinates in the
// original display service's transposed order. These are the physical screen
// positions produced by adding menu origin (212,126) to its six control rows.
static const ControlLayout kControlLayouts[] = {
	{ 270, 213, 0x001b },
	{ 338, 214, 0x1300 },
	{ 217, 192, 0x4b00 },
	{ 393, 198, 0x4d00 },
	{ 306, 203, 0x4800 },
	{ 306, 203, 0x5000 }
};

} // End of anonymous namespace

RemoteControlManager::RemoteControlManager(RipperEngine *engine) :
		_engine(engine), _initialized(false), _videoPaletteChanged(false) {
}

bool RemoteControlManager::initialize(ResourceManager &resources) {
	Common::Array<Common::String> gameText;
	if (!resources.loadInterfaceBitmapFont("5pt_font.fnt", _font) ||
		!resources.loadGameText(gameText) || gameText.size() < 0x129)
		return false;

	_labels.resize(RipperSettings::kSliderCount);
	for (uint i = 0; i < _labels.size(); ++i)
		_labels[i] = gameText[0x121 + i];

	_controls.resize(ARRAYSIZE(kControlLayouts));
	for (uint i = 0; i < _controls.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("remote%u.bbm", i), sequence) ||
			sequence.frames.empty())
			return false;
		_controls[i].frame = sequence.frames.front();
		uint hitWidth = _controls[i].frame.width;
		uint hitHeight = _controls[i].frame.height;
		if (i == 4) {
			// The fifth row at 0x1863a clips the shared +/- rocker to 27 by 19
			// in the display service's transposed coordinates: 19 by 27 here.
			hitWidth = MIN<uint>(19, hitWidth);
			hitHeight = MIN<uint>(27, hitHeight);
		}
		_controls[i].bounds = Common::Rect(kControlLayouts[i].x, kControlLayouts[i].y,
			kControlLayouts[i].x + hitWidth,
			kControlLayouts[i].y + hitHeight);
		_controls[i].command = kControlLayouts[i].command;
		debugC(2, kDebugResources,
			"Ripper: Remote Control control=%u command=0x%04x rect=%d,%d,%u,%u",
			i, _controls[i].command, _controls[i].bounds.left, _controls[i].bounds.top,
			_controls[i].bounds.width(), _controls[i].bounds.height());
	}

	_initialized = true;
	debugC(1, kDebugGeneral,
		"Ripper: initialized Remote Control sliders=%u controls=%u bounds=%d,%d,%d,%d",
		(uint)RipperSettings::kSliderCount, _controls.size(), kRemoteX, kRemoteY,
		kRemoteWidth, kRemoteHeight);
	return true;
}

bool RemoteControlManager::captureDisplay() {
	_videoPaletteChanged = false;
	return _savedDisplay.capture(Common::Rect(kRemoteX, kRemoteY,
		kRemoteX + kRemoteWidth, kRemoteY + kRemoteHeight));
}

void RemoteControlManager::restoreDisplay() {
	_savedDisplay.restorePixels();
	applyPalette();
	_savedDisplay.clear();
	presentScreen();
}

void RemoteControlManager::applyPalette() {
	// REMOTE.SMK draws only with the shared interface bands patched by
	// ApplySharedDisplayPalettePatch at 0x205d0. Its unused palette entries are
	// chroma-key green and must not replace the suspended scene palette.
	if (_videoPaletteChanged && _engine->getSettings()->restoreVideoPalette())
		return;
	_savedDisplay.restorePalette();
}

void RemoteControlManager::drawBitmap(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint row = 0; row < frame.height; ++row) {
		for (uint column = 0; column < frame.width; ++column) {
			const byte pixel = frame.pixels[row * frame.width + column];
			if (pixel != frame.transparentColor)
				screen[(y + row) * pitch + x + column] = pixel;
		}
	}
}

void RemoteControlManager::drawControls() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint i = 0; i < _controls.size(); ++i)
		drawBitmap(pixels, screen->pitch, _controls[i].frame,
			_controls[i].bounds.left, _controls[i].bounds.top);
	g_system->unlockScreen();
}

void RemoteControlManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text, kTextColor);
}

int RemoteControlManager::findControl(const Common::Point &point) const {
	// AppendUiControlStateToList and FindUiControlStateAtPoint at 0x4aae8
	// retain insertion order and return the first hit. This matters for the
	// clipped increase control placed over the full-height decrease rocker.
	for (uint i = 0; i < _controls.size(); ++i) {
		const Control &control = _controls[i];
		if (!control.bounds.contains(point))
			continue;
		const int x = point.x - control.bounds.left;
		const int y = point.y - control.bounds.top;
		if (control.frame.pixels[y * control.frame.width + x] !=
			control.frame.transparentColor)
			return i;
	}
	return -1;
}

bool RemoteControlManager::handleCommand(uint16 command, uint &selectedSlider) {
	RipperSettings *settings = _engine->getSettings();
	if (command == 0x001b)
		return false;
	if (command == 0x3b00) {
		debugC(1, kDebugGeneral,
			"Ripper: Remote Control opening help resource=0x%x", kHelpResource);
		_engine->getModalDialog()->run(kHelpResource);
		return true;
	}
	if (command == 0x1300) {
		bool videoPaletteChanged = false;
		for (uint i = RipperSettings::kBrightness; i <= RipperSettings::kTint; ++i) {
			const RipperSettings::Slider slider = (RipperSettings::Slider)i;
			if (settings->getValue(slider) != RipperSettings::getDescriptor(slider).defaultValue)
				videoPaletteChanged = true;
		}
		settings->resetDefaults();
		if (videoPaletteChanged) {
			_videoPaletteChanged = true;
			applyPalette();
		}
		return true;
	}
	if (command == 0x4b00) {
		selectedSlider = selectedSlider == 0 ? RipperSettings::kSliderCount - 1 :
			selectedSlider - 1;
		return true;
	}
	if (command == 0x4d00) {
		selectedSlider = (selectedSlider + 1) % RipperSettings::kSliderCount;
		return true;
	}
	if (command != 0x4800 && command != 0x5000 && command != '+' && command != '-')
		return true;

	const RipperSettings::Slider slider = (RipperSettings::Slider)selectedSlider;
	const RipperSettings::Descriptor &descriptor = RipperSettings::getDescriptor(slider);
	const int direction = command == 0x4800 || command == '+' ? 1 : -1;
	const int oldValue = settings->getValue(slider);
	settings->setValue(slider, oldValue + descriptor.step * direction);
	if (selectedSlider >= RipperSettings::kBrightness &&
		settings->getValue(slider) != oldValue) {
		_videoPaletteChanged = true;
		applyPalette();
	}
	return true;
}

bool RemoteControlManager::run() {
	if (!_initialized || !captureDisplay())
		return false;
	debugC(1, kDebugGeneral,
		"Ripper: entering RunTake2IniSliderSetupMenu controls=%u", _controls.size());
	_engine->getInput()->drainKeys();
	_engine->getInput()->publishMouseState();
	Common::Array<byte> remotePalette;
	if (!_engine->getMedia()->playInterfaceSequence(
		"remote.smk", kRemoteX, kRemoteY, remotePalette)) {
		restoreDisplay();
		return false;
	}
	applyPalette();
	drawControls();

	uint selectedSlider = 0;
	int hoveredControl = -1;
	int pressedControl = -1;
	bool active = true;
	auto redrawSettings = [&]() {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			return;
		}
		byte *pixels = (byte *)screen->getPixels();
		for (int y = 0; y < kLabelHeight; ++y)
			memset(pixels + (kLabelY + y) * screen->pitch + kLabelX,
				kLabelBackgroundColor, kLabelWidth);
		drawText(pixels, screen->pitch, kLabelX, kLabelY,
			_labels[selectedSlider]);
		const uint filledTicks = _engine->getSettings()->getFilledTickCount(
			(RipperSettings::Slider)selectedSlider);
		for (uint tick = 0; tick < 10; ++tick) {
			for (int y = 0; y < kTickHeight; ++y)
				memset(pixels + (kTickY + y) * screen->pitch +
					kTickX + tick * kTickSpacing,
					tick < filledTicks ? kFilledTickColor : kEmptyTickColor,
					kTickWidth);
		}
		g_system->unlockScreen();
		debugC(2, kDebugGeneral,
			"Ripper: Remote Control selected slider=%u label='%s' value=%d ticks=%u",
			selectedSlider, _labels[selectedSlider].c_str(),
			_engine->getSettings()->getValue((RipperSettings::Slider)selectedSlider),
			filledTicks);
	};
	redrawSettings();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();

	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		uint16 command = _engine->getInput()->hasPendingKey() ?
			_engine->getInput()->consumeKey() : 0;
		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int newHoveredControl = findControl(mouse.position);
		if (newHoveredControl != hoveredControl) {
			hoveredControl = newHoveredControl;
			_engine->getCursor()->update(hoveredControl >= 0 ? kActiveCursor : kDefaultCursor);
			debugC(2, kDebugInput,
				"Ripper: Remote Control hover control=%d point=%d,%d",
				hoveredControl, mouse.position.x, mouse.position.y);
		}
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			pressedControl = hoveredControl;
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (pressedControl >= 0 && pressedControl == hoveredControl)
				command = _controls[pressedControl].command;
			pressedControl = -1;
		}
		if (command != 0) {
			const uint previousSlider = selectedSlider;
			active = handleCommand(command, selectedSlider);
			if (command == 0x3b00)
				hoveredControl = -2;
			if (active && (previousSlider != selectedSlider || command == 0x1300 ||
				command == 0x4800 || command == 0x5000 || command == '+' || command == '-'))
				redrawSettings();
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	_engine->getSettings()->save();
	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugGeneral, "Ripper: exited RunTake2IniSliderSetupMenu");
	return !_engine->shouldQuit();
}

} // End of namespace Ripper
