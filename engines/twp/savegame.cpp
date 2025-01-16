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

#include "common/btea.h"
#include "common/debug-channels.h"
#include "common/savefile.h"
#include "twp/callback.h"
#include "twp/detection.h"
#include "twp/dialog.h"
#include "twp/ggpack.h"
#include "twp/hud.h"
#include "twp/object.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/savegame.h"
#include "twp/squtil.h"
#include "twp/time.h"

namespace Twp {

const static uint32 savegameKey[] = {0xAEA4EDF3, 0xAFF8332A, 0xB5A2DBB4, 0x9B4BA022};

static Common::SharedPtr<Object> actor(const Common::String &key) {
	for (size_t i = 0; i < g_twp->_actors.size(); i++) {
		Common::SharedPtr<Object> a = g_twp->_actors[i];
		if (a->_key == key)
			return a;
	}
	return nullptr;
}

static Common::SharedPtr<Object> actor(int id) {
	for (size_t i = 0; i < g_twp->_actors.size(); i++) {
		Common::SharedPtr<Object> a = g_twp->_actors[i];
		if (a->getId() == id)
			return a;
	}
	return nullptr;
}

static DialogConditionMode parseMode(char mode) {
	switch (mode) {
	case '?':
		return Once;
	case '#':
		return ShowOnce;
	case '&':
		return OnceEver;
	case '$':
		return ShowOnceEver;
	case '^':
		return TempOnce;
	default:
		warning("Invalid dialog condition mode: %c", mode);
		return TempOnce;
	}
}

static DialogConditionState parseState(Common::String &dialog) {
	Common::String dialogName;
	size_t i = 1;
	while (i < dialog.size() && !Common::isDigit(dialog[i])) {
		dialogName += dialog[i];
		i++;
	}

	while (!g_twp->_pack->assetExists((dialogName + ".byack").c_str()) && (i < dialog.size())) {
		dialogName += dialog[i];
		i++;
	}

	Common::String num;
	while ((i < dialog.size()) && Common::isDigit(dialog[i])) {
		num += dialog[i];
		i++;
	}

	DialogConditionState result;
	result.mode = parseMode(dialog[0]);
	result.dialog = Common::move(dialogName);
	result.line = atol(num.c_str());
	result.actorKey = dialog.substr(i);
	return result;
}

static Common::SharedPtr<Room> room(const Common::String &name) {
	for (size_t i = 0; i < g_twp->_rooms.size(); i++) {
		Common::SharedPtr<Room> r(g_twp->_rooms[i]);
		if (r->_name == name) {
			return r;
		}
	}
	return nullptr;
}

static Common::SharedPtr<Object> object(Common::SharedPtr<Room> room, const Common::String &key) {
	for (size_t i = 0; i < room->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = room->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> o = layer->_objects[j];
			if (o->_key == key)
				return o;
		}
	}
	return nullptr;
}

static Common::SharedPtr<Object> object(const Common::String &key) {
	for (size_t i = 0; i < g_twp->_rooms.size(); i++) {
		Common::SharedPtr<Room> r(g_twp->_rooms[i]);
		for (size_t j = 0; j < r->_layers.size(); j++) {
			Common::SharedPtr<Layer> layer(r->_layers[j]);
			for (size_t k = 0; k < layer->_objects.size(); k++) {
				Common::SharedPtr<Object> o(layer->_objects[k]);
				if (o->_key == key)
					return o;
			}
		}
	}
	return nullptr;
}

static SQRESULT toSquirrel(const Common::JSONValue *json, HSQOBJECT &obj) {
	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	sq_resetobject(&obj);
	if (json->isString()) {
		sqpush(v, json->asString());
		if (SQ_FAILED(sqget(v, -1, obj)))
			return sq_throwerror(v, "failed to get string");
	} else if (json->isIntegerNumber()) {
		sqpush(v, static_cast<int>(json->asIntegerNumber()));
		if (SQ_FAILED(sqget(v, -1, obj)))
			return sq_throwerror(v, "failed to get int");
	} else if (json->isBool()) {
		sqpush(v, json->asBool());
		if (SQ_FAILED(sqget(v, -1, obj)))
			return sq_throwerror(v, "failed to get bool");
	} else if (json->isNumber()) {
		sqpush(v, json->asNumber());
		if (SQ_FAILED(sqget(v, -1, obj)))
			return sq_throwerror(v, "failed to get float");
	} else if (json->isNull()) {
	} else if (json->isArray()) {
		sq_newarray(v, 0);
		const Common::JSONArray &jArr = json->asArray();
		for (size_t i = 0; i < jArr.size(); i++) {
			HSQOBJECT tmp;
			if (SQ_FAILED(toSquirrel(jArr[i], tmp)))
				return sq_throwerror(v, "failed to get array element");
			sqpush(v, tmp);
			sq_arrayappend(v, -2);
		}
		if (SQ_FAILED(sqget(v, -1, obj)))
			return sq_throwerror(v, "failed to get array");
	} else if (json->isObject()) {
		const Common::JSONObject &jObject = json->asObject();
		// check if it's a reference to an actor
		if (jObject.contains("_actorKey")) {
			obj = actor(jObject["_actorKey"]->asString())->_table;
		} else if (jObject.contains("_roomKey")) {
			Common::String roomName = jObject["_roomKey"]->asString();
			if (jObject.contains("_objectKey")) {
				Common::String objName = jObject["_objectKey"]->asString();
				Common::SharedPtr<Room> r = room(roomName);
				if (!r)
					warning("room with key=%s not found", roomName.c_str());
				else {
					Common::SharedPtr<Object> o = object(r, objName);
					if (!o)
						warning("room object with key=%s/%s not found", roomName.c_str(), objName.c_str());
					else
						obj = o->_table;
				}
			} else {
				Common::SharedPtr<Room> r = room(roomName);
				if (!r) {
					warning("room with key=%s not found", roomName.c_str());
				} else {
					obj = r->_table;
				}
			}
		} else if (jObject.contains("_objectKey")) {
			Common::String objName = jObject["_objectKey"]->asString();
			Common::SharedPtr<Object> o = object(objName);
			if (!o) {
				warning("object with key=%s not found", objName.c_str());
			} else {
				obj = o->_table;
			}
		} else {
			sq_newtable(v);
			for (auto it = jObject.begin(); it != jObject.end(); it++) {
				sqpush(v, it->_key);
				HSQOBJECT tmp;
				if (SQ_FAILED(toSquirrel(it->_value, tmp)))
					return sq_throwerror(v, "failed to get table element");
				sqpush(v, tmp);
				sq_newslot(v, -3, SQFalse);
			}
			if (SQ_FAILED(sqget(v, -1, obj)))
				return sq_throwerror(v, "failed to get table");
		}
	}
	sq_addref(v, &obj);
	sq_settop(v, top);
	return SQ_OK;
}

static Common::String sub(const Common::String &s, size_t pos, size_t end) {
	return s.substr(pos, s.size() - end);
}

static void setAnimations(Common::SharedPtr<Object> actor, const Common::JSONArray &anims) {
	Common::String headAnim = anims[0]->asString();
	Common::String standAnim = sub(anims[9]->asString(), 0, 7);
	Common::String walkAnim = sub(anims[11]->asString(), 0, 7);
	Common::String reachAnim = sub(anims[15]->asString(), 0, 11);
	actor->setAnimationNames(headAnim, standAnim, walkAnim, reachAnim);
}

static SQRESULT loadActor(Common::SharedPtr<Object> actor, const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	actor->setTouchable(!json.contains("_untouchable") || json["_untouchable"]->asIntegerNumber() != 1);
	actor->_node->setVisible(!json.contains("_hidden") || json["_hidden"]->asIntegerNumber() != 1);
	actor->_volume = json.contains("_volume") ? json["_volume"]->asNumber() : 1.0f;
	for (auto it = json.begin(); it != json.end(); it++) {
		if (it->_key == "_animations") {
			setAnimations(actor, it->_value->asArray());
		} else if (it->_key == "_pos") {
			actor->_node->setPos(parseVec2(it->_value->asString()));
		} else if (it->_key == "_costume") {
			Common::String sheet;
			if (json.contains("_costumeSheet"))
				sheet = json["_costumeSheet"]->asString();
			actor->setCostume(it->_value->asString(), sheet);
		} else if (it->_key == "_costumeSheet") {
		} else if (it->_key == "_color") {
			actor->_node->setColor(Color::rgb(it->_value->asIntegerNumber()));
			actor->_node->setAlpha(Color::fromRgba(it->_value->asIntegerNumber()).rgba.a);
		} else if (it->_key == "_dir") {
			actor->setFacing((Facing)it->_value->asIntegerNumber());
		} else if (it->_key == "_lockFacing") {
			actor->lockFacing(it->_value->asIntegerNumber());
		} else if (it->_key == "_useDir") {
			actor->_useDir = (Direction)it->_value->asIntegerNumber();
		} else if (it->_key == "_usePos") {
			actor->_usePos = parseVec2(it->_value->asString());
		} else if (it->_key == "_offset") {
			actor->_node->setOffset(parseVec2(it->_value->asString()));
		} else if (it->_key == "_renderOffset") {
			actor->_node->setRenderOffset(parseVec2(it->_value->asString()));
		} else if (it->_key == "_roomKey") {
			Object::setRoom(actor, room(it->_value->asString()));
		} else if ((it->_key == "_hidden") || (it->_key == "_untouchable") || (it->_key == "_volume")) {
		} else {
			if (!it->_key.hasPrefix("_")) {
				HSQOBJECT tmp;
				if (SQ_FAILED(toSquirrel(it->_value, tmp)))
					return sq_throwerror(v, "failed to get table object");
				if (sqrawexists(actor->_table, it->_key))
					sqsetf(actor->_table, it->_key, tmp);
				else
					sqnewf(actor->_table, it->_key, tmp);
			} else {
				warning("load actor: key '%s' is unknown", it->_key.c_str());
			}
		}
	}
	return SQ_OK;
}

static SQRESULT loadObject(Common::SharedPtr<Object> obj, const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	int state = 0;
	if (json.contains("_state"))
		state = json["_state"]->asIntegerNumber();
	if (obj->_node)
		obj->setState(state, true);
	else
		warning("obj '%s' has no node !", obj->_key.c_str());
	bool touchable = true;
	if (json.contains("_touchable"))
		touchable = json["_touchable"]->asIntegerNumber() == 1;
	obj->setTouchable(touchable);
	bool hidden = false;
	if (json.contains("_hidden"))
		hidden = json["_hidden"]->asIntegerNumber() == 1;
	obj->_node->setVisible(!hidden);

	for (auto it = json.begin(); it != json.end(); it++) {
		if ((it->_key == "_state") || (it->_key == "_touchable") || (it->_key == "_hidden")) {
			// discard
		} else if (it->_key == "_pos") {
			obj->_node->setPos(parseVec2(it->_value->asString()));
		} else if (it->_key == "_rotation") {
			obj->_node->setRotation(it->_value->asNumber());
		} else if (it->_key == "_dir") {
			obj->setFacing((Facing)it->_value->asIntegerNumber());
		} else if (it->_key == "_useDir") {
			obj->_useDir = (Direction)it->_value->asIntegerNumber();
		} else if (it->_key == "_usePos") {
			obj->_usePos = parseVec2(it->_value->asString());
		} else if (it->_key == "_offset") {
			obj->_node->setOffset(parseVec2(it->_value->asString()));
		} else if (it->_key == "_renderOffset") {
			obj->_node->setRenderOffset(parseVec2(it->_value->asString()));
		} else if (it->_key == "_roomKey") {
			Object::setRoom(obj, room(it->_value->asString()));
		} else if (!it->_key.hasPrefix("_")) {
			HSQOBJECT tmp;
			if (SQ_FAILED(toSquirrel(it->_value, tmp)))
				return sq_throwerror(v, "failed to get table object");
			if (sqrawexists(obj->_table, it->_key))
				sqsetf(obj->_table, it->_key, tmp);
			else
				sqnewf(obj->_table, it->_key, tmp);
		} else {
			warning("load object (%s): key '%s' is unknown", obj->_key.c_str(), it->_key.c_str());
		}
	}

	if (sqrawexists(obj->_table, "postLoad"))
		sqcall(obj->_table, "postLoad");

	return SQ_OK;
}

static void loadPseudoObjects(Common::SharedPtr<Room> room, const Common::JSONObject &json) {
	for (auto it = json.begin(); it != json.end(); it++) {
		Common::SharedPtr<Object> o(object(room, it->_key));
		if (!o)
			warning("load: room '%s' object '%s' not loaded because it has not been found", room->_name.c_str(), it->_key.c_str());
		else
			loadObject(o, it->_value->asObject());
	}
}

static SQRESULT loadRoom(Common::SharedPtr<Room> room, const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	for (auto it = json.begin(); it != json.end(); it++) {
		if (it->_key == "_pseudoObjects") {
			loadPseudoObjects(room, it->_value->asObject());
		} else {
			if (!it->_key.hasPrefix("_")) {
				Common::SharedPtr<Object> o(object(room, it->_key));
				if (!o) {
					HSQOBJECT tmp;
					if (SQ_FAILED(toSquirrel(it->_value, tmp)))
						return sq_throwerror(v, "failed to get table object");
					if (sqrawexists(room->_table, it->_key))
						sqsetf(room->_table, it->_key, tmp);
					else {
						sqnewf(room->_table, it->_key, tmp);
					}
				} else {
					loadObject(o, it->_value->asObject());
				}
			} else {
				warning("Load room: key '{%s}' is unknown", it->_key.c_str());
			}
		}
	}

	if (sqrawexists(room->_table, "postLoad"))
		sqcall(room->_table, "postLoad");

	return SQ_OK;
}

static void setActor(const Common::String &key) {
	if (key.empty())
		return;
	for (size_t i = 0; i < g_twp->_actors.size(); i++) {
		Common::SharedPtr<Object> a = g_twp->_actors[i];
		if (a->_key == key) {
			g_twp->setActor(a, false);
			return;
		}
	}
}

static int32 computeHash(byte *data, size_t n) {
	int32 result = 0x6583463;
	for (size_t i = 0; i < n; i++) {
		result += (int32)data[i];
	}
	return result;
}

bool SaveGameManager::loadGame(Common::SeekableReadStream &stream) {
	Common::Array<byte> data(500016);
	stream.read(data.data(), data.size());
	Common::BTEACrypto::decrypt((uint32 *)data.data(), data.size() / 4, savegameKey);

	MemStream ms;
	if (!ms.open(data.data(), data.size() - 16))
		return false;

	GGHashMapDecoder decoder;
	Common::JSONValue *jSavegame = decoder.open(&ms);
	if (!jSavegame)
		return false;

	// dump savegame as json
	if (DebugMan.isDebugChannelEnabled(kDebugGame)) {
		debugC(kDebugGame, "load game: %s", jSavegame->stringify().c_str());
	}

	const Common::JSONObject &json = jSavegame->asObject();
	long long int version = json["version"]->asIntegerNumber();
	if (version != 2) {
		delete jSavegame;
		error("Cannot load savegame version %lld", version);
		return false;
	}
	uint32 gameTime = (uint32)json["gameTime"]->asNumber();

	sqcall("preLoad");
	loadGameScene(json["gameScene"]->asObject());
	loadDialog(json["dialog"]->asObject());
	if (SQ_FAILED(loadCallbacks(json["callbacks"]->asObject()))) {
		delete jSavegame;
		return false;
	}
	if (SQ_FAILED(loadGlobals(json["globals"]->asObject()))) {
		delete jSavegame;
		return false;
	}
	if (SQ_FAILED(loadActors(json["actors"]->asObject()))) {
		delete jSavegame;
		return false;
	}
	loadInventory(json["inventory"]);
	if (SQ_FAILED(loadRooms(json["rooms"]->asObject()))) {
		delete jSavegame;
		return false;
	}
	g_twp->_time = (float)gameTime;
	// reset _nextHoldToMoveTime because it's based on time
	g_twp->_nextHoldToMoveTime = 0.f;
	g_twp->setTotalPlayTime(gameTime * 1000);
	g_twp->_inputState.setState((InputStateFlag)json["inputState"]->asIntegerNumber());
	if (SQ_FAILED(loadObjects(json["objects"]->asObject()))) {
		delete jSavegame;
		return false;
	}
	g_twp->setRoom(room(json["currentRoom"]->asString()), true);
	setActor(json["selectedActor"]->asString());
	if (g_twp->_actor)
		g_twp->cameraAt(g_twp->_actor->_node->getPos());

	HSQUIRRELVM v = g_twp->getVm();
	sqsetf(sqrootTbl(v), "SAVEBUILD", static_cast<int>(json["savebuild"]->asIntegerNumber()));

	for (auto a : g_twp->_actors) {
		if (sqrawexists(a->_table, "postLoad")) {
			sqcall(a->_table, "postLoad");
		}
	}

	sqcall("postLoad");

	delete jSavegame;
	return true;
}

bool SaveGameManager::getSaveGame(Common::SeekableReadStream *stream, SaveGame &savegame) {
	Common::Array<byte> data(stream->size());
	stream->read(data.data(), data.size());
	Common::BTEACrypto::decrypt((uint32 *)data.data(), data.size() / 4, savegameKey);
	savegame.hashData = *(int32 *)(&data[data.size() - 16]);
	savegame.time = *(int32 *)&data[data.size() - 12];
	int32 hashCheck = computeHash(data.data(), data.size() - 16);
	if (savegame.hashData != hashCheck)
		return false;

	MemStream ms;
	if (!ms.open(data.data(), data.size() - 16))
		return false;

	GGHashMapDecoder decoder;
	savegame.jSavegame.reset(decoder.open(&ms));
	if (!savegame.jSavegame)
		return false;

	const Common::JSONObject &jSavegame = savegame.jSavegame->asObject();
	savegame.gameTime = jSavegame["gameTime"]->asNumber();
	savegame.easyMode = jSavegame["easy_mode"]->asIntegerNumber() != 0;

	return true;
}

void SaveGameManager::loadGameScene(const Common::JSONObject &json) {
	int mode = 0;
	if (json["actorsSelectable"]->asIntegerNumber() != 0)
		mode |= asOn;
	if (json["actorsTempUnselectable"]->asIntegerNumber())
		mode |= asTemporaryUnselectable;
	g_twp->_actorSwitcher._mode = mode;
	// TODO: tmpPrefs().forceTalkieText = json["forceTalkieText"].getInt() != 0;
	const Common::JSONArray &jSelectableActors = json["selectableActors"]->asArray();
	for (size_t i = 0; i < jSelectableActors.size(); i++) {
		const Common::JSONObject &jSelectableActor = jSelectableActors[i]->asObject();
		Common::SharedPtr<Object> act = jSelectableActor.contains("_actorKey") ? actor(jSelectableActor["_actorKey"]->asString()) : nullptr;
		g_twp->_hud->_actorSlots[i].actor = act;
		g_twp->_hud->_actorSlots[i].selectable = jSelectableActor["selectable"]->asIntegerNumber() != 0;
	}
}

void SaveGameManager::loadDialog(const Common::JSONObject &json) {
	debugC(kDebugGame, "loadDialog");
	g_twp->_dialog->_states.clear();
	for (auto it = json.begin(); it != json.end(); it++) {
		Common::String dialog(it->_key);
		// dialog format: mode dialog number actor
		// example: #ChetAgentStreetDialog14reyes
		// mode:
		// ?: once
		// #: showonce
		// &: onceever
		// $: showonceever
		// ^: temponce
		DialogConditionState state = parseState(dialog);
		g_twp->_dialog->_states.push_back(state);
		// TODO: what to do with this dialog value ?
		// let value = property.second.getInt()
	}
}

struct GetHObjects {
	explicit GetHObjects(Common::Array<HSQOBJECT> &objs) : _objs(objs) {}

	void operator()(HSQOBJECT item) {
		_objs.push_back(item);
	}

private:
	Common::Array<HSQOBJECT> &_objs;
};

SQRESULT SaveGameManager::loadCallbacks(const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	debugC(kDebugGame, "loadCallbacks");
	g_twp->_callbacks.clear();
	if (!json["callbacks"]->isNull()) {
		const Common::JSONArray &jCallbacks = json["callbacks"]->asArray();
		for (size_t i = 0; i < jCallbacks.size(); i++) {
			const Common::JSONObject &jCallBackHash = jCallbacks[i]->asObject();
			const int id = jCallBackHash["guid"]->asIntegerNumber();
			const float time = jCallBackHash["time"]->isIntegerNumber() ? (float)jCallBackHash["time"]->asIntegerNumber() : 0.f;
			const Common::String name = jCallBackHash["function"]->asString();
			Common::Array<HSQOBJECT> args;
			if (jCallBackHash.contains("param")) {
				HSQOBJECT arg;
				if (SQ_FAILED(toSquirrel(jCallBackHash["param"], arg)))
					return sq_throwerror(v, "failed to get callback arg");
				sqgetitems(arg, GetHObjects(args));
			}
			g_twp->_callbacks.push_back(Common::SharedPtr<Callback>(new Callback(id, time, name, args)));
		}
	}
	g_twp->_resManager->resetIds(json["nextGuid"]->asIntegerNumber());
	return SQ_OK;
}

SQRESULT SaveGameManager::loadGlobals(const Common::JSONObject &json) {
	debugC(kDebugGame, "loadGlobals");
	HSQUIRRELVM v = g_twp->getVm();
	HSQOBJECT g;
	if (SQ_FAILED(sqgetf("g", g)))
		return sq_throwerror(v, "Failed to get globals variable");
	for (auto it = json.begin(); it != json.end(); it++) {
		HSQOBJECT tmp;
		if (SQ_FAILED(toSquirrel(it->_value, tmp)))
			return sq_throwerror(v, "failed to get callback arg");
		sq_addref(v, &tmp);
		sqsetf(g, it->_key, tmp);
	}
	return SQ_OK;
}

SQRESULT SaveGameManager::loadActors(const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	for (size_t i = 0; i < g_twp->_actors.size(); i++) {
		Common::SharedPtr<Object> a = g_twp->_actors[i];
		if (a->_key.size() > 0) {
			if (SQ_FAILED(loadActor(a, json[a->_key]->asObject())))
				return sq_throwerror(v, "failed to load actor");
		}
	}
	return SQ_OK;
}

void SaveGameManager::loadInventory(const Common::JSONValue *json) {
	if (json->isObject()) {
		const Common::JSONObject &jInventory = json->asObject();
		const Common::JSONArray &jSlots = jInventory["slots"]->asArray();
		for (int i = 0; i < NUMACTORS; i++) {
			Common::SharedPtr<Object> a(g_twp->_hud->_actorSlots[i].actor);
			if (a) {
				a->_inventory.clear();
				const Common::JSONObject &jSlot = jSlots[i]->asObject();
				if (jSlot.contains("objects")) {
					if (jSlot["objects"]->isArray()) {
						int jiggleCount = 0;
						const Common::JSONArray &jSlotObjects = jSlot["objects"]->asArray();
						for (size_t j = 0; j < jSlotObjects.size(); j++) {
							const Common::JSONValue *jObj = jSlotObjects[j];
							Common::SharedPtr<Object> obj = object(jObj->asString());
							if (!obj)
								warning("inventory obj '%s' not found", jObj->asString().c_str());
							else {
								Object::pickupObject(a, obj);
								obj->_jiggle = jSlot.contains("jiggle") && jSlot["jiggle"]->isArray() && jSlot["jiggle"]->asArray()[jiggleCount++]->asIntegerNumber() != 0;
							}
						}
					}
				}
				a->_inventoryOffset = jSlot["scroll"]->asIntegerNumber();
			}
		}
	}
}

SQRESULT SaveGameManager::loadRooms(const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	for (auto it = json.begin(); it != json.end(); it++) {
		if (SQ_FAILED(loadRoom(room(it->_key), it->_value->asObject())))
			return sq_throwerror(v, "failed to load room");
	}
	return SQ_OK;
}

SQRESULT SaveGameManager::loadObjects(const Common::JSONObject &json) {
	HSQUIRRELVM v = g_twp->getVm();
	for (auto it = json.begin(); it != json.end(); it++) {
		Common::SharedPtr<Object> o(object(it->_key));
		if (o) {
			if (SQ_FAILED(loadObject(o, it->_value->asObject())))
				return sq_throwerror(v, "failed to load object");
		} else {
			warning("object '%s' not found", it->_key.c_str());
		}
	}
	return SQ_OK;
}

struct JsonCallback {
	bool skipObj;
	bool pseudo;
	HSQOBJECT *rootTable;
	Common::JSONObject *jObj;
};

static Common::JSONValue *tojson(const HSQOBJECT &obj, bool checkId, bool skipObj = false, bool pseudo = false);

static void fillMissingProperties(const Common::String &k, HSQOBJECT &oTable, void *data) {
	JsonCallback *params = static_cast<JsonCallback *>(data);
	if ((k.size() > 0) && (!k.hasPrefix("_"))) {
		if (!(params->skipObj && g_twp->_resManager->isObject(getId(oTable)) && (params->pseudo || sqrawexists(*params->rootTable, k)))) {
			Common::JSONValue *json = tojson(oTable, true);
			if (json) {
				(*params->jObj)[k] = json;
			}
		}
	}
}

struct GetJArray {
	explicit GetJArray(Common::JSONArray &arr) : _arr(arr) {}

	void operator()(HSQOBJECT item) {
		_arr.push_back(tojson(item, true));
	}

private:
	Common::JSONArray &_arr;
};

static SQRESULT toObject(Common::JSONObject &jObj, const HSQOBJECT &obj, bool checkId, bool skipObj = false, bool pseudo = false) {
	HSQUIRRELVM v = g_twp->getVm();
	if (checkId) {
		SQInteger id = 0;
		if (sqrawexists(obj, "_id") && SQ_FAILED(sqgetf(obj, "_id", id)))
			return sq_throwerror(v, "Failed to get id");
		if (g_twp->_resManager->isActor(id)) {
			Common::SharedPtr<Object> a(actor(id));
			jObj["_actorKey"] = new Common::JSONValue(a->_key);
			return SQ_OK;
		}

		if (g_twp->_resManager->isObject(id)) {
			Common::SharedPtr<Object> o(sqobj(id));
			if (!o)
				return SQ_OK;
			jObj["_objectKey"] = new Common::JSONValue(o->_key);
			if (o->_room && o->_room->_pseudo)
				jObj["_roomKey"] = new Common::JSONValue(o->_room->_name);
			return SQ_OK;
		}

		if (g_twp->_resManager->isRoom(id)) {
			Common::SharedPtr<Room> r(getRoom(id));
			jObj["_roomKey"] = new Common::JSONValue(r->_name);
			return SQ_OK;
		}
	}

	HSQOBJECT rootTbl = sqrootTbl(v);

	JsonCallback params;
	params.jObj = &jObj;
	params.pseudo = pseudo;
	params.rootTable = &rootTbl;
	params.skipObj = skipObj;
	return sqgetpairs(obj, fillMissingProperties, &params);
}

static Common::JSONValue *tojson(const HSQOBJECT &obj, bool checkId, bool skipObj, bool pseudo) {
	switch (obj._type) {
	case OT_INTEGER:
		return new Common::JSONValue(static_cast<long long>(sq_objtointeger(&obj)));
	case OT_FLOAT:
		return new Common::JSONValue(sq_objtofloat(&obj));
	case OT_STRING:
		return new Common::JSONValue(sq_objtostring(&obj));
	case OT_NULL:
		return new Common::JSONValue();
	case OT_ARRAY: {
		Common::JSONArray arr;
		sqgetitems(obj, GetJArray(arr));
		return new Common::JSONValue(arr);
	}
	case OT_TABLE: {
		Common::JSONObject jObj;
		toObject(jObj, obj, checkId, skipObj, pseudo);
		return new Common::JSONValue(jObj);
	}
	default:
		return nullptr;
	}
}

static Common::String removeFileExt(const Common::String &s) {
	size_t i = s.findLastOf('.');
	if (i != Common::String::npos) {
		return s.substr(0, i);
	}
	return s;
}

Common::String toString(const Math::Vector2d &pos) {
	return Common::String::format("{%d,%d}", (int)pos.getX(), (int)pos.getY());
}

// static Common::String getCustomAnim(Common::SharedPtr<Object> actor, const Common::String &name) {
// 	return actor->_animNames.contains(name) ? actor->_animNames[name] : name;
// }

// static Common::JSONValue *getCustomAnims(Common::SharedPtr<Object> actor) {
// 	Common::JSONArray jAnims;
// 	// add head anims
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, HEAD_ANIMNAME)));
// 	for (int i = 1; i < 7; i++) {
// 		jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%d", HEAD_ANIMNAME, i))));
// 	}
// 	// add stand anims
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", STAND_ANIMNAME, "_front"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", STAND_ANIMNAME, "_back"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", STAND_ANIMNAME, "_left"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", STAND_ANIMNAME, "_right"))));
// 	// add walk anims
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", WALK_ANIMNAME, "_front"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", WALK_ANIMNAME, "_back"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", WALK_ANIMNAME, "_right"))));
// 	jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s", WALK_ANIMNAME, "_right"))));
// 	// add reach anims
// 	const char *dirs[] = {"_front", "_back", "_right", "_right"};
// 	for (int i = 0; i < ARRAYSIZE(dirs); i++) {
// 		const char *dir = dirs[i];
// 		jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s%s", REACH_ANIMNAME, "_low", dir))));
// 		jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s%s", REACH_ANIMNAME, "_med", dir))));
// 		jAnims.push_back(new Common::JSONValue(getCustomAnim(actor, Common::String::format("%s%s%s", REACH_ANIMNAME, "_high", dir))));
// 	}
// 	return new Common::JSONValue(jAnims);
// }

static Common::JSONValue *createJActor(Common::SharedPtr<Object> actor) {
	Common::JSONObject jActor;
	toObject(jActor, actor->_table, false);
	int color = actor->_node->getComputedColor().toInt();
	if (color != Color().toInt())
		jActor["_color"] = new Common::JSONValue((long long int)color);
	// if (!actor->_animNames.empty())
	// 	jActor["_animations"] = getCustomAnims(actor);
	jActor["_costume"] = new Common::JSONValue(removeFileExt(actor->_costumeName));
	jActor["_dir"] = new Common::JSONValue((long long int)actor->_facing);
	jActor["_lockFacing"] = new Common::JSONValue((long long int)actor->_facingLockValue);
	jActor["_pos"] = new Common::JSONValue(toString(actor->_node->getPos()));
	if (actor->_useDir != dNone)
		jActor["_useDir"] = new Common::JSONValue((long long int)actor->_useDir);
	if (actor->_usePos != Math::Vector2d(0.f, 0.f))
		jActor["_usePos"] = new Common::JSONValue(toString(actor->_usePos));
	if (actor->_node->getRenderOffset() != Math::Vector2d(0.f, 45.f))
		jActor["_renderOffset"] = new Common::JSONValue(toString(actor->_node->getRenderOffset()));
	if (actor->_costumeSheet.size() > 0)
		jActor["_costumeSheet"] = new Common::JSONValue(actor->_costumeSheet);
	if (actor->_room)
		jActor["_roomKey"] = new Common::JSONValue(actor->_room->_name);
	if (!actor->isTouchable() && actor->_node->isVisible())
		jActor["_untouchable"] = new Common::JSONValue(1LL);
	if (!actor->_node->isVisible())
		jActor["_hidden"] = new Common::JSONValue(1LL);
	if (actor->_volume != 1.f)
		jActor["_volume"] = new Common::JSONValue(actor->_volume);
	//   result.fields.sort(cmpKey);
	return new Common::JSONValue(jActor);
}

static Common::JSONValue *createJActors() {
	Common::JSONObject jActors;
	for (size_t i = 0; i < g_twp->_actors.size(); i++) {
		Common::SharedPtr<Object> a(g_twp->_actors[i]);
		if (a->_key.size() > 0) {
			jActors[a->_key] = createJActor(a);
		}
	}
	// result.fields.sort(cmpKey);
	return new Common::JSONValue(jActors);
}

static Common::JSONValue *createJCallback(const Callback &callback) {
	Common::JSONObject result;
	result["function"] = new Common::JSONValue(callback.getName());
	result["guid"] = new Common::JSONValue((long long int)callback.getId());
	result["time"] = new Common::JSONValue((long long int)MAX(0.f, (callback.getDuration() - callback.getElapsed())));
	Common::JSONArray jArgs;
	const Common::Array<HSQOBJECT> &args = callback.getArgs();
	for (size_t i = 0; i < args.size(); i++) {
		jArgs.push_back(tojson(args[i], false));
	}
	if (jArgs.size() > 0)
		result["param"] = new Common::JSONValue(jArgs);
	return new Common::JSONValue(result);
}

static Common::JSONValue *createJCallbackArray() {
	Common::JSONArray result;
	for (size_t i = 0; i < g_twp->_callbacks.size(); i++) {
		result.push_back(createJCallback(*g_twp->_callbacks[i]));
	}
	return new Common::JSONValue(result);
}

static Common::JSONValue *createJCallbacks() {
	Common::JSONObject json;
	json["callbacks"] = createJCallbackArray();
	json["nextGuid"] = new Common::JSONValue((long long int)g_twp->_resManager->getCallbackId());
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJRoomKey(Common::SharedPtr<Room> room) {
	return new Common::JSONValue(room ? room->_name : "Void");
}

Common::String createJDlgStateKey(const DialogConditionState &state) {
	Common::String s;
	switch (state.mode) {
	case OnceEver:
		s = "&";
		break;
	case ShowOnce:
		s = "#";
		break;
	case Once:
		s = "?";
		break;
	case ShowOnceEver:
		s = "$";
		break;
	default:
		break;
	}
	return Common::String::format("%s%s%d%s", s.c_str(), state.dialog.c_str(), state.line, state.actorKey.c_str());
}

static Common::JSONValue *createJDialog() {
	Common::JSONObject json;
	for (size_t i = 0; i < g_twp->_dialog->_states.size(); i++) {
		const DialogConditionState &state = g_twp->_dialog->_states[i];
		if (state.mode != TempOnce) {
			// TODO: value should be 1 or another value ?
			json[createJDlgStateKey(state)] = new Common::JSONValue(state.mode == ShowOnce ? 2LL : 1LL);
		}
	}
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJEasyMode() {
	HSQOBJECT g;
	if (SQ_FAILED(sqgetf("g", g)))
		error("Failed to get globals variable");
	SQInteger easyMode;
	if (SQ_FAILED(sqgetf(g, "easy_mode", easyMode)))
		error("Failed to get easy_mode variable");
	return new Common::JSONValue((long long int)easyMode);
}

static Common::JSONValue *createJBool(bool value) {
	return new Common::JSONValue(value ? 1LL : 0LL);
}

static Common::JSONValue *createJSelectableActor(const ActorSlot &slot) {
	Common::JSONObject json;
	if (slot.actor) {
		json["_actorKey"] = new Common::JSONValue(slot.actor->_key);
	}
	json["selectable"] = createJBool(slot.selectable);
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJSelectableActors() {
	Common::JSONArray json;
	for (int i = 0; i < NUMACTORS; i++) {
		const ActorSlot &slot = g_twp->_hud->_actorSlots[i];
		json.push_back(createJSelectableActor(slot));
	}
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJGameScene() {
	bool actorsSelectable = asOn & g_twp->_actorSwitcher._mode;
	bool actorsTempUnselectable = asTemporaryUnselectable & g_twp->_actorSwitcher._mode;
	Common::JSONObject json;
	json["actorsSelectable"] = createJBool(actorsSelectable);
	json["actorsTempUnselectable"] = createJBool(actorsTempUnselectable);
	// TODO: json["forceTalkieText"] = createJBool(tmpPrefs().forceTalkieText);
	json["selectableActors"] = createJSelectableActors();
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJGlobals() {
	HSQOBJECT g;
	if (SQ_FAILED(sqgetf("g", g)))
		error("Failed to get globals variable");
	//   result.fields.sort(cmpKey);
	return tojson(g, false);
}

static Common::JSONValue *createJInventory(const ActorSlot &slot) {
	Common::JSONObject json;
	if (!slot.actor) {
		json["scroll"] = new Common::JSONValue(0LL);
	} else {
		Common::JSONArray objKeys;
		Common::JSONArray jiggleArray;
		bool anyJiggle = false;
		for (size_t i = 0; i < slot.actor->_inventory.size(); i++) {
			Common::SharedPtr<Object> obj = slot.actor->_inventory[i];
			if (obj->_jiggle)
				anyJiggle = true;
			jiggleArray.push_back(createJBool(obj->_jiggle));
			objKeys.push_back(new Common::JSONValue(obj->_key));
		}

		if (objKeys.size() > 0) {
			json["objects"] = new Common::JSONValue(objKeys);
		}
		json["scroll"] = new Common::JSONValue((long long int)slot.actor->_inventoryOffset);
		if (anyJiggle)
			json["jiggle"] = new Common::JSONValue(jiggleArray);
	}
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJInventory() {
	Common::JSONArray slots;
	for (int i = 0; i < NUMACTORS; i++) {
		const ActorSlot &slot = g_twp->_hud->_actorSlots[i];
		slots.push_back(createJInventory(slot));
	}
	Common::JSONObject json;
	json["slots"] = new Common::JSONValue(slots);
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJObject(HSQOBJECT &table, Common::SharedPtr<Object> obj) {
	Common::JSONObject json;
	toObject(json, table, false);
	if (obj) {
		if (!obj->_node->isVisible())
			json["_hidden"] = new Common::JSONValue(1LL);
		if (obj->_state != 0)
			json["_state"] = new Common::JSONValue((long long int)obj->_state);
		if (obj->_node->isVisible() && !obj->isTouchable())
			json["_touchable"] = new Common::JSONValue(0LL);
		if (obj->_node->getOffset() != Math::Vector2d())
			json["_offset"] = new Common::JSONValue(toString(obj->_node->getOffset()));
	}
	//   result.fields.sort(cmpKey)
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJObjects() {
	Common::JSONObject json;
	for (const auto &r : g_twp->_rooms) {
		for (const auto &layer : r->_layers) {
			for (auto &obj : layer->_objects) {
				if (obj->_objType != ObjectType::otNone)
					continue;
				if (obj->_room && obj->_room->_pseudo)
					continue;
				json[obj->_key] = createJObject(obj->_table, obj);
			}
		}
	}
	//   result.fields.sort(cmpKey)
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJPseudoObjects(Common::SharedPtr<Room> room) {
	Common::JSONObject json;
	for (auto &layer : room->_layers) {
		for (auto &obj : layer->_objects) {
			if (obj->_objType != ObjectType::otNone)
				continue;
			if (obj->_room && obj->_room->_pseudo)
				continue;
			json[obj->_key] = createJObject(obj->_table, obj);
		}
	}

	//   result.fields.sort(cmpKey)
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJRoom(Common::SharedPtr<Room> room) {
	Common::JSONObject json;
	toObject(json, room->_table, false, true, room->_pseudo);
	if (room->_pseudo) {
		json["_pseudoObjects"] = createJPseudoObjects(room);
	}
	//   result.fields.sort(cmpKey)
	return new Common::JSONValue(json);
}

static Common::JSONValue *createJRooms() {
	Common::JSONObject json;
	for (size_t i = 0; i < g_twp->_rooms.size(); i++) {
		Common::SharedPtr<Room> r(g_twp->_rooms[i]);
		if (r)
			json[r->_name] = createJRoom(r);
	}
	//   result.fields.sort(cmpKey);
	return new Common::JSONValue(json);
}

static Common::JSONValue *createSaveGame() {
	Common::JSONObject json;
	json["actors"] = createJActors();
	json["callbacks"] = createJCallbacks();
	json["currentRoom"] = createJRoomKey(g_twp->_room);
	json["dialog"] = createJDialog();
	json["easy_mode"] = createJEasyMode();
	json["gameGUID"] = new Common::JSONValue("");
	json["gameScene"] = createJGameScene();
	json["gameTime"] = new Common::JSONValue(g_twp->_time);
	json["globals"] = createJGlobals();
	json["inputState"] = new Common::JSONValue((long long int)g_twp->_inputState.getState());
	json["inventory"] = createJInventory();
	json["objects"] = createJObjects();
	json["rooms"] = createJRooms();
	json["savebuild"] = new Common::JSONValue(958LL);
	json["savetime"] = new Common::JSONValue((long long)getTime());
	json["selectedActor"] = new Common::JSONValue(g_twp->_actor ? g_twp->_actor->_key : "");
	json["version"] = new Common::JSONValue((long long int)2);
	return new Common::JSONValue(json);
}

void SaveGameManager::saveGame(Common::WriteStream *ws) {
	Common::JSONValue *data = createSaveGame();

	if (DebugMan.isDebugChannelEnabled(kDebugGame)) {
		debugC(kDebugGame, "save game: %s", data->stringify().c_str());
	}

	const uint32 fullSize = 500000;
	Common::Array<byte> buffer(fullSize + 16);
	OutMemStream stream;
	stream.open(buffer.data(), buffer.size());

	GGHashMapEncoder encoder;
	encoder.open(&stream);
	encoder.write(data->asObject());

	int32 savetime = data->asObject()["savetime"]->asIntegerNumber();
	int32 hash = computeHash(buffer.data(), fullSize);
	byte marker = (8 - ((fullSize + 9) % 8));

	// write at the end 16 bytes: hashdata (4 bytes) + savetime (4 bytes) + marker (8 bytes)
	int32 *p = (int32 *)(buffer.data() + fullSize);
	p[0] = hash;
	p[1] = savetime;
	memset(&p[2], marker, 8);

	// then encode data
	Common::BTEACrypto::encrypt((uint32 *)buffer.data(), buffer.size() / 4, savegameKey);

	// and write data
	ws->write(buffer.data(), buffer.size());
	delete data;

	sqcall("postSave");
}

} // namespace Twp
