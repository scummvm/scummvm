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
 * $URL$
 * $Id$
 */


#include "teenagent/teenagent.h"
#include "common/system.h"
#include "common/events.h"
#include "common/debug.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "sound/mixer.h"
#include "scene.h"
#include "objects.h"
#include "music.h"

using namespace TeenAgent;

TeenAgentEngine::TeenAgentEngine(OSystem * system) : Engine(system), action(ActionNone) {
	music = new MusicPlayer();
}

void TeenAgentEngine::processObject() {
	if (dst_object == NULL)
		return;

	Resources * res = Resources::instance();
	switch (action) {
	case ActionExamine: {
		byte * dcall = res->dseg.ptr(0xb5ce);
		dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
		dcall += 2 * dst_object->id - 2;
		uint16 callback = READ_LE_UINT16(dcall);
		if (callback == 0) {
			Common::String desc = dst_object->description();
			scene->displayMessage(desc);
			//debug(0, "%s[%u]: description: %s", current_object->name, current_object->id, desc.c_str());
			break;
		}
		processCallback(callback);
		}
		break;
	case ActionUse:
		{
			InventoryObject *inv = inventory->selectedObject();
			if (inv != NULL) {
				byte * dcall = res->dseg.ptr(0xbb87);
				dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
				for(UseObject * obj = (UseObject *)dcall; obj->inventory_id != 0; ++obj) {
					if (obj->inventory_id == inv->id && dst_object->id == obj->object_id) {
						debug(0, "combine! %u,%u", obj->x, obj->y);
						//moveTo(Common::Point(obj->x, obj->y), NULL, Examine);
						processCallback(obj->callback);
						inventory->resetSelectedObject();
						return;
					}
				}
				
				//error
				inventory->resetSelectedObject();
				scene->displayMessage("That's no good.");
				
				break;
			} else {
				byte * dcall = res->dseg.ptr(0xb89c);
				dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
				dcall += 2 * dst_object->id - 2;
				uint16 callback = READ_LE_UINT16(dcall);
				processCallback(callback);
			}
		}
		break;

	case ActionNone:
		break;
	}
}


void TeenAgentEngine::use(Object *object) {
	if (object == NULL || scene->eventRunning())
		return;

	dst_object = object;
	object->rect.dump();
	object->actor_rect.dump();
	if (object->actor_rect.valid()) //some objects have 0xffff in left/right
		scene->moveTo(object->actor_rect.center(), object->actor_orientation);
	if (object->actor_orientation > 0)
		scene->setOrientation(object->actor_orientation);
	action = ActionUse;
}

void TeenAgentEngine::examine(const Common::Point &point, Object *object) {
	if (scene->eventRunning())
		return;
	
	if (object != NULL) {
		Common::Point dst = object->actor_rect.center();
		debug(0, "click %d, %d, object %d, %d", point.x, point.y, dst.x, dst.y);
		if (object->actor_rect.valid())
			scene->moveTo(dst, object->actor_orientation);
		action = ActionExamine;
		dst_object = object;
	} else {
		debug(0, "click %d, %d", point.x, point.y);
		scene->moveTo(point, 0, true);
		dst_object = NULL;
		action = ActionNone;
	}
}

void TeenAgentEngine::deinit() {
	_mixer->stopAll();
	delete scene;
	scene = NULL;
	delete inventory;
	inventory = NULL;
	//delete music;
	//music = NULL;
	Resources::instance()->deinit();
}

Common::Error TeenAgentEngine::loadGameState(int slot) {
	debug(0, "loading from slot %d", slot);
	char slotStr[16];
	snprintf(slotStr, sizeof(slotStr), "teenagent.%d", slot);
	Common::InSaveFile *in = _saveFileMan->openForLoading(slotStr);
	if (in == NULL)
		return Common::kReadPermissionDenied;

	Resources * res = Resources::instance();

	assert(res->dseg.size() >= 0x6478 + 0x777a);
	char data[0x777a];
	if (in->read(data, 0x777a) != 0x777a) {
		delete in;
		return Common::kReadingFailed;
	}
	
	delete in;
	
	memcpy(res->dseg.ptr(0x6478), data, sizeof(data));
	
	scene->clear();

	setMusic(Resources::instance()->dseg.get_byte(0xDB90));

	int id = res->dseg.get_byte(0xB4F3);
	uint16 x = res->dseg.get_word(0x64AF), y = res->dseg.get_word(0x64B1);
	scene->init(id, Common::Point(x, y));
	return Common::kNoError;
}

Common::Error TeenAgentEngine::saveGameState(int slot, const char *desc) {
	debug(0, "saving to slot %d", slot);
	char slotStr[16];
	snprintf(slotStr, sizeof(slotStr), "teenagent.%d", slot);
	Common::OutSaveFile *out = _saveFileMan->openForSaving(slotStr);
	if (out == NULL)
		return Common::kWritePermissionDenied;

	Resources * res = Resources::instance();
	res->dseg.set_byte(0xB4F3, scene->getId());
	Common::Point pos = scene->getPosition();
	res->dseg.set_word(0x64AF, pos.x);
	res->dseg.set_word(0x64B1, pos.y);

	assert(res->dseg.size() >= 0x6478 + 0x777a);
	strncpy((char *)res->dseg.ptr(0x6478), desc, 0x16);
	out->write(res->dseg.ptr(0x6478), 0x777a);
	delete out;
	return Common::kNoError;
}

Common::Error TeenAgentEngine::run() {
	Common::EventManager * _event = _system->getEventManager();

	initGraphics(320, 200, false);
	
	scene = new Scene;
	inventory = new Inventory;
	
	Resources * res = Resources::instance();
	res->loadArchives();
	
	
	scene->init(this, _system);
	inventory->init(this);

	//res.varia.read(6, palette, sizeof(palette));
	//14,15 - on type 2
	//scene->init(_system, 2);
	//scene->init(_system, 27);
	//scene->init(_system, 28);
	scene->init(10, Common::Point(136, 153));
	//scene->init(_system, 1);
	_system->disableCursorPalette(true);
	
	syncSoundSettings();
	
	music->load(4);
	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, music, -1, 255, 0, true, false);
	music->start();
	
	{
		int load_slot = Common::ConfigManager::instance().getInt("save_slot");
		debug(0, "slot: %d", load_slot);
		if (load_slot >= 0)
			loadGameState(load_slot);
	}

	uint32 frame = 0;
	
	Common::Event event;
	Common::Point mouse;
	
	do {
		_system->showMouse(true);
		uint32 t0 = _system->getMillis();
		Object * current_object = findObject(scene->getId(), mouse);
		
		while (_event->pollEvent(event)) {
			if (event.type == Common::EVENT_RTL) {
				deinit();
				return Common::kNoError;
			}
		
			if ((!scene_busy && inventory->processEvent(event)) || scene->processEvent(event)) 
				continue;
			
			//debug(0, "event");
			switch(event.type) {
			case Common::EVENT_LBUTTONDOWN:
				examine(event.mouse, current_object);
				break;
			case Common::EVENT_RBUTTONDOWN:
				use(current_object);
				break;
			case Common::EVENT_MOUSEMOVE:
				mouse = event.mouse;
				break;
			default:;
			}
		}
		
		uint32 f0 = frame * 10 / 25, f1 = (frame + 1) * 10 / 25;
		if (f0 != f1) {
			bool b = scene->render(_system);
			scene_busy = b;
			if (!inventory->active() && !scene_busy && action != ActionNone) {
				processObject();
				action = ActionNone;
				dst_object = NULL;
			}
		}
		bool busy = inventory->active() || scene_busy;
		
		Graphics::Surface * surface = _system->lockScreen();

		if (!busy) {
			InventoryObject *selected_object = inventory->selectedObject();
			if (current_object || selected_object) {
				Common::String name;
				if (selected_object) {
					name += selected_object->name;
					name += " & ";
				}
				if (current_object)
					name += current_object->name;
				
				uint w = res->font7.render(NULL, 0, 0, name);
				res->font7.render(surface, (320 - w) / 2, 180, name, true);
				if (current_object) {
					current_object->rect.render(surface, 0x80);
					current_object->actor_rect.render(surface, 0x81);
				}
			}
		}
		
		inventory->render(surface);
		
		_system->unlockScreen();
		
		_system->updateScreen();
		
		uint32 dt = _system->getMillis() - t0;
		if (dt < 40)
			_system->delayMillis(40 - dt);
		
		++frame;
	} while(!_event->shouldQuit());

	deinit();
	return Common::kNoError;
}

Object * TeenAgentEngine::findObject(int id, const Common::Point &point) {
	Resources * res = Resources::instance();
	uint16 addr = res->dseg.get_word(0x7254 + (id - 1) * 2);
	//debug(0, "object base: %04x, x: %d, %d", addr, point.x, point.y);
	uint16 object;
	for(;(object = res->dseg.get_word(addr)) != 0; addr += 2) {
		if (object == 0)
			return NULL;
		
		Object *obj = (Object *)res->dseg.ptr(object);
		//obj->dump();
		if (obj->enabled != 0 && obj->rect.in(point))
			return obj;
	}
	return NULL;
}

void TeenAgentEngine::displayMessage(const Common::String &str, byte color) {
	SceneEvent event(SceneEvent::Message);
	event.message = str;
	event.color = color;
	scene->push(event);
}


void TeenAgentEngine::displayMessage(uint16 addr, byte color) {
	Common::String message;
	for (
		const char * str = (const char *)Resources::instance()->dseg.ptr(addr);
		str[0] != 0 || str[1] != 0; 
		++str) 
	{
		char c = str[0];
		message += c != 0 && c != -1? c: '\n';
	}
	displayMessage(message, color);
}

void TeenAgentEngine::moveTo(const Common::Point & dst, bool warp) {
	moveTo(dst.x, dst.y);
}

void TeenAgentEngine::moveTo(uint16 x, uint16 y, bool warp) {
	SceneEvent event(SceneEvent::Walk);
	event.dst.x = x;
	event.dst.y = y;
	event.color = warp? 1: 0;
	scene->push(event);
}

void TeenAgentEngine::playAnimation(uint16 id, byte slot, bool async) {
	SceneEvent event(SceneEvent::PlayAnimation);
	event.animation = id;
	event.color = slot;
	if (async)
		event.color |= 0x80;
	scene->push(event);
}

void TeenAgentEngine::loadScene(byte id, const Common::Point &pos, byte o) {
	loadScene(id, pos.x, pos.y, o);
}

void TeenAgentEngine::loadScene(byte id, uint16 x, uint16 y, byte o) {
	SceneEvent event(SceneEvent::LoadScene);
	event.scene = id;
	event.dst.x = x;
	event.dst.y = y;
	event.orientation = o;
	scene->push(event);
}

void TeenAgentEngine::setOns(byte id, byte value, byte scene_id) {
	SceneEvent event(SceneEvent::SetOn);
	event.ons = id + 1;
	event.color = value;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::setLan(byte id, byte value, byte scene_id) {
	if (id == 0)
		error("setting lan 0 is invalid");
	SceneEvent event(SceneEvent::SetLan);
	event.lan = id;
	event.color = value;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::reloadLan() {
	SceneEvent event(SceneEvent::SetLan);
	event.lan = 0;
	scene->push(event);
}


void TeenAgentEngine::playMusic(byte id) {
	SceneEvent event(SceneEvent::PlayMusic);
	event.music = id;
	scene->push(event);
}

void TeenAgentEngine::playSound(byte id, byte skip_frames) {
	SceneEvent event(SceneEvent::PlaySound);
	event.sound = id;
	event.color = skip_frames;
	scene->push(event);
}

void TeenAgentEngine::enableObject(byte id, byte scene_id) {
	SceneEvent event(SceneEvent::EnableObject);
	event.object = id + 1;
	event.color = 1;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::disableObject(byte id, byte scene_id) {
	SceneEvent event(SceneEvent::EnableObject);
	event.object = id + 1;
	event.color = 0;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::waitAnimation() {
	SceneEvent event(SceneEvent::WaitForAnimation);
	scene->push(event);
}

void TeenAgentEngine::playSoundNow(byte id) {
	Resources * res = Resources::instance();
	Common::SeekableReadStream * in = res->sam_sam.getStream(id);
	if (in == NULL) {
		debug(0, "skipping invalid sound %u", id);
		return;
	}

	uint size = in->size();
	char *data = new char[size];
	in->read(data, size);
	debug(0, "playing %u samples...", size);
	
	_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_soundHandle, data, size, 11025, Audio::Mixer::FLAG_AUTOFREE);
}


void TeenAgentEngine::setMusic(byte id) {
	debug(0, "starting music %u", id);
	if (!music->load(id))
		return;
	*Resources::instance()->dseg.ptr(0xDB90) = id;
	music->start();
}


bool TeenAgentEngine::hasFeature(EngineFeature f) const {
	switch(f) {
	case kSupportsRTL:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}
