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

#include "common/debug.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/opengl/system_headers.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/console.h"
#include "twp/vm.h"
#include "twp/ggpack.h"
#include "twp/gfx.h"
#include "twp/lighting.h"
#include "twp/font.h"
#include "twp/thread.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"
#include "twp/object.h"
#include "twp/ids.h"
#include "twp/task.h"
#include "twp/squirrel/squirrel.h"
#include "twp/yack.h"

namespace Twp {

TwpEngine *g_engine;

TwpEngine::TwpEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst),
	  _gameDescription(gameDesc),
	  _randomSource("Twp") {
	g_engine = this;
	sq_resetobject(&_defaultObj);
	_screenScene.setName("Screen");
	_scene.addChild(&_walkboxNode);
	_screenScene.addChild(&_pathNode);
	_screenScene.addChild(&_inputState);
	_screenScene.addChild(&_sentence);
	_screenScene.addChild(&_dialog);
	_screenScene.addChild(&_uiInv);
}

TwpEngine::~TwpEngine() {
	delete _screen;
}

static Math::Vector2d winToScreen(Math::Vector2d pos) {
	return Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
}

Math::Vector2d TwpEngine::roomToScreen(Math::Vector2d pos) {
	Math::Vector2d screenSize = _room->getScreenSize();
	return Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) * (pos - _gfx.cameraPos()) / screenSize;
}

Math::Vector2d TwpEngine::screenToRoom(Math::Vector2d pos) {
	Math::Vector2d screenSize = _room->getScreenSize();
	pos = Math::Vector2d(pos.getX(), SCREEN_HEIGHT - pos.getY());
	return (pos * screenSize) / Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT) + _gfx.cameraPos();
}

uint32 TwpEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TwpEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool TwpEngine::clickedAtHandled(Math::Vector2d roomPos) {
	bool result = false;
	int x = roomPos.getX();
	int y = roomPos.getY();
	if (sqrawexists(_room->_table, "clickedAt")) {
		debug("clickedAt %d, %d", x, y);
		sqcallfunc(result, _room->_table, "clickedAt", x, y);
	}
	if (!result) {
		if (_actor && sqrawexists(_actor->_table, "clickedAt")) {
			sqcallfunc(result, _actor->_table, "clickedAt", x, y);
		}
	}
	return result;
}

bool TwpEngine::preWalk(Object *actor, VerbId verbId, Object *noun1, Object *noun2) {
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
		debug("actorPreWalk %d n1=%s(%s) n2=%s", verbId.id, noun1->_name.c_str(), noun1->_key.c_str(), n2Name.c_str());
		sqcallfunc(result, actor->_table, "actorPreWalk", verbId.id, noun1->_table, n2Table);
	}
	if (!result) {
		Common::String funcName = isActor(noun1->getId()) ? "actorPreWalk" : "objectPreWalk";
		if (sqrawexists(noun1->_table, funcName)) {
			sqcallfunc(result, noun1->_table, funcName.c_str(), verbId.id, noun1->_table, n2Table);
			debug("%s %d n1=%s(%s) n2=%s -> %s", funcName.c_str(), verbId.id, noun1->_name.c_str(), noun1->_key.c_str(), n2Name.c_str(), result ? "yes" : "no");
		}
	}
	return result;
}

static bool verbNoWalkTo(VerbId verbId, Object *noun1) {
	if (verbId.id == VERB_LOOKAT)
		return (noun1->getFlags() & FAR_LOOK) != 0;
	return false;
}

// Called to execute a sentence and, if needed, start the actor walking.
// If `actor` is `null` then the selectedActor is assumed.
bool TwpEngine::execSentence(Object *actor, VerbId verbId, Object *noun1, Object *noun2) {
	Common::String name = !actor ? "currentActor" : actor->_key;
	Common::String noun1name = !noun1 ? "null" : noun1->_key;
	Common::String noun2name = !noun2 ? "null" : noun2->_key;
	debug("exec({name},{verbId.VerbId},{noun1name},{noun2name})");
	actor = !actor ? g_engine->_actor : actor;
	if ((verbId.id <= 0) || (verbId.id > 13) || (!noun1) || (!actor))
		return false;
	// TODO
	// if (a?._verb_tid) stopthread(actor._verb_tid)

	debug("noun1.inInventory: {noun1.inInventory} and noun1.touchable: {noun1.touchable} nowalk: {verbNoWalkTo(verbId, noun1)}");

	// test if object became untouchable
	if (!noun1->inInventory() && !noun1->_touchable)
		return false;
	if (noun2 && (!noun2->inInventory()) && (!noun2->_touchable))
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
			actor->turn(noun1);
			callVerb(actor, verbId, noun1, noun2);
			return true;
		}
	}

	actor->_exec.verb = verbId;
	actor->_exec.noun1 = noun1;
	actor->_exec.noun2 = noun2;
	actor->_exec.enabled = true;
	if (!noun1->inInventory())
		actor->walk(noun1);
	else
		actor->walk(noun2);
	return true;
}

void TwpEngine::clickedAt(Math::Vector2d scrPos) {
	// TODO: update this
	if (_room) {
		Math::Vector2d roomPos = screenToRoom(scrPos);
		Object *obj = objAt(roomPos);

		if (_cursor.isLeftDown()) {
			// button left: execute selected verb
			bool handled = clickedAtHandled(roomPos);
			if (!handled && obj) {
				Verb vb = verb();
				sqcall("onVerbClick");
				handled = execSentence(nullptr, vb.id, _noun1, _noun2);
			}
			if (!handled) {
				if (_actor && (scrPos.getY() > 172)) {
					_actor->walk(roomPos);
					_hud._verb = _hud.actorSlot(_actor)->verbs[0];
				}
			}
			// TODO: Just clicking on the ground
			//     cancelSentence(self.actor)
		} else if (_cursor.isRightDown()) {
			// button right: execute default verb
			if (obj) {
				VerbId verb;
				verb.id = obj->defaultVerbId();
				execSentence(nullptr, verb, _noun1, _noun2);
			}
		} else if (_walkFastState && _cursor.leftDown && _actor && (scrPos.getY() > 172)) {
			_actor->walk(roomPos);
		}
	}
}

Verb TwpEngine::verb() {
	Verb result = _hud._verb;
	if (result.id.id == VERB_WALKTO && _noun1 && _noun1->inInventory())
		result = _hud.actorSlot(_actor)->verbs[_noun1->defaultVerbId()];
	return result;
}

Common::String TwpEngine::cursorText() {
	Common::String result;
	if (_dialog.getState() == DialogState::None) {
		if (_hud.isVisible() && _hud._over) {
			return _hud._verb.id.id > 1 ? _textDb.getText(verb().text) : "";
		}

		// give can be used only on inventory and talkto to talkable objects (actors)
		result = !_noun1 || (_hud._verb.id.id == VERB_GIVE && !_noun1->inInventory()) || (_hud._verb.id.id == VERB_TALKTO && !(_noun1->getFlags() & TALKABLE)) ? "" : _textDb.getText(_noun1->_name);

		// add verb if not walk to or if noun1 is present
		if ((_hud._verb.id.id > 1) || (result.size() > 0)) {
			// if inventory, use default verb instead of walkto
			Common::String verbText = verb().text;
			result = result.size() > 0 ? Common::String::format("%s %s", _textDb.getText(verbText).c_str(), result.c_str()) : _textDb.getText(verbText);
			if (_useFlag == ufUseWith)
				result += " " + _textDb.getText(10000);
			else if (_useFlag == ufUseOn)
				result += " " + _textDb.getText(10001);
			else if (_useFlag == ufUseIn)
				result += " " + _textDb.getText(10002);
			else if (_useFlag == ufGiveTo)
				result += " " + _textDb.getText(10003);
			if (_noun2)
				result += " " + _textDb.getText(_noun2->_name);
		}
	}
	return result;
}

template<typename TFunc>
void objsAt(Math::Vector2d pos, TFunc func) {
	if (g_engine->_uiInv.getObject() && g_engine->_room->_fullscreen == FULLSCREENROOM)
		func(g_engine->_uiInv.getObject());
	for (int i = 0; i < g_engine->_room->_layers.size(); i++) {
		Layer *layer = g_engine->_room->_layers[i];
		for (int j = 0; j < layer->_objects.size(); j++) {
			Object *obj = layer->_objects[j];
			if (obj != g_engine->_actor && (obj->_touchable || obj->inInventory()) && obj->_node->isVisible() && obj->_objType == otNone && obj->contains(pos))
				if (func(obj))
					return;
		}
	}
}

Object *inventoryAt(Math::Vector2d pos) {
	Object *result = nullptr;
	objsAt(pos, [&](Object *x) {
		if (x->inInventory()) {
			result = x;
			return true;
		}
		return false;
	});
	return result;
}

void TwpEngine::update(float elapsed) {
	_time += elapsed;
	_frameCounter++;

	// update mouse pos
	Math::Vector2d scrPos = winToScreen(_cursor.pos);
	_inputState.setVisible(_inputState.getShowCursor() || _dialog.getState() == WaitingForChoice);
	_inputState.setPos(scrPos);
	_sentence.setPos(scrPos);
	_dialog.setMousePos(scrPos);

	if (_room) {
		// update nouns and useFlag
		Math::Vector2d roomPos = screenToRoom(scrPos);
		if (_room->_fullscreen == FULLSCREENROOM) {
			if ((_hud._verb.id.id == VERB_USE) && (_useFlag != ufNone)) {
				_noun2 = objAt(roomPos);
			} else if (_hud._verb.id.id == VERB_GIVE) {
				if (_useFlag != ufGiveTo) {
					_noun1 = inventoryAt(roomPos);
					_useFlag = ufNone;
					_noun2 = nullptr;
				} else {
					objsAt(roomPos, [&](Object *x) {
						if (x != _actor && x->getFlags() & GIVEABLE) {
							_noun2 = x;
							return true;
						}
						return false;
					});
					if (_noun2)
						debug("Give '%s' to '%s'", _noun1->_key.c_str(), _noun2->_key.c_str());
				}
			} else {
				_noun1 = objAt(roomPos);
				_useFlag = ufNone;
				_noun2 = nullptr;
			}

			_inputState.setHotspot(_noun1 != nullptr);
			_hud.setVisible(_inputState.getInputActive() && _inputState.getInputVerbsActive() && _dialog.getState() == DialogState::None);
			_sentence.setVisible(_hud.isVisible());
			_uiInv.setVisible(_hud.isVisible() && !_cutscene);
			//_actorSwitcher.visible = _dialog.state == DialogState.None and self.cutscene.isNil;
			_sentence.setText(cursorText());

			// call clickedAt if any button down
			if ((_dialog.getState() == DialogState::None) && !_hud.isOver()) {
				// TODO:
				// if (_cursor.leftDown) {
				// 	if mouseDnDur > initDuration(milliseconds = 500):
				// 		walkFast();
				// } else {
				// 	walkFast(false);
				// }
				if (_cursor.leftDown || _cursor.rightDown)
					clickedAt(scrPos);
			}
		} else {
			_hud.setVisible(false);
			_uiInv.setVisible(false);
			_noun1 = objAt(roomPos);
			Common::String cText = !_noun1 ? "" : _textDb.getText(_noun1->_name);
			_sentence.setText(cText);
			// TODO: _inputState.setCursorShape(CursorShape::Normal);
			if (_cursor.leftDown)
				clickedAt(scrPos);
		}

		if (_cursor.leftDown || _cursor.rightDown)
			clickedAt(_cursor.pos);
	}

	_dialog.update(elapsed);
	_fadeShader->_elapsed += elapsed;

	// update camera
	_camera.update(_room, _followActor, elapsed);

	// update cutscene
	if (_cutscene) {
		if (_cutscene->update(elapsed)) {
			delete _cutscene;
			_cutscene = nullptr;
		}
	}

	// update threads: make a copy of the threads because during threads update, new threads can be added
	Common::Array<ThreadBase *> threads(_threads);
	Common::Array<ThreadBase *> threadsToRemove;

	for (auto it = threads.begin(); it != threads.end(); it++) {
		ThreadBase *thread = *it;
		if (thread->update(elapsed)) {
			threadsToRemove.push_back(thread);
		}
	}

	// remove threads that are terminated
	for (auto it = _threads.begin(); it != _threads.end();) {
		ThreadBase *thread = *it;
		if (find(threadsToRemove, thread) != -1) {
			it = _threads.erase(it);
			delete thread;
			continue;
		}
		it++;
	}

	// update callbacks
	for (auto it = _callbacks.begin(); it != _callbacks.end();) {
		Callback *cb = *it;
		if (cb->update(elapsed)) {
			it = _callbacks.erase(it);
			delete cb;
			continue;
		}
		it++;
	}

	// update tasks
	for (auto it = _tasks.begin(); it != _tasks.end();) {
		Task *task = *it;
		if (task->update(elapsed)) {
			it = _tasks.erase(it);
			delete task;
			continue;
		}
		it++;
	}

	// update objects
	if (_room) {
		_room->update(elapsed);
	}

	// update inventory
	if (!_actor) {
		_uiInv.update(elapsed);
	} else {
		_hud.update(scrPos, _noun1, _cursor.isLeftDown());
		VerbUiColors *verbUI = &_hud.actorSlot(_actor)->verbUiColors;
		_uiInv.update(elapsed, _actor, verbUI->inventoryBackground, verbUI->verbNormal);
	}

	updateTriggers();
}

void TwpEngine::setShaderEffect(RoomEffect effect) {
	_shaderParams.effect = effect;
	//   switch (effect) {
	//   case RoomEffect::None:
	//     _gfx.use(nullptr);
	// 	break;
	//   case RoomEffect::Sepia:
	//     let shader = newShader(vertexShader, sepiaShader);
	//     gfxShader(shader);
	//     shader.setUniform("sepiaFlicker", _shaderParams.sepiaFlicker);
	// 	break;
	//   case RoomEffect::BlackAndWhite:
	//     gfxShader(newShader(vertexShader, bwShader));
	// 	break;
	//   case RoomEffect::Ega:
	//     gfxShader(newShader(vertexShader, egaShader));
	// 	break;
	//   case RoomEffect::Vhs:
	//     gfxShader(newShader(vertexShader, vhsShader));
	// 	break;
	//   case RoomEffect::Ghost:
	//     let shader = newShader(vertexShader, ghostShader);
	//     gfxShader(shader);
	// 	break;
	//   }
}

void TwpEngine::draw() {
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
	_scene.draw();

	// then render this texture with room effect to another texture
	_gfx.setRenderTarget(&renderTexture2);
	// setShaderEffect(_room->_effect);
	// _shaderParams.randomValue[0] = g_engine.rand.rand(0f..1f);
	// _shaderParams.timeLapse = fmodf(_time, 1000.f);
	// _shaderParams.iGlobalTime = _shaderParams.timeLapse;
	// _shaderParams.updateShader();

	_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	bool flipY = _fadeShader->_effect == FadeEffect::Wobble;
	Math::Vector2d camPos = _gfx.cameraPos();
	_gfx.drawSprite(renderTexture, Color(), Math::Matrix4(), false, flipY);

	Texture *screenTexture = &renderTexture2;
	if (_fadeShader->_effect != FadeEffect::None) {
		// draw second room if any
		_gfx.setRenderTarget(&renderTexture);
		_gfx.use(nullptr);
		_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
		_gfx.cameraPos(_fadeShader->_cameraPos);
		_gfx.clear(Color(0, 0, 0));
		if (_fadeShader->_effect == FadeEffect::Wobble) {
			Math::Vector2d camSize = _fadeShader->_room->getScreenSize();
			_gfx.camera(camSize);
			_fadeShader->_room->_scene->draw();
		}

		_fadeShader->_fade = clamp(_fadeShader->_elapsed / _fadeShader->_duration, 0.f, 1.f);

		// draw fade
		Texture *texture1 = nullptr;
		Texture *texture2 = nullptr;
		switch (_fadeShader->_effect) {
		case FadeEffect::Wobble:
			texture1 = &renderTexture;
			texture2 = &renderTexture2;
			screenTexture = &renderTexture;
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
	_gfx.setRenderTarget(nullptr);
	_gfx.camera(Math::Vector2d(SCREEN_WIDTH, SCREEN_HEIGHT));
	_gfx.drawSprite(*screenTexture, Color(), Math::Matrix4(), false, _fadeShader->_effect != FadeEffect::None);

	// draw UI
	_gfx.cameraPos(camPos);
	_screenScene.draw();

	g_system->updateScreen();
}

Common::Error TwpEngine::run() {
	initGraphics3d(SCREEN_WIDTH, SCREEN_HEIGHT);
	_screen = new Graphics::Screen(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Set the engine's debugger console
	setDebugger(new Console());

	_gfx.init();
	_fadeShader.reset(new FadeShader());

	_lighting = new Lighting();

	_pack.init();

	// TODO: load with selected lang
	GGPackEntryReader entry;
	entry.open(_pack, "ThimbleweedText_en.tsv");
	_textDb.parseTsv(entry);

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	HSQUIRRELVM v = _vm.get();
	execNutEntry(v, "Defines.nut");
	execBnutEntry(v, "Boot.bnut");

	// const SQChar *code = "cameraInRoom(StartScreen)";
	const SQChar *code = "start(1)";

	_vm.exec(code);

	// Simple event handling loop
	Common::Event e;
	uint time = _system->getMillis();
	while (!shouldQuit()) {
		const int dx = 4;
		const int dy = 4;
		Math::Vector2d camPos = _gfx.cameraPos();
		while (g_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_KEYDOWN:
				switch (e.kbd.keycode) {
				case Common::KEYCODE_LEFT:
					camPos.setX(camPos.getX() - dx);
					break;
				case Common::KEYCODE_RIGHT:
					camPos.setX(camPos.getX() + dx);
					break;
				case Common::KEYCODE_UP:
					camPos.setY(camPos.getY() + dy);
					break;
				case Common::KEYCODE_DOWN:
					camPos.setY(camPos.getY() - dy);
					break;
				default:
					break;
				}
			case Common::EVENT_MOUSEMOVE:
				_cursor.pos = Math::Vector2d(e.mouse.x, e.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				_cursor.leftDown = true;
				if (!_cursor.oldLeftDown) {
					_cursor.mouseDownTime = g_engine->_time;
				}
				break;
			case Common::EVENT_LBUTTONUP:
				_cursor.leftDown = false;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_cursor.rightDown = true;
				break;
			case Common::EVENT_RBUTTONUP:
				_cursor.rightDown = false;
				break;
			default:
				break;
			}
		}

		_gfx.cameraPos(camPos);

		uint32 newTime = _system->getMillis();
		uint32 delta = newTime - time;
		time = newTime;
		update(8.f * delta / 1000.f);
		draw();
		_cursor.update();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

Common::Error TwpEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

Room *TwpEngine::defineRoom(const Common::String &name, HSQOBJECT table, bool pseudo) {
	HSQUIRRELVM v = _vm.get();
	debug("Load room: %s", name.c_str());
	Room *result;
	if (name == "Void") {
		result = new Room(name, table);
		result->_scene = new Scene();
		Layer *layer = new Layer("background", Math::Vector2d(1.f, 1.f), 0);
		layer->_node = new ParallaxNode(Math::Vector2d(1.f, 1.f), "", Common::StringArray());
		result->_layers.push_back(layer);
		result->_scene->addChild(layer->_node);
		sqsetf(sqrootTbl(v), name, result->_table);
	} else {
		result = new Room(name, table);
		Common::String background;
		sqgetf(table, "background", background);
		GGPackEntryReader entry;
		entry.open(_pack, background + ".wimpy");
		result->load(entry);
		result->_name = name;
		result->_pseudo = pseudo;
		for (int i = 0; i < result->_layers.size(); i++) {
			Layer *layer = result->_layers[i];
			// create layer node
			ParallaxNode *layerNode = new ParallaxNode(layer->_parallax, result->_sheet, layer->_names);
			layerNode->setZSort(layer->_zsort);
			layerNode->setName(Common::String::format("Layer %s(%d)", layer->_names[0].c_str(), layer->_zsort));
			layer->_node = layerNode;
			result->_scene->addChild(layerNode);

			for (int j = 0; j < layer->_objects.size(); j++) {
				Object *obj = layer->_objects[j];
				if (!sqrawexists(table, obj->_key)) {
					// this object does not exist, so create it
					sq_newtable(v);
					sq_getstackobj(v, -1, &obj->_table);
					sq_addref(v, &obj->_table);
					sq_pop(v, 1);

					// assign an id
					setId(obj->_table, newObjId());
					// info fmt"Create object with new table: {obj.name} #{obj.id}"

					// adds the object to the room table
					sqsetf(result->_table, obj->_key, obj->_table);
					obj->setRoom(result);
					obj->setState(0, true);

					if (obj->_objType == otNone)
						obj->_touchable = false;
				} else if (obj->_objType == otNone) {
					obj->_touchable = true;
				}

				layerNode->addChild(obj->_node);
			}
		}
	}

	// assign parent node
	for (int i = 0; i < result->_layers.size(); i++) {
		Layer *layer = result->_layers[i];
		for (int j = 0; j < layer->_objects.size(); j++) {
			Object *obj = layer->_objects[j];
			if (obj->_parent.size() > 0) {
				Object *parent = result->getObj(obj->_parent);
				if (!parent) {
					warning("parent: '%s' not found", obj->_parent.c_str());
				} else {
					parent->_node->addChild(obj->_node);
				}
			}
		}
	}

	sqgetpairs(result->_table, [&](const Common::String &k, HSQOBJECT &oTable) {
		if (oTable._type == OT_TABLE) {
			if (pseudo) {
				// if it's a pseudo room we need to clone each object
				sq_pushobject(v, oTable);
				sq_clone(v, -1);
				sq_getstackobj(v, -1, &oTable);
				sq_addref(v, &oTable);
				sq_pop(v, 2);
				sqsetf(result->_table, k, oTable);
			}

			if (sqrawexists(oTable, "icon")) {
				// Add inventory object to root table
				debug("Add %s to inventory", k.c_str());
				sqsetf(sqrootTbl(v), k, oTable);

				// set room as delegate
				sqsetdelegate(oTable, table);

				// declare flags if does not exist
				if (!sqrawexists(oTable, "flags"))
					sqsetf(oTable, "flags", 0);
				Object *obj = new Object(oTable, k);
				setId(obj->_table, newObjId());
				obj->_node = new Node(k);
				obj->_nodeAnim = new Anim(obj);
				obj->_node->addChild(obj->_nodeAnim);
				obj->setRoom(result);
				// set room as delegate
				sqsetdelegate(obj->_table, table);
			} else {
				Object *obj = result->getObj(k);
				if (!obj) {
					debug("object: %s not found in wimpy", k.c_str());
					if (sqrawexists(oTable, "name")) {
						obj = new Object();
						obj->_key = k;
						obj->_layer = result->layer(0);
						result->layer(0)->_objects.push_back(obj);
					} else {
						return;
					}
				}

				sqgetf(result->_table, k, obj->_table);
				setId(obj->_table, newObjId());
				debug("Create object: %s #%d", k.c_str(), obj->getId());

				// add it to the root table if not a pseudo room
				if (!pseudo)
					sqsetf(sqrootTbl(v), k, obj->_table);

				if (sqrawexists(obj->_table, "initState")) {
					// info fmt"initState {obj.key}"
					int state;
					sqgetf(obj->_table, "initState", state);
					obj->setState(state, true);
				} else {
					obj->setState(0, true);
				}
				obj->setRoom(result);

				// set room as delegate
				sqsetdelegate(obj->_table, table);

				// declare flags if does not exist
				if (!sqrawexists(obj->_table, "flags"))
					sqsetf(obj->_table, "flags", 0);
			}
		}
	});

	// declare the room in the root table
	setId(result->_table, newRoomId());
	sqsetf(sqrootTbl(v), name, result->_table);

	return result;
}

void TwpEngine::enterRoom(Room *room, Object *door) {
	HSQUIRRELVM v = getVm();
	// Called when the room is entered.
	debug("call enter room function of %s", room->_name.c_str());

	// exit current room
	exitRoom(_room);
	_fadeShader->_effect = FadeEffect::None;

	// sets the current room for scripts
	sqsetf(sqrootTbl(v), "currentRoom", room->_table);

	if (_room)
		_room->_scene->remove();
	_room = room;
	_scene.addChild(_room->_scene);
	_room->_lights._numLights = 0;
	_room->setOverlay(Color(0.f, 0.f, 0.f, 0.f));
	_camera.setBounds(Rectf::fromMinMax(Math::Vector2d(), _room->_roomSize));
	if (_actor)
		_hud._verb = _hud.actorSlot(_actor)->verbs[0];

	// move current actor to the new room
	Math::Vector2d camPos;
	if (_actor) {
		cancelSentence();
		if (door) {
			Facing facing = getOppositeFacing(door->getDoorFacing());
			_actor->_room = room;
			if (door) {
				_actor->setFacing(facing);
				_actor->_node->setPos(door->getUsePos());
			}
			camPos = _actor->_node->getPos();
		}
	}

	_camera.setRoom(room);
	_camera.setAt(camPos);

	// call actor enter function and objects enter function
	actorEnter();
	for (int i = 0; i < room->_layers.size(); i++) {
		Layer *layer = room->_layers[i];
		for (int j = 0; j < layer->_objects.size(); j++) {
			Object *obj = layer->_objects[i];
			if (sqrawexists(obj->_table, "enter"))
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

void TwpEngine::actorEnter() {
	if (_actor)
		sqcall(_actor->_table, "actorEnter");
	if (_room) {
		if (sqrawexists(_room->_table, "actorEnter")) {
			sqcall(_room->_table, "actorEnter", _actor->_table);
		}
	}
}

void TwpEngine::exitRoom(Room *nextRoom) {
	HSQUIRRELVM v = getVm();
	// TODO: _audio.stopAll()
	if (_room) {
		_room->_triggers.clear();

		actorExit();

		// call room exit function with the next room as a parameter if requested
		int nparams = sqparamCount(v, _room->_table, "exit");
		if (nparams == 2) {
			sqcall(_room->_table, "exit", nextRoom->_table);
		} else {
			sqcall(_room->_table, "exit");
		}

		// delete all temporary objects
		for (int i = 0; i < _room->_layers.size(); i++) {
			Layer *layer = _room->_layers[i];
			for (int j = 0; j < _room->_layers.size(); j++) {
				Object *obj = layer->_objects[i];
				if (obj->_temporary) {
					delete obj;
				} else if (isActor(obj->getId()) && _actor != obj) {
					obj->stopObjectMotors();
				}
			}
		}

		// call global function exitedRoom with the room as argument
		sqcall("exitedRoom", _room->_table);

		// stop all local threads
		for (int i = 0; i < _threads.size(); i++) {
			ThreadBase *thread = _threads[i];
			if (!thread->isGlobal()) {
				thread->stop();
			}
		}

		// stop all lights
		_room->_lights._numLights = 0;
	}
}

void TwpEngine::setRoom(Room *room) {
	if (room && _room != room)
		enterRoom(room);
}

void TwpEngine::actorExit() {
	if (!_actor && _room) {
		if (sqrawexists(_room->_table, "actorExit")) {
			sqcall(_room->_table, "actorExit", _actor->_table);
		}
	}
}

void TwpEngine::cancelSentence(Object *actor) {
	debug("cancelSentence");
	if (!actor)
		actor = _actor;
	if (actor)
		actor->_exec.enabled = false;
}

void TwpEngine::execBnutEntry(HSQUIRRELVM v, const Common::String &entry) {
	GGPackEntryReader reader;
	reader.open(_pack, entry);
	GGBnutReader nut;
	nut.open(&reader);
	Common::String code = nut.readString();
	sqexec(v, code.c_str(), entry.c_str());
}

void TwpEngine::execNutEntry(HSQUIRRELVM v, const Common::String &entry) {
	if (_pack.assetExists(entry.c_str())) {
		GGPackEntryReader reader;
		debug("read existing '%s'", entry.c_str());
		reader.open(_pack, entry);
		Common::String code = reader.readString();
		// debug("%s", code.c_str());
		sqexec(v, code.c_str(), entry.c_str());
	} else {
		Common::String newEntry = entry.substr(0, entry.size() - 4) + ".bnut";
		debug("read existing '%s'", newEntry.c_str());
		if (_pack.assetExists(newEntry.c_str())) {
			execBnutEntry(v, newEntry);
		} else {
			error("'%s' and '%s' have not been found", entry.c_str(), newEntry.c_str());
		}
	}
}

void TwpEngine::cameraAt(Math::Vector2d at) {
	_camera.setRoom(_room);
	_camera.setAt(at);
}

Math::Vector2d TwpEngine::cameraPos() {
	if (_room) {
		Math::Vector2d screenSize = _room->getScreenSize();
		return _camera.getAt() + screenSize / 2.0f;
	}
	return _camera.getAt();
}

void TwpEngine::follow(Object *actor) {
	_followActor = actor;
	if (actor) {
		Math::Vector2d pos = actor->_node->getPos();
		Room *oldRoom = _room;
		setRoom(actor->_room);
		if (oldRoom != actor->_room)
			cameraAt(pos);
	}
}

void TwpEngine::fadeTo(FadeEffect effect, float duration, bool fadeToSep) {
	_fadeShader->_fadeToSepia = fadeToSep;
	_fadeShader->_effect = effect;
	_fadeShader->_room = _room;
	_fadeShader->_cameraPos = cameraPos();
	_fadeShader->_duration = duration;
	_fadeShader->_movement = effect == FadeEffect::Wobble ? 0.005f : 0.f;
	_fadeShader->_elapsed = 0.f;
}

Object *TwpEngine::objAt(Math::Vector2d pos) {
	int zOrder = INT_MAX;
	Object *result = nullptr;
	objsAt(pos, [&](Object *obj) {
		if (obj->_node->getZSort() < zOrder) {
			result = obj;
			zOrder = obj->_node->getZSort();
		}
		return false;
	});
	return result;
}

void TwpEngine::setActor(Object *actor, bool userSelected) {
	_actor = actor;
	_hud._actor = actor;
	if (!_hud.getParent() && actor) {
		_screenScene.addChild(&_hud);
	} else if (_hud.getParent() && !actor) {
		_screenScene.removeChild(&_hud);
	}

	// call onActorSelected callbacks
	sqcall("onActorSelected", actor->_table, userSelected);
	Room *room = !actor ? nullptr : actor->_room;
	if (room) {
		if (sqrawexists(room->_table, "onActorSelected")) {
			sqcall(room->_table, "onActorSelected", actor->_table, userSelected);
		}
	}

	if (actor)
		follow(actor);
}

bool TwpEngine::selectable(Object *actor) {
	for (int i = 0; i < NUMACTORS; i++) {
		ActorSlot *slot = &_hud._actorSlots[i];
		if (slot->actor == actor)
			return slot->selectable;
	}
	return false;
}

static void giveTo(Object *actor1, Object *actor2, Object *obj) {
	obj->_owner = actor2;
	actor2->_inventory.push_back(obj);
	int index = find(actor1->_inventory, obj);
	if (index != -1)
		actor1->_inventory.remove_at(index);
}

void TwpEngine::resetVerb() {
	debug("reset nouns");
	_noun1 = nullptr;
	_noun2 = nullptr;
	_useFlag = ufNone;
	_hud._verb = _hud.actorSlot(_actor)->verbs[0];
}

bool TwpEngine::callVerb(Object *actor, VerbId verbId, Object *noun1, Object *noun2) {
	sqcall("onObjectClick", noun1->_table);

	// Called after the actor has walked to the object.
	Common::String name = !actor ? "currentActor" : actor->_key;
	Common::String noun1name = !noun1 ? "null" : noun1->_key;
	Common::String noun2name = !noun2 ? "null" : noun2->_key;
	Common::String verbFuncName = _hud.actorSlot(actor)->verbs[verbId.id].fun;
	debug("callVerb(%s,%s,%s,%s)", name.c_str(), verbFuncName.c_str(), noun1name.c_str(), noun2name.c_str());

	// test if object became untouchable
	if (!noun1->inInventory() && !noun1->_touchable)
		return false;
	if (noun2 && !noun2->inInventory() && !noun2->_touchable)
		return false;

	// check if verb is use and object can be used with or in or on
	if ((verbId.id == VERB_USE) && !noun2) {
		_useFlag = noun1->useFlag();
		if (_useFlag != ufNone) {
			_noun1 = noun1;
			return false;
		}
	}

	if (verbId.id == VERB_GIVE) {
		if (!noun2) {
			debug("set use flag to ufGiveTo");
			_useFlag = ufGiveTo;
			_noun1 = noun1;
		} else {
			bool handled = false;
			if (sqrawexists(noun2->_table, verbFuncName)) {
				debug("call %s on %s", verbFuncName.c_str(), noun2->_key.c_str());
				sqcallfunc(handled, noun2->_table, verbFuncName.c_str(), noun1->_table);
			}
			// verbGive is called on object only for non selectable actors
			if (!handled && !selectable(noun2) && sqrawexists(noun1->_table, verbFuncName)) {
				debug("call %s on %s", verbFuncName.c_str(), noun1->_key.c_str());
				sqcall(noun1->_table, verbFuncName.c_str(), noun2->_table);
				handled = true;
			}
			if (!handled) {
				debug("call objectGive");
				sqcall("objectGive", noun1->_table, _actor->_table, noun2->_table);
				giveTo(_actor, noun2, noun1);
			}
			resetVerb();
		}
		return false;
	}

	if (!noun2) {
		if (sqrawexists(noun1->_table, verbFuncName)) {
			int count = sqparamCount(getVm(), noun1->_table, verbFuncName);
			debug("call %s.%s", noun1->_key.c_str(), verbFuncName.c_str());
			if (count == 1) {
				sqcall(noun1->_table, verbFuncName.c_str());
			} else {
				sqcall(noun1->_table, verbFuncName.c_str(), actor->_table);
			}
		} else if (sqrawexists(noun1->_table, VERBDEFAULT)) {
			sqcall(noun1->_table, VERBDEFAULT);
		} else {
			debug("call defaultObject.%s", verbFuncName.c_str());
			sqcall(_defaultObj, verbFuncName.c_str(), noun1->_table, actor->_table);
		}
	} else {
		if (sqrawexists(noun1->_table, verbFuncName)) {
			debug("call %s.%s", noun1->_key.c_str(), verbFuncName.c_str());
			sqcall(noun1->_table, verbFuncName.c_str(), noun2->_table);
		} else if (sqrawexists(noun1->_table, VERBDEFAULT)) {
			sqcall(noun1->_table, VERBDEFAULT);
		} else {
			debug("call defaultObject.%s", verbFuncName.c_str());
			sqcall(_defaultObj, verbFuncName.c_str(), noun1->_table, noun2->_table);
		}
	}

	if (verbId.id == VERB_PICKUP) {
		sqcall("onPickup", noun1->_table, actor->_table);
	}

	resetVerb();
	return false;
}

void TwpEngine::callTrigger(Object *obj, HSQOBJECT trigger) {
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

		int id = newThreadId();
		Thread *thread = new Thread(id);
		thread->setName("Trigger");
		thread->_global = false;
		thread->_threadObj = threadObj;
		thread->_envObj = obj->_table;
		thread->_closureObj = trigger;
		if(nParams == 2) {
			thread->_args.push_back(_actor->_table);
		}
		debug("create triggerthread id: %d}", thread->getId());
		g_engine->_threads.push_back(thread);

		// call the closure in the thread
		if (!thread->call()) {
			error("trigger call failed");
		}
	}
}

void TwpEngine::updateTriggers() {
	if (_actor) {
		for (int i = 0; i < _room->_triggers.size(); i++) {
			Object *trigger = _room->_triggers[i];
			if (!trigger->_triggerActive && trigger->contains(_actor->_node->getAbsPos())) {
				debug("call enter trigger %s", trigger->_name.c_str());
				trigger->_triggerActive = true;
				callTrigger(trigger, trigger->_enter);
			} else if (trigger->_triggerActive && !trigger->contains(_actor->_node->getAbsPos())) {
				debug("call leave trigger %s", trigger->_name.c_str());
				trigger->_triggerActive = false;
				callTrigger(trigger, trigger->_leave);
			}
		}
	}
}

} // End of namespace Twp
