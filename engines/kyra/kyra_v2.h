/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_KYRA_V2_H
#define KYRA_KYRA_V2_H

namespace Kyra {

enum kSequences {
	kSequenceVirgin = 0,
	kSequenceWestwood = 1,
	kSequenceTitle = 2,
	kSequenceOverview = 3,
	kSequenceLibrary = 4,
	kSequenceHand = 5
};

class WSAMovieV2;
class KyraEngine_v2;

struct SequenceControl {
	int8 frameIndex;
	int8 frameDelay;
};

struct ActiveWSA {
	WSAMovieV2 *movie;
	uint16 currentFrame;
	uint16 endFrame;
	uint16 frameDelay;
	uint32 nextFrame;
	void (KyraEngine_v2::*callback)(int);
	const SequenceControl *control;
};

struct ActiveChat {
	uint16 strIndex;
	uint16 x;
	uint16 y;
	int duration;
	uint16 field_8;
	uint16 startTime;
	uint16 field_E;
};

struct Sequence {
	uint8 type;
	const char *filename;
	int (KyraEngine_v2::*callback)(int);
	uint8 frameDelay;
	uint16 duration;
	uint8 numFrames;
	bool timeOut;
	bool fadeOut;
};

class KyraEngine_v2 : public KyraEngine {
public:
	KyraEngine_v2(OSystem *system, const GameFlags &flags);
	~KyraEngine_v2();

	int go();

private:
	void seq_playSequences(int startSeq, int endSeq = -1);
	int seq_introWestwood(int seqNum);
	int seq_introTitle(int seqNum);
	int seq_introOverview(int seqNum);
	int seq_introLibrary(int seqNum);	
	int seq_introHand(int seqNum);

	void seq_introOverviewOver1(int currentFrame);
	void seq_introOverviewForest(int currentFrame);	
	void seq_introOverviewDragon(int currentFrame);
	void seq_loadWSA(int wsaNum, const char *filename, int frameDelay, void (KyraEngine_v2::*callback)(int) = 0, 
					 const SequenceControl *control = 0 );
	void seq_unloadWSA(int wsaNum);
	void seq_playWSAs();
	void seq_showChats();
	void seq_playIntroChat(uint8 chatNum);
	void seq_resetAllChatEntries();
	void seq_waitForChatsToFinish();
	void seq_setChatEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 unk1);

	void mainMenu();
	int init();
	
	ActiveWSA *_activeWSA;
	ActiveChat *_activeChat;
	uint8 *_gameShapes[50];
	uint8 *_mouseSHPBuf;

	static const char *_introSoundList[];
	static const int _introSoundListSize;
	static const char *_introStrings[];
	static const int _introStringsSize;
	
	int _introStringsDuration[21];
	
};

} // end of namespace Kyra

#endif
