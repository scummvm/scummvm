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
 */

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/algorithm.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#include "graphics/palette.h"

#include "teenagent/scene.h"
#include "teenagent/inventory.h"
#include "teenagent/resources.h"
#include "teenagent/surface.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"
#include "teenagent/music.h"

namespace TeenAgent {

Scene::Scene(TeenAgentEngine *vm) : _vm(vm), intro(false), _id(0), ons(0),
	orientation(kActorRight), actor_talking(false), teenagent(vm), teenagent_idle(vm),
	message_timer(0), message_first_frame(0), message_last_frame(0), message_animation(NULL),
	current_event(SceneEvent::kNone), hide_actor(false), callback(0), callback_timer(0), _idle_timer(0) {

	_fade_timer = 0;
	_fadeOld = 0;
	on_enabled = true;

	memset(palette, 0, sizeof(palette));
	background.pixels = 0;

	FilePack varia;
	varia.open("varia.res");

	Common::ScopedPtr<Common::SeekableReadStream> s(varia.getStream(1));
	if (!s)
		error("invalid resource data");

	teenagent.load(*s, Animation::kTypeVaria);
	if (teenagent.empty())
		error("invalid mark animation");

	s.reset(varia.getStream(2));
	if (!s)
		error("invalid resource data");

	teenagent_idle.load(*s, Animation::kTypeVaria);
	if (teenagent_idle.empty())
		error("invalid mark animation");

	varia.close();
	loadObjectData();
}

Scene::~Scene() {
	if (background.pixels)
		background.free();

	delete[] ons;
	ons = 0;
}

void Scene::warp(const Common::Point &_point, byte o) {
	Common::Point point(_point);
	position = point;
	path.clear();
	if (o)
		orientation = o;
}

bool Scene::findPath(Scene::Path &p, const Common::Point &src, const Common::Point &dst) const {
	const Common::Array<Walkbox> &scene_walkboxes = walkboxes[_id - 1];
	if (dst.x < 0 || dst.x >= screenWidth || dst.y < 0 || dst.y >= screenHeight)
		return false;

	debugC(1, kDebugScene, "findPath %d,%d -> %d,%d", src.x, src.y, dst.x, dst.y);
	p.clear();
	p.push_back(src);
	p.push_back(dst);

	Common::List<uint> boxes;
	for (uint i = 0; i < scene_walkboxes.size(); ++i) {
		const Walkbox &w = scene_walkboxes[i];
		if (!w.rect.in(src) && !w.rect.in(dst))
			boxes.push_back(i);
	}

	for (Path::iterator i = p.begin(); i != p.end() && !boxes.empty();) {
		Path::iterator next = i;
		++next;
		if (next == p.end())
			break;

		const Common::Point &p1 = *i, &p2 = *next;
		debugC(1, kDebugScene, "%d,%d -> %d,%d", p1.x, p1.y, p2.x, p2.y);

		Common::List<uint>::iterator wi;
		for (wi = boxes.begin(); wi != boxes.end(); ++wi) {
			const Walkbox &w = scene_walkboxes[*wi];
			int mask = w.rect.intersects_line(p1, p2);
			if (mask == 0) {
				continue;
			}

			w.dump(1);
			debugC(1, kDebugScene, "%u: intersection mask 0x%04x, searching hints", *wi, mask);
			int dx = p2.x - p1.x, dy = p2.y - p1.y;
			if (dx >= 0) {
				if ((mask & 8) != 0 && w.side_hint[3] != 0) {
					debugC(1, kDebugScene, "hint left: %u", w.side_hint[3]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.side_hint[3], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 2)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			} else {
				if ((mask & 2) != 0 && w.side_hint[1] != 0) {
					debugC(1, kDebugScene, "hint right: %u", w.side_hint[1]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.side_hint[1], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 8)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			}

			if (dy >= 0) {
				if ((mask & 1) != 0 && w.side_hint[0] != 0) {
					debugC(1, kDebugScene, "hint top: %u", w.side_hint[0]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.side_hint[0], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 4)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			} else {
				if ((mask & 4) != 0 && w.side_hint[2] != 0) {
					debugC(1, kDebugScene, "hint bottom: %u", w.side_hint[2]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.side_hint[2], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 1)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			}
		}
		if (wi == boxes.end())
			++i;
	}
	p.pop_front();
	return true;
}

void Scene::moveTo(const Common::Point &_point, byte orient, bool validate) {
	Common::Point point(_point);
	debugC(0, kDebugScene, "moveTo(%d, %d, %u)", point.x, point.y, orient);
	const Common::Array<Walkbox> &scene_walkboxes = walkboxes[_id - 1];

	for (byte i = 0; i < scene_walkboxes.size(); ++i) {
		const Walkbox &w = scene_walkboxes[i];
		if (w.rect.in(point)) {
			debugC(0, kDebugScene, "bumped into walkbox %u", i);
			w.dump();
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
				if (validate)
					return;
			}
		}
	}
	if (point == position) {
		if (orient != 0)
			orientation = orient;
		nextEvent();
		return;
	}

	path.clear();
	if (scene_walkboxes.empty()) {
		path.push_back(point);
		return;
	}

	if (!findPath(path, position, point)) {
		_vm->cancel();
		return;
	}

	orientation = orient;
}

void Scene::loadObjectData() {
	//loading objects & walkboxes
	objects.resize(42);
	walkboxes.resize(42);
	fades.resize(42);

	for (byte i = 0; i < 42; ++i) {
		Common::Array<Object> &scene_objects = objects[i];
		scene_objects.clear();

		uint16 scene_table = _vm->res->dseg.get_word(dsAddr_sceneObjectTablePtr + (i * 2));
		uint16 object_addr;
		while ((object_addr = _vm->res->dseg.get_word(scene_table)) != 0) {
			Object obj;
			obj.load(_vm->res->dseg.ptr(object_addr));
			//obj.dump();
			scene_objects.push_back(obj);
			scene_table += 2;
		}
		debugC(0, kDebugScene, "scene[%u] has %u object(s)", i + 1, scene_objects.size());

		byte *walkboxes_base = _vm->res->dseg.ptr(READ_LE_UINT16(_vm->res->dseg.ptr(dsAddr_sceneWalkboxTablePtr + i * 2)));
		byte walkboxes_n = *walkboxes_base++;
		debugC(0, kDebugScene, "scene[%u] has %u walkboxes", i + 1, walkboxes_n);

		Common::Array<Walkbox> &scene_walkboxes = walkboxes[i];
		for (byte j = 0; j < walkboxes_n; ++j) {
			Walkbox w;
			w.load(walkboxes_base + 14 * j);
			if ((w.side_hint[0] | w.side_hint[1] | w.side_hint[2] | w.side_hint[3]) == 0) {
				w.side_hint[0] = 2;
				w.side_hint[1] = 3;
				w.side_hint[2] = 4;
				w.side_hint[3] = 1;
			}
			//walkbox[i]->dump();
			scene_walkboxes.push_back(w);
		}

		byte *fade_table = _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_sceneFadeTablePtr + i * 2));
		Common::Array<FadeType> &scene_fades = fades[i];
		while (READ_LE_UINT16(fade_table) != 0xffff) {
			FadeType fade;
			fade.load(fade_table);
			fade_table += 9;
			scene_fades.push_back(fade);
		}
		debugC(0, kDebugScene, "scene[%u] has %u fadeboxes", i + 1, scene_fades.size());
	}
}

Object *Scene::findObject(const Common::Point &point) {
	if (_id == 0)
		return NULL;

	Common::Array<Object> &scene_objects = objects[_id - 1];

	for (uint i = 0; i < scene_objects.size(); ++i) {
		Object &obj = scene_objects[i];
		if (obj.enabled != 0 && obj.rect.in(point))
			return &obj;
	}
	return NULL;
}

byte *Scene::getOns(int id) {
	return _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_onsAnimationTablePtr + (id - 1) * 2));
}

byte *Scene::getLans(int id) {
	return _vm->res->dseg.ptr(dsAddr_lansAnimationTablePtr + (id - 1) * 4);
}

void Scene::loadOns() {
	debugC(0, kDebugScene, "loading ons animation");

	uint16 addr = _vm->res->dseg.get_word(dsAddr_onsAnimationTablePtr + (_id - 1) * 2);
	debugC(0, kDebugScene, "ons index: %04x", addr);

	ons_count = 0;
	byte b;
	byte on_id[16];
	while ((b = _vm->res->dseg.get_byte(addr)) != 0xff) {
		debugC(0, kDebugScene, "on: %04x = %02x", addr, b);
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
			Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->ons.getStream(on_id[i]));
			if (s) {
				ons[i].load(*s, Surface::kTypeOns);
			}
		}
	}
}

void Scene::loadLans() {
	debugC(0, kDebugScene, "loading lans animation");
	// load lan000

	for (byte i = 0; i < 4; ++i) {
		animation[i].free();

		uint16 bx = dsAddr_lansAnimationTablePtr + (_id - 1) * 4 + i;
		byte bxv = _vm->res->dseg.get_byte(bx);
		uint16 res_id = 4 * (_id - 1) + i + 1;
		debugC(0, kDebugScene, "lan[%u]@%04x = %02x, resource id: %u", i, bx, bxv, res_id);
		if (bxv == 0)
			continue;

		Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan000(res_id));
		if (s) {
			animation[i].load(*s, Animation::kTypeLan);
			if (bxv != 0 && bxv != 0xff)
				animation[i].id = bxv;
		}
	}
}

void Scene::init(int id, const Common::Point &pos) {
	debugC(0, kDebugScene, "init(%d)", id);
	_id = id;
	on_enabled = true; //reset on-rendering flag on loading.
	sounds.clear();
	for (byte i = 0; i < 4; ++i)
		custom_animation[i].free();

	if (background.pixels == NULL)
		background.create(screenWidth, screenHeight, Graphics::PixelFormat::createFormatCLUT8());

	warp(pos);

	_vm->res->loadOff(background, palette, id);
	if (id == 24) {
		// dark scene
		if (_vm->res->dseg.get_byte(dsAddr_lightOnFlag) != 1) {
			// dim down palette
			uint i;
			for (i = 0; i < 624; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
			for (i = 726; i < 768; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
		}
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->res->on.getStream(id));
	int sub_hack = 0;
	if (id == 7) { // something patched in the captains room
		switch (_vm->res->dseg.get_byte(dsAddr_captainDrawerState)) {
		case 2:
			break;
		case 1:
			sub_hack = 1;
			break;
		default:
			sub_hack = 2;
		}
	}
	on.load(*stream, SurfaceList::kTypeOn, sub_hack);

	loadOns();
	loadLans();

	// check music
	int now_playing = _vm->music->getId();

	if (now_playing != _vm->res->dseg.get_byte(dsAddr_currentMusic))
		_vm->music->load(_vm->res->dseg.get_byte(dsAddr_currentMusic));

	_vm->_system->copyRectToScreen(background.pixels, background.pitch, 0, 0, background.w, background.h);
	setPalette(0);
}

void Scene::playAnimation(byte idx, uint id, bool loop, bool paused, bool ignore) {
	debugC(0, kDebugScene, "playAnimation(%u, %u, loop:%s, paused:%s, ignore:%s)", idx, id, loop ? "true" : "false", paused ? "true" : "false", ignore ? "true" : "false");
	assert(idx < 4);
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan(id + 1));
	if (!s)
		error("playing animation %u failed", id);

	custom_animation[idx].load(*s);
	custom_animation[idx].loop = loop;
	custom_animation[idx].paused = paused;
	custom_animation[idx].ignore = ignore;
}

void Scene::playActorAnimation(uint id, bool loop, bool ignore) {
	debugC(0, kDebugScene, "playActorAnimation(%u, loop:%s, ignore:%s)", id, loop ? "true" : "false", ignore ? "true" : "false");
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan(id + 1));
	if (!s)
		error("playing animation %u failed", id);

	actor_animation.load(*s);
	actor_animation.loop = loop;
	actor_animation.ignore = ignore;
	actor_animation.id = id;
}

Animation *Scene::getAnimation(byte slot) {
	assert(slot < 4);
	return custom_animation + slot;
}

byte Scene::peekFlagEvent(uint16 addr) const {
	for (EventList::const_iterator i = events.reverse_begin(); i != events.end(); --i) {
		const SceneEvent &e = *i;
		if (e.type == SceneEvent::kSetFlag && e.callback == addr)
			return e.color;
	}
	return _vm->res->dseg.get_byte(addr);
}

void Scene::push(const SceneEvent &event) {
	debugC(0, kDebugScene, "push");
	//event.dump();
	if (event.type == SceneEvent::kWalk && !events.empty()) {
		SceneEvent &prev = events.back();
		if (prev.type == SceneEvent::kWalk && prev.color == event.color) {
			debugC(0, kDebugScene, "fixing double-move [skipping event!]");
			if ((event.color & 2) != 0) { // relative move
				prev.dst.x += event.dst.x;
				prev.dst.y += event.dst.y;
			} else {
				prev.dst = event.dst;
			}
			return;
		}
	}
	events.push_back(event);
}

bool Scene::processEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		if (!message.empty() && message_first_frame == 0) {
			clearMessage();
			nextEvent();
			return true;
		}
		return false;

	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_SPACE: {
			if (intro && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				intro = false;
				clearMessage();
				events.clear();
				sounds.clear();
				current_event.clear();
				message_color = 0xd1;
				for (int i = 0; i < 4; ++i)
					custom_animation[i].free();
				_vm->playMusic(4);
				_vm->loadScene(10, Common::Point(136, 153));
				return true;
			}

			if (!message.empty() && message_first_frame == 0) {
				clearMessage();
				nextEvent();
				return true;
			}
			break;
		}
#if 0
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (event.kbd.flags & Common::KBD_CTRL) {
				uint feature = event.kbd.keycode - '1';
				if (feature < DebugFeatures::kMax) {
					debug_features.feature[feature] = !debug_features.feature[feature];
					debugC(0, kDebugScene, "switched feature %u %s", feature, debug_features.feature[feature] ? "on" : "off");
				}
			}
			break;
#endif
		default:
			break;
		}

	default:
		return false;
	}
}

struct ZOrderCmp {
	inline bool operator()(const Surface *a, const Surface *b) const {
		return a->y + a->h < b->y + b->h;
	}
};

int Scene::lookupZoom(uint y) const {
	debugC(2, kDebugScene, "lookupZoom(%d)", y);
	for (byte *zoom_table = _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_sceneZoomTablePtr + (_id - 1) * 2));
	        zoom_table[0] != 0xff && zoom_table[1] != 0xff;
	        zoom_table += 2) {
		debugC(2, kDebugScene, "\t%d %d->%d", y, zoom_table[0], zoom_table[1]);
		if (y <= zoom_table[0]) {
			return 256u * (100 - zoom_table[1]) / 100;
		}
	}
	return 256;
}

void Scene::paletteEffect(byte step) {
	byte *src = _vm->res->dseg.ptr(dsAddr_paletteEffectData);
	byte *dst = palette + (3 * 242);
	for (byte i = 0; i < 13; ++i) {
		for (byte c = 0; c < 3; ++c, ++src)
			*dst++ = *src > step ? *src - step : 0;
	}
}

byte Scene::findFade() const {
	if (_id <= 0)
		return 0;

	const Common::Array<FadeType> &scene_fades = fades[_id - 1];
	for (uint i = 0; i < scene_fades.size(); ++i) {
		const FadeType &fade = scene_fades[i];
		if (fade.rect.in(position)) {
			return fade.value;
		}
	}
	return 0;
}

bool Scene::render(bool tick_game, bool tick_mark, uint32 delta) {
	bool busy;
	bool restart;
	uint32 game_delta = tick_game ? 1 : 0;
	uint32 mark_delta = tick_mark ? 1 : 0;

	do {
		restart = false;
		busy = processEventQueue();

		if (_fade_timer && game_delta != 0) {
			if (_fade_timer > 0) {
				_fade_timer -= game_delta;
				setPalette(_fade_timer);
			} else {
				_fade_timer += game_delta;
				setPalette(_fade_timer + 4);
			}
		}

		switch (current_event.type) {
		case SceneEvent::kCredits: {
			_vm->_system->fillScreen(0);
			// TODO: optimize me
			Graphics::Surface *surface = _vm->_system->lockScreen();
			_vm->res->font7.render(surface, current_event.dst.x, current_event.dst.y -= game_delta, current_event.message, current_event.color);
			_vm->_system->unlockScreen();

			if (current_event.dst.y < -(int)current_event.timer)
				current_event.clear();
			}
			return true;
		default:
			;
		}

		if (!message.empty() && message_timer != 0) {
			if (message_timer <= delta) {
				clearMessage();
				nextEvent();
				continue;
			} else
				message_timer -= delta;
		}

		if (current_event.type == SceneEvent::kCreditsMessage) {
			_vm->_system->fillScreen(0);
			Graphics::Surface *surface = _vm->_system->lockScreen();
			if (current_event.lan == 8) {
				_vm->res->font8.shadow_color = current_event.orientation;
				_vm->res->font8.render(surface, current_event.dst.x, current_event.dst.y, message, current_event.color);
			} else {
				_vm->res->font7.render(surface, current_event.dst.x, current_event.dst.y, message, 0xd1);
			}
			_vm->_system->unlockScreen();
			return true;
		}

		if (background.pixels && debug_features.feature[DebugFeatures::kShowBack]) {
			_vm->_system->copyRectToScreen(background.pixels, background.pitch, 0, 0, background.w, background.h);
		} else
			_vm->_system->fillScreen(0);

		Graphics::Surface *surface = _vm->_system->lockScreen();

		bool got_any_animation = false;

		if (ons != NULL && debug_features.feature[DebugFeatures::kShowOns]) {
			for (uint32 i = 0; i < ons_count; ++i) {
				Surface *s = ons + i;
				if (s != NULL)
					s->render(surface);
			}
		}

		Common::List<Surface *> z_order;

		for (byte i = 0; i < 4; ++i) {
			Animation *a = custom_animation + i;
			Surface *s = a->currentFrame(game_delta);
			if (s != NULL) {
				if (!a->ignore)
					busy = true;
				if (!a->paused && !a->loop)
					got_any_animation = true;
			} else {
				a = animation + i;
				if (!custom_animation[i].empty()) {
					debugC(0, kDebugScene, "custom animation ended, restart animation in the same slot.");
					custom_animation[i].free();
					a->restart();
				}
				s = a->currentFrame(game_delta);
			}

			if (current_event.type == SceneEvent::kWaitLanAnimationFrame && current_event.slot == i) {
				if (s == NULL) {
					restart |= nextEvent();
					continue;
				}
				int index = a->currentIndex();
				if (index == current_event.animation) {
					debugC(0, kDebugScene, "kWaitLanAnimationFrame(%d, %d) complete", current_event.slot, current_event.animation);
					restart |= nextEvent();
				}
			}

			if (s == NULL)
				continue;

			if (debug_features.feature[DebugFeatures::kShowLan])
				z_order.push_back(s);

			if (a->id == 0)
				continue;

			Object *obj = getObject(a->id);
			if (obj != NULL) {
				obj->rect.left = s->x;
				obj->rect.top = s->y;
				obj->rect.right = s->w + s->x;
				obj->rect.bottom = s->h + s->y;
				obj->rect.save();
				//obj->dump();
			}
		}

		Common::sort(z_order.begin(), z_order.end(), ZOrderCmp());
		Common::List<Surface *>::iterator z_order_it;

		Surface *mark = actor_animation.currentFrame(game_delta);
		int horizon = position.y;

		for (z_order_it = z_order.begin(); z_order_it != z_order.end(); ++z_order_it) {
			Surface *s = *z_order_it;
			if (s->y + s->h > horizon)
				break;
			s->render(surface);
		}

		if (mark != NULL) {
			actor_animation_position = mark->render(surface);
			if (!actor_animation.ignore)
				busy = true;
			else
				busy = false;
			got_any_animation = true;
		} else if (!hide_actor) {
			actor_animation.free();
			uint zoom = lookupZoom(position.y);

			byte fade = findFade();
			if (fade != _fadeOld) {
				_fadeOld = fade;
				paletteEffect(fade);
				if (_fade_timer == 0)
					setPalette(4);
			}

			if (!path.empty()) {
				const Common::Point &destination = path.front();
				Common::Point dp(destination.x - position.x, destination.y - position.y);

				int o;
				if (ABS(dp.x) > ABS(dp.y))
					o = dp.x > 0 ? kActorRight : kActorLeft;
				else {
					o = dp.y > 0 ? kActorDown : kActorUp;
				}

				if (tick_mark) {
					int speed_x = zoom / 32; // 8 * zoom / 256
					int speed_y = (o == kActorDown || o == kActorUp ? 2 : 1) * zoom / 256;
					if (speed_x == 0)
						speed_x = 1;
					if (speed_y == 0)
						speed_y = 1;

					position.y += (ABS(dp.y) < speed_y ? dp.y : SIGN(dp.y) * speed_y);
					position.x += (o == kActorDown || o == kActorUp) ?
					              (ABS(dp.x) < speed_y ? dp.x : SIGN(dp.x) * speed_y) :
					              (ABS(dp.x) < speed_x ? dp.x : SIGN(dp.x) * speed_x);
				}

				_idle_timer = 0;
				teenagent_idle.resetIndex();
				actor_animation_position = teenagent.render(surface, position, o, mark_delta, false, zoom);

				if (tick_mark && position == destination) {
					path.pop_front();
					if (path.empty()) {
						if (orientation == 0)
							orientation = o; // save last orientation
						nextEvent();
						got_any_animation = true;
						restart = true;
					}
					busy = true;
				} else
					busy = true;
			} else {
				teenagent.resetIndex();
				_idle_timer += mark_delta;
				if (_idle_timer < 50)
					actor_animation_position = teenagent.render(surface, position, orientation, 0, actor_talking, zoom);
				else
					actor_animation_position = teenagent_idle.renderIdle(surface, position, orientation, mark_delta, zoom, _vm->_rnd);
			}
		}

		if (restart) {
			_vm->_system->unlockScreen();
			continue;
		}
		// removed mark == null. In final scene of chapter 2 mark rendered above table.
		// if it'd cause any bugs, add hack here. (_id != 23 && mark == NULL)
		if (on_enabled && debug_features.feature[DebugFeatures::kShowOn])
			on.render(surface, actor_animation_position);

		for (; z_order_it != z_order.end(); ++z_order_it) {
			Surface *s = *z_order_it;
			s->render(surface);
		}

		if (!message.empty()) {
			bool visible = true;
			if (message_first_frame != 0 && message_animation != NULL) {
				int index = message_animation->currentIndex() + 1;
				debugC(0, kDebugScene, "message: %s first: %u index: %u", message.c_str(), message_first_frame, index);
				if (index < message_first_frame)
					visible = false;
				if (index > message_last_frame) {
					clearMessage();
					visible = false;
				}
			}

			if (visible) {
				_vm->res->font7.render(surface, message_pos.x, message_pos.y, message, message_color);
				busy = true;
			}
		}

		if (!busy && !restart && tick_game && callback_timer) {
			if (--callback_timer == 0) {
				if (_vm->inventory->active())
					_vm->inventory->activate(false);
				_vm->processCallback(callback);
			}
			debugC(0, kDebugScene, "callback timer = %u", callback_timer);
		}

		//if (!current_event.empty())
		//	current_event.dump();

		if (!debug_features.feature[DebugFeatures::kHidePath]) {
			const Common::Array<Walkbox> & scene_walkboxes = walkboxes[_id - 1];
			for (uint i = 0; i < scene_walkboxes.size(); ++i) {
				scene_walkboxes[i].rect.render(surface, 0xd0 + i);
			}

			Common::Point last_p = position;
			for (Path::const_iterator p = path.begin(); p != path.end(); ++p) {
				const Common::Point dp(p->x - last_p.x, p->y - last_p.y);
				if (dp.x != 0) {
					surface->hLine(last_p.x, last_p.y, p->x, 0xfe);
				} else if (dp.y != 0) {
					surface->vLine(last_p.x, last_p.y, p->y, 0xfe);
				}
				last_p = *p;
			}
		}

		_vm->_system->unlockScreen();

		if (current_event.type == SceneEvent::kWait) {
			if (current_event.timer > delta) {
				busy = true;
				current_event.timer -= delta;
			}

			if (current_event.timer <= delta)
				restart |= nextEvent();

		}

		if (!restart && current_event.type == SceneEvent::kWaitForAnimation && !got_any_animation) {
			debugC(0, kDebugScene, "no animations, nextevent");
			nextEvent();
			restart = true;
		}

		if (busy) {
			_idle_timer = 0;
			teenagent_idle.resetIndex();
		}
	} while (restart);

	for (Sounds::iterator i = sounds.begin(); i != sounds.end();) {
		Sound &sound = *i;
		if (sound.delay == 0) {
			debugC(1, kDebugScene, "sound %u started", sound.id);
			_vm->playSoundNow(sound.id);
			i = sounds.erase(i);
		} else {
			sound.delay -= game_delta;
			++i;
		}
	}

	return busy;
}

bool Scene::processEventQueue() {
	while (!events.empty() && current_event.empty()) {
		debugC(0, kDebugScene, "processing next event");
		current_event = events.front();
		events.pop_front();
		switch (current_event.type) {

		case SceneEvent::kSetOn: {
			byte on_id = current_event.ons;
			if (on_id != 0) {
				--on_id;
				byte *ptr = getOns(current_event.scene == 0 ? _id : current_event.scene);
				debugC(0, kDebugScene, "on[%u] = %02x", on_id, current_event.color);
				ptr[on_id] = current_event.color;
			} else {
				on_enabled = current_event.color != 0;
				debugC(0, kDebugScene, "%s on rendering", on_enabled ? "enabling" : "disabling");
			}
			loadOns();
			current_event.clear();
		}
		break;

		case SceneEvent::kSetLan: {
			if (current_event.lan != 0) {
				debugC(0, kDebugScene, "lan[%u] = %02x", current_event.lan - 1, current_event.color);
				byte *ptr = getLans(current_event.scene == 0 ? _id : current_event.scene);
				ptr[current_event.lan - 1] = current_event.color;
			}
			loadLans();
			current_event.clear();
		}
		break;

		case SceneEvent::kLoadScene: {
			if (current_event.scene != 0) {
				init(current_event.scene, current_event.dst);
				if (current_event.orientation != 0)
					orientation = current_event.orientation;
			} else {
				// special case, empty scene
				background.free();
				on.free();
				delete[] ons;
				ons = NULL;
				for (byte i = 0; i < 4; ++i) {
					animation[i].free();
					custom_animation[i].free();
				}
			}
			current_event.clear();
		}
		break;

		case SceneEvent::kWalk: {
			Common::Point dst = current_event.dst;
			if ((current_event.color & 2) != 0) { // relative move
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
			message_animation = NULL;
			if (current_event.first_frame) {
				message_timer = 0;
				message_first_frame = current_event.first_frame;
				message_last_frame = current_event.last_frame;
				if (current_event.slot > 0) {
					message_animation = custom_animation + (current_event.slot - 1);
					//else if (!animation[current_event.slot].empty())
					//  message_animation = animation + current_event.slot;
				} else
					message_animation = &actor_animation;
				debugC(0, kDebugScene, "async message %d-%d (slot %u)", message_first_frame, message_last_frame, current_event.slot);
			} else {
				message_timer = current_event.timer ? current_event.timer * 110 : messageDuration(message);
				message_first_frame = message_last_frame = 0;
			}
			Common::Point p;
			if (current_event.dst.x == 0 && current_event.dst.y == 0) {
				p = Common::Point((actor_animation_position.left + actor_animation_position.right) / 2,
				                  actor_animation_position.top);
			} else {
				p = current_event.dst;
			}

			byte message_slot = current_event.slot;
			if (message_slot != 0) {
				--message_slot;
				assert(message_slot < 4);
				const Surface *s = custom_animation[message_slot].currentFrame(0);
				if (s == NULL)
					s = animation[message_slot].currentFrame(0);
				if (s != NULL) {
					p.x = s->x + s->w / 2;
					p.y = s->y;
				} else
					warning("no animation in slot %u", message_slot);
			}
			message_pos = messagePosition(message, p);
			message_color = current_event.color;

			if (message_first_frame)
				current_event.clear(); // async message, clearing event
			}
			break;

		case SceneEvent::kPlayAnimation: {
			byte slot = current_event.slot & 7; // 0 - mark's
			if (current_event.animation != 0) {
				debugC(0, kDebugScene, "playing animation %u in slot %u(%02x)", current_event.animation, slot, current_event.slot);
				if (slot != 0) {
					--slot;
					assert(slot < 4);
					playAnimation(slot, current_event.animation, (current_event.slot & 0x80) != 0, (current_event.slot & 0x40) != 0, (current_event.slot & 0x20) != 0);
				} else
					actor_talking = true;
			} else {
				if (slot != 0) {
					--slot;
					debugC(0, kDebugScene, "cancelling animation in slot %u", slot);
					assert(slot < 4);
					custom_animation[slot].free();
				} else
					actor_talking = true;
			}
			current_event.clear();
			}
			break;

		case SceneEvent::kPauseAnimation: {
			byte slot = current_event.slot & 7; // 0 - mark's
			if (slot != 0) {
				--slot;
				debugC(1, kDebugScene, "pause animation in slot %u", slot);
				custom_animation[slot].paused = (current_event.slot & 0x80) != 0;
			} else {
				actor_talking = false;
			}
			current_event.clear();
			}
			break;

		case SceneEvent::kClearAnimations:
			for (byte i = 0; i < 4; ++i)
				custom_animation[i].free();
			actor_talking = false;
			current_event.clear();
			break;

		case SceneEvent::kPlayActorAnimation:
			debugC(0, kDebugScene, "playing actor animation %u", current_event.animation);
			playActorAnimation(current_event.animation, (current_event.slot & 0x80) != 0, (current_event.slot & 0x20) != 0);
			current_event.clear();
			break;

		case SceneEvent::kPlayMusic:
			debugC(0, kDebugScene, "setting music %u", current_event.music);
			_vm->setMusic(current_event.music);
			_vm->res->dseg.set_byte(dsAddr_currentMusic, current_event.music);
			current_event.clear();
			break;

		case SceneEvent::kPlaySound:
			debugC(0, kDebugScene, "playing sound %u, delay: %u", current_event.sound, current_event.color);
			sounds.push_back(Sound(current_event.sound, current_event.color));
			current_event.clear();
			break;

		case SceneEvent::kEnableObject: {
			debugC(0, kDebugScene, "%s object #%u", current_event.color ? "enabling" : "disabling", current_event.object - 1);
			Object *obj = getObject(current_event.object - 1, current_event.scene == 0 ? _id : current_event.scene);
			obj->enabled = current_event.color;
			obj->save();
			current_event.clear();
			}
			break;

		case SceneEvent::kHideActor:
			hide_actor = current_event.color != 0;
			current_event.clear();
			break;

		case SceneEvent::kWaitForAnimation:
			debugC(0, kDebugScene, "waiting for the animation");
			break;

		case SceneEvent::kWaitLanAnimationFrame:
			debugC(0, kDebugScene, "waiting for the frame %d in slot %d", current_event.animation, current_event.slot);
			break;

		case SceneEvent::kTimer:
			callback = current_event.callback;
			callback_timer = current_event.timer;
			debugC(0, kDebugScene, "triggering callback %04x in %u frames", callback, callback_timer);
			current_event.clear();
			break;

		case SceneEvent::kEffect:
			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(8);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(0);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(4);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(0);
			_vm->_system->updateScreen();

			current_event.clear();
			break;

		case SceneEvent::kFade:
			_fade_timer = current_event.orientation != 0 ? 5 : -5;
			current_event.clear();
			break;

		case SceneEvent::kWait:
			debugC(0, kDebugScene, "wait %u", current_event.timer);
			break;

		case SceneEvent::kCredits:
			debugC(0, kDebugScene, "showing credits");
			break;

		case SceneEvent::kQuit:
			debugC(0, kDebugScene, "quit!");
			_vm->quitGame();
			break;

		case SceneEvent::kSetFlag:
			debugC(0, kDebugScene, "async set_flag(%04x, %d)", current_event.callback, current_event.color);
			_vm->res->dseg.set_byte(current_event.callback, current_event.color);
			current_event.clear();
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

void Scene::setPalette(unsigned mul) {
	debugC(0, kDebugScene, "setPalette(%u)", mul);
	byte p[3 * 256];

	for (int i = 0; i < 3 * 256; ++i) {
		p[i] = (unsigned)palette[i] * mul;
	}

	_vm->_system->getPaletteManager()->setPalette(p, 0, 256);
}

Object *Scene::getObject(int id, int scene_id) {
	assert(id > 0);

	if (scene_id == 0)
		scene_id = _id;

	if (scene_id == 0)
		return NULL;

	Common::Array<Object> &scene_objects = objects[scene_id - 1];
	--id;
	if (id >= (int)scene_objects.size())
		return NULL;

	return &scene_objects[id];
}

Common::Point Scene::messagePosition(const Common::String &str, Common::Point message_position) {
	int lines = 1;
	for (uint i = 0; i < str.size(); ++i)
		if (str[i] == '\n')
			++lines;

	uint w = _vm->res->font7.render(NULL, 0, 0, str, 0);
	uint h = _vm->res->font7.height * lines + 3;

	message_position.x -= w / 2;
	message_position.y -= h;

	if (message_position.x + w > screenWidth)
		message_position.x = screenWidth - w;
	if (message_position.x < 0)
		message_position.x = 0;
	if (message_position.y + h > screenHeight)
		message_position.y = screenHeight - h;
	if (message_position.y < 0)
		message_position.y = 0;

	return message_position;
}

uint Scene::messageDuration(const Common::String &str) {
	// original game uses static delays: 100-slow, 50, 20 and 1 tick - crazy speed.
	// total delay = total message length * delay / 8 + 60.
	uint total_width = str.size();

	int speed = ConfMan.getInt("talkspeed");
	if (speed < 0)
		speed = 60;
	uint delay_delta = 1 + (255 - speed) * 99 / 255;

	uint delay = 60 + (total_width * delay_delta) / 8;
	debugC(0, kDebugScene, "delay = %u, delta: %u", delay, delay_delta);
	return delay * 10;
}

void Scene::displayMessage(const Common::String &str, byte color, const Common::Point &pos) {
	//assert(!str.empty());
	debugC(0, kDebugScene, "displayMessage: %s", str.c_str());
	message = str;
	message_pos = (pos.x | pos.y) ? pos : messagePosition(str, position);
	message_color = color;
	message_timer = messageDuration(message);
}

void Scene::clear() {
	clearMessage();
	events.clear();
	current_event.clear();
	for (int i = 0; i < 4; ++i) {
		animation[i].free();
		custom_animation[i].free();
	}
	callback = 0;
	callback_timer = 0;
}

void Scene::clearMessage() {
	message.clear();
	message_timer = 0;
	message_color = 0xd1;
	message_first_frame = 0;
	message_last_frame = 0;
	message_animation = NULL;
}

} // End of namespace TeenAgent
