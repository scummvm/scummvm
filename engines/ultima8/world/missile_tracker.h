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
 */

#ifndef ULTIMA8_WORLD_MISSILETRACKER_H
#define ULTIMA8_WORLD_MISSILETRACKER_H

class Item;

class MissileTracker {
public:
	MissileTracker(Item *item, int32 sx, int32 sy, int32 sz,
	               int32 tx, int32 ty, int32 tz,
	               int32 speed, int32 gravity);
	MissileTracker(Item *item, int32 tx, int32 ty, int32 tz,
	               int32 speed, int32 gravity);
	~MissileTracker();

	bool isPathClear();

	void launchItem();

protected:
	void init(int32 sx, int32 sy, int32 sz, int32 speed);

private:
	ObjId objid;
	int32 destx, desty, destz;
	int32 speedx, speedy, speedz;
	int32 gravity;
	int frames;
};

#endif
