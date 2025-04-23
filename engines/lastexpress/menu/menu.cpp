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

#include "lastexpress/menu/menu.h"

// Data
#include "lastexpress/data/animation.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/snd.h"
#include "lastexpress/data/scene.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/clock.h"
#include "lastexpress/menu/trainline.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/rational.h"

#define getNextGameId() (GameId)((_gameId + 1) % SaveLoad::kMaximumSaveSlots)

namespace LastExpress {

// Bottom-left buttons (quit.seq)
enum StartMenuButtons {
	kButtonVolumeDownPushed,
	kButtonVolumeDown,
	kButtonVolume,
	kButtonVolumeUp,
	kButtonVolumeUpPushed,
	kButtonBrightnessDownPushed,    // 5
	kButtonBrightnessDown,
	kButtonBrightness,
	kButtonBrightnessUp,
	kButtonBrightnessUpPushed,
	kButtonQuit,                    // 10
	kButtonQuitPushed
};

// Egg buttons (buttns.seq)
enum StartMenuEggButtons {
	kButtonShield,
	kButtonRewind,
	kButtonRewindPushed,
	kButtonForward,
	kButtonForwardPushed,
	kButtonCredits,                // 5
	kButtonCreditsPushed,
	kButtonContinue
};

// Tooltips sequence (helpnewr.seq)
enum StartMenuTooltips {
	kTooltipInsertCd1,
	kTooltipInsertCd2,
	kTooltipInsertCd3,
	kTooltipContinueGame,
	kTooltipReplayGame,
	kTooltipContinueRewoundGame,    // 5
	kTooltipViewGameEnding,
	kTooltipStartAnotherGame,
	kTooltipVolumeUp,
	kTooltipVolumeDown,
	kTooltipBrightnessUp,           // 10
	kTooltipBrightnessDown,
	kTooltipQuit,
	kTooltipRewindParis,
	kTooltipForwardStrasbourg,
	kTooltipRewindStrasbourg,      // 15
	kTooltipRewindMunich,
	kTooltipForwardMunich,
	kTooltipForwardVienna,
	kTooltipRewindVienna,
	kTooltipRewindBudapest,        // 20
	kTooltipForwardBudapest,
	kTooltipForwardBelgrade,
	kTooltipRewindBelgrade,
	kTooltipForwardConstantinople,
	kTooltipSwitchBlueGame,        // 25
	kTooltipSwitchRedGame,
	kTooltipSwitchGoldGame,
	kTooltipSwitchGreenGame,
	kTooltipSwitchTealGame,
	kTooltipSwitchPurpleGame,      // 30
	kTooltipPlayNewGame,
	kTooltipCredits,
	kTooltipFastForward,
	kTooltipRewind
};

//////////////////////////////////////////////////////////////////////////
// DATA
//////////////////////////////////////////////////////////////////////////
static const struct {
	TimeValue time;
	uint index;
	StartMenuTooltips rewind;
	StartMenuTooltips forward;
} _cityButtonsInfo[7] = {
	{kTimeCityParis, 64, kTooltipRewindParis, kTooltipRewindParis},
	{kTimeCityStrasbourg, 128, kTooltipRewindStrasbourg, kTooltipForwardStrasbourg},
	{kTimeCityMunich, 129, kTooltipRewindMunich, kTooltipForwardMunich},
	{kTimeCityVienna, 130, kTooltipRewindVienna, kTooltipForwardVienna},
	{kTimeCityBudapest, 131, kTooltipRewindBudapest, kTooltipForwardBudapest},
	{kTimeCityBelgrade, 132, kTooltipRewindBelgrade, kTooltipForwardBelgrade},
	{kTimeCityConstantinople, 192, kTooltipForwardConstantinople, kTooltipForwardConstantinople}
};

//////////////////////////////////////////////////////////////////////////
// Menu
//////////////////////////////////////////////////////////////////////////
MenuOld::MenuOld(LastExpressEngine *engine) : _engine(engine),
	_seqTooltips(nullptr), _seqEggButtons(nullptr), _seqButtons(nullptr), _seqAcorn(nullptr), _seqCity1(nullptr), _seqCity2(nullptr), _seqCity3(nullptr), _seqCredits(nullptr),
	_gameId(kGameBlue), _hasShownStartScreen(false), _hasShownIntro(false),
	_isShowingCredits(false), _isGameStarted(false), _isShowingMenu(false),
	_creditsSequenceIndex(0), _checkHotspotsTicks(15),  _mouseFlags(Common::EVENT_INVALID), _lastHotspot(nullptr),
	_currentTime(kTimeNone), _lowerTime(kTimeNone), _time(kTimeNone), _currentIndex(0), _index(0), _lastIndex(0), _delta(0), _handleTimeDelta(false) {

	_clock = new ClockOld(_engine);
	if (!_engine->isDemo()) {
		_trainLine = new TrainLine(_engine);
	}
}

MenuOld::~MenuOld() {
	SAFE_DELETE(_clock);
	if (!_engine->isDemo()) {
		SAFE_DELETE(_trainLine);
	}

	SAFE_DELETE(_seqTooltips);
	SAFE_DELETE(_seqEggButtons);
	SAFE_DELETE(_seqButtons);
	SAFE_DELETE(_seqAcorn);
	SAFE_DELETE(_seqCity1);
	SAFE_DELETE(_seqCity2);
	SAFE_DELETE(_seqCity3);
	SAFE_DELETE(_seqCredits);

	_lastHotspot = nullptr;

	// Cleanup frames
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		SAFE_DELETE(it->_value);

	_frames.clear();

	// Zero passed pointers
	_engine = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Setup
void MenuOld::setup() {

	// Clear drawing queue
	getScenes()->removeAndRedraw(&_frames[kOverlayAcorn], false);
	SAFE_DELETE(_seqAcorn);

	// Load Menu scene
	// + 1 = normal menu with open egg / clock
	// + 2 = shield menu, when no savegame exists (no game has been started)
	_isGameStarted = _lowerTime >= kTimeStartGame;
	getScenes()->loadScene((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2));
	getFlags()->shouldRedraw = true;
	getLogic()->updateCursor();

	//////////////////////////////////////////////////////////////////////////
	// Load Acorn sequence
	_seqAcorn = loadSequence(getAcornSequenceName(_isGameStarted ? getNextGameId() : kGameBlue));

	//////////////////////////////////////////////////////////////////////////
	// Check if we loaded sequences before
	if (_seqTooltips && _seqTooltips->count() > 0)
		return;

	// Load all static data
	_seqTooltips = loadSequence("helpnewr.seq");
	_seqEggButtons = loadSequence("buttns.seq");
	_seqButtons = loadSequence("quit.seq");
	_seqCity1 = loadSequence("jlinetl.seq");
	_seqCity2 = loadSequence("jlinecen.seq");
	_seqCity3 = loadSequence("jlinebr.seq");
	_seqCredits = loadSequence("credits.seq");

	_frames[kOverlayTooltip] = new SequenceFrame(_seqTooltips);
	_frames[kOverlayEggButtons] = new SequenceFrame(_seqEggButtons);
	_frames[kOverlayButtons] = new SequenceFrame(_seqButtons);
	_frames[kOverlayAcorn] = new SequenceFrame(_seqAcorn);
	_frames[kOverlayCity1] = new SequenceFrame(_seqCity1);
	_frames[kOverlayCity2] = new SequenceFrame(_seqCity2);
	_frames[kOverlayCity3] = new SequenceFrame(_seqCity3);
	_frames[kOverlayCredits] = new SequenceFrame(_seqCredits);
}

//////////////////////////////////////////////////////////////////////////
// Handle events
void MenuOld::eventMouse(const Common::Event &ev) {
	if (!getFlags()->shouldRedraw)
		return;

	bool redraw = true;
	getFlags()->shouldRedraw = false;

	// Update coordinates
	setCoords(ev.mouse);
	//_mouseFlags = (Common::EventType)(ev.type & Common::EVENT_LBUTTONUP);

	if (_isShowingCredits) {
		if (ev.type == Common::EVENT_RBUTTONUP) {
			showFrame(kOverlayCredits, -1, true);
			_isShowingCredits = false;
		}

		if (ev.type == Common::EVENT_LBUTTONUP) {
			// Last frame of the credits
			if (_seqCredits && _creditsSequenceIndex == _seqCredits->count() - 1) {
				showFrame(kOverlayCredits, -1, true);
				_isShowingCredits = false;
			} else {
				++_creditsSequenceIndex;
				showFrame(kOverlayCredits, _creditsSequenceIndex, true);
			}
		}
	} else {
		// Check for hotspots
		SceneHotspot *hotspot = nullptr;
		getScenes()->get(getState()->scene)->checkHotSpot(ev.mouse, &hotspot);

		if (_lastHotspot != hotspot || ev.type == Common::EVENT_LBUTTONUP) {
			_lastHotspot = hotspot;

			if (ev.type == Common::EVENT_MOUSEMOVE) { /* todo check event type */
				if (!_handleTimeDelta && hasTimeDelta())
					setTime();
			}

			if (hotspot) {
				redraw = handleEvent((StartMenuAction)hotspot->action, ev.type);
				getFlags()->mouseRightClick = false;
				getFlags()->mouseLeftClick = false;
			} else {
				hideOverlays();
			}
		}
	}

	if (redraw) {
		getFlags()->shouldRedraw = true;
		askForRedraw();
	}
}

void MenuOld::eventTick(const Common::Event&) {
	if (hasTimeDelta())
		adjustTime();
	else if (_handleTimeDelta)
		_handleTimeDelta = false;

	// Check hotspots
	if (!--_checkHotspotsTicks) {
		checkHotspots();
		_checkHotspotsTicks = 15;
	}
}

//////////////////////////////////////////////////////////////////////////
// Show the intro and load the main menu scene
void MenuOld::show(bool doSavegame, SavegameType type, uint32 value) {

	if (_isShowingMenu)
		return;

	_isShowingMenu = true;
	getEntities()->reset();

	// If no blue savegame exists, this might be the first time we start the game, so we show the full intro
	if (!getFlags()->mouseRightClick) {
		if (!SaveLoad::isSavegameValid(_engine->getTargetName(), kGameBlue) && _engine->getResourceManager()->loadArchive(kArchiveCd1) && !_engine->isDemo()) {

			if (!_hasShownIntro) {
				// Show Broderbrund logo
				Animation animation;
				if (animation.load(getArchiveMember("1930.nis")))
					animation.play();

				getFlags()->mouseRightClick = false;

				// Play intro music
				getSound()->playSoundWithSubtitles("MUS001.SND", kSoundTypeIntro | kVolumeFull, kCharacterCath);

				// Show The Smoking Car logo
				if (animation.load(getArchiveMember("1931.nis")))
					animation.play();

				_hasShownIntro = true;
			}
		} else {
			// Only show the quick intro
			if (!_hasShownStartScreen) {
				getSound()->playSoundWithSubtitles("MUS018.SND", kSoundTypeIntro | kVolumeFull, kCharacterCath);
				getScenes()->loadScene(kSceneStartScreen);

				// Original game waits 60 frames and loops Sound::unknownFunction1 unless the right button is pressed
				uint32 nextFrameCount = getFrameCount() + 60;
				while (getFrameCount() < nextFrameCount) {
					_engine->pollEventsOld();

					if (getFlags()->mouseRightClick)
						break;

					getSoundQueue()->updateQueue();
				}
			}
		}
	}

	_hasShownStartScreen = true;

	// Init Menu
	init(doSavegame, type, value);

	// Setup sound
	getSoundQueue()->stopAmbient();
	getSoundQueue()->stopAllExcept(kSoundTagIntro, kSoundTagMenu);
	if (getSoundQueue()->isBuffered("TIMER"))
		getSoundQueue()->stop("TIMER");

	// Init flags & misc
	_isShowingCredits = false;
	_handleTimeDelta = hasTimeDelta();
	getInventory()->unselectItem();

	// Set Cursor type
	_engine->getCursor()->setStyle(kCursorNormal);
	_engine->getCursor()->show(true);

	setup();
	checkHotspots();

	// Set event handlers
	SET_EVENT_HANDLERS(MenuOld, this);
}

bool MenuOld::handleEvent(StartMenuAction action, Common::EventType type) {
	bool clicked = (type == Common::EVENT_LBUTTONUP);

	switch(action) {
	default:
		hideOverlays();
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCredits:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			showFrame(kOverlayEggButtons, kButtonCreditsPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kCharacterCath, "LIB046");

			hideOverlays();

			_isShowingCredits = true;
			_creditsSequenceIndex = 0;

			showFrame(kOverlayCredits, 0, true);
		} else {
			// TODO check flags ?

			showFrame(kOverlayEggButtons, kButtonCredits, true);
			showFrame(kOverlayTooltip, kTooltipCredits, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuQuitGame:
		showFrame(kOverlayTooltip, kTooltipQuit, true);

		if (clicked) {
			showFrame(kOverlayButtons, kButtonQuitPushed, true);

			getSoundQueue()->stopAll();
			getSoundQueue()->updateQueue();
			getSound()->playSound(kCharacterCath, "LIB046");

			// FIXME uncomment when sound queue is properly implemented
			/*while (getSoundQueue()->isBuffered("LIB046"))
				getSoundQueue()->updateQueue();*/

			getFlags()->shouldRedraw = false;

			Engine::quitGame();

			return false;
		} else {
			showFrame(kOverlayButtons, kButtonQuit, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuCase4:
		if (clicked)
			_index = 0;
		// fall through

	//////////////////////////////////////////////////////////////////////////
	case kMenuContinue: {
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Determine the proper CD archive
		ArchiveIndex cd = kArchiveCd1;
		if (getProgress().chapter > kChapter1)
			cd = (getProgress().chapter > kChapter3) ? kArchiveCd3 : kArchiveCd2;

		// Show tooltips & buttons to start a game, continue a game or load the proper cd
		if (_engine->getResourceManager()->isArchivePresent(cd)) {
			if (_isGameStarted) {
				showFrame(kOverlayEggButtons, kButtonContinue, true);

				if (_lastIndex == _index) {
					showFrame(kOverlayTooltip, getSaveLoad()->isGameFinished(_index, _lastIndex) ? kTooltipViewGameEnding : kTooltipContinueGame, true);
				} else {
					showFrame(kOverlayTooltip, kTooltipContinueRewoundGame, true);
				}

			} else {
				showFrame(kOverlayEggButtons, kButtonShield, true);
				showFrame(kOverlayTooltip, kTooltipPlayNewGame, true);
			}
		} else {
			showFrame(kOverlayEggButtons, -1, true);
			showFrame(kOverlayTooltip, cd - 1, true);
		}

		if (!clicked)
			break;

		// Try loading the archive file
		if (!_engine->getResourceManager()->loadArchive(cd))
			break;

		// Load the train data file and setup game
		getScenes()->loadSceneDataFile(cd);
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kCharacterCath, "LIB046");

		// Setup new game
		getSavePoints()->reset();
		setLogicEventHandlers();

		if (_index) {
			getSoundQueue()->fade(kSoundTagIntro);
		} else {
			if (!getFlags()->mouseRightClick) {
				getScenes()->loadScene((SceneIndex)(5 * _gameId + 3));

				if (!getFlags()->mouseRightClick) {
					getScenes()->loadScene((SceneIndex)(5 * _gameId + 4));

					if (!getFlags()->mouseRightClick) {
						getScenes()->loadScene((SceneIndex)(5 * _gameId + 5));

						if (!getFlags()->mouseRightClick) {
							getSoundQueue()->fade(kSoundTagIntro);

							// Show intro
							Animation animation;
							if (animation.load(getArchiveMember("1601.nis")))
								animation.play();

							HELPERgetEvent(kEventIntro) = 1;
						}
					}
				}
			}

			if (!HELPERgetEvent(kEventIntro))	{
				HELPERgetEvent(kEventIntro) = 1;

				getSoundQueue()->fade(kSoundTagIntro);
			}
		}

		// Setup game
		getFlags()->isGameRunning = true;
		startGame();

		if (!_isShowingMenu)
			getInventory()->show();

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	case kMenuSwitchSaveGame:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (!_engine->isDemo()) {
				showFrame(kOverlayAcorn, 1, true);
				showFrame(kOverlayTooltip, -1, true);
				getSound()->playSound(kCharacterCath, "LIB047");

				// Setup new menu screen
				switchGame();
				setup();

				// Set fight state to 0
				//getFight()->resetState();
			}
			return true;
		}

		// TODO Check for flag

		showFrame(kOverlayAcorn, 0, true);

		if (_isGameStarted && !_engine->isDemo()) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (_gameId == kGameGold) {
			showFrame(kOverlayTooltip, kTooltipSwitchBlueGame, true);
			break;
		}

		if (_engine->isDemo() || !SaveLoad::isSavegameValid(_engine->getTargetName(), getNextGameId())) {
			showFrame(kOverlayTooltip, kTooltipStartAnotherGame, true);
			break;
		}

		// Stupid tooltips ids are not in order, so we can't just increment them...
		switch(_gameId) {
		default:
			break;

		case kGameBlue:
			showFrame(kOverlayTooltip, kTooltipSwitchRedGame, true);
			break;

		case kGameRed:
			showFrame(kOverlayTooltip, kTooltipSwitchGreenGame, true);
			break;

		case kGameGreen:
			showFrame(kOverlayTooltip, kTooltipSwitchPurpleGame, true);
			break;

		case kGamePurple:
			showFrame(kOverlayTooltip, kTooltipSwitchTealGame, true);
			break;

		case kGameTeal:
			showFrame(kOverlayTooltip, kTooltipSwitchGoldGame, true);
			break;
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuRewindGame:
		if (!_index || _currentTime < _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonRewindPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kCharacterCath, "LIB046");

			rewindTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonRewind, true);
			showFrame(kOverlayTooltip, kTooltipRewind, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuForwardGame:
		if (_lastIndex <= _index || _currentTime > _time) {
			hideOverlays();
			break;
		}

		if (clicked) {
			if (hasTimeDelta())
				_handleTimeDelta = false;

			showFrame(kOverlayEggButtons, kButtonForwardPushed, true);
			showFrame(kOverlayTooltip, -1, true);

			getSound()->playSound(kCharacterCath, "LIB046");

			forwardTime();

			_handleTimeDelta = false;
		} else {
			showFrame(kOverlayEggButtons, kButtonForward, true);
			showFrame(kOverlayTooltip, kTooltipFastForward, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuParis:
		moveToCity(kParis, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuStrasBourg:
		moveToCity(kStrasbourg, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuMunich:
		moveToCity(kMunich, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuVienna:
		moveToCity(kVienna, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBudapest:
		moveToCity(kBudapest, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuBelgrade:
		moveToCity(kBelgrade, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuConstantinople:
		moveToCity(kConstantinople, clicked);
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot decrease volume further
		if (getVolume() == 0) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeDown, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeDownPushed, true);
			getSound()->playSound(kCharacterCath, "LIB046");
			setVolume(getVolume() - 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEventsOld();

				getSoundQueue()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseVolume:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase volume further
		if (getVolume() >= 7) {
			showFrame(kOverlayButtons, kButtonVolume, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipVolumeUp, true);

		// Show highlight on button & adjust volume if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonVolumeUpPushed, true);
			getSound()->playSound(kCharacterCath, "LIB046");
			setVolume(getVolume() + 1);

			getSaveLoad()->saveVolumeBrightness();

			uint32 nextFrameCount = getFrameCount() + 15;
			while (nextFrameCount > getFrameCount()) {
				_engine->pollEventsOld();

				getSoundQueue()->updateQueue();
			}
		} else {
			showFrame(kOverlayButtons, kButtonVolumeUp, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuDecreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() == 0) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessDown, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, true);
			getSound()->playSound(kCharacterCath, "LIB046");
			setBrightness(getBrightness() - 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManagerOld)
			_engine->getGraphicsManagerOld()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManagerOld::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessDown, false);
			showFrame(kOverlayButtons, kButtonBrightnessDownPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessDown, true);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case kMenuIncreaseBrightness:
		if (hasTimeDelta()) {
			hideOverlays();
			break;
		}

		// Cannot increase brightness further
		if (getBrightness() >= 6) {
			showFrame(kOverlayButtons, kButtonBrightness, true);
			showFrame(kOverlayTooltip, -1, true);
			break;
		}

		showFrame(kOverlayTooltip, kTooltipBrightnessUp, true);

		// Show highlight on button & adjust brightness if needed
		if (clicked) {
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, true);
			getSound()->playSound(kCharacterCath, "LIB046");
			setBrightness(getBrightness() + 1);

			getSaveLoad()->saveVolumeBrightness();

			// Reshow the background and frames (they will pick up the new brightness through the GraphicsManagerOld)
			_engine->getGraphicsManagerOld()->draw(getScenes()->get((SceneIndex)(_isGameStarted ? _gameId * 5 + 1 : _gameId * 5 + 2)), GraphicsManagerOld::kBackgroundC, true);
			showFrame(kOverlayTooltip, kTooltipBrightnessUp, false);
			showFrame(kOverlayButtons, kButtonBrightnessUpPushed, false);
		} else {
			showFrame(kOverlayButtons, kButtonBrightnessUp, true);
		}
		break;
	}

	return true;
}

void MenuOld::setLogicEventHandlers() {
	SET_EVENT_HANDLERS(Logic, getLogic());
	clear();
	_isShowingMenu = false;
}

//////////////////////////////////////////////////////////////////////////
// Game-related
//////////////////////////////////////////////////////////////////////////
void MenuOld::init(bool doSavegame, SavegameType type, uint32 value) {
	bool useSameIndex = true;

	if (getGlobalTimer()) {
		value = 0;

		// Check if the CD file is present
		ArchiveIndex index = kArchiveCd1;
		switch (getProgress().chapter) {
		default:
		case kChapter1:
			break;

		case kChapter2:
		case kChapter3:
			index = kArchiveCd2;
			break;

		case kChapter4:
		case kChapter5:
			index = kArchiveCd3;
			break;
		}

		if (_engine->getResourceManager()->isArchivePresent(index)) {
			setGlobalTimer(0);
			useSameIndex = false;

			// TODO remove existing savegame and reset index & savegame eraseData
			warning("[Menu::initGame] Not implemented");
		}

		doSavegame = false;
	} else {
		warning("[Menu::initGame] Renaming saves not implemented");
	}

	// Create a new savegame if needed
	if (!SaveLoad::isSavegamePresent(_engine->getTargetName(), _gameId))
		getSaveLoad()->create(_engine->getTargetName(), _gameId);

	if (doSavegame)
		getSaveLoad()->saveGame(kSavegameTypeEvent2, kCharacterCath, kEventNone);

	if (!getGlobalTimer()) {
		warning("[Menu::initGame] Removing temporary saves not implemented");
	}

	// Init savegame & menu values
	_lastIndex = getSaveLoad()->init(_engine->getTargetName(), _gameId, true);
	_lowerTime = getSaveLoad()->getTime(_lastIndex);

	if (useSameIndex)
		_index = _lastIndex;

	//if (!getGlobalTimer())
	//	_index3 = 0;

	if (!getProgress().chapter) {
		if (_engine->isDemo()) {
			getProgress().chapter = kChapter3;
		} else {
			getProgress().chapter = kChapter1;
		}
	}

	getState()->time = (TimeValue)getSaveLoad()->getTime(_index);
	getProgress().chapter = getSaveLoad()->getChapter(_index);

	if (_lowerTime >= kTimeStartGame) {
		_currentTime = (uint32)getState()->time;
		_time = (uint32)getState()->time;
		_clock->draw(_time);
		if (!_engine->isDemo()) {
			_trainLine->draw(_time);
		}

		initTime(type, value);
	}
}

// Start a game (or load an existing savegame)
void MenuOld::startGame() {
	// Clear savegame headers
	getSaveLoad()->clear();

	// Hide menu elements
	_clock->clear();
	if (!_engine->isDemo()) {
		_trainLine->clear();
	}

	if (_lastIndex == _index) {
		setGlobalTimer(0);
		if (_index) {
			getSaveLoad()->loadLastGame();
		} else {
			getLogic()->resetState();
			// TODO This check and code (for demo case) may be removed in the future
			if (_engine->isDemo()) {
				getState()->time = kTime2241000;
				getProgress().chapter = kChapter3;
			}
			getEntities()->setup(true, kCharacterCath);
		}
	} else {
		getSaveLoad()->loadGame(_index);
	}
}

// Switch to the next savegame
void MenuOld::switchGame() {

	// Switch back to blue game if the current game is not started
	_gameId = SaveLoad::isSavegameValid(_engine->getTargetName(), _gameId) ? getNextGameId() : kGameBlue;

	// Initialize savegame if needed
	if (!SaveLoad::isSavegamePresent(_engine->getTargetName(), _gameId))
		getSaveLoad()->create(_engine->getTargetName(), _gameId);

	getState()->time = kTimeNone;

	// Clear menu elements
	_clock->clear();
	if (!_engine->isDemo()) {
		_trainLine->clear();
	}

	// Clear loaded savegame data
	getSaveLoad()->clear(true);

	init(false, kSavegameTypeIndex, 0);
}

//////////////////////////////////////////////////////////////////////////
// Overlays & elements
//////////////////////////////////////////////////////////////////////////
void MenuOld::checkHotspots() {
	if (!_isShowingMenu)
		return;

	if (!getFlags()->shouldRedraw)
		return;

	if (_isShowingCredits)
		return;

	SceneHotspot *hotspot = nullptr;
	getScenes()->get(getState()->scene)->checkHotSpot(getCoords(), &hotspot);

	if (hotspot)
		handleEvent((StartMenuAction)hotspot->action, _mouseFlags);
	else
		hideOverlays();
}

void MenuOld::hideOverlays() {
	_lastHotspot = nullptr;

	// Hide all menu overlays
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		showFrame(it->_key, -1, false);

	getScenes()->drawFrames(true);
}

void MenuOld::showFrame(StartMenuOverlay overlayType, int index, bool redraw) {
	if (index == -1) {
		getScenes()->removeFromQueue(_frames[overlayType]);
	} else {
		// Check that the overlay is valid
		if (!_frames[overlayType])
			return;

		// Remove the frame and add a new one with the proper index
		getScenes()->removeFromQueue(_frames[overlayType]);
		_frames[overlayType]->setFrame((uint16)index);
		getScenes()->addToQueue(_frames[overlayType]);
	}

	if (redraw)
		getScenes()->drawFrames(true);
}

// Remove all frames from the queue
void MenuOld::clear() {
	for (MenuFrames::iterator it = _frames.begin(); it != _frames.end(); it++)
		getScenes()->removeAndRedraw(&it->_value, false);

	clearBg(GraphicsManagerOld::kBackgroundOverlay);
}

// Get the sequence eraseData to use for the acorn highlight, depending of the currently loaded savegame
Common::String MenuOld::getAcornSequenceName(GameId id) const {
	if (_engine->isDemo()) {
		return "aconred.seq";
	}

	Common::String name = "";
	switch (id) {
	default:
	case kGameBlue:
		name = "aconblu3.seq";
		break;

	case kGameRed:
		name = "aconred.seq";
		break;

	case kGameGreen:
		name = "acongren.seq";
		break;

	case kGamePurple:
		name = "aconpurp.seq";
		break;

	case kGameTeal:
		name = "aconteal.seq";
		break;

	case kGameGold:
		name = "acongold.seq";
		break;
	}

	return name;
}

//////////////////////////////////////////////////////////////////////////
// Time
//////////////////////////////////////////////////////////////////////////
void MenuOld::initTime(SavegameType type, uint32 value) {
	if (!value)
		return;

	// The savegame entry index
	uint32 entryIndex = 0;

	switch (type) {
	default:
		break;

	case kSavegameTypeIndex:
		entryIndex = (_index <= value) ? 1 : _index - value;
		break;

	case kSavegameTypeTime:
		if (value < kTimeStartGame)
			break;

		entryIndex = _index;
		if (!entryIndex)
			break;

		// Iterate through existing entries
		do {
			if (getSaveLoad()->getTime(entryIndex) <= value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent:
		entryIndex = _index;
		if (!entryIndex)
			break;

		do {
			if (getSaveLoad()->getValue(entryIndex) == value)
				break;

			entryIndex--;
		} while (entryIndex);
		break;

	case kSavegameTypeEvent2:
		// TODO rewrite in a more legible way
		if (_index > 1) {
			uint32 index = _index;
			do {
				if (getSaveLoad()->getValue(index) == value)
					break;

				index--;
			} while (index > 1);

			entryIndex = index - 1;
		} else {
			entryIndex = _index - 1;
		}
		break;
	}

	if (entryIndex) {
		_currentIndex = entryIndex;
		updateTime(getSaveLoad()->getTime(entryIndex));
	}
}

void MenuOld::updateTime(uint32 time) {
	if (_currentTime == _time)
		_delta = 0;

	_currentTime = time;

	if (_time != time) {
		if (getSoundQueue()->isBuffered(kCharacterMaster))
			getSoundQueue()->stop(kCharacterMaster);

		getSound()->playSoundWithSubtitles((_currentTime >= _time) ? "LIB042" : "LIB041", kSoundTypeMenu | kSoundFlagFixedVolume | kVolumeFull, kCharacterMaster);
		adjustIndex(_currentTime, _time, false);
	}
}

void MenuOld::adjustIndex(uint32 time1, uint32 time2, bool searchEntry) {
	uint32 index = 0;
	int32 timeDelta = -1;

	if (time1 != time2) {

		index = _index;

		if (time2 >= time1) {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if ((int32)_index >= 0) {
					do {
						// Calculate new delta
						int32 newDelta = time1 - (uint32)getSaveLoad()->getTime(currentIndex);

						if (newDelta >= 0 && timeDelta >= newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						--currentIndex;
					} while ((int32)currentIndex >= 0);
				}
			} else {
				index = _index - 1;
			}
		} else {
			if (searchEntry) {
				uint32 currentIndex = _index;

				if (_lastIndex >= _index) {
					do {
						// Calculate new delta
						int32 newDelta = (uint32)getSaveLoad()->getTime(currentIndex) - time1;

						if (newDelta >= 0 && timeDelta > newDelta) {
							timeDelta = newDelta;
							index = currentIndex;
						}

						++currentIndex;
					} while (currentIndex <= _lastIndex);
				}
			} else {
				index = _index + 1;
			}
		}

		_index = index;
		checkHotspots();
	}

	if (_index == _currentIndex) {
		if (getProgress().chapter != getSaveLoad()->getChapter(index))
			getProgress().chapter = getSaveLoad()->getChapter(_index);
	}
}

void MenuOld::goToTime(uint32 time) {

	uint32 entryIndex = 0;
	uint32 deltaTime = (uint32)ABS((int32)(getSaveLoad()->getTime(0) - time));
	uint32 index = 0;

	do {
		uint32 deltaTime2 = (uint32)ABS((int32)(getSaveLoad()->getTime(index) - time));
		if (deltaTime2 < deltaTime) {
			deltaTime = deltaTime2;
			entryIndex = index;
		}

		++index;
	} while (_lastIndex >= index);

	_currentIndex = entryIndex;
	updateTime(getSaveLoad()->getTime(entryIndex));
}

void MenuOld::setTime() {
	_currentIndex = _index;
	_currentTime = getSaveLoad()->getTime(_currentIndex);

	if (_time == _currentTime)
		adjustTime();
}

void MenuOld::forwardTime() {
	if (_lastIndex <= _index)
		return;

	_currentIndex = _lastIndex;
	updateTime(getSaveLoad()->getTime(_currentIndex));
}

void MenuOld::rewindTime() {
	if (!_index)
		return;

	_currentIndex = 0;
	updateTime(getSaveLoad()->getTime(_currentIndex));
}

void MenuOld::adjustTime() {
	uint32 originalTime = _time;

	// Adjust time delta
	Common::Rational timeDelta(_delta >= 90 ? 9 : (9 * _delta + 89), _delta >= 90 ? 1 : 90);

	if (_currentTime < _time) {
		timeDelta *= 900;
		_time -= (uint)timeDelta.toInt();

		if (_currentTime > _time)
			_time = _currentTime;
	} else {
		timeDelta *= 900;
		_time += (uint)timeDelta.toInt();

		if (_currentTime < _time)
			_time = _currentTime;
	}

	if (_currentTime == _time && getSoundQueue()->isBuffered(kCharacterMaster))
		getSoundQueue()->stop(kCharacterMaster);

	_clock->draw(_time);
	if (!_engine->isDemo()) {
		_trainLine->draw(_time);
	}
	getScenes()->drawFrames(true);

	adjustIndex(_time, originalTime, true);

	++_delta;
}

void MenuOld::moveToCity(CityButton city, bool clicked) {
	uint32 time = (uint32)_cityButtonsInfo[city].time;

	// TODO Check if we have access (there seems to be more checks on some internal times) - probably : current_time (menu only) / game time / some other?
	if (_lowerTime < time || _time == time || _currentTime == time) {
		hideOverlays();
		return;
	}

	// Show city overlay
	showFrame((StartMenuOverlay)((_cityButtonsInfo[city].index >> 6) + 3), _cityButtonsInfo[city].index & 63, true);

	if (clicked) {
		showFrame(kOverlayTooltip, -1, true);
		getSound()->playSound(kCharacterCath, "LIB046");
		goToTime(time);

		_handleTimeDelta = true;

		return;
	}

	// Special case of first and last cities
	if (city == kParis || city == kConstantinople) {
		showFrame(kOverlayTooltip, (city == kParis) ? kTooltipRewindParis : kTooltipForwardConstantinople, true);
		return;
	}

	showFrame(kOverlayTooltip, (_time <= time) ? _cityButtonsInfo[city].forward : _cityButtonsInfo[city].rewind, true);
}

//////////////////////////////////////////////////////////////////////////
// Sound / Brightness
//////////////////////////////////////////////////////////////////////////

// Get current volume (converted internal ScummVM value)
uint32 MenuOld::getVolume() const {
	return getState()->volume;
}

// Set the volume (converts to ScummVM values)
void MenuOld::setVolume(uint32 volume) const {
	getState()->volume = volume;

	// Clamp volume
	uint32 value = volume * Audio::Mixer::kMaxMixerVolume / 7;

	if (value > Audio::Mixer::kMaxMixerVolume)
		value = Audio::Mixer::kMaxMixerVolume;

	_engine->_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, (int32)value);
}

uint32 MenuOld::getBrightness() const {
	return getState()->brightness;
}

void MenuOld::setBrightness(uint32 brightness) const {
	getState()->brightness = brightness;

	// TODO reload cursor & font with adjusted brightness
}

Menu::Menu(LastExpressEngine *engine) {
	_engine = engine;
}

void Menu::doEgg(bool doSaveGame, int type, int32 time) {
	if (!_isShowingMenu) {
		_isShowingMenu = true;

		_engine->getOtisManager()->wipeAllGSysInfo();

		if (!_engine->mouseHasRightClicked()) {
			if (_engine->getVCR()->isVirgin(0) && _engine->getArchiveManager()->lockCD(1)) {
				if (!_hasShownIntro) {
					_engine->getNISManager()->doNIS("1930.NIS", 0x4000);
					_engine->getMessageManager()->clearClickEvents();
					_engine->mouseSetRightClicked(false);
					_engine->getSoundManager()->playSoundFile("MUS001.SND", kSoundTypeIntro | kVolumeFull, 0, 0);
					_engine->getNISManager()->doNIS("1931.NIS", 0x4000);
					_hasShownIntro = true;
				}
			} else if (!_hasShownStartScreen) {
				_engine->getSoundManager()->playSoundFile("MUS018.SND", kSoundTypeIntro | kVolumeFull, 0, 0);
				_engine->getLogicManager()->bumpCathNode(65);

				int32 delay = _engine->getSoundFrameCounter() + 60;

				while (_engine->getSoundFrameCounter() < delay) {
					if (_engine->mouseHasRightClicked())
						break;
					_engine->getSoundManager()->soundThread();
				}
			}
		}

		_hasShownStartScreen = true;
		_engine->getVCR()->init(doSaveGame, type, time);
		_engine->getSoundManager()->killAmbient();
		_engine->getSoundManager()->killAllExcept(kSoundTagIntro, kSoundTagMenu, 0, 0, 0, 0, 0);

		if (_engine->getLogicManager()->dialogRunning("TIMER"))
			_engine->getLogicManager()->endDialog("TIMER");

		_engine->getArchiveManager()->unlockCD();
		_currentHotspotLink = 0;
		_engine->_doShowCredits = false;
		_engine->setEventTickInternal(false);
		_engine->getLogicManager()->_inventorySelectedItemIdx = 0;
		_moveClockHandsFlag = _engine->getClock()->statusClock();
		_engine->_navigationEngineIsRunning = false;

		_engine->getMessageManager()->clearEventQueue();

		_engine->_cursorX = _engine->_systemEventLastMouseCoords.x;
		_engine->_cursorY = _engine->_systemEventLastMouseCoords.y;

		bool oldShouldRedraw = _engine->getGraphicsManager()->canDrawMouse();
		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->_cursorType = 0;

		_engine->getGraphicsManager()->newMouseLoc();
		_engine->getGraphicsManager()->setMouseDrawable(oldShouldRedraw);

		_engine->getMessageManager()->setEventHandle(1, &LastExpressEngine::eggMouseWrapper);
		_engine->getMessageManager()->setEventHandle(3, &LastExpressEngine::eggTimerWrapper);

		_menuSeqs[1] = _engine->getArchiveManager()->loadSeq("buttns.seq", 15, 0);
		_menuSeqs[0] = _engine->getArchiveManager()->loadSeq("helpnewr.seq", 15, 0);
		switchEggs(_engine->_currentGameFileColorId);
		updateEgg();

		_engine->getMessageManager()->setEventHandle(1, &LastExpressEngine::eggMouseWrapper);
		_engine->getMessageManager()->setEventHandle(3, &LastExpressEngine::eggTimerWrapper);
	}
}

void Menu::endEgg() {
	_engine->getMessageManager()->setEventHandle(1, &LastExpressEngine::nodeStepMouseWrapper);
	_engine->getMessageManager()->setEventHandle(3, &LastExpressEngine::nodeStepTimerWrapper);

	eggFree();
	_engine->getVCR()->free();

	_isShowingMenu = false;
}

void Menu::eggFree() {
	for (int i = 0; i < 8; i++) {
		_engine->getSpriteManager()->destroySprite(&_startMenuFrames[i], false);
		if (_menuSeqs[i]) {
			_engine->getMemoryManager()->freeMem(_menuSeqs[i]->rawSeqData);
			delete _menuSeqs[i];
			_menuSeqs[i] = nullptr;
		}
	}
}

void Menu::eggMouse(Event *event) {
	if (_engine->getGraphicsManager()->canDrawMouse()) {
		bool redrawMouse = true;

		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->getGraphicsManager()->burstMouseArea(false); // The original updated the screen, we don't to avoid flickering...
		_engine->_cursorX = event->x;
		_engine->_cursorY = event->y;
		_eggCurrentMouseFlags = (event->flags & 1) != 0;
		_engine->_cursorType = 0;

		if (_engine->_doShowCredits) {
			if ((event->flags & 0x10) != 0) {
				setSprite(7, -1, true);
				_engine->_doShowCredits = false;
			}

			if ((event->flags & 8) != 0) {
				if (_eggCreditsIndex == _menuSeqs[7]->numFrames - 1) {
					setSprite(7, -1, true);
					_engine->_doShowCredits = false;
				} else {
					_eggCreditsIndex++;
					setSprite(7, _eggCreditsIndex, true);
				}
			}
		} else {
			uint8 location = 0;
			Link *foundLink = nullptr;

			for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].link; i; i = i->next) {
				if (_engine->getLogicManager()->pointIn(_engine->_cursorX, _engine->_cursorY, i) && i->location >= location) {
					location = i->location;
					foundLink = i;
				}
			}

			if (foundLink != _currentHotspotLink || (event->flags & 8) != 0 || (event->flags & 0x80) != 0) {
				_currentHotspotLink = foundLink;

				if ((event->flags & 0x80) != 0 && !_moveClockHandsFlag && _engine->getClock()->statusClock())
					_engine->getVCR()->stop();

				if (foundLink) {
					redrawMouse = eggCursorAction(foundLink->action, event->flags);
					_engine->mouseSetRightClicked(false);
					_engine->mouseSetLeftClicked(false);
				} else {
					clearSprites();
				}
			}
		}

		if (redrawMouse) {
			_engine->getGraphicsManager()->setMouseDrawable(true);
			_engine->getGraphicsManager()->newMouseLoc();
			_engine->getGraphicsManager()->burstMouseArea();
		}
	}
}

void Menu::eggTimer(Event *event) {
	_engine->setEventTickInternal(false);

	if (_engine->getClock()->statusClock()) {
		_engine->getClock()->tickClock();
	} else if (_moveClockHandsFlag) {
		_moveClockHandsFlag = false;
	}

	if (!_eggTimerDelta--) {
		updateEgg();
		_eggTimerDelta = 15;
	}
}

void Menu::clearSprites() {
	_currentHotspotLink = 0;

	for (int i = 0; i < 8; i++)
		setSprite(i, -1, false);

	_engine->getSpriteManager()->drawCycle();
}

void Menu::updateEgg() {
	if (_isShowingMenu && _engine->getGraphicsManager()->canDrawMouse() && !_engine->_doShowCredits) {
		Link *chosenLink = nullptr;
		uint16 location = 0;

		for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].link; i; i = i->next) {
			if (_engine->getLogicManager()->pointIn(_engine->_cursorX, _engine->_cursorY, i) && location <= i->location) {
				location = i->location;
				chosenLink = i;
			}
		}

		if (chosenLink) {
			eggCursorAction(chosenLink->action, _eggCurrentMouseFlags);
		} else {
			clearSprites();
		}
	}
}

bool Menu::eggCursorAction(int8 action, int8 flags) {
	switch (action) {
	case kMenuActionCredits:
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		if ((flags & 8) != 0) {
			setSprite(1, 6, true);
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);

			clearSprites();

			_engine->_doShowCredits = true;
			_eggCreditsIndex = 0;

			setSprite(7, 0, true);

			return true;
		} else {
			if ((flags & 0x80) != 0)
				return true;

			setSprite(1, 5, true);
			setSprite(0, 32, true);

			return true;
		}
	case kMenuActionQuit:
		setSprite(0, 12, true);

		if ((flags & 8) != 0) {
			setSprite(2, 11, true);

			_engine->getSoundManager()->killAllSlots();
			_engine->getSoundManager()->soundThread();
			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);

			while (_engine->getLogicManager()->dialogRunning("LIB046"))
				_engine->getSoundManager()->soundThread();

			g_system->delayMillis(334);

			_engine->getGraphicsManager()->setMouseDrawable(false);
			endEgg();

			_engine->quitGame();
		} else {
			setSprite(2, 10, true);
			return true;
		}

		return false;
	case kMenuActionPlayGame:
	case kMenuAction4:
	{
		if (action == kMenuAction4) {
			if ((flags & 8) != 0)
				_engine->_currentSavePoint = 0;
		}

		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		int whichCD = 1;
		if (_engine->getLogicManager()->_gameProgress[kProgressChapter] > 1)
			whichCD = (_engine->getLogicManager()->_gameProgress[kProgressChapter] > 3) + 2;

		char path[80];
		if (_engine->getArchiveManager()->isCDAvailable(whichCD, path, sizeof(path))) {
			if (_gameInNotStartedInFile) {
				setSprite(1, 0, true);
				setSprite(0, 31, true);
			} else {
				setSprite(1, 7, true);

				if (_engine->_lastSavePointIdInFile == _engine->_currentSavePoint) {
					if (_engine->getVCR()->currentEndsGame()) {
						setSprite(0, 6, true);
					} else {
						setSprite(0, 3, true);
					}
				} else {
					setSprite(0, 5, true);
				}
			}
		} else {
			setSprite(1, -1, true);
			setSprite(0, whichCD - 1, true);
		}

		if ((flags & 8) == 0)
			return true;

		if (!_engine->getArchiveManager()->lockCD(whichCD))
			return true;

		_engine->getLogicManager()->loadTrain(whichCD);

		setSprite(0, -1, true);

		_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
		_engine->getMessageManager()->reset();
		endEgg();

		if (!_engine->_currentSavePoint) {

			if (!_engine->mouseHasRightClicked()) {
				_engine->getLogicManager()->bumpCathNode(5 * _engine->_currentGameFileColorId + 3);

				if (!_engine->mouseHasRightClicked()) {
					_engine->getLogicManager()->bumpCathNode(5 * _engine->_currentGameFileColorId + 4);

					if (!_engine->mouseHasRightClicked()) {
						_engine->getLogicManager()->bumpCathNode(5 * _engine->_currentGameFileColorId + 5);

						if (!_engine->mouseHasRightClicked()) {
							Slot *slot = _engine->getSoundManager()->_soundCache;
							if (_engine->getSoundManager()->_soundCache) {
								do {
									if (slot->_tag == kSoundTagIntro)
										break;

									slot = slot->_next;
								} while (slot);

								if (slot)
									slot->setFade(0);
							}

							_engine->getNISManager()->doNIS("1601.NIS", 0x4000);
							_engine->getLogicManager()->_gameEvents[kEventIntro] = 1;
						}
					}
				}
			}

			if (!_engine->getLogicManager()->_gameEvents[kEventIntro]) {
				_engine->getLogicManager()->_gameEvents[kEventIntro] = 1;
				Slot *slot = _engine->getSoundManager()->_soundCache;
				if (_engine->getSoundManager()->_soundCache) {
					do {
						if (slot->_tag == kSoundTagIntro)
							break;

						slot = slot->_next;
					} while (slot);

					if (slot)
						slot->setFade(0);

					_engine->getLogicManager()->fadeToBlack();
				}
			}
		} else {
			Slot *slot = _engine->getSoundManager()->_soundCache;
			if (_engine->getSoundManager()->_soundCache) {
				do {
					if (slot->_tag == kSoundTagIntro)
						break;

					slot = slot->_next;
				} while (slot);

				if (slot)
					slot->setFade(0);

				_engine->getLogicManager()->fadeToBlack();
			}
		}

		_engine->_navigationEngineIsRunning = true;
		_engine->getVCR()->go();

		if (!_isShowingMenu)
			_engine->getLogicManager()->restoreIcons();

		return false;
	}
	case kMenuActionSwitchEggs:
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		if ((flags & 8) != 0) {
			setSprite(3, 1, true);
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB047", -1, 0);

			switchEggs(_engine->getVCR()->switchGames());

			_engine->_fightSkipCounter = 0;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		setSprite(3, 0, true);

		if (_gameInNotStartedInFile || _engine->_currentGameFileColorId == 5) {
			setSprite(0, 25, true);
		} else if (_engine->getVCR()->isVirgin(_engine->_currentGameFileColorId + 1)) {
			setSprite(0, 7, true);
		} else {
			switch (_engine->_currentGameFileColorId) {
			case 0:
				setSprite(0, 26, true);
				break;
			case 1:
				setSprite(0, 28, true);
				break;
			case 2:
				setSprite(0, 30, true);
				break;
			case 3:
				setSprite(0, 29, true);
				break;
			case 4:
				setSprite(0, 27, true);
				break;
			default:
				break;
			}
		}

		return true;
	case kMenuActionRewind:
		if (!_engine->_currentSavePoint) {
			clearSprites();
			return true;
		}

		if (_engine->getClock()->getTimeTo() < _engine->getClock()->getTimeShowing()) {
			clearSprites();
			return true;
		}

		if ((flags & 1) != 0) {
			if ((flags & 8) == 0)
				return true;

			if (_engine->getClock()->statusClock())
				_moveClockHandsFlag = false;

			setSprite(1, 2, true);
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->rewind();

			_moveClockHandsFlag = false;
		} else {
			setSprite(1, 1, true);

			if ((flags & 0x80) == 0)
				setSprite(0, 34, true);
		}

		return true;
	case kMenuActionFastForward:
		if (_engine->_lastSavePointIdInFile <= _engine->_currentSavePoint) {
			clearSprites();
			return true;
		}

		if (_engine->getClock()->getTimeTo() > _engine->getClock()->getTimeShowing()) {
			clearSprites();
			return true;
		}

		if ((flags & 1) != 0) {
			if ((flags & 8) == 0)
				return true;

			if (_engine->getClock()->statusClock())
				_moveClockHandsFlag = false;

			setSprite(1, 4, true);
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->forward();

			_moveClockHandsFlag = false;
		} else {
			setSprite(1, 3, true);

			if ((flags & 0x80) == 0)
				setSprite(0, 33, true);
		}

		return true;
	case kMenuActionGoToParis:
		if (_engine->_gameTimeOfLastSavePointInFile < 1037700 || _engine->getClock()->getTimeShowing() == 1037700 || _engine->getClock()->getTimeTo() == 1037700) {
			clearSprites();
			return true;
		}

		setCity(0);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(1037700);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		setSprite(0, 13, true);

		return true;
	case kMenuActionGoToStrasbourg:
		if (_engine->_gameTimeOfLastSavePointInFile < 1490400 || _engine->getClock()->getTimeShowing() == 1490400 || _engine->getClock()->getTimeTo() == 1490400) {
			clearSprites();
			return true;
		}

		setCity(1);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(1490400);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 1490400) {
			setSprite(0, 14, true);
		} else {
			setSprite(0, 15, true);
		}

		return true;
	case kMenuActionGoToMunich:
		if (_engine->_gameTimeOfLastSavePointInFile < 1852200 || _engine->getClock()->getTimeShowing() == 1852200 || _engine->getClock()->getTimeTo() == 1852200) {
			clearSprites();
			return true;
		}

		setCity(2);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(1852200);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 1852200) {
			setSprite(0, 17, true);
		} else {
			setSprite(0, 16, true);
		}

		return true;
	case kMenuActionGoToVienna:
		if (_engine->_gameTimeOfLastSavePointInFile < 2268000 || _engine->getClock()->getTimeShowing() == 2268000 || _engine->getClock()->getTimeTo() == 2268000) {
			clearSprites();
			return true;
		}

		setCity(3);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2268000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2268000) {
			setSprite(0, 18, true);
		} else {
			setSprite(0, 19, true);
		}

		return true;
	case kMenuActionGoToBudapest:
		if (_engine->_gameTimeOfLastSavePointInFile < 2551500 || _engine->getClock()->getTimeShowing() == 2551500 || _engine->getClock()->getTimeTo() == 2551500) {
			clearSprites();
			return true;
		}

		setCity(4);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2551500);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2551500) {
			setSprite(0, 21, true);
		} else {
			setSprite(0, 20, true);
		}

		return true;
	case kMenuActionGoToBelgrad:
		if (_engine->_gameTimeOfLastSavePointInFile < 2952000 || _engine->getClock()->getTimeShowing() == 2952000 || _engine->getClock()->getTimeTo() == 2952000) {
			clearSprites();
			return true;
		}

		setCity(5);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2952000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2952000) {
			setSprite(0, 22, true);
		} else {
			setSprite(0, 23, true);
		}

		return true;
	case kMenuActionGoToCostantinople:
		if (_engine->_gameTimeOfLastSavePointInFile < 4941000 || _engine->getClock()->getTimeShowing() == 4941000 || _engine->getClock()->getTimeTo() == 4941000) {
			clearSprites();
			return true;
		}

		setCity(6);

		if ((flags & 8) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(4941000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & 0x80) != 0)
			return true;

		setSprite(0, 24, true);
		return true;
	case kMenuActionVolumeDown:
	{
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		if (_engine->getSoundManager()->getMasterVolume() <= 0) {
			setSprite(2, 2, true);
			setSprite(0, -1, true);
			return true;
		}

		setSprite(0, 9, true);

		if ((flags & 8) == 0) {
			setSprite(2, 1, true);
			return true;
		}

		setSprite(2, 0, true);

		_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
		_engine->getSoundManager()->setMasterVolume(_engine->getSoundManager()->getMasterVolume() - 1);
		_engine->getVCR()->storeSettings();

		int32 delay = _engine->getSoundFrameCounter() + 15;
		if (_engine->getSoundFrameCounter() + 15 < _engine->getSoundFrameCounter())
			return true;

		do {
			_engine->getSoundManager()->soundThread();
		} while (delay > _engine->getSoundFrameCounter());

		return true;
	}
	case kMenuActionVolumeUp:
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		if (_engine->getSoundManager()->getMasterVolume() >= 7) {
			setSprite(2, 2, true);
			setSprite(0, -1, true);
			return true;
		} else {
			setSprite(0, 8, true);

			if ((flags & 8) != 0) {
				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);

				setSprite(2, 4, true);

				_engine->getSoundManager()->setMasterVolume(_engine->getSoundManager()->getMasterVolume() + 1);
				_engine->getVCR()->storeSettings();

				int32 delay = _engine->getSoundFrameCounter() + 15;
				if (_engine->getSoundFrameCounter() >= (_engine->getSoundFrameCounter() + 15)) {
					return true;
				}

				do {
					_engine->getSoundManager()->soundThread();
				} while (_engine->getSoundFrameCounter() < delay);

				return true;
			} else {
				setSprite(2, 3, true);
				return true;
			}
		}
	case kMenuActionBrightnessDown:
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		if (_engine->getGraphicsManager()->getGammaLevel() <= 0) {
			setSprite(2, 7, true);
			setSprite(0, -1, true);
			return true;
		}

		setSprite(0, 11, true);

		if ((flags & 8) == 0) {
			setSprite(2, 6, true);
			return true;
		}

		setSprite(2, 5, true);

		_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
		_engine->getGraphicsManager()->setGammaLevel(_engine->getGraphicsManager()->getGammaLevel() - 1);
		_engine->getVCR()->storeSettings();

		_engine->getArchiveManager()->loadBG(_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].sceneFilename);

		for (int i = 0; i < 8; i++) {
			_engine->getSpriteManager()->destroySprite(&_startMenuFrames[i], false);
			if (_menuSeqs[i]) {
				_engine->getMemoryManager()->freeMem(_menuSeqs[i]->rawSeqData);
				delete _menuSeqs[i];
				_menuSeqs[i] = nullptr;
			}
		}

		setSprite(0, 11, false);
		setSprite(2, 5, false);

		_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_backgroundBuffer);

		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backgroundBuffer, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), 0, 0, 640, 480);
			_engine->getGraphicsManager()->unlockSurface();
		}

		_engine->getGraphicsManager()->burstAll();
		return true;
	case kMenuActionBrightnessUp:
		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		} else {
			if (_engine->getGraphicsManager()->getGammaLevel() >= 6) {
				setSprite(2, 7, true);
				setSprite(0, -1, true);
				return true;
			} else {
				setSprite(0, 10, true);

				if ((flags & 8) != 0) {
					setSprite(2, 9, true);

					_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
					_engine->getGraphicsManager()->setGammaLevel(_engine->getGraphicsManager()->getGammaLevel() + 1);
					_engine->getVCR()->storeSettings();

					_engine->getArchiveManager()->loadBG(_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].sceneFilename);

					for (int i = 0; i < 8; i++) {
						_engine->getSpriteManager()->destroySprite(&_startMenuFrames[i], false);
						if (_menuSeqs[i]) {
							_engine->getMemoryManager()->freeMem(_menuSeqs[i]->rawSeqData);
							delete _menuSeqs[i];
							_menuSeqs[i] = nullptr;
						}
					}

					setSprite(0, 10, false);
					setSprite(2, 9, false);

					_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_backgroundBuffer);
					if (_engine->getGraphicsManager()->acquireSurface()) {
						_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backgroundBuffer, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), 0, 0, 640, 480);
						_engine->getGraphicsManager()->unlockSurface();
					}

					_engine->getGraphicsManager()->burstAll();
					return true;
				} else {
					setSprite(2, 8, true);
					return true;
				}
			}
		}
	default:
		clearSprites();
		return true;
	}
}

void Menu::setSprite(int sequenceType, int index, bool redrawFlag) {
	if (index == -1) {
		_engine->getSpriteManager()->destroySprite(&_startMenuFrames[sequenceType], redrawFlag);
	} else {
		if (!_menuSeqs[sequenceType]) {
			_menuSeqs[sequenceType] = _engine->getArchiveManager()->loadSeq(_eggButtonsSeqNames[sequenceType], 15, 0);
		}

		if (_menuSeqs[sequenceType]) {
			if (_startMenuFrames[sequenceType] != &_menuSeqs[sequenceType]->sprites[index]) {
				_engine->getSpriteManager()->destroySprite(&_startMenuFrames[sequenceType], false);

				_startMenuFrames[sequenceType] = &_menuSeqs[sequenceType]->sprites[index];
				_engine->getSpriteManager()->drawSprite(_startMenuFrames[sequenceType]);

				if (redrawFlag) {
					_engine->getSpriteManager()->drawCycle();
				}
			}
		}
	}
}

void Menu::setCity(int cityIndex) {
	setSprite((_cityIndexes[cityIndex] >> 6) + 3, _cityIndexes[cityIndex] & 0x3F, true);
}

void Menu::switchEggs(int whichEgg) {
	_engine->getSpriteManager()->destroySprite(&_startMenuFrames[3], false);
	_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_backgroundBuffer);

	if (_menuSeqs[3]) {
		_engine->getMemoryManager()->freeMem(_menuSeqs[3]->rawSeqData);
		delete _menuSeqs[3];
		_menuSeqs[3] = nullptr;
	}

	_gameInNotStartedInFile = _engine->_gameTimeOfLastSavePointInFile < 1061100;

	if (_engine->_gameTimeOfLastSavePointInFile >= 1061100) {
		_engine->getLogicManager()->bumpCathNode((5 * whichEgg) + 1);
	} else {
		_engine->getLogicManager()->bumpCathNode((5 * whichEgg) + 2);
	}

	_engine->getGraphicsManager()->setMouseDrawable(true);
	_engine->getLogicManager()->mouseStatus();

	if (_gameInNotStartedInFile && whichEgg == 0) {
		return;
	}

	if (_gameInNotStartedInFile) {
		Common::strcpy_s(_eggButtonsSeqNames[3], "aconblu3.seq");
	} else {
		switch (whichEgg) {
		case 0:
			Common::strcpy_s(_eggButtonsSeqNames[3], "aconred.seq");
			break;
		case 1:
			Common::strcpy_s(_eggButtonsSeqNames[3], "acongren.seq");
			break;
		case 2:
			Common::strcpy_s(_eggButtonsSeqNames[3], "aconpurp.seq");
			break;
		case 3:
			Common::strcpy_s(_eggButtonsSeqNames[3], "aconteal.seq");
			break;
		case 4:
			Common::strcpy_s(_eggButtonsSeqNames[3], "acongold.seq");
			break;
		case 5:
			Common::strcpy_s(_eggButtonsSeqNames[3], "aconblu3.seq");
			break;
		default:
			break;
		}
	}

	_menuSeqs[3] = _engine->getArchiveManager()->loadSeq(_eggButtonsSeqNames[3], 15, 0);
}

bool Menu::isShowingMenu() {
	return _isShowingMenu;
}

} // End of namespace LastExpress
