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

#include "mohawk/zoombini_resource.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/cursorman.h"
#include "gui/message.h"

#include "mohawk/cursors.h"
#include "mohawk/dialogs.h"
#include "mohawk/resource.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/zoombini_console.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_debug.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_pages/dialog_credits.h"
#include "mohawk/zoombini_pages/dialog_debug.h"
#include "mohawk/zoombini_pages/dialog_help.h"
#include "mohawk/zoombini_pages/dialog_msgbox.h"
#include "mohawk/zoombini_pages/dialog_options.h"
#include "mohawk/zoombini_pages/dialog_saveload.h"
#include "mohawk/zoombini_pages/interactive_base.h"
#include "mohawk/zoombini_pages/interactive_rodmap.h"
#include "mohawk/zoombini_pages/puzzle_bridge.h"
#include "mohawk/zoombini_pages/puzzle_caves.h"
#include "mohawk/zoombini_pages/puzzle_ferry.h"
#include "mohawk/zoombini_pages/puzzle_fleens.h"
#include "mohawk/zoombini_pages/puzzle_hotel.h"
#include "mohawk/zoombini_pages/puzzle_lilly.h"
#include "mohawk/zoombini_pages/puzzle_maze.h"
#include "mohawk/zoombini_pages/puzzle_net.h"
#include "mohawk/zoombini_pages/puzzle_pizza.h"
#include "mohawk/zoombini_pages/puzzle_slides.h"
#include "mohawk/zoombini_pages/puzzle_smoke.h"
#include "mohawk/zoombini_pages/puzzle_tunnels.h"
#include "mohawk/zoombini_pages/shelter_basecamp1.h"
#include "mohawk/zoombini_pages/shelter_basecamp2.h"
#include "mohawk/zoombini_pages/shelter_picker.h"
#include "mohawk/zoombini_pages/shelter_town.h"
#include "mohawk/zoombini_pages/transition_logo.h"
#include "mohawk/zoombini_pages/transition_xfer.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"
#include "mohawk/zoombini_text.h"

namespace Mohawk {

constexpr uint16 MohawkEngine_Zoombini::kBuiltinMidiTestResourceIds[18];
constexpr uint16 MohawkEngine_Zoombini::kBuiltinTlcMidiTestSoundResourceId;

static void showSaveLoadUnavailableMessage() {
	GUI::MessageDialog dialog(_c("Saving or loading is not available right now.", "zoombini-save-load"));
	dialog.runModal();
}

bool MohawkEngine_Zoombini::parsePracticeBootParam(int32 bootParam, ZoombiniPageType &pageType, uint16 &level) {
	if (bootParam <= 0)
		return false;

	int32 levelVal = bootParam % 100;
	int32 pageVal = bootParam / 100;
	if (levelVal < 1 || 4 < levelVal)
		return false;
	if (pageVal < static_cast<int32>(ZoombiniPageType::kPicker) ||
		static_cast<int32>(ZoombiniPageType::kMaze) < pageVal)
		return false;

	pageType = static_cast<ZoombiniPageType>(pageVal);
	level = static_cast<uint16>(levelVal);
	return true;
}

void MohawkEngine_Zoombini::addSearchDirectoryIfPresent(const Common::FSNode &node, int priority, int depth) {
	if (!node.exists() || !node.isDirectory())
		return;
	Common::String path = node.getPath().toString();
	if (!SearchMan.hasArchive(path))
		SearchMan.addDirectory(path, node, priority, depth);
}

static bool hasRegisteredSubDirectoryMatching(const Common::FSNode &root, const Common::String &name) {
	Common::FSList children;
	if (!root.getChildren(children, Common::FSNode::kListDirectoriesOnly))
		return false;

	for (const Common::FSNode &child : children) {
		if (child.getName().equalsIgnoreCase(name) && SearchMan.hasArchive(child.getName()))
			return true;
	}

	return false;
}

bool MohawkEngine_Zoombini::hasSubDirectoryMatching(const Common::FSNode &root, const Common::String &name) {
	if (!root.exists() || !root.isDirectory())
		return false;

	Common::FSList children;
	if (!root.getChildren(children, Common::FSNode::kListDirectoriesOnly))
		return false;

	for (const Common::FSNode &child : children) {
		if (child.getName().equalsIgnoreCase(name))
			return true;
	}

	return false;
}

bool MohawkEngine_Zoombini::hasEuropeLanguageDirectories(const Common::FSNode &root) {
	return (hasSubDirectoryMatching(root, "english") &&
			hasSubDirectoryMatching(root, "french") &&
			hasSubDirectoryMatching(root, "german")) ||
		   hasSubDirectoryMatching(root, "italian");
}

bool MohawkEngine_Zoombini::tryAddZoombiniIsoRootSearchPath(const Common::FSNode &root) {
	if (hasEuropeLanguageDirectories(root)) {
		addSearchDirectoryIfPresent(root, 0, 2);
		return true;
	}

	if (!hasSubDirectoryMatching(root, "data"))
		return false;

	addSearchDirectoryIfPresent(root, 0, 4);
	if (!hasRegisteredSubDirectoryMatching(root, "data"))
		SearchMan.addSubDirectoryMatching(root, "data", 0, 1);
	SearchMan.addSubDirectoryMatching(root, "setup/data1/data32", 0, 1);
	SearchMan.addSubDirectoryMatching(root, "setup/data1/data16", 0, 1);
	SearchMan.addSubDirectoryMatching(root, "install/hd", 0, 1);
	return true;
}

MohawkEngine_Zoombini::MohawkEngine_Zoombini(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	// Opt in before page initialization so ordinary pages use direct key events.
	// Editable text fields enable composition only while they own keyboard input.
	syst->acquireImeCompositionControl();

	DebugMan.addDebugChannel(MohawkEngine_Zoombini::kDebugSaveLoad01, "SaveLoad", "Log Zoombini save and roster I/O");
	DebugMan.addDebugChannel(MohawkEngine_Zoombini::kDebugState03, "State", "Log Zoombini persistent game-state transitions");
	DebugMan.addDebugChannel(MohawkEngine_Zoombini::kDebugPage02, "Page", "Log Zoombini page lifecycle and puzzle-state transitions");

	assert(!_mainMenuDialog);
	_mainMenuDialog = new ZoombiniMenuDialog(this);
}

MohawkEngine_Zoombini::~MohawkEngine_Zoombini() {
	while (!_dialogPageStack.empty())
		delete _dialogPageStack.pop();

	delete _activePage;
	delete _sysMhk;
	delete _helpMhk;
	delete _midiMhk;
	delete _snoidShapeOffsetRegs;
	delete _smallSnoidShapeOffsetRegs;
	delete _snoidScriptShapeRegs;

	delete _midi;
	delete _sound;
	delete _video;
	delete _gfx;
	delete _rnd;
	delete _state;
	delete _text;
}

Common::Error MohawkEngine_Zoombini::run() {
	MohawkEngine::run();

	if (!_mixer->isReady()) {
		return Common::kAudioDeviceInitFailed;
	}

	setDebugger(new ZoombiniConsole(this));
	initSearchPaths();

	_language = getLanguage();

	_gfx = new ZoombiniGraphics(this);
	_gfx->showDemoStartupLoadingScreen();
	_video = new VideoManager(this);
	_sound = new ZoombiniSound(this);
	_midi = new ZoombiniMidiPlayer(this);
	_rnd = new ZoombiniRandom(getGameId());
	_state = new ZoombiniGameState(this, _saveFileMan);
	_text = new ZoombiniText(this, _language);
	applyGameSettings();


	// Set MHK archive root based on game variant and language
	_mhkArchiveRoot = ZMB_MHK_ROOT_GENERIC;
	if (isVersionFamilyEuV1()) {
		switch (_language) {
		case Common::EN_GRB:
			_mhkArchiveRoot = ZMB_MHK_ROOT_EUR_GB;
			break;
		case Common::FR_FRA:
			_mhkArchiveRoot = ZMB_MHK_ROOT_EUR_FR;
			break;
		case Common::DE_DEU:
			_mhkArchiveRoot = ZMB_MHK_ROOT_EUR_GE;
			break;
		case Common::ES_ESP:
		case Common::PT_PRT:
			_mhkArchiveRoot = ZMB_MHK_ROOT_GENERIC;
			break;
		case Common::IT_ITA:
			_mhkArchiveRoot = ZMB_MHK_ROOT_EUR_IT;
			break;
		default:
			error("engine: unsupported language for v1.x Europe variant: %s", Common::getLanguageLocale(_language));
			break;
		}
	}

	// Load ZOOMBINI.MHK
	_sysMhk = loadSystemArchive();
	if (isVersionFamilyTlcV2())
		_helpMhk = loadHelpArchive();
	if (isVersionFamilyV1())
		loadMidiArchive();

	ZoombiniCursorManager *zmbCursor = new ZoombiniCursorManager(this);
	zmbCursor->preloadCursors();
	_cursor = zmbCursor;
	_cursor->setDefaultCursor();
	_cursor->showCursor();

	// Load global registration-point offsets for snoid body-part shapes.
	// Loaded from REGS 100+101 in ZOOMBINI.MHK.
	// Each resource is a paired-axis table: one big-endian int16 per entry,
	// with a reserved entry 0 and one entry per tBMP 3000 sub-image.
	// Snoid rendering subtracts the pair as the layer's width and height.
	_snoidShapeOffsetRegs = new ZmbShapeOffsetRegs();
	_snoidShapeOffsetRegs->parseStreams(this, ZmbResource::kSystem, 100, 101);

	// Load REGS 3200+3201 for small snoid shapes (tBMP 3200).
	// Same paired-axis form as REGS 100+101, again consumed as width and height.
	// Load these after swapping body-part tables.
	_smallSnoidShapeOffsetRegs = new ZmbShapeOffsetRegs();
	_smallSnoidShapeOffsetRegs->parseStreams(this, ZmbResource::kSystem, 3200, 3201);

	// Load REGS 102+103 for SCRS-rendered Snoids using shape archive tBMP 3100.
	// Same paired-axis form as REGS 100+101, sized for the script shape set.
	// Selected for state 9 NORMAL playback, which is what Ferry's reject-flight (case 1's SCRS 1900-1906) needs.
	_snoidScriptShapeRegs = new ZmbShapeOffsetRegs();
	_snoidScriptShapeRegs->parseStreams(this, ZmbResource::kSystem, 102, 103);

	// Load a roster of game saves
	_state->loadRoster();

	// Load default page or a direct practice-mode target encoded as page*100+level.
	setActiveResourceKind(ZmbResource::kPage);
	ZoombiniPageType bootPracticePage = ZoombiniPageType::kNone;
	uint16 bootPracticeLevel = 0;
	int32 bootParam = ConfMan.getInt("boot_param");
	if (parsePracticeBootParam(bootParam, bootPracticePage, bootPracticeLevel)) {
		_state->_practiceLevel = bootPracticeLevel;
		_state->generateRandomPack();
		_state->markGameStateReady();
		debug(1, "engine: boot_param %d selects practice page %d at level %u",
			  bootParam, static_cast<int32>(bootPracticePage), bootPracticeLevel);
		debugC(1, MohawkEngine_Zoombini::kDebugPage02, "engine: boot_param %d selects practice page %d at level %u",
			   bootParam, static_cast<int32>(bootPracticePage), bootPracticeLevel);
		setNextPage(bootPracticePage);
	} else {
		if (bootParam != 0) {
			warning("engine: ignoring unsupported boot_param %d (expected practicePage*100 + level, e.g. 401 for Basecamp 1)",
					bootParam);
		}
		if (isDemo()) {
			_state->_practiceLevel = 1;
			_state->markGameStateReady();
		} else if (_state->_r._nextSaveFileNameCounter == 0) {
			_state->markGameStateReady();
		}
		setNextPage(ZoombiniPageType::kLogo);
	}
	loadNextPage();

	// Main game loop
	while (!mustQuit()) {
		doFrame();
	}

	return Common::kNoError;
}

void MohawkEngine_Zoombini::resetFidgetActivity() {
	// Activity restarts both idle timers without ending an active fidget suppression.
	_lastActivityFrame = getAnimationFrameCounter(_system->getMillis());
	_lastFidgetIntervalFrame = _lastActivityFrame;
	if (_fidgetThreshold)
		_fidgetThreshold = 64;
}

uint32 MohawkEngine_Zoombini::getActivityIdleFrames() const {
	if (hasDialogOpened())
		return 0;
	return getAnimationFrameCounter(_system->getMillis()) - _lastActivityFrame;
}

void MohawkEngine_Zoombini::setArrivalTurnDirection(ArrivalTurnDirection dir) {
	// Map movement direction to the post-arrival turn state.
	// State 1 settles facing left, state 2 settles facing right, and state 0 remains idle.
	if (dir == ArrivalTurnDirection::kLeft)
		_arrivalTurnState = kSnoidAnimState001_TurnLeft;
	else if (dir == ArrivalTurnDirection::kRight)
		_arrivalTurnState = kSnoidAnimState002_TurnRight;
	else
		_arrivalTurnState = kSnoidAnimState000_Idle;
}

void MohawkEngine_Zoombini::processEvents(ZoombiniPage *page) {
	Common::Event event;

	// If a fade is queued or running, defer event processing until it is done.
	if (_gfx->isFading()) {
		// @ref Common::EventManager::pollEvent() must be called to keep the mouse cursor moving.
		while (_system->getEventManager()->pollEvent(event))
			_deferredEventQueue.push(event);

		return;
	}

	// Process deferred events first
	while (!_deferredEventQueue.empty()) {
		event = _deferredEventQueue.front();
		_deferredEventQueue.pop();
		processEvent(page, event);
	}

	// Process new events
	while (_system->getEventManager()->pollEvent(event))
		processEvent(page, event);
}

void MohawkEngine_Zoombini::processEvent(ZoombiniPage *page, const Common::Event &event) {
	// Reset the fidget threshold and idle timer on mouse or keyboard input.
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
	case Common::EVENT_IME_COMPOSITION:
		resetFidgetActivity();
		break;
	default:
		break;
	}
	if (page && page->isBuiltinDebugInputWaitActive() &&
		(event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN ||
		 event.type == Common::EVENT_RBUTTONDOWN))
		page->releaseBuiltinDebugInputWait();

	if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat)
		observeBuiltinCheatKey(event.kbd);

	// Ctrl+S and Ctrl+L are emitted as ordinary key events by the keymapper.
	// Interactive pages own the global shortcuts. Dialogs keep the events in
	// their own input state machines; Options maps them to its Save/Load buttons.
	if (event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat && page &&
		page->getPageCategory() != ZoombiniPageCategory::kDialog &&
		event.kbd.hasFlags(Common::KBD_CTRL) &&
		(event.kbd.keycode == Common::KEYCODE_s || event.kbd.keycode == Common::KEYCODE_l)) {
		if (hasSaveLoadDialogOpened())
			return;

		if (!page->canOpenSaveLoadDialog()) {
			showSaveLoadUnavailableMessage();
			return;
		}
	}

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		page->handleLButtonDown(event.mouse, event.relMouse);
		break;
	case Common::EVENT_LBUTTONUP:
		page->handleLButtonUp(event.mouse, event.relMouse);
		break;
	case Common::EVENT_RBUTTONDOWN:
		page->handleRButtonDown(event.mouse, event.relMouse);
		break;
	case Common::EVENT_RBUTTONUP:
		page->handleRButtonUp(event.mouse, event.relMouse);
		break;
	case Common::EVENT_WHEELUP:
		page->onWheelUp(event.mouse);
		break;
	case Common::EVENT_WHEELDOWN:
		page->onWheelDown(event.mouse);
		break;
	case Common::EVENT_MOUSEMOVE:
		page->onMouseMove(event.mouse, event.relMouse);
		break;
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode == Common::KEYCODE_F5) {
			const ZmbEventHandleResult result = page->onKeyDown(event.kbd, event.kbdRepeat);
			if (result != ZmbEventHandleResult::kConsumed)
				openMainMenuDialog();
		} else {
			page->onKeyDown(event.kbd, event.kbdRepeat);
		}
		break;
	case Common::EVENT_KEYUP:
		page->onKeyUp(event.kbd, event.kbdRepeat);
		break;
	case Common::EVENT_IME_COMPOSITION:
		page->onImeComposition(event.imeComposition);
		break;
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		beginQuitEvent(page);
		break;
	default:
		break;
	}
}

void MohawkEngine_Zoombini::observeBuiltinCheatKey(const Common::KeyState &kbd) {
	if (kbd.ascii == 1 || (kbd.keycode == Common::KEYCODE_a && kbd.hasFlags(Common::KBD_CTRL))) {
		_builtinCheatCaptureArmed = true;
		_builtinCheatCapture.clear();
		return;
	}

	if (!_builtinCheatCaptureArmed)
		return;

	uint32 capturedAscii = kbd.ascii;
	if (kbd.keycode == Common::KEYCODE_SPACE && capturedAscii == 0)
		capturedAscii = ' ';

	// The Windows engine accumulates translated WM_CHAR values only. Physical
	// key messages, including Shift, have values above its character range and
	// leave the sequence unchanged.
	if (capturedAscii < 0x20 || 0x7e < capturedAscii)
		return;
	if (16 <= _builtinCheatCapture.size()) {
		_builtinCheatCaptureArmed = false;
		_builtinCheatCapture.clear();
		return;
	}

	_builtinCheatCapture += static_cast<char>(capturedAscii);
}

bool MohawkEngine_Zoombini::handleBuiltinCheatSpace(const Common::KeyState &kbd) {
	if (kbd.keycode != Common::KEYCODE_SPACE)
		return false;

	bool phraseMatched = false;
	if (_builtinCheatCaptureArmed) {
		phraseMatched = true;
		if (_builtinCheatCapture == "go snoids ") {
			setBuiltinDebugMode(true);
		} else if (_builtinCheatCapture == "midi ") {
			setBuiltinMidiTestMode(!_builtinMidiTestMode);
		} else {
			phraseMatched = false;
		}

		// An unmatched Space remains in the sequence. This is required for the
		// interior Space in "go snoids " to reach the final comparison.
		if (phraseMatched) {
			_builtinCheatCaptureArmed = false;
			_builtinCheatCapture.clear();
		}
	}

	if (phraseMatched || !_builtinMidiTestMode)
		return phraseMatched;

	playBuiltinMidiTestResource(kbd.hasFlags(Common::KBD_SHIFT));
	return true;
}

void MohawkEngine_Zoombini::setBuiltinDebugMode(bool enabled) {
	_state->_f.setDebugEnabled(enabled);
	_state->markDebugStateMutation();
	_state->markSaveBeforeQuitPending();
	if (!enabled) {
		_builtinDebug = ZmbBuiltinDebugRuntimeState();
		if (_activePage)
			_activePage->scheduleForceRedraw();
	}

	showBuiltinNotiBox(Common::U32String("you got it"), false);
}

void MohawkEngine_Zoombini::setBuiltinMidiTestMode(bool enabled) {
	_builtinMidiTestMode = enabled;
	if (enabled)
		_builtinMidiTestResourceIndex = 0;

	showBuiltinNotiBox(Common::U32String(enabled ? "midi test on" : "midi test off"), false);
}

uint16 MohawkEngine_Zoombini::getBuiltinMidiTestResource() const {
	return kBuiltinMidiTestResourceIds[_builtinMidiTestResourceIndex];
}

uint16 MohawkEngine_Zoombini::playBuiltinMidiTestResource(bool advance) {
	if (advance) {
		_builtinMidiTestResourceIndex += 1;
		if (ARRAYSIZE(kBuiltinMidiTestResourceIds) <= _builtinMidiTestResourceIndex)
			_builtinMidiTestResourceIndex = 0;
	}

	const uint16 resourceId = getBuiltinMidiTestResource();
	if (!_state || _state->getEnableMusic()) {
		if (isVersionFamilyTlcV2()) {
			const ZmbResource musicResource(ZmbResource::kPage, static_cast<int16>(resourceId));
			if (resourceId == kBuiltinTlcMidiTestSoundResourceId && hasResource(ID_SND, musicResource) && !_sound->isSoundPlaying(musicResource))
				_sound->playSound(musicResource, Audio::Mixer::kMusicSoundType);
		} else {
			if (advance || !_midi->isPlaying())
				_midi->playMidi(ZmbResource(ZmbResource::kPage, static_cast<int16>(resourceId)));
		}
	}

	const Common::U32String notificationText = Common::U32String::format("midi test:%u", resourceId);
	showBuiltinNotiBox(notificationText, true);
	return resourceId;
}

void MohawkEngine_Zoombini::showBuiltinNotiBox(const Common::U32String &text, bool persistent) {
	if (hasDialogOpened())
		return;

	ZoombiniInteractive *interactive = dynamic_cast<ZoombiniInteractive *>(getActivePage());
	if (!interactive)
		return;
	if (persistent)
		interactive->showNotiBoxLongPersistent(text);
	else
		interactive->showNotiBoxLong(text);
}

void MohawkEngine_Zoombini::beginQuitEvent(ZoombiniPage *page) {
	if (_quitEventState != kQuitEventNone)
		return;

	// Run the save-before-quit prompt before releasing the active puzzle.
	// Set the prompting state first so the modal dialog's nested frame loop cannot re-enter the quit path.
	_quitEventState = kQuitEventPrompting;
	askSaveBeforeQuit();
	if (_quitEventState != kQuitEventPrompting)
		return;

	if (page && page != _activePage) {
		page->onQuit();
		page->close();
	}

	if (_activePage) {
		_activePage->onQuit();
		_activePage->closeForQuit();
	}

	_quitEventState = kQuitEventRunning;
	_gfx->startMouseCursorEyeAnimation();
}

void MohawkEngine_Zoombini::processPendingQuitRequest() {
	switch (_pendingQuitRequest) {
	case kPendingQuitCloseOptions:
		if (_dialogPageStack.empty()) {
			_pendingQuitRequest = kPendingQuitFinalizing;
			Engine::quitGame();
		} else {
			_dialogPageStack.top()->close();
			_pendingQuitRequest = kPendingQuitWaitForOptionsClose;
		}
		break;
	case kPendingQuitWaitForOptionsClose:
		if (_dialogPageStack.empty()) {
			_pendingQuitRequest = kPendingQuitFinalizing;
			Engine::quitGame();
		}
		break;
	case kPendingQuitNone:
	case kPendingQuitFinalizing:
		break;
	}
}

bool MohawkEngine_Zoombini::shouldAskSaveBeforeQuit() const {
	// The runtime save-exit marker gates this prompt.
	// It is not limited to an unsaved new game: Basecamp setup also sets it after loading a save.
	if (isDemo() || !_state->isGameStateReady() || _state->inPracticeMode() ||
		!_state->needsSaveBeforeQuit() || !_dialogPageStack.empty() || !_activePage)
		return false;

	const ZoombiniPageType pageType = _activePage->getPageType();
	return ZoombiniPageType::kRodMap <= pageType && pageType <= ZoombiniPageType::kMaze;
}

void MohawkEngine_Zoombini::askSaveBeforeQuit() {
	if (!shouldAskSaveBeforeQuit())
		return;

	if (openMsgBoxDialog(ZoombiniMsgBoxType::kAskSaveBeforeQuit) == ZoombiniDialogResult::kYes)
		openSaveDialog();
}

void MohawkEngine_Zoombini::doFrame() {
	// Update background running things
	uint32 frameStartTime = _system->getMillis();
	bool isDialogOpened = !_dialogPageStack.empty();

	_sound->updateSoundQueue();

	// When idle > 3600 ticks (~60s at 60fps), halve the fidget threshold (minimum 1) to increase fidget frequency.
	// Modal time does not count toward this idle interval.
	if (_fidgetThreshold && !isDialogOpened) {
		uint32 curFrame = getAnimationFrameCounter(frameStartTime);
		if (3600 < curFrame - _lastFidgetIntervalFrame) {
			_lastFidgetIntervalFrame = curFrame;
			_fidgetThreshold /= 2;
			if (!_fidgetThreshold)
				_fidgetThreshold = 1;
		}
	}

	ZoombiniPage *page = nullptr;
	if (isDialogOpened)
		page = _dialogPageStack.top();
	else
		page = _activePage;

	processPendingQuitRequest();

	// Page or dialog callbacks may call @ref Engine::quitGame() after this frame's event polling has finished.
	// Enter the same close/fade state here so a closed page is never treated as a normal page transition.
	if (shouldQuit() && _quitEventState == kQuitEventNone)
		beginQuitEvent(page);

	// The debugger console is spawned in @ref Common::EventManager::pollEvent() when requested.
	processEvents(page);
	if (mustQuit())
		return;

	processPendingMainMenuSaveLoadDialog();
	if (mustQuit())
		return;
	if (page->isBuiltinDebugInputWaitActive()) {
		_gfx->flushScreens();
		_system->updateScreen();
		const uint32 waitElapsed = _system->getMillis() - frameStartTime;
		if (waitElapsed < kTargetFrameTimeMs)
			_system->delayMillis(kTargetFrameTimeMs - waitElapsed);
		return;
	}

	// @ref ZoombiniConsole::CmdSub_DrawMsgBox() closes the debugger before queuing this key.
	// Open the modal on the following frame so the debugger cannot cover it.
	if (_pendingTextMsgBoxKey != 0) {
		const uint32 textKey = _pendingTextMsgBoxKey;
		_pendingTextMsgBoxKey = 0;
		openConfirmMsgBoxDialog(_text->getLocalizedString(textKey));
	}
	if (mustQuit())
		return;

	// The underlying page freezes rendering, input, and puzzle controllers while a modal owns the frame.
	// Interactive pages retain only their ambient scheduler in this branch.
	if (isDialogOpened && _activePage)
		_activePage->onModalFrame();

	// Dialog-owned animation and audio continue through the selected page below.
	// Page frame update
	page->onFrame();

	// Copy any changed compositor regions before the backend presents this frame.
	_gfx->flushScreens();
	bool inFade = _gfx->applyFadeEffect(frameStartTime);
	// Process the page-loading cursor with the authored pose timing.
	if (_gfx->isMouseCursorEyeAnimationActive()) {
		uint32 currentFrame = getAnimationFrameCounter(frameStartTime);
		bool keepCursor = inFade;
		if (isVersionFamilyTlcV2() && currentFrame <= _pageLoadingCursorStopFrame)
			keepCursor = true;

		if (keepCursor)
			_gfx->runMouseCursorEyeAnimationFrame(currentFrame);
		else
			_gfx->stopMouseCursorEyeAnimation();
	}

	// The backend owns software-cursor dirtiness in addition to the page
	// compositor. Let it inspect and present both on each target-rate main-loop frame.
	_system->updateScreen();

	// Check if page is finished
	if (!inFade && page->isClosed()) {
		if (isDialogOpened) {
			closeActiveDialog();
			if (_quitEventState == kQuitEventRunning)
				_quitEventState = kQuitEventDone;
		} else {
			loadNextPage();
		}
		page = nullptr;
	}

	// Cut down on CPU usage
	uint32 loopElapsed = _system->getMillis() - frameStartTime;
	if (loopElapsed < kTargetFrameTimeMs)
		_system->delayMillis(kTargetFrameTimeMs - loopElapsed);
}

void MohawkEngine_Zoombini::delayRunningFrames(uint32 ms) {
	uint32 startTime = _system->getMillis();

	while (_system->getMillis() < startTime + ms && !mustQuit()) {
		doFrame();
	}
}

void MohawkEngine_Zoombini::initSearchPaths() {
	Common::FSNode candidate(ConfMan.getPath("path"));
	for (int depth = 0; depth < 4 && candidate.exists() && candidate.isDirectory(); depth++) {
		if (tryAddZoombiniIsoRootSearchPath(candidate))
			break;

		Common::FSNode parent = candidate.getParent();
		if (parent.getPath().equals(candidate.getPath()))
			break;
		candidate = parent;
	}
}

ZoombiniPage *MohawkEngine_Zoombini::getCurrentPage() const {
	if (!_dialogPageStack.empty())
		return _dialogPageStack.top();
	return _activePage;
}

void MohawkEngine_Zoombini::syncSoundSettings() {
	Engine::syncSoundSettings();

	const bool musicEnabled = !_state || _state->getEnableMusic();
	const bool sfxEnabled = !_state || _state->getEnableSound();
	if (!musicEnabled)
		_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, true);
	if (!sfxEnabled) {
		_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, true);
		_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, true);
	}

	if (_midi)
		_midi->syncBgmVolume(musicEnabled);
}

void MohawkEngine_Zoombini::applyGameSettings() {
	// The original_prng setting is intentionally not refreshed here.
	// @ref ZoombiniRandom samples it once at engine startup.
	// Changing the PRNG mid-run can alter puzzle algorithms.
	const bool brightenPalette = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionBrightenPalette);
	if (_brightenPalette != brightenPalette) {
		_brightenPalette = brightenPalette;
		if (_gfx) {
			_gfx->refreshPalette();
			ZoombiniPage *page = getCurrentPage();
			if (page)
				page->scheduleForceRedraw();
		}
	}

	const bool useAccurate60FPS = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionUseAccurate60FPS);
	if (_useAccurate60FPS != useAccurate60FPS) {
		const uint32 now = _system->getMillis();
		const uint32 currentFrame = getAnimationFrameCounter(now);
		_animationClockEpochTimeMs = now;
		_animationClockEpochFrame = currentFrame;
		_useAccurate60FPS = useAccurate60FPS;
		if (useAccurate60FPS)
			debug(1, "engine: using exact 60fps animation clock");
		else
			debug(1, "engine: using %u ms integer animation tick", kOriginalAnimateFrameTimeMs);

		ZoombiniPage *page = getCurrentPage();
		if (page)
			page->scheduleForceRedraw();
	}

	const bool colorBlindMode = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionColorBlindMode);
	if (_colorBlindMode != colorBlindMode) {
		_colorBlindMode = colorBlindMode;
		ZoombiniPage *page = getCurrentPage();
		if (page)
			page->scheduleForceRedraw();
	}

	_enhancedKbdShortcuts = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionEnhancedKbdShortcuts);
}

Common::KeymapArray MohawkEngine_Zoombini::initKeymaps(const char *target) {
	using namespace Common;

	Keymap *defaultKeymap = new Keymap(Keymap::kKeymapTypeGame, "engine-default", _("Default game keymappings"));
	defaultKeymap->setPartialMatchAllowed(false);

	// The global keymap supplies left-stick cursor movement and RB cursor slowdown.
	// Start remains reserved for the global menu.
	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMiddleClick, _("Middle click"));
	act->setMiddleClickEvent();
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionOpenMainMenu, _("Ingame menu"));
	// Keep F5 as a page-visible key so original page handlers can own it first.
	// @ref MohawkEngine_Zoombini::processEvent opens the menu when the page leaves it unconsumed.
	act->setKeyEvent(KEYCODE_F5);
	act->addDefaultInputMapping("F5");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	// Keep every "zoombini-keymap" context argument as a string literal.
	// xgettext cannot extract contextual translations when _c() receives
	// a variable or named constant.
	act = new Action(MohawkMetaEngine_Zoombini::kActionConfirm, _c("OK", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	Keymap *shortcutKeymap = new Keymap(Keymap::kKeymapTypeGame, "zoombini", _c("Ingame Shortcuts", "zoombini-keymap"));
	shortcutKeymap->setPartialMatchAllowed(false);

	// Remappable shortcuts emit their original key events instead of changing engine state here.
	// The active page or dialog remains the authority for release, context, and state checks.
	act = new Action(kStandardActionOpenSettings, _c("Ingame Options", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_SLASH, '/'));
	act->addDefaultInputMapping("SLASH");
	act->addDefaultInputMapping("QUESTION");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleDialogAndSfx, _c("Toggle SFX", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_d, 'd', KBD_CTRL));
	act->addDefaultInputMapping("C+d");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleMusic, _c("Toggle BGM", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_b, 'b', KBD_CTRL));
	act->addDefaultInputMapping("C+b");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleStickyMouse, _c("Toggle sticky mouse", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_j, 'j', KBD_CTRL));
	act->addDefaultInputMapping("C+j");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleAutoStickyMouse, _c("Toggle auto sticky mouse", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_u, 'u', KBD_CTRL));
	act->addDefaultInputMapping("C+u");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleTransitions, _c("Toggle transitions", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_t, 't', KBD_CTRL));
	act->addDefaultInputMapping("C+t");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleLessMoreAction, _c("Toggle action level", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_g, 'g', KBD_CTRL));
	act->addDefaultInputMapping("C+g");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionToggleCursor, _c("Toggle cursor", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_h, 'h', KBD_CTRL));
	act->addDefaultInputMapping("C+h");
	shortcutKeymap->addAction(act);

	// F1 replays the puzzle instruction voice selected for the current page.
	act = new Action(MohawkMetaEngine_Zoombini::kActionReplayPuzzleInstructions, _c("Replay puzzle instructions SFX", "zoombini-keymap"));
	act->setKeyEvent(KEYCODE_F1);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_RIGHT_STICK");
	shortcutKeymap->addAction(act);

	String guiOptions = ConfMan.get("guioptions", target);
	if (checkGameGUIOption(GAMEOPTION_ZMB_V20, guiOptions)) {
		// TLC v2 adds page narration to the Help dialog and exposes its Ctrl+A toggle.
		act = new Action(MohawkMetaEngine_Zoombini::kActionToggleHelpDialogNarration, _c("Toggle Help dialog narration", "zoombini-keymap"));
		act->setKeyEvent(KeyState(KEYCODE_a, 'a', KBD_CTRL));
		act->addDefaultInputMapping("C+a");
		shortcutKeymap->addAction(act);

		// ScummVM preserves the TLC v2 toggle state and notifications, but does
		// not implement TouchSense hardware feedback.
		act = new Action(MohawkMetaEngine_Zoombini::kActionToggleTouchSense, _c("Toggle Immersion TouchSense (not implemented)", "zoombini-keymap"));
		act->setKeyEvent(KeyState(KEYCODE_k, 'k', KBD_CTRL));
		act->addDefaultInputMapping("C+k");
		shortcutKeymap->addAction(act);
	}

	act = new Action(MohawkMetaEngine_Zoombini::kActionPracticeMode, _c("Toggle practice mode (Map)", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_p, 'p', KBD_CTRL));
	act->addDefaultInputMapping("C+p");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionNewGame, _c("New game", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_n, 'n', KBD_CTRL));
	act->addDefaultInputMapping("C+n");
	shortcutKeymap->addAction(act);

	act = new Action(kStandardActionLoad, _c("Load game", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_l, 'l', KBD_CTRL));
	act->addDefaultInputMapping("C+l");
	shortcutKeymap->addAction(act);

	act = new Action(kStandardActionSave, _c("Save game", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_s, 's', KBD_CTRL));
	act->addDefaultInputMapping("C+s");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionRemoveSave, _c("Delete game", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_r, 'r', KBD_CTRL));
	act->addDefaultInputMapping("C+r");
	shortcutKeymap->addAction(act);

	act = new Action(MohawkMetaEngine_Zoombini::kActionQuit, _c("Quit", "zoombini-keymap"));
	act->setKeyEvent(KeyState(KEYCODE_q, 'q', KBD_CTRL));
	act->addDefaultInputMapping("C+q");
	shortcutKeymap->addAction(act);

	KeymapArray keymaps;
	keymaps.push_back(defaultKeymap);
	keymaps.push_back(shortcutKeymap);
	return keymaps;
}

bool MohawkEngine_Zoombini::useEnhancedKbdShortcuts() const {
	return _enhancedKbdShortcuts;
}

void MohawkEngine_Zoombini::initializeMazeLayoutVariants() {
	ZoombiniGameState::MazeLayoutVariantState &variants = _state->getMazeLayoutVariantState();
	// Latch this state even with the option disabled.
	// A later option change must not alter an active engine session.
	if (variants._initialVariantsSelected)
		return;
	variants._initialVariantsSelected = true;

	const bool randomizeInitialLayout = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionMazeRandomizeInitialLayout);
	if (randomizeInitialLayout) {
		// Use system time instead of ZoombiniRandom so this selection does not affect/consume gameplay RNG state.
		const uint32 entryMillis = _system->getMillis();
		const int16 baseAltVariant = static_cast<int16>(entryMillis % 2);
		// Practice mode can switch levels after entry, so each level needs its own initialized selector.
		// Levels 1 through 3 use the same Base/Alt selection.
		variants._level1 = baseAltVariant;
		variants._level2 = baseAltVariant;
		variants._level3 = baseAltVariant;
		if (ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionMazeRestoreUnusedL4Layout)) {
			// Level 4 uses 0 for Base, 1 for Restored, and 2 for Alt.
			variants._level4 = static_cast<int16>(entryMillis % 3);
		} else {
			// Without Restored, map the shared Alt choice from 1 to Level 4's index 2.
			variants._level4 = static_cast<int16>(baseAltVariant * 2);
		}
	}
}

uint32 MohawkEngine_Zoombini::getAnimationFrameCounter(uint32 timeMs) const {
	// Let's avoid floating-point math, since ScummVM can be ported to everywhere.
	const uint32 elapsed = timeMs - _animationClockEpochTimeMs;
	if (_useAccurate60FPS) { // 16.6667ms mode (perfect 60FPS for modern displays)
		return _animationClockEpochFrame + static_cast<uint32>((static_cast<uint64>(elapsed) * kAnimateFrameRate) / kAnimationClockTimeBaseMs);
	} else { // 17ms mode (faithful to original Zoombini engine, 58.8235FPS)
		return _animationClockEpochFrame + elapsed / kOriginalAnimateFrameTimeMs;
	}
}

MohawkArchive *MohawkEngine_Zoombini::loadSystemArchive() {
	MohawkArchive *mhkArchive = new MohawkArchive();

	Common::Path mhkPath = Common::Path(_mhkArchiveRoot).append(ZMB_MHK_ZOOMBINI);
	if (mhkArchive->openFile(mhkPath))
		return mhkArchive;

	delete mhkArchive;
	error("engine: cannot open or parse required resource file '%s'", mhkPath.toString().c_str());
	return nullptr;
}

MohawkArchive *MohawkEngine_Zoombini::loadHelpArchive() {
	MohawkArchive *mhkArchive = new MohawkArchive();
	if (!mhkArchive->openFile(Common::Path(ZMB_MHK_HELP))) {
		delete mhkArchive;
		warning("engine: cannot open TLC help voice archive '%s'", ZMB_MHK_HELP);
		return nullptr;
	}

	return mhkArchive;
}

MohawkArchive *MohawkEngine_Zoombini::loadMidiArchive() {
	if (!isVersionFamilyV1())
		return nullptr;

	const bool useMac = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionUseMacMidi);
	if (_midiMhk && _midiMhkUsesMacProfile == useMac)
		return _midiMhk;
	if (_midiMhk) {
		delete _midiMhk;
		_midiMhk = nullptr;
	}
	const char *archiveName = useMac ? ZMB_MHK_MIDIMAC : ZMB_MHK_MIDIMPC;
	const Common::Path archivePath = Common::Path(_mhkArchiveRoot).append(archiveName);
	MohawkArchive *mhkArchive = new MohawkArchive();
	if (!mhkArchive->openFile(archivePath)) {
		delete mhkArchive;
		error("engine: cannot open or parse required resource file '%s'", archivePath.toString().c_str());
		return nullptr;
	}

	_midiMhk = mhkArchive;
	_midiMhkUsesMacProfile = useMac;
	return _midiMhk;
}

void MohawkEngine_Zoombini::loadNextPage() {
	if (_quitEventState == kQuitEventRunning) {
		_quitEventState = kQuitEventDone;
		return;
	}

	const bool replacingPage = _activePage != nullptr;
	if (_activePage) {
		delete _activePage;
		_activePage = nullptr;
		updateImeCompositionState();
	}
	_gfx->clearScreens();
	// GraphicsManager's page-image caches are keyed only by the 16-bit resource id.
	// Page archives reuse those ids, so invalidate both page tBMP caches between page replacement.
	// @ref ZoombiniGraphics keeps system-archive images in separate caches.
	_gfx->clearCache();

	if (_pageQueue.empty() && shouldQuit()) {
		_quitEventState = kQuitEventDone;
		return;
	}

	if (replacingPage) {
		_gfx->startMouseCursorEyeAnimation();
		_pageLoadingCursorStopFrame = 0;
		// Advance the persistent busy-cursor schedule at the page boundary before
		// synchronous archive and page initialization begins.
		const uint32 currentFrame = getAnimationFrameCounter(_system->getMillis());
		_gfx->runMouseCursorEyeAnimationFrame(currentFrame);
		// Present the black loading boundary with the pose selected by that tick.
		_system->updateScreen();
	}

	// Cache the setting before the page loads its first palette.
	_brightenPalette = ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionBrightenPalette);

	assert(!_pageQueue.empty());
	ZoombiniPageType nextPageType = _pageQueue.pop();

	ZoombiniPage *page;
	switch (nextPageType) {
	case ZoombiniPageType::kLogo:
		page = new ZoombiniTransitionLogo(this);
		break;
	case ZoombiniPageType::kRodMap:
		page = new ZoombiniInteractiveRodMap(this);
		break;
	case ZoombiniPageType::kXfer:
		page = new ZoombiniTransitionXfer(this);
		break;
	case ZoombiniPageType::kPicker:
		page = new ZoombiniShelterPicker(this);
		break;
	case ZoombiniPageType::kBasecamp1:
		page = new ZoombiniShelterBasecampOne(this);
		break;
	case ZoombiniPageType::kTown:
		page = new ZoombiniShelterTown(this);
		break;
	case ZoombiniPageType::kBasecamp2:
		page = new ZoombiniShelterBasecampTwo(this);
		break;
	case ZoombiniPageType::kBridge:
		page = new ZoombiniPuzzleBridge(this);
		break;
	case ZoombiniPageType::kCaves:
		page = new ZoombiniPuzzleCaves(this);
		break;
	case ZoombiniPageType::kPizza:
		page = new ZoombiniPuzzlePizza(this);
		break;
	case ZoombiniPageType::kFerry:
		page = new ZoombiniPuzzleFerry(this);
		break;
	case ZoombiniPageType::kLilly:
		page = new ZoombiniPuzzleLilly(this);
		break;
	case ZoombiniPageType::kSlides:
		page = new ZoombiniPuzzleSlides(this);
		break;
	case ZoombiniPageType::kFleens:
		page = new ZoombiniPuzzleFleens(this);
		break;
	case ZoombiniPageType::kHotel:
		page = new ZoombiniPuzzleHotel(this);
		break;
	case ZoombiniPageType::kNet:
		// Demo Net starts each page instance with a fresh active pack.
		if (isDemo())
			_state->generateRandomPack();
		page = new ZoombiniPuzzleNet(this);
		break;
	case ZoombiniPageType::kTunnels:
		page = new ZoombiniPuzzleTunnels(this);
		break;
	case ZoombiniPageType::kSmoke:
		page = new ZoombiniPuzzleSmoke(this);
		break;
	case ZoombiniPageType::kMaze:
		page = new ZoombiniPuzzleMaze(this);
		break;
	default:
		error("engine: invalid queued page: %d", static_cast<int32>(nextPageType));
		break;
	}

	_activePage = page;
	if (page->getPageCategory() == ZoombiniPageCategory::kInteractive)
		_state->_f.setCurrentPageType(nextPageType);

	// Perfect streak flag.
	if (_state->inPracticeMode()) {
		// In practice mode, always clear.
		_state->_perfectStreakFlag = false;
	} else {
		// Set to true when entering the first puzzle of a route.
		// Do not touch it in other puzzles.
		switch (nextPageType) {
		case ZoombiniPageType::kBridge:
		case ZoombiniPageType::kFerry:
		case ZoombiniPageType::kFleens:
		case ZoombiniPageType::kCaves:
			_state->_perfectStreakFlag = true;
			break;
		default:
			break;
		}
	}

	page->open();
	page->setBackgroundMusic();
	page->setBackgroundBitmap();
	page->initStates();
	page->loadFeatures();
	page->initSfxGroupFlags();
	if (page->requiresFidgetActivityResetOnLoad())
		resetFidgetActivity();
	page->initHelpPrompt();
	page->activatePage();

	if (replacingPage && !_gfx->isMouseCursorEyeAnimationActive()) {
		_gfx->startMouseCursorEyeAnimation();
	}
	page->onFadeIn();
	if (replacingPage && isVersionFamilyTlcV2()) {
		uint32 currentFrame = getAnimationFrameCounter(_system->getMillis());
		_pageLoadingCursorStopFrame = currentFrame + kTlcV2BusyCursorStopDelay;
	} else if (replacingPage && !_gfx->isFading()) {
		_gfx->stopMouseCursorEyeAnimation();
	}
	updateImeCompositionState();
}

void MohawkEngine_Zoombini::addPageArchive(Archive *archive) {
	_mhk.push_back(archive);
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "engine: added page archive; active archive count is %u", _mhk.size());
}

void MohawkEngine_Zoombini::removePageArchive(Archive *archive) {
	for (uint i = 0; i < _mhk.size(); i++) {
		if (archive != _mhk[i])
			continue;
		_mhk.remove_at(i);
		delete archive;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "engine: removed page archive; active archive count is %u", _mhk.size());
		return;
	}

	error("engine: removeArchive didn't find archive %p, now size: %u", reinterpret_cast<void *>(archive), _mhk.size());
	return;
}

void MohawkEngine_Zoombini::clearPageArchives() {
	MohawkEngine::closeAllArchives();
}

bool MohawkEngine_Zoombini::isVersionFamilyEuV1() const {
	return isGameVariant(MohawkGameFeatures::GF_ZMB_10_EU) || isGameVariant(MohawkGameFeatures::GF_ZMB_11_EU) ||
		   isGameVariant(MohawkGameFeatures::GF_ZMB_11_FR_2002) || isGameVariant(MohawkGameFeatures::GF_ZMB_10_ESPT);
}

bool MohawkEngine_Zoombini::isVersionFamilyUsV1() const {
	return isGameVariant(MohawkGameFeatures::GF_ZMB_11_US);
}

bool MohawkEngine_Zoombini::isVersionFamilyTlcV2() const {
	return isGameVariant(MohawkGameFeatures::GF_ZMB_20_US);
}

bool MohawkEngine_Zoombini::isVersionFamilyV1() const {
	return isVersionFamilyEuV1() || isVersionFamilyUsV1();
}

bool MohawkEngine_Zoombini::hasRoutePerfectCounterState() const {
	return isVersionFamilyUsV1() || isVersionFamilyTlcV2();
}

const char *MohawkEngine_Zoombini::getVersionFamilyName() const {
	if (isVersionFamilyTlcV2())
		return "TLCv2";
	if (isVersionFamilyUsV1())
		return "USv1";
	if (isVersionFamilyEuV1())
		return "EUv1";
	return "unknown";
}

bool MohawkEngine_Zoombini::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

bool MohawkEngine_Zoombini::consumeDemoStartupLogoReveal() {
	if (!isDemo() || !_demoStartupLogoRevealPending)
		return false;

	_demoStartupLogoRevealPending = false;
	return true;
}

Common::Language MohawkEngine_Zoombini::getLanguage() const {
	Common::Language language = MohawkEngine::getLanguage();
	if (language == Common::UNK_LANG)
		language = Common::EN_USA;
	return language;
}

void MohawkEngine_Zoombini::setNextPage(ZoombiniPageType type) {
	// The transition option skips the XFER scene during normal progression.
	if (type == ZoombiniPageType::kXfer &&
		!_debugPreserveActivePackOnXferClose && !_state->getEnableTransitions()) {
		const ZmbXferRouteInfo *routeInfo = ZmbXferRouteInfo::getZmbXferRouteInfo(_xferSrcPage);
		if (routeInfo) {
			const ZmbDestPageKind destPage = routeInfo->destPage;
			_state->markXferContainerArrival(_xferSrcPage, *routeInfo);

			_xferSrcPage = ZmbSrcPageKind::kMinus1;
			type = static_cast<ZoombiniPageType>(destPage);
		}
	}

	// Mark normal interactive page changes as needing the save-before-quit prompt.
	// The load path consumes a one-transition guard.
	// Basecamp 1 and Basecamp 2 then set the marker again in their own setup routines.
	if (_activePage && _activePage->getPageCategory() == ZoombiniPageCategory::kInteractive && !_state->inPracticeMode()) {
		if (!_state->consumeSaveBeforeQuitMarkSuppression())
			_state->markSaveBeforeQuitPending();
	}

	_pageQueue.clear();
	_pageQueue.push(type);
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "engine: queued page type %d", static_cast<int>(type));

	// A page transition replaces the original engine's shared page and dialog runners.
	// ScummVM dialogs instead own nested synchronous loops, so mark every modal layer
	// closed and let those loops unwind without deleting a dialog from its callback.
	for (uint32 i = 0; i < _dialogPageStack.size(); i++)
		_dialogPageStack[i]->close();
}

bool MohawkEngine_Zoombini::hasDialogOpened() const {
	return !_dialogPageStack.empty();
}

bool MohawkEngine_Zoombini::hasSaveLoadDialogOpened() const {
	for (uint32 i = 0; i < _dialogPageStack.size(); i++) {
		if (_dialogPageStack[i]->getPageType() == ZoombiniPageType::kDialogSaveLoad)
			return true;
	}

	return false;
}

void MohawkEngine_Zoombini::openOptionsDialog() {
	ZoombiniDialog *dialogPage = new ZoombiniDialogOptions(this);
	loadModalDialog(dialogPage);
}

void MohawkEngine_Zoombini::requestQuit() {
	if (_quitEventState != kQuitEventNone || _pendingQuitRequest != kPendingQuitNone || shouldQuit())
		return;

	// Open the quit confirmation only for the Options dialog's Quit button.
	// Ctrl+Q and window-close events do not open this first confirmation.
	if (_dialogPageStack.empty() || _dialogPageStack.top()->getPageType() != ZoombiniPageType::kDialogOptions)
		return;

	if (openMsgBoxDialog(ZoombiniMsgBoxType::kAskReallyQuit) != ZoombiniDialogResult::kYes)
		return;

	// Close Options after confirmation, then request shutdown once the matching ScummVM dialog has closed.
	_pendingQuitRequest = kPendingQuitCloseOptions;
}

void MohawkEngine_Zoombini::requestMainMenuSaveLoadDialog(bool loadDialog) {
	_pendingMainMenuSaveLoadDialog = loadDialog ? kPendingMainMenuLoadDialog : kPendingMainMenuSaveDialog;
}

void MohawkEngine_Zoombini::processPendingMainMenuSaveLoadDialog() {
	const PendingMainMenuSaveLoadDialog pendingDialog = _pendingMainMenuSaveLoadDialog;
	_pendingMainMenuSaveLoadDialog = kPendingMainMenuSaveLoadNone;

	if (pendingDialog == kPendingMainMenuSaveDialog)
		openSaveDialog();
	else if (pendingDialog == kPendingMainMenuLoadDialog)
		openLoadDialogWithSavePrompt();
}

bool MohawkEngine_Zoombini::canOpenSaveLoadDialog() const {
	return _activePage && _activePage->canOpenSaveLoadDialog();
}

ZoombiniDialogResult MohawkEngine_Zoombini::openSaveDialog() {
	if (hasSaveLoadDialogOpened())
		return ZoombiniDialogResult::kNo;

	if (!canOpenSaveLoadDialog()) {
		showSaveLoadUnavailableMessage();
		return ZoombiniDialogResult::kNo;
	}

	if (_state->inPracticeMode()) {
		openMsgBoxDialog(ZoombiniMsgBoxType::kAlertCannotSaveInPractice);
		return ZoombiniDialogResult::kNo;
	}

	if (_state->hasUnsafeSyntheticDebugState()) {
		openMsgBoxDialog(Common::U32String("Saving is disabled because Shift+Generate created a synthetic debug state. "
										   "Start a new game or load a saved game first."));
		return ZoombiniDialogResult::kNo;
	}

	if (_state->requiresSaveConfirmationForDebugStateMutation()) {
		ZoombiniDialogResult res = openConfirmMsgBoxDialog(_text->getLocalizedString(ZoombiniText::kSvmDialogBodyAskSaveAfterDebugStateMutation));
		if (res != ZoombiniDialogResult::kYes)
			return ZoombiniDialogResult::kNo;
	}

	ZoombiniDialog *dialogPage = new ZoombiniDialogSaveLoad(this, ZoombiniDialogSaveLoad::kSaveMode);
	return loadModalDialog(dialogPage);
}

ZoombiniDialogResult MohawkEngine_Zoombini::openLoadDialog(bool newGameMode) {
	if (hasSaveLoadDialogOpened())
		return ZoombiniDialogResult::kNo;

	if (!canOpenSaveLoadDialog()) {
		showSaveLoadUnavailableMessage();
		return ZoombiniDialogResult::kNo;
	}

	if (_state->inPracticeMode()) {
		openMsgBoxDialog(ZoombiniMsgBoxType::kAlertCannotLoadInPractice);
		return ZoombiniDialogResult::kNo;
	}

	if (_state->_r.getEntryCount() == 0) {
		openMsgBoxDialog(ZoombiniMsgBoxType::kAlertNoSavedGame);
		return ZoombiniDialogResult::kNo;
	}

	ZoombiniDialogSaveLoad::SaveLoadMode mode;
	if (newGameMode)
		mode = ZoombiniDialogSaveLoad::kLoadOrNewMode;
	else
		mode = ZoombiniDialogSaveLoad::kLoadMode;
	ZoombiniDialog *dialogPage = new ZoombiniDialogSaveLoad(this, mode);
	return loadModalDialog(dialogPage);
}

ZoombiniDialogResult MohawkEngine_Zoombini::openLoadDialogWithSavePrompt() {
	if (hasSaveLoadDialogOpened())
		return ZoombiniDialogResult::kNo;

	if (!canOpenSaveLoadDialog()) {
		showSaveLoadUnavailableMessage();
		return ZoombiniDialogResult::kNo;
	}

	if (_state->_f._zmbGeneratedCount != 0 &&
		openMsgBoxDialog(ZoombiniMsgBoxType::kAskSaveCurrentGame) == ZoombiniDialogResult::kYes)
		openSaveDialog();

	return openLoadDialog();
}

ZoombiniDialogResult MohawkEngine_Zoombini::openMsgBoxDialog(ZoombiniMsgBoxType type) {
	ZoombiniDialog *dialogPage = new ZoombiniDialogMsgBox(this, type);
	return loadModalDialog(dialogPage);
}

ZoombiniDialogResult MohawkEngine_Zoombini::openMsgBoxDialog(const Common::U32String &message) {
	ZoombiniDialog *dialogPage = new ZoombiniDialogMsgBox(this, message);
	return loadModalDialog(dialogPage);
}

ZoombiniDialogResult MohawkEngine_Zoombini::openConfirmMsgBoxDialog(const Common::U32String &message) {
	ZoombiniDialog *dialogPage = new ZoombiniDialogMsgBox(this, message, ZoombiniText::kDialogButtonYes, ZoombiniText::kDialogButtonNo);
	return loadModalDialog(dialogPage);
}

void MohawkEngine_Zoombini::requestMsgBoxDialog(uint32 textKey) {
	// Keep the text key until the next frame has closed the debugger console.
	_pendingTextMsgBoxKey = textKey;
}

void MohawkEngine_Zoombini::openCreditsDialog() {
	ZoombiniDialog *dialogPage = new ZoombiniDialogCredits(this);
	loadModalDialog(dialogPage);
}

void MohawkEngine_Zoombini::openHelpDialog(ZoombiniPageType forPage) {
	if (!supportsOnScreenHelp())
		return;

	ZoombiniDialog *dialogPage = new ZoombiniDialogHelp(this, forPage);
	loadModalDialog(dialogPage);
}

void MohawkEngine_Zoombini::requestPickerUpdateHelp() {
	_pickerUpdateHelpPending = true;
}

bool MohawkEngine_Zoombini::consumePickerUpdateHelpRequest() {
	const bool result = _pickerUpdateHelpPending;
	_pickerUpdateHelpPending = false;
	return result;
}

bool MohawkEngine_Zoombini::supportsOnScreenHelp() const {
	// The Europe v1.x executables does not have the onscreen Help button and dialog.
	// This feature was introduced since the US v1.1 version.
	return !isVersionFamilyEuV1();
}

void MohawkEngine_Zoombini::openDebugDialog(const ZoombiniDebugCommand &cmd) {
	ZoombiniDialog *dialogPage = new ZoombiniDialogDebug(this, cmd);
	loadModalDialog(dialogPage);
}

ZoombiniDialogResult MohawkEngine_Zoombini::loadModalDialog(ZoombiniDialog *dialogPage) {
	// Pause every active non-music SND instance except for the standard SND 999 button click.
	// The exact streams resume when this dialog closes.
	// A dialog may additionally pause a page music stream that it must resume on close.
	_sound->pauseActiveSoundsForDialog();
	// Modal input uses the standard cursor instead of a page-local hover cursor.
	ZoombiniPage *inputPage = _dialogPageStack.empty() ? _activePage : _dialogPageStack.top();
	if (inputPage)
		inputPage->clearHoverCursor();
	_cursor->setDefaultCursor();
	dialogPage->open();
	dialogPage->setBackgroundBitmap();
	dialogPage->loadFeatures();
	if (!_dialogPageStack.empty()) {
		const Common::ImeComposition composition(Common::ImeComposition::kCancelled);
		_dialogPageStack.top()->onImeComposition(composition);
	}
	_dialogPageStack.push(dialogPage);
	updateImeCompositionState();

	// Loop on a dialog page until the dialog is closed.
	// Do not loop on DebugDialog, as looping here make a debugger console blocking the screen.
	if (dialogPage->getPageType() != ZoombiniPageType::kDialogDebug) {
		uint32 dialogStackSize = _dialogPageStack.size();
		while (!mustQuit() && _dialogPageStack.size() == dialogStackSize)
			doFrame();
	}

	ZoombiniDialogResult result = _lastDialogResult;
	_lastDialogResult = ZoombiniDialogResult::kNone;
	return result;
}

void MohawkEngine_Zoombini::closeActiveDialog() {
	if (_dialogPageStack.empty()) {
		error("engine: there is no modal dialog opened");
		return;
	}

	ZoombiniDialog *dialogPage = _dialogPageStack.pop();
	_lastDialogResult = dialogPage->getResult();
	assert(dialogPage != nullptr);
	delete dialogPage;
	_sound->resumeActiveSoundsAfterDialog();
	resetFidgetActivity();
	updateImeCompositionState();

	if (_dialogPageStack.empty()) {
		// Force a full redraw of the underlying page
		// so dialog-painted pixels on the persistent shape screen are overwritten.
		if (_activePage)
			_activePage->scheduleForceRedraw();
	}
}

void MohawkEngine_Zoombini::updateImeCompositionState() {
	ZoombiniPage *inputPage = _dialogPageStack.empty() ? _activePage : _dialogPageStack.top();
	const bool enable = inputPage && inputPage->wantsImeComposition();
	g_system->setFeatureState(OSystem::kFeatureImeComposition, enable);
}

ZmbResource::ArchiveKind MohawkEngine_Zoombini::setActiveResourceKind(ZmbResource::ArchiveKind kind) {
	ZmbResource::ArchiveKind lastKind = _activeResourceKind;
	_activeResourceKind = kind;
	return lastKind;
}

ZmbResource MohawkEngine_Zoombini::setActiveResource(ZmbResource resource) {
	ZmbResource lastResource = _activeResource;
	_activeResource = resource;
	return lastResource;
}

ZmbResource MohawkEngine_Zoombini::resolveLegacySoundResource(int16 soundId) const {
	if (soundId < 1000 || 20000 <= soundId)
		return ZmbResource(ZmbResource::kSystem, soundId);

	return ZmbResource(ZmbResource::kPage, soundId);
}

Common::SeekableReadStream *MohawkEngine_Zoombini::getResource(uint32 tag, uint16 id) {
	if (INT16_MAX < id) {
		error("engine: resource id %u is outside the authored int16 range", id);
		return nullptr;
	}

	const int16 zmbId = static_cast<int16>(id);
	if (tag == ID_SND && _activeResource.hasId() && _activeResource._id == zmbId)
		return getResource(tag, _activeResource);

	if (tag == ID_SND)
		return getResource(tag, resolveLegacySoundResource(zmbId));

	return getResource(tag, ZmbResource(_activeResourceKind, zmbId));
}

Common::SeekableReadStream *MohawkEngine_Zoombini::getResource(uint32 tag, ZmbResource res) {
	// Pre-check resource existence so the failure message identifies the searched archive.
	// @ref ZmbResource::kPage selects the current page archive.
	// @ref ZmbResource::kSystem selects ZOOMBINI.MHK and the optional TLC HELP.MHK.
	// The Mohawk base-class error path reports only "Archive does not contain 'XXXX' NNNN!".
	// Without the archive name, the caller cannot distinguish a wrong target from a missing resource.
	const Common::String tagString = Common::tag2string(tag, true);
	if (res._id < 0) {
		error("engine: negative resource id %d", res._id);
		return nullptr;
	}
	const uint16 archiveId = static_cast<uint16>(res._id);
	switch (res._archiveKind) {
	case ZmbResource::kSystem:
		if (_sysMhk && _sysMhk->hasResource(tag, archiveId))
			return _sysMhk->getResource(tag, archiveId);
		if (_helpMhk && _helpMhk->hasResource(tag, archiveId))
			return _helpMhk->getResource(tag, archiveId);
		error("engine: required resource '%s' id %d (0x%04x) not found in kSystem (ZOOMBINI.MHK/HELP.MHK)",
			  tagString.c_str(), res._id, res._id);
		return nullptr;
	case ZmbResource::kPage:
		if (tag == ID_TMID) {
			MohawkArchive *midiMhk = loadMidiArchive();
			if (midiMhk && midiMhk->hasResource(tag, archiveId))
				return midiMhk->getResource(tag, archiveId);
		}
		if (MohawkEngine::hasResource(tag, archiveId))
			return MohawkEngine::getResource(tag, archiveId);
		error("engine: required resource '%s' id %d (0x%04x) not found in kPage (current page/shared MIDI archive)",
			  tagString.c_str(), res._id, res._id);
		return nullptr;
	default:
		error("engine: invalid archive kind: %u", static_cast<uint32>(res._archiveKind));
		break;
	}
	return nullptr;
}

bool MohawkEngine_Zoombini::hasResource(uint32 tag, ZmbResource res) {
	if (res._id < 0)
		return false;
	const uint16 archiveId = static_cast<uint16>(res._id);
	switch (res._archiveKind) {
	case ZmbResource::kSystem:
		return (_sysMhk && _sysMhk->hasResource(tag, archiveId)) || (_helpMhk && _helpMhk->hasResource(tag, archiveId));
	case ZmbResource::kPage:
		if (tag == ID_TMID) {
			MohawkArchive *midiMhk = loadMidiArchive();
			if (midiMhk && midiMhk->hasResource(tag, archiveId))
				return true;
		}
		return MohawkEngine::hasResource(tag, archiveId);
	default:
		warning("Invalid ZmbResourceKind: %u", static_cast<uint32>(res._archiveKind));
		break;
	}
	return false;
}

Common::Array<int16> MohawkEngine_Zoombini::getResourceIDList(ZmbResource::ArchiveKind kind, uint32 tag) const {
	Common::Array<int16> ids;
	Common::Array<uint16> archiveIds;

	switch (kind) {
	case ZmbResource::kSystem:
		if (_sysMhk)
			archiveIds.push_back(_sysMhk->getResourceIDList(tag));
		if (_helpMhk)
			archiveIds.push_back(_helpMhk->getResourceIDList(tag));
		break;
	case ZmbResource::kPage:
		if (tag == ID_TMID && _midiMhk)
			archiveIds.push_back(_midiMhk->getResourceIDList(tag));
		for (Mohawk::Archive *mhk : _mhk)
			archiveIds.push_back(mhk->getResourceIDList(tag));
		break;
	default:
		error("engine: invalid archive kind: %u", static_cast<uint32>(kind));
		break;
	}

	for (uint16 archiveId : archiveIds) {
		if (0x7FFF < archiveId) {
			error("engine: resource id %u is outside the authored int16 range", archiveId);
			return ids;
		}
		ids.push_back(static_cast<int16>(archiveId));
	}

	return ids;
}

const ZmbScriptDecoder::DecodedScrb *MohawkEngine_Zoombini::getSystemDecodedScrb(int16 scrbResId) {
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrb>::iterator it = _systemDecodedScrbCache.find(scrbResId);
	if (it != _systemDecodedScrbCache.end())
		return &it->second;

	ZmbScriptDecoder::DecodedScrb decodedScrb;
	if (!ZmbScriptDecoder::decodeScrbResource(this, ZmbResource(ZmbResource::kSystem, scrbResId), decodedScrb))
		return nullptr;

	ZmbScriptDecoder::DecodedScrb &cachedScrb = _systemDecodedScrbCache[scrbResId];
	cachedScrb = decodedScrb;
	return &cachedScrb;
}

const ZmbScriptDecoder::DecodedScrs *MohawkEngine_Zoombini::getSystemDecodedScrs(int16 scrsResId) {
	Common::StableMap<int16, ZmbScriptDecoder::DecodedScrs>::iterator it = _systemDecodedScrsCache.find(scrsResId);
	if (it != _systemDecodedScrsCache.end())
		return &it->second;

	ZmbScriptDecoder::DecodedScrs decodedScrs;
	if (!ZmbScriptDecoder::decodeScrsResource(this, ZmbResource(ZmbResource::kSystem, scrsResId), decodedScrs))
		return nullptr;

	ZmbScriptDecoder::DecodedScrs &cachedScrs = _systemDecodedScrsCache[scrsResId];
	cachedScrs = decodedScrs;
	return &cachedScrs;
}

uint MohawkEngine_Zoombini::getArchiveCount(ZmbResource::ArchiveKind kind) const {
	switch (kind) {
	case ZmbResource::kSystem: {
		uint count = 0;
		if (_sysMhk)
			count += 1;
		if (_helpMhk)
			count += 1;
		return count;
	}
	case ZmbResource::kPage:
		return _mhk.size();
	default:
		error("engine: invalid archive kind: %u", static_cast<uint32>(kind));
		break;
	}
	return 0;
}

Archive *MohawkEngine_Zoombini::getArchive(ZmbResource::ArchiveKind kind, uint archiveIdx) const {
	switch (kind) {
	case ZmbResource::kSystem:
		if (getArchiveCount(kind) <= archiveIdx) {
			error("engine: system archive index %u is out of bounds", archiveIdx);
			return nullptr;
		}
		return (archiveIdx == 0) ? _sysMhk : _helpMhk;
	case ZmbResource::kPage:
		if (_mhk.size() <= archiveIdx) {
			error("engine: page archive index %u is out of bounds", archiveIdx);
			return nullptr;
		}
		return _mhk[archiveIdx];
	default:
		error("engine: invalid archive kind: %u", static_cast<uint32>(kind));
		break;
	}
	return nullptr;
}

bool MohawkEngine_Zoombini::mustQuit() const {
	// When there is a scheduled quit event, wait until the fadeOut is done.
	return shouldQuit() && _quitEventState == kQuitEventDone;
}

} // End of namespace Mohawk
