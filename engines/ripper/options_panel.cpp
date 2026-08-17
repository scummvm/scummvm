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

#include "ripper/options_panel.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kDefaultCursor = 14;
static const uint kActiveCursor = 16;
static const uint kBusyCursor = 19;
static const uint kHelpResource = 0x191;
static const uint kDemoHelpResource = 0xfb;
static const int kAnimationX = 544;
static const int kAnimationY = 44;
static const int kKeyLabelCenterX = 505;
static const int kKeyLabelY = 295;
static const int kKeyLabelLeft = 430;
static const int kKeyLabelTop = 286;
static const int kKeyLabelWidth = 110;
static const int kKeyLabelHeight = 18;
static const byte kKeyLabelColor = 255;

struct ControlLayout {
	int x;
	int y;
	int width;
	int height;
};

// g_optionsMenuControlLayoutRows at 0x1be40 stores ordinary controls in the
// original display service's vertical/horizontal order. RunOptionsMenu at
// 0x1c001 supplies the two special rows directly.
static const ControlLayout kControlLayouts[] = {
	{ 77, 71, 23, 16 },
	{ 103, 71, 23, 16 },
	{ 142, 108, 94, 23 },
	{ 142, 142, 94, 23 },
	{ 142, 178, 94, 23 },
	{ 142, 213, 94, 23 },
	{ 261, 348, 40, 32 },
	{ 330, 60, 36, 28 },
	{ 406, 60, 36, 28 },
	{ 359, 112, 56, 19 },
	{ 553, 48, 38, 28 },
	{ 553, 81, 38, 28 },
	{ 553, 116, 38, 28 },
	{ 553, 151, 38, 28 },
	{ 553, 184, 38, 28 },
	{ 553, 220, 38, 28 },
	{ 553, 254, 38, 28 },
	{ 553, 290, 38, 28 },
	{ 551, 328, 50, 40 }
};

static const uint kActionKeySlots[] = { 0, 1, 2, 3, 4, 5, 6, 8 };

// g_take2IniSliderSetupLayout at 0x62208 is stored as vertical/horizontal
// coordinate pairs. RunTake2IniSliderSetupMenu at 0x17fc0 creates six toggle,
// three slider, eight hotkey, and four fixed-text controls from these rows.
static const ControlLayout kDemoControlLayouts[] = {
	{ 30, 30, 15, 15 },
	{ 30, 60, 15, 15 },
	{ 30, 90, 15, 15 },
	{ 30, 120, 15, 15 },
	{ 220, 30, 15, 15 },
	{ 220, 90, 15, 15 },
	{ 248, 50, 121, 22 },
	{ 248, 110, 121, 22 },
	{ 248, 170, 121, 22 },
	{ 26, 331, 72, 52 },
	{ 99, 331, 72, 52 },
	{ 172, 331, 72, 52 },
	{ 245, 331, 72, 52 },
	{ 318, 331, 72, 52 },
	{ 391, 331, 72, 52 },
	{ 464, 331, 72, 52 },
	{ 537, 331, 72, 52 },
	{ 500, 110, 100, 20 },
	{ 500, 150, 100, 20 },
	{ 500, 190, 100, 20 },
	{ 500, 230, 100, 20 }
};

static const uint16 kDemoFixedControlIds[] = {
	0x000d, 0x001b, 0x2000, 0x3b00
};

static const uint16 kDemoDefaultActionKeys[] = {
	0x1100, 0x1700, 0x0077, 0x1f00,
	0x1300, 0x1800, 0x3b00, 0x1000
};

} // End of anonymous namespace

OptionsPanelManager::OptionsPanelManager(RipperEngine *engine) :
		_engine(engine), _demoVariant(false), _initialized(false) {
}

bool OptionsPanelManager::loadFrameSet(ResourceManager &resources,
		const char *prefix, uint count, Common::Array<BitmapAssetFrame> &frames) {
	frames.clear();
	for (uint i = 0; i < count; ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadOptionsBitmapSequence(
				Common::String::format("%s%u", prefix, i), sequence) ||
				sequence.frames.empty())
			return false;
		frames.push_back(Common::move(sequence.frames.front()));
	}
	return true;
}

bool OptionsPanelManager::initialize(ResourceManager &resources, bool demoVariant) {
	_demoVariant = demoVariant;
	return demoVariant ? initializeDemo(resources) : initializeRetail(resources);
}

bool OptionsPanelManager::initializeRetail(ResourceManager &resources) {
	if (!resources.loadOptionsPcx("bg.pcx", _background) ||
			!resources.loadInterfaceBitmapFont("7pt_font.fnt", _font) ||
			!loadFrameSet(resources, "onoff", 9, _toggleFrames) ||
			!loadFrameSet(resources, "slide", 25, _videoModeFrames) ||
			!loadFrameSet(resources, "diff", 18, _combatFrames) ||
			!loadFrameSet(resources, "pdiff", 26, _puzzleFrames))
		return false;

	static const char *const accentNames[] = { "opt_yel", "opt_p1", "opt_bhk" };
	for (uint i = 0; i < ARRAYSIZE(accentNames); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadOptionsBitmapSequence(accentNames[i], sequence) ||
				sequence.frames.empty())
			return false;
		_accentFrames.push_back(Common::move(sequence.frames.front()));
	}

	_controls.clear();
	for (uint i = 0; i < ARRAYSIZE(kControlLayouts); ++i) {
		const ControlLayout &layout = kControlLayouts[i];
		_controls.add(Common::Rect(layout.x, layout.y,
			layout.x + layout.width, layout.y + layout.height), 1000 + i);
		debugC(2, kDebugResources,
			"Ripper: Options Panel control=%u id=%u rect=%d,%d,%d,%d",
			i, _controls[i].action, layout.x, layout.y, layout.width, layout.height);
	}

	_initialized = _background.width == 640 && _background.height == 400 &&
		_background.palette.size() >= 256 * 3;
	if (!_initialized)
		warning("Ripper: invalid Options Panel background size=%ux%u paletteBytes=%u",
			_background.width, _background.height, _background.palette.size());
	else
		debugC(1, kDebugGeneral,
			"Ripper: initialized Options Panel controls=%u actionKeys=%u background=%ux%u",
			_controls.size(), ARRAYSIZE(kActionKeySlots), _background.width,
			_background.height);
	return _initialized;
}

bool OptionsPanelManager::initializeDemo(ResourceManager &resources) {
	Common::Array<Common::String> gameText;
	if (!resources.loadInterfacePcx("options.pcx", _background) ||
			!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!resources.loadGameText(gameText) || gameText.size() < 0x41)
		return false;

	_demoFrames.clear();
	for (uint i = 1; i <= 11; ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("option%u.bbm", i), sequence) ||
				sequence.frames.empty())
			return false;
		_demoFrames.push_back(Common::move(sequence.frames.front()));
	}

	_demoButtonLabels.clear();
	for (uint resourceId = 0x1b; resourceId <= 0x1e; ++resourceId)
		_demoButtonLabels.push_back(gameText[resourceId - 1]);
	_demoCaptureLabels.clear();
	for (uint resourceId = 0x13; resourceId <= 0x1a; ++resourceId)
		_demoCaptureLabels.push_back(gameText[resourceId - 1]);
	_demoCapturePrompt = gameText[0x41 - 1];

	_controls.clear();
	for (uint i = 0; i < ARRAYSIZE(kDemoControlLayouts); ++i) {
		const ControlLayout &layout = kDemoControlLayouts[i];
		uint16 action = 0;
		if (i < 17)
			action = 0x546 + i;
		else
			action = kDemoFixedControlIds[i - 17];
		_controls.add(Common::Rect(layout.x, layout.y,
			layout.x + layout.width, layout.y + layout.height), action);
		debugC(2, kDebugResources,
			"Ripper: demo Options control=%u id=0x%04x rect=%d,%d,%d,%d",
			i, action, layout.x, layout.y, layout.width, layout.height);
	}

	_initialized = _background.width == 640 && _background.height == 400 &&
		_background.palette.size() >= 256 * 3 && _demoFrames.size() == 11;
	if (!_initialized) {
		warning("Ripper: invalid demo Options assets background=%ux%u paletteBytes=%u frames=%u",
			_background.width, _background.height, _background.palette.size(),
			_demoFrames.size());
	} else {
		debugC(1, kDebugGeneral,
			"Ripper: initialized demo Options Panel function=RunTake2IniSliderSetupMenu@0x17fc0 controls=%u frames=%u",
			_controls.size(), _demoFrames.size());
	}
	return _initialized;
}

bool OptionsPanelManager::captureDisplay() {
	return _savedDisplay.capture();
}

void OptionsPanelManager::restoreDisplay() {
	_savedDisplay.restore(true, false);
	_savedDisplay.clear();
	_engine->getCursor()->refresh();
	presentScreen();
}

void OptionsPanelManager::applyPalette() {
	byte palette[256 * 3];
	memcpy(palette, _background.palette.data(), sizeof(palette));
	_engine->applySharedPalettePatch(palette, 256);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void OptionsPanelManager::drawBackground() {
	g_system->copyRectToScreen(_background.pixels.data(), _background.width,
		0, 0, _background.width, _background.height);
}

void OptionsPanelManager::drawBitmap(const BitmapAssetFrame &frame, int x, int y) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(), screen->pitch,
		frame, x, y, Common::Rect(0, 0, screen->w, screen->h));
	g_system->unlockScreen();
}

void OptionsPanelManager::drawState(const State &state) {
	// RenderOptionsStateStripSelections at 0x1d935 selects these fixed anchor
	// frames from the four strip animations.
	drawBitmap(_toggleFrames[state.bufferedVideo ? 8 : 0], 68, 64);
	drawBitmap(_videoModeFrames[state.videoMode * 8], 54, 90);
	drawBitmap(_combatFrames[(state.combatLevel - 1) * 6], 125, 340);
	uint puzzleFrame = 0;
	if (state.puzzleLevel == 3)
		puzzleFrame = 8;
	else if (state.puzzleLevel == 2)
		puzzleFrame = 16;
	drawBitmap(_puzzleFrames[puzzleFrame], 353, 62);
}

uint OptionsPanelManager::measureText(const Common::String &text) const {
	return BitmapFontRenderer::measureText(_font, text);
}

void OptionsPanelManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text, color);
}

Common::String OptionsPanelManager::formatKeyLabel(uint16 command) const {
	return formatKeyCommandLabel(command);
}

void OptionsPanelManager::clearKeyLabel() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kKeyLabelHeight; ++y)
		memcpy(screen->getBasePtr(kKeyLabelLeft, kKeyLabelTop + y),
			_background.pixels.data() + (kKeyLabelTop + y) * 640 + kKeyLabelLeft,
			kKeyLabelWidth);
	g_system->unlockScreen();
}

void OptionsPanelManager::drawKeyLabel(uint16 command) {
	clearKeyLabel();
	const Common::String label = formatKeyLabel(command);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	const int x = kKeyLabelCenterX - measureText(label) / 2;
	drawText((byte *)screen->getPixels(), screen->pitch, x, kKeyLabelY,
		label, kKeyLabelColor);
	g_system->unlockScreen();
}

int OptionsPanelManager::findControl(const Common::Point &point) const {
	// FindUiControlStateAtPoint at 0x4aae8 returns the first rectangular hit in
	// insertion order; the options controls do not use per-pixel hit masks.
	return _controls.findFirst(point);
}

bool OptionsPanelManager::serviceBackground(Video::SmackerDecoder &decoder) {
	if (decoder.endOfVideo()) {
		if (!decoder.rewind()) {
			warning("Ripper: could not rewind Options Panel animation 'opt.smk'");
			return false;
		}
		debugC(3, kDebugVideo, "Ripper: rewound Options Panel animation 'opt.smk'");
	}
	if (!decoder.needsUpdate())
		return true;
	const Graphics::Surface *frame = decoder.decodeNextFrame();
	if (frame)
		g_system->copyRectToScreen(frame->getPixels(), frame->pitch,
			kAnimationX, kAnimationY, frame->w, frame->h);
	debugC(11, kDebugVideo, "Ripper: Options Panel animation frame=%d",
		decoder.getCurFrame());
	return true;
}

void OptionsPanelManager::playControlSound(uint controlIndex) {
	uint soundIndex = 7;
	if (controlIndex <= 1)
		soundIndex = 0;
	else if (controlIndex <= 5)
		soundIndex = 2;
	else if (controlIndex == 6)
		soundIndex = 1;
	else if (controlIndex <= 9)
		soundIndex = 3;
	else if (controlIndex <= 17)
		soundIndex = 4;
	_engine->getMedia()->playSoundEffect(
		Common::String::format("opt_%u.wav", soundIndex), _soundHandle);
}

void OptionsPanelManager::commitState(const State &state) {
	RipperSettings *settings = _engine->getSettings();
	settings->setBufferedVideo(state.bufferedVideo);
	settings->setVideoMode(state.videoMode);
	settings->setCombatLevel(state.combatLevel);
	settings->setPuzzleLevel(state.puzzleLevel);
	for (uint i = 0; i < RipperSettings::kActionKeyCount; ++i)
		settings->setActionKey(i, state.actionKeys[i]);
	settings->save();
	debugC(1, kDebugGeneral,
		"Ripper: committed Options Panel settings bufferedVideo=%d videoMode=%u combat=%u puzzle=%u",
		state.bufferedVideo, state.videoMode, state.combatLevel, state.puzzleLevel);
}

void OptionsPanelManager::drawDemoState(const DemoState &state, int captureSlot) {
	drawBackground();

	for (uint i = 0; i < RipperSettings::kDemoToggleCount; ++i) {
		const ControlLayout &layout = kDemoControlLayouts[i];
		// OPTION1 and OPTION11 are the checked and unchecked 15x15 images.
		drawBitmap(_demoFrames[state.toggles[i] ? 0 : 10], layout.x, layout.y);
	}

	const BitmapAssetFrame &knob = _demoFrames[1];
	for (uint i = 0; i < RipperSettings::kDemoSliderCount; ++i) {
		const ControlLayout &layout = kDemoControlLayouts[6 + i];
		const int travel = layout.width - knob.width;
		const int x = layout.x + (state.sliders[i] * travel + 50) / 100;
		drawBitmap(knob, x, layout.y);
	}

	for (uint i = 0; i < RipperSettings::kActionKeyCount - 1; ++i) {
		const ControlLayout &layout = kDemoControlLayouts[9 + i];
		// OPTION3 through OPTION10 are the eight toolbar-action hotkey images.
		drawBitmap(_demoFrames[2 + i], layout.x, layout.y);
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint i = 0; i < RipperSettings::kActionKeyCount - 1; ++i) {
		const ControlLayout &layout = kDemoControlLayouts[9 + i];
		const Common::String label = formatKeyLabel(state.actionKeys[i]);
		const int x = layout.x + (layout.width - measureText(label)) / 2;
		drawText(pixels, screen->pitch, x, layout.y - 19, label, 0);
	}
	for (uint i = 0; i < _demoButtonLabels.size(); ++i) {
		const ControlLayout &layout = kDemoControlLayouts[17 + i];
		Common::String label = _demoButtonLabels[i];
		if (!label.empty() && label[0] == '^')
			label = label.substr(1);
		const int x = layout.x + (layout.width - measureText(label)) / 2;
		const int y = layout.y + (layout.height - _font.lineHeight) / 2;
		drawText(pixels, screen->pitch, x, y, label, 0);
	}
	if (captureSlot >= 0 && (uint)captureSlot < _demoCaptureLabels.size()) {
		const Common::String prompt = Common::String::format("%s: %s",
			_demoCapturePrompt.c_str(), _demoCaptureLabels[captureSlot].c_str());
		const int x = (640 - measureText(prompt)) / 2;
		drawText(pixels, screen->pitch, x, 270, prompt, 0);
	}
	g_system->unlockScreen();
}

void OptionsPanelManager::commitDemoState(const DemoState &state) {
	RipperSettings *settings = _engine->getSettings();
	for (uint i = 0; i < RipperSettings::kDemoToggleCount; ++i)
		settings->setDemoToggle((RipperSettings::DemoToggle)i, state.toggles[i]);
	for (uint i = 0; i < RipperSettings::kDemoSliderCount; ++i)
		settings->setDemoSlider((RipperSettings::DemoSlider)i, state.sliders[i]);
	for (uint i = 0; i < RipperSettings::kActionKeyCount - 1; ++i)
		settings->setActionKey(i + 1, state.actionKeys[i]);
	settings->save();
	debugC(1, kDebugGeneral,
		"Ripper: committed demo Options settings subtitles=%d bufferedVideo=%d "
		"toolbarHelp=%d toolbarPermanent=%d sound=%d music=%d mouse=%d",
		state.toggles[RipperSettings::kDemoSubtitles],
		state.toggles[RipperSettings::kDemoBufferedVideo],
		state.toggles[RipperSettings::kDemoToolbarHelp],
		state.toggles[RipperSettings::kDemoToolbarPermanent],
		state.sliders[RipperSettings::kDemoSoundVolume],
		state.sliders[RipperSettings::kDemoMusicVolume],
		state.sliders[RipperSettings::kDemoMouseSensitivity]);
}

void OptionsPanelManager::resetDemoState(DemoState &state) const {
	static const bool defaultToggles[] = { true, false, true, false, true, true };
	static const int defaultSliders[] = { 100, 50, 100 };
	memcpy(state.toggles, defaultToggles, sizeof(state.toggles));
	memcpy(state.sliders, defaultSliders, sizeof(state.sliders));
	memcpy(state.actionKeys, kDemoDefaultActionKeys, sizeof(state.actionKeys));
}

int OptionsPanelManager::demoSliderValueFromPoint(uint slider, int x) const {
	if (slider >= RipperSettings::kDemoSliderCount)
		return 0;
	const ControlLayout &layout = kDemoControlLayouts[6 + slider];
	const int travel = layout.width - _demoFrames[1].width;
	return travel > 0 ? CLIP<int>(((x - layout.x) * 100 + travel / 2) / travel,
		0, 100) : 0;
}

bool OptionsPanelManager::runDemo() {
	if (!_initialized || !captureDisplay())
		return false;

	RipperSettings *settings = _engine->getSettings();
	DemoState state;
	for (uint i = 0; i < RipperSettings::kDemoToggleCount; ++i)
		state.toggles[i] = settings->getDemoToggle((RipperSettings::DemoToggle)i);
	for (uint i = 0; i < RipperSettings::kDemoSliderCount; ++i)
		state.sliders[i] = settings->getDemoSlider((RipperSettings::DemoSlider)i);
	for (uint i = 0; i < RipperSettings::kActionKeyCount - 1; ++i)
		state.actionKeys[i] = settings->getActionKey(i + 1);
	const int originalMusicVolume =
		settings->getDemoSlider(RipperSettings::kDemoMusicVolume);

	debugC(1, kDebugGeneral,
		"Ripper: entering demo Options function=RunTake2IniSliderSetupMenu@0x17fc0 controls=%u sound=%d music=%d mouse=%d",
		_controls.size(), state.sliders[RipperSettings::kDemoSoundVolume],
		state.sliders[RipperSettings::kDemoMusicVolume],
		state.sliders[RipperSettings::kDemoMouseSensitivity]);
	// The demo installs cursor rows 0..17 and selects row 14 on entry.
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	drawDemoState(state);
	applyPalette();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getInput()->drainKeys();
	_engine->getInput()->publishMouseState();
	_engine->getInput()->discardMouseTransitions();
	presentScreen();

	int hoveredControl = -1;
	int pressedControl = -1;
	int captureSlot = -1;
	bool committed = false;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		uint16 command = _engine->getInput()->hasPendingKey() ?
			_engine->getInput()->consumeKey() : 0;
		bool redraw = false;
		if (captureSlot >= 0 && command != 0) {
			if (command == 0x001b) {
				debugC(2, kDebugInput,
					"Ripper: demo Options key capture cancelled slot=%d", captureSlot);
			} else {
				state.actionKeys[captureSlot] = command;
				debugC(1, kDebugInput,
					"Ripper: demo Options captured hotkey=%d command=0x%04x label='%s'",
					captureSlot, command, formatKeyLabel(command).c_str());
			}
			captureSlot = -1;
			command = 0;
			redraw = true;
		}

		if (captureSlot < 0 && command != 0) {
			if (command == 0x000d) {
				committed = true;
				active = false;
			} else if (command == 0x001b) {
				active = false;
			} else if (command == 0x2000) {
				resetDemoState(state);
				settings->setDemoSlider(RipperSettings::kDemoMusicVolume,
					state.sliders[RipperSettings::kDemoMusicVolume]);
				redraw = true;
				debugC(2, kDebugGeneral,
					"Ripper: demo Options restored staged SETTINGS.DEF defaults");
			} else if (command == 0x3b00) {
				debugC(1, kDebugGeneral,
					"Ripper: demo Options opening help resource=0x%x", kDemoHelpResource);
				_engine->getModalDialog()->run(kDemoHelpResource);
				redraw = true;
			} else {
				for (uint i = 0; i < RipperSettings::kActionKeyCount - 1; ++i) {
					if (state.actionKeys[i] == command) {
						captureSlot = i;
						redraw = true;
						break;
					}
				}
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int newHoveredControl = captureSlot < 0 ? findControl(mouse.position) : -1;
		if (newHoveredControl != hoveredControl) {
			hoveredControl = newHoveredControl;
			_engine->getCursor()->update(hoveredControl >= 0 ? kActiveCursor : kDefaultCursor);
			debugC(2, kDebugInput,
				"Ripper: demo Options hover control=%d id=0x%04x point=%d,%d captureSlot=%d",
				hoveredControl,
				hoveredControl >= 0 ? _controls[hoveredControl].action : 0,
				mouse.position.x, mouse.position.y, captureSlot);
		}

		if (captureSlot < 0 && (mouse.pressed & kMouseButtonLeft) != 0) {
			pressedControl = hoveredControl;
			if (pressedControl >= 6 && pressedControl <= 8) {
				const uint slider = pressedControl - 6;
				state.sliders[slider] = demoSliderValueFromPoint(slider, mouse.position.x);
				if (slider == RipperSettings::kDemoMusicVolume)
					settings->setDemoSlider(RipperSettings::kDemoMusicVolume,
						state.sliders[slider]);
				redraw = true;
			}
		}
		if (captureSlot < 0 && pressedControl >= 6 && pressedControl <= 8 &&
				(mouse.buttons & kMouseButtonLeft) != 0) {
			const uint slider = pressedControl - 6;
			const int value = demoSliderValueFromPoint(slider, mouse.position.x);
			if (state.sliders[slider] != value) {
				state.sliders[slider] = value;
				if (slider == RipperSettings::kDemoMusicVolume)
					settings->setDemoSlider(RipperSettings::kDemoMusicVolume, value);
				redraw = true;
			}
		}
		if (captureSlot < 0 && (mouse.released & kMouseButtonLeft) != 0) {
			if (pressedControl >= 0 && pressedControl == hoveredControl) {
				const uint selected = pressedControl;
				debugC(1, kDebugInput,
					"Ripper: demo Options selected control=%u id=0x%04x point=%d,%d",
					selected, _controls[selected].action,
					mouse.position.x, mouse.position.y);
				if (selected < 6) {
					state.toggles[selected] = !state.toggles[selected];
					redraw = true;
				} else if (selected < 9) {
					redraw = true;
				} else if (selected < 17) {
					captureSlot = selected - 9;
					redraw = true;
				} else if (selected == 17) {
					committed = true;
					active = false;
				} else if (selected == 18) {
					active = false;
				} else if (selected == 19) {
					resetDemoState(state);
					settings->setDemoSlider(RipperSettings::kDemoMusicVolume,
						state.sliders[RipperSettings::kDemoMusicVolume]);
					redraw = true;
				} else if (selected == 20) {
					_engine->getModalDialog()->run(kDemoHelpResource);
					redraw = true;
				}
			}
			pressedControl = -1;
		}

		if (redraw)
			drawDemoState(state, captureSlot);
		_engine->getCursor()->refresh();
		presentScreen();
		g_system->delayMillis(10);
	}

	if (!_engine->shouldQuit() && committed)
		commitDemoState(state);
	else
		settings->setDemoSlider(RipperSettings::kDemoMusicVolume, originalMusicVolume);
	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugGeneral,
		"Ripper: exited demo RunTake2IniSliderSetupMenu committed=%d quit=%d",
		committed, _engine->shouldQuit());
	return !_engine->shouldQuit();
}

bool OptionsPanelManager::run() {
	return _demoVariant ? runDemo() : runRetail();
}

bool OptionsPanelManager::runRetail() {
	if (!_initialized || !captureDisplay())
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> animationStream(
		_engine->getResources()->options().createReadStreamForMember("opt.smk"));
	Video::SmackerDecoder decoder;
	if (!animationStream || !decoder.loadStream(animationStream.release())) {
		warning("Ripper: could not load Options Panel animation 'opt.smk'");
		restoreDisplay();
		return false;
	}

	RipperSettings *settings = _engine->getSettings();
	State state;
	state.bufferedVideo = settings->getBufferedVideo();
	state.videoMode = settings->getVideoMode();
	state.combatLevel = settings->getCombatLevel();
	state.puzzleLevel = settings->getPuzzleLevel();
	for (uint i = 0; i < RipperSettings::kActionKeyCount; ++i)
		state.actionKeys[i] = settings->getActionKey(i);

	debugC(1, kDebugGeneral,
		"Ripper: entering RunOptionsMenu controls=%u bufferedVideo=%d videoMode=%u combat=%u puzzle=%u",
		_controls.size(), state.bufferedVideo, state.videoMode,
		state.combatLevel, state.puzzleLevel);
	_engine->getCursor()->update(kBusyCursor);
	_engine->getCursor()->setVisible(true);
	drawBackground();
	applyPalette();
	drawState(state);
	decoder.start();
	serviceBackground(decoder);
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getInput()->drainKeys();
	_engine->getInput()->publishMouseState();
	_engine->getInput()->discardMouseTransitions();

	int hoveredControl = -1;
	int pressedControl = -1;
	int captureSlot = -1;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		uint16 command = _engine->getInput()->hasPendingKey() ?
			_engine->getInput()->consumeKey() : 0;
		if (captureSlot >= 0 && command != 0) {
			if (command == 0x1b) {
				debugC(2, kDebugInput,
					"Ripper: Options Panel key capture cancelled slot=%d", captureSlot);
			} else {
				state.actionKeys[captureSlot] = command;
				drawKeyLabel(command);
				debugC(1, kDebugInput,
					"Ripper: Options Panel captured actionKeySlot=%d command=0x%04x label='%s'",
					captureSlot, command, formatKeyLabel(command).c_str());
			}
			captureSlot = -1;
			command = 0;
		}

		if (captureSlot < 0 && command == 0x3b00) {
			debugC(1, kDebugGeneral,
				"Ripper: Options Panel opening help resource=0x%x", kHelpResource);
			_engine->getModalDialog()->run(kHelpResource);
		} else if (captureSlot < 0 && command == 0x1b) {
			active = false;
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int newHoveredControl = findControl(mouse.position);
		if (newHoveredControl != hoveredControl) {
			hoveredControl = newHoveredControl;
			_engine->getCursor()->update(hoveredControl >= 0 ? kActiveCursor : kDefaultCursor);
			if (captureSlot < 0) {
				if (hoveredControl >= 10 && hoveredControl <= 17)
					drawKeyLabel(state.actionKeys[kActionKeySlots[hoveredControl - 10]]);
				else
					clearKeyLabel();
			}
			debugC(2, kDebugInput,
				"Ripper: Options Panel hover control=%d id=%u point=%d,%d captureSlot=%d",
				hoveredControl, hoveredControl >= 0 ? _controls[hoveredControl].action : 0,
				mouse.position.x, mouse.position.y, captureSlot);
		}
		if (captureSlot < 0 && (mouse.pressed & kMouseButtonLeft) != 0)
			pressedControl = hoveredControl;
		if (captureSlot < 0 && (mouse.released & kMouseButtonLeft) != 0) {
			if (pressedControl >= 0 && pressedControl == hoveredControl) {
				const uint selected = pressedControl;
				const uint previousCombatLevel = state.combatLevel;
				const uint previousPuzzleLevel = state.puzzleLevel;
				playControlSound(selected);
				debugC(1, kDebugInput,
					"Ripper: Options Panel selected control=%u id=%u point=%d,%d",
					selected, _controls[selected].action, mouse.position.x, mouse.position.y);
				if (selected <= 1)
					state.bufferedVideo = !state.bufferedVideo;
				else if (selected <= 5)
					state.videoMode = selected - 2;
				else if (selected == 6)
					state.combatLevel = state.combatLevel == 3 ? 1 : state.combatLevel + 1;
				else if (selected == 7)
					state.puzzleLevel = 1;
				else if (selected == 8)
					state.puzzleLevel = 3;
				else if (selected == 9)
					state.puzzleLevel = 2;
				else if (selected <= 17) {
					captureSlot = kActionKeySlots[selected - 10];
					drawKeyLabel(state.actionKeys[captureSlot]);
					debugC(1, kDebugInput,
						"Ripper: Options Panel awaiting key actionKeySlot=%d current=0x%04x",
						captureSlot, state.actionKeys[captureSlot]);
				} else {
					active = false;
				}
				if (selected == 6)
					debugC(2, kDebugGeneral,
						"Ripper: Options Panel staged combat difficulty previous=%u new=%u changed=%d commit=on-exit",
						previousCombatLevel, state.combatLevel,
						previousCombatLevel != state.combatLevel);
				else if (selected >= 7 && selected <= 9)
					debugC(2, kDebugGeneral,
						"Ripper: Options Panel staged puzzle difficulty control=%u previous=%u new=%u changed=%d commit=on-exit",
						selected, previousPuzzleLevel, state.puzzleLevel,
						previousPuzzleLevel != state.puzzleLevel);
				if (selected <= 9)
					drawState(state);
			}
			pressedControl = -1;
		}

		if (!serviceBackground(decoder))
			active = false;
		_engine->getCursor()->refresh();
		presentScreen();
		g_system->delayMillis(MIN<uint32>(decoder.getTimeToNextFrame(), 10));
	}

	_engine->getMedia()->stopSoundEffect(_soundHandle);
	if (!_engine->shouldQuit())
		commitState(state);
	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	debugC(1, kDebugGeneral, "Ripper: exited RunOptionsMenu committed=%d",
		!_engine->shouldQuit());
	return !_engine->shouldQuit();
}

} // End of namespace Ripper
