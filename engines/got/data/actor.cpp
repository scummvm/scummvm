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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "got/data/actor.h"

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
    for (int d = 0; d < DIRECTION_COUNT; ++d)
        for (int f = 0; f < FRAME_COUNT; ++f)
            pic[d][f] = src.pic[d][f];
}

} // namespace Got
