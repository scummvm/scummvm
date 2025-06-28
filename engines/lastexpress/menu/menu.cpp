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

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/savegame.h"

#include "lastexpress/menu/menu.h"
#include "lastexpress/menu/clock.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"

#include "common/config-manager.h"

namespace LastExpress {

Menu::Menu(LastExpressEngine *engine) {
	_engine = engine;

	if (_engine->isDemo())
		_eggTimerDelta = DEMO_TIMEOUT;
}

Menu::~Menu() {
	for (int i = 0; i < 8; i++) {
		if (_menuSeqs[i]) {
			_engine->getMemoryManager()->freeMem(_menuSeqs[i]->rawSeqData);
			delete _menuSeqs[i];
			_menuSeqs[i] = nullptr;
		}
	}
}

void Menu::doEgg(bool doSaveGame, int type, int32 time) {
	if (!_isShowingMenu) {
		_isShowingMenu = true;

		if (_engine->isDemo())
			_eggTimerDelta = DEMO_TIMEOUT;

		_engine->getOtisManager()->wipeAllGSysInfo();

		if (!_engine->mouseHasRightClicked()) {
			if (!_engine->isDemo() && _engine->getVCR()->isVirgin(0) && _engine->getArchiveManager()->lockCD(1)) {
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

					 _engine->waitForTimer(4);
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
		_engine->getLogicManager()->_activeItem = 0;
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

		_engine->getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::eggMouseWrapper);
		_engine->getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::eggTimerWrapper);

		_menuSeqs[1] = _engine->getArchiveManager()->loadSeq("buttns.seq", 15, 0);
		_menuSeqs[0] = _engine->getArchiveManager()->loadSeq("helpnewr.seq", 15, 0);
		switchEggs(_engine->_currentGameFileColorId);
		updateEgg();

		_engine->getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::eggMouseWrapper);
		_engine->getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::eggTimerWrapper);
	}
}

void Menu::endEgg() {
	_engine->getMessageManager()->setEventHandle(kEventChannelMouse, &LastExpressEngine::nodeStepMouseWrapper);
	_engine->getMessageManager()->setEventHandle(kEventChannelTimer, &LastExpressEngine::nodeStepTimerWrapper);

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
	if (_engine->isDemo())
		_eggTimerDelta = DEMO_TIMEOUT;

	if (_engine->getGraphicsManager()->canDrawMouse()) {
		bool redrawMouse = true;

		_engine->getGraphicsManager()->setMouseDrawable(false);
		_engine->getGraphicsManager()->burstMouseArea(false); // The original updated the screen, we don't to avoid flickering...
		_engine->_cursorX = event->x;
		_engine->_cursorY = event->y;
		_eggCurrentMouseFlags = (event->flags & kMouseFlagLeftButton) != 0;
		_engine->_cursorType = 0;

		if (_engine->_doShowCredits) {
			if ((event->flags & kMouseFlagRightDown) != 0) {
				setSprite(7, -1, true);
				_engine->_doShowCredits = false;
			}

			if ((event->flags & kMouseFlagLeftDown) != 0) {
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

			for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].link; i; i = i->next) {
				if (_engine->getLogicManager()->pointIn(_engine->_cursorX, _engine->_cursorY, i) && i->location >= location) {
					location = i->location;
					foundLink = i;
				}
			}

			if (foundLink != _currentHotspotLink || (event->flags & kMouseFlagLeftDown) != 0 || (event->flags & kMouseFlagLeftUp) != 0) {
				_currentHotspotLink = foundLink;

				if ((event->flags & kMouseFlagLeftUp) != 0 && !_moveClockHandsFlag && _engine->getClock()->statusClock())
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

	if (!--_eggTimerDelta) {
		if (_engine->isDemo()) {
			endEgg();

			if (!_engine->demoEnding(false))
				_engine->getSaveManager()->removeSavegame(_engine->_savegameNames[_engine->_currentGameFileColorId]);

			doEgg(false, 0, 0);
		} else {
			updateEgg();
			_eggTimerDelta = 15;
		}
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

		for (Link *i = _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].link; i; i = i->next) {
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

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(1, 6, true);
			setSprite(0, -1, true);

			if (_engine->isDemo()) {
				_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
			} else {
				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			}

			clearSprites();

			_engine->_doShowCredits = true;
			_eggCreditsIndex = 0;

			setSprite(7, 0, true);

			return true;
		} else {
			if ((flags & kMouseFlagLeftUp) != 0)
				return true;

			setSprite(1, 5, true);
			setSprite(0, 32, true);

			return true;
		}
	case kMenuActionQuit:
		setSprite(0, 12, true);

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(2, 11, true);

			if (!ConfMan.getBool("confirm_exit")) {
				_engine->getSoundManager()->killAllSlots();
				_engine->getSoundManager()->soundThread();
			}

			if (_engine->isDemo()) {
				_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
			} else {
				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			}

			while (_engine->getLogicManager()->dialogRunning("LIB046")) {
				_engine->getSoundManager()->soundThread();
				_engine->waitForTimer(4);
			}

			g_system->delayMillis(334);

			_engine->getGraphicsManager()->setMouseDrawable(false);

			if (!ConfMan.getBool("confirm_exit")) {
				endEgg();
			}

			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);

			_engine->_exitFromMenuButton = true;

			if (ConfMan.getBool("confirm_exit")) {
				return true;
			}
		} else {
			setSprite(2, 10, true);
			return true;
		}

		return false;
	case kMenuActionPlayGame:
	case kMenuAction4:
	{
		if (action == kMenuAction4) {
			if (_engine->isDemo()) {
				clearSprites();
				return true;
			}

			if ((flags & kMouseFlagLeftDown) != 0)
				_engine->_currentSavePoint = 0;
		}

		if (_engine->getClock()->statusClock()) {
			clearSprites();
			return true;
		}

		int whichCD = 1;
		if (_engine->getLogicManager()->_globals[kGlobalChapter] > 1)
			whichCD = (_engine->getLogicManager()->_globals[kGlobalChapter] > 3) + 2;

		if (_engine->isDemo()) {
			if (!_gameInNotStartedInFile) {
				setSprite(1, 7, true);
				if (_engine->_currentSavePoint == _engine->_lastSavePointIdInFile) {
					if (_engine->getVCR()->currentEndsGame()) {
						setSprite(0, 6, true);
					} else {
						setSprite(0, 3, true);
					}
				} else {
					setSprite(0, 5, true);
				}
			}

			if ((flags & kMouseFlagLeftDown) == 0)
				return true;

			_engine->getLogicManager()->loadTrain(whichCD);
			setSprite(0, -1, true);
			_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
			_engine->getMessageManager()->clearMessageQueue();
			endEgg();

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

			_engine->getLogicManager()->fadeToBlack();
		} else {
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

			if ((flags & kMouseFlagLeftDown) == 0)
				return true;

			if (!_engine->getArchiveManager()->lockCD(whichCD))
				return true;

			_engine->getLogicManager()->loadTrain(whichCD);

			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getMessageManager()->clearMessageQueue();
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
								_engine->getLogicManager()->_doneNIS[kEventIntro] = 1;
							}
						}
					}
				}

				if (!_engine->getLogicManager()->_doneNIS[kEventIntro]) {
					_engine->getLogicManager()->_doneNIS[kEventIntro] = 1;
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

		if (_engine->isDemo()) {
			if ((flags & (kMouseFlagLeftDown | kMouseFlagLeftUp)) != 0)
				return true;

			setSprite(3, 0, true);
			setSprite(0, 7, true);
			return true;
		} else {
			if ((flags & kMouseFlagLeftDown) != 0) {
				setSprite(3, 1, true);
				setSprite(0, -1, true);

				_engine->getLogicManager()->playDialog(0, "LIB047", -1, 0);

				switchEggs(_engine->getVCR()->switchGames());

				_engine->_fightSkipCounter = 0;
				return true;
			}

			if ((flags & kMouseFlagLeftUp) != 0)
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
		}
	case kMenuActionRewind:
		if (!_engine->_currentSavePoint) {
			clearSprites();
			return true;
		}

		if (_engine->getClock()->getTimeTo() < _engine->getClock()->getTimeShowing()) {
			clearSprites();
			return true;
		}

		if ((flags & kMouseFlagLeftButton) != 0) {
			if ((flags & kMouseFlagLeftDown) == 0)
				return true;

			if (_engine->getClock()->statusClock())
				_moveClockHandsFlag = false;

			setSprite(1, 2, true);
			setSprite(0, -1, true);

			if (_engine->isDemo()) {
				_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
			} else {
				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			}

			_engine->getVCR()->rewind();

			_moveClockHandsFlag = false;
		} else {
			setSprite(1, 1, true);

			if ((flags & kMouseFlagLeftUp) == 0)
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

		if ((flags & kMouseFlagLeftButton) != 0) {
			if ((flags & kMouseFlagLeftDown) == 0)
				return true;

			if (_engine->getClock()->statusClock())
				_moveClockHandsFlag = false;

			setSprite(1, 4, true);
			setSprite(0, -1, true);

			if (_engine->isDemo()) {
				_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
			} else {
				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			}

			_engine->getVCR()->forward();

			_moveClockHandsFlag = false;
		} else {
			setSprite(1, 3, true);

			if ((flags & kMouseFlagLeftUp) == 0)
				setSprite(0, 33, true);
		}

		return true;
	case kMenuActionGoToParis:
		if (_engine->_gameTimeOfLastSavePointInFile < 1037700 || _engine->getClock()->getTimeShowing() == 1037700 || _engine->getClock()->getTimeTo() == 1037700) {
			clearSprites();
			return true;
		}

		setCity(0);

		if (_engine->isDemo()) {
			// Demo doesn't implement time seeking...
			if ((flags & (kMouseFlagLeftDown | kMouseFlagLeftUp)) != 0)
				return true;

			setSprite(0, 13, true);
			return true;
		} else {
			if ((flags & kMouseFlagLeftDown) != 0) {
				setSprite(0, -1, true);

				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
				_engine->getVCR()->seekToTime(1037700);

				_moveClockHandsFlag = true;
				return true;
			}

			if ((flags & kMouseFlagLeftUp) != 0)
				return true;

			setSprite(0, 13, true);

			return true;
		}
	case kMenuActionGoToStrasbourg:
		if (_engine->_gameTimeOfLastSavePointInFile < 1490400 || _engine->getClock()->getTimeShowing() == 1490400 || _engine->getClock()->getTimeTo() == 1490400) {
			clearSprites();
			return true;
		}

		setCity(1);

		if (_engine->isDemo()) {
			// Demo doesn't implement time seeking...
			if ((flags & (kMouseFlagLeftDown | kMouseFlagLeftUp)) != 0)
				return true;

			if (_engine->getClock()->getTimeShowing() <= 1490400) {
				setSprite(0, 14, true);
			} else {
				setSprite(0, 15, true);
			}

			return true;
		} else {
			if ((flags & kMouseFlagLeftDown) != 0) {
				setSprite(0, -1, true);

				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
				_engine->getVCR()->seekToTime(1490400);

				_moveClockHandsFlag = true;
				return true;
			}

			if ((flags & kMouseFlagLeftUp) != 0)
				return true;

			if (_engine->getClock()->getTimeShowing() <= 1490400) {
				setSprite(0, 14, true);
			} else {
				setSprite(0, 15, true);
			}

			return true;
		}
	case kMenuActionGoToMunich:
		if (_engine->_gameTimeOfLastSavePointInFile < 1852200 || _engine->getClock()->getTimeShowing() == 1852200 || _engine->getClock()->getTimeTo() == 1852200) {
			clearSprites();
			return true;
		}

		setCity(2);

		if (_engine->isDemo()) {
			// Demo doesn't implement time seeking...
			if ((flags & (kMouseFlagLeftDown | kMouseFlagLeftUp)) != 0)
				return true;

			if (_engine->getClock()->getTimeShowing() <= 1852200) {
				setSprite(0, 17, true);
			} else {
				setSprite(0, 16, true);
			}

			return true;
		} else {
			if ((flags & kMouseFlagLeftDown) != 0) {
				setSprite(0, -1, true);

				_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
				_engine->getVCR()->seekToTime(1852200);

				_moveClockHandsFlag = true;
				return true;
			}

			if ((flags & kMouseFlagLeftUp) != 0)
				return true;

			if (_engine->getClock()->getTimeShowing() <= 1852200) {
				setSprite(0, 17, true);
			} else {
				setSprite(0, 16, true);
			}

			return true;
		}
	case kMenuActionGoToVienna:
		if (_engine->isDemo()) {
			clearSprites();
			return true;
		}

		if (_engine->_gameTimeOfLastSavePointInFile < 2268000 || _engine->getClock()->getTimeShowing() == 2268000 || _engine->getClock()->getTimeTo() == 2268000) {
			clearSprites();
			return true;
		}

		setCity(3);

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2268000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & kMouseFlagLeftUp) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2268000) {
			setSprite(0, 18, true);
		} else {
			setSprite(0, 19, true);
		}

		return true;
	case kMenuActionGoToBudapest:
		if (_engine->isDemo()) {
			clearSprites();
			return true;
		}

		if (_engine->_gameTimeOfLastSavePointInFile < 2551500 || _engine->getClock()->getTimeShowing() == 2551500 || _engine->getClock()->getTimeTo() == 2551500) {
			clearSprites();
			return true;
		}

		setCity(4);

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2551500);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & kMouseFlagLeftUp) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2551500) {
			setSprite(0, 21, true);
		} else {
			setSprite(0, 20, true);
		}

		return true;
	case kMenuActionGoToBelgrad:
		if (_engine->isDemo()) {
			clearSprites();
			return true;
		}

		if (_engine->_gameTimeOfLastSavePointInFile < 2952000 || _engine->getClock()->getTimeShowing() == 2952000 || _engine->getClock()->getTimeTo() == 2952000) {
			clearSprites();
			return true;
		}

		setCity(5);

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(2952000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & kMouseFlagLeftUp) != 0)
			return true;

		if (_engine->getClock()->getTimeShowing() <= 2952000) {
			setSprite(0, 22, true);
		} else {
			setSprite(0, 23, true);
		}

		return true;
	case kMenuActionGoToCostantinople:
		if (_engine->isDemo()) {
			clearSprites();
			return true;
		}

		if (_engine->_gameTimeOfLastSavePointInFile < 4941000 || _engine->getClock()->getTimeShowing() == 4941000 || _engine->getClock()->getTimeTo() == 4941000) {
			clearSprites();
			return true;
		}

		setCity(6);

		if ((flags & kMouseFlagLeftDown) != 0) {
			setSprite(0, -1, true);

			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
			_engine->getVCR()->seekToTime(4941000);

			_moveClockHandsFlag = true;
			return true;
		}

		if ((flags & kMouseFlagLeftUp) != 0)
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

		if ((flags & kMouseFlagLeftDown) == 0) {
			setSprite(2, 1, true);
			return true;
		}

		setSprite(2, 0, true);

		if (_engine->isDemo()) {
			_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
		} else {
			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
		}

		_engine->getSoundManager()->setMasterVolume(_engine->getSoundManager()->getMasterVolume() - 1);
		_engine->getVCR()->storeSettings();

		int32 delay = _engine->getSoundFrameCounter() + 15;
		if (_engine->getSoundFrameCounter() + 15 < _engine->getSoundFrameCounter())
			return true;

		do {
			_engine->getSoundManager()->soundThread();
			_engine->waitForTimer(4);
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

			if ((flags & kMouseFlagLeftDown) != 0) {
				if (_engine->isDemo()) {
					_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
				} else {
					_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
				}

				setSprite(2, 4, true);

				_engine->getSoundManager()->setMasterVolume(_engine->getSoundManager()->getMasterVolume() + 1);
				_engine->getVCR()->storeSettings();

				int32 delay = _engine->getSoundFrameCounter() + 15;
				if (_engine->getSoundFrameCounter() >= (_engine->getSoundFrameCounter() + 15)) {
					return true;
				}

				do {
					_engine->getSoundManager()->soundThread();
					_engine->waitForTimer(4);
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

		if ((flags & kMouseFlagLeftDown) == 0) {
			setSprite(2, 6, true);
			return true;
		}

		setSprite(2, 5, true);

		if (_engine->isDemo()) {
			_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
		} else {
			_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
		}

		_engine->getGraphicsManager()->setGammaLevel(_engine->getGraphicsManager()->getGammaLevel() - 1);
		_engine->getVCR()->storeSettings();

		_engine->getArchiveManager()->loadBG(_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].sceneFilename);

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

		_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_frontBuffer);

		if (_engine->getGraphicsManager()->acquireSurface()) {
			_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_frontBuffer, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), 0, 0, 640, 480);
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

				if ((flags & kMouseFlagLeftDown) != 0) {
					setSprite(2, 9, true);

					if (_engine->isDemo()) {
						_engine->getSoundManager()->playSoundFile("LIB046.SND", 16, 0, 0);
					} else {
						_engine->getLogicManager()->playDialog(0, "LIB046", -1, 0);
					}

					_engine->getGraphicsManager()->setGammaLevel(_engine->getGraphicsManager()->getGammaLevel() + 1);
					_engine->getVCR()->storeSettings();

					_engine->getArchiveManager()->loadBG(_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].sceneFilename);

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

					_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_frontBuffer);
					if (_engine->getGraphicsManager()->acquireSurface()) {
						_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_frontBuffer, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(), 0, 0, 640, 480);
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
	_engine->getSpriteManager()->drawCycleSimple(_engine->getGraphicsManager()->_frontBuffer);

	if (_menuSeqs[3]) {
		_engine->getMemoryManager()->freeMem(_menuSeqs[3]->rawSeqData);
		delete _menuSeqs[3];
		_menuSeqs[3] = nullptr;
	}

	_gameInNotStartedInFile = _engine->isDemo() || _engine->_gameTimeOfLastSavePointInFile < 1061100;

	if (_engine->isDemo() || _engine->_gameTimeOfLastSavePointInFile >= 1061100) {
		_engine->getLogicManager()->bumpCathNode((5 * whichEgg) + 1);
	} else {
		_engine->getLogicManager()->bumpCathNode((5 * whichEgg) + 2);
	}

	_engine->getGraphicsManager()->setMouseDrawable(true);
	_engine->getLogicManager()->mouseStatus();

	if (_engine->isDemo()) {
		Common::strcpy_s(_eggButtonsSeqNames[3], "aconred.seq");
		_menuSeqs[3] = _engine->getArchiveManager()->loadSeq(_eggButtonsSeqNames[3], 15, 0);
		return;
	}

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

void Menu::setEggTimerDelta(int delta) {
	_eggTimerDelta = delta;
}

int Menu::getEggTimerDelta() {
	return _eggTimerDelta;
}

} // End of namespace LastExpress
