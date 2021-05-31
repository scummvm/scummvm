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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ANNOY_H
#define SAGA2_ANNOY_H

namespace Saga2 {

//-----------------------------------------------------------------------
// known annoying test sounds

#define TEST_OPEN   1
#define TEST_CLOSE  2
#define TEST_USE    3
#define TEST_THROW  4

/* ===================================================================== *
   Imports
 * ===================================================================== */

class Location;

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

//-----------------------------------------------------------------------
//	play routines

void playMusic(uint32 s);

void playSound(uint32 s);
void playSoundAt(uint32 s, Location playAt);
void playSoundAt(uint32 s, Point32 playAt);

void playVoice(uint32 s);
bool sayVoiceAt(uint32 s[], Location l);
bool sayVoiceAt(uint32 s[], Point32 l);

void playLoop(uint32 s);
void playLoopAt(uint32 s, Location l);
void playLoopAt(uint32 s, Point32 l);
void moveLoop(Point32 loc);

//-----------------------------------------------------------------------
//	play routines for SAGA

void PlaySound(char IDstr[]);
void PlayVoice(char IDstr[]);
void PlayLoop(char IDstr[]);
void PlayMusic(char IDstr[]);
void PlayLongSound(char IDstr[]);

//-----------------------------------------------------------------------
//	general maintainence

bool initAudio(void);
void startAudio(void);
void suspendAudio(void);
void resumeAudio(void);
void cleanupAudio(void);
void writeConfig(void);

void audioEventLoop(void);
bool stillDoingVoice(uint32 sampno);

//-----------------------------------------------------------------------
//	environmental sounds

void audioEnvironmentUseSet(int16 audioSet, int32 auxID, Point32 relPos);
void audioEnvironmentCheck(void);

void audioEnvironmentSetAggression(bool onOff);
void audioEnvironmentSetDaytime(bool onOff);
void audioEnvironmentSuspend(bool onOff);
void audioEnvironmentSetWorld(int mapNum);

//-----------------------------------------------------------------------
//	environmental music

void clearActiveFactions(void);
void useActiveFactions(void);


//-----------------------------------------------------------------------
//	prototypes

#if DEBUG
int annoyingTestSound(int32);          // prototype for annoying test sound calls
int annoyingTestMusic(int32);
#endif

}

#endif  //ANNOY_H
