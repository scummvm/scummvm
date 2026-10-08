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

// ScummVM integration glue for Hopkins FBI's Windows-only WBASE game.

#include "hopkins/base.h"

#include "hopkins/base_renderer.h"
#include "hopkins/events.h"
#include "hopkins/font.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "common/algorithm.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/cursorman.h"

namespace Hopkins {

namespace {

static const char *const kBaseKeymapId = "hopkins-base";
static const char *const kDefaultKeymapId = "hopkins-default";
static const char *const kShortcutKeymapId = "game-shortcuts";
static const uint32 kAutoplayMenuInputDelayMs = 200;
static const int kForcedAutoplayPromptTicks = 12;

static uint32 enhancementActionMask(uint32 action) {
	if (action < kActionBaseForward || action > kActionWBASEEnhancementsAutoplay)
		return 0;
	return 1U << (action - kActionBaseForward);
}

// WBASE renders a 320x200 frame through the original Hopkins 640x480
// presentation. Backend mouse events already use that virtual coordinate
// space, so only the presentation offset and scale need to be removed.
static bool presentationToWBASEPoint(const Common::Point &point, Common::Point &result) {
	if (point.x < 0 || point.x >= 640 || point.y < 30 || point.y >= 430)
		return false;
	result = Common::Point(point.x / 2, (point.y - 30) / 2);
	return true;
}

static bool autoplayMenuContainsPoint(const Common::Point &point) {
	return point.x >= 42 && point.x <= 277 && point.y >= 25 && point.y <= 180;
}

static const char *const kBaseSounds[] = {
	nullptr,
	nullptr,
	"SOUND54.WAV", // guard shot
	"SOUND40.WAV", // player shot
	"SOUND53.WAV", // door / exit
	"SOUND55.WAV"  // guard hit
};

static int voiceModeForSound(int soundIndex) {
	switch (soundIndex) {
	case kBaseSoundPlayerShot:
		return 5;
	case kBaseSoundDoorOrExit:
		return 6;
	case kBaseSoundEnemyShot:
	case kBaseSoundGuardHit:
		return 7;
	default:
		return 9;
	}
}

} // End of anonymous namespace

class BaseGame::SessionGuard {
public:
	explicit SessionGuard(BaseGame &game) :
			_game(game), _presentationReady(false), _audioReady(false),
			_keymapsReady(false), _engineMarkedActive(false) {
	}

	~SessionGuard() {
		_game.resetEnhancementSession();
		_game.releaseEnhancementCursor();
		if (_keymapsReady)
			_game.switchKeymaps(false);
		if (_audioReady)
			_game.releaseAudio();
		if (_presentationReady)
			_game.restorePresentation(_backup);
		if (_engineMarkedActive)
			_game.setEngineActive(false);
	}

	bool initialize() {
		_game.setEngineActive(true);
		_engineMarkedActive = true;

		if (!_game.initializePresentation(_backup))
			return false;
		_presentationReady = true;
		_game.initializeEnhancementCursor();

		_game.initializeAudio();
		_audioReady = true;

		_game.switchKeymaps(true);
		_keymapsReady = true;
		return true;
	}

private:
	BaseGame &_game;
	GraphicsStateBackup _backup;
	bool _presentationReady;
	bool _audioReady;
	bool _keymapsReady;
	bool _engineMarkedActive;
};

BaseGame::BaseGame(HopkinsEngine *vm) :
		_vm(vm), _wbaseEnhancements(vm->getTargetName()), _engine(nullptr), _renderer(nullptr),
		_wbaseMappedInputHolds(0), _wbaseMappedInputPulses(0), _wbaseInputGeneration(0),
		_wbaseTextureTogglePending(false), _entry(nullptr), _result(-1),
		_keymapsSwitched(false), _defaultKeymapWasEnabled(false),
		_shortcutKeymapWasEnabled(false), _baseKeymapWasEnabled(false), _wbaseEnhancementsKeymapWasEnabled(false),
		_inputSuspended(false), _mainMenuRequested(false),
		_presentationRefreshRequested(false), _timingResetRequested(false),
		_wbaseEnhancementPanel(kWBASEEnhancementPanelNone), _wbaseEnhancementHeldActions(0),
		_wbaseAutoplayMenuOpenedAt(0), _wbaseForcedAutoplayPromptTicks(0),
		_wbaseForcedAutoplayPromptIssued(false), _wbaseEnhancementHoveredControl(kWBASEEnhancementControlNone),
		_wbaseEnhancementInputArmed(false), _wbaseAutoplayMenuInputArmed(false),
		_wbaseEnhancementCursorPushed(false), _wbaseEnhancementCursorVisible(false),
		_wbaseOverlayVisible(false), _quitRequested(false) {
	_framebuffer.resize(kBaseFrameWidth * kBaseFrameHeight);
	Common::fill(_audioLoaded, _audioLoaded + ARRAYSIZE(_audioLoaded), false);
}

BaseGame::~BaseGame() {
	delete _renderer;
	delete _engine;
}

bool BaseGame::hasRequiredResources(Common::String *missingResources) {
	return BaseData::hasRequiredResources(missingResources);
}

const BaseEntryPoint *BaseGame::entryPoints(uint &count) {
	static const BaseEntryPoint entries[] = {
		{ 194, 2144, 2144,    0, 0x0860, 94 },
		{ 195, 3680, 2144,  915, 0x087a, 95 },
		{ 196, 2016,  544,  440, 0x01df, 96 },
		{ 197, 2784, 1504,  440, 0x05ab, 97 },
		{ 198, 2784, 2912, 1440, 0x0bab, 98 },
		{ 199, 3808,  544,  480, 0x01fb, 99 }
	};
	count = ARRAYSIZE(entries);
	return entries;
}

const BaseEntryPoint *BaseGame::findEntryPoint(int entryId) {
	uint count = 0;
	const BaseEntryPoint *entries = entryPoints(count);
	for (uint i = 0; i < count; ++i) {
		if (entries[i].entryId == entryId)
			return &entries[i];
	}
	return nullptr;
}

BaseRunResult BaseGame::run(int entryId) {
	_entry = findEntryPoint(entryId);
	if (!_entry) {
		warning("Hopkins WBASE: unsupported entry id %d", entryId);
		return BaseRunResult(kBaseRunFallback);
	}

	Common::String errorMessage;
	if (!_data.load(errorMessage)) {
		warning("Hopkins WBASE initialization failed: %s", errorMessage.c_str());
		return BaseRunResult(kBaseRunFallback);
	}

	delete _engine;
	_engine = new BaseEngine(_data);
	_engine->initialize(*_entry);

	delete _renderer;
	_renderer = BaseRenderer::createSoftware();
	if (!_renderer) {
		warning("Hopkins WBASE: could not create the software renderer");
		return BaseRunResult(kBaseRunFallback);
	}

	SessionGuard session(*this);
	if (!session.initialize())
		return BaseRunResult(kBaseRunFallback);

	resetEnhancementSession();
	_inputSuspended = false;
	_mainMenuRequested = false;
	_presentationRefreshRequested = false;
	_timingResetRequested = false;
	_quitRequested = false;
	_result = -1;
	renderFrame();

	// The original uses a 240 Hz timer and advances WBASE every tenth tick.
	// Accumulating elapsed milliseconds * 24 reproduces that 24 Hz cadence
	// without tying simulation speed to the host display refresh rate.
	uint32 previousTime = g_system->getMillis();
	uint32 accumulator = 0;
	while (_result == -1 && !_quitRequested && !_vm->shouldQuit()) {
		pollInput();
		const uint32 now = g_system->getMillis();
		if (_timingResetRequested) {
			previousTime = now;
			accumulator = 0;
			_timingResetRequested = false;
		}
		if (_result != -1 || _quitRequested || _vm->shouldQuit())
			break;
		if (_inputSuspended || enhancementPanelVisible()) {
			previousTime = now;
			accumulator = 0;
			if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay && _wbaseAutoplayMenuInputArmed) {
				Common::Point mouse;
				if (presentationToWBASEPoint(g_system->getEventManager()->getMousePos(), mouse))
					_wbaseEnhancementsAutoplay.updateMenuPointer(mouse.x, mouse.y);
				else
					_wbaseEnhancementsAutoplay.clearMenuPointer();
			}
			if (enhancementPanelVisible() && !_inputSuspended)
				renderFrame();
			_vm->_soundMan->checkSounds();
			g_system->delayMillis(enhancementPanelVisible() ? 50 : 10);
			continue;
		}
		const uint32 elapsed = MIN<uint32>(now - previousTime, 250);
		previousTime = now;
		accumulator += elapsed * 24;

		bool advanced = false;
		while (accumulator >= 1000 && _result == -1) {
			accumulator -= 1000;
			BaseInputState gameplayInput;
			BaseInputState autoplayInput;
			BaseInputState *tickInput = &gameplayInput;
			if (_wbaseEnhancementsAutoplay.active()) {
				const int forcedRoom = _wbaseEnhancementsAutoplay.update(*_engine, autoplayInput);
				if (forcedRoom >= 94 && forcedRoom <= 99) {
					_result = forcedRoom;
					break;
				}
				if (_wbaseEnhancements.forcedAutoplayEnabled() && !_wbaseEnhancementsAutoplay.active()) {
					openAutoplayMenu();
					accumulator = 0;
					break;
				}
				tickInput = &autoplayInput;
			} else if (_wbaseEnhancements.forcedAutoplayEnabled()) {
				tickInput = &autoplayInput;
			} else {
				gameplayInput = consumeGameplayInput();
			}
			const int tickResult = _engine->tick(*tickInput);
			processSoundEvents();
			if (tickResult >= 0)
				_result = tickResult;
			advanced = true;
			if (_result == -1 && _wbaseEnhancements.forcedAutoplayEnabled() &&
					!_wbaseForcedAutoplayPromptIssued && _wbaseForcedAutoplayPromptTicks > 0 &&
					--_wbaseForcedAutoplayPromptTicks == 0) {
				_wbaseForcedAutoplayPromptIssued = true;
				openAutoplayMenu();
				accumulator = 0;
				break;
			}
		}

		if (advanced && _result == -1 && !enhancementPanelVisible())
			renderFrame();
		_vm->_soundMan->checkSounds();
		g_system->delayMillis(2);
	}

	if (_quitRequested || _vm->shouldQuit())
		return BaseRunResult(kBaseRunQuit);
	return BaseRunResult(kBaseRunCompleted, _result);
}

void BaseGame::setEngineActive(bool active) {
	_vm->_inBaseGame = active;
}

bool BaseGame::initializePresentation(GraphicsStateBackup &backup) {
	GraphicsManager *graphics = _vm->_graphicsMan;
	EventsManager *events = _vm->_events;
	if (!graphics || !events || !graphics->_frontBuffer || !graphics->_backBuffer)
		return false;

	backup.breakout = events->_breakoutFl;
	backup.mouseVisible = events->_mouseFl;
	backup.disableInventory = _vm->_globals->_disableInventFl;
	backup.lineNbr = graphics->_lineNbr;
	backup.lineNbr2 = graphics->_lineNbr2;
	backup.minX = graphics->_minX;
	backup.minY = graphics->_minY;
	backup.maxX = graphics->_maxX;
	backup.maxY = graphics->_maxY;
	Common::copy(graphics->_paletteBuffer, graphics->_paletteBuffer + sizeof(backup.paletteBuffer), backup.paletteBuffer);
	Common::copy(graphics->_palette, graphics->_palette + sizeof(backup.palette), backup.palette);
	Common::copy(graphics->_oldPalette, graphics->_oldPalette + sizeof(backup.oldPalette), backup.oldPalette);

	_vm->_globals->_disableInventFl = true;
	events->_breakoutFl = true;
	events->_escKeyFl = false;
	events->_gameKey = KEY_NONE;
	events->mouseOff();

	graphics->resetDirtyRects();
	graphics->resetRefreshRects();
	graphics->setScreenWidth(kBaseFrameWidth);
	graphics->_minX = 0;
	graphics->_minY = 0;
	graphics->_maxX = kBaseFrameWidth;
	graphics->_maxY = kBaseFrameHeight;
	Common::fill(graphics->_frontBuffer, graphics->_frontBuffer + kBaseFrameWidth * kBaseFrameHeight, 0);
	Common::fill(graphics->_backBuffer, graphics->_backBuffer + kBaseFrameWidth * kBaseFrameHeight, 0);
	Common::fill(graphics->_palette, graphics->_palette + sizeof(graphics->_palette), 0);
	Common::copy(_data.palette(), _data.palette() + 256 * 3, graphics->_palette);
	graphics->setPaletteVGA256(_data.palette());
	graphics->clearScreen();
	return true;
}

void BaseGame::restorePresentation(const GraphicsStateBackup &backup) {
	GraphicsManager *graphics = _vm->_graphicsMan;
	EventsManager *events = _vm->_events;

	graphics->resetDirtyRects();
	graphics->resetRefreshRects();
	graphics->clearScreen();
	graphics->updateScreen();

	events->_breakoutFl = backup.breakout;
	events->_escKeyFl = false;
	events->_gameKey = KEY_NONE;
	_vm->_globals->_disableInventFl = backup.disableInventory;
	graphics->_lineNbr = backup.lineNbr;
	graphics->_lineNbr2 = backup.lineNbr2;
	graphics->_minX = backup.minX;
	graphics->_minY = backup.minY;
	graphics->_maxX = backup.maxX;
	graphics->_maxY = backup.maxY;
	Common::copy(backup.paletteBuffer, backup.paletteBuffer + sizeof(backup.paletteBuffer), graphics->_paletteBuffer);
	Common::copy(backup.palette, backup.palette + sizeof(backup.palette), graphics->_palette);
	Common::copy(backup.oldPalette, backup.oldPalette + sizeof(backup.oldPalette), graphics->_oldPalette);

	if (backup.mouseVisible)
		events->mouseOn();
	else
		events->mouseOff();
}

void BaseGame::initializeAudio() {
	Common::String missingAudio;
	BaseData::appendAudioResourceReport(missingAudio);
	if (!missingAudio.empty())
		warning("Hopkins WBASE: optional audio resources unavailable: %s", missingAudio.c_str());

	for (int soundIndex = kBaseSoundEnemyShot; soundIndex <= kBaseSoundGuardHit; ++soundIndex) {
		const Common::Path filename(kBaseSounds[soundIndex]);
		if (!Common::File::exists(filename))
			continue;

		_audioLoaded[soundIndex] = _vm->_soundMan->loadSample(soundIndex, filename);
		if (!_audioLoaded[soundIndex])
			warning("Hopkins WBASE: could not decode optional audio resource %s", filename.toString().c_str());
	}
}

void BaseGame::releaseAudio() {
	for (int soundIndex = kBaseSoundEnemyShot; soundIndex <= kBaseSoundGuardHit; ++soundIndex) {
		if (_audioLoaded[soundIndex]) {
			_vm->_soundMan->removeSample(soundIndex);
			_audioLoaded[soundIndex] = false;
		}
	}
}

void BaseGame::switchKeymaps(bool entering) {
	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	if (!keymapper)
		return;

	Common::Keymap *defaultKeymap = keymapper->getKeymap(kDefaultKeymapId);
	Common::Keymap *shortcutKeymap = keymapper->getKeymap(kShortcutKeymapId);
	Common::Keymap *baseKeymap = keymapper->getKeymap(kBaseKeymapId);
	Common::Keymap *wbaseEnhancementsKeymap = keymapper->getKeymap(kWBASEEnhancementsKeymapId);

	if (entering) {
		if (_keymapsSwitched)
			return;
		_defaultKeymapWasEnabled = defaultKeymap && defaultKeymap->isEnabled();
		_shortcutKeymapWasEnabled = shortcutKeymap && shortcutKeymap->isEnabled();
		_baseKeymapWasEnabled = baseKeymap && baseKeymap->isEnabled();
		_wbaseEnhancementsKeymapWasEnabled = wbaseEnhancementsKeymap && wbaseEnhancementsKeymap->isEnabled();
		if (defaultKeymap)
			defaultKeymap->setEnabled(false);
		if (shortcutKeymap)
			shortcutKeymap->setEnabled(false);
		if (baseKeymap)
			baseKeymap->setEnabled(true);
		if (wbaseEnhancementsKeymap)
			wbaseEnhancementsKeymap->setEnabled(_wbaseEnhancements.controlsEnabled());
		_keymapsSwitched = true;
	} else if (_keymapsSwitched) {
		if (defaultKeymap)
			defaultKeymap->setEnabled(_defaultKeymapWasEnabled);
		if (shortcutKeymap)
			shortcutKeymap->setEnabled(_shortcutKeymapWasEnabled);
		if (baseKeymap)
			baseKeymap->setEnabled(_baseKeymapWasEnabled);
		if (wbaseEnhancementsKeymap)
			wbaseEnhancementsKeymap->setEnabled(_wbaseEnhancementsKeymapWasEnabled);
		_keymapsSwitched = false;
	}
}

void BaseGame::initializeEnhancementCursor() {
	if (!_wbaseEnhancements.controlsEnabled() || _wbaseEnhancementCursorPushed)
		return;
	CursorMan.setDefaultArrowCursor(true);
	_wbaseEnhancementCursorPushed = true;
	_wbaseEnhancementCursorVisible = false;
	CursorMan.showMouse(false);
	if (_vm->_events)
		_vm->_events->_mouseFl = false;
	updateEnhancementCursor();
}

void BaseGame::releaseEnhancementCursor() {
	if (!_wbaseEnhancementCursorPushed)
		return;
	CursorMan.showMouse(false);
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
	if (_vm->_events)
		_vm->_events->_mouseFl = false;
	_wbaseEnhancementCursorPushed = false;
	_wbaseEnhancementCursorVisible = false;
}

void BaseGame::updateEnhancementCursor() {
	if (!_wbaseEnhancementCursorPushed || g_system->isOverlayVisible())
		return;

	Common::Point mouse;
	bool visible = false;
	if (!_inputSuspended && presentationToWBASEPoint(g_system->getEventManager()->getMousePos(), mouse)) {
		visible = mouse.y >= kBaseViewHeight;
		if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay)
			visible = visible || autoplayMenuContainsPoint(mouse);
	}

	if (visible != _wbaseEnhancementCursorVisible) {
		CursorMan.showMouse(visible);
		_wbaseEnhancementCursorVisible = visible;
	}
	if (_vm->_events)
		_vm->_events->_mouseFl = visible;
}

void BaseGame::resetEnhancementSession() {
	clearGameplayInput(true);
	_wbaseEnhancementPanel = kWBASEEnhancementPanelNone;
	_wbaseAutoplayMenuOpenedAt = 0;
	_wbaseForcedAutoplayPromptTicks = _wbaseEnhancements.forcedAutoplayEnabled() ? kForcedAutoplayPromptTicks : 0;
	_wbaseForcedAutoplayPromptIssued = false;
	_wbaseEnhancementHoveredControl = kWBASEEnhancementControlNone;
	_wbaseEnhancementInputArmed = !_wbaseEnhancements.controlsEnabled();
	_wbaseAutoplayMenuInputArmed = false;
	_wbaseOverlayVisible = g_system->isOverlayVisible();
	_wbaseEnhancementsAutoplay.reset();
}

void BaseGame::clearGameplayInput(bool disarm) {
	_wbaseMappedInputHolds = 0;
	_wbaseMappedInputPulses = 0;
	_wbaseTextureTogglePending = false;
	_wbasePendingGameplayRequests.clear();
	++_wbaseInputGeneration;
	if (_wbaseInputGeneration == 0)
		++_wbaseInputGeneration;
	if (disarm) {
		_wbaseEnhancementHeldActions = 0;
		_wbaseEnhancementInputArmed = !_wbaseEnhancements.controlsEnabled();
		_wbaseAutoplayMenuInputArmed = false;
	}
}

BaseInputState BaseGame::consumeGameplayInput() {
	const uint32 active = _wbaseMappedInputHolds | _wbaseMappedInputPulses;
	BaseInputState input;
	input.forward = (active & enhancementActionMask(kActionBaseForward)) != 0;
	input.backward = (active & enhancementActionMask(kActionBaseBackward)) != 0;
	input.turnLeft = (active & enhancementActionMask(kActionBaseTurnLeft)) != 0;
	input.turnRight = (active & enhancementActionMask(kActionBaseTurnRight)) != 0;
	input.fire = (active & enhancementActionMask(kActionBaseFire)) != 0;
	input.toggleTextures = _wbaseTextureTogglePending;
	_wbaseMappedInputPulses = 0;
	_wbaseTextureTogglePending = false;

	while (!_wbasePendingGameplayRequests.empty() &&
			_wbasePendingGameplayRequests.front().generation != _wbaseInputGeneration)
		_wbasePendingGameplayRequests.pop();
	if (!_wbasePendingGameplayRequests.empty()) {
		const PendingGameplayRequest request = _wbasePendingGameplayRequests.pop();
		if (request.type == kPendingGameplayExit)
			input.exitRequested = true;
	}
	return input;
}

void BaseGame::setMappedGameplayAction(uint32 action, bool pressed) {
	if (action < kActionBaseForward || action > kActionBaseFire)
		return;
	const uint32 mask = enhancementActionMask(action);
	if (pressed) {
		if (_wbaseEnhancements.controlsEnabled() && !(_wbaseMappedInputHolds & mask))
			_wbaseMappedInputPulses |= mask;
		_wbaseMappedInputHolds |= mask;
	} else {
		_wbaseMappedInputHolds &= ~mask;
	}
}

void BaseGame::queueGameplayRequest(PendingGameplayRequestType type) {
	PendingGameplayRequest request;
	request.type = type;
	request.generation = _wbaseInputGeneration;
	_wbasePendingGameplayRequests.push(request);
}

bool BaseGame::updateEnhancementActionState(uint32 action, bool pressed) {
	const uint32 mask = enhancementActionMask(action);
	if (!mask)
		return true;
	const bool wasPressed = (_wbaseEnhancementHeldActions & mask) != 0;
	if (pressed)
		_wbaseEnhancementHeldActions |= mask;
	else
		_wbaseEnhancementHeldActions &= ~mask;
	return wasPressed != pressed;
}

void BaseGame::updateEnhancementInputArming(uint32 now) {
	if (_inputSuspended || _wbaseOverlayVisible)
		return;
	const bool mouseReleased = !(g_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON);
	if (!_wbaseEnhancementInputArmed && !_wbaseEnhancementHeldActions && mouseReleased) {
		_wbaseEnhancementInputArmed = true;
		debug(2, "Hopkins WBASE enhancement input armed after entry boundary");
	}
	if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay &&
			!_wbaseAutoplayMenuInputArmed &&
			now - _wbaseAutoplayMenuOpenedAt >= kAutoplayMenuInputDelayMs &&
			!_wbaseEnhancementHeldActions && mouseReleased) {
		_wbaseAutoplayMenuInputArmed = true;
		debug(2, "Hopkins WBASE autoplay menu input armed");
	}
}

void BaseGame::updateOverlayInputState() {
	const bool overlayVisible = g_system->isOverlayVisible();
	if (overlayVisible == _wbaseOverlayVisible)
		return;
	_wbaseOverlayVisible = overlayVisible;
	clearGameplayInput(true);
	_timingResetRequested = true;
}

void BaseGame::setEnhancementPanel(WBASEEnhancementPanel panel) {
	_wbaseEnhancementPanel = panel;
	clearGameplayInput(false);
	_timingResetRequested = true;
	_wbaseEnhancementsAutoplay.clearMenuPointer();
	_wbaseAutoplayMenuInputArmed = false;
	if (panel == kWBASEEnhancementPanelAutoplay)
		_wbaseAutoplayMenuOpenedAt = g_system->getMillis();
	else
		_wbaseAutoplayMenuOpenedAt = 0;
	debug(2, "Hopkins WBASE enhancement panel changed to %d", panel);
	if (_engine && _renderer)
		renderFrame();
}

void BaseGame::openAutoplayMenu() {
	if (_wbaseEnhancements.forcedAutoplayEnabled()) {
		_wbaseForcedAutoplayPromptTicks = 0;
		_wbaseForcedAutoplayPromptIssued = true;
	}
	setEnhancementPanel(kWBASEEnhancementPanelAutoplay);
}

void BaseGame::closeNavigationMap() {
	if (_wbaseEnhancements.forcedAutoplayEnabled() &&
			_wbaseForcedAutoplayPromptIssued && !_wbaseEnhancementsAutoplay.active())
		setEnhancementPanel(kWBASEEnhancementPanelAutoplay);
	else
		setEnhancementPanel(kWBASEEnhancementPanelNone);
}

void BaseGame::startSelectedAutoplay() {
	if (_wbaseEnhancementsAutoplay.startSelected(*_engine)) {
		_wbaseForcedAutoplayPromptTicks = 0;
		_wbaseForcedAutoplayPromptIssued = true;
		setEnhancementPanel(kWBASEEnhancementPanelNone);
	} else {
		renderFrame();
	}
}

void BaseGame::updateEnhancementPointer(int x, int y) {
	const WBASEEnhancementControl hoveredControl = _wbaseEnhancements.controlAtPoint(x, y);
	if (_wbaseEnhancementHoveredControl == hoveredControl)
		return;
	_wbaseEnhancementHoveredControl = hoveredControl;
	if (_engine && _renderer)
		renderFrame();
}

bool BaseGame::handleEnhancementControlClick(int x, int y) {
	const WBASEEnhancementControl control = _wbaseEnhancements.controlAtPoint(x, y);
	if (control == kWBASEEnhancementControlNone)
		return false;
	_wbaseEnhancementHoveredControl = control;
	if (control == kWBASEEnhancementControlNavigationMap) {
		if (_wbaseEnhancementPanel == kWBASEEnhancementPanelNavigationMap)
			closeNavigationMap();
		else
			setEnhancementPanel(kWBASEEnhancementPanelNavigationMap);
	} else if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay) {
		if (!_wbaseEnhancements.forcedAutoplayEnabled() || _wbaseEnhancementsAutoplay.active())
			setEnhancementPanel(kWBASEEnhancementPanelNone);
	} else {
		openAutoplayMenu();
	}
	return true;
}

bool BaseGame::enhancementPanelVisible() const {
	return _wbaseEnhancementPanel != kWBASEEnhancementPanelNone;
}

void BaseGame::pollInput() {
	updateOverlayInputState();
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			clearGameplayInput(true);
			_quitRequested = true;
			break;
		case Common::EVENT_MAINMENU:
			_mainMenuRequested = true;
			break;
		case Common::EVENT_SCREEN_CHANGED:
			clearGameplayInput(true);
			_presentationRefreshRequested = true;
			_timingResetRequested = true;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START: {
			const bool changed = updateEnhancementActionState(event.customType, true);
			if (changed) {
				if (_wbaseEnhancementInputArmed)
					handleAction(event.customType, true);
				else
					debug(2, "Hopkins WBASE suppressed entry-boundary action %u", event.customType);
			}
			break;
		}
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END: {
			const bool changed = updateEnhancementActionState(event.customType, false);
			if (_wbaseEnhancementInputArmed && changed)
				handleAction(event.customType, false);
			break;
		}
		case Common::EVENT_FOCUS_LOST:
			clearGameplayInput(true);
			_inputSuspended = true;
			_timingResetRequested = true;
			break;
		case Common::EVENT_FOCUS_GAINED:
			_inputSuspended = false;
			_timingResetRequested = true;
			break;
		case Common::EVENT_INPUT_CHANGED:
			clearGameplayInput(true);
			_timingResetRequested = true;
			break;
		case Common::EVENT_KEYDOWN:
			if (_wbaseEnhancementInputArmed && _wbaseAutoplayMenuInputArmed &&
					_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay &&
					(event.kbd.keycode == Common::KEYCODE_RETURN ||
					 event.kbd.keycode == Common::KEYCODE_KP_ENTER)) {
				startSelectedAutoplay();
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			if (_wbaseEnhancementInputArmed && _wbaseEnhancements.controlsEnabled()) {
				Common::Point mouse;
				if (presentationToWBASEPoint(event.mouse, mouse))
					updateEnhancementPointer(mouse.x, mouse.y);
				else
					updateEnhancementPointer(-1, -1);
			}
			break;
		case Common::EVENT_LBUTTONDOWN: {
			Common::Point mouse;
			if (!presentationToWBASEPoint(event.mouse, mouse))
				break;
			if (_wbaseEnhancementInputArmed && handleEnhancementControlClick(mouse.x, mouse.y))
				break;
			if (_wbaseEnhancementInputArmed && _wbaseAutoplayMenuInputArmed &&
					_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay) {
				if (!_wbaseEnhancementsAutoplay.selectMenuPointer(mouse.x, mouse.y))
					break;
				startSelectedAutoplay();
			}
			break;
		}
		default:
			break;
		}
	}
	updateOverlayInputState();
	updateEnhancementInputArming(g_system->getMillis());
	updateEnhancementCursor();

	if (_mainMenuRequested && _result == -1 && !_quitRequested && !_vm->shouldQuit())
		openMainMenu();
	if (_presentationRefreshRequested && _result == -1 && !_quitRequested && !_vm->shouldQuit())
		refreshPresentation();
}

void BaseGame::handleAction(uint32 action, bool pressed) {
	if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay) {
		if (!pressed)
			return;
		if (_wbaseEnhancements.forcedAutoplayEnabled() && action == kActionBaseMenu) {
			_mainMenuRequested = true;
			return;
		}
		if (!_wbaseAutoplayMenuInputArmed)
			return;
		switch (action) {
		case kActionBaseForward:
			_wbaseEnhancementsAutoplay.selectPrevious();
			break;
		case kActionBaseBackward:
			_wbaseEnhancementsAutoplay.selectNext();
			break;
		case kActionBaseFire:
		case kActionBaseUse:
			startSelectedAutoplay();
			return;
		case kActionBaseMenu:
			if (_wbaseEnhancements.forcedAutoplayEnabled())
				_mainMenuRequested = true;
			else
				setEnhancementPanel(kWBASEEnhancementPanelNone);
			return;
		case kActionWBASEEnhancementsAutoplay:
			if (!_wbaseEnhancements.forcedAutoplayEnabled() || _wbaseEnhancementsAutoplay.active())
				setEnhancementPanel(kWBASEEnhancementPanelNone);
			return;
		case kActionWBASEEnhancementsNavigationMap:
			setEnhancementPanel(kWBASEEnhancementPanelNavigationMap);
			return;
		default:
			return;
		}
		clearGameplayInput(false);
		_timingResetRequested = true;
		renderFrame();
		return;
	}
	if (_wbaseEnhancementPanel == kWBASEEnhancementPanelNavigationMap) {
		if (!pressed)
			return;
		switch (action) {
		case kActionWBASEEnhancementsNavigationMap:
			closeNavigationMap();
			break;
		case kActionBaseMenu:
			if (_wbaseEnhancements.forcedAutoplayEnabled())
				_mainMenuRequested = true;
			else
				setEnhancementPanel(kWBASEEnhancementPanelNone);
			break;
		case kActionWBASEEnhancementsAutoplay:
			openAutoplayMenu();
			break;
		default:
			break;
		}
		return;
	}
	if (_wbaseEnhancements.forcedAutoplayEnabled()) {
		if (!pressed)
			return;
		switch (action) {
		case kActionBaseMenu:
			_mainMenuRequested = true;
			break;
		case kActionWBASEEnhancementsNavigationMap:
			setEnhancementPanel(kWBASEEnhancementPanelNavigationMap);
			break;
		case kActionWBASEEnhancementsAutoplay:
			openAutoplayMenu();
			break;
		default:
			break;
		}
		return;
	}

	if (pressed && _wbaseEnhancementsAutoplay.active()) {
		switch (action) {
		case kActionBaseForward:
		case kActionBaseBackward:
		case kActionBaseTurnLeft:
		case kActionBaseTurnRight:
		case kActionBaseFire:
		case kActionBaseUse:
			_wbaseEnhancementsAutoplay.cancel();
			clearGameplayInput(false);
			break;
		default:
			break;
		}
	}

	switch (action) {
	case kActionBaseForward:
	case kActionBaseBackward:
	case kActionBaseTurnLeft:
	case kActionBaseTurnRight:
	case kActionBaseFire:
		setMappedGameplayAction(action, pressed);
		break;
	case kActionBaseUse:
		if (pressed)
			queueGameplayRequest(kPendingGameplayExit);
		break;
	case kActionBaseToggleTextures:
		if (pressed)
			_wbaseTextureTogglePending = true;
		break;
	case kActionBaseMenu:
		if (pressed)
			_mainMenuRequested = true;
		break;
	case kActionWBASEEnhancementsNavigationMap:
		if (pressed && _wbaseEnhancements.navigationMapEnabled())
			setEnhancementPanel(kWBASEEnhancementPanelNavigationMap);
		break;
	case kActionWBASEEnhancementsAutoplay:
		if (pressed && _wbaseEnhancements.enabled())
			openAutoplayMenu();
		break;
	default:
		break;
	}
}

void BaseGame::openMainMenu() {
	_mainMenuRequested = false;
	clearGameplayInput(true);
	_inputSuspended = true;
	_vm->openMainMenuDialog();
	// A modal menu may consume the KEYUP for the key that opened it. Purge
	// keyboard events before resuming so WBASE cannot inherit a stuck action.
	g_system->getEventManager()->purgeKeyboardEvents();
	_inputSuspended = false;
	_wbaseOverlayVisible = g_system->isOverlayVisible();
	clearGameplayInput(true);
	_timingResetRequested = true;

	if (_vm->shouldQuit())
		_quitRequested = true;
	else
		refreshPresentation();
}

void BaseGame::refreshPresentation() {
	_presentationRefreshRequested = false;
	if (!_vm->_graphicsMan || !_renderer || !_engine)
		return;

	_vm->_graphicsMan->setPaletteVGA256(_data.palette());
	renderFrame();
}

void BaseGame::processSoundEvents() {
	const Common::Array<int> &events = _engine->soundEvents();
	for (uint i = 0; i < events.size(); ++i) {
		const int soundIndex = events[i];
		if (soundIndex >= 0 && soundIndex < (int)ARRAYSIZE(_audioLoaded) && _audioLoaded[soundIndex])
			_vm->_soundMan->playSample(soundIndex, voiceModeForSound(soundIndex));
	}
	_engine->clearSoundEvents();
}

void BaseGame::renderFrame() {
	if (!_renderer || !_engine || _framebuffer.empty())
		return;
	_renderer->render(*_engine, _framebuffer.begin());
	if (_wbaseEnhancementPanel == kWBASEEnhancementPanelNavigationMap) {
		_wbaseEnhancements.renderNavigationMap(_data, *_engine, _framebuffer.begin());
	} else if (_wbaseEnhancementPanel == kWBASEEnhancementPanelAutoplay) {
		_wbaseEnhancementsAutoplay.renderMenu(_data, _engine->entryReturnId(), _framebuffer.begin(),
				_wbaseEnhancements.forcedAutoplayEnabled());
	} else {
		_wbaseEnhancementsAutoplay.renderStatus(_data, _framebuffer.begin(),
				_wbaseEnhancements.forcedAutoplayEnabled());
	}
	_wbaseEnhancements.renderControls(_data, _wbaseEnhancementPanel,
			_wbaseEnhancementHoveredControl, _wbaseEnhancementsAutoplay.active(), _framebuffer.begin());
	Common::copy(_framebuffer.begin(), _framebuffer.end(), _vm->_graphicsMan->_frontBuffer);
	_vm->_graphicsMan->addDirtyRect(0, 0, kBaseFrameWidth, kBaseFrameHeight);
	updateEnhancementCursor();
	_vm->_graphicsMan->updateScreen();
}

} // End of namespace Hopkins
