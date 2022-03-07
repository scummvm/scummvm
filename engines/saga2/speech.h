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
 * aint32 with this program; if not, write to the Free Software
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

void    updateSpeech();
bool    isVisible(GameObject *obj);

#ifdef FRANKC
void    sentenceGenerator(char *);
void    abortSpeech();
void    abortAllSpeeches();
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

extern int16        speechButtonCount;      // count of speech buttons

//  Actor speech enums -- move these to include file

enum {
	speakContinued  = (1 << 0),         // Append next speech
	speakNoAnimate  = (1 << 1),         // Don't animate speaking
	speakWait       = (1 << 2),         // wait until speech finished
	speakLock       = (1 << 3)          // lock UI while speech in progress
};

class Speech {
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
	gPixelMap           _speechImage;
	gPort               _textPort;

private:
	//  Reconstruct this SpeechTask from an archive buffer
	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive this SpeechTask
	int32 archiveSize();

	//  Archive this SpeechTask in a buffer
	void *archive(void *buf);

	void write(Common::MemoryWriteStreamDynamic *out);

	bool setupActive();                  // render speech into temp image
	bool displayText();
	int16 fits(int16 space);
	void setWidth();
	bool calcPosition(StaticPoint16 &p);       // calculate position
	void remove();                   //  Remove from active list

public:

	enum SpeechFlags {
		spNoAnimate     = (1 << 0),         //  Don't animate actor
		spHasVoice      = (1 << 1),         //  The audio interface is playing this voice
		spQueued        = (1 << 2),         //  In active queue
		spActive        = (1 << 3),         //  Is current active speech
		spLock          = (1 << 4)          //  Lock UI while speaking
	};

	// remove speech, dealloc resources
	void dispose();

	//  Append text and samples to speech
	bool append(char *text, int32 sampID);

	//  Move speech to active list
	bool activate();

	//  Set speech to wake up thread when done
	void setWakeUp(ThreadID th) {
		thread = th;
	}

	//  See if its time to kill it
	bool longEnough();

	//  Abort the current speech.
	void abortSpeech();
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

	Common::List<Speech *> _list,
	                       _inactiveList;

	int8            lockFlag;

	void SetLock(int newState);

public:

	//  Constructor
	SpeechTaskList();

	//  Constructor -- reconstruct from archive buffer
	SpeechTaskList(void **buf);

	SpeechTaskList(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive the speech tasks
	int32 archiveSize();

	//  Create an archive of the speech tasks in an archive buffer
	void *archive(void *buf);

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Cleanup the speech tasks
	void cleanup();

	//  Allocate a new speech task
	Speech *newTask(ObjectID id, uint16 flags);

	//  Find a non-active speech for a given actor
	Speech *findSpeech(ObjectID id);

	Speech *currentActive() {
		if (_list.size() > 0)
			return _list.front();
		return nullptr;
	}

	int32 activeCount() {
		return _list.size();
	}

	int speechCount() {
		return _list.size() + _inactiveList.size();
	}

	void remove(Speech *p);
};

extern SpeechTaskList &speechList;


//  Initialize the speech task list
void initSpeechTasks();

//  Save the speech tasks in a save file
void saveSpeechTasks(Common::OutSaveFile *outS);
void loadSpeechTasks(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the speech task list
void cleanupSpeechTasks();

} // end of namespace Saga2

#endif
