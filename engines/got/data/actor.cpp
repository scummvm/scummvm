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

#include "got/data/actor.h"
#include "common/algorithm.h"
#include "common/memstream.h"

namespace Got {

void ACTOR::loadFixed(Common::SeekableReadStream *src) {
	move = src->readByte();
	width = src->readByte();
	height = src->readByte();
	directions = src->readByte();
	frames = src->readByte();
	frame_speed = src->readByte();
	src->read(frame_sequence, 4);
	speed = src->readByte();
	size_x = src->readByte();
	size_y = src->readByte();
	strength = src->readByte();
	health = src->readByte();
	num_moves = src->readByte();
	shot_type = src->readByte();
	shot_pattern = src->readByte();
	shots_allowed = src->readByte();
	solid = src->readByte();
	flying = src->readByte();
	rating = src->readByte();
	type = src->readByte();
	src->read(name, 9);
	func_num = src->readByte();
	func_pass = src->readByte();
	magic_hurts = src->readSint16LE();
	src->read(future1, 4);
}

void ACTOR::loadFixed(const byte *src) {
	Common::MemoryReadStream stream(src, 40);
	loadFixed(&stream);
}

void ACTOR::copyFixedAndPics(const ACTOR &src) {
	move = src.move;
	width = src.width;
	height = src.height;
	directions = src.directions;
	frames = src.frames;
	frame_speed = src.frame_speed;
	Common::copy(src.frame_sequence, src.frame_sequence + 4, frame_sequence);
	speed = src.speed;
	size_x = src.size_x;
	size_y = src.size_y;
	strength = src.strength;
	health = src.health;
	num_moves = src.num_moves;
	shot_type = src.shot_type;
	shot_pattern = src.shot_pattern;
	shots_allowed = src.shots_allowed;
	solid = src.solid;
	flying = src.flying;
	rating = src.rating;
	type = src.type;
	Common::copy(src.name, src.name + 9, name);
	func_num = src.func_num;
	func_pass = src.func_pass;
	magic_hurts = src.magic_hurts;
	Common::copy(src.future1, src.future1 + 4, future1);

	// Copy all the surfaces for all the directions over
	for (int d = 0; d < DIRECTION_COUNT; ++d) {
		for (int f = 0; f < FRAME_COUNT; ++f)
			pic[d][f].copyFrom(src.pic[d][f]);
	}
}

ACTOR &ACTOR::operator=(const ACTOR &src) {
	// First copy the fixed portion and the pics
	copyFixedAndPics(src);

	// Copy temporary fields
	frame_count = src.frame_count;
	dir = src.dir;
	last_dir = src.last_dir;
	x = src.x;
	y = src.y;
	center = src.center;
	Common::copy(src.last_x, src.last_x + 2, last_x);
	Common::copy(src.last_y, src.last_y + 2, last_y);
	used = src.used;
	next = src.next;
	speed_count = src.speed_count;
	vunerable = src.vunerable;
	shot_cnt = src.shot_cnt;
	num_shots = src.num_shots;
	creator = src.creator;
	pause = src.pause;
	actor_num = src.actor_num;
	move_count = src.move_count;
	dead = src.dead;
	toggle = src.toggle;
	center_x = src.center_x;
	center_y = src.center_y;
	show = src.show;
	temp1 = src.temp1;
	temp2 = src.temp2;
	counter = src.counter;
	move_counter = src.move_counter;
	edge_counter = src.edge_counter;
	temp3 = src.temp3;
	temp4 = src.temp4;
	temp5 = src.temp5;
	hit_thor = src.hit_thor;
	rand = src.rand;
	init_dir = src.init_dir;
	pass_value = src.pass_value;
	shot_actor = src.shot_actor;
	magic_hit = src.magic_hit;
	temp6 = src.temp6;
	i1 = src.i1;
	i2 = src.i2;
	i3 = src.i3;
	i4 = src.i4;
	i5 = src.i5;
	i6 = src.i6;
	init_health = src.init_health;
	talk_counter = src.talk_counter;
	etype = src.etype;
	Common::copy(src.future2, src.future2 + 25, future2);

	return *this;
}

} // namespace Got
