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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_SCHEDULE_H
#define HUGO_SCHEDULE_H

#include "common/file.h"

namespace Hugo {

/**
 * Following defines the action types and action list
 */
enum action_t {                                     // Parameters:
	ANULL              = 0xff,                      // Special NOP used to 'delete' events in DEL_EVENTS
	ASCHEDULE          = 0,                         //  0 - Ptr to action list to be rescheduled
	START_OBJ,                                      //  1 - Object number
	INIT_OBJXY,                                     //  2 - Object number, x,y
	PROMPT,                                         //  3 - index of prompt & response string, ptrs to action
	                                                //      lists.  First if response matches, 2nd if not.
	BKGD_COLOR,                                     //  4 - new background color
	INIT_OBJVXY,                                    //  5 - Object number, vx, vy
	INIT_CARRY,                                     //  6 - Object number, carried status
	INIT_HF_COORD,                                  //  7 - Object number (gets hero's 'feet' coordinates)
	NEW_SCREEN,                                     //  8 - New screen number
	INIT_OBJSTATE,                                  //  9 - Object number, new object state
	INIT_PATH,                                      // 10 - Object number, new path type
	COND_R,                                         // 11 - Conditional on object state - req state, 2 act_lists
	TEXT,                                           // 12 - Simple text box
	SWAP_IMAGES,                                    // 13 - Swap 2 object images
	COND_SCR,                                       // 14 - Conditional on current screen
	AUTOPILOT,                                      // 15 - Set object to home in on another (stationary) object
	INIT_OBJ_SEQ,                                   // 16 - Object number, sequence index to set curr_seq_p to
	SET_STATE_BITS,                                 // 17 - Objnum, mask to OR with obj states word
	CLEAR_STATE_BITS,                               // 18 - Objnum, mask to ~AND with obj states word
	TEST_STATE_BITS,                                // 19 - Objnum, mask to test obj states word
	DEL_EVENTS,                                     // 20 - Action type to delete all occurrences of
	GAMEOVER,                                       // 21 - Disable hero & commands.  Game is over
	INIT_HH_COORD,                                  // 22 - Object number (gets hero's actual coordinates)
	EXIT,                                           // 23 - Exit game back to DOS
	BONUS,                                          // 24 - Get score bonus for an action
	COND_BOX,                                       // 25 - Conditional on object within bounding box
	SOUND,                                          // 26 - Set currently playing sound
	ADD_SCORE,                                      // 27 - Add object's value to current score
	SUB_SCORE,                                      // 28 - Subtract object's value from current score
	COND_CARRY,                                     // 29 - Conditional on carrying object
	INIT_MAZE,                                      // 30 - Start special maze hotspot processing
	EXIT_MAZE,                                      // 31 - Exit special maze processing
	INIT_PRIORITY,                                  // 32 - Initialize fbg field
	INIT_SCREEN,                                    // 33 - Initialize screen field of object
	AGSCHEDULE,                                     // 34 - Global schedule - lasts over new screen
	REMAPPAL,                                       // 35 - Remappe palette - palette index, color
	COND_NOUN,                                      // 36 - Conditional on noun appearing in line
	SCREEN_STATE,                                   // 37 - Set new screen state - used for comments
	INIT_LIPS,                                      // 38 - Position lips object for supplied object
	INIT_STORY_MODE,                                // 39 - Set story mode TRUE/FALSE (user can't type)
	WARN,                                           // 40 - Same as TEXT but can't dismiss box by typing
	COND_BONUS,                                     // 41 - Conditional on bonus having been scored
	TEXT_TAKE,                                      // 42 - Issue text box with "take" info string
	YESNO,                                          // 43 - Prompt user for Yes or No
	STOP_ROUTE,                                     // 44 - Skip any route in progress (hero still walks)
	COND_ROUTE,                                     // 45 - Conditional on route in progress
	INIT_JUMPEXIT,                                  // 46 - Initialize status.jumpexit
	INIT_VIEW,                                      // 47 - Initialize viewx, viewy, dir
	INIT_OBJ_FRAME,                                 // 48 - Object number, seq,frame to set curr_seq_p to
	OLD_SONG           = 49                         // Added by Strangerke - Set currently playing sound, old way: that is, using a string index instead of a reference in a file
};

struct act0 {                                       // Type 0 - Schedule
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Ptr to an action list
};

struct act1 {                                       // Type 1 - Start an object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      cycleNumb;                             // Number of times to cycle
	cycle_t  cycle;                                 // Direction to start cycling
};

struct act2 {                                       // Type 2 - Initialize an object coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      x, y;                                  // Coordinates
};

struct act3 {                                       // Type 3 - Prompt user for text
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   promptIndex;                           // Index of prompt string
	int     *responsePtr;                           // Array of indexes to valid response string(s) (terminate list with -1)
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
	bool     encodedFl;                             // (HUGO 1 DOS ONLY) Whether response is encoded or not
};

struct act4 {                                       // Type 4 - Set new background color
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	long     newBackgroundColor;                    // New color
};

struct act5 {                                       // Type 5 - Initialize an object velocity
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      vx, vy;                                // velocity
};

struct act6 {                                       // Type 6 - Initialize an object carrying
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	bool     carriedFl;                             // carrying
};

struct act7 {                                       // Type 7 - Initialize an object to hero's coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
};

struct act8 {                                       // Type 8 - switch to new screen
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The new screen number
};

struct act9 {                                       // Type 9 - Initialize an object state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	byte     newState;                              // New state
};

struct act10 {                                      // Type 10 - Initialize an object path type
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      newPathType;                           // New path type
	int8     vxPath, vyPath;                        // Max delta velocities e.g. for CHASE
};

struct act11 {                                      // Type 11 - Conditional on object's state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	byte     stateReq;                              // Required state
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act12 {                                      // Type 12 - Simple text box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act13 {                                      // Type 13 - Swap first object image with second
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex1;                             // Index of first object
	int      objIndex2;                             // 2nd
};

struct act14 {                                      // Type 14 - Conditional on current screen
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The required object
	int      screenReq;                             // The required screen number
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act15 {                                      // Type 15 - Home in on an object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex1;                             // The object number homing in
	int      objIndex2;                             // The object number to home in on
	int8     dx, dy;                                // Max delta velocities
};
// Note: Don't set a sequence at time 0 of a new screen, it causes
// problems clearing the boundary bits of the object!  timer > 0 is safe
struct act16 {                                      // Type 16 - Set curr_seq_p to seq
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      seqIndex;                              // The index of seq array to set to
};

struct act17 {                                      // Type 17 - SET obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      stateMask;                             // The mask to OR with current obj state
};

struct act18 {                                      // Type 18 - CLEAR obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      stateMask;                             // The mask to ~AND with current obj state
};

struct act19 {                                      // Type 19 - TEST obj individual state bits
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      stateMask;                             // The mask to AND with current obj state
	uint16   actPassIndex;                          // Ptr to action list (all bits set)
	uint16   actFailIndex;                          // Ptr to action list (not all set)
};

struct act20 {                                      // Type 20 - Remove all events with this type of action
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	action_t actTypeDel;                            // The action type to remove
};

struct act21 {                                      // Type 21 - Gameover.  Disable hero & commands
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act22 {                                      // Type 22 - Initialize an object to hero's coords
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
};

struct act23 {                                      // Type 23 - Exit game back to DOS
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act24 {                                      // Type 24 - Get bonus score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      pointIndex;                            // Index into points array
};

struct act25 {                                      // Type 25 - Conditional on bounding box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The required object number
	int      x1, y1, x2, y2;                        // The bounding box
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act26 {                                      // Type 26 - Play a sound
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    soundIndex;                            // Sound index in data file
};

struct act27 {                                      // Type 27 - Add object's value to score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // object number
};

struct act28 {                                      // Type 28 - Subtract object's value from score
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // object number
};

struct act29 {                                      // Type 29 - Conditional on object carried
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The required object number
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act30 {                                      // Type 30 - Start special maze processing
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	byte     mazeSize;                              // Size of (square) maze
	int      x1, y1, x2, y2;                        // Bounding box of maze
	int      x3, x4;                                // Extra x points for perspective correction
	byte     firstScreenIndex;                      // First (top left) screen of maze
};

struct act31 {                                      // Type 31 - Exit special maze processing
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act32 {                                      // Type 32 - Init fbg field of object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	byte     priority;                              // Value of foreground/background field
};

struct act33 {                                      // Type 33 - Init screen field of object
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      screenIndex;                           // Screen number
};

struct act34 {                                      // Type 34 - Global Schedule
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   actIndex;                              // Ptr to an action list
};

struct act35 {                                      // Type 35 - Remappe palette
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int16    oldColorIndex;                         // Old color index, 0..15
	int16    newColorIndex;                         // New color index, 0..15
};

struct act36 {                                      // Type 36 - Conditional on noun mentioned
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   nounIndex;                             // The required noun (list)
	uint16   actPassIndex;                          // Ptr to action list if success
	uint16   actFailIndex;                          // Ptr to action list if failure
};

struct act37 {                                      // Type 37 - Set new screen state
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      screenIndex;                           // The screen number
	byte     newState;                              // The new state
};

struct act38 {                                      // Type 38 - Position lips
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      lipsObjIndex;                          // The LIPS object
	int      objIndex;                              // The object to speak
	byte     dxLips;                                // Relative offset of x
	byte     dyLips;                                // Relative offset of y
};

struct act39 {                                      // Type 39 - Init story mode
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     storyModeFl;                           // New state of story_mode flag
};

struct act40 {                                      // Type 40 - Unsolicited text box
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      stringIndex;                           // Index (enum) of string in strings.dat
};

struct act41 {                                      // Type 41 - Conditional on bonus scored
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      BonusIndex;                            // Index into bonus list
	uint16   actPassIndex;                          // Index of the action list if scored for the first time
	uint16   actFailIndex;                          // Index of the action list if already scored
};

struct act42 {                                      // Type 42 - Text box with "take" string
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object taken
};

struct act43 {                                      // Type 43 - Prompt user for Yes or No
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      promptIndex;                           // index of prompt string
	uint16   actYesIndex;                           // Ptr to action list if YES
	uint16   actNoIndex;                            // Ptr to action list if NO
};

struct act44 {                                      // Type 44 - Stop any route in progress
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
};

struct act45 {                                      // Type 45 - Conditional on route in progress
	action_t   actType;                             // The type of action
	int        timer;                               // Time to set off the action
	int        routeIndex;                          // Must be >= current status.rindex
	uint16     actPassIndex;                        // Ptr to action list if en-route
	uint16     actFailIndex;                        // Ptr to action list if not
};

struct act46 {                                      // Type 46 - Init status.jumpexit
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	bool     jumpExitFl;                            // New state of jumpexit flag
};

struct act47 {                                      // Type 47 - Init viewx,viewy,dir
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object
	int16    viewx;                                 // object.viewx
	int16    viewy;                                 // object.viewy
	int16    direction;                             // object.dir
};

struct act48 {                                      // Type 48 - Set curr_seq_p to frame n
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	int      objIndex;                              // The object number
	int      seqIndex;                              // The index of seq array to set to
	int      frameIndex;                            // The index of frame to set to
};

struct act49 {                                      // Added by Strangerke - Type 79 - Play a song (DOS way)
	action_t actType;                               // The type of action
	int      timer;                                 // Time to set off the action
	uint16   songIndex;                             // Song index in string array
};

union act {
	act0     a0;
	act1     a1;
	act2     a2;
	act3     a3;
	act4     a4;
	act5     a5;
	act6     a6;
	act7     a7;
	act8     a8;
	act9     a9;
	act10    a10;
	act11    a11;
	act12    a12;
	act13    a13;
	act14    a14;
	act15    a15;
	act16    a16;
	act17    a17;
	act18    a18;
	act19    a19;
	act20    a20;
	act21    a21;
	act22    a22;
	act23    a23;
	act24    a24;
	act25    a25;
	act26    a26;
	act27    a27;
	act28    a28;
	act29    a29;
	act30    a30;
	act31    a31;
	act32    a32;
	act33    a33;
	act34    a34;
	act35    a35;
	act36    a36;
	act37    a37;
	act38    a38;
	act39    a39;
	act40    a40;
	act41    a41;
	act42    a42;
	act43    a43;
	act44    a44;
	act45    a45;
	act46    a46;
	act47    a47;
	act48    a48;
	act49    a49;
};

struct event_t {
	act            *action;                         // Ptr to action to perform
	bool            localActionFl;                  // true if action is only for this screen
	uint32          time;                           // (absolute) time to perform action
	struct event_t *prevEvent;                      // Chain to previous event
	struct event_t *nextEvent;                      // Chain to next event
};

/**
 * Following are points for achieving certain actions.
 */
struct point_t {
	byte score;                                     // The value of the point
	bool scoredFl;                                  // Whether scored yet
};

class Scheduler {
public:
	Scheduler(HugoEngine *vm);
	virtual ~Scheduler();

	virtual void decodeString(char *line) = 0;
	virtual void runScheduler() = 0;

	int16 calcMaxPoints() const;

	void freeScheduler();
	void initCypher();
	void initEventQueue();
	void insertActionList(const uint16 actIndex);
	void loadActListArr(Common::ReadStream &in);
	void loadAlNewscrIndex(Common::ReadStream &in);
	void loadPoints(Common::SeekableReadStream &in);
	void loadScreenAct(Common::SeekableReadStream &in);
	void newScreen(const int screenIndex);
	void processBonus(const int bonusIndex);
	void processMaze(const int x1, const int x2, const int y1, const int y2);
	void restoreSchedulerData(Common::ReadStream *in);
	void restoreScreen(const int screenIndex);
	void saveSchedulerData(Common::WriteStream *out);
	void waitForRefresh();

protected:
	HugoEngine *_vm;
	static const int kFilenameLength = 12;          // Max length of a DOS file name
	static const int kMaxEvents = 50;               // Max events in event queue
	static const int kShiftSize = 8;                // Place hero this far inside bounding box

	Common::String _cypher;

	uint16   _actListArrSize;
	uint16   _alNewscrIndex;
	uint16   _screenActsSize;
	uint16 **_screenActs;

	byte     _numBonuses;
	point_t *_points;

	uint32 _curTick;                                // Current system time in ticks
	uint32 _oldTime;                                // The previous wall time in ticks
	uint32 _refreshTimeout;

	event_t *_freeEvent;                            // Free list of event structures
	event_t *_headEvent;                            // Head of list (earliest time)
	event_t *_tailEvent;                            // Tail of list (latest time)
	event_t  _events[kMaxEvents];                   // Statically declare event structures

	act    **_actListArr;

	virtual const char *getCypher() const = 0;

	virtual uint32 getTicks() = 0;

	virtual void promptAction(act *action) = 0;

	event_t *doAction(event_t *curEvent);
	event_t *getQueue();

	uint32 getDosTicks(const bool updateFl);
	uint32 getWinTicks() const;

	void delEventType(const action_t actTypeDel);
	void delQueue(event_t *curEvent);
	void findAction(const act* action, int16* index, int16* subElem);
	void insertAction(act *action);
	void readAct(Common::ReadStream &in, act &curAct);
	void restoreActions(Common::ReadStream *f);
	void restoreEvents(Common::ReadStream *f);
	void restorePoints(Common::ReadStream *in);
	void saveActions(Common::WriteStream* f) const;
	void saveEvents(Common::WriteStream *f);
	void savePoints(Common::WriteStream *out) const;
	void screenActions(const int screenNum);

};

class Scheduler_v1d : public Scheduler {
public:
	Scheduler_v1d(HugoEngine *vm);
	~Scheduler_v1d();

	virtual void decodeString(char *line);
	virtual void runScheduler();

protected:
	virtual const char *getCypher() const;
	virtual uint32 getTicks();
	virtual void promptAction(act *action);
};

class Scheduler_v2d : public Scheduler_v1d {
public:
	Scheduler_v2d(HugoEngine *vm);
	virtual ~Scheduler_v2d();

	void decodeString(char *line);

protected:
	virtual const char *getCypher() const;

	void promptAction(act *action);
};

class Scheduler_v3d : public Scheduler_v2d {
public:
	Scheduler_v3d(HugoEngine *vm);
	~Scheduler_v3d();

protected:
	const char *getCypher() const;
};

class Scheduler_v1w : public Scheduler_v3d {
public:
	Scheduler_v1w(HugoEngine *vm);
	~Scheduler_v1w();

	void runScheduler();

protected:
	uint32 getTicks();

};
} // End of namespace Hugo
#endif //HUGO_SCHEDULE_H
