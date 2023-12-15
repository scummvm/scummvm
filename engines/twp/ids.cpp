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

#include "ids.h"

namespace Twp {

static int gRoomId = START_ROOMID;
static int gActorId = START_ACTORID;
static int gObjId = START_OBJECTID;
static int gSoundDefId = START_SOUNDDEFID;
static int gSoundId = START_SOUNDID;
static int gThreadId = START_THREADID;
static int gCallbackId = START_CALLBACKID;
static int gLightId = START_LIGHTID;

static inline bool isBetween(int id, int startId, int endId) {
	return id >= startId && id < endId;
}

bool isThread(int id) {
	return isBetween(id, START_THREADID, END_THREADID);
}

bool isRoom(int id) {
	return isBetween(id, START_ROOMID, END_THREADID);
}

bool isActor(int id) {
	return isBetween(id, START_ACTORID, END_ACTORID);
}

bool isObject(int id) {
	return isBetween(id, START_OBJECTID, END_OBJECTID);
}

bool isSound(int id) {
	return isBetween(id, START_SOUNDID, END_SOUNDID);
}

bool isLight(int id) {
	return isBetween(id, START_LIGHTID, END_LIGHTID);
}

bool isCallback(int id) {
	return isBetween(id, START_CALLBACKID, END_CALLBACKID);
}

int newRoomId() {
	return gRoomId++;
}

int newObjId() {
	return gObjId++;
}

int newActorId() {
	return gActorId++;
}

int newSoundDefId() {
	return gSoundDefId++;
}

int newSoundId() {
	return gSoundId++;
}

int newThreadId() {
	return gThreadId++;
}

int newCallbackId() {
	return gCallbackId++;
}

void setCallbackId(int id) {
	gCallbackId = id;
}

int getCallbackId() {
	return gCallbackId;
}

int newLightId() {
	return gLightId++;
}
}
