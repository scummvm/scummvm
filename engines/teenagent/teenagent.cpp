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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"

#include "backends/audiocd/audiocd.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "graphics/cursorman.h"
#include "graphics/thumbnail.h"

#include "teenagent/console.h"
#include "teenagent/music.h"
#include "teenagent/objects.h"
#include "teenagent/pack.h"
#include "teenagent/scene.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

TeenAgentEngine::TeenAgentEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), action(kActionNone), _gameDescription(gd) {
	music = new MusicPlayer();

	console = 0;
}

TeenAgentEngine::~TeenAgentEngine() {
	delete music;

	delete console;
}

bool TeenAgentEngine::trySelectedObject() {
	InventoryObject *inv = inventory->selectedObject();
	if (inv == NULL)
		return false;

	Resources *res = Resources::instance();
	debug(0, "checking active object %u on %u", inv->id, dst_object->id);

	//mouse time challenge hack:
	if (
		(res->dseg.get_byte(0) == 1 && inv->id == 49 && dst_object->id == 5) ||
		(res->dseg.get_byte(0) == 2 && inv->id == 29 && dst_object->id == 5)
	) {
		//putting rock into hole or superglue on rock
		processCallback(0x8d57);
		return true;
	}

	const Common::Array<UseHotspot> &hotspots = use_hotspots[scene->getId() - 1];
	for (uint i = 0; i < hotspots.size(); ++i) {
		const UseHotspot &spot = hotspots[i];
		if (spot.inventory_id == inv->id && dst_object->id == spot.object_id) {
			debug(0, "use object on hotspot!");
			spot.dump();
			if (spot.actor_x != 0xffff && spot.actor_y != 0xffff)
				moveTo(spot.actor_x, spot.actor_y, spot.orientation);
			if (!processCallback(spot.callback))
				debug(0, "fixme! display proper description");
			inventory->resetSelectedObject();
			return true;
		}
	}

	//error
	inventory->resetSelectedObject();
	displayMessage(0x3457);
	return true;
}

void TeenAgentEngine::processObject() {
	if (dst_object == NULL)
		return;

	Resources *res = Resources::instance();
	switch (action) {
	case kActionExamine: {
		if (trySelectedObject())
			break;

		byte *dcall = res->dseg.ptr(0xb5ce);
		dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
		dcall += 2 * dst_object->id - 2;
		uint16 callback = READ_LE_UINT16(dcall);
		if (callback == 0 || !processCallback(callback))
			displayMessage(dst_object->description);
	}
	break;
	case kActionUse: {
		if (trySelectedObject())
			break;

		byte *dcall = res->dseg.ptr(0xb89c);
		dcall = res->dseg.ptr(READ_LE_UINT16(dcall + scene->getId() * 2 - 2));
		dcall += 2 * dst_object->id - 2;
		uint16 callback = READ_LE_UINT16(dcall);
		if (!processCallback(callback))
			displayMessage(dst_object->description);
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

	action = kActionUse;
	if (object->actor_rect.valid())
		scene->moveTo(Common::Point(object->actor_rect.right, object->actor_rect.bottom), object->actor_orientation);
	else if (object->actor_orientation > 0)
		scene->setOrientation(object->actor_orientation);
}

void TeenAgentEngine::examine(const Common::Point &point, Object *object) {
	if (scene->eventRunning())
		return;

	if (object != NULL) {
		Common::Point dst = object->actor_rect.center();
		debug(0, "click %d, %d, object %d, %d", point.x, point.y, dst.x, dst.y);
		action = kActionExamine;
		if (object->actor_rect.valid())
			scene->moveTo(dst, object->actor_orientation, true); //validate examine message. Original engine does not let you into walkboxes
		dst_object = object;
	} else if (!scene_busy) {
		//do not reset anything while scene is busy, but allow interrupts while walking.
		debug(0, "click %d, %d", point.x, point.y);
		action = kActionNone;
		scene->moveTo(point, 0, true);
		dst_object = NULL;
	}
}

void TeenAgentEngine::init() {
	_mark_delay = 80;
	_game_delay = 110;

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
	CursorMan.popCursor();
}

Common::Error TeenAgentEngine::loadGameState(int slot) {
	debug(0, "loading from slot %d", slot);
	Common::ScopedPtr<Common::InSaveFile> 
		in(_saveFileMan->openForLoading(Common::String::format("teenagent.%02d", slot)));
	if (!in)
		in.reset(_saveFileMan->openForLoading(Common::String::format("teenagent.%d", slot)));

	if (!in)
		return Common::kReadPermissionDenied;

	Resources *res = Resources::instance();

	assert(res->dseg.size() >= 0x6478 + 0x777a);
	char data[0x777a];
	in->seek(0);
	if (in->read(data, 0x777a) != 0x777a) {
		return Common::kReadingFailed;
	}

	memcpy(res->dseg.ptr(0x6478), data, sizeof(data));

	scene->clear();
	inventory->activate(false);
	inventory->reload();

	setMusic(Resources::instance()->dseg.get_byte(0xDB90));

	int id = res->dseg.get_byte(0xB4F3);
	uint16 x = res->dseg.get_word(0x64AF), y = res->dseg.get_word(0x64B1);
	scene->loadObjectData();
	scene->init(id, Common::Point(x, y));
	scene->setPalette(4);
	scene->intro = false;
	return Common::kNoError;
}

Common::Error TeenAgentEngine::saveGameState(int slot, const char *desc) {
	debug(0, "saving to slot %d", slot);
	Common::ScopedPtr<Common::OutSaveFile> out(_saveFileMan->openForSaving(Common::String::format("teenagent.%02d", slot)));
	if (!out)
		return Common::kWritingFailed;

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

	out->finalize();
	return Common::kNoError;
}


int TeenAgentEngine::skipEvents() const {
	Common::EventManager *_event = _system->getEventManager();
	Common::Event event;
	while (_event->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			return -1;
		case Common::EVENT_MAINMENU:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			return 1;
		case Common::EVENT_KEYDOWN:
		if (event.kbd.ascii)
			return 1;
		default:
			break;
		}
	}
	return 0;
}

bool TeenAgentEngine::showCDLogo() {
	Common::File cdlogo;
	if (!cdlogo.exists("cdlogo.res") || !cdlogo.open("cdlogo.res"))
		return true;

	byte bg[0xfa00];
	byte palette[3*256];

	cdlogo.read(bg, sizeof(bg));
	cdlogo.read(palette, sizeof(palette));
	for (uint c = 0; c < 3*256; ++c)
		palette[c] *= 4;
	_system->getPaletteManager()->setPalette(palette, 0, 0x100);
	_system->copyRectToScreen(bg, 320, 0, 0, 320, 200);
	_system->updateScreen();

	for(uint i = 0; i < 20; ++i) {
		int r = skipEvents();
		if (r != 0)
			return r > 0? true: false;
		_system->delayMillis(100);
	}
	cdlogo.close();

	return true;
}

bool TeenAgentEngine::showLogo() {
	FilePack logo;
	if (!logo.open("unlogic.res"))
		return true;

	byte bg[0xfa00];
	byte palette[3*256];

	Common::ScopedPtr<Common::SeekableReadStream> frame(logo.getStream(1));
	if (!frame)
		return true;

	frame->read(bg, sizeof(bg));
	frame->read(palette, sizeof(palette));
	for (uint c = 0; c < 3*256; ++c)
		palette[c] *= 4;
	_system->getPaletteManager()->setPalette(palette, 0, 0x100);

	uint n = logo.fileCount();
	for(uint f = 0; f < 4; ++f)
		for(uint i = 2; i <= n; ++i) {
			{
				int r = skipEvents();
				if (r != 0)
					return r > 0? true: false;
			}
			_system->copyRectToScreen(bg, 320, 0, 0, 320, 200);

			frame.reset(logo.getStream(i));
			if (!frame)
				return true;

			Surface s;
			s.load(frame, Surface::kTypeOns);
			if (s.empty())
				return true;

			_system->copyRectToScreen((const byte *)s.pixels, s.w, s.x, s.y, s.w, s.h);
			_system->updateScreen();

			_system->delayMillis(100);
		}
	return true;
}

bool TeenAgentEngine::showMetropolis() {
	_system->fillScreen(0);
	_system->updateScreen();

	FilePack varia;
	varia.open("varia.res");

	byte palette[3*256];
	{
		Common::ScopedPtr<Common::SeekableReadStream> s(varia.getStream(5));
		s->read(palette, sizeof(palette));
		for (uint c = 0; c < 3*256; ++c)
			palette[c] *= 4;
	}

	_system->getPaletteManager()->setPalette(palette, 0, 0x100);

	byte varia_6[21760], varia_9[18302];
	varia.read(6, varia_6, sizeof(varia_6));
	varia.read(9, varia_9, sizeof(varia_9));

	byte colors[56 * 160 * 2];
	memset(colors, 0, sizeof(colors));

	int logo_y = -56;
	for(uint f = 0; f < 300; ++f) {
		{
			int r = skipEvents();
			if (r != 0)
				return r > 0? true: false;
		}

		Graphics::Surface *surface = _system->lockScreen();
		if (logo_y > 0) {
			surface->fillRect(Common::Rect(0, 0, 320, logo_y), 0);
		}

		{
			//generate colors matrix
			memmove(colors + 320, colors + 480, 8480);
			for(uint c = 0; c < 17; ++c) {
				byte x = (random.getRandomNumber(184) + 5) & 0xff;
				uint offset = 8800 + random.getRandomNumber(158);
				colors[offset++] = x;
				colors[offset++] = x;
			}
			for(uint y = 1; y < 56; ++y) {
				for(uint x = 1; x < 160; ++x) {
					uint offset = y * 160 + x;
					uint v =
						(uint)colors[offset - 161] + colors[offset - 160] + colors[offset - 159] +
						(uint)colors[offset - 1] + colors[offset + 1] +
						(uint)colors[offset + 161] + colors[offset + 160] + colors[offset + 159];
					v >>= 3;
					colors[offset + 8960] = v;
				}
			}
			memmove(colors, colors + 8960, 8960);
		}

		byte *dst = (byte *)surface->getBasePtr(0, 131);
		byte *src = varia_6;
		for(uint y = 0; y < 68; ++y) {
			for(uint x = 0; x < 320; ++x) {
				if (*src++ == 1) {
					*dst++ = colors[19 * 160 + y / 2 * 160 + x / 2];
				} else
					++dst;
			}
		}
		_system->unlockScreen();

		_system->copyRectToScreen(
			varia_9 + (logo_y < 0? -logo_y * 320: 0), 320,
			0, logo_y >= 0? logo_y: 0,
			320, logo_y >= 0? 57: 57 + logo_y);

		if (logo_y < 82 - 57)
			++logo_y;


		_system->updateScreen();
		_system->delayMillis(100);
	}
	return true;
}

Common::Error TeenAgentEngine::run() {
	Resources *res = Resources::instance();
	if (!res->loadArchives(_gameDescription))
		return Common::kUnknownError;

	Common::EventManager *_event = _system->getEventManager();

	initGraphics(320, 200, false);
	console = new Console(this);

	scene = new Scene(this, _system);
	inventory = new Inventory(this);

	init();

	CursorMan.pushCursor(res->dseg.ptr(0x00da), 8, 12, 0, 0, 1);

	syncSoundSettings();

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, music, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, false);
	setMusic(1);
	music->start();

	int load_slot = Common::ConfigManager::instance().getInt("save_slot");
	if (load_slot >= 0) {
		loadGameState(load_slot);
	} else {
		if (!showCDLogo())
			return Common::kNoError;
		if (!showLogo())
			return Common::kNoError;
		if (!showMetropolis())
			return Common::kNoError;
		scene->intro = true;
		scene_busy = true;
		processCallback(0x24c);
	}

	CursorMan.showMouse(true);

	uint32 game_timer = 0;
	uint32 mark_timer = 0;

	Common::Event event;
	Common::Point mouse;
	uint32 timer = _system->getMillis();

	do {
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
				if ((event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) ||
					event.kbd.ascii == '~' || event.kbd.ascii == '#') {
					console->attach();
				} else if (event.kbd.hasFlags(0) && event.kbd.keycode == Common::KEYCODE_F5) {
					openMainMenuDialog();
				} if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_f) {
					_mark_delay = _mark_delay == 80? 40: 80;
					debug(0, "mark_delay = %u", _mark_delay);
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (scene->getId() < 0)
					break;
				examine(event.mouse, current_object);
				break;
			case Common::EVENT_RBUTTONDOWN:
				//if (current_object)
				//	debug(0, "%d, %s", current_object->id, current_object->name.c_str());
				if (scene->getId() < 0)
					break;

				if (current_object == NULL)
					break;

				if (res->dseg.get_byte(0) == 3 && current_object->id == 1) {
					processCallback(0x5189); //boo!
					break;
				}
				if (res->dseg.get_byte(0) == 4 && current_object->id == 5) {
					processCallback(0x99e0); //getting an anchor
					break;
				}
				use(current_object);
				break;
			case Common::EVENT_MOUSEMOVE:
				mouse = event.mouse;
				break;
			default:
				;
			}
		}

		//game delays: slow 16, normal 11, fast 5, crazy 1
		//mark delays: 4 * (3 - hero_speed), normal == 1
		//game delays in 1/100th of seconds
		uint32 new_timer = _system->getMillis();
		uint32 delta = new_timer - timer;
		timer = new_timer;

		bool tick_game = game_timer <= delta;
		if (tick_game)
			game_timer = _game_delay - ((delta - game_timer) % _game_delay);
		else
			game_timer -= delta;

		bool tick_mark = mark_timer <= delta;
		if (tick_mark)
			mark_timer = _mark_delay - ((delta - mark_timer) % _mark_delay);
		else
			mark_timer -= delta;

		if (tick_game || tick_mark) {
			bool b = scene->render(tick_game, tick_mark, delta);
			if (!inventory->active() && !b && action != kActionNone) {
				processObject();
				action = kActionNone;
				dst_object = NULL;
			}
			scene_busy = b;
		}
		_system->showMouse(scene->getMessage().empty() && !scene_busy);

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
#if 0
				if (current_object) {
					current_object->rect.render(surface, 0x80);
					current_object->actor_rect.render(surface, 0x81);
				}
#endif
			}
		}

		inventory->render(surface, tick_game? 1: 0);

		_system->unlockScreen();

		_system->updateScreen();

		console->onFrame();

		uint32 next_tick = MIN(game_timer, mark_timer);
		if (next_tick > 0) {
			_system->delayMillis(next_tick > 40? 40: next_tick);
		}
	} while (!shouldQuit());

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
		message += c != 0 && (signed char)c != -1 ? c : '\n';
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

	if (color == 0xd1) { //mark's
		SceneEvent e(SceneEvent::kPlayAnimation);
		e.animation = 0;
		e.slot = 0x80;
		scene->push(e);
	}

	{
		SceneEvent event(SceneEvent::kMessage);
		event.message = str;
		event.color = color;
		event.slot = 0;
		event.dst.x = position % 320;
		event.dst.y = position / 320;
		scene->push(event);
	}

	{
		SceneEvent e(SceneEvent::kPauseAnimation);
		e.animation = 0;
		e.slot = 0x80;
		scene->push(e);
	}
}

void TeenAgentEngine::displayMessage(uint16 addr, byte color, uint16 position) {
	displayMessage(parseMessage(addr), color, position);
}

void TeenAgentEngine::displayAsyncMessage(uint16 addr, uint16 position, uint16 first_frame, uint16 last_frame, byte color) {
	SceneEvent event(SceneEvent::kMessage);
	event.message = parseMessage(addr);
	event.slot = 0;
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
	event.slot = slot + 1;
	event.color = color;
	event.first_frame = first_frame;
	event.last_frame = last_frame;

	scene->push(event);
}


void TeenAgentEngine::displayCredits(uint16 addr, uint16 timer) {
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
	event.timer = timer;
	scene->push(event);
}

void TeenAgentEngine::displayCredits() {
	SceneEvent event(SceneEvent::kCredits);
	event.message = parseMessage(0xe488);
	event.dst.y = 200;

	int lines = 1;
	for(uint i = 0; i < event.message.size(); ++i)
		if (event.message[i] == '\n')
			++lines;
	event.dst.x = (320 - Resources::instance()->font7.render(NULL, 0, 0, event.message, 0xd1)) / 2;
	event.timer = 11 * lines - event.dst.y + 22;
	//debug(0, "credits = %s", event.message.c_str());
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

void TeenAgentEngine::playAnimation(uint16 id, byte slot, bool async, bool ignore, bool loop) {
	SceneEvent event(SceneEvent::kPlayAnimation);
	event.animation = id;
	event.slot = (slot + 1) | (ignore? 0x20: 0) | (loop? 0x80: 0);
	scene->push(event);
	if (!async)
		waitAnimation();
}

void TeenAgentEngine::playActorAnimation(uint16 id, bool async, bool ignore) {
	SceneEvent event(SceneEvent::kPlayActorAnimation);
	event.animation = id;
	event.slot = ignore? 0x20: 0;
	scene->push(event);
	if (!async)
		waitAnimation();
}


void TeenAgentEngine::loadScene(byte id, const Common::Point &pos, byte o) {
	loadScene(id, pos.x, pos.y, o);
}

void TeenAgentEngine::loadScene(byte id, uint16 x, uint16 y, byte o) {
	if (scene->last_event_type() != SceneEvent::kCreditsMessage)
		fadeOut();

	SceneEvent event(SceneEvent::kLoadScene);
	event.scene = id;
	event.dst.x = x;
	event.dst.y = y;
	event.orientation = o;
	scene->push(event);
	fadeIn();
}

void TeenAgentEngine::enableOn(bool enable) {
	SceneEvent event(SceneEvent::kSetOn);
	event.ons = 0;
	event.color = enable? 1: 0;
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

void TeenAgentEngine::setFlag(uint16 addr, byte value) {
	SceneEvent event(SceneEvent::kSetFlag);
	event.callback = addr;
	event.color = value;
	scene->push(event);
}

byte TeenAgentEngine::getFlag(uint16 addr) {
	return scene->peekFlagEvent(addr);
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

void TeenAgentEngine::shakeScreen() {
	SceneEvent event(SceneEvent::kEffect);
	scene->push(event);
}

void TeenAgentEngine::fadeIn() {
	SceneEvent event(SceneEvent::kFade);
	event.orientation = 0;
	scene->push(event);
}

void TeenAgentEngine::fadeOut() {
	SceneEvent event(SceneEvent::kFade);
	event.orientation = 1;
	scene->push(event);
}

void TeenAgentEngine::wait(uint16 frames) {
	SceneEvent event(SceneEvent::kWait);
	event.timer = frames * 10;
	scene->push(event);
}

void TeenAgentEngine::playSoundNow(byte id) {
	Resources *res = Resources::instance();
	uint size = res->sam_sam.getSize(id);
	if (size == 0) {
		warning("skipping invalid sound %u", id);
		return;
	}

	byte *data = (byte *)malloc(size);
	res->sam_sam.read(id, data, size);
	//debug(0, "playing %u samples...", size);

	Audio::AudioStream *stream = Audio::makeRawStream(data, size, 11025, 0);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream); //dispose is YES by default
}


void TeenAgentEngine::setMusic(byte id) {
	debug(0, "starting music %u", id);
	Resources *res = Resources::instance();
	
	if (id != 1) //intro music
		*res->dseg.ptr(0xDB90) = id;
	
	if (_gameDescription->flags & ADGF_CD) {
		byte track2cd[] = {7, 2, 0, 9, 3, 6, 8, 10, 4, 5, 11};
		if (id == 0 || id > 11 || track2cd[id - 1] == 0) {
			debug(0, "no cd music for id %u", id);
			return;
		}
		byte track = track2cd[id - 1];
		debug(0, "playing cd track %u", track);
		_system->getAudioCDManager()->play(track, -1, 0, 0);
	} else if (music->load(id))
		music->start();
}

bool TeenAgentEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsRTL:
	case kSupportsSubtitleOptions:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

} // End of namespace TeenAgent
