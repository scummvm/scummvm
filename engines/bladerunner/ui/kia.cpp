/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/ui/kia.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/combat.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/shape.h"
#include "bladerunner/script/kia_script.h"
#include "bladerunner/settings.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/kia_log.h"
#include "bladerunner/ui/kia_section_base.h"
#include "bladerunner/ui/kia_section_clues.h"
#include "bladerunner/ui/kia_section_crimes.h"
#include "bladerunner/ui/kia_section_diagnostic.h"
#include "bladerunner/ui/kia_section_help.h"
#include "bladerunner/ui/kia_section_load.h"
#include "bladerunner/ui/kia_section_settings.h"
#include "bladerunner/ui/kia_section_pogo.h"
#include "bladerunner/ui/kia_section_save.h"
#include "bladerunner/ui/kia_section_suspects.h"
#include "bladerunner/ui/kia_shapes.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/subtitles.h"

#include "common/str.h"
#include "common/keyboard.h"

namespace BladeRunner {

const char *KIA::kPogo = "POGO";

KIA::KIA(BladeRunnerEngine *vm) {
	_vm = vm;

	_script = new KIAScript(_vm);
	_log = new KIALog(_vm);
	_shapes = new KIAShapes(_vm);

	_forceOpen = false;
	_currentSectionId = kKIASectionNone;
	_lastSectionIdKIA = kKIASectionCrimes;
	_lastSectionIdOptions = kKIASectionSettings;
	_playerVqaTimeLast = _vm->_time->currentSystem();
	_playerVqaFrame = 0;
	_playerVisualizerState = 0;
	_playerPhotographId = -1;
	_playerPhotograph = nullptr;
	_playerSliceModelId = -1;
	_playerSliceModelAngle = 0.0f;
	_timeLast = _vm->_time->currentSystem();
	_playerActorDialogueQueuePosition = 0;
	_playerActorDialogueQueueSize = 0;
	_playerActorDialogueState = 0;
	_currentSection = nullptr;
	_mainVqaPlayer = nullptr;
	_playerVqaPlayer = nullptr;
	_transitionId = 0;

	_pogoPos = 0;

	// original imageCount was 22. We add +1 to have a description box for objects in cut content
	// We don't have separated cases here, for _vm->_cutContent since that causes assertion fault if
	// loading a "restored content" save game in a "original game" version
	_buttons = new UIImagePicker(_vm, 23);

	_crimesSection     = new KIASectionCrimes(_vm, _vm->_playerActor->_clues);
	_suspectsSection   = new KIASectionSuspects(_vm, _vm->_playerActor->_clues);
	_cluesSection      = new KIASectionClues(_vm, _vm->_playerActor->_clues);
	_settingsSection   = new KIASectionSettings(_vm);
	_helpSection       = new KIASectionHelp(_vm);
	_saveSection       = new KIASectionSave(_vm);
	_loadSection       = new KIASectionLoad(_vm);
	_diagnosticSection = new KIASectionDiagnostic(_vm);
	_pogoSection       = new KIASectionPogo(_vm);

	for (int i = 0; i < kPlayerActorDialogueQueueCapacity; ++i) {
		_playerActorDialogueQueue[i].actorId    = -1;
		_playerActorDialogueQueue[i].sentenceId = -1;
	}
}

KIA::~KIA() {
	if (isOpen()) {
		unload();
	}

	_thumbnail.free();
	delete _crimesSection;
	delete _suspectsSection;
	delete _cluesSection;
	delete _settingsSection;
	delete _helpSection;
	delete _saveSection;
	delete _loadSection;
	delete _diagnosticSection;
	delete _pogoSection;
	_playerImage.free();
	delete _playerPhotograph;
	delete _buttons;
	delete _shapes;
	delete _log;
	delete _script;
}

void KIA::reset() {
	_lastSectionIdKIA = kKIASectionCrimes;
	_lastSectionIdOptions = kKIASectionSettings;
	_playerImage.free();
	_playerVqaFrame = 0;
	_playerVisualizerState = 0;
	_playerSliceModelAngle = 0.0f;

	_crimesSection->reset();
	_suspectsSection->reset();
	_cluesSection->reset();
}

void KIA::openLastOpened() {
	open(_lastSectionIdKIA);
}

void KIA::open(KIASections sectionId) {
	if (_currentSectionId == sectionId) {
		return;
	}

	if (sectionId == kKIASectionNone) {
		unload();
		return;
	}

	if (!isOpen()) {
		init();
	}

	switch (_currentSectionId) {
	case kKIASectionCrimes:
		_crimesSection->saveToLog();
		break;
	case kKIASectionSuspects:
		_suspectsSection->saveToLog();
		break;
	case kKIASectionClues:
		_cluesSection->saveToLog();
		break;
	default:
		break;
	}

	if (sectionId != kKIASectionCrimes && sectionId != kKIASectionSuspects && sectionId != kKIASectionClues) {
		playerReset();
	}

	_transitionId = getTransitionId(_currentSectionId, sectionId);
	const char *name = getSectionVqaName(sectionId);
	if (getSectionVqaName(_currentSectionId) != name) {
		if (_mainVqaPlayer) {
			_mainVqaPlayer->close();
			delete _mainVqaPlayer;
			_mainVqaPlayer = nullptr;
		}

		_mainVqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, name);
		_mainVqaPlayer->open();
	}

	if (_transitionId) {
		playTransitionSound(_transitionId);
		_mainVqaPlayer->setLoop(getVqaLoopTransition(_transitionId), -1, kLoopSetModeImmediate, nullptr, nullptr);
		_mainVqaPlayer->setLoop(getVqaLoopMain(sectionId), -1, kLoopSetModeEnqueue, loopEnded, this);
	} else {
		int loopId = getVqaLoopMain(sectionId);
		_mainVqaPlayer->setLoop(loopId, -1, kLoopSetModeImmediate, nullptr, nullptr);
		_mainVqaPlayer->setLoop(loopId + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);
	}

	_buttons->resetImages();
	createButtons(sectionId);
	switchSection(sectionId);
	_currentSectionId = sectionId;

	if (sectionId == kKIASectionCrimes || sectionId == kKIASectionSuspects || sectionId == kKIASectionClues) {
		_lastSectionIdKIA = _currentSectionId;
	}

	if (sectionId == kKIASectionSettings || sectionId == kKIASectionHelp || sectionId == kKIASectionSave || sectionId == kKIASectionLoad) {
		 _lastSectionIdOptions = _currentSectionId;
	}
}

bool KIA::isOpen() const {
	return _currentSectionId != kKIASectionNone;
}

void KIA::tick() {
	if (!isOpen()) {
		return;
	}

	uint32 timeNow = _vm->_time->currentSystem();
	// unsigned difference is intentional
	uint32 timeDiff = timeNow - _timeLast;

	if (_playerActorDialogueQueueSize == _playerActorDialogueQueuePosition) {
		_playerActorDialogueState = 0;
	} else if (_playerActorDialogueState == 0) {
		if (_playerSliceModelId == -1 && _playerPhotographId == -1 && _playerImage.getPixels() == nullptr) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBEEP16), 70, 0, 0, 50, 0);
		}
		_playerActorDialogueState = 1;
	} else if (_playerActorDialogueState == 200) {
		if (!_vm->_actors[_playerActorDialogueQueue[_playerActorDialogueQueuePosition].actorId]->isSpeeching()) {
			if (_playerActorDialogueQueueSize != _playerActorDialogueQueuePosition) {
				_playerActorDialogueQueuePosition = (_playerActorDialogueQueuePosition + 1) % kPlayerActorDialogueQueueCapacity;
			}
			if (_playerActorDialogueQueueSize != _playerActorDialogueQueuePosition) {
				_vm->_actors[_playerActorDialogueQueue[_playerActorDialogueQueuePosition].actorId]->speechPlay(_playerActorDialogueQueue[_playerActorDialogueQueuePosition].sentenceId, true);
			}
		}
	} else {
		_playerActorDialogueState += timeDiff;

		if (_playerActorDialogueState >= 200) {
			_playerActorDialogueState = 200;
			_vm->_actors[_playerActorDialogueQueue[_playerActorDialogueQueuePosition].actorId]->speechPlay(_playerActorDialogueQueue[_playerActorDialogueQueuePosition].sentenceId, true);
		}
	}

	uint32 timeDiffDiv48 = (timeNow < _playerVqaTimeLast) ? 0u : (timeNow - _playerVqaTimeLast) / 48;
	if (timeDiffDiv48 > 0u) {
		_playerVqaTimeLast = timeNow;
		if (_playerActorDialogueQueueSize == _playerActorDialogueQueuePosition || _playerSliceModelId != -1 || _playerPhotographId != -1 || _playerImage.getPixels() != nullptr) {
			if (_playerVisualizerState > 0) {
				_playerVisualizerState = (_playerVisualizerState < timeDiffDiv48) ? 0u : MAX<uint32>(_playerVisualizerState - timeDiffDiv48, 0u);
			}
		} else {
			if (_playerVisualizerState < 2) {
				_playerVisualizerState = MIN<uint32>(_playerVisualizerState + timeDiffDiv48, 2u);
			}
		}

		if ( _playerSliceModelId != -1 || _playerPhotographId != -1 || _playerImage.getPixels() != nullptr) {
			if (_playerVqaFrame < 8) {
				int newVqaFrame  = MIN<uint32>(timeDiffDiv48 + _playerVqaFrame, 8u);
				if (_playerVqaFrame <= 0 && newVqaFrame > 0) {
					_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxMECHAN1), 100, 70, 70, 50, 0);
				}
				_playerVqaFrame = newVqaFrame;
			}
		} else {
			if (_playerVqaFrame > 0) {
				int newVqaFrame = (_playerVqaFrame < timeDiffDiv48) ? 0 : MAX<uint32>(_playerVqaFrame - timeDiffDiv48, 0u);
				if (_playerVqaFrame >= 8 && newVqaFrame < 8) {
					_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxMECHAN1C), 100, 70, 70, 50, 0);
				}
				_playerVqaFrame = newVqaFrame;
			}
		}
	}

	_mainVqaPlayer->update(false);
	blit(_vm->_surfaceBack, _vm->_surfaceFront);

	Common::Point mouse = _vm->getMousePos();

	if (!_transitionId) {
		_buttons->handleMouseAction(mouse.x, mouse.y, false, false, false);
		if (_buttons->hasHoveredImage()) {
			_vm->_mouse->setCursor(1);
		} else {
			_vm->_mouse->setCursor(0);
		}
		if (_currentSection) {
			_currentSection->handleMouseMove(mouse.x, mouse.y);
		}
	}

	if (_vm->_gameFlags->query(kFlagKIAPrivacyAddon)) {
		_shapes->get(40)->draw(_vm->_surfaceFront, 0, 0);
		_shapes->get(41)->draw(_vm->_surfaceFront, 211, 447);
	}
	if (_currentSectionId != kKIASectionQuit && _transitionId != 14) {
		if (_vm->_settings->getDifficulty() > kGameDifficultyEasy) {
			_vm->_mainFont->drawString(&_vm->_surfaceFront, Common::String::format("%04d", _vm->_gameVars[kVariableChinyen]), 580, 341, _vm->_surfaceFront.w, _vm->_surfaceFront.format.RGBToColor(80, 96, 136));
		} else {
			_shapes->get(39)->draw(_vm->_surfaceFront, 583, 342);
		}
	}

	_playerVqaPlayer->seekToFrame(_playerVqaFrame);
	_playerVqaPlayer->update(true, true);

	_playerSliceModelAngle += (float)(timeDiff) * 1.0f/400.0f;
	while (_playerSliceModelAngle >= 2 * M_PI) {
		_playerSliceModelAngle -= (float)(2 * M_PI);
	}

	if (_playerVqaFrame == 8) {
		if (_playerSliceModelId != -1) {
			_vm->_sliceRenderer->drawOnScreen(_playerSliceModelId, 0, 585, 80, _playerSliceModelAngle, 100.0, _vm->_surfaceFront);
		} else if (_playerPhotographId != -1) {
			int width  = _playerPhotograph->getWidth();
			int height  = _playerPhotograph->getHeight();
			_playerPhotograph->draw(_vm->_surfaceFront, 590 - width / 2, 80 - height / 2);
		} else if (_playerImage.getPixels() != nullptr) {
			_vm->_surfaceFront.fillRect(Common::Rect(549, 49, 631, 111), _vm->_surfaceFront.format.RGBToColor(255, 255, 255));
			_vm->_surfaceFront.copyRectToSurface(_playerImage.getPixels(), _playerImage.pitch, 550, 50, _playerImage.w,  _playerImage.h);
		}
	}

	if (_playerVisualizerState == 1) {
		_shapes->get(51)->draw(_vm->_surfaceFront, 576, 174);
	} else if (_playerVisualizerState == 2) {
		_shapes->get(50)->draw(_vm->_surfaceFront, 576, 174);
		_shapes->get(_vm->_rnd.getRandomNumberRng(90, 105))->draw(_vm->_surfaceFront, 576, 174);
	}
	if (!_transitionId) {
		_buttons->draw(_vm->_surfaceFront);
		if (_currentSection) {
			_currentSection->draw(_vm->_surfaceFront);
		}
	}
	if (_vm->_settings->getAmmo(0) > 0) {
		if (_vm->_settings->getAmmoType() == 0) {
			_shapes->get(42)->draw(_vm->_surfaceFront, 147, 405);
		} else {
			_shapes->get(45)->draw(_vm->_surfaceFront, 140, 446);
		}
	}
	if (_vm->_settings->getAmmo(1) > 0) {
		if (_vm->_settings->getAmmoType() == 1) {
			_shapes->get(43)->draw(_vm->_surfaceFront, 167, 394);
		} else {
			_shapes->get(46)->draw(_vm->_surfaceFront, 160, 446);
		}
	}
	if (_vm->_settings->getAmmo(2) > 0) {
		if (_vm->_settings->getAmmoType() == 2) {
			_shapes->get(44)->draw(_vm->_surfaceFront, 189, 385);
		} else {
			_shapes->get(47)->draw(_vm->_surfaceFront, 182, 446);
		}
	}
	_vm->_mainFont->drawString(&_vm->_surfaceFront, "1.00", 438, 471, _vm->_surfaceFront.w, _vm->_surfaceFront.format.RGBToColor(56, 56, 56)); // 1.01 is DVD version, but only cd handling routines were changed, no game logic
	if (!_transitionId) {
		_buttons->drawTooltip(_vm->_surfaceFront, mouse.x, mouse.y);
	}
	_vm->_mouse->draw(_vm->_surfaceFront, mouse.x, mouse.y);

	_vm->_subtitles->tick(_vm->_surfaceFront);

	_vm->blitToScreen(_vm->_surfaceFront);

	_timeLast = timeNow;
}

void KIA::resume() {
	// vqaPlayer::clear(this->_vqaPlayerMain);
	if (_transitionId) {
		_mainVqaPlayer->setLoop(getVqaLoopTransition(_transitionId), -1, kLoopSetModeImmediate, nullptr, nullptr);
		_mainVqaPlayer->setLoop(getVqaLoopMain(_currentSectionId), -1, kLoopSetModeEnqueue, loopEnded, this);
	} else {
		_mainVqaPlayer->setLoop(getVqaLoopMain(_currentSectionId), -1, kLoopSetModeImmediate, nullptr, nullptr);
		_mainVqaPlayer->setLoop(getVqaLoopMain(_currentSectionId) + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);
	}
}

void KIA::handleMouseDown(int mouseX, int mouseY, bool mainButton) {
	if (!isOpen()) {
		return;
	}
	if (mainButton) {
		_buttons->handleMouseAction(mouseX, mouseY, true, false, false);
	}
	if (_currentSection) {
		_currentSection->handleMouseDown(mainButton);
	}
}

void KIA::handleMouseUp(int mouseX, int mouseY, bool mainButton) {
	if (!isOpen()) {
		return;
	}
	if (mainButton) {
		_buttons->handleMouseAction(mouseX, mouseY, false, true, false);
	}
	if (_currentSection) {
		_currentSection->handleMouseUp(mainButton);
	}
	if (_currentSection && _currentSection->_scheduledSwitch) {
		if (_currentSectionId == kKIASectionCrimes) {
			open(kKIASectionSuspects);
			_suspectsSection->selectSuspect(_crimesSection->_suspectSelected);
			_log->next();
			_log->clearFuture();
		} else if (_currentSectionId == kKIASectionSuspects) {
			open(kKIASectionCrimes);
			_crimesSection->selectCrime(_suspectsSection->_crimeSelected);
			_log->next();
			_log->clearFuture();
		} else {
			open(kKIASectionNone);
		}
	}
}

void KIA::handleMouseScroll(int mouseX, int mouseY, int direction) {
	if (!isOpen()) {
		return;
	}
	if (_currentSection) {
		_currentSection->handleMouseScroll(direction);
	}
}

void KIA::handleKeyUp(const Common::KeyState &kbd) {
	if (!isOpen()) {
		return;
	}

	if (_currentSection) {
		_currentSection->handleKeyUp(kbd);
	}
}

void KIA::handleKeyDown(const Common::KeyState &kbd) {
	if (!isOpen()) {
		return;
	}

	if (toupper(kbd.ascii) != kPogo[_pogoPos]) {
		_pogoPos = 0;
	}
	if (_currentSectionId != kKIASectionSave) {
		if (toupper(kbd.ascii) == kPogo[_pogoPos]) {
			++_pogoPos;
			if (!kPogo[_pogoPos]) {
				open(kKIASectionPogo);
				_pogoPos = 0;
			}
		}
	}

	switch (kbd.keycode) {
	case Common::KEYCODE_ESCAPE:
		if (!_forceOpen) {
			open(kKIASectionNone);
		}
		break;

	case Common::KEYCODE_F1:
		open(kKIASectionHelp);
		break;

	case Common::KEYCODE_F2:
		if (!_forceOpen) {
			open(kKIASectionSave);
		}
		break;
	case Common::KEYCODE_F3:
		open(kKIASectionLoad);
		break;

	case Common::KEYCODE_F10:
		open(kKIASectionQuit);
		break;

	case Common::KEYCODE_F4:
		if (_currentSectionId != kKIASectionCrimes) {
			if (!_forceOpen) {
				open(kKIASectionCrimes);
				_log->next();
				_log->clearFuture();
			}
		}
		break;

	case Common::KEYCODE_F5:
		if (_currentSectionId != kKIASectionSuspects) {
			if (!_forceOpen) {
				open(kKIASectionSuspects);
				_log->next();
				_log->clearFuture();
			}
		}
		break;

	case Common::KEYCODE_F6:
		if (_currentSectionId != kKIASectionClues) {
			if (!_forceOpen) {
				open(kKIASectionClues);
				_log->next();
				_log->clearFuture();
			}
		}
		break;

	default:
		if (_currentSection) {
			_currentSection->handleKeyDown(kbd);
		}
		break;
	}

	if (_currentSection && _currentSection->_scheduledSwitch) {
		open(kKIASectionNone);
	}
}

void KIA::playerReset() {
	if (_playerActorDialogueQueueSize != _playerActorDialogueQueuePosition) {
		int actorId = _playerActorDialogueQueue[_playerActorDialogueQueuePosition].actorId;
		if (_vm->_actors[actorId]->isSpeeching()) {
			_vm->_actors[actorId]->speechStop();
		}
	}

	_playerActorDialogueQueueSize = _playerActorDialogueQueuePosition;
	_playerSliceModelId = -1;
	if (_playerPhotographId != -1) {
		delete _playerPhotograph;
		_playerPhotograph = nullptr;
	}
	_playerPhotographId = -1;
	_playerImage.free();
	_playerActorDialogueState = 0;
	if (_vm->_cutContent) {
		_buttons->resetImage(22);
	}
}

void KIA::playActorDialogue(int actorId, int sentenceId) {
	int newQueueSize = (_playerActorDialogueQueueSize + 1) % kPlayerActorDialogueQueueCapacity;
	if (newQueueSize != _playerActorDialogueQueuePosition) {
		_playerActorDialogueQueue[_playerActorDialogueQueueSize].actorId = actorId;
		_playerActorDialogueQueue[_playerActorDialogueQueueSize].sentenceId = sentenceId;
		_playerActorDialogueQueueSize = newQueueSize;
	}
}

void KIA::playSliceModel(int sliceModelId) {
	if (_playerVqaFrame == 8) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBEEP1), 70, 0, 0, 50, 0);
	}
	_playerSliceModelId = sliceModelId;
	if (_vm->_cutContent) {
		_buttons->defineImage(22, Common::Rect(530, 32, 635, 126), nullptr, nullptr, nullptr, _vm->_textClueTypes->getText(3)); // "OBJECT"
	}
}

void KIA::playPhotograph(int photographId) {
	if (_playerPhotographId != -1) {
		delete _playerPhotograph;
		_playerPhotograph = nullptr;
	}
	if (_playerVqaFrame == 8) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxBEEP1), 70, 0, 0, 50, 0);
	}
	_playerPhotographId = photographId;
	_playerPhotograph = new Shape(_vm);
	_playerPhotograph->open("photos.shp", photographId);
}

void KIA::playImage(const Graphics::Surface &image) {
	_playerImage.copyFrom(image);
	_playerImage.convertToInPlace(screenPixelFormat());
}

const char *KIA::scrambleSuspectsName(const char *name) {
	static char buffer[32];

	unsigned char *bufferPtr = (unsigned char *)buffer;
	const unsigned char *namePtr = (const unsigned char *)name;

	for (int i = 0 ; i < 6; ++i) {
		if (_vm->_language == Common::RU_RUS && _vm->_russianCP1251) {
			// Algorithm added by Siberian Studio in R4 patch
			if (*namePtr >= 0xC0) {
				unsigned char upper = *namePtr & 0xDF;
				if (upper < 201) {
					*bufferPtr++ = upper - 143; /* Map А-И to 1-9 */
				} else {
					*bufferPtr++ = upper - 136; /* Map Й-Я to A-W */
				}
			} else {
				*bufferPtr++ = '0';
			}
		} else {
			if (Common::isAlpha(*namePtr)) {
				char upper = toupper(*namePtr);
				if ( upper < 'J' ) {
					*bufferPtr++ = upper - 16; /* Map A-I to 1-9 */
				} else {
					*bufferPtr++ = upper - 9;  /* Map J-Z to A-Q */
				}
			} else {
				*bufferPtr++ = '0';
			}
		}
		if (*namePtr) {
			++namePtr;
		}
		if (i == 1) {
			*bufferPtr++ = '-';
		}
	}
	*bufferPtr = 0;
	return buffer;
}

void KIA::mouseDownCallback(int buttonId, void *callbackData) {
	KIA *self = (KIA *)callbackData;
	switch (buttonId) {
	case 0:
	case 6:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN4P), 100, -65, -65, 50, 0);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN5P), 70, 0, 0, 50, 0);
		if (buttonId == 12) {
			int endTrackId = self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxSHUTDOWN), 70, 0, 0, 50, 0);
			if (endTrackId != -1) {
				// wait until the full clip has played (similar to the original)
				while (self->_vm->_audioPlayer->isActive(endTrackId)) {
					self->_vm->_system->delayMillis(16);
				}
			}
		}
		break;
	case 15:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN4P), 70, 0, 0, 50, 0);
		break;
	default:
		break;
	}
}

void KIA::mouseUpCallback(int buttonId, void *callbackData) {
	KIA *self = (KIA *)callbackData;
	switch (buttonId) {
	case 0:
	case 6:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN4R), 100, -65, -65, 50, 0);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN5R), 70, 0, 0, 50, 0);
		break;
	case 15:
		self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxBUTN4R), 100, 0, 0, 50, 0);
		break;
	default:
		break;
	}
	self->buttonClicked(buttonId);
}

void KIA::loopEnded(void *callbackData, int frame, int loopId) {
	KIA *self = (KIA *)callbackData;
	self->_mainVqaPlayer->setLoop(self->getVqaLoopMain(self->_currentSectionId) + 1, -1, kLoopSetModeJustStart, nullptr, nullptr);
	self->_transitionId = 0;
}

void KIA::init() {
	_thumbnail = _vm->generateThumbnail();

	if (!_vm->openArchive("MODE.MIX")) {
		return;
	}

	playerReset();
	_playerVqaFrame = 0;
	_playerVqaTimeLast = _vm->_time->currentSystem();
	_timeLast = _vm->_time->currentSystem();

	if (_vm->_gameFlags->query(kFlagKIAPrivacyAddon) && !_vm->_gameFlags->query(kFlagKIAPrivacyAddonIntro)) {
		_vm->_gameFlags->set(kFlagKIAPrivacyAddonIntro);
		playPrivateAddon();
	}

	_shapes->load();
	_buttons->activate(nullptr, nullptr, mouseDownCallback, mouseUpCallback, this);
	_vm->_mouse->setCursor(0);
	if (_playerVqaPlayer == nullptr) {

		_playerVqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceFront, "kiaover.vqa");
		_playerVqaPlayer->open();
		_playerVqaPlayer->setLoop(0, -1, kLoopSetModeJustStart, nullptr, nullptr);
	}
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECTRO1), 70, 0, 0, 50, 0);

	_vm->_time->pause();
}

void KIA::unload() {
	_thumbnail.free();

	if (!isOpen()) {
		return;
	}

	_forceOpen = false;

	if (_currentSection) {
		_currentSection->close();
		_currentSection = nullptr;
	}

	_buttons->resetImages();
	_buttons->deactivate();

	_shapes->unload();

	if (_mainVqaPlayer) {
		_mainVqaPlayer->close();
		delete _mainVqaPlayer;
		_mainVqaPlayer = nullptr;
	}

	if (_playerVqaPlayer) {
		_playerVqaPlayer->close();
		delete _playerVqaPlayer;
		_playerVqaPlayer = nullptr;
	}

	_vm->closeArchive("MODE.MIX");

	_currentSectionId = kKIASectionNone;

	_vm->_time->resume();

	if (!_vm->_settings->isLoadingGame() && _vm->_gameIsRunning) {
		_vm->_scene->resume();
	}
}

void KIA::createButtons(int sectionId) {
	Common::Rect kiaButton6(  66,  00, 122,  44);
	Common::Rect kiaButton7( 191,  29, 233,  70);
	Common::Rect kiaButton8( 234,  29, 278,  70);
	Common::Rect kiaButton9( 278,  29, 321,  70);
	Common::Rect kiaButton10(322,  29, 365,  70);
	Common::Rect kiaButton11(366,  29, 410,  70);
	Common::Rect kiaButton12(420, 286, 472, 328);
	Common::Rect kiaButton13(334, 286, 386, 328);
	Common::Rect kiaButton14(411,  29, 453,  70);
	Common::Rect kiaButton15(264,   9, 304,  26);
	Common::Rect kiaButton16(140, 406, 160, 479);
	Common::Rect kiaButton17(161, 406, 180, 479);
	Common::Rect kiaButton18(181, 406, 202, 479);
	Common::Rect kiaButton19(575, 307, 606, 350);
	Common::Rect kiaButton21(211, 443, 291, 479);

	const Shape *shapeUp = nullptr;
	const Shape *shapeHovered = nullptr;
	const Shape *shapeDown = nullptr;

	switch (sectionId) {
	case kKIASectionCrimes:
	case kKIASectionSuspects:
	case kKIASectionClues:
		_buttons->defineImage(0, kiaButton6, nullptr, nullptr, _shapes->get(1), _vm->_textKIA->getText(23));

		if (_vm->_cutContent && _playerSliceModelId != -1) {
			_buttons->defineImage(22, Common::Rect(530, 32, 635, 126), nullptr, nullptr, nullptr, _vm->_textClueTypes->getText(3)); // "OBJECT"
		}

		if (sectionId == kKIASectionCrimes) {
			shapeUp = _shapes->get(2);
			shapeHovered = _shapes->get(2);
			shapeDown = _shapes->get(10);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(18);
		}
		_buttons->defineImage(1, kiaButton7, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(24));

		if (sectionId == kKIASectionSuspects) {
			shapeUp = _shapes->get(3);
			shapeHovered = _shapes->get(3);
			shapeDown = _shapes->get(11);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(19);
		}
		_buttons->defineImage(2, kiaButton8, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(25));

		if (sectionId == kKIASectionClues) {
			shapeUp = _shapes->get(4);
			shapeHovered = _shapes->get(4);
			shapeDown = _shapes->get(12);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(20);
		}
		_buttons->defineImage(3, kiaButton9, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(26));

		_buttons->defineImage(4, kiaButton10, nullptr, nullptr, _shapes->get(21), _vm->_textKIA->getText(27));

		_buttons->defineImage(5, kiaButton11, nullptr, nullptr, _shapes->get(22), _vm->_textKIA->getText(28));

		_buttons->defineImage(14, kiaButton14, nullptr, nullptr, _shapes->get(23), _vm->_textKIA->getText(29));

		break;
	case kKIASectionSettings:
	case kKIASectionHelp:
	case kKIASectionSave:
	case kKIASectionLoad:
	case kKIASectionDiagnostic:
	case kKIASectionPogo:
		_buttons->defineImage(6, kiaButton6, nullptr, nullptr, _shapes->get(0), _vm->_textKIA->getText(37));

		if (sectionId == kKIASectionSettings) {
			shapeUp = _shapes->get(5);
			shapeHovered = _shapes->get(5);
			shapeDown = _shapes->get(13);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(24);
		}
		_buttons->defineImage(7, kiaButton7, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(38));

		if (sectionId == kKIASectionHelp) {
			shapeUp = _shapes->get(6);
			shapeHovered = _shapes->get(6);
			shapeDown = _shapes->get(14);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(25);
		}
		_buttons->defineImage(8, kiaButton8, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(41));

		if (sectionId == kKIASectionSave) {
			shapeUp = _shapes->get(7);
			shapeHovered = _shapes->get(7);
			shapeDown = _shapes->get(15);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(26);
		}
		_buttons->defineImage(9, kiaButton9, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(39));

		if (sectionId == kKIASectionLoad) {
			shapeUp = _shapes->get(8);
			shapeHovered = _shapes->get(8);
			shapeDown = _shapes->get(16);
		} else {
			shapeUp = nullptr;
			shapeHovered = nullptr;
			shapeDown = _shapes->get(27);
		}
		_buttons->defineImage(10, kiaButton10, shapeUp, shapeHovered, shapeDown, _vm->_textKIA->getText(40));

		_buttons->defineImage(11, kiaButton11, nullptr, nullptr, _shapes->get(28), _vm->_textKIA->getText(42));

		_buttons->defineImage(14, kiaButton14, nullptr, nullptr, _shapes->get(29), _vm->_textKIA->getText(29));

		break;
	case kKIASectionQuit:
		_buttons->defineImage(12, kiaButton12, _shapes->get(124), _shapes->get(124), _shapes->get(48), _vm->_textKIA->getText(42));

		_buttons->defineImage(13, kiaButton13, _shapes->get(125), _shapes->get(125), _shapes->get(49), _vm->_textKIA->getText(29));
			break;
	}

	if (sectionId != kKIASectionQuit) {
		_buttons->defineImage(15, kiaButton15, nullptr, nullptr, _shapes->get(38), _vm->_textKIA->getText(43));

		_buttons->defineImage(19, kiaButton19, nullptr, nullptr, nullptr, _vm->_textKIA->getText(44));

		if (_vm->_settings->getAmmo(0) > 0) {
			_buttons->defineImage(16, kiaButton16, nullptr, nullptr, nullptr, _vm->_textKIA->getText(50));
		}
		Common::String tooltip;
		if (_vm->_settings->getAmmo(1) > 0) {
			if (_vm->_settings->getDifficulty() > kGameDifficultyEasy) {
				tooltip = Common::String::format("%d", _vm->_settings->getAmmo(1));
			} else {
				tooltip = _vm->_textKIA->getText(50);
			}
			_buttons->defineImage(17, kiaButton17, nullptr, nullptr, nullptr, tooltip.c_str());
		}
		if (_vm->_settings->getAmmo(2) > 0) {
			if (_vm->_settings->getDifficulty() > kGameDifficultyEasy) {
				tooltip = Common::String::format("%d", _vm->_settings->getAmmo(2));
			} else {
				tooltip = _vm->_textKIA->getText(50);
			}
			_buttons->defineImage(18, kiaButton18, nullptr, nullptr, nullptr, tooltip.c_str());
		}
		if (_vm->_gameFlags->query(kFlagKIAPrivacyAddon)) {
			_buttons->defineImage(21, kiaButton21, nullptr, nullptr, nullptr, nullptr);
		}
	}
}

void KIA::buttonClicked(int buttonId) {
	int soundId = 0;

	if (!isOpen()) {
		return;
	}
	switch (buttonId) {
	case 0:
		open(kKIASectionSettings);
		break;
	case 1:
		if (_currentSectionId != kKIASectionCrimes) {
			open(kKIASectionCrimes);
			_log->next();
			_log->clearFuture();
		}
		break;
	case 2:
		if (_currentSectionId != kKIASectionSuspects) {
			open(kKIASectionSuspects);
			_log->next();
			_log->clearFuture();
		}
		break;
	case 3:
		if (_currentSectionId != kKIASectionClues) {
			open(kKIASectionClues);
			_log->next();
			_log->clearFuture();
		}
		break;
	case 4:
		if (_log->hasPrev()) {
			int kiaLogPrevType = _log->getPrevType();
			switch (kiaLogPrevType) {
			case 2:
				open(kKIASectionCrimes);
				_log->prev();
				_crimesSection->loadFromLog();
				break;
			case 1:
				open(kKIASectionSuspects);
				_log->prev();
				_suspectsSection->loadFromLog();
				break;
			case 0:
				open(kKIASectionClues);
				_log->prev();
				_cluesSection->loadFromLog();
				break;
			}
		}
		break;
	case 5:
		if (_log->hasNext()) {
			int kiaLogNextType = _log->getNextType();
			switch (kiaLogNextType) {
			case 2:
				open(kKIASectionCrimes);
				_log->next();
				_crimesSection->loadFromLog();
				break;
			case 1:
				open(kKIASectionSuspects);
				_log->next();
				_suspectsSection->loadFromLog();
				break;
			case 0:
				open(kKIASectionClues);
				_log->next();
				_cluesSection->loadFromLog();
				break;
			}
		}
		break;
	case 6:
		if (!_forceOpen) {
			open(_lastSectionIdKIA);
		}
		break;
	case 7:
		open(kKIASectionSettings);
		break;
	case 8:
		open(kKIASectionHelp);
		break;
	case 9:
		if (!_forceOpen) {
			open(kKIASectionSave);
		}
		break;
	case 10:
		open(kKIASectionLoad);
		break;
	case 11:
		open(kKIASectionQuit);
		break;
	case 12:
		_vm->_gameIsRunning = false;
		open(kKIASectionNone);
		break;
	case 13:
		open(_lastSectionIdOptions);
		break;
	case 14:
		if (!_forceOpen) {
			open(kKIASectionNone);
		}
		break;
	case 15:
		open(kKIASectionDiagnostic);
		break;
	case 16:
		_vm->_settings->setAmmoType(0);
		if (_vm->_rnd.getRandomNumber(1)) {
			soundId = _vm->_combat->getHitSound();
		} else {
			soundId = _vm->_combat->getMissSound();
		}
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(soundId), 70, 0, 0, 50, 0);
		break;
	case 17:
		_vm->_settings->setAmmoType(1);
		if (_vm->_rnd.getRandomNumber(1)) {
			soundId = _vm->_combat->getHitSound();
		} else {
			soundId = _vm->_combat->getMissSound();
		}
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(soundId), 70, 0, 0, 50, 0);
		break;
	case 18:
		_vm->_settings->setAmmoType(2);
		if (_vm->_rnd.getRandomNumber(1)) {
			soundId = _vm->_combat->getHitSound();
		} else {
			soundId = _vm->_combat->getMissSound();
		}
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(soundId), 70, 0, 0, 50, 0);
		break;
	case 19:
		return;
	case 20:
		playerReset();
		break;
	case 21:
		playPrivateAddon();
		break;
	case 22:
		if (_vm->_cutContent) {
			// play audio description
			playObjectDescription();
		}
		break;
	}
}

void KIA::switchSection(int sectionId) {
	if (_currentSection) {
		_currentSection->close();
	}
	switch (sectionId) {
	case kKIASectionCrimes:
		_currentSection = _crimesSection;
		break;
	case kKIASectionSuspects:
		_currentSection = _suspectsSection;
		break;
	case kKIASectionClues:
		_currentSection = _cluesSection;
		break;
	case kKIASectionSettings:
		_currentSection = _settingsSection;
		break;
	case kKIASectionHelp:
		_currentSection = _helpSection;
		break;
	case kKIASectionSave:
		_currentSection = _saveSection;
		break;
	case kKIASectionLoad:
		_currentSection = _loadSection;
		break;
	case kKIASectionQuit:
		_currentSection = nullptr;
		break;
	case kKIASectionDiagnostic:
		_currentSection = _diagnosticSection;
		break;
	case kKIASectionPogo:
		_currentSection = _pogoSection;
		break;
	default:
		_currentSection = nullptr;
		break;
	}
	if (_currentSection) {
		_currentSection->open();
	}
}

const char *KIA::getSectionVqaName(int sectionId) {
	switch (sectionId) {
	case kKIASectionCrimes:
		return "kia_crim.vqa";
	case kKIASectionSuspects:
		return "kia_susp.vqa";
	case kKIASectionClues:
		return "kia_clue.vqa";
	case kKIASectionSettings:
	case kKIASectionHelp:
	case kKIASectionSave:
	case kKIASectionLoad:
	case kKIASectionQuit:
	case kKIASectionDiagnostic:
	case kKIASectionPogo:
		return "kia_ingm.vqa";
	default:
		return nullptr;
	}
}

int KIA::getVqaLoopMain(int sectionId) {
	switch (sectionId) {
	case kKIASectionCrimes:
	case kKIASectionSuspects:
	case kKIASectionClues:
		return 3;
	case kKIASectionSettings:
	case kKIASectionHelp:
	case kKIASectionSave:
	case kKIASectionLoad:
	case kKIASectionDiagnostic:
	case kKIASectionPogo:
		return 4;
	case kKIASectionQuit:
		return 7;
	default:
		return 0;
	}
}

int KIA::getVqaLoopTransition(int transitionId) {
	switch (transitionId) {
	case 0:
	case 2:
	case 7:
	case 8:
		return 0;
	case 1:
	case 4:
	case 5:
	case 9:
		return 1;
	case 3:
	case 10:
	case 11:
	case 12:
		return 2;
	case 6:
		return 3;
	case 13:
		return 6;
	default:
		return 0;
	}
}

int KIA::getTransitionId(int oldSectionId, int newSectionId) {
	switch (oldSectionId) {
	case kKIASectionNone:
		return 0;
	case kKIASectionCrimes:
		switch (newSectionId) {
		case kKIASectionCrimes:
			return 0;
		case kKIASectionSuspects:
			return 1;
		case kKIASectionClues:
			return 2;
		case kKIASectionSettings:
		case kKIASectionHelp:
		case kKIASectionSave:
		case kKIASectionLoad:
		case kKIASectionDiagnostic:
		case kKIASectionPogo:
			return 3;
		case kKIASectionQuit:
			return 13;
		}
		return 0;
	case kKIASectionSuspects:
		switch (newSectionId) {
		case kKIASectionCrimes:
			return 4;
		case kKIASectionSuspects:
			return 0;
		case kKIASectionClues:
			return 5;
		case kKIASectionSettings:
		case kKIASectionHelp:
		case kKIASectionSave:
		case kKIASectionLoad:
		case kKIASectionDiagnostic:
		case kKIASectionPogo:
			return 6;
		case kKIASectionQuit:
			return 13;
		}
		return 0;
	case kKIASectionClues:
		switch (newSectionId) {
		case kKIASectionCrimes:
			return 7;
		case kKIASectionSuspects:
			return 8;
		case kKIASectionClues:
			return 0;
		case kKIASectionSettings:
		case kKIASectionHelp:
		case kKIASectionSave:
		case kKIASectionLoad:
		case kKIASectionDiagnostic:
		case kKIASectionPogo:
			return 9;
		case kKIASectionQuit:
			return 13;
		}
		return 0;
	case kKIASectionSettings:
	case kKIASectionHelp:
	case kKIASectionSave:
	case kKIASectionLoad:
	case kKIASectionDiagnostic:
	case kKIASectionPogo:
		switch (newSectionId) {
		case kKIASectionCrimes:
			return 10;
		case kKIASectionSuspects:
			return 11;
		case kKIASectionClues:
			return 12;
		case kKIASectionSettings:
		case kKIASectionHelp:
		case kKIASectionSave:
		case kKIASectionLoad:
		case kKIASectionDiagnostic:
		case kKIASectionPogo:
			return 0;
		case kKIASectionQuit:
			return 13;
		}
		return 0;
	case kKIASectionQuit:
		if (newSectionId != kKIASectionQuit) {
			return 14;
		}
	}
	return 0;
}

void KIA::playTransitionSound(int transitionId) {
	switch (transitionId) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxPANEL1),  100, 0, 0, 50, 0);
		break;
	case 13:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxPANEL2),  100, 0, 0, 50, 0);
		break;
	case 14:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxPANOPEN), 100, 0, 0, 50, 0);
		break;
	}
}

void KIA::playPrivateAddon() {
	playerReset();
	playSliceModel(524);
	playActorDialogue(kActorBulletBob, 2060);
	playActorDialogue(kActorBulletBob, 2070);
}

void KIA::playObjectDescription() {
	if (_playerSliceModelId == -1) {
		return;
	}
	switch (_playerSliceModelId) {
	case kModelAnimationShellCasings:
		playActorDialogue(kActorMcCoy, 8730);
		break;
	case kModelAnimationCandy:
		playActorDialogue(kActorMcCoy, 8735);
		break;
	case kModelAnimationToyDog:
		playActorDialogue(kActorMcCoy, 8740);
		break;
	case kModelAnimationChopstickWrapper:
		playActorDialogue(kActorMcCoy, 8745);
		break;
	case kModelAnimationReferenceLetter:
		playActorDialogue(kActorMcCoy, 8750);
		break;
	case kModelAnimationChromeDebris:
		playActorDialogue(kActorMcCoy, 8755);
		break;
	case kModelAnimationLicensePlate:
		playActorDialogue(kActorMcCoy, 8760);
		break;
	case kModelAnimationDragonflyEarring:
		playActorDialogue(kActorMcCoy, 8765);
		break;
	case kModelAnimationDetonatorWire:
		playActorDialogue(kActorMcCoy, 8770);
		break;
	case kModelAnimationKingstonKitchenBox:
		playActorDialogue(kActorMcCoy, 8775);
		break;
	case kModelAnimationTyrellSalesPamphletKIA:
		playActorDialogue(kActorMcCoy, 8780);
		break;
	case kModelAnimationRadiationGoggles:
		playActorDialogue(kActorMcCoy, 8785);
		break;
	case kModelAnimationDogCollar:
		playActorDialogue(kActorMcCoy, 8790);
		break;
	case kModelAnimationMaggieBracelet:
		playActorDialogue(kActorMcCoy, 8795);
		break;
	case kModelAnimationEnvelope:
		playActorDialogue(kActorMcCoy, 8800);
		break;
	case kModelAnimationWeaponsOrderForm:
		// fall through
	case kModelAnimationRequisitionForm:
		// fall through
	case kModelAnimationOriginalShippingForm:
		// fall through
	case kModelAnimationOriginalRequisitionForm:
		playActorDialogue(kActorMcCoy, 8805); // A requisition form
		break;
	case kModelAnimationHysteriaToken:
		playActorDialogue(kActorMcCoy, 8810);
		break;
	case kModelAnimationRagDoll:
		playActorDialogue(kActorMcCoy, 8815);
		break;
	case kModelAnimationCheese:
		playActorDialogue(kActorMcCoy, 8820);
		break;
	case kModelAnimationDragonflyBelt:
		playActorDialogue(kActorMcCoy, 8825);
		break;
	case kModelAnimationStrangeScale:
		playActorDialogue(kActorMcCoy, 8830);
		break;
	case kModelAnimationDektorasCard:
		playActorDialogue(kActorMcCoy, 8835);
		break;
	case kModelAnimationGrigoriansNote:
		playActorDialogue(kActorMcCoy, 8840);
		break;
	case kModelAnimationCollectionReceipt:
		playActorDialogue(kActorMcCoy, 8845);
		break;
	case kModelAnimationGordosLighterReplicant:
		// fall through
	case kModelAnimationGordosLighterHuman:
		playActorDialogue(kActorMcCoy, 8850);
		break;
//	case kModelAnimationBadge: // This is never discovered in-game, and uses same model as Holden's badge
//		playActorDialogue(kActorMcCoy, 8855); // Baker's Badge
//		break;
	case kModelAnimationBadge:
		playActorDialogue(kActorMcCoy, 8860);
		break;
	case kModelAnimationLichenDogWrapper:
		playActorDialogue(kActorMcCoy, 8865);
		break;
	case kModelAnimationFolder:
		playActorDialogue(kActorMcCoy, 8870);
		break;
	case kModelAnimationCandyWrapper:
		playActorDialogue(kActorMcCoy, 8875);
		break;
	case kModelAnimationFlaskOfAbsinthe:
		playActorDialogue(kActorMcCoy, 8880);
		break;
	case kModelAnimationPowerSource:
		playActorDialogue(kActorMcCoy, 8885);
		break;
	case kModelAnimationBomb: // TODO - does this ever appear in KIA?
		playActorDialogue(kActorMcCoy, 8890);
		break;
	case kModelAnimationGarterSnake:
		playActorDialogue(kActorMcCoy, 8895);
		break;
	case kModelAnimationSlug:
		playActorDialogue(kActorMcCoy, 8900);
		break;
	case kModelAnimationGoldfish:
		playActorDialogue(kActorMcCoy, 8905);
		break;
	case kModelAnimationDNAEvidence01OutOf6:
//		fall through
//	case kModelAnimationDNAEvidence02OutOf6:
//		fall through
	case kModelAnimationDNAEvidence03OutOf6:
//		fall through
	case kModelAnimationDNAEvidence04OutOf6:
//		fall through
//	case kModelAnimationDNAEvidence05OutOf6:
//		fall through
	case kModelAnimationDNAEvidenceComplete:
//		fall through
	case kModelAnimationSpinnerKeys:
//		fall through
	case kModelAnimationBriefcase:
//		fall through
	case kModelAnimationCrystalsCigarette:
//		fall through
	case kModelAnimationVideoDisc:
//		fall through
	default:
		playActorDialogue(kActorMcCoy, 8525);
		break;
	}
}

} // End of namespace BladeRunner
