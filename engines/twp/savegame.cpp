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

#include "twp/ggpack.h"
#include "twp/savegame.h"
#include "twp/squtil.h"
#include "twp/btea.h"

namespace Twp {

static Object *actor(const Common::String &key) {
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *a = g_engine->_actors[i];
		if (a->_key == key)
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
	int i = 1;
	while (i < dialog.size() && !Common::isDigit(dialog[i])) {
		dialogName += dialog[i];
		i++;
	}

	while (!g_engine->_pack.assetExists((dialogName + ".byack").c_str()) && (i < dialog.size())) {
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
	result.dialog = dialogName;
	result.line = atol(num.c_str());
	result.actorKey = dialog.substr(i);
	return result;
}

static Room *room(const Common::String &name) {
	for (int i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		if (room->_name == name) {
			return room;
		}
	}
	return nullptr;
}

static Object *object(Room *room, const Common::String &key) {
	for (int i = 0; i < room->_layers.size(); i++) {
		Layer *layer = room->_layers[i];
		for (int j = 0; j < layer->_objects.size(); j++) {
			Object *o = layer->_objects[j];
			if (o->_key == key)
				return o;
		}
	}
	return nullptr;
}

static Object *object(const Common::String &key) {
	for (int i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		for (int j = 0; j < room->_layers.size(); j++) {
			Layer *layer = room->_layers[j];
			for (int k = 0; k < layer->_objects.size(); k++) {
				Object *o = layer->_objects[k];
				if (o->_key == key)
					return o;
			}
		}
	}
	return nullptr;
}

static void toSquirrel(const Common::JSONValue *json, HSQOBJECT &obj) {
	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	sq_resetobject(&obj);
	if (json->isString()) {
		sqpush(v, json->asString());
		sqget(v, -1, obj);
	} else if (json->isIntegerNumber()) {
		sqpush(v, json->asIntegerNumber());
		sqget(v, -1, obj);
	} else if (json->isBool()) {
		sqpush(v, json->asBool());
		sqget(v, -1, obj);
	} else if (json->isNumber()) {
		sqpush(v, json->asNumber());
		sqget(v, -1, obj);
	} else if (json->isNull()) {
	} else if (json->isArray()) {
		sq_newarray(v, 0);
		const Common::JSONArray &jArr = json->asArray();
		for (int i = 0; i < jArr.size(); i++) {
			HSQOBJECT tmp;
			toSquirrel(jArr[i], tmp);
			sqpush(v, tmp);
			sq_arrayappend(v, -2);
		}
		sqget(v, -1, obj);
	} else if (json->isObject()) {
		const Common::JSONObject &jObject = json->asObject();
		// check if it's a reference to an actor
		if (jObject.contains("_actorKey")) {
			obj = actor(jObject["_actorKey"]->asString())->_table;
		} else if (jObject.contains("_roomKey")) {
			Common::String roomName = jObject["_roomKey"]->asString();
			if (jObject.contains("_objectKey")) {
				Common::String objName = jObject["_objectKey"]->asString();
				Room *r = room(roomName);
				if (!r)
					warning("room with key=%s not found", roomName.c_str());
				else {
					Object *o = object(r, objName);
					if (!o)
						warning("room object with key=%s/%s not found", roomName.c_str(), objName.c_str());
					else
						obj = o->_table;
				}
			} else {
				Room *r = room(roomName);
				if (!r) {
					warning("room with key=%s not found", roomName.c_str());
				} else {
					obj = r->_table;
				}
			}
		} else if (jObject.contains("_objectKey")) {
			Common::String objName = jObject["_objectKey"]->asString();
			Object *o = object(objName);
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
				toSquirrel(it->_value, tmp);
				sqpush(v, tmp);
				sq_newslot(v, -3, SQFalse);
			}
			sqget(v, -1, obj);
		}
	}
	sq_addref(v, &obj);
	sq_settop(v, top);
}

static Common::String sub(const Common::String &s, size_t pos, size_t end) {
	return s.substr(pos, s.size() - end);
}

static void setAnimations(Object *actor, const Common::JSONArray &anims) {
	Common::String headAnim = anims[0]->asString();
	Common::String standAnim = sub(anims[9]->asString(), 0, 7);
	Common::String walkAnim = sub(anims[11]->asString(), 0, 7);
	Common::String reachAnim = sub(anims[15]->asString(), 0, 11);
	actor->setAnimationNames(headAnim, standAnim, walkAnim, reachAnim);
}

static void loadActor(Object *actor, const Common::JSONObject &json) {
	bool touchable = true;
	if (json.contains("_untouchable"))
		touchable = json["_untouchable"]->asIntegerNumber() == 0;
	actor->setTouchable(touchable);
	bool hidden = false;
	if (json.contains("_hidden"))
		hidden = json["_hidden"]->asIntegerNumber() == 1;
	actor->_node->setVisible(!hidden);
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
			actor->setRoom(room(it->_value->asString()));
		} else if ((it->_key == "_hidden") || (it->_key == "_untouchable")) {
		} else if (it->_key == "_volume") {
			actor->_volume = it->_value->asNumber();
		} else {
			if (!it->_key.hasPrefix("_")) {
				HSQOBJECT tmp;
				toSquirrel(it->_value, tmp);
				if (sqrawexists(actor->_table, it->_key))
					sqsetf(actor->_table, it->_key, tmp);
				else
					sqnewf(actor->_table, it->_key, tmp);
			} else {
				warning("load actor: key '%s' is unknown", it->_key.c_str());
			}
		}
	}

	if (sqrawexists(actor->_table, "postLoad"))
		sqcall(actor->_table, "postLoad");
}

static void loadObject(Object *obj, const Common::JSONObject &json) {
	int state = 0;
	if (json.contains("_state"))
		state = json["_state"]->asIntegerNumber();
	if (obj->_node)
		obj->setState(state, true);
	else
		warning("obj '{obj.key}' has no node !");
	bool touchable = true;
	if (json.contains("_touchable"))
		touchable = json["_touchable"]->asIntegerNumber() == 1;
	obj->setTouchable(touchable);
	bool hidden = false;
	if (json.contains("_hidden"))
		hidden = json["_hidden"]->asIntegerNumber() == 1;
	obj->_node->setVisible(!hidden);

	for (auto it = json.begin(); it != json.end(); it++) {
		if ((it->_key == "_state") || (it->_key == "_state") || (it->_key == "_state")) {
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
			obj->setRoom(room(it->_value->asString()));
		} else if (!it->_key.hasPrefix("_")) {
			HSQOBJECT tmp;
			toSquirrel(it->_value, tmp);
			if (sqrawexists(obj->_table, it->_key))
				sqsetf(obj->_table, it->_key, tmp);
			else
				sqnewf(obj->_table, it->_key, tmp);
		} else {
			warning("load object: key '{%s}' is unknown", it->_key.c_str());
		}
	}

	if (sqrawexists(obj->_table, "postLoad"))
		sqcall(obj->_table, "postLoad");
}

static void loadPseudoObjects(Room *room, const Common::JSONObject &json) {
	for (auto it = json.begin(); it != json.end(); it++) {
		Object *o = object(room, it->_key);
		if (!o)
			warning("load: room '%s' object '%s' not loaded because it has not been found", room->_name.c_str(), it->_key.c_str());
		else
			loadObject(o, it->_value->asObject());
	}
}

static void loadRoom(Room *room, const Common::JSONObject &json) {
	for (auto it = json.begin(); it != json.end(); it++) {
		if (it->_key == "_pseudoObjects") {
			loadPseudoObjects(room, it->_value->asObject());
		} else {
			if (!it->_key.hasPrefix("_")) {
				Object *o = object(room, it->_key);
				if (!o) {
					HSQOBJECT tmp;
					toSquirrel(it->_value, tmp);
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
}

static void setActor(const Common::String &key) {
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *a = g_engine->_actors[i];
		if (a->_key == key) {
			g_engine->setActor(a, false);
			return;
		}
	}
}

static int32_t computeHash(byte *data, size_t n) {
	int32_t result = 0x6583463;
	for (size_t i = 0; i < n; i++) {
		result += (int32_t)data[i];
	}
	return result;
}

bool SaveGameManager::loadGame(const SaveGame& savegame) {
	const Common::JSONObject& json = savegame.jSavegame->asObject();
	long long int version = json["version"]->asIntegerNumber();
	if (version != 2) {
		error("Cannot load savegame version %lld", version);
		delete savegame.jSavegame;
		return false;
	}

	sqcall("preLoad");
	loadGameScene(json["gameScene"]->asObject());
	loadDialog(json["gameScene"]->asObject());
	loadCallbacks(json["callbacks"]->asObject());
	loadGlobals(json["globals"]->asObject());
	loadActors(json["actors"]->asObject());
	loadInventory(json["inventory"]);
	loadRooms(json["rooms"]->asObject());
	g_engine->_time = json["gameTime"]->asNumber();
	g_engine->setTotalPlayTime(savegame.gameTime * 1000);
	g_engine->_inputState.setState((InputStateFlag)json["inputState"]->asIntegerNumber());
	loadObjects(json["objects"]->asObject());
	setActor(json["selectedActor"]->asString());
	g_engine->setRoom(room(json["currentRoom"]->asString()));

	HSQUIRRELVM v = g_engine->getVm();
	sqsetf(sqrootTbl(v), "SAVEBUILD", json["savebuild"]->asIntegerNumber());

	sqcall("postLoad");

	delete savegame.jSavegame;
	return true;
}

bool SaveGameManager::getSaveGame(Common::SeekableReadStream *stream, SaveGame& savegame) {
	static uint32_t savegameKey[] = {0xAEA4EDF3, 0xAFF8332A, 0xB5A2DBB4, 0x9B4BA022};
	Common::Array<byte> data(stream->size());
	stream->read(&data[0], data.size());
	BTEACrypto::decrypt((uint32_t *)&data[0], data.size() / 4, savegameKey);
	savegame.hashData = *(int32_t *)(&data[data.size() - 16]);
	savegame.time = *(int32_t *)&data[data.size() - 12];
	int32_t hashCheck = computeHash(&data[0], data.size() - 16);
	if (savegame.hashData != hashCheck)
		return false;

	MemStream ms;
	ms.open(&data[0], data.size() - 16);

	GGHashMapDecoder decoder;
	savegame.jSavegame = decoder.open(&ms);
	const Common::JSONObject& jSavegame = savegame.jSavegame->asObject();
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
	g_engine->_actorSwitcher._mode = mode;
	// TODO: tmpPrefs().forceTalkieText = json["forceTalkieText"].getInt() != 0;
	const Common::JSONArray &jSelectableActors = json["selectableActors"]->asArray();
	for (int i = 0; i < jSelectableActors.size(); i++) {
		const Common::JSONObject &jSelectableActor = jSelectableActors[i]->asObject();
		Object *act = jSelectableActor.contains("_actorKey") ? actor(jSelectableActor["_actorKey"]->asString()) : nullptr;
		g_engine->_hud._actorSlots[i].actor = act;
		g_engine->_hud._actorSlots[i].selectable = jSelectableActor["selectable"]->asIntegerNumber() != 0;
	}
}

void SaveGameManager::loadDialog(const Common::JSONObject &json) {
	debug("loadDialog");
	g_engine->_dialog._states.clear();
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
		g_engine->_dialog._states.push_back(state);
		// TODO: what to do with this dialog value ?
		// let value = property.second.getInt()
	}
}

void SaveGameManager::loadCallbacks(const Common::JSONObject &json) {
	debug("loadCallbacks");
	g_engine->_callbacks.clear();
	if (!json["callbacks"]->isNull()) {
		const Common::JSONArray &jCallbacks = json["callbacks"]->asArray();
		for (int i = 0; i < jCallbacks.size(); i++) {
			const Common::JSONObject &jCallBackHash = jCallbacks[i]->asObject();
			int id = jCallBackHash["guid"]->asIntegerNumber();
			float time = ((float)jCallBackHash["time"]->asIntegerNumber()) / 1000.f;
			Common::String name = jCallBackHash["function"]->asString();
			Common::Array<HSQOBJECT> args;
			if (jCallBackHash.contains("param")) {
				HSQOBJECT arg;
				toSquirrel(jCallBackHash["param"], arg);
				sqgetitems(arg, [&args](HSQOBJECT &o) { args.push_back(o); });
			}
			g_engine->_callbacks.push_back(new Callback(id, time, name, args));
		}
	}
	setCallbackId(json["nextGuid"]->asIntegerNumber());
}

void SaveGameManager::loadGlobals(const Common::JSONObject &json) {
	debug("loadGlobals");
	HSQUIRRELVM v = g_engine->getVm();
	HSQOBJECT g;
	sqgetf("g", g);
	for (auto it = json.begin(); it != json.end(); it++) {
		HSQOBJECT tmp;
		toSquirrel(it->_value, tmp);
		sq_addref(v, &tmp);
		sqsetf(g, it->_key, tmp);
	}
}

void SaveGameManager::loadActors(const Common::JSONObject &json) {
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *a = g_engine->_actors[i];
		if (a->_key.size() > 0) {
			loadActor(a, json[a->_key]->asObject());
		}
	}
}

void SaveGameManager::loadInventory(const Common::JSONValue *json) {
	if (json->isObject()) {
		const Common::JSONObject &jInventory = json->asObject();
		const Common::JSONArray &jSlots = jInventory["slots"]->asArray();
		for (int i = 0; i < NUMACTORS; i++) {
			Object *actor = g_engine->_hud._actorSlots[i].actor;
			if (actor) {
				actor->_inventory.clear();
				const Common::JSONObject &jSlot = jSlots[i]->asObject();
				if (jSlot.contains("objects")) {
					if (jSlot["objects"]->isArray()) {
						const Common::JSONArray &jSlotObjects = jSlot["objects"]->asArray();
						for (int j = 0; j < jSlotObjects.size(); j++) {
							const Common::JSONValue *jObj = jSlotObjects[j];
							Object *obj = object(jObj->asString());
							if (!obj)
								warning("inventory obj '%s' not found", jObj->asString().c_str());
							else
								actor->pickupObject(obj);
						}
					}
					// TODO: "jiggle"
				}
				actor->_inventoryOffset = jSlot["scroll"]->asIntegerNumber();
			}
		}
	}
}

void SaveGameManager::loadRooms(const Common::JSONObject &json) {
	for (auto it = json.begin(); it != json.end(); it++) {
		loadRoom(room(it->_key), it->_value->asObject());
	}
}

void SaveGameManager::loadObjects(const Common::JSONObject &json) {
	for (auto it = json.begin(); it != json.end(); it++) {
		Object *o = object(it->_key);
		if (o)
			loadObject(o, it->_value->asObject());
		else
			warning("object '%s' not found", it->_key.c_str());
	}
}

} // namespace Twp
