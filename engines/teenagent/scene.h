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

#ifndef TEENAGENT_SCENE_H
#define TEENAGENT_SCENE_H

#include "teenagent/surface.h"
#include "teenagent/actor.h"
#include "common/system.h"
#include "common/list.h"
#include "teenagent/objects.h"

namespace TeenAgent {

class TeenAgentEngine;
class Dialog;

struct SceneEvent {
	enum Type {
		kNone,					//0
		kMessage,
		kWalk,
		kPlayAnimation,
		kPlayActorAnimation,	//4
		kPauseAnimation,
		kClearAnimations,
		kLoadScene,
		kSetOn,					//8
		kSetLan,
		kPlayMusic,
		kPlaySound,
		kEnableObject,			//12
		kHideActor,
		kWaitForAnimation,
		kWaitLanAnimationFrame,
		kCreditsMessage,		//16
		kTimer,
		kQuit
	} type;

	Common::String message;
	byte color;
	byte slot;
	union {
		uint16 animation;
		uint16 callback;
	};
	uint16 timer;
	byte orientation;
	Common::Point dst;
	byte scene; //fixme: put some of these to the union?
	byte ons;
	byte lan;
	union {
		byte music;
		byte first_frame;
	};
	union {
		byte sound;
		byte last_frame;
	};
	byte object;

	SceneEvent(Type type_) :
			type(type_), message(), color(0xd1), slot(0), animation(0), timer(0), orientation(0), dst(),
			scene(0), ons(0), lan(0), music(0), sound(0), object(0) {}

	void clear() {
		type = kNone;
		message.clear();
		color = 0xd1;
		slot = 0;
		orientation = 0;
		animation = 0;
		timer = 0;
		dst.x = dst.y = 0;
		scene = 0;
		ons = 0;
		lan = 0;
		music = 0;
		sound = 0;
		object = 0;
	}

	inline bool empty() const {
		return type == kNone;
	}

	void dump() const {
		debug(0, "event[%d]: \"%s\"[%02x], slot: %d, animation: %u, timer: %u, dst: (%d, %d) [%u], scene: %u, ons: %u, lan: %u, object: %u, music: %u, sound: %u",
			(int)type, message.c_str(), color, slot, animation, timer, dst.x, dst.y, orientation, scene, ons, lan, object, music, sound
		);
	}
};

class Scene {
public:
	bool intro;

	Scene();

	void init(TeenAgentEngine *engine, OSystem *system);
	void init(int id, const Common::Point &pos);
	bool render(OSystem *system);
	int getId() const { return _id; }

	void warp(const Common::Point &point, byte orientation = 0);

	void moveTo(const Common::Point &point, byte orientation = 0, bool validate = 0);
	Common::Point getPosition() const { return position; }

	void displayMessage(const Common::String &str, byte color = 0xd1, const Common::Point &pos = Common::Point());
	void setOrientation(uint8 o) { orientation = o; }
	void push(const SceneEvent &event);

	bool processEvent(const Common::Event &event);

	void clear();

	byte *getOns(int id);
	byte *getLans(int id);

	bool eventRunning() const { return !current_event.empty(); }

	Walkbox *getWalkbox(byte id) { return &walkboxes[_id - 1][id]; }
	Object *getObject(int id, int scene_id = 0);
	Object *findObject(const Common::Point &point);

	void loadObjectData();
	Animation * getAnimation(byte slot);
	inline Animation * getActorAnimation() { return &actor_animation; }

private:
	void loadOns();
	void loadLans();

	void playAnimation(byte idx, uint id, bool loop, bool paused, bool ignore);
	void playActorAnimation(uint id, bool loop, bool ignore);

	byte palette[768];
	void setPalette(OSystem *system, const byte *palette, unsigned mul = 1);
	static Common::Point messagePosition(const Common::String &str, Common::Point position);
	static uint messageDuration(const Common::String &str);

	bool processEventQueue();
	inline bool nextEvent() {
		current_event.clear();
		return processEventQueue();
	}
	void clearMessage();

	TeenAgentEngine *_engine;
	OSystem *_system;

	int _id;
	Graphics::Surface background;
	Surface on;
	Surface *ons;
	uint32 ons_count;
	Animation actor_animation, animation[4], custom_animation[4];
	Common::Rect actor_animation_position, animation_position[4];

	Actor teenagent, teenagent_idle;
	Common::Point position;

	typedef Common::List<Common::Point> Path;
	Path path;
	uint8 orientation;
	bool actor_talking;

	bool findPath(Path &p, const Common::Point &src, const Common::Point &dst) const;
	
	Common::Array<Common::Array<Object> > objects;
	Common::Array<Common::Array<Walkbox> > walkboxes;

	Common::String message;
	Common::Point message_pos;
	byte message_color;
	uint message_timer;
	byte message_first_frame;
	byte message_last_frame;
	Animation * message_animation;

	typedef Common::List<SceneEvent> EventList;
	EventList events;
	SceneEvent current_event;
	bool hide_actor;
	
	uint16 callback, callback_timer;

	struct Sound {
		byte id, delay;
		Sound(byte i, byte d): id(i), delay(d) {}
	};
	typedef Common::List<Sound> Sounds;
	Sounds sounds;
};

} // End of namespace TeenAgent

#endif
