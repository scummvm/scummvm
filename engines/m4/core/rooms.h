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

#include "common/array.h"
#include "m4/adv_r/adv.h"

namespace M4 {

class Room {
public:
	uint _roomNum;
public:
	Room(uint roomNum) : _roomNum(roomNum) {
	}
	virtual ~Room() {
	}

	virtual void preload() {}
	virtual void init() {}
	virtual void daemon() {}
	virtual void pre_parser() {}
	virtual void parser() {}
	virtual void parser_code() {}
	virtual void error() {}
	virtual void shutdown() {}
	virtual void custom_hotspot_which() {}
};

class Section {
private:
	Common::Array<Room *> _rooms;
protected:
	// Add a room to the section
	void add(Room *room) {
		_rooms.push_back(room);
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

	/**
	 * Used to tell if x,y is over the walker hotspot
	 */
	virtual HotSpotRec *walker_spotter(int32 x, int32 y);

	virtual void global_room_init() {}
	virtual void daemon_code() {}
};

class Sections {
private:
	int32 _cameraShiftAmount = 0;
	int32 cameraShift_vert_Amount = 0;
	bool shut_down_digi_tracks_between_rooms = true;
	int32 camera_pan_step = 10;
	bool _visited_room = false;

	void get_ipl();
	void get_walker();

	void game_control_cycle();
	void parse_player_command_now();

protected:
	Common::Array<Section *> _sections;

public:
	Section *_activeSection = nullptr;
	Room *_activeRoom = nullptr;
public:
	Sections() {}

	void global_section_constructor();
	void section_room_constructor();

	void section_init() {
		_activeSection->init();
	}
	void daemon_code() {
		_activeSection->daemon_code();
	}
	void global_room_init() {
		_activeSection->global_room_init();
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
		_activeRoom->error();
	}
	void room_shutdown() {
		_activeRoom->shutdown();
	}
	void custom_hotspot_which() {
		_activeRoom->custom_hotspot_which();
	}

	void m4SceneLoad();
	void m4RunScene();
	void m4EndScene();
};

} // namespace M4

#endif
