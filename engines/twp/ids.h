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

#ifndef TWP_IDS_H
#define TWP_IDS_H

#define START_ACTORID    1000
#define END_ACTORID      2000
#define START_ROOMID     2000
#define END_ROOMID       3000
#define START_OBJECTID   3000
#define END_OBJECTID     100000
#define START_LIGHTID    100000
#define END_LIGHTID      200000
#define START_SOUNDDEFID 200000
#define END_SOUNDDEFID   250000
#define START_SOUNDID    250000
#define END_SOUNDID      300000
#define START_THREADID   300000
#define END_THREADID     8000000
#define START_CALLBACKID 8000000
#define END_CALLBACKID   10000000

#define FAR_LOOK 		8
#define VERB_WALKTO 	1
#define VERB_LOOKAT 	2
#define VERB_TALKTO 	3
#define VERB_PICKUP 	4
#define VERB_OPEN 		5
#define VERB_CLOSE 		6
#define VERB_PUSH 		7
#define VERB_PULL 		8
#define VERB_GIVE 		9
#define VERB_USE 		10
#define VERB_DIALOG 	13

#define GONE 			4
#define USE_WITH 		2
#define USE_ON 			4
#define USE_IN 			32

namespace Twp {

bool isThread(int id);
bool isRoom(int id);
bool isActor(int id);
bool isObject(int id);
bool isSound(int id);
bool isLight(int id);
bool isCallback(int id);

int newRoomId();
int newObjId();
int newActorId();
int newSoundDefId();
int newSoundId();
int newThreadId();
int newCallbackId();
int newLightId();
void setCallbackId(int id);
int getCallbackId();

}

#endif
