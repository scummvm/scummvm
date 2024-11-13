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
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, MojoTouch has exclusively licensed
 * this code on November 10th, 2021, to be use in closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#ifndef GROOVIE_LOGIC_TLCGAME_H
#define GROOVIE_LOGIC_TLCGAME_H

#include "common/textconsole.h"
#include "common/random.h"

#define GROOVIE_TLC_MAX_EPSIODES    (15)
#define GROOVIE_TLC_MAX_QUEST_EP	(50)

namespace Groovie {

class GroovieEngine;

// The regions.rle contains 898 entries. Round about 18 kByte in memory.
struct TlcRegionsHeader {
	char   name[12];
	int    numAnswers;
	uint32 offset;
};

struct TlcRegion {
	uint16 left;
	uint16 top;
	uint16 right;
	uint16 bottom;
};

struct TlcEpQuestionData {
	bool   questionUsed;
	uint32 questionScore;
};

struct TlcTatHeader {
	uint32 questionsNum;
	uint32 questionsOffset;
	uint8  binDividends[16];
};

struct TlcTatAnswer {
	uint8  binScore[16];
};

struct TlcTatQuestions {
	char name[6];
	int  answerCount;
	TlcTatAnswer answerData[8];
};

class TlcGame
{
public:
#ifdef ENABLE_GROOVIE2
	TlcGame(byte *scriptVariables);
	~TlcGame();

	static const char *getTlcMusicFilename(int musicId);

	void handleOp(uint8 op);

	/**
	 * Handle region commands. A region describes the coordinates of
	 * a rectangle as clickable area in the question dialogs. These regions
	 * are provided by a the extra file.
	 * screen coordinates.
	 */
	void opRegions();

	/**
	 * Get the coordiantes of the region for the next answer. There are
	 * up to 8 answers possible for each question. In the script the
	 * coordinates are (0,0,0,0) which will be replace by the new ones.
	 * @param left        Left value of the rectangle
	 * @param top         Top value of the rectangle
	 * @param right       Right value of the rectangle
	 * @param bottom      Bottom value of the rectangle
	 * @return            0 if anwer was found. -1 in case no more answer
	 *                    available for this question
	 */
	int getRegionNext(uint16 &left, uint16 &top, uint16 &right, uint16 &bottom);

	/**
	 * Rewinds the internal answer counter for the function
	 * getRegionNext()
	 */
	void getRegionRewind();


	/**
	 * Handles some flags which are used during a TAT. The game seems to
	 * use this flags to skip some questions during a TAT.
	 * OpCode_0x42(2)
	 */
	void opFlags();


	/**
	 * Handles all Exit Poll commands. The exit poll (EP) questions are
	 * described in detail in the file EPAIDB.RLE.
	 * OpCode_0x42(1)
	 */
	void opExitPoll();

	/**
	 * Handles all TAT commands. The TAT questions are described in detail
	 * in the file TATAIDB.RLE
	 */
	void opTat();

private:
	Common::RandomSource _random;

	void inline setScriptVar(uint16 var, byte value);
	void inline setScriptVar16(uint16 var, uint16 value);
	uint16 inline getScriptVar16(uint16 var);
	byte *_scriptVariables;

	/**
	 * Loads the description part of the regions.rle file into memory
	 * This makes it faster to search for the correct quesion.
	 */
	void regionsInit();
	void regionsLoad();

	// Variables for region handling
	int       _numRegionHeaders;
	int       _curAnswerIndex;
	int       _curQuestNumAnswers;
	TlcRegion _curQuestRegions[8];
	TlcRegionsHeader *_regionHeader;

	/**
	 * Functions for Exit Poll Commands
	 */
	void epInit();
	void epSelectNextQuestion();
	void epResultQuestion();
	void epResultEpisode();

	// Variables for Exit Poll handling
	int16  _epScoreBin[6];
	int    _epEpisodeIdx;                   // 15 Episodes: 0..14
	int    _epQuestionIdx;					// 1..X (questions in current episode. counted up for every question)
	int    _epQuestionNumOfPool;            // 1..X (question number in the data base. The questions are played in random order)
	int    _epQuestionsInEpisode;
	TlcEpQuestionData *_epQuestionsData;

	// Variables for flag handling
	byte _tatFlags[0x0E][0x09];

	/**
	 * Functions for TAT Commands
	 */
	void tatInitRegs();
	void tatLoadDB();
	void tatLoadDBHeaders();
	void tatResultQuest();
	void tatResultEpisode();
	void tatGetProfile();
	void debugTatFlags(int y, int y2);

	// Variables for TAT handling
	int _tatEpisodes;
	int _tatQuestCount;
	TlcTatHeader *_tatHeaders;
	TlcTatQuestions *_tatQuestions;
	// uint8 _tatCoeffs[15][16];
#endif
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_TLCGAME_H
