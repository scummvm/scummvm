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

#include "backends/modular-backend.h"
#include "backends/graphics/graphics.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "common/debug-channels.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/screen.h"
#include "graphics/opengl/system_headers.h"
#include "gui/message.h"
#include "image/png.h"

#include "twp/twp.h"
#include "twp/actions.h"
#include "twp/callback.h"
#include "twp/console.h"
#include "twp/debugtools.h"
#include "twp/detection.h"
#include "twp/enginedialogtarget.h"
#include "twp/hud.h"
#include "twp/lighting.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/savegame.h"
#include "twp/scenegraph.h"
#include "twp/shaders.h"
#include "twp/squtil.h"
#include "twp/task.h"
#include "twp/thread.h"
#include "twp/tsv.h"
#include "twp/vm.h"
#include "twp/walkboxnode.h"

namespace Twp {

#define TWP_SAVEGAME_VERSION 1

TwpEngine *g_twp;

struct GetNoun {
	GetNoun(int verbId, Common::SharedPtr<Object> &obj) : _verbId(verbId), _noun(obj) {
		_noun = nullptr;
	}

	bool operator()(Common::SharedPtr<Object> obj) {
		if (((_verbId == VERB_TALKTO) || (_verbId == VERB_WALKTO) || !g_twp->_resManager->isActor(obj->getId())) && (obj->_node->getZSort() <= _zOrder)) {
			_noun = obj;
			_zOrder = obj->_node->getZSort();
		}
		return false;
	}

public:
	Common::SharedPtr<Object> &_noun;
	int _zOrder = INT_MAX;
	const int _verbId;
};

struct GetUseNoun2 {
	explicit GetUseNoun2(Common::SharedPtr<Object> &obj) : _noun2(obj) {
		_noun2 = nullptr;
	}

	bool operator()(Common::SharedPtr<Object> obj) {
		if (obj->_node->getZSort() <= _zOrder) {
			if ((obj != g_twp->_actor) && (g_twp->_noun2 != obj)) {
				_noun2 = obj;
				_zOrder = obj->_node->getZSort();
			}
		}
		return false;
	}

public:
	Common::SharedPtr<Object> &_noun2;
	int _zOrder = INT_MAX;
};

struct GetGiveableNoun2 {
	explicit GetGiveableNoun2(Common::SharedPtr<Object> &obj) : _noun2(obj) {
		_noun2 = nullptr;
	}

	bool operator()(Common::SharedPtr<Object> obj) {
		if ((obj != g_twp->_actor) && (obj->getFlags() & GIVEABLE) && (g_twp->_noun2 != obj)) {
			_noun2 = obj;
			return true;
		}
		return false;
	}

public:
	Common::SharedPtr<Object> &_noun2;
};

struct InInventory {
	explicit InInventory(Common::SharedPtr<Object> &obj) : _obj(obj) { _obj = nullptr; }
	bool operator()(Common::SharedPtr<Object> obj) {
		if (obj->inInventory()) {
			_obj = obj;
			return true;
		}
		return false;
	}

public:
	Common::SharedPtr<Object> &_obj;
};

typedef struct ObjPos {
	ObjPos(Common::SharedPtr<Object> obj, const Math::Vector2d &pos, float distance, int index) : _obj(obj), _pos(pos), _distance(distance), _index(index) {}
	float _distance = 0.f;
	Math::Vector2d _pos;
	Common::SharedPtr<Object> _obj;
	int _index;
} ObjPos;

struct ObjPosComparer {
	bool operator()(const ObjPos &x, const ObjPos &y) const {
		return x._distance < y._distance;
	}
};

struct GetByZOrder {
	explicit GetByZOrder(Common::SharedPtr<Object> &result) : _result(result) {
		result = nullptr;
	}

	bool operator()(Common::SharedPtr<Object> obj) {
		if (obj->_node->getZSort() <= _zOrder) {
			if (!g_twp->_resManager->isActor(obj->getId()) || !obj->_key.empty()) {
				_result = obj;
				_zOrder = obj->_node->getZSort();
			}
		}
		return false;
	}

public:
	Common::SharedPtr<Object> &_result;

private:
	int _zOrder = INT_MAX;
};

struct DefineObjectParams {
	HSQUIRRELVM v;
	bool pseudo;
	Common::SharedPtr<Room> room;
};

TwpEngine::TwpEngine(OSystem *syst, const TwpGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _randomSource("Twp"),
	  _useFlag(UseFlag::ufNone) {
	g_twp = this;
	_dialog.reset(new Dialog());
	_dialog->_tgt.reset(new EngineDialogTarget());
	sq_resetobject(&_defaultObj);

	_audio.reset(new AudioSystem());
	_scene.reset(new Scene());
	_screenScene.reset(new Scene());
	_walkboxNode.reset(new WalkboxNode());
	_pathNode.reset(new PathNode());
	_hotspotMarker.reset(new HotspotMarkerNode());
	_lightingNode.reset(new LightingNode());
	_noOverride.reset(new NoOverrideNode());
	_hud.reset(new Hud());
	_pack.reset(new GGPackSet());
	_saveGameManager.reset(new SaveGameManager());

	_screenScene->setName("Screen");
	_scene->addChild(_walkboxNode.get());
	_screenScene->addChild(_pathNode.get());
	_screenScene->addChild(_lightingNode.get());
	_screenScene->addChild(_hotspotMarker.get());
	_screenScene->addChild(&_inputState);
	_screenScene->addChild(&_sentence);
	_screenScene->addChild(_dialog.get());
	_screenScene->addChild(&_uiInv);
	_screenScene->addChild(&_actorSwitcher);
	_screenScene->addChild(_noOverride.get());
}

TwpEngine::~TwpEngine() {
	_mixer->stopAll();
}

Math::Vector2d TwpEngine::winToScreen(const Math::Vector2d &pos) const {
	return Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
}

Math::Vector2d TwpEngine::screenToWin(const Math::Vector2d &pos) const {
	return Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
}

Math::Vector2d TwpEngine::roomToScreen(const Math::Vector2d &pos) const {
	Math::Vector2d screenSize = _room->getScreenSize();
	return Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) * (pos - _gfx.cameraPos()) / screenSize;
}

Math::Vector2d TwpEngine::screenToRoom(const Math::Vector2d &pos) const {
	Math::Vector2d screenSize = _room->getScreenSize();
	return (pos * screenSize) / Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) + _gfx.cameraPos();
}

uint32 TwpEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String TwpEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

bool TwpEngine::clickedAtHandled(const Math::Vector2d &roomPos) {
	bool result = false;
	int x = roomPos.getX();
	int y = roomPos.getY();
	if (sqrawexists(_room->_table, "clickedAt")) {
		debugC(kDebugGame, "clickedAt %d, %d", x, y);
		sqcallfunc(result, _room->_table, "clickedAt", x, y);
	}
	if (!result) {
		if (_actor && sqrawexists(_actor->_table, "clickedAt")) {
			sqcallfunc(result, _actor->_table, "clickedAt", x, y);
		}
	}
	return result;
}

bool TwpEngine::preWalk(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2) {
	bool result = false;
	HSQOBJECT n2Table;
	Common::String n2Name;
	if (noun2) {
		n2Table = noun2->_table;
		n2Name = Common::String::format("%s(%s)", noun2->_name.c_str(), noun2->_key.c_str());
	} else {
		sq_resetobject(&n2Table);
	}
	if (sqrawexists(actor->_table, "actorPreWalk")) {
		debugC(kDebugGame, "actorPreWalk %d n1=%s(%s) n2=%s", verbId.id, noun1->_name.c_str(), noun1->_key.c_str(), n2Name.c_str());
		sqcallfunc(result, actor->_table, "actorPreWalk", verbId.id, noun1->_table, n2Table);
	}
	if (!result) {
		Common::String funcName = _resManager->isActor(noun1->getId()) ? "actorPreWalk" : "objectPreWalk";
		if (sqrawexists(noun1->_table, funcName)) {
			sqcallfunc(result, noun1->_table, funcName.c_str(), verbId.id, noun1->_table, n2Table);
			debugC(kDebugGame, "%s %d n1=%s(%s) n2=%s -> %s", funcName.c_str(), verbId.id, noun1->_name.c_str(), noun1->_key.c_str(), n2Name.c_str(), result ? "yes" : "no");
		}
	}
	return result;
}

static bool verbNoWalkTo(VerbId verbId, Common::SharedPtr<Object> noun1) {
	if (verbId.id == VERB_LOOKAT)
		return (noun1->getFlags() & FAR_LOOK) != 0;
	return false;
}

// Called to execute a sentence and, if needed, start the actor walking.
// If `actor` is `null` then the selectedActor is assumed.
bool TwpEngine::execSentence(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2) {
	Common::String name = !actor ? "currentActor" : actor->_key;
	Common::String noun1name = !noun1 ? "null" : noun1->_key;
	Common::String noun2name = !noun2 ? "null" : noun2->_key;
	debugC(kDebugGame, "exec(%s,%d,%s,%s)", name.c_str(), verbId.id, noun1name.c_str(), noun2name.c_str());
	actor = !actor ? _actor : actor;
	if ((verbId.id <= 0) || (verbId.id > MAX_VERBS) || (!noun1) || (!actor))
		return false;

	debugC(kDebugGame, "noun1.inInventory: %s and noun1.touchable: %s nowalk: %s", noun1->inInventory() ? "YES" : "NO", noun1->isTouchable() ? "YES" : "NO", verbNoWalkTo(verbId, noun1) ? "YES" : "NO");

	// test if object became untouchable
	if (!noun1->inInventory() && !noun1->isTouchable())
		return false;
	if (noun2 && (!noun2->inInventory()) && (!noun2->isTouchable()))
		return false;

	if (noun1->inInventory()) {
		if (!noun2 || noun2->inInventory()) {
			callVerb(actor, verbId, noun1, noun2);
			return true;
		}
	}

	if (preWalk(actor, verbId, noun1, noun2))
		return true;

	if (verbNoWalkTo(verbId, noun1)) {
		if (!noun1->inInventory()) { // TODO: test if verb.flags != VERB_INSTANT
			Object::turn(actor, noun1);
			callVerb(actor, verbId, noun1, noun2);
			return true;
		}
	}

	actor->_exec.verb = verbId;
	actor->_exec.noun1 = noun1;
	actor->_exec.noun2 = noun2;
	actor->_exec.enabled = true;
	Object::walk(actor, noun1->inInventory() ? noun2 : noun1);
	return true;
}

void TwpEngine::flashSelectableActor(int flash) {
	_actorSwitcher.setFlash(flash);
}

void TwpEngine::walkFast(bool state) {
	if (_walkFastState != state) {
		debugC(kDebugGame, "walk fast: %s", state ? "yes" : "no");
		_walkFastState = state;
		if (_actor)
			sqcall(_actor->_table, "run", state);
	}
}

void TwpEngine::clickedAt(const Math::Vector2d &scrPos) {
	if (_room && !_actorSwitcher.isMouseOver()) {
		Math::Vector2d roomPos = screenToRoom(scrPos);
		Common::SharedPtr<Object> obj = objAt(roomPos);

		if (!_hud->_active && _cursor.doubleClick) {
			walkFast(true);
			_holdToMove = true;
		}

		if (_cursor.isLeftDown()) {
			// button left: execute selected verb
			bool handled = clickedAtHandled(roomPos);
			if (!handled && obj && (!_hud->_active || _uiInv.isOver())) {
				Verb vb = verb();
				sqcall("onVerbClick");
				handled = execSentence(nullptr, vb.id, _noun1, _noun2);
			}
			if (!handled && !_hud->_active && _actor && (!_hud->_active || _uiInv.isOver())) {
				// Just clicking on the ground
				cancelSentence(_actor);
				if (_actor->_room == _room)
					Object::walk(_actor, roomPos);
				_hud->selectVerb(_hud->actorSlot(_actor)->verbSlots[0]._verb);
				_holdToMove = true;
			}

		} else if (_cursor.isRightDown() && (!_hud->_active || _uiInv.isOver())) {
			// button right: execute default verb
			if (obj) {
				VerbId verb;
				verb.id = obj->defaultVerbId();
				execSentence(nullptr, verb, _noun1, _noun2);
			}
		}
	}
}

Verb TwpEngine::verb() {
	Verb result = _hud->_verb;
	if (result.id.id == VERB_WALKTO && _noun1 && _noun1->inInventory())
		result = *_hud->actorSlot(_actor)->getVerb(_noun1->defaultVerbId());
	else if (_actor) {
		result = *_hud->actorSlot(_actor)->getVerb(_hud->_verb.id.id);
	}
	return result;
}

Common::String TwpEngine::cursorText() {
	Common::String result;
	if (_dialog->getState() != DialogState::None || !_inputState.getInputActive())
		return result;

	if (_hud->_active && !_uiInv.isOver()) {
		return _hud->_verb.id.id > 1 ? _textDb->getText(verb().text) : "";
	}

	// give can be used only on inventory and talkto to talkable objects (actors)
	result = !_noun1 || (_hud->_verb.id.id == VERB_GIVE && !_noun1->inInventory()) || (_hud->_verb.id.id == VERB_TALKTO && !(_noun1->getFlags() & TALKABLE)) ? "" : _textDb->getText(_noun1->getName());

	// add verb if not walk to or if noun1 is present
	if ((_hud->_verb.id.id > 1) || (result.size() > 0)) {
		// if inventory, use default verb instead of walkto
		Common::String verbText = verb().text;
		result = result.size() > 0 ? Common::String::format("%s %s", _textDb->getText(verbText).c_str(), result.c_str()) : _textDb->getText(verbText);
		if (_useFlag == UseFlag::ufUseWith)
			result += " " + _textDb->getText(10000);
		else if (_useFlag == UseFlag::ufUseOn)
			result += " " + _textDb->getText(10001);
		else if (_useFlag == UseFlag::ufUseIn)
			result += " " + _textDb->getText(10002);
		else if (_useFlag == UseFlag::ufGiveTo)
			result += " " + _textDb->getText(10003);
		if (_noun2)
			result += " " + _textDb->getText(_noun2->getName());
	}
	return result;
}

template<typename TFunc>
void objsAt(Math::Vector2d pos, TFunc func) {
	if (g_twp->_uiInv.getObject() && g_twp->_room->_fullscreen == FULLSCREENROOM)
		func(g_twp->_uiInv.getObject());
	for (size_t i = 0; i < g_twp->_room->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = g_twp->_room->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj = layer->_objects[j];
			if ((obj != g_twp->_actor) && (obj->isTouchable() || obj->inInventory()) && (obj->_node->isVisible()) && (obj->_objType == otNone) && (obj->contains(pos)))
				if (func(obj))
					return;
		}
	}
}

Common::SharedPtr<Object> inventoryAt(Math::Vector2d pos) {
	Common::SharedPtr<Object> result;
	objsAt(Common::move(pos), InInventory(result));
	return result;
}

static void selectSlotActor(int id) {
	if (g_twp->_actorSwitcher._mode == asOn) {
		for (size_t i = 0; i < g_twp->_actors.size(); i++) {
			if (g_twp->_actors[i]->getId() == id) {
				g_twp->setActor(g_twp->_actors[i]);
				break;
			}
		}
	}
}

static void showOptions(int id) {
	if (g_twp && !g_twp->isPaused())
		g_twp->openMainMenuDialog();
}

ActorSwitcherSlot TwpEngine::actorSwitcherSlot(ActorSlot *slot) {
	return ActorSwitcherSlot(slot->actor->getIcon(),
							 slot->verbUiColors.inventoryBackground,
							 slot->verbUiColors.inventoryFrame, selectSlotActor, slot->actor->getId());
}

Common::Array<ActorSwitcherSlot> TwpEngine::actorSwitcherSlots() {
	Common::Array<ActorSwitcherSlot> result;
	if (_actor) {
		// add current actor first
		{
			ActorSlot *slot = _hud->actorSlot(_actor);
			result.push_back(actorSwitcherSlot(slot));
		}

		// then other selectable actors
		for (int i = 0; i < NUMACTORS; i++) {
			ActorSlot *slot = &_hud->_actorSlots[i];
			if (slot->selectable && slot->actor && (slot->actor != _actor) && (slot->actor->_room->_name != "Void"))
				result.push_back(actorSwitcherSlot(slot));
		}

		// add gear icon
		result.push_back(ActorSwitcherSlot("icon_gear", Color(0.f, 0.f, 0.f), Color(0.8f, 0.8f, 0.8f), showOptions));
	}
	return result;
}

void TwpEngine::update(float elapsed) {
	const uint32 startUpdateTime = _system->getMillis();
	_time += elapsed;
	_frameCounter++;

	_audio->update(elapsed);
	_noOverride->update(elapsed);
	if (_talking)
		_talking->update(elapsed);
	if (_moveCursorTo)
		_moveCursorTo->update(elapsed);

	// update mouse pos
	Math::Vector2d scrPos = winToScreen(_cursor.pos);
	_inputState.setVisible(_inputState.getShowCursor() || _dialog->getState() == WaitingForChoice);
	_inputState.setPos(scrPos);
	_sentence.setPos(scrPos);
	_dialog->setMousePos(scrPos);

	if (_room) {
		// update nouns and useFlag
		Math::Vector2d roomPos = screenToRoom(scrPos);
		if (_room->_fullscreen == FULLSCREENROOM) {
			if ((_hud->_verb.id.id == VERB_USE) && (_useFlag != UseFlag::ufNone)) {
				objsAt(roomPos, GetUseNoun2(_noun2));
			} else if (_hud->_verb.id.id == VERB_GIVE) {
				if (_useFlag != UseFlag::ufGiveTo) {
					_noun1 = inventoryAt(roomPos);
					_useFlag = UseFlag::ufNone;
					_noun2 = nullptr;
				} else {
					objsAt(roomPos, GetGiveableNoun2(_noun2));
					if (_noun2)
						debugC(kDebugGame, "Give '%s' to '%s'", _noun1->_key.c_str(), _noun2->_key.c_str());
				}
			} else {
				objsAt(roomPos, GetNoun(_hud->_verb.id.id, _noun1));
				_useFlag = UseFlag::ufNone;
				_noun2 = nullptr;
			}

			// update cursor shape
			// if cursor is in the margin of the screen and if camera can move again
			// then show a left arrow or right arrow
			Math::Vector2d screenSize = _room->getScreenSize();
			if ((scrPos.getX() < SCREEN_MARGIN) && (_gfx.cameraPos().getX() >= 1.f)) {
				_inputState.setCursorShape(CursorShape::Left);
			} else if ((scrPos.getX() > (SCREEN_WIDTH - SCREEN_MARGIN)) && _gfx.cameraPos().getX() < (_room->_roomSize.getX() - screenSize.getX())) {
				_inputState.setCursorShape(CursorShape::Right);
			} else if (_room->_fullscreen == FULLSCREENROOM && _noun1) {
				// if the object is a door, it has a flag indicating its direction: left, right, front, back
				int flags = _noun1->getFlags();
				if ((flags & DOOR_LEFT) == DOOR_LEFT)
					_inputState.setCursorShape(CursorShape::Left);
				else if ((flags & DOOR_RIGHT) == DOOR_RIGHT)
					_inputState.setCursorShape(CursorShape::Right);
				else if ((flags & DOOR_FRONT) == DOOR_FRONT)
					_inputState.setCursorShape(CursorShape::Front);
				else if ((flags & DOOR_BACK) == DOOR_BACK)
					_inputState.setCursorShape(CursorShape::Back);
				else
					_inputState.setCursorShape(CursorShape::Normal);
			} else {
				_inputState.setCursorShape(CursorShape::Normal);
			}

			_inputState.setHotspot(_noun1 != nullptr);
			bool hudVisible = _inputState.getInputActive() && _inputState.getInputVerbsActive() && _dialog->getState() == DialogState::None && !_cutscene;
			_hud->setVisible(hudVisible);
			_sentence.setVisible(_hud->isVisible());
			_uiInv.setVisible(hudVisible);
			_actorSwitcher.setVisible((_dialog->getState() == DialogState::None) && !_cutscene);
			// Common::String cursortxt = Common::String::format("%s (%d, %d) - (%d, %d)", cursorText().c_str(), (int)roomPos.getX(), (int)roomPos.getY(), (int)scrPos.getX(), (int)scrPos.getY());
			//_sentence.setText(cursortxt.c_str());
			_sentence.setText(cursorText());

			// call clickedAt if any button down
			if ((_inputState.getInputActive() && _dialog->getState() == DialogState::None)) {
				if (_cursor.isLeftDown() || _cursor.isRightDown()) {
					clickedAt(scrPos);
				} else if (_cursor.leftDown || _cursor.rightDown) {
					if (_holdToMove && (_time > _nextHoldToMoveTime)) {
						walkFast();
						cancelSentence(_actor);
						if (_actor->_room == _room && (distance(_actor->_node->getAbsPos(), roomPos) > 5)) {
							Object::walk(_actor, roomPos);
						}
						_nextHoldToMoveTime = _time + 0.250f;
					}
				} else if (!_cursor.doubleClick) {
					_holdToMove = false;
					walkFast(false);
				}
			}
		} else {
			_hud->setVisible(false);
			_uiInv.setVisible(false);
			_noun1 = objAt(roomPos);
			Common::String cText = !_noun1 ? "" : _textDb->getText(_noun1->getName());
			_sentence.setText(cText);
			_inputState.setCursorShape(CursorShape::Normal);
			if (_cursor.isLeftDown() || _cursor.isRightDown())
				clickedAt(scrPos);
		}
	}

	_dialog->update(elapsed);
	_fadeShader->_elapsed += elapsed;

	// update camera
	_camera->update(_room, _followActor, elapsed);

	// update actorswitcher
	_actorSwitcher.update(actorSwitcherSlots(), elapsed);
	const uint32 endMiscTime = _system->getMillis();

	// update cutscene
	if (_cutscene) {
		if (_cutscene->update(elapsed)) {
			_cutscene.reset();
		}
	}
	const uint32 endUpdateCutsceneTime = _system->getMillis();

	// update threads: make a copy of the threads because during threads update, new threads can be added
	Common::Array<Common::SharedPtr<ThreadBase> > threads(_threads);
	Common::Array<Common::SharedPtr<ThreadBase> > threadsToRemove;

	bool isNotInDialog = _dialog->getState() == DialogState::None;
	for (auto it = threads.begin(); it != threads.end(); it++) {
		Common::SharedPtr<ThreadBase> thread(*it);
		if ((isNotInDialog || !thread->isGlobal()) && thread->update(elapsed)) {
			threadsToRemove.push_back(thread);
		}
	}
	// remove threads that are terminated
	for (auto it = threadsToRemove.begin(); it != threadsToRemove.end(); it++) {
		Common::SharedPtr<ThreadBase> thread(*it);
		size_t i = find(_threads, *it);
		if (i != (size_t)-1) {
			_threads.remove_at(i);
		}
	}
	const uint32 endUpdateThreadTime = _system->getMillis();

	// update callbacks
	for (auto it = _callbacks.begin(); it != _callbacks.end();) {
		Common::SharedPtr<Callback> cb(*it);
		if (!cb || cb->update(elapsed)) {
			it = _callbacks.erase(it);
			continue;
		}
		it++;
	}

	const uint32 endUpdateCallbacksTime = _system->getMillis();
	// update tasks
	Common::Array<Common::SharedPtr<Task> > tasks(_tasks);
	Common::Array<Common::SharedPtr<Task> > tasksToRemove;
	for (auto it = tasks.begin(); it != tasks.end();) {
		Common::SharedPtr<Task> task(*it);
		if (task->update(elapsed)) {
			tasksToRemove.push_back(task);
		}
		it++;
	}

	// remove tasks that are terminated
	for (auto it = tasksToRemove.begin(); it != tasksToRemove.end(); it++) {
		Common::SharedPtr<Task> task(*it);
		size_t i = find(_tasks, *it);
		if (i != (size_t)-1) {
			_tasks.remove_at(i);
		}
	}
	const uint32 endUpdateTasksTimes = _system->getMillis();

	// update objects
	if (_room) {
		_room->update(elapsed);
	}
	const uint32 endUpdateTimeRoom = _system->getMillis();

	// update inventory
	const bool hudActive = (_room->_fullscreen == FULLSCREENROOM && (scrPos.getY() < 180.f));
	_hud->_active = hudActive;
	_uiInv._active = hudActive;
	if (!_actor) {
		_uiInv.update(elapsed);
	} else {
		_hud->update(elapsed, scrPos, _noun1, _cursor.isLeftDown());
		VerbUiColors *verbUI = &_hud->actorSlot(_actor)->verbUiColors;
		_uiInv.update(elapsed, _actor, verbUI->inventoryBackground, verbUI->verbNormal);
	}

	updateTriggers();
	const uint32 endUpdateTime = _system->getMillis();
	_stats.totalUpdateTime = endUpdateTime - startUpdateTime;
	_stats.updateRoomTime = endUpdateTimeRoom - endUpdateTasksTimes;
	_stats.updateTasksTime = endUpdateTasksTimes - endUpdateCallbacksTime;
	_stats.updateMiscTime = endMiscTime - startUpdateTime;
	_stats.updateCutsceneTime = endUpdateCutsceneTime - endMiscTime;
	_stats.updateThreadsTime = endUpdateThreadTime - endUpdateCutsceneTime;
	_stats.updateCallbacksTime = endUpdateCallbacksTime - endUpdateThreadTime;
}

void TwpEngine::setShaderEffect(RoomEffect effect) {
	_shaderParams->effect = effect;
	switch (effect) {
	case RoomEffect::None:
		_gfx.use(nullptr);
		break;
	case RoomEffect::Sepia: {
		_gfx.use(_sepiaShader.get());
		_sepiaShader->setUniform("sepiaFlicker", _shaderParams->sepiaFlicker);
	} break;
	case RoomEffect::BlackAndWhite:
		_gfx.use(_bwShader.get());
		break;
	case RoomEffect::Ega:
		// TODO: _gfx.use(&_egaShader);
		break;
	case RoomEffect::Vhs:
		// TODO:_gfx.use(&_vhsShader);
		break;
	case RoomEffect::Ghost:
		_gfx.use(_ghostShader.get());
		break;
	}
}

void TwpEngine::draw(RenderTexture *outTexture) {
	if (_room) {
		Math::Vector2d screenSize = _room->getScreenSize();
		_gfx.camera(screenSize);
	}

	RenderTexture renderTexture(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	RenderTexture renderTexture2(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	RenderTexture renderTexture3(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));

	// draw scene into a texture
	_gfx.setRenderTarget(&renderTexture);
	_gfx.clear(Color(0, 0, 0));
	_gfx.use(nullptr);
	_scene->draw();

	// then render this texture with room effect to another texture
	_gfx.setRenderTarget(&renderTexture2);
	if (_room) {
		setShaderEffect(_room->_effect);
		_lighting->update(_room->_lights);
	}
	_shaderParams->randomValue[0] = getRandom();
	_shaderParams->timeLapse = fmodf(_time, 1000.f);
	_shaderParams->iGlobalTime = _shaderParams->timeLapse;
	_shaderParams->updateShader();

	_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	Math::Vector2d camPos = _gfx.cameraPos();
	_gfx.drawSprite(renderTexture, Color(), Math::Matrix4(), false, _fadeShader->_effect != FadeEffect::None);

	Texture *screenTexture = &renderTexture2;
	if (_fadeShader->_effect != FadeEffect::None) {
		// draw second room if any
		_gfx.setRenderTarget(&renderTexture);
		_gfx.use(nullptr);
		_gfx.cameraPos(_fadeShader->_cameraPos);
		_gfx.clear(Color(0, 0, 0));
		if (_fadeShader->_room && _fadeShader->_effect == FadeEffect::Wobble) {
			Math::Vector2d screenSize = _fadeShader->_room->getScreenSize();
			_gfx.camera(screenSize);
			_fadeShader->_room->_scene->draw();
		}

		_fadeShader->_fade = CLIP(_fadeShader->_elapsed / _fadeShader->_duration, 0.f, 1.f);

		// draw fade
		Texture *texture1 = nullptr;
		Texture *texture2 = nullptr;
		switch (_fadeShader->_effect) {
		case FadeEffect::Wobble:
			texture1 = &renderTexture;
			texture2 = &renderTexture2;
			screenTexture = &renderTexture3;
			break;
		case FadeEffect::In:
			texture1 = &renderTexture,
			texture2 = &renderTexture2;
			screenTexture = &renderTexture3;
			break;
		case FadeEffect::Out:
			texture1 = &renderTexture;
			texture2 = &renderTexture2;
			_fadeShader->_fade = 1.f - _fadeShader->_fade;
			screenTexture = &renderTexture3;
			break;
		case FadeEffect::None:
			break;
		}

		_gfx.setRenderTarget(&renderTexture3);
		_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
		_fadeShader->_texture1 = texture1;
		_fadeShader->_texture2 = texture2;
		_gfx.use(_fadeShader.get());
		_gfx.cameraPos(Math::Vector2d());
		_gfx.drawSprite(*texture1);
	}

	// draw to screen
	_gfx.use(nullptr);
	_gfx.setRenderTarget(outTexture);
	_gfx.clear(Color(0, 0, 0));
	_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	Math::Matrix4 m1, m2, m;
	m1.setPosition(Math::Vector3d(-SCREEN_WIDTH / 2.f, -SCREEN_HEIGHT / 2.f, 0.f));
	m2.setPosition(Math::Vector3d(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 0.f));
	Math::Angle angle;
	m.buildAroundZ(Math::Angle(-_room->_rotation));
	_gfx.drawSprite(*screenTexture, Color(), m2 * m * m1, false, false);

	// draw UI
	_gfx.cameraPos(camPos);
	_screenScene->draw();

	// imgui render
	_gfx.use(nullptr);
	_system->updateScreen();
}

void TwpEngine::updateSettingVars() {
	_resManager->resetSaylineFont();
	sqcall("setSettingVar", "toilet_paper_over", ConfMan.getBool("toiletPaperOver"));
	sqcall("setSettingVar", "annoying_injokes", ConfMan.getBool("annoyingInJokes"));
	if (ConfMan.getBool("ransomeUnbeeped") && !_pack->containsDLC()) {
		GUI::MessageDialog alert(_("You selected Ransome *unbeeped* (DLC) but the DLC has not been detected!"));
		Engine::runDialog(alert);
	}
	sqcall("setSettingVar", "ransome_unbeeped", ConfMan.getBool("ransomeUnbeeped") && _pack->containsDLC());
}

static void setVerbAction(int verbSlot) {
	ActorSlot *slot = g_twp->_hud->actorSlot(g_twp->_actor);
	if (!slot)
		return;
	g_twp->_hud->selectVerb(slot->verbSlots[verbSlot]._verb);
}

static bool isOnScreen(Common::SharedPtr<Object> obj) {
	Math::Vector2d pos = obj->_node->getPos() - g_twp->getGfx().cameraPos();
	Math::Vector2d size = g_twp->getGfx().camera();
	return Common::Rect(0.0f, 0.0f, size.getX(), size.getY()).contains(pos.getX(), pos.getY());
}

static void moveCursorTo(const Math::Vector2d &pos) {
	g_twp->_moveCursorTo.reset(new MoveCursorTo(pos, 0.1f));
}

static void gotoObject(bool next) {
	if (!g_twp->_room || !g_twp->_actor || !g_twp->_inputState._inputActive || !g_twp->_inputState._showCursor)
		return;

	// get all objects touchable and on screen and get their distance to the actor
	Math::Vector2d actorPos(g_twp->_actor->_node->getAbsPos());
	Common::Array<ObjPos> objPos;
	for (size_t i = 0; i < g_twp->_room->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = g_twp->_room->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj(layer->_objects[j]);
			if (g_twp->_resManager->isActor(obj->getId()) || !obj->isTouchable() || !isOnScreen(obj))
				continue;

			Math::Vector2d pos(obj->_node->getAbsPos());
			if (pos == Math::Vector2d())
				continue;

			Common::Rect hotspot = obj->_hotspot;
			Math::Vector2d center(hotspot.left + hotspot.width() / 2.f, hotspot.top + hotspot.height() / 2.f);
			pos += center;
			const float d = actorPos.getX() - pos.getX();
			objPos.push_back(ObjPos(obj, pos, d, j));
		}
	}

	if (objPos.empty())
		return;

	// sort these objects by distance
	Common::sort(objPos.begin(), objPos.end(), ObjPosComparer());

	// find between them if the cursor is currently on one object if so find the next object
	int index = 0;
	int zsort = INT_MAX;
	int objIndex = INT_MAX;
	Math::Vector2d mousePos(g_twp->screenToRoom(g_twp->winToScreen(g_twp->_cursor.pos)));
	for (size_t i = 0; i < objPos.size(); i++) {
		if (objPos[i]._obj->contains(mousePos)) {
			const int objZ = objPos[i]._obj->_node->getZSort();
			if (objZ > zsort)
				continue;
			if (objZ == zsort && objPos[i]._index >= objIndex)
				continue;
			objIndex = objPos[i]._index;
			zsort = objZ;
			if (next) {
				index = (i + 1) % objPos.size();
			} else {
				index = i - 1;
				if (index < 0) {
					index = objPos.size() - 1;
				}
			}
		}
	}

	// move the cursor to this object
	Math::Vector2d pos(objPos[index]._pos);
	pos = g_twp->roomToScreen(pos);
	moveCursorTo(g_twp->screenToWin(pos));
}

static void selectVerbInventory(int direction) {
	if (!g_twp->_room || !g_twp->_inputState._inputActive || !g_twp->_hud->isVisible())
		return;

	ActorSlot *slot = g_twp->_hud->actorSlot(g_twp->_actor);
	if (!slot)
		return;

	if (g_twp->_uiInv.isOver()) {
		int invIndex = g_twp->_uiInv.getOverIndex();
		if (invIndex != -1) {
			switch (direction) {
			case 0: // Left
			{
				if (invIndex == 0) {
					moveCursorTo(g_twp->_hud->getVerbPos(slot->verbSlots[7]));
					return;
				}
				if (invIndex == 4) {
					moveCursorTo(g_twp->_hud->getVerbPos(slot->verbSlots[8]));
					return;
				}
				invIndex--;
			} break;
			case 1: // Right
			{
				if (invIndex == 3 || invIndex == 7)
					return;
				invIndex++;
			} break;
			case 2: // Up
			{
				if (invIndex < 4) {
					g_twp->_actor->inventoryScrollUp();
					return;
				}
				invIndex -= 4;
			} break;
			case 3: // Down
				if (invIndex > 3) {
					g_twp->_actor->inventoryScrollDown();
					return;
				}
				invIndex += 4;
				break;
			}
			moveCursorTo(g_twp->screenToWin(g_twp->_uiInv.getPos(invIndex)));
			return;
		}
	}

	// get the verb where the cursor is
	int id = 0;
	for (int i = 1; i < MAX_VERBS; i++) {
		const VerbSlot &verbSlot = slot->verbSlots[i];
		if (verbSlot._over) {
			id = i;
			break;
		}
	}

	if (!id) {
		const VerbSlot &verbSlot = slot->verbSlots[5];
		Math::Vector2d pos(g_twp->_hud->getVerbPos(verbSlot));
		moveCursorTo(pos);
		return;
	}

	switch (direction) {
	case 0: // Left
		if (id >= 4)
			id -= 3;
		break;
	case 1: // Right
		if (id > 6) {
			moveCursorTo(g_twp->screenToWin(g_twp->_uiInv.getPos(id == 7 ? 0 : 4)));
			return;
		}
		id += 3;
		break;
	case 2: // Up
		if ((id % 3) != 1)
			id--;
		break;
	case 3: // Down
		if ((id % 3) != 0)
			id++;
		break;
	}

	const VerbSlot &verbSlot = slot->verbSlots[id];
	Math::Vector2d pos(g_twp->_hud->getVerbPos(verbSlot));
	moveCursorTo(pos);
}

Common::Error TwpEngine::run() {
	const Common::String &gameTarget = ConfMan.getActiveDomainName();
	AchMan.setActiveDomain(getMetaEngine()->getAchievementsInfo(gameTarget));

	initGraphics3d(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Set the engine's debugger console
	setDebugger(new Console());

	g_system->showMouse(false);
	g_system->lockMouse(true);

	_gfx.init();
	_hud->init();

	_camera.reset(new Camera());
	_shaderParams.reset(new ShaderParams());
	_bwShader.reset(new Shader());
	_bwShader->init("black&white", vsrc, bwShader);
	_ghostShader.reset(new Shader());
	_ghostShader->init("ghost", vsrc, ghostShader);
	_sepiaShader.reset(new Shader());
	_sepiaShader->init("sepia", vsrc, sepiaShader);
	_fadeShader.reset(new FadeShader());
	_lighting.reset(new Lighting());
	_resManager.reset(new ResManager());

	switch (_gameDescription->xorKey) {
	case GAME_XORKEY_56AD: {
		const XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};
		_pack->init(key);
		break;
	}
	case GAME_XORKEY_566D: {
		const XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x56, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
		_pack->init(key);
		break;
	}
	case GAME_XORKEY_5B6D: {
		const XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0x6D};
		_pack->init(key);
		break;
	}
	case GAME_XORKEY_5BAD: {
		const XorKey key{{0x4F, 0xD0, 0xA0, 0xAC, 0x4A, 0x5B, 0xB9, 0xE5, 0x93, 0x79, 0x45, 0xA5, 0xC1, 0xCB, 0x31, 0x93}, 0xAD};
		_pack->init(key);
		break;
	}
	default:
		error("This version of the game is invalid or not supported (yet?)");
		break;
	}

	Common::String lang(Common::String::format("ThimbleweedText_%s.tsv", ConfMan.get("language").c_str()));
	GGPackEntryReader entry;
	entry.open(*_pack, lang);
	_textDb.reset(new TextDb());
	_textDb->parseTsv(entry);

	CursorMan.showMouse(false);

	_vm.reset(new Vm());
	HSQUIRRELVM v = _vm->get();
	execNutEntry(v, "Defines.nut");
	execBnutEntry(v, "Boot.bnut");

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);
	else {
		// const SQChar *code = "cameraInRoom(StartScreen)";
		const SQChar *code = "start(1)";
		_vm->exec(code);
	}

	updateSettingVars();

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	callbacks.init = onImGuiInit;
	callbacks.render = onImGuiRender;
	callbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	// Simple event handling loop
	Common::Event e;
	uint time = _system->getMillis();
	while (!shouldQuit()) {
		while (_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START: {
				switch ((TwpAction)e.customType) {
				case TwpAction::kSkipCutscene:
					skipCutscene();
					break;
				case TwpAction::kGotoNextObject:
				case TwpAction::kGotoPreviousObject:
					gotoObject((TwpAction)e.customType == TwpAction::kGotoNextObject);
					break;
				case TwpAction::kSelectVerbInventoryLeft:
				case TwpAction::kSelectVerbInventoryRight:
				case TwpAction::kSelectVerbInventoryUp:
				case TwpAction::kSelectVerbInventoryDown:
					selectVerbInventory(e.customType - (int)TwpAction::kSelectVerbInventoryLeft);
					break;
				case TwpAction::kSelectActor1:
				case TwpAction::kSelectActor2:
				case TwpAction::kSelectActor3:
				case TwpAction::kSelectActor4:
				case TwpAction::kSelectActor5:
					if (_actorSwitcher._mode == asOn) {
						int index = (TwpAction)e.customType - kSelectActor1;
						ActorSlot *slot = &_hud->_actorSlots[index];
						if (slot->selectable && slot->actor && (slot->actor->_room->_name != "Void")) {
							setActor(slot->actor, true);
						}
					}
					break;
				case TwpAction::kSelectPreviousActor:
					if ((_actorSwitcher._mode == asOn) && _actor) {
						Common::Array<Common::SharedPtr<Object> > actors;
						for (int i = 0; i < NUMACTORS; i++) {
							ActorSlot *slot = &_hud->_actorSlots[i];
							if (slot->selectable && (slot->actor->_room->_name != "Void")) {
								actors.push_back(slot->actor);
							}
						}
						int index = find(actors, _actor) - 1;
						if (index < 0)
							index += actors.size();
						setActor(actors[index], true);
					}
					break;
				case TwpAction::kSelectNextActor:
					if ((_actorSwitcher._mode == asOn) && _actor) {
						Common::Array<Common::SharedPtr<Object> > actors;
						for (int i = 0; i < NUMACTORS; i++) {
							ActorSlot *slot = &_hud->_actorSlots[i];
							if (slot->selectable && (slot->actor->_room->_name != "Void")) {
								actors.push_back(slot->actor);
							}
						}
						size_t index = find(actors, _actor) + 1;
						if (index >= actors.size())
							index -= actors.size();
						setActor(actors[index], true);
					}
					break;
				case TwpAction::kSkipText:
					stopTalking();
					break;
				case TwpAction::kSelectChoice1:
				case TwpAction::kSelectChoice2:
				case TwpAction::kSelectChoice3:
				case TwpAction::kSelectChoice4:
				case TwpAction::kSelectChoice5:
				case TwpAction::kSelectChoice6:
					if (_dialog->getState() == DialogState::None) {
						int index = (TwpAction)e.customType - kSelectChoice1;
						_dialog->choose(index);
					}
					break;
				case TwpAction::kShowHotspots:
					_hotspotMarker->setVisible(!_hotspotMarker->isVisible());
					break;
				case TwpAction::kOpen:
				case TwpAction::kClose:
				case TwpAction::kGive:
				case TwpAction::kPickUp:
				case TwpAction::kLookAt:
				case TwpAction::kTalkTo:
				case TwpAction::kPush:
				case TwpAction::kPull:
				case TwpAction::kUse:
					setVerbAction(1 + (int)e.customType - (int)TwpAction::kOpen);
					break;
				default:
					break;
				}
			} break;
			case Common::EVENT_KEYDOWN:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					if (_control)
						_speed = MAX(_speed - 1, 1);
					_cursor.holdLeft = true;
					break;
				case Common::KEYCODE_RIGHT:
					if (_control)
						_speed = MIN(_speed + 1, 8);
					_cursor.holdRight = true;
					break;
				case Common::KEYCODE_UP:
					if (_dialog->getState() == WaitingForChoice) {
						moveCursorTo(screenToWin(_dialog->getPreviousChoicePos(winToScreen(_cursor.pos))));
					} else {
						_cursor.holdUp = true;
					}
					break;
				case Common::KEYCODE_DOWN:
					if (_dialog->getState() == WaitingForChoice) {
						moveCursorTo(screenToWin(_dialog->getNextChoicePos(winToScreen(_cursor.pos))));
					} else {
						_cursor.holdDown = true;
					}
					break;
				case Common::KEYCODE_LCTRL:
					_control = true;
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_KEYUP:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					_cursor.holdLeft = false;
					break;
				case Common::KEYCODE_RIGHT:
					_cursor.holdRight = false;
					break;
				case Common::KEYCODE_UP:
					_cursor.holdUp = false;
					break;
				case Common::KEYCODE_DOWN:
					_cursor.holdDown = false;
					break;
				case Common::KEYCODE_LCTRL:
					_control = false;
					break;
				case Common::KEYCODE_w:
					if (_control) {
						WalkboxMode mode = (WalkboxMode)(((int)_walkboxNode->getMode() + 1) % 3);
						debugC(kDebugGame, "set walkbox mode to: %s", (mode == WalkboxMode::Merged ? "merged" : mode == WalkboxMode::All ? "all"
																																		 : "none"));
						_walkboxNode->setMode(mode);
					}
					break;
				case Common::KEYCODE_g:
					if (_control) {
						PathMode mode = (PathMode)(((int)_pathNode->getMode() + 1) % 3);
						debugC(kDebugGame, "set path mode to: %s", (mode == PathMode::GraphMode ? "graph" : mode == PathMode::All ? "all"
																																  : "none"));
						_pathNode->setMode(mode);
					}
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				_cursor.pos = Math::Vector2d(e.mouse.x, e.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				_cursor.leftDown = true;
				if ((_time - _cursor.leftDownTime) < 1.0f) {
					_cursor.doubleClick = true;
				} else {
					_cursor.doubleClick = false;
				}
				break;
			case Common::EVENT_LBUTTONUP:
				_cursor.leftDown = false;
				_cursor.leftDownTime = _time;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_cursor.doubleClick = false;
				_cursor.rightDown = true;
				break;
			case Common::EVENT_RBUTTONUP:
				_cursor.rightDown = false;
				break;
			case Common::EVENT_WHEELDOWN:
				if (_actor) {
					_actor->inventoryScrollDown();
				}
				break;
			case Common::EVENT_WHEELUP:
				if (_actor) {
					_actor->inventoryScrollUp();
				}
				break;
			default:
				break;
			}
		}

		const float mouseMoveSpeed = 4.f;
		if (_cursor.holdLeft) {
			_cursor.pos.setX(MAX(_cursor.pos.getX() - mouseMoveSpeed, 0.f));
		}
		if (_cursor.holdRight) {
			_cursor.pos.setX(MIN(_cursor.pos.getX() + mouseMoveSpeed, (float)SCREEN_WIDTH));
		}
		if (_cursor.holdUp) {
			_cursor.pos.setY(MAX(_cursor.pos.getY() - mouseMoveSpeed, 0.f));
		}
		if (_cursor.holdDown) {
			_cursor.pos.setY(MIN(_cursor.pos.getY() + mouseMoveSpeed, (float)SCREEN_HEIGHT));
		}

		uint32 newTime = _system->getMillis();
		uint32 delta = newTime - time;
		time = newTime;
		update(_speed * delta / 1000.f);

		const uint32 startDrawTime = _system->getMillis();
		draw();
		_stats.drawTime = _system->getMillis() - startDrawTime;
		_cursor.update();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		if (delta < 10) {
			_system->delayMillis(10 - delta);
		}
	}

#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	g_system->showMouse(true);
	g_system->lockMouse(false);

	return Common::kNoError;
}

Common::Error TwpEngine::loadGameState(int slot) {
	Common::Error result = Engine::loadGameState(slot);
	if (result.getCode() == Common::kNoError)
		return Common::kNoError;

	Common::InSaveFile *file = getSaveFileManager()->openRawFile(getSaveStateName(slot));
	if (file) {
		return loadGameStream(file);
	}
	return Common::kPathDoesNotExist;
}

Common::Error TwpEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (!_saveGameManager->loadGame(*stream)) {
		return Common::kUnknownError;
	}
	Common::String md5 = stream->readString(0, 32);
	uint16 savegameVersion = stream->readUint16LE();
	debug("Load game with MD5: %s, version: %u", md5.c_str(), savegameVersion);
	return Common::kNoError;
}

bool TwpEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _saveGameManager->_allowSaveGame && !_cutscene;
}

Common::Error TwpEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Engine::saveGameState(slot, desc, isAutosave);
	if (result.getCode() != Common::kNoError)
		return result;

	if (DebugMan.isDebugChannelEnabled(kDebugSave)) {
		Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(Common::String::format("Savegame%d.save", slot), false);
		if (!saveFile)
			return Common::kWritingFailed;

		_saveGameManager->saveGame(saveFile);
		Common::OutSaveFile *thumbnail = _saveFileMan->openForSaving(Common::String::format("Savegame%d.png", slot), false);
		Graphics::Surface surface;
		capture(surface, 320, 180);
		Image::writePNG(*thumbnail, surface);
		thumbnail->finalize();
		delete thumbnail;

		saveFile->finalize();
		delete saveFile;
	}
	return result;
}

Common::Error TwpEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	_saveGameManager->saveGame(stream);
	stream->writeString(_gameDescription->desc.filesDescriptions[0].md5);
	stream->writeUint16LE(TWP_SAVEGAME_VERSION);
	return Common::kNoError;
}

static void onGetPairs(const Common::String &k, HSQOBJECT &oTable, void *data) {
	DefineObjectParams *params = static_cast<DefineObjectParams *>(data);
	if (oTable._type == OT_TABLE) {
		if (sqrawexists(oTable, "icon")) {
			// Add inventory object to root table
			debugC(kDebugGame, "Add %s to inventory", k.c_str());
			sqsetf(sqrootTbl(params->v), k, oTable);

			// set room as delegate
			sqsetdelegate(oTable, params->room->_table);

			// declare flags if does not exist
			if (!sqrawexists(oTable, "flags"))
				sqsetf(oTable, "flags", 0);
			Common::SharedPtr<Object> obj(new Object(oTable, k));
			const int id = g_twp->_resManager->newObjId();
			setId(obj->_table, id);
			g_twp->_resManager->_allObjects[id] = obj;
			obj->_node = Common::SharedPtr<Node>(new Node(k));
			obj->_nodeAnim = Common::SharedPtr<Anim>(new Anim(obj.get()));
			obj->_node->addChild(obj->_nodeAnim.get());
			Object::setRoom(obj, params->room);
			// set room as delegate
			sqsetdelegate(obj->_table, params->room->_table);
		} else {
			Common::SharedPtr<Object> obj = params->room->getObj(k);
			if (!obj) {
				debugC(kDebugGame, "object: %s not found in wimpy", k.c_str());
				if (sqrawexists(oTable, "name")) {
					obj.reset(new Object());
					obj->_key = k;
					obj->_layer = params->room->layer(0);
					params->room->layer(0)->_objects.push_back(obj);
				} else {
					return;
				}
			}

			if (SQ_FAILED(sqgetf(params->room->_table, k, obj->_table)))
				error("Failed to get room object");
			const int id = g_twp->_resManager->newObjId();
			setId(obj->_table, id);
			g_twp->_resManager->_allObjects[id] = obj;
			debugC(kDebugGame, "Create object: %s #%d", k.c_str(), obj->getId());

			// add it to the root table if not a pseudo room
			if (!params->pseudo)
				sqsetf(sqrootTbl(params->v), k, obj->_table);

			if (sqrawexists(obj->_table, "initState")) {
				// info fmt"initState {obj.key}"
				SQInteger state;
				if (SQ_FAILED(sqgetf(obj->_table, "initState", state)))
					error("Failed to get initState");
				obj->setState(state, true);
			} else {
				obj->setState(0, true);
			}
			Object::setRoom(obj, params->room);

			// set room as delegate
			sqsetdelegate(obj->_table, params->room->_table);

			// declare flags if does not exist
			if (!sqrawexists(obj->_table, "flags"))
				sqsetf(obj->_table, "flags", 0);
		}
	}
}

Common::SharedPtr<Room> TwpEngine::defineRoom(const Common::String &name, HSQOBJECT table, bool pseudo) {
	HSQUIRRELVM v = _vm->get();
	debugC(kDebugGame, "Load room: %s", name.c_str());
	Common::SharedPtr<Room> result;
	if (name == "Void") {
		result.reset(new Room(name, table));
		result->_scene = Common::SharedPtr<Scene>(new Scene());
		Common::SharedPtr<Layer> layer(new Layer("background", Math::Vector2d(1.f, 1.f), 0));
		layer->_node = Common::SharedPtr<Node>(new ParallaxNode(Math::Vector2d(1.f, 1.f), "", Common::StringArray()));
		result->_layers.push_back(layer);
		result->_scene->addChild(layer->_node.get());
		sqsetf(sqrootTbl(v), name, result->_table);
	} else {
		result.reset(new Room(name, table));
		Common::String background;
		if (SQ_FAILED(sqgetf(table, "background", background)))
			error("Failed to get room background");
		GGPackEntryReader entry;
		entry.open(*_pack, background + ".wimpy");
		Room::load(result, entry);
		result->_name = name;
		result->_pseudo = pseudo;
		for (size_t i = 0; i < result->_layers.size(); i++) {
			Common::SharedPtr<Layer> layer = result->_layers[i];
			// create layer node
			Common::SharedPtr<ParallaxNode> layerNode(new ParallaxNode(layer->_parallax, result->_sheet, layer->_names));
			layerNode->setZSort(layer->_zsort);
			layerNode->setName(Common::String::format("Layer %s(%d)", layer->_names[0].c_str(), layer->_zsort));
			layer->_node = layerNode;
			result->_scene->addChild(layerNode.get());

			for (size_t j = 0; j < layer->_objects.size(); j++) {
				Common::SharedPtr<Object> obj = layer->_objects[j];
				if (!sqrawexists(result->_table, obj->_key)) {
					// this object does not exist, so create it
					sq_newtable(v);
					sq_getstackobj(v, -1, &obj->_table);
					sq_addref(v, &obj->_table);
					sq_pop(v, 1);

					// assign an id
					const int id = _resManager->newObjId();
					setId(obj->_table, id);
					_resManager->_allObjects[id] = obj;
					// info fmt"Create object with new table: {obj.name} #{obj.id}"

					// adds the object to the room table
					sqsetf(result->_table, obj->_key, obj->_table);
					Object::setRoom(obj, result);
					obj->setState(0, true);

					if (obj->_objType == otNone)
						obj->setTouchable(false);
				} else {
					if (pseudo) {
						// if it's a pseudo room we need to clone each object
						if (SQ_FAILED(sqgetf(result->_table, obj->_key, obj->_table)))
							error("Failed to get room object");
						sq_pushobject(v, obj->_table);
						sq_clone(v, -1);
						sq_getstackobj(v, -1, &obj->_table);
						sq_addref(v, &obj->_table);
						sq_remove(v, -2);
						sqsetf(result->_table, obj->_key, obj->_table);
					}
					if (obj->_objType == otNone) {
						obj->setTouchable(true);
					}
				}

				layerNode->addChild(obj->_node.get());
			}
		}
	}

	// assign parent node
	for (size_t i = 0; i < result->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = result->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj = layer->_objects[j];
			if (obj->_parent.size() > 0) {
				Common::SharedPtr<Object> parent = result->getObj(obj->_parent);
				if (!parent) {
					warning("parent: '%s' not found", obj->_parent.c_str());
				} else {
					parent->_node->addChild(obj->_node.get());
				}
			}
		}
	}

	DefineObjectParams params;
	params.pseudo = pseudo;
	params.v = v;
	params.room = result;
	if (SQ_FAILED(sqgetpairs(result->_table, onGetPairs, &params)))
		error("Falied to define objects");

	// declare the room in the root table
	setId(result->_table, _resManager->newRoomId());
	sqsetf(sqrootTbl(v), name, result->_table);

	return result;
}

void TwpEngine::enterRoom(Common::SharedPtr<Room> room, Common::SharedPtr<Object> door) {
	HSQUIRRELVM v = getVm();
	// Called when the room is entered.
	debugC(kDebugGame, "call enter room function of %s", room->_name.c_str());

	// exit current room
	exitRoom(_room);

	// reset fade effect if we change the room except for wobble effect
	if (_fadeShader->_effect != FadeEffect::Wobble) {
		_fadeShader->_effect = FadeEffect::None;
	}

	// sets the current room for scripts
	sqsetf(sqrootTbl(v), "currentRoom", room->_table);

	if (_room)
		_room->_scene->remove();
	_room = room;
	room->_effect = RoomEffect::None;
	_scene->addChild(_room->_scene.get());
	_room->_lights._numLights = 0;
	_room->setOverlay(Color(0.f, 0.f, 0.f, 0.f));
	_camera->setBounds(Rectf::fromMinMax(Math::Vector2d(), _room->_roomSize));
	if (_actor && _hud->actorSlot(_actor))
		_hud->selectVerb(_hud->actorSlot(_actor)->verbSlots[0]._verb);

	// move current actor to the new room
	Math::Vector2d camPos;
	if (_actor) {
		cancelSentence();
		if (door) {
			Facing facing = flip(door->getDoorFacing());
			Object::setRoom(_actor, room);
			if (door) {
				_actor->setFacing(facing);
				_actor->_node->setPos(door->getUsePos());
			}
			camPos = _actor->_node->getPos();
		}
	}

	_camera->setRoom(room);
	_camera->setAt(camPos);

	stopTalking();

	// call actor enter function and objects enter function
	for (size_t i = 0; i < room->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = room->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj = layer->_objects[j];
			// add all scaling triggers
			if (obj->_objType == ObjectType::otTrigger) {
				Scaling *scaling = getScaling(obj->_key);
				if (scaling) {
					_room->_scalingTriggers.push_back(ScalingTrigger(obj, scaling));
				}
			}
			if (_resManager->isActor(obj->getId())) {
				actorEnter(obj);
			} else if (sqrawexists(obj->_table, "enter"))
				sqcall(obj->_table, "enter");
		}
	}

	// call room enter function with the door as a parameter if requested
	int nparams = sqparamCount(v, _room->_table, "enter");
	if (nparams == 2) {
		if (!door) {
			HSQOBJECT doorTable;
			sq_resetobject(&doorTable);
			sqcall(_room->_table, "enter", doorTable);
		} else {
			sqcall(_room->_table, "enter", door->_table);
		}
	} else {
		sqcall(_room->_table, "enter");
	}

	// call global function enteredRoom with the room as argument
	sqcall("enteredRoom", room->_table);
}

void TwpEngine::actorEnter(Common::SharedPtr<Object> actor) {
	if (!actor)
		return;
	if (sqrawexists(_room->_table, "actorEnter")) {
		sqcall(_room->_table, "actorEnter", actor->_table);
	} else {
		sqcall("actorEnter", actor->_table);
	}
}

void TwpEngine::exitRoom(Common::SharedPtr<Room> nextRoom) {
	HSQUIRRELVM v = getVm();
	_mixer->stopAll();
	_talking.reset();

	if (_room) {
		_room->_triggers.clear();
		_room->_scalingTriggers.clear();

		for (size_t i = 0; i < _room->_layers.size(); i++) {
			Common::SharedPtr<Layer> layer(_room->_layers[i]);
			for (size_t j = 0; j < layer->_objects.size(); j++) {
				Common::SharedPtr<Object> obj = layer->_objects[j];
				obj->stopObjectMotors();
				if (_resManager->isActor(obj->getId())) {
					actorExit(obj);
				}
			}
		}

		// call room exit function with the next room as a parameter if requested
		int nparams = sqparamCount(v, _room->_table, "exit");
		if (nparams == 2) {
			sqcall(_room->_table, "exit", nextRoom->_table);
		} else {
			sqcall(_room->_table, "exit");
		}

		// delete all temporary objects
		for (size_t i = 0; i < _room->_layers.size(); i++) {
			Common::SharedPtr<Layer> layer = _room->_layers[i];
			for (auto it = layer->_objects.begin(); it != layer->_objects.end();) {
				Common::SharedPtr<Object> obj(*it);
				if (obj->_temporary) {
					obj->_node->remove();
					it = layer->_objects.erase(it);
					continue;
				} else if (_resManager->isActor(obj->getId()) && _actor != obj) {
					obj->stopObjectMotors();
				}
				it++;
			}
		}

		// call global function exitedRoom with the room as argument
		sqcall("exitedRoom", _room->_table);

		// stop all local threads
		for (size_t i = 0; i < _threads.size(); i++) {
			Common::SharedPtr<ThreadBase> thread = _threads[i];
			if (!thread->isGlobal()) {
				thread->stop();
			}
		}

		// stop all lights
		_room->_lights._numLights = 0;
	}
}

void TwpEngine::setRoom(Common::SharedPtr<Room> room, bool force) {
	if (room && ((_room != room) || force))
		enterRoom(room);
}

void TwpEngine::actorExit(Common::SharedPtr<Object> actor) {
	if (actor && _room) {
		if (sqrawexists(_room->_table, "actorExit")) {
			sqcall(_room->_table, "actorExit", actor->_table);
		}
	}
}

void TwpEngine::cancelSentence(Common::SharedPtr<Object> actor) {
	debugC(kDebugGame, "cancelSentence");
	if (!actor)
		actor = _actor;
	if (actor)
		actor->_exec.enabled = false;
}

void TwpEngine::execBnutEntry(HSQUIRRELVM v, const Common::String &entry) {
	GGPackEntryReader reader;
	reader.open(*_pack, entry);
	GGBnutReader nut;
	nut.open(&reader);
	Common::String code = nut.readString();
	sqexec(v, code.c_str(), entry.c_str());
}

void TwpEngine::execNutEntry(HSQUIRRELVM v, const Common::String &entry) {
	if (_pack->assetExists(entry.c_str())) {
		GGPackEntryReader reader;
		debugC(kDebugGame, "read existing '%s'", entry.c_str());
		reader.open(*_pack, entry);
		Common::String code = reader.readString();
		// debugC(kDebugGame, "%s", code.c_str());
		sqexec(v, code.c_str(), entry.c_str());
	} else {
		Common::String newEntry = entry.substr(0, entry.size() - 4) + ".bnut";
		debugC(kDebugGame, "read existing '%s'", newEntry.c_str());
		if (_pack->assetExists(newEntry.c_str())) {
			execBnutEntry(v, newEntry);
		} else {
			error("'%s' and '%s' have not been found", entry.c_str(), newEntry.c_str());
		}
	}
}

void TwpEngine::cameraAt(const Math::Vector2d &at) {
	_camera->setRoom(_room);
	_camera->setAt(at);
}

Math::Vector2d TwpEngine::cameraPos() {
	return _camera->getAt();
}

void TwpEngine::follow(Common::SharedPtr<Object> actor) {
	_followActor = actor;
	if (actor) {
		Math::Vector2d pos = actor->_node->getPos();
		Common::SharedPtr<Room> oldRoom = _room;
		setRoom(actor->_room);
		if (oldRoom != actor->_room)
			cameraAt(pos);
	}
}

void TwpEngine::fadeTo(FadeEffect effect, float duration, bool fadeToSep) {
	_fadeShader->_fadeToSepia = fadeToSep;
	_fadeShader->_effect = effect;
	_fadeShader->_room = _room;
	_fadeShader->_cameraPos = _gfx.cameraPos();
	_fadeShader->_duration = duration;
	_fadeShader->_movement = effect == FadeEffect::Wobble ? 0.005f : 0.f;
	_fadeShader->_elapsed = 0.f;
}

Common::SharedPtr<Object> TwpEngine::objAt(const Math::Vector2d &pos) {
	Common::SharedPtr<Object> result;
	objsAt(pos, GetByZOrder(result));
	return result;
}

void TwpEngine::setActor(Common::SharedPtr<Object> actor, bool userSelected) {
	_actor = actor;
	_hud->_actor = actor;
	resetVerb();
	if (!_hud->getParent() && actor) {
		_screenScene->addChild(_hud.get());
	} else if (_hud->getParent() && !actor) {
		_screenScene->removeChild(_hud.get());
	}

	// call onActorSelected callbacks
	sqcall("onActorSelected", actor->_table, userSelected);
	Common::SharedPtr<Room> room = !actor ? nullptr : actor->_room;
	if (room) {
		if (sqrawexists(room->_table, "onActorSelected")) {
			sqcall(room->_table, "onActorSelected", actor->_table, userSelected);
		}
	}

	if (actor)
		follow(actor);
}

bool TwpEngine::selectable(Common::SharedPtr<Object> actor) {
	for (int i = 0; i < NUMACTORS; i++) {
		ActorSlot *slot = &_hud->_actorSlots[i];
		if (slot->actor == actor)
			return slot->selectable;
	}
	return false;
}

static void giveTo(Common::SharedPtr<Object> actor1, Common::SharedPtr<Object> actor2, Common::SharedPtr<Object> obj) {
	obj->_owner = actor2;
	actor2->_inventory.push_back(obj);
	// force actors to be face to face
	actor2->setFacing(flip(actor1->getFacing()));
	int index = find(actor1->_inventory, obj);
	if (index != -1)
		actor1->_inventory.remove_at(index);
}

void TwpEngine::resetVerb() {
	debugC(kDebugGame, "reset nouns");
	_noun1 = nullptr;
	_noun2 = nullptr;
	_useFlag = UseFlag::ufNone;
	_hud->_verb = _hud->actorSlot(_actor)->verbSlots[0]._verb;
}

bool TwpEngine::callVerb(Common::SharedPtr<Object> actor, VerbId verbId, Common::SharedPtr<Object> noun1, Common::SharedPtr<Object> noun2) {
	if (noun1) {
		sqcall("onObjectClick", noun1->_table);
	}

	// Called after the actor has walked to the object.
	Common::String name = !actor ? "currentActor" : actor->_key;
	Common::String noun1name = !noun1 ? "null" : noun1->_key;
	Common::String noun2name = !noun2 ? "null" : noun2->_key;
	ActorSlot *slot = _hud->actorSlot(actor);
	Verb *verb = slot->getVerb(verbId.id);
	Common::String verbFuncName = verb ? verb->fun : slot->verbSlots[0]._verb.fun;
	debugC(kDebugGame, "callVerb(%s,%s,%s,%s)", name.c_str(), verbFuncName.c_str(), noun1name.c_str(), noun2name.c_str());

	// test if object became untouchable
	if (noun1 && !noun1->inInventory() && !noun1->isTouchable())
		return false;
	if (noun2 && !noun2->inInventory() && !noun2->isTouchable())
		return false;

	// check if verb is use and object can be used with or in or on
	if (noun1 && (verbId.id == VERB_USE) && !noun2) {
		_useFlag = noun1->useFlag();
		if (_useFlag != UseFlag::ufNone) {
			_noun1 = noun1;
			return false;
		}
	}

	if (verbId.id == VERB_GIVE) {
		if (!noun2) {
			debugC(kDebugGame, "set use flag to ufGiveTo");
			_useFlag = UseFlag::ufGiveTo;
			_noun1 = noun1;
		} else if (noun1) {
			bool handled = false;
			if (sqrawexists(noun2->_table, verbFuncName)) {
				debugC(kDebugGame, "call %s on %s", verbFuncName.c_str(), noun2->_key.c_str());
				sqcallfunc(handled, noun2->_table, verbFuncName.c_str(), noun1->_table);
			}
			// verbGive is called on object only for non selectable actors
			if (!handled && !selectable(noun2) && sqrawexists(noun1->_table, verbFuncName)) {
				debugC(kDebugGame, "call %s on %s", verbFuncName.c_str(), noun1->_key.c_str());
				sqcall(noun1->_table, verbFuncName.c_str(), noun2->_table);
				handled = true;
			}
			if (!handled) {
				debugC(kDebugGame, "call objectGive");
				sqcall("objectGive", noun1->_table, _actor->_table, noun2->_table);
				giveTo(_actor, noun2, noun1);
			}
			resetVerb();
		}
		return false;
	}

	if (!noun1) {
		HSQOBJECT emptyTable;
		sq_resetobject(&emptyTable);
		debugC(kDebugGame, "call defaultObject.%s", verbFuncName.c_str());
		sqcall(_defaultObj, verbFuncName.c_str(), emptyTable, emptyTable);
	} else if (!noun2) {
		if (sqrawexists(noun1->_table, verbFuncName)) {
			int count = sqparamCount(getVm(), noun1->_table, verbFuncName);
			debugC(kDebugGame, "call %s.%s", noun1->_key.c_str(), verbFuncName.c_str());
			if (count == 1) {
				sqcall(noun1->_table, verbFuncName.c_str());
			} else {
				sqcall(noun1->_table, verbFuncName.c_str(), actor->_table);
			}
		} else if (sqrawexists(noun1->_table, VERBDEFAULT)) {
			sqcall(noun1->_table, VERBDEFAULT);
		} else {
			debugC(kDebugGame, "call defaultObject.%s", verbFuncName.c_str());
			sqcall(_defaultObj, verbFuncName.c_str(), noun1->_table, actor->_table);
		}
	} else {
		if (sqrawexists(noun1->_table, verbFuncName)) {
			debugC(kDebugGame, "call %s.%s", noun1->_key.c_str(), verbFuncName.c_str());
			sqcall(noun1->_table, verbFuncName.c_str(), noun2->_table);
		} else if (sqrawexists(noun1->_table, VERBDEFAULT)) {
			sqcall(noun1->_table, VERBDEFAULT);
		} else {
			debugC(kDebugGame, "call defaultObject.%s", verbFuncName.c_str());
			sqcall(_defaultObj, verbFuncName.c_str(), noun1->_table, noun2->_table);
		}
	}

	if (verbId.id == VERB_PICKUP) {
		sqcall("onPickup", noun1->_table, actor->_table);
	}

	resetVerb();
	return false;
}

void TwpEngine::callTrigger(Common::SharedPtr<Object> obj, HSQOBJECT trigger) {
	if (trigger._type != OT_NULL) {
		HSQUIRRELVM v = getVm();
		// create trigger thread
		sq_newthread(v, 1024);
		HSQOBJECT threadObj;
		sq_resetobject(&threadObj);
		if (SQ_FAILED(sq_getstackobj(v, -1, &threadObj))) {
			error("Couldn't get coroutine thread from stack");
			return;
		}
		sq_addref(v, &threadObj);
		sq_pop(v, 1);

		// create args
		SQInteger nParams, nfreevars;
		sq_pushobject(v, trigger);
		sq_getclosureinfo(v, -1, &nParams, &nfreevars);
		sq_pop(v, 1);

		Common::Array<HSQOBJECT> args;
		if (nParams == 2) {
			args.push_back(_actor->_table);
		}

		Common::SharedPtr<Thread> thread(new Thread("Trigger", false, threadObj, obj->_table, trigger, Common::move(args)));

		debugC(kDebugGame, "create triggerthread id: %d}", thread->getId());
		_threads.push_back(thread);

		// call the closure in the thread
		if (!thread->call()) {
			error("trigger call failed");
		}
	}
}

void TwpEngine::updateTriggers() {
	if (_actor) {
		// check if actor enters or leaves an object trigger
		for (size_t i = 0; i < _room->_triggers.size(); i++) {
			Common::SharedPtr<Object> trigger = _room->_triggers[i];
			if (!trigger->_triggerActive && trigger->contains(_actor->_node->getAbsPos())) {
				debugC(kDebugGame, "call enter trigger %s", trigger->_key.c_str());
				trigger->_triggerActive = true;
				callTrigger(trigger, trigger->_enter);
			} else if (trigger->_triggerActive && !trigger->contains(_actor->_node->getAbsPos())) {
				debugC(kDebugGame, "call leave trigger %s", trigger->_key.c_str());
				trigger->_triggerActive = false;
				callTrigger(trigger, trigger->_leave);
			}
		}

		// check if actor enters or leaves a scaling trigger
		for (size_t i = 0; i < _room->_scalingTriggers.size(); i++) {
			ScalingTrigger *trigger = &_room->_scalingTriggers[i];
			if (trigger->_obj->_triggerActive && !trigger->_obj->contains(_actor->_node->getAbsPos())) {
				debugC(kDebugGame, "leave scaling trigger %s", trigger->_obj->_key.c_str());
				trigger->_obj->_triggerActive = false;
				_room->_scaling = _room->_scalings[0];
			}
		}
		for (size_t i = 0; i < _room->_scalingTriggers.size(); i++) {
			ScalingTrigger *trigger = &_room->_scalingTriggers[i];
			if (!trigger->_obj->_triggerActive && trigger->_obj->contains(_actor->_node->getAbsPos())) {
				debugC(kDebugGame, "enter scaling trigger %s", trigger->_obj->_key.c_str());
				trigger->_obj->_triggerActive = true;
				_room->_scaling = *trigger->_scaling;
			}
		}
	}
}

void TwpEngine::sayLineAt(const Math::Vector2d &pos, const Color &color, Common::SharedPtr<Object> actor, float duration, const Common::String &text) {
	_talking = Common::ScopedPtr<SayLineAt>(new SayLineAt(pos, color, actor, duration, text));
}

void TwpEngine::stopTalking() {
	_talking.reset();
	if (!_room)
		return;
	for (auto it = _room->_layers.begin(); it != _room->_layers.end(); it++) {
		Common::SharedPtr<Layer> layer = *it;
		for (auto it2 = layer->_objects.begin(); it2 != layer->_objects.end(); it2++) {
			(*it2)->stopTalking();
		}
	}
}

bool TwpEngine::isSomeoneTalking() const {
	if (_talking && _talking->isEnabled())
		return true;
	if (!_room)
		return false;
	for (auto it = _actors.begin(); it != _actors.end(); it++) {
		Common::SharedPtr<Object> obj = *it;
		if (obj->_room != _room)
			continue;
		if (obj->getTalking() && obj->getTalking()->isEnabled())
			return true;
	}
	for (auto it = _room->_layers.begin(); it != _room->_layers.end(); it++) {
		Common::SharedPtr<Layer> layer = *it;
		for (auto it2 = layer->_objects.begin(); it2 != layer->_objects.end(); it2++) {
			Common::SharedPtr<Object> obj = *it2;
			if (obj->_room != _room)
				continue;
			if (obj->getTalking() && obj->getTalking()->isEnabled())
				return true;
		}
	}
	return false;
}

float TwpEngine::getRandom() const {
	return g_twp->getRandomSource().getRandomNumber(RAND_MAX) / (float)RAND_MAX;
}

float TwpEngine::getRandom(float min, float max) const {
	float scale = getRandom();
	return min + scale * (max - min);
}

void TwpEngine::skipCutscene() {
	if (!_cutscene)
		return;
	if (_cutscene->hasOverride()) {
		_cutscene->cutsceneOverride();
		return;
	}
	_noOverride->reset();
}

Scaling *TwpEngine::getScaling(const Common::String &name) {
	for (size_t i = 0; i < _room->_scalings.size(); i++) {
		Scaling *scaling = &_room->_scalings[i];
		if (scaling->trigger == name) {
			return scaling;
		}
	}
	return nullptr;
}

void TwpEngine::capture(Graphics::Surface &surface, int width, int height) {
	// render scene into texture
	Common::Array<byte> data;
	RenderTexture rt(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	draw(&rt);

	// get the data from it
	rt.capture(data);

	// flip it (due to opengl) and scale it to the desired size
	Graphics::PixelFormat fmt(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface s;
	s.init(SCREEN_WIDTH, SCREEN_HEIGHT, 4 * SCREEN_WIDTH, data.data(), fmt);
	s.flipVertical(Common::Rect(s.w, s.h));

	Graphics::Surface *scaledSurface = s.scale(width, height);
	// and save to stream
	surface.copyFrom(*scaledSurface);
	delete scaledSurface;
}

HSQUIRRELVM TwpEngine::getVm() { return _vm->get(); }

int TwpEngine::runDialog(GUI::Dialog &dialog) {
	g_system->showMouse(true);
	g_system->lockMouse(false);

	int result = Engine::runDialog(dialog);
	updateSettingVars();
	return result;
}

ScalingTrigger::ScalingTrigger(Common::SharedPtr<Object> obj, Scaling *scaling) : _obj(obj), _scaling(scaling) {}

} // End of namespace Twp
