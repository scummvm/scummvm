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

#include "teenagent/scene.h"
#include "teenagent/resources.h"
#include "teenagent/surface.h"
#include "common/debug.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"
#include "teenagent/dialog.h"
#include "teenagent/music.h"

namespace TeenAgent {

Scene::Scene() : intro(false), _engine(NULL),
		_system(NULL),
		_id(0), ons(0),
		orientation(Object::kActorRight),
		current_event(SceneEvent::kNone), hide_actor(false) {}

void Scene::warp(const Common::Point &_point, byte o) {
	Common::Point point(_point);
	destination = position = position0 = point;
	progress = 0;
	progress_total = 1;
	if (o)
		orientation = o;
}

void Scene::moveTo(const Common::Point &_point, byte orient, bool validate) {
	Common::Point point(_point);
	debug(0, "moveTo(%d, %d, %u)", point.x, point.y, orient);
	if (validate) {
		const Common::Array<Walkbox> &scene_walkboxes = walkboxes[_id - 1];
		for (byte i = 0; i < scene_walkboxes.size(); ++i) {
			const Walkbox &w = scene_walkboxes[i];
			if (w.rect.in(point)) {
				debug(0, "bumped into walkbox %u", i);
				byte o = w.orientation;
				switch (o) {
				case 1:
					point.y = w.rect.top - 1;
					break;
				case 2:
					point.x = w.rect.right + 1;
					break;
				case 3:
					point.y = w.rect.bottom + 1;
					break;
				case 4:
					point.x = w.rect.left - 1;
					break;
				default:
					return;
				}
			}
		}
	}
	if (point == position) {
		if (orient != 0)
			orientation = orient;
		nextEvent();
		return;
	}
	destination = point;
	orientation = orient;
	position0 = position;
	progress_total = 1 + (int)(sqrt((float)position.sqrDist(destination)) / 10);
	progress = 0;
}


void Scene::init(TeenAgentEngine *engine, OSystem *system) {
	_engine = engine;
	_system = system;

	Resources *res = Resources::instance();
	Common::SeekableReadStream *s = res->varia.getStream(1);
	if (s == NULL)
		error("invalid resource data");

	teenagent.load(s, Animation::kTypeVaria);
	if (teenagent.empty())
		error("invalid mark animation");

	s = res->varia.getStream(2);
	if (s == NULL)
		error("invalid resource data");

	teenagent_idle.load(s, Animation::kTypeVaria);
	if (teenagent_idle.empty())
		error("invalid mark animation");
		
	//loading objects & walkboxes
	objects.resize(42);
	walkboxes.resize(42);
	
	for(byte i = 0; i < 42; ++i) {
		Common::Array<Object> &scene_objects = objects[i];
		scene_objects.clear();
		
		uint16 scene_table = res->dseg.get_word(0x7254 + i * 2);
		uint16 object_addr;
		while ((object_addr = res->dseg.get_word(scene_table)) != 0) {
			Object obj;
			obj.load(res->dseg.ptr(object_addr));
			//obj.dump();
			scene_objects.push_back(obj);
			scene_table += 2;
		}
		debug(0, "scene[%u] has %u object(s)", i + 1, scene_objects.size());

		byte *walkboxes_base = res->dseg.ptr(READ_LE_UINT16(res->dseg.ptr(0x6746 + i * 2)));
		byte walkboxes_n = *walkboxes_base++;
		debug(0, "scene[%u] has %u walkboxes", i + 1, walkboxes_n);

		Common::Array<Walkbox> &scene_walkboxes = walkboxes[i];
		for (byte j = 0; j < walkboxes_n; ++j) {
			Walkbox w;
			w.load(walkboxes_base + 14 * j);
			//walkbox[i]->dump();
			scene_walkboxes.push_back(w);
		}
	}
}

Object *Scene::findObject(const Common::Point &point) {
	if (_id == 0)
		return NULL;
	
	Common::Array<Object> &scene_objects = objects[_id - 1];
	
	for(uint i = 0; i < scene_objects.size(); ++i) {
		Object &obj = scene_objects[i];
		if (obj.enabled != 0 && obj.rect.in(point))
			return &obj;
	}
	return NULL;
}



byte *Scene::getOns(int id) {
	Resources *res = Resources::instance();
	return res->dseg.ptr(res->dseg.get_word(0xb4f5 + (id - 1) * 2));
}

byte *Scene::getLans(int id) {
	Resources *res = Resources::instance();
	return res->dseg.ptr(0xd89e + (id - 1) * 4);
}

void Scene::loadOns() {
	debug(0, "loading ons animation");
	Resources *res = Resources::instance();

	uint16 addr = res->dseg.get_word(0xb4f5 + (_id - 1) * 2);
	//debug(0, "ons index: %04x", addr);

	ons_count = 0;
	byte b;
	byte on_id[16];
	while ((b = res->dseg.get_byte(addr)) != 0xff) {
		debug(0, "on: %04x = %02x", addr, b);
		++addr;
		if (b == 0)
			continue;

		on_id[ons_count++] = b;
	}

	delete[] ons;
	ons = NULL;

	if (ons_count > 0) {
		ons = new Surface[ons_count];
		for (uint32 i = 0; i < ons_count; ++i) {
			Common::SeekableReadStream *s = res->ons.getStream(on_id[i]);
			if (s != NULL)
				ons[i].load(s, Surface::kTypeOns);
		}
	}
}

void Scene::loadLans() {
	debug(0, "loading lans animation");
	Resources *res = Resources::instance();
	//load lan000

	for (byte i = 0; i < 4; ++i) {
		animation[i].free();

		uint16 bx = 0xd89e + (_id - 1) * 4 + i;
		byte bxv = res->dseg.get_byte(bx);
		uint16 res_id = 4 * (_id - 1) + i + 1;
		debug(0, "lan[%u]@%04x = %02x, resource id: %u", i, bx, bxv, res_id);
		if (bxv == 0)
			continue;

		Common::SeekableReadStream *s = res->loadLan000(res_id);
		if (s != NULL) {
			animation[i].load(s, Animation::kTypeLan);
			if (bxv != 0 && bxv != 0xff)
				animation[i].id = bxv;
			delete s;
		}

		//uint16 bp = res->dseg.get_word();
	}

}

void Scene::init(int id, const Common::Point &pos) {
	debug(0, "init(%d)", id);
	_id = id;

	if (background.pixels == NULL)
		background.create(320, 200, 1);

	warp(pos);

	Resources *res = Resources::instance();
	res->loadOff(background, palette, id);
	if (id == 24) {
		//dark scene
		if (res->dseg.get_byte(0xDBA4) != 1) {
			//dim down palette
			uint i;
			for (i = 0; i < 624; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
			for (i = 726; i < 768; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
		}
	}
	setPalette(_system, palette, 4);

	Common::SeekableReadStream *stream = res->on.getStream(id);
	on.load(stream, Surface::kTypeOn);
	delete stream;

	loadOns();
	loadLans();

	//check music
	int now_playing = _engine->music->getId();

	if (now_playing != res->dseg.get_byte(0xDB90))
		_engine->music->load(res->dseg.get_byte(0xDB90));
}

void Scene::playAnimation(byte idx, uint id, bool loop, bool paused) {
	assert(idx < 4);
	Common::SeekableReadStream *s = Resources::instance()->loadLan(id + 1);
	if (s == NULL)
		error("playing animation %u failed", id);

	custom_animation[idx].load(s);
	custom_animation[idx].loop = loop;
	custom_animation[idx].paused = paused;
}

void Scene::playActorAnimation(uint id, bool loop) {
	Common::SeekableReadStream *s = Resources::instance()->loadLan(id + 1);
	if (s == NULL)
		error("playing animation %u failed", id);

	actor_animation.load(s);
	actor_animation.loop = loop;
}

void Scene::push(const SceneEvent &event) {
	//debug(0, "push");
	//event.dump();
	events.push_back(event);
}

bool Scene::processEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		if (!message.empty()) {
			message.clear();
			nextEvent();
			return true;
		}
		return false;

	case Common::EVENT_KEYUP:
		if (intro && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			intro = false;
			message.clear();
			events.clear();
			sounds.clear();
			current_event.clear();
			message_color = 0xd1;
			for (int i = 0; i < 4; ++i)
				custom_animation[i].free();
			_engine->playMusic(4);
			init(10, Common::Point(136, 153));
		}
		return false;

	default:
		return false;
	}
}

bool Scene::render(OSystem *system) {
	//render background
	Resources *res = Resources::instance();
	if (current_event.type == SceneEvent::kCreditsMessage) {
		system->fillScreen(0);
		Graphics::Surface *surface = system->lockScreen();
		res->font8.color = current_event.color;
		res->font8.shadow_color = current_event.orientation;
		res->font8.render(surface, current_event.dst.x, current_event.dst.y, message);
		system->unlockScreen();
		return true;
	}

	bool busy;
	bool restart;

	do {
		restart = false;
		busy = processEventQueue();
		system->copyRectToScreen((const byte *)background.pixels, background.pitch, 0, 0, background.w, background.h);

		Graphics::Surface *surface = system->lockScreen();

		if (ons != NULL) {
			for (uint32 i = 0; i < ons_count; ++i) {
				Surface *s = ons + i;
				if (s != NULL)
					s->render(surface);
			}
		}

		//render on
		if (on.pixels != NULL) {
			on.render(surface);
		}

		bool got_any_animation = false;

		for (byte i = 0; i < 4; ++i) {
			Animation *a = custom_animation + i;
			Surface *s = a->currentFrame();
			if (s != NULL) {
				animation_position[i] = s->render(surface);
				busy = true;
				got_any_animation = true;
				continue;
			}

			a = animation + i;
			s = a->currentFrame();
			if (s == NULL)
				continue;

			animation_position[i] = s->render(surface);

			if (a->id == 0)
				continue;

			Object *obj = getObject(a->id);
			if (obj != NULL) {
				obj->rect.left = s->x;
				obj->rect.top = s->y;
				obj->rect.right = s->w + s->x;
				obj->rect.bottom = s->h + s->y;
				//obj->dump();
			}
		}

		{
			Surface *mark = actor_animation.currentFrame();
			if (mark != NULL) {
				actor_animation_position = mark->render(surface);
				busy = true;
				got_any_animation = true;
			} else if (!hide_actor) {
				actor_animation.free();

				if (destination != position) {
					Common::Point dp(destination.x - position0.x, destination.y - position0.y);
					int o;
					if (ABS(dp.x) > ABS(dp.y))
						o = dp.x > 0 ? Object::kActorRight : Object::kActorLeft;
					else
						o = dp.y > 0 ? Object::kActorDown : Object::kActorUp;

					position.x = position0.x + dp.x * progress / progress_total;
					position.y = position0.y + dp.y * progress / progress_total;
					actor_animation_position = teenagent.render(surface, position, o, 1);
					++progress;
					if (progress >= progress_total) {
						position = destination;
						if (orientation == 0)
							orientation = o; //save last orientation
						nextEvent();
						got_any_animation = true;
						restart = true;
					} else
						busy = true;
				} else
					actor_animation_position = teenagent.render(surface, position, orientation, 0);
			}
		}

		if (!message.empty()) {
			res->font7.color = message_color;
			res->font7.render(surface, message_pos.x, message_pos.y, message);
			busy = true;
		}

		system->unlockScreen();

		if (current_event.type == SceneEvent::kWaitForAnimation && !got_any_animation) {
			debug(0, "no animations, nextevent");
			nextEvent();
			restart = true;
		}

		//if (!current_event.empty())
		//	current_event.dump();
		/*
		{
			const Common::Array<Walkbox> & scene_walkboxes = walkboxes[_id - 1];
			for (uint i = 0; i < scene_walkboxes.size(); ++i) {
				scene_walkboxes[i].rect.render(surface, 0xd0 + i);
			}
		}
		*/

	} while (restart);

	for (Sounds::iterator i = sounds.begin(); i != sounds.end();) {
		Sound &sound = *i;
		if (sound.delay == 0) {
			debug(0, "sound %u started", sound.id);
			_engine->playSoundNow(sound.id);
			i = sounds.erase(i);
		} else {
			--sound.delay;
			++i;
		}
	}

	return busy;
}

bool Scene::processEventQueue() {
	while (!events.empty() && current_event.empty()) {
		//debug(0, "processing next event");
		current_event = events.front();
		events.pop_front();
		switch (current_event.type) {

		case SceneEvent::kSetOn: {
			byte *ptr = getOns(current_event.scene == 0 ? _id : current_event.scene);
			debug(0, "on[%u] = %02x", current_event.ons - 1, current_event.color);
			ptr[current_event.ons - 1] = current_event.color;
			loadOns();
			current_event.clear();
		}
		break;

		case SceneEvent::kSetLan: {
			if (current_event.lan != 0) {
				debug(0, "lan[%u] = %02x", current_event.lan - 1, current_event.color);
				byte *ptr = getLans(current_event.scene == 0 ? _id : current_event.scene);
				ptr[current_event.lan - 1] = current_event.color;
			}
			loadLans();
			current_event.clear();
		}
		break;

		case SceneEvent::kLoadScene: {
			init(current_event.scene, current_event.dst);
			sounds.clear();
			current_event.clear();
		}
		break;

		case SceneEvent::kWalk: {
			Common::Point dst = current_event.dst;
			if ((current_event.color & 2) != 0) { //relative move
				dst.x += position.x;
				dst.y += position.y;
			}
			if ((current_event.color & 1) != 0) {
				warp(dst, current_event.orientation);
				current_event.clear();
			} else
				moveTo(dst, current_event.orientation);
		}
		break;

		case SceneEvent::kCreditsMessage:
		case SceneEvent::kMessage: {
				message = current_event.message;
				Common::Point p(
					(actor_animation_position.left + actor_animation_position.right) / 2, 
					actor_animation_position.top 
				);
				//FIXME: rewrite it:
				if (current_event.lan < 4) {
					const Surface *s = custom_animation[current_event.lan].currentFrame(0);
					if (s == NULL)
						s = animation[current_event.lan].currentFrame(0);
					if (s != NULL) {
						p.x = s->x + s->w / 2;
						p.y = s->y;
					} else 
						warning("no animation in slot %u", current_event.lan);
				}
				message_pos = messagePosition(message, p);
				message_color = current_event.color;
			}
			break;

		case SceneEvent::kPlayAnimation:
			debug(0, "playing animation %u in slot %u", current_event.animation, current_event.lan & 3);
			playAnimation(current_event.lan & 3, current_event.animation, (current_event.lan & 0x80) != 0, (current_event.lan & 0x40) != 0);
			current_event.clear();
			break;

		case SceneEvent::kPauseAnimation:
			debug(0, "pause animation in slot %u", current_event.lan & 3);
			custom_animation[current_event.lan & 3].paused = (current_event.lan & 0x80) != 0;
			current_event.clear();
			break;

		case SceneEvent::kClearAnimations:
			for (byte i = 0; i < 4; ++i)
				custom_animation[i].free();
			current_event.clear();
			break;

		case SceneEvent::kPlayActorAnimation:
			debug(0, "playing actor animation %u", current_event.animation);
			playActorAnimation(current_event.animation, (current_event.lan & 0x80) != 0);
			current_event.clear();
			break;

		case SceneEvent::kPlayMusic:
			debug(0, "setting music %u", current_event.music);
			_engine->setMusic(current_event.music);
			Resources::instance()->dseg.set_byte(0xDB90, current_event.music);
			current_event.clear();
			break;

		case SceneEvent::kPlaySound:
			debug(0, "playing sound %u, delay: %u", current_event.sound, current_event.color);
			sounds.push_back(Sound(current_event.sound, current_event.color));
			current_event.clear();
			break;

		case SceneEvent::kEnableObject: {
			debug(0, "%s object #%u", current_event.color ? "enabling" : "disabling", current_event.object - 1);
			Object *obj = getObject(current_event.object - 1, current_event.scene == 0 ? _id : current_event.scene);
			obj->enabled = current_event.color;
			current_event.clear();
		}
		break;

		case SceneEvent::kHideActor:
			hide_actor = current_event.color != 0;
			current_event.clear();
			break;

		case SceneEvent::kWaitForAnimation:
			debug(0, "waiting for the animation");
			break;

		case SceneEvent::kQuit:
			debug(0, "quit!");
			_engine->quitGame();
			break;

		default:
			error("empty/unhandler event[%d]", (int)current_event.type);
		}
	}
	if (events.empty()) {
		message_color = 0xd1;
		hide_actor = false;
	}
	return !current_event.empty();
}

void Scene::setPalette(OSystem *system, const byte *buf, unsigned mul) {
	byte p[1024];

	memset(p, 0, 1024);
	for (int i = 0; i < 256; ++i) {
		for (int c = 0; c < 3; ++c)
			p[i * 4 + c] = buf[i * 3 + c] * mul;
	}

	system->setPalette(p, 0, 256);
}

Object *Scene::getObject(int id, int scene_id) {
	assert(id > 0);
	
	if (scene_id == 0)
		scene_id = _id;

	if (scene_id == 0)
		return NULL;

	return &objects[scene_id - 1][id - 1];
}

Common::Point Scene::messagePosition(const Common::String &str, Common::Point position) {
	Resources *res = Resources::instance();
	uint w = res->font7.render(NULL, 0, 0, str);
	position.x -= w / 2;
	position.y -= res->font7.height + 3;
	if (position.x + w > 320)
		position.x = 320 - w;
	if (position.x < 0)
		position.x = 0;

	return position;
}

void Scene::displayMessage(const Common::String &str, byte color) {
	debug(0, "displayMessage: %s", str.c_str());
	message = str;
	message_pos = messagePosition(str, position);
	message_color = color;
}

void Scene::clear() {
	events.clear();
	current_event.clear();
}

} // End of namespace TeenAgent
