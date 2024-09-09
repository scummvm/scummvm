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

#ifndef M4_CORE_ROOMS_H
#define M4_CORE_ROOMS_H

#include "common/hashmap.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_hotspot.h"

namespace M4 {

class Room {
public:
	Room() {}
	virtual ~Room() {}

	virtual void preload();
	virtual void init() {}
	virtual void daemon() {}
	virtual void pre_parser() {}
	virtual void parser();
	virtual void parser_code() {}
	virtual void roomError() {}
	virtual void shutdown() {}

	/**
	 * Used to return custom hotspots at a given position
	 */
	virtual HotSpotRec *custom_hotspot_which(int32 x, int32 y) {
		return nullptr;
	}
};

class Section {
private:
	Common::HashMap<int, Room *> _rooms;
protected:
	// Add a room to the section
	void add(int roomNum, Room *room) {
		_rooms[roomNum] = room;
	}

public:
	Section() {}
	virtual ~Section() {}

	virtual void preLoad() {}

	/**
	 * Section initialization
	 */
	virtual void init() {}

	/**
	 * Iterates through the rooms array to find a given room
	 */
	Room *operator[](uint roomNum);

	virtual void global_room_init() {}
	virtual void daemon() = 0;
	virtual void tick() {}
	virtual void pre_parser() {}
	virtual void parser() {}
};

class Sections {
private:
	int32 _cameraShiftAmount = 0;
	int32 _cameraShift_vert_Amount = 0;
	int32 camera_pan_step = 10;

	void get_ipl();
	void get_walker();

	void game_control_cycle();

protected:
	Common::Array<Section *> _sections;

public:
	Section *_activeSection = nullptr;
	Room *_activeRoom = nullptr;
public:
	Sections() {}
	virtual ~Sections() {}

	void global_section_constructor();
	void section_room_constructor();
	void game_daemon_code();
	void parse_player_command_now();

	void section_init() {
		_activeSection->init();
	}
	void daemon() {
		_activeSection->daemon();
	}
	void global_room_init() {
		_activeSection->global_room_init();
	}
	void tick() {
		_activeSection->tick();
	}
	void section_parser() {
		_activeSection->parser();
	}

	void room_preload() {
		_activeRoom->preload();
	}
	void room_init() {
		_activeRoom->init();
	}
	void room_daemon() {
		_activeRoom->daemon();
	}
	void room_pre_parser() {
		_activeRoom->pre_parser();
	}
	void room_parser() {
		_activeRoom->parser();
	}
	void parser_code() {
		_activeRoom->parser_code();
	}
	void room_error() {
		_activeRoom->roomError();
	}
	void room_shutdown() {
		_activeRoom->shutdown();
	}
	HotSpotRec *custom_hotspot_which(int x, int y) {
		return _activeRoom->custom_hotspot_which(x, y);
	}

	void m4SceneLoad();
	void m4RunScene();
	void m4EndScene();

	void pal_game_task();
	void camera_shift_xy(int32 x, int32 y);
	bool game_camera_panning() const {
		return _cameraShiftAmount != 0 || _cameraShift_vert_Amount != 0;
	}

	virtual void global_daemon() = 0;
	virtual void global_pre_parser() = 0;
	virtual void global_parser() = 0;

	void global_error_code() {
		// No implementation
	}
};

} // namespace M4

#endif
