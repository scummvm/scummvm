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

namespace Hopkins {

namespace {

static const char *const kBaseKeymapId = "hopkins-base";
static const char *const kDefaultKeymapId = "hopkins-default";
static const char *const kShortcutKeymapId = "game-shortcuts";

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
		_game._input = BaseInputState();
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
		_vm(vm), _engine(nullptr), _renderer(nullptr), _entry(nullptr), _result(-1),
		_keymapsSwitched(false), _defaultKeymapWasEnabled(false),
		_shortcutKeymapWasEnabled(false), _baseKeymapWasEnabled(false),
		_inputSuspended(false), _mainMenuRequested(false),
		_presentationRefreshRequested(false), _timingResetRequested(false),
		_quitRequested(false) {
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

	_input = BaseInputState();
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
		if (_inputSuspended) {
			previousTime = now;
			accumulator = 0;
			_vm->_soundMan->checkSounds();
			g_system->delayMillis(10);
			continue;
		}
		const uint32 elapsed = MIN<uint32>(now - previousTime, 250);
		previousTime = now;
		accumulator += elapsed * 24;

		bool advanced = false;
		while (accumulator >= 1000 && _result == -1) {
			accumulator -= 1000;
			const int tickResult = _engine->tick(_input);
			processSoundEvents();
			if (tickResult >= 0)
				_result = tickResult;
			advanced = true;
		}

		if (advanced && _result == -1)
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

	if (entering) {
		if (_keymapsSwitched)
			return;
		_defaultKeymapWasEnabled = defaultKeymap && defaultKeymap->isEnabled();
		_shortcutKeymapWasEnabled = shortcutKeymap && shortcutKeymap->isEnabled();
		_baseKeymapWasEnabled = baseKeymap && baseKeymap->isEnabled();
		if (defaultKeymap)
			defaultKeymap->setEnabled(false);
		if (shortcutKeymap)
			shortcutKeymap->setEnabled(false);
		if (baseKeymap)
			baseKeymap->setEnabled(true);
		_keymapsSwitched = true;
	} else if (_keymapsSwitched) {
		if (defaultKeymap)
			defaultKeymap->setEnabled(_defaultKeymapWasEnabled);
		if (shortcutKeymap)
			shortcutKeymap->setEnabled(_shortcutKeymapWasEnabled);
		if (baseKeymap)
			baseKeymap->setEnabled(_baseKeymapWasEnabled);
		_keymapsSwitched = false;
	}
}

void BaseGame::pollInput() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			_input = BaseInputState();
			_quitRequested = true;
			break;
		case Common::EVENT_MAINMENU:
			_mainMenuRequested = true;
			break;
		case Common::EVENT_SCREEN_CHANGED:
			_input = BaseInputState();
			_presentationRefreshRequested = true;
			_timingResetRequested = true;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			handleAction(event.customType, true);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			handleAction(event.customType, false);
			break;
		case Common::EVENT_FOCUS_LOST:
			_input = BaseInputState();
			_inputSuspended = true;
			_timingResetRequested = true;
			break;
		case Common::EVENT_FOCUS_GAINED:
			_inputSuspended = false;
			_timingResetRequested = true;
			break;
		case Common::EVENT_INPUT_CHANGED:
			_input = BaseInputState();
			_timingResetRequested = true;
			break;
		default:
			break;
		}
	}

	if (_mainMenuRequested && _result == -1 && !_quitRequested && !_vm->shouldQuit())
		openMainMenu();
	if (_presentationRefreshRequested && _result == -1 && !_quitRequested && !_vm->shouldQuit())
		refreshPresentation();
}

void BaseGame::handleAction(uint32 action, bool pressed) {
	switch (action) {
	case kActionBaseForward:
		_input.forward = pressed;
		break;
	case kActionBaseBackward:
		_input.backward = pressed;
		break;
	case kActionBaseTurnLeft:
		_input.turnLeft = pressed;
		break;
	case kActionBaseTurnRight:
		_input.turnRight = pressed;
		break;
	case kActionBaseFire:
		_input.fire = pressed;
		break;
	case kActionBaseUse:
		if (pressed)
			_input.exitRequested = true;
		break;
	case kActionBaseToggleTextures:
		if (pressed)
			_input.toggleTextures = true;
		break;
	case kActionBaseMenu:
		if (pressed)
			_mainMenuRequested = true;
		break;
	default:
		break;
	}
}

void BaseGame::openMainMenu() {
	_mainMenuRequested = false;
	_input = BaseInputState();
	_inputSuspended = true;
	_vm->openMainMenuDialog();
	// A modal menu may consume the KEYUP for the key that opened it. Purge
	// keyboard events before resuming so WBASE cannot inherit a stuck action.
	g_system->getEventManager()->purgeKeyboardEvents();
	_inputSuspended = false;
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
	Common::copy(_framebuffer.begin(), _framebuffer.end(), _vm->_graphicsMan->_frontBuffer);	_vm->_graphicsMan->addDirtyRect(0, 0, kBaseFrameWidth, kBaseFrameHeight);
	_vm->_graphicsMan->updateScreen();
}

} // End of namespace Hopkins
