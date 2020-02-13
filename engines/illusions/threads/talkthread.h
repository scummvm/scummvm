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

#ifndef ILLUSIONS_TALKTHREAD_H
#define ILLUSIONS_TALKTHREAD_H

#include "illusions/thread.h"

namespace Illusions {

class IllusionsEngine;
struct TalkEntry;

enum {
	kMsgQueryTalkThreadActive    = 0,
	kMsgClearSequenceId1         = 1,
	kMsgClearSequenceId2         = 2
};

class TalkThread : public Thread {
public:
	TalkThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		int16 duration, uint32 objectId, uint32 talkId, uint32 sequenceId1, uint32 sequenceId2,
		uint32 namedPointId);
	int onUpdate() override;
	void onSuspend() override;
	void onPause() override;
	void onUnpause() override;
	void onTerminated() override;
	void onKill() override;
	uint32 sendMessage(int msgNum, uint32 msgValue) override;
public:
	//field0 dw
	int _status;
	uint _flags;
	uint32 _textStartTime;
	uint32 _textEndTime;
	uint32 _textDuration;
	uint32 _defDurationMult;
	uint32 _textDurationElapsed;
	uint32 _durationMult;
	//field12 dw
	uint32 _objectId;
	uint32 _talkId;
	uint32 _sequenceId1;
	uint32 _sequenceId2;
	byte *_entryTblPtr;
	byte *_entryText;
	byte *_currEntryText;
	//field30 dd
	uint32 _namedPointId;
	uint32 _voiceStartTime;
	uint32 _voiceEndTime;
	uint32 _voiceDuration;
	uint32 _voiceDurationElapsed;
	void refreshText();
	int insertText();
	TalkEntry *getTalkResourceEntry(uint32 talkId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_TALKTHREAD_H
