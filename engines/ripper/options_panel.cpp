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

} // End of anonymous namespace

OptionsPanelManager::OptionsPanelManager(RipperEngine *engine) :
		_engine(engine), _initialized(false) {
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

bool OptionsPanelManager::initialize(ResourceManager &resources) {
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

bool OptionsPanelManager::captureDisplay() {
	return _savedDisplay.capture();
}

void OptionsPanelManager::restoreDisplay() {
	_savedDisplay.restore(true, false);
	_savedDisplay.clear();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void OptionsPanelManager::applyPalette() {
	byte palette[256 * 3];
	memcpy(palette, _background.palette.data(), sizeof(palette));
	_engine->getToolbar()->applySharedPalettePatch(palette, 256);
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
	for (uint row = 0; row < frame.height && y + (int)row < screen->h; ++row) {
		if (y + (int)row < 0)
			continue;
		byte *destination = (byte *)screen->getBasePtr(0, y + row);
		const byte *source = frame.pixels.data() + row * frame.width;
		for (uint column = 0; column < frame.width && x + (int)column < screen->w; ++column) {
			if (x + (int)column >= 0 && source[column] != frame.transparentColor)
				destination[x + column] = source[column];
		}
	}
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
	if ((command & 0xff00) == 0 && (command & 0xff) >= 0x20 && (command & 0xff) <= 0x7e) {
		char character = command & 0xff;
		if (character >= 'a' && character <= 'z')
			character -= 'a' - 'A';
		return Common::String(character);
	}
	if ((command & 0xff00) >= 0x3b00 && (command & 0xff00) <= 0x4400)
		return Common::String::format("F%u", ((command >> 8) - 0x3b) + 1);
	switch (command) {
	case 0x0008: return "BACKSPACE";
	case 0x0009: return "TAB";
	case 0x000d: return "ENTER";
	case 0x001b: return "ESC";
	case 0x4700: return "HOME";
	case 0x4800: return "UP";
	case 0x4900: return "PAGE UP";
	case 0x4b00: return "LEFT";
	case 0x4d00: return "RIGHT";
	case 0x4f00: return "END";
	case 0x5000: return "DOWN";
	case 0x5100: return "PAGE DOWN";
	case 0x5200: return "INSERT";
	case 0x5300: return "DELETE";
	default: return Common::String::format("0x%04X", command);
	}
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

bool OptionsPanelManager::run() {
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
			command = 0;
		} else if (captureSlot < 0 && command == 0x1b) {
			active = false;
			command = 0;
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
				if (selected <= 9)
					drawState(state);
			}
			pressedControl = -1;
		}

		if (!serviceBackground(decoder))
			active = false;
		_engine->getCursor()->refresh();
		g_system->updateScreen();
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
