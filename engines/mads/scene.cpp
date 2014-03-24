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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/scene.h"
#include "mads/compression.h"
#include "mads/mads.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

Scene::Scene(MADSEngine *vm): _vm(vm), _action(_vm), _depthSurface(vm),
		_dirtyAreas(_vm),  _dynamicHotspots(vm), _hotspots(vm),
		_kernelMessages(vm), _sequences(vm), _sprites(vm), _spriteSlots(vm), 
		_textDisplay(vm), _userInterface(vm) {
	_priorSceneId = 0;
	_nextSceneId = 0;
	_currentSceneId = 0;
	_sceneLogic = nullptr;
	_sceneInfo = nullptr;
	_animFlag = false;
	_animVal1 = 0;
	_depthStyle = 0;
	_roomChanged = false;
	_reloadSceneFlag = false;
	_destFacing = 0;
	_freeAnimationFlag = false;
	_animationData = nullptr;
	_activeAnimation = nullptr;
	_textSpacing = -1;
	_frameStartTime = 0;
	_layer = LAYER_GUI;
	_lookFlag = false;
	_highlightedHotspot = 0;

	_verbList.push_back(VerbInit(VERB_LOOK, 2, 0));
	_verbList.push_back(VerbInit(VERB_TAKE, 2, 0));
	_verbList.push_back(VerbInit(VERB_PUSH, 2, 0));
	_verbList.push_back(VerbInit(VERB_OPEN, 2, 0));
	_verbList.push_back(VerbInit(VERB_PUT, 1, -1));
	_verbList.push_back(VerbInit(VERB_TALKTO, 2, 0));
	_verbList.push_back(VerbInit(VERB_GIVE, 1, 2));
	_verbList.push_back(VerbInit(VERB_PULL, 2, 0));
	_verbList.push_back(VerbInit(VERB_CLOSE, 2, 0));
	_verbList.push_back(VerbInit(VERB_THROW, 1, 2));
}

Scene::~Scene() {
	delete _sceneLogic;
	delete _sceneInfo;
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
	_talkFont = "*FONTCONV.FF";
	_textSpacing  = -1;
}

void Scene::loadSceneLogic() {
	delete _sceneLogic;

	switch (_vm->getGameID()) {
	case GType_RexNebular:
		_sceneLogic = Nebular::SceneFactory::createScene(_vm);
		break;
	default:
		error("Unknown game");
	}
}

void Scene::loadScene(int sceneId, const Common::String &prefix, bool palFlag) {
	// Store the previously active scene number and set the new one
	_priorSceneId = _currentSceneId;
	_currentSceneId = sceneId;

	_v1 = 0;
	if (palFlag)
		_vm->_palette->resetGamePalette(18, 10);

	_spriteSlots.reset(false);
	_sequences.clear();
	_kernelMessages.clear();

	// TODO: palletteUsage reset?  setPalette(_nullPalette);
	_sceneInfo = SceneInfo::init(_vm);
	_sceneInfo->load(_currentSceneId, _v1, Common::String(), _vm->_game->_v2 ? 17 : 16,
		_depthSurface, _backgroundSurface);

	// Initialise palette animation for the scene
	initPaletteAnimation(_sceneInfo->_palAnimData, false);

	// Copy over nodes
	_nodes.clear();
	for (uint i = 0; i < _sceneInfo->_nodes.size(); ++i)
		_nodes.push_back(_sceneInfo->_nodes[i]);

	// Load hotspots
	loadHotspots();

	// Load vocab
	loadVocab();

	// Load palette usage
	_vm->_palette->_paletteUsage.load(1, 0xF);

	// Load interface
	int flags = _vm->_game->_v2 ? 0x4101 : 0x4100;
	if (!_vm->_textWindowStill)
		flags |= 0x200;

	_animationData = Animation::init(_vm, this);
	MSurface depthSurface;
	_animationData->load(_userInterface, depthSurface, prefix, flags, nullptr, nullptr);
	
	_vm->_palette->_paletteUsage.load(0);

	_bandsRange = _sceneInfo->_yBandsEnd - _sceneInfo->_yBandsStart;
	_scaleRange = _sceneInfo->_maxScale - _sceneInfo->_minScale;

	_spriteSlots.reset(false);
	_interfaceY = MADS_SCENE_HEIGHT;
	_spritesCount = _sprites.size();

	_userInterface.setup(_vm->_game->_screenObjects._v832EC);

	warning("TODO: showMouse");

	warning("TODO: inventory_anim_allocate");
}

void Scene::loadHotspots() {
	File f(Resources::formatName(RESPREFIX_RM, _currentSceneId, ".HH"));
	MadsPack madsPack(&f);

	Common::SeekableReadStream *stream = madsPack.getItemStream(0);
	int count = stream->readUint16LE();
	delete stream;

	stream = madsPack.getItemStream(1);
	_hotspots.clear();
	for (int i = 0; i < count; ++i)
		_hotspots.push_back(Hotspot(*stream));

	delete stream;
	f.close();
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

void Scene::initPaletteAnimation(Common::Array<RGB4> &animData, bool animFlag) {
	// Initialise the animation palette and ticks list
	_animTicksList.clear();
	_animPalData.clear();

	for (uint i = 0; i < animData.size(); ++i) {
		_animTicksList.push_back(_vm->_events->getFrameCounter());
		_animPalData.push_back(animData[i]);
	}

	// Save the initial starting palette
	Common::copy(&_vm->_palette->_mainPalette[0], &_vm->_palette->_mainPalette[PALETTE_SIZE],
		&_vm->_palette->_savedPalette[0]);

	// Calculate total
	_animCount = 0;
	for (uint i = 0; i < _animPalData.size(); ++i)
		_animCount += _animPalData[i].r;

	_animVal1 = (_animCount > 16) ? 3 : 0;
	_animFlag = animFlag;
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
	_nextSceneId = _currentSceneId;

	while (!_vm->shouldQuit() && !_reloadSceneFlag && _nextSceneId == _currentSceneId) {
		// Handle drawing a game frame
		doFrame();

		// Reset mouse flags
		_vm->_events->resetMouseFlags();

		// TODO: Verify correctness of frame wait
		_vm->_events->waitForNextFrame();

		if (_vm->_dialogs->_pendingDialog != DIALOG_NONE && !_vm->_game->_abortTimers
			&& _vm->_game->_player._stepEnabled)
			_reloadSceneFlag = true;
	}
}

void Scene::doFrame() {
	Player &player = _vm->_game->_player;
	bool flag = false;

	if (_action._selectedAction || !player._stepEnabled) {
		_action.clear();
		_action._selectedAction = 0;
	}

	if (!_vm->_game->_abortTimers && !player._unk3) {
		// Refresh the dynamic hotspots if they've changed
		if (_dynamicHotspots._changed)
			_dynamicHotspots.refresh();

		// Check all on-screen visual objects
		_vm->_game->_screenObjects.check(player._stepEnabled && !_action._startWalkFlag &&
				!_vm->_game->_abortTimers2);
	}

	if (_action._selectedAction && player._stepEnabled && !_action._startWalkFlag &&
			!_vm->_game->_abortTimers && !player._unk3) {
		_action.startAction();
		if (_action._activeAction._verbId == Nebular::NOUN_LOOK_AT) {
			_action._activeAction._verbId = VERB_LOOK;
			_action._savedFields._selectedRow = false;
		}

		flag = true;
	}

	if (flag || (_vm->_game->_abortTimers && _vm->_game->_abortTimersMode == ABORTMODE_2)) {
		doPreactions();
	}

	checkStartWalk();
	if (!_vm->_game->_abortTimers2)
		_frameStartTime = _vm->_events->getFrameCounter();

	if ((_action._inProgress && !player._moving && !_action._startWalkFlag &&
			player._newDirection == player._direction) ||
			(_vm->_game->_abortTimers && _vm->_game->_abortTimersMode == ABORTMODE_0)) {
		doAction();
	}

	if (_currentSceneId != _nextSceneId) {
		_freeAnimationFlag = true;
	} else {
		doSceneStep();
		checkKeyboard();

		if (_currentSceneId != _nextSceneId) {
			_freeAnimationFlag = true;
		} else {
			player.nextFrame();

			// Cursor update code
			updateCursor();

			if (!_vm->_game->_abortTimers) {
				// Handle any active sequences
				_sequences.tick();

				// Handle any active animation
				if (_activeAnimation)
					_activeAnimation->update();
			}

			// If the debugget flag is set, show the mouse position
			int mouseTextIndex = 0;
			if (_vm->_debugger->_showMousePos) {
				Common::Point pt = _vm->_events->mousePos();
				Common::String msg = Common::String::format("(%d,%d)", pt.x, pt.y);
				mouseTextIndex = _kernelMessages.add(Common::Point(5, 5), 
					0x203, 0, 0, 1, msg);
			}

			if (!_vm->_game->_abortTimers) {
				if (_reloadSceneFlag || _currentSceneId != _nextSceneId)
					_kernelMessages.reset();
				_kernelMessages.update();
			}

			_userInterface._uiSlots.draw(_vm->_game->_abortTimers2 == 0,
				_vm->_game->_abortTimers2 != 0);

			// Write any text needed by the interface
			if (_vm->_game->_abortTimers2)
				_userInterface.drawTextElements();

			// Draw any elements
			drawElements((ScreenTransition)_vm->_game->_abortTimers2, _vm->_game->_abortTimers2);

			// Handle message updates
			if (_vm->_game->_abortTimers2) {
				uint32 priorTime = _vm->_game->_priorFrameTimer;
				uint32 newTime = _vm->_events->getFrameCounter();
				_sequences.delay(newTime, priorTime);
				_kernelMessages.delay(newTime, priorTime);
			}

			if (_vm->_debugger->_showMousePos)
				// Mouse position display isn't persistent, so remove it
				_kernelMessages.remove(mouseTextIndex);


			warning("TODO: sub_1DA3E");
		}
	}

	if (_vm->_game->_abortTimers2)
		_animFlag = true;
	_vm->_game->_abortTimers2 = 0;

	if (_freeAnimationFlag) {
		_activeAnimation->free();
		_activeAnimation = nullptr;
	}
}

void Scene::drawElements(ScreenTransition transitionType, bool surfaceFlag) {
	// Draw any sprites
	_spriteSlots.drawBackground();

	// Set up dirty areas for any text display
	_textDisplay.setDirtyAreas();

	// Merge any identified dirty areas
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Copy background for the dirty areas to the screen
	_dirtyAreas.copy(&_backgroundSurface, &_vm->_screen, _posAdjust);

	// Handle dirty areas for foreground objects
	_spriteSlots.setDirtyAreas();
	_textDisplay.setDirtyAreas2();
	_dirtyAreas.merge(1, DIRTY_AREAS_SIZE);

	// Draw foreground sprites
	_spriteSlots.drawForeground(&_vm->_screen);

	// Draw text elements onto the view
	_textDisplay.draw(&_vm->_screen);

	// 
	_vm->_screen.setPointer(&_vm->_screen);
	_userInterface.setBounds(Common::Rect(_vm->_screen._offset.x, _vm->_screen._offset.y,
		_vm->_screen._offset.x + _vm->_screen.w, _vm->_screen._offset.y + _vm->_screen.h));

	if (transitionType) {
		// Fading in the screen
		_vm->_screen.transition(transitionType, surfaceFlag);
		_vm->_sound->startQueuedCommands();
	} else {
		// Copy dirty areas to the screen
		_dirtyAreas.copyToScreen(_vm->_screen._offset);
	}

	warning("TODO: sub_115A2");
	_spriteSlots.cleanUp();
	_textDisplay.cleanUp();
}

void Scene::doPreactions() {
	if (_vm->_game->_screenObjects._v832EC == 0 || _vm->_game->_screenObjects._v832EC == 2) {
		_vm->_game->_abortTimersMode2 = ABORTMODE_2;
		_action.checkAction();
		_sceneLogic->preActions();

		if (_vm->_game->_abortTimersMode == ABORTMODE_2)
			_vm->_game->_abortTimers = 0;
	}
}

void Scene::doAction() {
	int flag = 0;

	_vm->_game->_abortTimersMode2 = ABORTMODE_0;
	if ((_action._inProgress || _vm->_game->_abortTimers) && !_action._v8453A) {
		_sceneLogic->actions();
		_action._inProgress = true;
		flag = -1;
	}

	if (_vm->_game->_screenObjects._v832EC == 1) {
		_action._inProgress = false;
	} else {
		if ((_action._inProgress || _vm->_game->_abortTimers) ||
				(!flag && _action._v8453A == flag)) {
			_vm->_game->_sectionHandler->sectionPtr2();
			_action._inProgress = true;
			flag = -1;
		}

		if ((_action._inProgress || _vm->_game->_abortTimers) && (!flag || _action._v8453A == flag)) {
			_vm->_game->doObjectAction();
		}

		if (!_action._savedFields._lookFlag) {
			if (!_action._inProgress) {
				_action._v8453A = -1;
				_sceneLogic->postActions();
			}

			if (!_action._inProgress) {
				_action._v8453A = -1;
				warning("TODO: PtrUnk4");
			}

			if (_action._inProgress)
				warning("TODO: sub_1D9DE");
		}
	}

	_action._inProgress = false;
	if (_vm->_game->_abortTimersMode == ABORTMODE_0)
		_vm->_game->_abortTimers = 0;
}

void Scene::checkStartWalk() {
	if (_action._startWalkFlag && _action._walkFlag) {
		_vm->_game->_player.setDest(_destPos, _destFacing);
		_action._startWalkFlag = false;
	}
}

void Scene::doSceneStep() {
	_vm->_game->_abortTimersMode2 = ABORTMODE_1;
	_sceneLogic->step();
	_vm->_game->_sectionHandler->step();

	_vm->_game->_player.step();
	_vm->_game->_player._unk3 = 0;

	if (_vm->_game->_abortTimersMode == ABORTMODE_1)
		_vm->_game->_abortTimers = 0;
}

void Scene::checkKeyboard() {
	warning("TODO: Scene::checkKeyboard");
}

void Scene::loadAnimation(const Common::String &resName, int abortTimers) {
	assert(_activeAnimation == nullptr);
	MSurface depthSurface;
	UserInterface interfaceSurface(_vm);

	_activeAnimation = Animation::init(_vm, this);
	_activeAnimation->load(interfaceSurface, depthSurface, resName, 
		_vm->_game->_v2 ? 1 : 0, nullptr, nullptr);
	_activeAnimation->startAnimation(abortTimers);
}

void Scene::updateCursor() {
	Player &player = _vm->_game->_player;

	CursorType cursorId = CURSOR_ARROW;
	if (_action._v83338 == 1 && !_vm->_game->_screenObjects._v7FECA &&
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

void Scene::free() {
	warning("TODO: Scene::free");
}

void Scene::changeDepthSurface(int arg1) {
	_v1 = arg1;
	_sceneInfo->loadCodes(_depthSurface, nullptr);
	_spriteSlots.fullRefresh();
}

void Scene::resetScene() {
	_vm->_game->clearQuotes();
	_spriteSlots.fullRefresh(true);
	_sequences.clear();
}

} // End of namespace MADS
