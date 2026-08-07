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
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/modal_dialog.h"
#include "ripper/options_panel.h"
#include "ripper/remote_control.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/wac/wac.h"

namespace Ripper {

static const uint kToolbarActionCount = 9;
static const int kToolbarActivationHeight = 50;
static const int kToolbarRightEdge = 630;
static const int kToolbarActionGap = 5;
static const uint32 kDosTickMillis = 55;
static const uint32 kPreviewDelayMillis = 27 * kDosTickMillis;
static const byte kToolbarBorderColor = 0;
static const byte kToolbarTextColor = 4;
static const byte kToolbarFillColor = 253;
static const uint kToolbarHelpActionIndex = 7;
static const uint kToolbarExitActionIndex = 8;
static const uint kToolbarExitPromptResourceId = 0x3f;

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

ToolbarManager::ToolbarManager(RipperEngine *engine) : _sessionStartMillis(0), _lastFrameMillis(0),
		_enabledActionMask(0), _hoveredAction(-1), _pressedAction(-1),
		_active(false), _previewEnabled(false),
		_engine(engine), _remoteControl(new RemoteControlManager(engine)),
		_optionsPanel(new OptionsPanelManager(engine)) {
}

ToolbarManager::~ToolbarManager() {
}

bool ToolbarManager::initialize(ResourceManager &resources) {
	Common::Array<Common::String> gameText;
	if (!resources.loadGameText(gameText) || gameText.size() < kToolbarActionCount ||
		!resources.loadInterfaceBitmapFont("7pt_font.fnt", _font) ||
		!_remoteControl->initialize(resources) ||
		!_optionsPanel->initialize(resources))
		return false;

	_actions.clear();
	_actions.resize(kToolbarActionCount);
	// InitializeSharedPresentationTemplates at 0x1196f captures the shared
	// interface palette while loading the startup indexed bitmap set. MNU0 is
	// the first menu bitmap in that set; keep its reserved interface bands
	// independent from scene/video palettes.
	BitmapAssetSequence menuPalette;
	if (!resources.loadInterfaceBitmapSequence("mnu0", menuPalette) ||
		menuPalette.frames.empty() || menuPalette.frames.front().palette.size() < 256 * 3) {
		warning("Ripper: startup interface palette MNU0 is unavailable");
		return false;
	}
	_sharedPalette = menuPalette.frames.front().palette;
	debugC(2, kDebugResources,
		"Ripper: captured shared interface palette from startup MNU0 (%u bytes)",
		_sharedPalette.size());
	for (uint i = 0; i < kToolbarActionCount; ++i) {
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("toolbar%u.pl", i + 1), _actions[i].sequence) ||
			_actions[i].sequence.frames.size() != 10) {
			warning("Ripper: invalid toolbar action asset %u", i + 1);
			return false;
		}
		_actions[i].label = gameText[i];
	}
	for (uint i = 0; i < kToolbarActionCount; ++i) {
		const BitmapAssetFrame &frame = _actions[i].sequence.frames[0];
		for (uint frameIndex = 1; frameIndex < _actions[i].sequence.frames.size(); ++frameIndex) {
			const BitmapAssetFrame &candidate = _actions[i].sequence.frames[frameIndex];
			if (candidate.width != frame.width || candidate.height != frame.height)
				return false;
		}
	}
	layoutActions((1 << kToolbarActionCount) - 1);

	debugC(1, kDebugScene,
		"Ripper: initialized front-end toolbar actions=%u activationHeight=%d previewTicks=27",
		_actions.size(), kToolbarActivationHeight);
	return true;
}

void ToolbarManager::applySharedPalettePatch(byte *palette, uint colorCount) {
	if (!palette || colorCount < 256)
		return;
	if (_sharedPalette.size() < 256 * 3)
		return;

	// ApplySharedDisplayPalettePatch at 0x205d0 reserves index 0, indices 4-9,
	// and indices 246-255 across every presentation palette.
	memset(palette, 0, 3);
	memcpy(palette + 4 * 3, _sharedPalette.data() + 4 * 3, 6 * 3);
	memcpy(palette + 246 * 3, _sharedPalette.data() + 246 * 3, 10 * 3);
}

void ToolbarManager::layoutActions(uint enabledActionMask) {
	// RunFrontEndActionMenu at 0x18b3a packs only the enabled controls from
	// right to left, subtracting each bitmap width and a five-pixel gap from
	// x=630, then centers the bitmap in the 50-pixel toolbar band.
	int x = kToolbarRightEdge;
	for (int i = kToolbarActionCount - 1; i >= 0; --i) {
		if ((enabledActionMask & (1 << i)) == 0) {
			_actions[i].bounds = Common::Rect();
			continue;
		}
		const BitmapAssetFrame &frame = _actions[i].sequence.frames[0];
		x -= frame.width + kToolbarActionGap;
		const int y = (kToolbarActivationHeight - frame.height) / 2;
		_actions[i].bounds = Common::Rect(x, y, x + frame.width, y + frame.height);
		debugC(3, kDebugScene,
			"Ripper: toolbar layout action=%u mask=0x%03x rect=%d,%d,%d,%d",
			i + 1, enabledActionMask, _actions[i].bounds.left, _actions[i].bounds.top,
			_actions[i].bounds.width(), _actions[i].bounds.height());
	}
}

void ToolbarManager::enter(uint32 now, uint enabledActionMask) {
	layoutActions(enabledActionMask);
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	applySharedPalettePatch(palette, 256);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w < 640 || screen->h < 400) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	_topBacking.resize(640 * kToolbarActivationHeight);
	for (int y = 0; y < kToolbarActivationHeight; ++y)
		memcpy(_topBacking.data() + y * 640, screen->getBasePtr(0, y), 640);
	g_system->unlockScreen();

	// RunFrontEndActionMenu at 0x18b3a constructs fresh UiControlState entries
	// from each toolbar resource's base pointer on every invocation, so the
	// initial dirty-region update always presents frame zero. Do not carry a
	// partially advanced hover animation into the next toolbar session.
	for (uint i = 0; i < _actions.size(); ++i)
		_actions[i].frameIndex = 0;

	_active = true;
	_enabledActionMask = enabledActionMask;
	_previewEnabled = false;
	_hoveredAction = -1;
	_pressedAction = -1;
	_sessionStartMillis = now;
	_lastFrameMillis = now;
	drawIcons();
	debugC(2, kDebugScene,
		"Ripper: entered toolbar input band y<%d enabledMask=0x%03x animationFrame=0",
		kToolbarActivationHeight, _enabledActionMask);
}

void ToolbarManager::leave() {
	if (!_active)
		return;
	removeTooltip();
	if (!_topBacking.empty())
		g_system->copyRectToScreen(_topBacking.data(), 640, 0, 0, 640, kToolbarActivationHeight);
	_topBacking.clear();
	_active = false;
	_enabledActionMask = 0;
	_previewEnabled = false;
	_hoveredAction = -1;
	_pressedAction = -1;
	debugC(2, kDebugScene, "Ripper: left toolbar input band");
}

int ToolbarManager::findAction(const Common::Point &point) const {
	for (uint i = 0; i < _actions.size(); ++i) {
		if ((_enabledActionMask & (1 << i)) != 0 && _actions[i].bounds.contains(point))
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
		if ((_enabledActionMask & (1 << i)) == 0)
			continue;
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
	return BitmapFontRenderer::measureText(_font, text);
}

void ToolbarManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text,
		kToolbarTextColor);
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
	// RunFrontEndActionMenu publishes the original coordinates in transposed
	// fields. RenderFrontEndActionPreviewSprite therefore adds 20 to the
	// vertical coordinate, placing the preview below the pointer.
	int x = point.x;
	int y = point.y + 20;
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
		memset(screen->getBasePtr(x, y + row), kToolbarFillColor, width);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (int column = 0; column < width; ++column) {
		pixels[y * screen->pitch + x + column] = kToolbarBorderColor;
		pixels[(y + height - 1) * screen->pitch + x + column] = kToolbarBorderColor;
	}
	for (int row = 0; row < height; ++row) {
		pixels[(y + row) * screen->pitch + x] = kToolbarBorderColor;
		pixels[(y + row) * screen->pitch + x + width - 1] = kToolbarBorderColor;
	}
	drawText(pixels, screen->pitch, x + 2, y + 2, label);
	g_system->unlockScreen();
}

void ToolbarManager::dispatchAction(uint actionIndex) {
	if (actionIndex == 0) {
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=1 id=0x514 label='%s' entering RunTake2IniSliderSetupMenu",
			_actions[actionIndex].label.c_str());
		if (!_remoteControl->run())
			warning("Ripper: Remote Control action failed");
		return;
	}
	if (actionIndex == kToolbarExitActionIndex && _engine->getScripts()->isCyberActive()) {
		// DispatchFrontEndAction at 0x190b7 bypasses the normal binary prompt
		// for action 0x51c while the Cyber transition flag is set.
		debugC(1, kDebugCyber,
			"Ripper: toolbar action=9 id=0x51c exiting Cyber nested runtime");
		leave();
		_engine->getScripts()->requestCyberExit("toolbar-exit");
		return;
	}
	if (actionIndex == 1) {
		debugC(1, kDebugScene,
			"Ripper: toolbar action=2 id=0x515 label='%s' entering HandleSceneSelectionAction",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->openWorldMap())
			warning("Ripper: toolbar world map action failed");
		return;
	}
	if (actionIndex == 2) {
		debugC(1, kDebugScene,
			"Ripper: toolbar action=3 id=0x516 label='%s' entering RunUnlockGatedSelectionMenu",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->openInventory())
			warning("Ripper: toolbar inventory action failed");
		return;
	}
	if (actionIndex == 3) {
		debugC(1, kDebugWac,
			"Ripper: toolbar action=4 id=0x517 label='%s' entering RunWacFrontEndLoop",
			_actions[actionIndex].label.c_str());
		_engine->getWac()->run();
		return;
	}
	if (actionIndex == 4 || actionIndex == 5) {
		const bool saving = actionIndex == 4;
		debugC(1, kDebugSaveLoad,
			"Ripper: toolbar action=%u id=0x%x label='%s' entering RunSaveRestoreSlotMenu mode=%s",
			actionIndex + 1, actionIndex + 0x514, _actions[actionIndex].label.c_str(),
			saving ? "save" : "restore");
		// RunFrontEndActionMenu at 0x18b3a removes its toolbar presentation before
		// DispatchFrontEndAction at 0x190b7 opens the modal slot menu.
		leave();
		_engine->getInput()->discardMouseTransitions();
		const bool completed = saving ? _engine->saveGameDialog() : _engine->loadGameDialog();
		_engine->getInput()->discardMouseTransitions();
		debugC(1, kDebugSaveLoad,
			"Ripper: toolbar RunSaveRestoreSlotMenu mode=%s completed=%d",
			saving ? "save" : "restore", completed);
		return;
	}
	if (actionIndex == 6) {
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=7 id=0x51a label='%s' entering RunOptionsMenu",
			_actions[actionIndex].label.c_str());
		// RunFrontEndActionMenu at 0x18b3a removes the toolbar presentation
		// before DispatchFrontEndAction at 0x190b7 enters the full-screen panel.
		leave();
		_engine->getInput()->discardMouseTransitions();
		if (!_optionsPanel->run())
			warning("Ripper: Options Panel action failed");
		_engine->getInput()->discardMouseTransitions();
		return;
	}
	if (actionIndex == kToolbarHelpActionIndex) {
		// DispatchFrontEndAction at 0x190b7 selects resource 0x19b while
		// SceneRuntime+0x189 bit 0x20 marks a prompt/chooser as active;
		// otherwise it presents the general help resource 400.
		debugC(1, kDebugScene,
			"Ripper: toolbar action=8 id=0x51b label='%s' entering "
			"RunModalSelectionTableDialogWithRestore",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->showHelp("toolbar"))
			warning("Ripper: toolbar help action failed");
		return;
	}
	if (actionIndex == kToolbarExitActionIndex) {
		// DispatchFrontEndAction at 0x190b7 resolves GAMETEXT.TF resource
		// 0x3f ("Quit Game?") and calls RunBinaryPromptChooser at 0x1803c
		// with "No" selected. The Cyber path above bypasses this prompt.
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=9 id=0x51c label='%s' entering "
			"RunBinaryPromptChooser",
			_actions[actionIndex].label.c_str());
		leave();
		_engine->getInput()->discardMouseTransitions();
		const bool confirmed = _engine->getModalDialog()->runBinaryPrompt(
			kToolbarExitPromptResourceId, false);
		_engine->getInput()->discardMouseTransitions();
		debugC(1, kDebugGeneral,
			"Ripper: toolbar quit prompt completed confirmed=%d quit=%d",
			confirmed, _engine->shouldQuit());
		if (confirmed)
			_engine->quitGame();
		return;
	}
	debugC(1, kDebugScene,
		"Ripper: toolbar action=%u id=0x%x label='%s' handler=%s is stubbed",
		actionIndex + 1, actionIndex + 0x514, _actions[actionIndex].label.c_str(),
		kToolbarHandlerNames[actionIndex]);
}

bool ToolbarManager::service(const MouseState &mouse, uint enabledActionMask,
		int *selectedAction) {
	if (mouse.position.y >= kToolbarActivationHeight) {
		leave();
		return false;
	}

	enabledActionMask &= (1 << kToolbarActionCount) - 1;
	const uint32 now = g_system->getMillis();
	if (_active && enabledActionMask != _enabledActionMask)
		leave();
	if (!_active)
		enter(now, enabledActionMask);
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
		const int action = _pressedAction;
		_pressedAction = -1;
		if (_hoveredAction == action) {
			if (selectedAction)
				*selectedAction = action;
			else
				dispatchAction(action);
			leave();
		} else {
			debugC(3, kDebugInput,
				"Ripper: toolbar primary release cancelled pressedAction=%u hoverAction=%d",
				action + 1, _hoveredAction < 0 ? 0 : _hoveredAction + 1);
		}
	}
	return true;
}

} // End of namespace Ripper
