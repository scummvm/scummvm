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
#include "engines/advancedDetector.h"
#include "sound/mixer.h"
#include "graphics/thumbnail.h"
#include "teenagent/scene.h"
#include "teenagent/objects.h"
#include "teenagent/music.h"
#include "teenagent/console.h"

namespace TeenAgent {

TeenAgentEngine::TeenAgentEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), action(kActionNone), _gameDescription(gd) {
	music = new MusicPlayer();
}

void TeenAgentEngine::processObject() {
	if (dst_object == NULL)
		return;

	Resources *res = Resources::instance();
	switch (action) {
	case kActionExamine: {
		byte *dcall = res->dseg.ptr(0xb5ce);
		dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
		dcall += 2 * dst_object->id - 2;
		uint16 callback = READ_LE_UINT16(dcall);
		if (callback == 0 || !processCallback(callback)) {
			Common::String desc = dst_object->description;
			scene->displayMessage(desc);
			//debug(0, "%s[%u]: description: %s", current_object->name, current_object->id, desc.c_str());
		}
	}
	break;
	case kActionUse: {
		InventoryObject *inv = inventory->selectedObject();
		if (inv != NULL) {
			debug(0, "checking active object %u on %u", inv->id, dst_object->id);
			
			//mouse time challenge hack:
			if (
				(res->dseg.get_byte(0) == 1 && inv->id == 49 && dst_object->id == 5) ||
				(res->dseg.get_byte(0) == 2 && inv->id == 29 && dst_object->id == 5)
			) {
				//putting rock into hole or superglue on rock
				processCallback(0x8d57);
				return;
			}
			
			
			const Common::Array<UseHotspot> &hotspots = use_hotspots[scene->getId() - 1];
			for (uint i = 0; i < hotspots.size(); ++i) {
				const UseHotspot &spot = hotspots[i];
				if (spot.inventory_id == inv->id && dst_object->id == spot.object_id) {
					debug(0, "use object on hotspot!");
					spot.dump();
					if (spot.actor_x != 0xffff && spot.actor_y != 0xffff)
						moveTo(spot.actor_x, spot.actor_y, spot.orientation);
					inventory->resetSelectedObject();
					if (!processCallback(TO_LE_16(spot.callback)))
						debug(0, "fixme! display proper description");
					return;
				}
			}

			//error
			inventory->resetSelectedObject();
			displayMessage(0x3457);

			break;
		} else {
			byte *dcall = res->dseg.ptr(0xb89c);
			dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
			dcall += 2 * dst_object->id - 2;
			uint16 callback = READ_LE_UINT16(dcall);
			if (!processCallback(callback))
				scene->displayMessage(dst_object->description);
		}
	}
	break;

	case kActionNone:
		break;
	}
}


void TeenAgentEngine::use(Object *object) {
	if (object == NULL || scene->eventRunning())
		return;

	dst_object = object;
	object->rect.dump();
	object->actor_rect.dump();

	if (object->actor_rect.valid())
		scene->moveTo(Common::Point(object->actor_rect.right, object->actor_rect.bottom), object->actor_orientation);
	if (object->actor_orientation > 0)
		scene->setOrientation(object->actor_orientation);
	action = kActionUse;
}

void TeenAgentEngine::examine(const Common::Point &point, Object *object) {
	if (scene->eventRunning())
		return;

	if (object != NULL) {
		Common::Point dst = object->actor_rect.center();
		debug(0, "click %d, %d, object %d, %d", point.x, point.y, dst.x, dst.y);
		if (object->actor_rect.valid())
			scene->moveTo(dst, object->actor_orientation);
		action = kActionExamine;
		dst_object = object;
	} else {
		debug(0, "click %d, %d", point.x, point.y);
		scene->moveTo(point, 0, true);
		dst_object = NULL;
		action = kActionNone;
	}
}

void TeenAgentEngine::init() {
	Resources * res = Resources::instance();
	use_hotspots.resize(42);
	byte *scene_hotspots = res->dseg.ptr(0xbb87);
	for (byte i = 0; i < 42; ++i) {
		Common::Array<UseHotspot> & hotspots = use_hotspots[i];
		byte * hotspots_ptr = res->dseg.ptr(READ_LE_UINT16(scene_hotspots + i * 2));
		while (*hotspots_ptr) {
			UseHotspot h;
			h.load(hotspots_ptr);
			hotspots_ptr += 9;
			hotspots.push_back(h);
		}
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
	use_hotspots.clear();
	Resources::instance()->deinit();
}

Common::Error TeenAgentEngine::loadGameState(int slot) {
	debug(0, "loading from slot %d", slot);
	Common::InSaveFile *in = _saveFileMan->openForLoading(Common::String::printf("teenagent.%02d", slot));
	if (in == NULL)
		in = _saveFileMan->openForLoading(Common::String::printf("teenagent.%d", slot));

	if (in == NULL)
		return Common::kReadPermissionDenied;

	Resources *res = Resources::instance();

	assert(res->dseg.size() >= 0x6478 + 0x777a);
	char data[0x777a];
	in->seek(0);
	if (in->read(data, 0x777a) != 0x777a) {
		delete in;
		return Common::kReadingFailed;
	}

	delete in;

	memcpy(res->dseg.ptr(0x6478), data, sizeof(data));

	scene->clear();
	inventory->activate(false);

	setMusic(Resources::instance()->dseg.get_byte(0xDB90));

	int id = res->dseg.get_byte(0xB4F3);
	uint16 x = res->dseg.get_word(0x64AF), y = res->dseg.get_word(0x64B1);
	scene->loadObjectData();
	scene->init(id, Common::Point(x, y));
	return Common::kNoError;
}

Common::Error TeenAgentEngine::saveGameState(int slot, const char *desc) {
	debug(0, "saving to slot %d", slot);
	Common::OutSaveFile *out = _saveFileMan->openForSaving(Common::String::printf("teenagent.%02d", slot));
	if (out == NULL)
		return Common::kWritePermissionDenied;

	Resources *res = Resources::instance();
	res->dseg.set_byte(0xB4F3, scene->getId());
	Common::Point pos = scene->getPosition();
	res->dseg.set_word(0x64AF, pos.x);
	res->dseg.set_word(0x64B1, pos.y);

	assert(res->dseg.size() >= 0x6478 + 0x777a);
	strncpy((char *)res->dseg.ptr(0x6478), desc, 0x16);
	out->write(res->dseg.ptr(0x6478), 0x777a);
	if (!Graphics::saveThumbnail(*out))
		warning("saveThumbnail failed");
	delete out;

	return Common::kNoError;
}

Common::Error TeenAgentEngine::run() {
	Resources *res = Resources::instance();
	if (!res->loadArchives(_gameDescription))
		return Common::kUnknownError;

	Common::EventManager *_event = _system->getEventManager();

	initGraphics(320, 200, false);

	scene = new Scene;
	inventory = new Inventory;
	console = new Console(this);

	scene->init(this, _system);
	inventory->init(this);
	
	init();

	_system->setMouseCursor(res->dseg.ptr(0x00da), 8, 12, 0, 0, 1);

	syncSoundSettings();

	music->load(1);
	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, music, -1, 255, 0, true, false);
	music->start();

	{
		int load_slot = Common::ConfigManager::instance().getInt("save_slot");
		debug(0, "slot: %d", load_slot);
		if (load_slot >= 0) {
			loadGameState(load_slot);
		} else {
			scene->intro = true;
			scene_busy = true;
			processCallback(0x24c);
		}
	}

	uint32 frame = 0;

	Common::Event event;
	Common::Point mouse;

	do {
		_system->showMouse(true);
		uint32 t0 = _system->getMillis();
		Object *current_object = scene->findObject(mouse);

		while (_event->pollEvent(event)) {
			if (event.type == Common::EVENT_RTL) {
				deinit();
				return Common::kNoError;
			}

			if ((!scene_busy && inventory->processEvent(event)) || scene->processEvent(event))
				continue;

			//debug(0, "event");
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if ((event.kbd.flags == Common::KBD_CTRL && event.kbd.keycode == 'd') ||
					event.kbd.ascii == '~' || event.kbd.ascii == '#') {
					console->attach();
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				examine(event.mouse, current_object);
				break;
			case Common::EVENT_RBUTTONDOWN:
				use(current_object);
				break;
			case Common::EVENT_MOUSEMOVE:
				mouse = event.mouse;
				break;
			default:
				;
			}
		}

		uint32 f0 = frame * 10 / 25, f1 = (frame + 1) * 10 / 25;
		if (f0 != f1) {
			bool b = scene->render(_system);
			scene_busy = b;
			if (!inventory->active() && !scene_busy && action != kActionNone) {
				processObject();
				action = kActionNone;
				dst_object = NULL;
			}
		}
		bool busy = inventory->active() || scene_busy;

		Graphics::Surface *surface = _system->lockScreen();

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

				uint w = res->font7.render(NULL, 0, 0, name, 0xd1);
				res->font7.render(surface, (320 - w) / 2, 180, name, 0xd1, true);
				if (current_object) {
					//current_object->rect.render(surface, 0x80);
					//current_object->actor_rect.render(surface, 0x81);
				}
			}
		}

		inventory->render(surface);

		_system->unlockScreen();

		_system->updateScreen();

		if (console->isAttached()) {
			console->onFrame();
		}

		uint32 dt = _system->getMillis() - t0;
		if (dt < 40)
			_system->delayMillis(40 - dt);

		++frame;
	} while (!_event->shouldQuit());

	deinit();
	return Common::kNoError;
}

Common::String TeenAgentEngine::parseMessage(uint16 addr) {
	Common::String message;
	for (
	    const char *str = (const char *)Resources::instance()->dseg.ptr(addr);
	    str[0] != 0 || str[1] != 0;
	    ++str) {
		char c = str[0];
		message += c != 0 && c != -1 ? c : '\n';
	}
	if (message.empty()) {
		warning("empty message parsed for %04x", addr);
	}
	return message;
}

void TeenAgentEngine::displayMessage(const Common::String &str, byte color, uint16 position) {
	if (str.empty()) {
		return;
	}
	SceneEvent event(SceneEvent::kMessage);
	event.message = str;
	event.color = color;
	event.slot = 4;
	event.dst.x = position % 320;
	event.dst.y = position / 320;

	scene->push(event);
}

void TeenAgentEngine::displayMessage(uint16 addr, byte color, uint16 position) {
	displayMessage(parseMessage(addr), color, position);
}

void TeenAgentEngine::displayAsyncMessage(uint16 addr, uint16 position, uint16 first_frame, uint16 last_frame, byte color) {
	SceneEvent event(SceneEvent::kMessage);
	event.message = parseMessage(addr);
	event.slot = 4;
	event.color = color;
	event.dst.x = position % 320;
	event.dst.y = position / 320;
	event.first_frame = first_frame;
	event.last_frame = last_frame;

	scene->push(event);
}

void TeenAgentEngine::displayAsyncMessageInSlot(uint16 addr, byte slot, uint16 first_frame, uint16 last_frame, byte color) {
	SceneEvent event(SceneEvent::kMessage);
	event.message = parseMessage(addr);
	event.slot = slot;
	event.color = color;
	event.first_frame = first_frame;
	event.last_frame = last_frame;

	scene->push(event);
}


void TeenAgentEngine::displayCredits(uint16 addr) {
	SceneEvent event(SceneEvent::kCreditsMessage);

	const byte *src = Resources::instance()->dseg.ptr(addr);
	event.orientation = *src++;
	event.color = *src++;
	event.lan = 8;

	event.dst.y = *src;
	while (true) {
		++src; //skip y position
		Common::String line((const char *)src);
		event.message += line;
		src += line.size() + 1;
		if (*src == 0)
			break;
		event.message += "\n";
	}
	int w = Resources::instance()->font8.render(NULL, 0, 0, event.message, 0xd1);
	event.dst.x = (320 - w) / 2;
	scene->push(event);
}

void TeenAgentEngine::displayCutsceneMessage(uint16 addr, uint16 position) {
	SceneEvent event(SceneEvent::kCreditsMessage);

	event.message = parseMessage(addr);
	event.dst.x = position % 320;
	event.dst.y = position / 320;
	event.lan = 7;
	
	scene->push(event);
}


void TeenAgentEngine::moveTo(const Common::Point &dst, byte o, bool warp) {
	moveTo(dst.x, dst.y, o, warp);
}

void TeenAgentEngine::moveTo(Object *obj) {
	moveTo(obj->actor_rect.right, obj->actor_rect.bottom, obj->actor_orientation);
}

void TeenAgentEngine::moveTo(uint16 x, uint16 y, byte o, bool warp) {
	SceneEvent event(SceneEvent::kWalk);
	event.dst.x = x;
	event.dst.y = y;
	if (o > 4) {
		warning("invalid orientation %d", o);
		o = 0;
	}
	event.orientation = o;
	event.color = warp ? 1 : 0;
	scene->push(event);
}

void TeenAgentEngine::moveRel(int16 x, int16 y, byte o, bool warp) {
	SceneEvent event(SceneEvent::kWalk);
	event.dst.x = x;
	event.dst.y = y;
	event.orientation = o;
	event.color = (warp ? 1 : 0) | 2;
	scene->push(event);
}

void TeenAgentEngine::playAnimation(uint16 id, byte slot, bool async) {
	SceneEvent event(SceneEvent::kPlayAnimation);
	event.animation = id;
	event.slot = slot;
	scene->push(event);
	if (!async)
		waitAnimation();
}

void TeenAgentEngine::playActorAnimation(uint16 id, bool async) {
	SceneEvent event(SceneEvent::kPlayActorAnimation);
	event.animation = id;
	scene->push(event);
	if (!async)
		waitAnimation();
}


void TeenAgentEngine::loadScene(byte id, const Common::Point &pos, byte o) {
	loadScene(id, pos.x, pos.y, o);
}

void TeenAgentEngine::loadScene(byte id, uint16 x, uint16 y, byte o) {
	SceneEvent event(SceneEvent::kLoadScene);
	event.scene = id;
	event.dst.x = x;
	event.dst.y = y;
	event.orientation = o;
	scene->push(event);
}

void TeenAgentEngine::setOns(byte id, byte value, byte scene_id) {
	SceneEvent event(SceneEvent::kSetOn);
	event.ons = id + 1;
	event.color = value;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::setLan(byte id, byte value, byte scene_id) {
	if (id == 0)
		error("setting lan 0 is invalid");
	SceneEvent event(SceneEvent::kSetLan);
	event.lan = id;
	event.color = value;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::reloadLan() {
	SceneEvent event(SceneEvent::kSetLan);
	event.lan = 0;
	scene->push(event);
}


void TeenAgentEngine::playMusic(byte id) {
	SceneEvent event(SceneEvent::kPlayMusic);
	event.music = id;
	scene->push(event);
}

void TeenAgentEngine::playSound(byte id, byte skip_frames) {
	if (skip_frames > 0)
		--skip_frames;
	SceneEvent event(SceneEvent::kPlaySound);
	event.sound = id;
	event.color = skip_frames;
	scene->push(event);
}

void TeenAgentEngine::enableObject(byte id, byte scene_id) {
	SceneEvent event(SceneEvent::kEnableObject);
	event.object = id + 1;
	event.color = 1;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::disableObject(byte id, byte scene_id) {
	SceneEvent event(SceneEvent::kEnableObject);
	event.object = id + 1;
	event.color = 0;
	event.scene = scene_id;
	scene->push(event);
}

void TeenAgentEngine::hideActor() {
	SceneEvent event(SceneEvent::kHideActor);
	event.color = 1;
	scene->push(event);
}

void TeenAgentEngine::showActor() {
	SceneEvent event(SceneEvent::kHideActor);
	event.color = 0;
	scene->push(event);
}

void TeenAgentEngine::waitAnimation() {
	SceneEvent event(SceneEvent::kWaitForAnimation);
	scene->push(event);
}

void TeenAgentEngine::waitLanAnimationFrame(byte slot, uint16 frame) {
	SceneEvent event(SceneEvent::kWaitLanAnimationFrame);
	if (frame > 0)
		--frame;
	
	event.slot = slot - 1;
	event.animation = frame;
	scene->push(event);
}

void TeenAgentEngine::setTimerCallback(uint16 addr, uint16 frames) {
	SceneEvent event(SceneEvent::kTimer);
	event.callback = addr;
	event.timer = frames;
	scene->push(event);
}

void TeenAgentEngine::playSoundNow(byte id) {
	Resources *res = Resources::instance();
	Common::SeekableReadStream *in = res->sam_sam.getStream(id);
	if (in == NULL) {
		debug(0, "skipping invalid sound %u", id);
		return;
	}

	uint size = in->size();
	char *data = new char[size];
	in->read(data, size);
	//debug(0, "playing %u samples...", size);

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
	switch (f) {
	case kSupportsRTL:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

} // End of namespace TeenAgent
