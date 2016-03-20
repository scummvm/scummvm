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

#include "common/scummsys.h"
#include "mads/scene.h"
#include "mads/compression.h"
#include "mads/mads.h"
#include "mads/audio.h"
#include "mads/dragonsphere/dragonsphere_scenes.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

Scene::Scene(MADSEngine *vm)
	: _vm(vm), _action(_vm), _depthSurface(),
	  _dirtyAreas(_vm),  _dynamicHotspots(vm), _hotspots(vm),
	  _kernelMessages(vm), _sequences(vm), _sprites(vm), _spriteSlots(vm),
	  _textDisplay(vm), _userInterface(vm) {
	_priorSceneId = 0;
	_nextSceneId = 0;
	_currentSceneId = 0;
	_sceneLogic = nullptr;
	_sceneInfo = nullptr;
	_cyclingActive = false;
	_cyclingThreshold = 0;
	_cyclingDelay = 0;
	_totalCycleColors = 0;
	_depthStyle = 0;
	_roomChanged = false;
	_reloadSceneFlag = false;
	_freeAnimationFlag = false;
	_animationData = nullptr;
	for (int i = 0; i < 10; i++)
		_animation[i] = nullptr;
	_textSpacing = -1;
	_frameStartTime = 0;
	_mode = SCREENMODE_VGA;
	_lookFlag = false;
	_bandsRange = 0;
	_scaleRange = 0;
	_interfaceY = 0;
	_spritesCount = 0;
	_variant = 0;

	_speechReady = -1;

	_paletteUsageF.push_back(PaletteUsage::UsageEntry(0xF));

	// Set up a scene surface that maps to our physical screen drawing surface
	restrictScene();

	// Set up the verb list
	_verbList.push_back(VerbInit(VERB_LOOK, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_TAKE, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_PUSH, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_OPEN, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_PUT, VERB_THIS, PREP_RELATIONAL));
	_verbList.push_back(VerbInit(VERB_TALKTO, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_GIVE, VERB_THIS, PREP_TO));
	_verbList.push_back(VerbInit(VERB_PULL, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_CLOSE, VERB_THAT, PREP_NONE));
	_verbList.push_back(VerbInit(VERB_THROW, VERB_THIS, PREP_AT));
}

Scene::~Scene() {
	delete _sceneLogic;
	delete _sceneInfo;
	delete _animationData;
}

void Scene::restrictScene() {
	_sceneSurface.create(*_vm->_screen, Common::Rect(0, 0, MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT));
}

void Scene::clearVocab() {
	_activeVocabs.clear();
}

void Scene::addActiveVocab(int vocabId) {
	if (activeVocabIndexOf(vocabId) == -1) {
		assert(_activeVocabs.size() < 200);
		_activeVocabs.push_back(vocabId);
	}
}

int Scene::activeVocabIndexOf(int vocabId) {
	for (uint i = 0; i < _activeVocabs.size(); ++i) {
		if (_activeVocabs[i] == vocabId)
			return i;
	}

	return -1;
}

void Scene::clearSequenceList() {
	_sequences.clear();
}

void Scene::clearMessageList() {
	_kernelMessages.clear();
	_talkFont = FONT_CONVERSATION;
	_textSpacing  = -1;
}

void Scene::loadSceneLogic() {
	delete _sceneLogic;

	switch (_vm->getGameID()) {
	case GType_RexNebular:
		_sceneLogic = Nebular::SceneFactory::createScene(_vm);
		break;
	case GType_Dragonsphere:
		_sceneLogic = Dragonsphere::SceneFactory::createScene(_vm);
		break;
	case GType_Phantom:
		_sceneLogic = Phantom::SceneFactory::createScene(_vm);
		break;
	default:
		error("Scene logic: Unknown game");
	}
}

void Scene::loadScene(int sceneId, const Common::String &prefix, bool palFlag) {
	// Store the previously active scene number and set the new one
	_priorSceneId = _currentSceneId;
	_currentSceneId = sceneId;

	_variant = 0;
	if (palFlag)
		_vm->_palette->resetGamePalette(18, 10);

	_spriteSlots.reset(false);
	_sequences.clear();
	_kernelMessages.clear();
	_vm->_palette->_paletteUsage.load(&_scenePaletteUsage);
	if (_vm->getGameID() != GType_RexNebular)
		setCamera(Common::Point(0, 0));

	int flags = SCENEFLAG_LOAD_SHADOW;
	if (_vm->_dithering)
		flags |= SCENEFLAG_DITHER;

	_sceneInfo = SceneInfo::init(_vm);
	_sceneInfo->load(_currentSceneId, _variant, Common::String(), flags,
		_depthSurface, _backgroundSurface);

	// Initialize palette animation for the scene
	initPaletteAnimation(_sceneInfo->_paletteCycles, false);

	// Copy over nodes
	_rails.load(_sceneInfo->_nodes, &_depthSurface, _sceneInfo->_depthStyle);

	// Load hotspots
	loadHotspots();

	// Load vocab
	loadVocab();

	// Load palette usage
	_vm->_palette->_paletteUsage.load(&_paletteUsageF);

	// Load interface
	flags = PALFLAG_RESERVED | ANIMFLAG_LOAD_BACKGROUND;
	if (_vm->_dithering)
		flags |= ANIMFLAG_DITHER;
	if (_vm->_textWindowStill)
		flags |= ANIMFLAG_LOAD_BACKGROUND_ONLY;

	_animationData = Animation::init(_vm, this);
	DepthSurface depthSurface;
	_animationData->load(_userInterface, depthSurface, prefix, flags, nullptr, nullptr);

	_vm->_palette->_paletteUsage.load(&_scenePaletteUsage);

	_bandsRange = _sceneInfo->_yBandsEnd - _sceneInfo->_yBandsStart;
	_scaleRange = _sceneInfo->_maxScale - _sceneInfo->_minScale;

	_spriteSlots.reset(false);
	_interfaceY = MADS_SCENE_HEIGHT;
	_spritesCount = _sprites.size();

	_userInterface.setup(_vm->_game->_screenObjects._inputMode);

	_vm->_game->_screenObjects._category = CAT_NONE;
	_vm->_events->showCursor();
}

void Scene::loadHotspots() {
	_hotspots.clear();

	Common::File f;
	if (f.open(Resources::formatName(RESPREFIX_RM, _currentSceneId, ".HH"))) {
		MadsPack madsPack(&f);
		bool isV2 = (_vm->getGameID() != GType_RexNebular);

		Common::SeekableReadStream *stream = madsPack.getItemStream(0);
		int count = stream->readUint16LE();
		delete stream;

		stream = madsPack.getItemStream(1);
		for (int i = 0; i < count; ++i)
			_hotspots.push_back(Hotspot(*stream, isV2));

		delete stream;
		f.close();
	}
}

void Scene::loadVocab() {
	// Add all the verbs to the active vocab list
	for (uint i = 0; i < _verbList.size(); ++i)
		addActiveVocab(_verbList[i]._id);

	// Load the vocabs for any object descriptions and custom actions
	for (uint objIndex = 0; objIndex < _vm->_game->_objects.size(); ++objIndex) {
		InventoryObject &io = _vm->_game->_objects[objIndex];
		addActiveVocab(io._descId);

		for (int vocabIndex = 0; vocabIndex <io._vocabCount; ++vocabIndex) {
			addActiveVocab(io._vocabList[vocabIndex]._vocabId);
		}
	}

	// Load scene hotspot list vocabs and verbs
	for (uint i = 0; i < _hotspots.size(); ++i) {
		addActiveVocab(_hotspots[i]._vocabId);
		if (_hotspots[i]._verbId)
			addActiveVocab(_hotspots[i]._verbId);
	}

	loadVocabStrings();
}

void Scene::loadVocabStrings() {
	_vocabStrings.clear();
	File f("*VOCAB.DAT");
	Common::String msg;

	for (;;) {
		char c = (char)f.readByte();
		if (f.eos()) break;

		if (c == '\0') {
			_vocabStrings.push_back(msg);
			msg = "";
		} else {
			msg += c;
		}
	}

	f.close();
}

uint32 Scene::getVocabStringsCount() const {
	return _vocabStrings.size();
}

void Scene::initPaletteAnimation(Common::Array<PaletteCycle> &palCycles, bool animFlag) {
	// Initialize the animation palette and ticks list
	_cycleTicks.clear();
	_paletteCycles.clear();

	for (uint i = 0; i < palCycles.size(); ++i) {
		_cycleTicks.push_back(_vm->_events->getFrameCounter());
		_paletteCycles.push_back(palCycles[i]);
	}

	// Save the initial starting palette
	Common::copy(&_vm->_palette->_mainPalette[0], &_vm->_palette->_mainPalette[PALETTE_SIZE],
		&_vm->_palette->_cyclingPalette[0]);

	// Calculate total
	_totalCycleColors = 0;
	for (uint i = 0; i < _paletteCycles.size(); ++i)
		_totalCycleColors += _paletteCycles[i]._colorCount;

	_cyclingThreshold = (_totalCycleColors > 16) ? 3 : 0;
	_cyclingActive = animFlag;
}

void Scene::animatePalette() {
	byte rgb[3];

	if (_cyclingActive) {
		Scene::_cyclingDelay++;
		if (_cyclingDelay >= _cyclingThreshold) {
			uint32 frameCounter = _vm->_events->getFrameCounter();
			bool changesFlag = false;
			for (uint16 idx = 0; idx < _paletteCycles.size(); idx++) {
				if (frameCounter >= (_cycleTicks[idx] + _paletteCycles[idx]._ticks)) {
					_cycleTicks[idx] = frameCounter;
					int count = _paletteCycles[idx]._colorCount;
					int first = _paletteCycles[idx]._firstColorIndex;
					int listIndex = _paletteCycles[idx]._firstListColor;
					changesFlag = true;

					if (count > 1) {
						// Make a copy of the last color
						byte *pSrc = &_vm->_palette->_cyclingPalette[first * 3];
						byte *pEnd = pSrc + count * 3;
						Common::copy(pEnd - 3, pEnd, &rgb[0]);

						// Shift the cycle palette forward one entry
						Common::copy_backward(pSrc, pEnd - 3, pEnd);

						// Move the saved color to the start of the cycle
						Common::copy(&rgb[0], &rgb[3], pSrc);

						if (++listIndex >= count)
							listIndex = 0;
					}

					_paletteCycles[idx]._firstListColor = listIndex;
				}
			}

			if (changesFlag) {
				int firstColor = _paletteCycles[0]._firstColorIndex;
				byte *pSrc = &_vm->_palette->_cyclingPalette[firstColor * 3];
				_vm->_palette->setPalette(pSrc, firstColor, _totalCycleColors);
			}

			_cyclingDelay = 0;
		}
	}
}

bool Scene::getDepthHighBits(const Common::Point &pt) {
	if (_sceneInfo->_depthStyle == 2) {
		return 0;
	} else {
		const byte *p = _depthSurface.getBasePtr(pt.x, pt.y);
		return (*p & 0x70) >> 4;
	}
}

void Scene::loop() {
	while (!_vm->shouldQuit() && !_reloadSceneFlag && (_nextSceneId == _currentSceneId)) {
		// Handle drawing a game frame
		doFrame();

		// Wait for the next frame
		_vm->_events->waitForNextFrame();

		if (_vm->_dialogs->_pendingDialog != DIALOG_NONE && !_vm->_game->_trigger
			&& _vm->_game->_player._stepEnabled)
			_reloadSceneFlag = true;

		if (_vm->_game->_winStatus)
			break;
	}
}

void Scene::doFrame() {
	Player &player = _vm->_game->_player;
	bool flag = false;

	if (_action._selectedAction || !player._stepEnabled) {
		_action.clear();
		_action._selectedAction = 0;
	}

	if (!_vm->_game->_trigger && !player._trigger) {
		// Refresh the dynamic hotspots if they've changed
		if (_dynamicHotspots._changed)
			_dynamicHotspots.refresh();

		// Check all on-screen visual objects
		_vm->_game->_screenObjects.check(player._stepEnabled && !player._needToWalk &&
				!_vm->_game->_fx);
	}

	_vm->_game->camUpdate();
	if (_action._selectedAction && player._stepEnabled && !player._needToWalk &&
			!_vm->_game->_trigger && !player._trigger) {
		_action.startAction();
		if (_action._activeAction._verbId == Nebular::VERB_LOOK_AT) {
			_action._activeAction._verbId = VERB_LOOK;
			_action._savedFields._command = false;
		}

		flag = true;
	}

	if (flag || (_vm->_game->_trigger && _vm->_game->_triggerMode == SEQUENCE_TRIGGER_PREPARE)) {
		doPreactions();
	}

	player.newWalk();
	if (!_vm->_game->_fx)
		_frameStartTime = _vm->_events->getFrameCounter();

	// Handle parser actions as well as game triggers
	if ((_action._inProgress && !player._moving && !player._needToWalk &&
			(player._facing == player._turnToFacing) && !_vm->_game->_trigger) ||
			(_vm->_game->_trigger && (_vm->_game->_triggerMode == SEQUENCE_TRIGGER_PARSER))) {
		doAction();
	}

	if (_currentSceneId != _nextSceneId) {
		_vm->_gameConv->stop();
		_freeAnimationFlag = true;
		// TODO: Handle Phantom/Dragonsphere animation list free
	} else {
		doSceneStep();
		checkKeyboard();

		if (_currentSceneId != _nextSceneId) {
			_vm->_gameConv->stop();
			_freeAnimationFlag = true;
			// TODO: Handle Phantom/Dragonsphere animation list free
		} else {
			// Handle conversation updates if one is active
			if (!_vm->_game->_trigger && _vm->_gameConv->active() &&
				!_vm->_game->_camX._activeFl && !_vm->_game->_camY._activeFl)
				_vm->_gameConv->update(false);

			// Update the player
			player.nextFrame();

			// Cursor update code
			updateCursor();

			if (!_vm->_game->_trigger) {
				// Handle any active sequences
				_sequences.tick();

				// Handle any active animation
				animations_tick();
			}

			// If the debugget flag is set, show the mouse position
			int mouseTextIndex = 0;
			if (_vm->_debugger->_showMousePos) {
				Common::Point pt = _vm->_events->mousePos();
				Common::String msg = Common::String::format("(%d,%d)", pt.x, pt.y);
				mouseTextIndex = _kernelMessages.add(Common::Point(5, 5),
					0x203, 0, 0, 1, msg);
			}

			if (!_vm->_game->_trigger) {
				if (_reloadSceneFlag || _currentSceneId != _nextSceneId)
					_kernelMessages.reset();
				_kernelMessages.update();
			}

			_userInterface._uiSlots.draw(!_vm->_game->_fx, _vm->_game->_fx);

			// Write any text needed by the interface
			if (_vm->_game->_fx)
				_userInterface.drawTextElements();

			// Draw any elements
			drawElements((ScreenTransition)_vm->_game->_fx, _vm->_game->_fx);

			// Handle message updates
			if (_vm->_game->_fx) {
				uint32 priorTime = _vm->_game->_priorFrameTimer;
				uint32 newTime = _vm->_events->getFrameCounter();
				_sequences.delay(priorTime, newTime);
				_kernelMessages.delay(priorTime, newTime);
			}

			if (_vm->_debugger->_showMousePos)
				// Mouse position display isn't persistent, so remove it
				_kernelMessages.remove(mouseTextIndex);

			// Original had a debugger check/call here to allow pausing after
			// drawing each frame. Not implemented under ScummVM
		}
	}

	if (_vm->_game->_fx)
		_cyclingActive = true;
	_vm->_game->_fx = kTransitionNone;

	// Handle freeing animation if necessary
	for (int i = 0; i < 10; i++) {
		if (_animation[i] && _animation[i]->freeFlag())
			_freeAnimationFlag = true;
	}
	if (_freeAnimationFlag)
		freeAnimation();
}

void  Scene::drawElements(ScreenTransition transitionType, bool surfaceFlag) {
	// Draw any sprite backgrounds
	_spriteSlots.drawBackground();

	// Set up dirty areas for any text display
	_textDisplay.setDirtyAreas();

	// Merge any identified dirty areas
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	if (_posAdjust != Common::Point(0, 0))
		warning("Adjust used %d %d", _posAdjust.x, _posAdjust.y);
	// Copy background for the dirty areas to the screen
	_dirtyAreas.copy(&_backgroundSurface, _vm->_screen, _posAdjust);

	// Handle dirty areas for foreground objects
	_spriteSlots.setDirtyAreas();
	_textDisplay.setDirtyAreas2();
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Draw sprites that have changed
	_spriteSlots.drawSprites(&_sceneSurface);

	// Draw text elements onto the view
	_textDisplay.draw(_vm->_screen);

	if (transitionType) {
		// Fading in the screen
		_vm->_screen->transition(transitionType, surfaceFlag);
		_vm->_sound->startQueuedCommands();
	} else {
		// Copy dirty areas to the screen
		_dirtyAreas.copyToScreen();
	}

	_spriteSlots.cleanUp();
	_textDisplay.cleanUp();
}

void Scene::doPreactions() {
	if (_vm->_game->_screenObjects._inputMode == kInputBuildingSentences ||
			_vm->_game->_screenObjects._inputMode == kInputLimitedSentences) {
		_vm->_game->_triggerSetupMode = SEQUENCE_TRIGGER_PREPARE;
		_action.checkAction();
		_sceneLogic->preActions();

		if (_vm->_game->_triggerMode == SEQUENCE_TRIGGER_PREPARE)
			_vm->_game->_trigger = 0;
	}
}

void Scene::doAction() {
	bool flag = false;

	// Don't allow the player to move if a conversation is active
	if (_vm->_gameConv->active()) {
		_vm->_game->_scene._action._savedFields._lookFlag = false;
		if (_vm->_gameConv->currentMode() == CONVMODE_2 || _vm->_gameConv->currentMode() == CONVMODE_3)
			_vm->_game->_player._stepEnabled = false;
	}

	_vm->_game->_triggerSetupMode = SEQUENCE_TRIGGER_PARSER;
	if ((_action._inProgress || _vm->_game->_trigger) && !_action._savedFields._commandError) {
		_sceneLogic->actions();
		flag = !_action._inProgress;
	}

	if (_vm->_gameConv->active() || _vm->_game->_screenObjects._inputMode == kInputConversation) {
		_action._inProgress = false;
	} else {
		if ((_action._inProgress || _vm->_game->_trigger) ||
			(!flag && _action._savedFields._commandError == flag)) {
			_vm->_game->_sectionHandler->sectionPtr2();
			flag = !_action._inProgress;
		}

		if ((_action._inProgress || _vm->_game->_trigger) &&
				(!flag || _action._savedFields._commandError == flag)) {
			_vm->_game->doObjectAction();
		}

		if (!_action._savedFields._lookFlag) {
			if (_action._inProgress) {
				_action._savedFields._commandError = true;
				_sceneLogic->postActions();
			}

			if (_action._inProgress) {
				_action._savedFields._commandError = true;
				_sceneLogic->unhandledAction();
			}

			if (_action._inProgress)
				_vm->_game->unhandledAction();
		}
	}

	_action._inProgress = false;
	if (_vm->_game->_triggerMode == SEQUENCE_TRIGGER_PARSER)
		_vm->_game->_trigger = 0;

	if (_vm->_gameConv->active() && (_vm->_gameConv->currentMode() == CONVMODE_1 ||
			_vm->_gameConv->currentMode() == CONVMODE_2))
		_vm->_gameConv->update(true);
}

void Scene::doSceneStep() {
	_vm->_game->_triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_sceneLogic->step();
	_vm->_game->_sectionHandler->step();
	_vm->_game->step();

	if (_vm->_game->_triggerMode == SEQUENCE_TRIGGER_DAEMON)
		_vm->_game->_trigger = 0;
}

void Scene::checkKeyboard() {
	EventsManager &events = *_vm->_events;

	if (events.isKeyPressed()) {
		Common::KeyState evt = events.getKey();
		_vm->_game->handleKeypress(evt);
	}

	if ((events._mouseStatus & 3) == 3 && _vm->_game->_player._stepEnabled) {
		_reloadSceneFlag = true;
		_vm->_dialogs->_pendingDialog = DIALOG_GAME_MENU;
		_action.clear();
		_action._selectedAction = 0;
	}
}

int Scene::loadAnimation(const Common::String &resName, int trigger) {
	// WORKAROUND: If there's already a previous active animation used by the
	// scene, then free it before we create the new one
	if ((_vm->getGameID() == GType_RexNebular) && _animation[0])
		freeAnimation(0);

	DepthSurface depthSurface;
	UserInterface interfaceSurface(_vm);

	for (int i = 0; i < 10; i++) {
		if (!_animation[i]) {
			_animation[i] = Animation::init(_vm, this);
			_animation[i]->load(interfaceSurface, depthSurface, resName,
				_vm->_dithering ? ANIMFLAG_DITHER : 0, nullptr, nullptr);
			_animation[i]->startAnimation(trigger);

			return i;
		}
	}

	error("Unable to find an available animation slot");

	return -1;
}

void Scene::updateCursor() {
	Player &player = _vm->_game->_player;

	CursorType cursorId = CURSOR_ARROW;
	if (_action._interAwaiting == AWAITING_COMMAND && !_vm->_events->_rightMousePressed &&
		_vm->_game->_screenObjects._category == CAT_HOTSPOT) {
		int idx = _vm->_game->_screenObjects._selectedObject -
			_userInterface._categoryIndexes[CAT_HOTSPOT - 1];
		assert(idx >= 0);

		if (idx >= (int)_hotspots.size()) {
			idx -= _hotspots.size();
			_vm->_events->_newCursorId = _dynamicHotspots[idx]._cursor;
		} else {
			idx = _hotspots.size() - idx - 1;
			_vm->_events->_newCursorId = _hotspots[idx]._cursor;
		}

		cursorId = _vm->_events->_newCursorId == CURSOR_NONE ?
		CURSOR_ARROW : _vm->_events->_newCursorId;
	}

	if (!player._stepEnabled)
		cursorId = CURSOR_WAIT;
	if (cursorId >= _vm->_events->_cursorSprites->getCount())
		cursorId = (CursorType)_vm->_events->_cursorSprites->getCount();
	_vm->_events->_newCursorId = cursorId;

	if (cursorId != _vm->_events->_cursorId) {
		_vm->_events->setCursor(cursorId);
	}
}

void Scene::freeCurrentScene() {
	if (_animationData) {
		delete _animationData;
		_animationData = nullptr;
	}

	for (int i = 0; i < 10; i++) {
		if (_animation[i]) {
			delete _animation[i];
			_animation[i] = nullptr;
		}
	}

	_vm->_palette->_paletteUsage.load(nullptr);
	_cyclingActive = false;
	_hotspots.clear();
	_backgroundSurface.free();
	_depthSurface.free();

	delete _sceneInfo;
	_sceneInfo = nullptr;
}

void Scene::removeSprites() {
	for (int idx = _sprites.size() - 1; idx >= _spritesCount; --idx)
		_sprites.remove(idx);
}

void Scene::changeVariant(int variant) {
	_variant = variant;
	_sceneInfo->loadCodes(_depthSurface, variant);
	_spriteSlots.fullRefresh();
}

void Scene::resetScene() {
	_vm->_game->clearQuotes();
	removeSprites();
	_spriteSlots.fullRefresh(true);
	_sequences.clear();
}

void Scene::freeAnimation() {
	for (int j = 0; j < 10; j++)
		freeAnimation(j);

	_freeAnimationFlag = false;
}

void Scene::freeAnimation(int idx) {
	if (_animation[idx]) {
		if (idx == 0) {
			Player &player = _vm->_game->_player;

			if (!_freeAnimationFlag) {
				_spriteSlots.fullRefresh(true);
				_sequences.scan();
			}

			// Refresh the player
			if (player._visible) {
				player._forceRefresh = true;
				player.update();
			}
		}

		// Remove any kernel messages in use by the animation
		for (uint i = 0; i < _animation[idx]->_messages.size(); ++i) {
			int msgIndex = _animation[idx]->_messages[i]._kernelMsgIndex;
			if (msgIndex >= 0)
				_kernelMessages.remove(msgIndex);
		}

		// Delete the animation
		delete _animation[idx];
		_animation[idx] = nullptr;
	}
}

void Scene::synchronize(Common::Serializer &s) {
	_action.synchronize(s);
	_rails.synchronize(s);
	_userInterface.synchronize(s);
	s.syncAsByte(_reloadSceneFlag);
	s.syncAsByte(_roomChanged);
	s.syncAsUint16LE(_nextSceneId);
	s.syncAsUint16LE(_priorSceneId);
	s.syncAsSint16LE(_variant);
	_dynamicHotspots.synchronize(s);
}

void Scene::setAnimFrame(int id, int val) {
	if ((id >= 0) && _animation[id])
		_animation[id]->setCurrentFrame(val);
}

int Scene::getAnimFrame(int id) {
	if ((id >= 0) && _animation[id])
		return _animation[id]->getCurrentFrame();

	return -1;
}

void Scene::setDynamicAnim(int id, int anim_id, int segment) {
	if (id >= 0 && id <= DYNAMIC_HOTSPOTS_SIZE && _animation[anim_id]) {
		_animation[anim_id]->_dynamicHotspotIndex = id;
		if (_dynamicHotspots[id]._animIndex < 0)
			_dynamicHotspots[id]._active = false;
		_dynamicHotspots[id]._animIndex = anim_id;

		// TODO: Anim segments

		// NOTE: Only remove the TODO below when _dynamicHotspotIndex
		// in the Animation class is actually used in the engine!

		warning("TODO: Scene::setDynamicAnim");
	}
}

void Scene::setCamera(Common::Point pos) {
	_posAdjust = pos;
	warning("setCamera: Incomplete function");
}

void Scene::drawToBackground(int spriteId, int frameId, Common::Point pos, int depth, int scale) {
	SpriteAsset &asset = *_sprites[spriteId];

	if (pos.x == -32000)
		pos.x = asset.getFramePos(frameId - 1).x;
	if (pos.y == -32000)
		pos.y = asset.getFramePos(frameId - 1).y;

	int slotIndex = _spriteSlots.add();
	SpriteSlot &slot = _spriteSlots[slotIndex];
	slot._spritesIndex = spriteId;
	slot._frameNumber = frameId;
	slot._seqIndex = 1;
	slot._position = pos;
	slot._depth = depth;
	slot._scale = scale;
	slot._flags = IMG_DELTA;
}

void Scene::deleteSequence(int idx) {
	if (_sequences[idx]._active && _sequences[idx]._dynamicHotspotIndex >= 0)
		_dynamicHotspots.remove(_sequences[idx]._dynamicHotspotIndex);

	_sequences[idx]._active = false;

	if (!_sequences[idx]._doneFlag) {
		warning("TODO: deleteSequence: Sequence %d not done", idx);
		// TODO: This is wrong, and crashes Phantom at scene 102 when the door is opened
		//doFrame();	// FIXME/CHECKME: Is this correct?
	} else {
		_sequences.remove(idx);
	}
}

void Scene::loadSpeech(int idx) {
	_vm->_audio->setDefaultSoundGroup();
	// NOTE: The original actually preloads the speech sample here, but the samples
	// are so small that it's not really worth it...

	// TODO: As the speech samples aren't cached anymore, _speechReady should be remove
	_speechReady = idx;
}

void Scene::playSpeech(int idx) {
	_vm->_audio->stop();
	_vm->_audio->playSound(idx - 1);
}

void Scene::sceneScale(int yFront, int maxScale, int yBack,  int minScale) {
	_sceneInfo->_yBandsEnd = yFront;
	_sceneInfo->_maxScale = maxScale;
	_sceneInfo->_yBandsStart = yBack;
	_sceneInfo->_minScale = minScale;

	_bandsRange = _sceneInfo->_yBandsEnd - _sceneInfo->_yBandsStart;
	_scaleRange = _sceneInfo->_maxScale - _sceneInfo->_minScale;
}

void Scene::animations_tick() {
	//warning("TODO: Implement _animations as an AnimationList and refactor (and check implementation)");
	for (int i = 0; i < 10; i++) {
		if (_animation[i])
			_animation[i]->update();
	}
}

} // End of namespace MADS
