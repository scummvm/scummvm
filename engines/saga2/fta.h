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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_FTA_H
#define SAGA2_FTA_H

#include "saga2/gpointer.h"

namespace Saga2 {

class hResContext;
class hResource;

/* ===================================================================== *
   Constants
 * ===================================================================== */

//  For GameMode Stack
const int           Max_Modes   =   8,  //Max Game Mode Objects
                    End_List    =   0;  //Variable To Indicate End Of Arg List

//  Width and height of full screen
const int           screenWidth = 640,
                    screenHeight = 480;

//  Number of ticks per second
const int           ticksPerSecond = 72;    // 72 ticks per second

//  Desired frame rate of game (fps)
const int           frameRate = 10,
                    framePeriod = ticksPerSecond / frameRate;

/* ====================================================================== *
   Overall game modes
 * ====================================================================== */

/*  There are basically 5 different 'modes' in the game. Three of those modes
    are exclusive, while the other two "nest" on top of the others.

    The exclusive modes are:
        1.  Intro           The animated slideshow of the introduction.
        2.  Finale          The animated slideshow of the finale.
        3.  Main Play       The scrolling tilemap view.

    The modes which can nest on top of the others are:
        4.  Modal Dialog    A popup window that blocks all other input.
        5.  Cut Scene       A non-interactive animated interlude.

    The GameMode class represents these modes as an explicit object.
*/

struct gMouseState;

class GameMode {
public:
	GameMode        *prev;                  // previous nested mode
	int16           nestable;               // true if mode nests

	static  GameMode    *modeStackPtr[Max_Modes]; // Array Of Current Mode Stack Pointers to Game Objects
	static  int         modeStackCtr;       // Counter For Array Of Mode Stack Pointers to Game Objects
	static  GameMode    *newmodeStackPtr[Max_Modes]; // Array Of New Mode Stack Pointers to Game Objects
	static  int         newmodeStackCtr;        // Counter For Array Of Mode Stack Pointers to Game Objects
	static  int         newmodeFlag;        // Flag Telling EventLoop Theres A New Mode So Update
	static GameMode *currentMode,           // pointer to current mode.
	       *newMode;               // next mode to run

	void (*setup)();                     // initialize this mode
	void (*cleanup)();               // deallocate mode resources
	void (*handleTask)();                // "application task" for mode
	void (*handleKey)(int16 key, int16 qual);  // handle keystroke from window
	void (*draw)();                          // handle draw functions for window

	static  void modeUnStack();
	static  void modeUnStack(int StopHere);
	static  int getStack(GameMode **saveStackPtr);
	static  void SetStack(GameMode *modeFirst, ...);
	static  void SetStack(GameMode **newStack, int newStackSize);
	static  bool update();
	static  void modeStack(GameMode *AddThisMode);
	void begin();                        // launch this mode
	void end();                          // quit this mode
	static void modeSwitch();            // quit this mode
};

extern GameMode     IntroMode,
       FinaleMode,
       PlayMode,
       TileMode,
       StageMode,
       ModalDialogMode,
       CutSceneMode,
       VideoMode; //stageprob

/* ====================================================================== *
   Game Clock routines
 * ====================================================================== */

//  Initialize the timer
void initTimer();

void saveTimer(Common::OutSaveFile *out);
void loadTimer(Common::InSaveFile *in);

void pauseTimer();               // pause game clock
void resumeTimer();                  // resume game clock

//  Alarm is a useful class for specifying time delays. It will
//  work correctly even if the game counter wraps around.

class Alarm {
public:
	uint32 basetime;                            // timer alarm was set
	uint32 duration;                            // duration of alarm

	void set(uint32 duration);
	bool check();
	uint32 elapsed();                    // time elapsed since alarm set

	void write(Common::MemoryWriteStreamDynamic *out);
	void read(Common::InSaveFile *in);
};

/* ===================================================================== *
   Misc Prototypes
 * ===================================================================== */

//  Debugging status

void WriteStatusF(int16 lin, const char *msg, ...);

//  File loading

void  *LoadFile(char *filename);             // load file into buffer

//  Resource loading

void *LoadResource(hResContext *con, uint32 id, const char desc[]);
void dumpResource(hResContext *con, uint32 id);
Common::SeekableReadStream *loadResourceToStream(hResContext *con, uint32 id, const char desc[]);

//  Directory control

void restoreProgramDir();                // chdir() to program directory

// Returns Random Number

int16 GetRandomBetween(int start, int end);

//  Distance calculation

int16 quickDistance(const Point16 &p);
int32 quickDistance(const Point32 &p);

void initPathFinder();
void cleanupPathFinder();

/* ===================================================================== *
   Miscellaneous globals management functions
 * ===================================================================== */

void initGlobals();
void saveGlobals(Common::OutSaveFile *outS);
void loadGlobals(Common::InSaveFile *in);
inline void cleanupGlobals() {}      // do nothing

/* ===================================================================== *
   Script-related
 * ===================================================================== */

typedef int16 ThreadID;
const ThreadID  NoThread = -1;

//  Wake up a thread unconditionally
void wakeUpThread(ThreadID thread);

//  Wake up a thread and set return value from SAGA function
void wakeUpThread(ThreadID thread, int16 returnVal);

//  Script system initialization and cleanup
void initScripts();
void cleanupScripts();
void dispatchScripts();

//  An extended script is running -- suspend all background processing.
extern int16            extendedThreadLevel;

/* ===================================================================== *
   Misc Globals
 * ===================================================================== */

extern volatile int32 gameTime;             // current timer

//  Resource files

extern hResource    *resFile,               // our main resource file
       *objResFile,
       *auxResFile;

} // end of namespace Saga2

#endif
