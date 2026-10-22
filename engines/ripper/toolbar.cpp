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
#include "ripper/diagnostics/screen_presenter.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/modal_dialog.h"
#include "ripper/options_panel.h"
#include "ripper/remote_control.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/settings.h"
#include "ripper/wac/wac.h"

namespace Ripper {

static const uint kRetailToolbarActionCount = 9;
static const uint kDemoToolbarActionCount = 8;
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

ToolbarManager::ToolbarManager(RipperEngine *engine) : _sessionStartMillis(0), _lastFrameMillis(0),
		_enabledActionMask(0), _hoveredAction(-1), _pressedAction(-1),
		_active(false), _previewEnabled(false), _demoVariant(false),
		_engine(engine), _remoteControl(new RemoteControlManager(engine)),
		_optionsPanel(new OptionsPanelManager(engine)) {
}

ToolbarManager::~ToolbarManager() {
}

bool ToolbarManager::initialize(ResourceManager &resources, bool demoVariant) {
	_demoVariant = demoVariant;
	const uint actionCount = _demoVariant ? kDemoToolbarActionCount :
		kRetailToolbarActionCount;
	Common::Array<Common::String> gameText;
	if (!resources.loadGameText(gameText) || gameText.size() < actionCount ||
		!resources.loadInterfaceBitmapFont(
			_demoVariant ? "small.fnt" : "7pt_font.fnt", _font) ||
		(!_demoVariant && !_remoteControl->initialize(resources)) ||
		!_optionsPanel->initialize(resources, _demoVariant))
		return false;

	_actions.clear();
	_actions.resize(actionCount);
	for (uint i = 0; i < actionCount; ++i) {
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("toolbar%u.pl", i + 1), _actions[i].sequence) ||
			_actions[i].sequence.frames.size() != 10) {
			warning("Ripper: invalid toolbar action asset %u", i + 1);
			return false;
		}
		_actions[i].label = gameText[i];
	}
	for (uint i = 0; i < actionCount; ++i) {
		const BitmapAssetFrame &frame = _actions[i].sequence.frames[0];
		for (uint frameIndex = 1; frameIndex < _actions[i].sequence.frames.size(); ++frameIndex) {
			const BitmapAssetFrame &candidate = _actions[i].sequence.frames[frameIndex];
			if (candidate.width != frame.width || candidate.height != frame.height)
				return false;
		}
	}
	layoutActions((1 << actionCount) - 1);

	debugC(1, kDebugScene,
		"Ripper: initialized %s front-end toolbar actions=%u activationHeight=%d previewTicks=27",
		_demoVariant ? "demo" : "retail", _actions.size(), kToolbarActivationHeight);
	return true;
}

void ToolbarManager::layoutActions(uint enabledActionMask) {
	int x = kToolbarRightEdge;
	for (int i = (int)_actions.size() - 1; i >= 0; --i) {
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
	_engine->applySharedPalettePatch(palette, 256);
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

	// Do not carry a hover animation into the next toolbar session.
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
	if (_demoVariant) {
		if (actionIndex == 0) {
			debugC(1, kDebugScene,
				"Ripper: demo toolbar action=1 id=0x514 label='%s' opening world map",
				_actions[actionIndex].label.c_str());
			if (!_engine->getScripts()->openWorldMap())
				warning("Ripper: demo toolbar scene-selection action failed");
			return;
		}
		if (actionIndex == 1 || actionIndex == 6) {
			debugC(2, kDebugScene,
				"Ripper: demo toolbar action=%u id=0x%x label='%s' completed as no-op",
				actionIndex + 1, actionIndex + 0x514,
				_actions[actionIndex].label.c_str());
			return;
		}
		if (actionIndex == 2) {
			debugC(1, kDebugWac,
				"Ripper: demo toolbar action=3 id=0x516 label='%s' opening WAC",
				_actions[actionIndex].label.c_str());
			_engine->getWac()->run();
			return;
		}
		if (actionIndex == 3 || actionIndex == 4) {
			const bool saving = actionIndex == 3;
			debugC(1, kDebugSaveLoad,
				"Ripper: demo toolbar action=%u id=0x%x label='%s' opening save/restore dialog mode=%s",
				actionIndex + 1, actionIndex + 0x514,
				_actions[actionIndex].label.c_str(), saving ? "save" : "restore");
			leave();
			_engine->getInput()->discardMouseTransitions();
			const bool completed = saving ? _engine->saveGameDialog() :
				_engine->loadGameDialog();
			_engine->getInput()->discardMouseTransitions();
			debugC(1, kDebugSaveLoad,
				"Ripper: demo toolbar save/restore dialog mode=%s completed=%d",
				saving ? "save" : "restore", completed);
			return;
		}
		if (actionIndex == 5) {
			debugC(1, kDebugGeneral,
				"Ripper: demo toolbar action=6 id=0x519 label='%s' opening Options",
				_actions[actionIndex].label.c_str());
			leave();
			_engine->getInput()->discardMouseTransitions();
			if (!_optionsPanel->run())
				warning("Ripper: demo Options action failed");
			_engine->getInput()->discardMouseTransitions();
			return;
		}
		if (actionIndex == 7) {
			debugC(1, kDebugGeneral,
				"Ripper: demo toolbar action=8 id=0x51b label='%s' opening quit prompt",
				_actions[actionIndex].label.c_str());
			leave();
			_engine->getInput()->discardMouseTransitions();
			const bool confirmed = _engine->getModalDialog()->runBinaryPrompt(
				kToolbarExitPromptResourceId, false);
			_engine->getInput()->discardMouseTransitions();
			debugC(1, kDebugGeneral,
				"Ripper: demo toolbar quit prompt completed confirmed=%d quit=%d",
				confirmed, _engine->shouldQuit());
			if (confirmed)
				_engine->quitGame();
			return;
		}
	}

	if (actionIndex == 0) {
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=1 id=0x514 label='%s' opening Remote Control",
			_actions[actionIndex].label.c_str());
		if (!_remoteControl->run())
			warning("Ripper: Remote Control action failed");
		return;
	}
	if (actionIndex == kToolbarExitActionIndex && _engine->getScripts()->isCyberActive()) {
		debugC(1, kDebugCyber,
			"Ripper: toolbar action=9 id=0x51c exiting Cyber nested runtime");
		leave();
		_engine->getScripts()->requestCyberExit("toolbar-exit");
		return;
	}
	if (actionIndex == 1) {
		debugC(1, kDebugScene,
			"Ripper: toolbar action=2 id=0x515 label='%s' opening world map",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->openWorldMap())
			warning("Ripper: toolbar world map action failed");
		return;
	}
	if (actionIndex == 2) {
		debugC(1, kDebugScene,
			"Ripper: toolbar action=3 id=0x516 label='%s' opening inventory",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->openInventory())
			warning("Ripper: toolbar inventory action failed");
		return;
	}
	if (actionIndex == 3) {
		debugC(1, kDebugWac,
			"Ripper: toolbar action=4 id=0x517 label='%s' opening WAC",
			_actions[actionIndex].label.c_str());
		_engine->getWac()->run();
		return;
	}
	if (actionIndex == 4 || actionIndex == 5) {
		const bool saving = actionIndex == 4;
		debugC(1, kDebugSaveLoad,
			"Ripper: toolbar action=%u id=0x%x label='%s' opening save/restore dialog mode=%s",
			actionIndex + 1, actionIndex + 0x514, _actions[actionIndex].label.c_str(),
			saving ? "save" : "restore");
		leave();
		_engine->getInput()->discardMouseTransitions();
		const bool completed = saving ? _engine->saveGameDialog() : _engine->loadGameDialog();
		_engine->getInput()->discardMouseTransitions();
		debugC(1, kDebugSaveLoad,
			"Ripper: toolbar save/restore dialog mode=%s completed=%d",
			saving ? "save" : "restore", completed);
		return;
	}
	if (actionIndex == 6) {
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=7 id=0x51a label='%s' opening Options Panel",
			_actions[actionIndex].label.c_str());
		leave();
		_engine->getInput()->discardMouseTransitions();
		if (!_optionsPanel->run())
			warning("Ripper: Options Panel action failed");
		_engine->getInput()->discardMouseTransitions();
		return;
	}
	if (actionIndex == kToolbarHelpActionIndex) {
		debugC(1, kDebugScene,
			"Ripper: toolbar action=8 id=0x51b label='%s' opening help",
			_actions[actionIndex].label.c_str());
		if (!_engine->getScripts()->showHelp("toolbar"))
			warning("Ripper: toolbar help action failed");
		return;
	}
	if (actionIndex == kToolbarExitActionIndex) {
		debugC(1, kDebugGeneral,
			"Ripper: toolbar action=9 id=0x51c label='%s' opening quit prompt",
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
		"Ripper: unhandled toolbar action=%u id=0x%x label='%s'",
		actionIndex + 1, actionIndex + 0x514, _actions[actionIndex].label.c_str());
}

int ToolbarManager::resolveActionKey(uint16 command, uint enabledActionMask) const {
	if (command == 0)
		return -1;

	for (uint actionIndex = 0; actionIndex < _actions.size(); ++actionIndex) {
		const uint settingsIndex = _demoVariant ? actionIndex + 1 : actionIndex;
		if ((!_demoVariant && (enabledActionMask & (1 << actionIndex)) == 0) ||
				_engine->getSettings()->getActionKey(settingsIndex) != command)
			continue;
		debugC(3, kDebugInput,
			"Ripper: resolved toolbar key command=0x%04x action=%u id=0x%x "
			"enabledMask=0x%03x demo=%d",
			command, actionIndex + 1, actionIndex + 0x514,
			enabledActionMask, _demoVariant);
		return actionIndex;
	}
	return -1;
}

bool ToolbarManager::dispatchActionKey(uint actionIndex, uint16 command,
		const char *source) {
	if (actionIndex >= _actions.size())
		return false;

	const Common::String keyLabel = formatKeyCommandLabel(command);
	const Common::String actionLabel = _actions[actionIndex].label;
	leave();
	debugC(1, kDebugInput,
		"Ripper: toolbar hotkey triggered command=0x%04x key='%s' action=%u "
		"id=0x%x label='%s' source=%s",
		command, keyLabel.c_str(), actionIndex + 1, actionIndex + 0x514,
		actionLabel.c_str(), source ? source : "unknown");
	showScreenMessage(Common::String::format("Toolbar hotkey %s: %s",
		keyLabel.c_str(), actionLabel.c_str()));
	presentScreen();
	dispatchAction(actionIndex);
	return true;
}

bool ToolbarManager::service(const MouseState &mouse, uint enabledActionMask,
		int *selectedAction) {
	if (_actions.empty())
		return false;
	if (mouse.position.y >= kToolbarActivationHeight) {
		leave();
		return false;
	}

	enabledActionMask &= (1 << _actions.size()) - 1;
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

	const bool previewAllowed = !_demoVariant ||
		_engine->getSettings()->getDemoToggle(RipperSettings::kDemoToolbarHelp);
	if (!_previewEnabled && previewAllowed &&
			now - _sessionStartMillis >= kPreviewDelayMillis) {
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
