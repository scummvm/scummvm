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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SPEECH_H
#define SAGA2_SPEECH_H

#include "saga2/objects.h"

namespace Saga2 {

#define MAX_SPEECH_PTRS 20
#define MAX_SAMPLES     50
#define CHARSPERSECOND  22
#define SHORTEST_DURATION   TICKSPERSECOND

extern  gPort       backPort;
void    TileToScreenCoords(const TilePoint &tp, Point16 &p);
void    updateSpeech();
extern  TilePoint centerActorCoords(void);
bool    isVisible(GameObject *obj);

#ifdef FRANKC
void    sentenceGenerator(char *);
void    abortSpeech(void);
void    abortAllSpeeches(void);
void    queueActorSpeech(
    GameObject          *obj,
    char                *text,
    int                 count,
    int32               sampleID,
    int                 flags
);
#endif

bool sayVoice(uint32 s[]);

//  REM: This function is no longer used by the speech code,
//  but it may be useful for other things.

int16 TextWrap(
    char            *lines[],
    int16           line_chars[],
    int16           line_pixels[],
    char            *text,                  // the text to render
    int16           width);                 // width to constrain text

extern int16        speechButtonCount;      // count of speech buttons

//  Actor speech enums -- move these to include file

enum {
	speakContinued  = (1 << 0),         // Append next speech
	speakNoAnimate  = (1 << 1),         // Don't animate speaking
	speakWait       = (1 << 2),         // wait until speech finished
	speakLock       = (1 << 3),         // lock UI while speech in progress
};

class Speech : private DNode {
private:
	friend class SpeechTaskList;
	friend void setNextActive();
	friend void deleteSpeech(ObjectID id);          // voice sound sample ID
	friend void updateSpeech();
	friend void queueActorSpeech(
	    GameObject          *obj,
	    char                *text,
	    int                 count,
	    int32               sampleID,
	    int                 flags
	);

	int16               sampleCount,        // number of sound samples
	                    charCount;          // number of characters in buffer

	Rect16              bounds;             // bounds of speech.
	uint16              penColor,           // penColor to draw in
	                    outlineColor;       // pen color for outline

	ObjectID            objID;              // ID of speaking object
	ThreadID            thread;             // SAGA thread to wake up when done

	int16               speechFlags;        // flags from speaking
	uint32              sampleID[MAX_SAMPLES];// voice sound sample ID
	char                speechBuffer[512];// longest possible speech

public:
	int16               selectedButton;     // which button was hit

private:
	//  Reconstruct this SpeechTask from an archive buffer
	void *restore(void *buf);

	//  Return the number of bytes needed to archive this SpeechTask
	int32 archiveSize(void);

	//  Archive this SpeechTask in a buffer
	void *archive(void *buf);

	bool setupActive(void);                  // render speech into temp image
	bool displayText(void);
	int16 fits(int16 space);
	void setWidth(void);
	bool calcPosition(Point16 &p);       // calculate position
	void remove(void);                   //  Remove from active list

public:

	enum SpeechFlags {
		spNoAnimate     = (1 << 0),         //  Don't animate actor
		spHasVoice      = (1 << 1),         //  The audio interface is playing this voice
		spQueued        = (1 << 2),         //  In active queue
		spActive        = (1 << 3),         //  Is current active speech
		spLock          = (1 << 4),         //  Lock UI while speaking
	};

	// remove speech, dealloc resources
	void dispose(void);

	//  Append text and samples to speech
	bool append(char *text, int32 sampID);

	//  Move speech to active list
	bool activate(void);

	//  Set speech to wake up thread when done
	void setWakeUp(ThreadID th) {
		thread = th;
	}

	//  See if its time to kill it
	bool longEnough(void);

	//  Abort the current speech.
	void abortSpeech(void);
};

class SpeechTaskList {
	friend void setNextActive();
	friend void deleteSpeech(ObjectID id);          // voice sound sample ID
	friend void updateSpeech();
	friend class    Speech;
	friend void queueActorSpeech(
	    GameObject          *obj,
	    char                *text,
	    int                 count,
	    int32               sampleID,
	    int                 flags
	);

	DList           activeList,
	                nonActiveList,
	                free;
	Speech          array[MAX_SPEECH_PTRS];

	int8            lockFlag;

	void SetLock(int newState);

public:

	//  Constructor
	SpeechTaskList(void);

	//  Constructor -- reconstruct from archive buffer
	SpeechTaskList(void **buf);

	//  Return the number of bytes needed to archive the speech tasks
	int32 archiveSize(void);

	//  Create an archive of the speech tasks in an archive buffer
	void *archive(void *buf);

	//  Cleanup the speech tasks
	void cleanup(void);

	//  Allocate a new speech task
	Speech *newTask(ObjectID id, uint16 flags);

	//  Find a non-active speech for a given actor
	Speech *findSpeech(ObjectID id);

	Speech *currentActive(void) {
		return (Speech *)activeList.first();
	}

	int32 activeCount(void) {
		return activeList.count();
	}
};

extern SpeechTaskList &speechList;


//  Initialize the speech task list
void initSpeechTasks(void);

//  Save the speech tasks in a save file
void saveSpeechTasks(SaveFileConstructor &saveGame);

//  Load the speech tasks from a save file
void loadSpeechTasks(SaveFileReader &saveGame);

//  Cleanup the speech task list
void cleanupSpeechTasks(void);

} // end of namespace Saga2

#endif
