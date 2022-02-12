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
 * In addition to the GPLv2 license mentioned above, MojoTouch has exclusively licensed
 * this code on November 10th, 2021, to be use in closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#include "groovie/logic/tlcgame.h"
#include "groovie/groovie.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/translation.h"
#include "common/util.h"

namespace Groovie {

// This a list of files for background music. This list is hardcoded in the TLC player.
const char *kTlcMusicFiles[] = {
	"ep01epm",  "ep01tatm", "amb_hs",   "amb_mr",   "amb_kr",
	"amb_mo",   "music_rc", "amb_ds",   "amb_ds3",  "amb_jr",
	"amb_mr4",  "amb_jr4",  "amb_jr2",  "amb_kr2",  "amb_mr2",
	"amb_br",   "amb_ds2",  "amb_jr3",  "amb_ds4",  "amb_kr3",
	"amb_to1",  "amb_to2",  "ep02epm",  "ep02tatm", "ep03epm",
	"ep03tatm", "ep04epm",  "ep04tatm", "ep05epm",  "ep05tatm",
	"ep06epm",  "ep06tatm", "ep07epm",  "ep07tatm", "ep08epm",
	"ep08tatm", "ep09epm",  "ep09tatm", "ep10epm",  "ep10tatm",
	"ep11epm",  "ep11tatm", "ep12epm",  "ep12tatm", "ep13epm",
	"ep13tatm", "ep14epm",  "ep14tatm", "ep15epm",  "ep15tatm"
};

const uint8 kTlcEpQuestToPlay[] = {
	0x0E, 0x0F, 0x0B, 0x10, 0x11,
	0x12, 0x0C, 0x0C, 0x09, 0x06,
	0x0F, 0x0C, 0x0B, 0x0D, 0x0D
};


TlcGame::TlcGame(byte *scriptVariables) :
	_numRegionHeaders(0), _regionHeader(NULL), _curQuestNumAnswers(-1), _epQuestionsData(NULL),
	_random("GroovieTlcGame"), _scriptVariables(scriptVariables),
	_tatHeaders(NULL), _tatQuestions(NULL), _curQuestRegions(), _epScoreBin(), _tatFlags() {
	_curAnswerIndex = 0;
	_epEpisodeIdx = 0;
	_epQuestionIdx = 0;
	_epQuestionNumOfPool = -1;
	_epQuestionsInEpisode = 0;
	_tatEpisodes = 0;
	_tatQuestCount = 0;
}

TlcGame::~TlcGame() {
	delete[] _regionHeader;
	delete[] _epQuestionsData;
	delete[] _tatHeaders;
	delete[] _tatQuestions;
}

void TlcGame::handleOp(uint8 op) {
	switch (op) {
	case 0:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Regions", op);
		opRegions();
		break;

	case 1:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Exit Polls", op);
		opExitPoll();
		break;

	case 2:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC TATFlags", op);
		opFlags();
		break;

	case 3:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC TATs (TODO)", op);
		opTat();
		break;

	default:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Invalid -> NOP", op);
	}
}

// This function is mainly for debugging purpose
void inline TlcGame::setScriptVar(uint16 var, byte value) {
	_scriptVariables[var] = value;
	debugC(5, kDebugLogic, "script variable[0x%03X] = %d (0x%04X)", var, value, value);
}

void inline TlcGame::setScriptVar16(uint16 var, uint16 value) {
	_scriptVariables[var] = value & 0xFF;
	_scriptVariables[var + 1] = (value >> 8) & 0xFF;
	debugC(5, kDebugLogic, "script variable[0x%03X, 0x%03X] = %d (0x%02X, 0x%02X)", var, var+1, value, _scriptVariables[var], _scriptVariables[var+1]);
}

uint16 inline TlcGame::getScriptVar16(uint16 var) {
	uint16 value;

	value = _scriptVariables[var];
	value += _scriptVariables[var + 1] << 8;

	return value;
}

// Gets the filename of the background music file.
const char *TlcGame::getTlcMusicFilename(int musicId) {
	return kTlcMusicFiles[musicId];
}

void TlcGame::opRegions() {
	if (_scriptVariables[0x1A] == 1) {
		regionsInit();
		setScriptVar(0x1A, 0);

	} else {
		regionsLoad();
	}
}


// Loads the header of the file regions.rle into memory. This files contains
// a database for the GUI positions (regions) of all answers for the questions.
void TlcGame::regionsInit() {
	Common::SeekableReadStream *regionsfile = 0;

	_curQuestNumAnswers = -1;

	// Check if header was already loaded.
	if (_regionHeader != NULL) {
		debugC(1, kDebugLogic, "TLC:RegionsInit: Regions already loaded.");
		return;
	}

	regionsfile = SearchMan.createReadStreamForMember("SYSTEM/REGIONS.RLE");
	if (!regionsfile) {
		error("TLC:RegionsInit: Could not open 'SYSTEM/REGIONS.RLE'");
	}

	// Read number of question entries
	_numRegionHeaders = regionsfile->readUint32LE();
	if (regionsfile->eos()) {
		error("TLC:RegionsInit: Error reading numEntries from 'REGIONS.RLE'");
	}

	// Read header for each question entry
	_regionHeader = new TlcRegionsHeader[_numRegionHeaders];
	for (int i = 0; i < _numRegionHeaders; i++) {
		regionsfile->read(_regionHeader[i].name, sizeof(TlcRegionsHeader::name));
		regionsfile->seek(25 - sizeof(TlcRegionsHeader::name), SEEK_CUR);
		_regionHeader[i].numAnswers = regionsfile->readUint32LE();
		_regionHeader[i].offset = regionsfile->readUint32LE();
	}

	if (regionsfile->eos()) {
		error("TLC:RegionsInit: Error reading headers from 'REGIONS.RLE'");
	}

	delete regionsfile;

	debugC(1, kDebugLogic, "TLC:RegionsInit: Loaded %d region headers", _numRegionHeaders);
}


// Loads the specific regions for one questions.
void TlcGame::regionsLoad() {
	// Check if initRegions was called before
	if (_regionHeader == NULL) {
		error("TLC:RegionsLoad: initRegions was not called.");
	}

	// Open regions.rle
	Common::SeekableReadStream *regionsfile = SearchMan.createReadStreamForMember("SYSTEM/REGIONS.RLE");
	if (!regionsfile) {
		error("TLC:RegionsLoad: Could not open 'SYSTEM/REGIONS.RLE'");
	}

	// Get length of question name from variables
	int nameLen = _scriptVariables[0x1B] * 10 + _scriptVariables[0x1C];
	if (nameLen >= ARRAYSIZE(TlcRegionsHeader::name)) {
		error("TLC:RegionsLoad: Name to long for loadRegions!");
	}

	// Decoded and copy name from variables
	char questName[sizeof(TlcRegionsHeader::name)];
	for (int i = 0; i < nameLen; i++) {
		setScriptVar(0x1D + i, _scriptVariables[0x1D + i] + 0x30);
		questName[i] = _scriptVariables[0x1D + i];
	}
	questName[nameLen] = '\0';

	// Search for the question entry
	for (int i = 0; i <= _numRegionHeaders; i++) {
		if (strcmp(questName, _regionHeader[i].name) == 0) {

			// move to coordinates for this question
			regionsfile->seek(_regionHeader[i].offset, SEEK_SET);

			// Copy region of each answer
			_curQuestNumAnswers = _regionHeader[i].numAnswers;
			for (int iAns = 0; iAns < _curQuestNumAnswers; iAns++) {
				_curQuestRegions[iAns].left   = regionsfile->readUint16LE();
				_curQuestRegions[iAns].top    = regionsfile->readUint16LE();
				_curQuestRegions[iAns].right  = regionsfile->readUint16LE();
				_curQuestRegions[iAns].bottom = regionsfile->readUint16LE();
			}

			delete regionsfile;

			debugC(1, kDebugLogic, "TLC:RegionsLoad: Loaded %d regions for question %s", _curQuestNumAnswers, questName);
			return;
		}
	}

	// If we got here, the entry was not found
	error("TLC:RegionsLoad: Question '%s' was not found", questName);
	delete regionsfile; // unreachable
}


void TlcGame::getRegionRewind() {
	_curAnswerIndex = 0;
}


int TlcGame::getRegionNext(uint16 &left, uint16 &top, uint16 &right, uint16 &bottom) {

	// Check if initialization was done
	if (_curQuestNumAnswers < 0) {
		warning("TLC:GetRegionNext: Uninitialized call to getRegionNext.");
		return -1;
	}

	// Check if there is another region for this answer
	if (_curAnswerIndex >= _curQuestNumAnswers) {
		return -1;
	}

	// return next region
	left   = _curQuestRegions[_curAnswerIndex].left;
	top    = _curQuestRegions[_curAnswerIndex].top;
	right  = _curQuestRegions[_curAnswerIndex].right;
	bottom = _curQuestRegions[_curAnswerIndex].bottom;
	_curAnswerIndex++;

	return 0;
}


void TlcGame::opExitPoll() {
	switch (_scriptVariables[0]) {
	case 0x00:
		epInit();
		break;
	case 0x01:
		epSelectNextQuestion();
		break;
	case 0x02:
		epResultQuestion();
		break;
	case 0x03:
		epResultEpisode();
		break;
	case 0x04:
		// Load internal score-bin 4 and 5
		_epScoreBin[4] = _scriptVariables[1];
		_epScoreBin[5] = _scriptVariables[2];
		setScriptVar(0, 0x09);
		debugC(1, kDebugLogic, "TLC:EpInitBins: Init bins: bin[4]=%d, bin[5]=%d", _epScoreBin[4], _epScoreBin[5]);

		break;
	default:
		// Unknown subcommand
		debugC(0, kDebugLogic, "TLC:opExitPoll: Unknown subcommand=%d", _scriptVariables[0]);
		setScriptVar(0, 0x08);
	}
}


void TlcGame::epInit() {
	Common::SeekableReadStream *epaidbfile = 0;
	int i;
	int numEpisodesInDB;
	uint32 scoreDataOffset;

	// Delete previous question data
	delete[] _epQuestionsData;

	// Get current episode from script variables
	_epEpisodeIdx = _scriptVariables[0x01] - 0x31;
	_epQuestionIdx = 0;

	// Init score bin 0..3 of overall 6
	_epScoreBin[0] = 0;
	_epScoreBin[1] = 0;
	_epScoreBin[2] = 0;
	_epScoreBin[3] = 0;
	if (_epEpisodeIdx == 0) {
		_epScoreBin[4] = 0;
		_epScoreBin[5] = 0;
	}

	// Open epaidb.rle
	epaidbfile = SearchMan.createReadStreamForMember("SYSTEM/EPAIDB.RLE");
	if (!epaidbfile) {
		error("TLC:EpInit: Could not open 'SYSTEM/EPAIDB.RLE'");
	}

	// Read number of episodes. It is not dynamic because the result functions highly depend on the database
	// Thus we knew, we must check the result functions, if this would change (most propably not)
	numEpisodesInDB = epaidbfile->readSint32LE();
	if (numEpisodesInDB != GROOVIE_TLC_MAX_EPSIODES) {
		error("TLC:EpInit: Unexpected number of episodes in epaidb.rle. Read: %d, expected: %d", numEpisodesInDB, GROOVIE_TLC_MAX_EPSIODES);
	}

	// read header of ep score data of current episode TO
	if (_epEpisodeIdx < 0 || _epEpisodeIdx >= numEpisodesInDB) {
		error("TLC:EpInit: Requested episode out of range (0..%d)", GROOVIE_TLC_MAX_EPSIODES-1);
	}
	epaidbfile->seek((_epEpisodeIdx * 8) + 4, SEEK_SET);
	_epQuestionsInEpisode = epaidbfile->readSint32LE();
	scoreDataOffset = epaidbfile->readSint32LE();

	// Load scores for this episode.
	_epQuestionsData = new TlcEpQuestionData[_epQuestionsInEpisode];
	epaidbfile->seek(scoreDataOffset, SEEK_SET);

	for (i = 0; i < _epQuestionsInEpisode; i++) {
		_epQuestionsData[i].questionUsed = false;
		_epQuestionsData[i].questionScore = epaidbfile->readUint32LE();
	}

	if (epaidbfile->eos()) {
		error("TLC:EpInit: Error reading scores from 'EPAIDB.RLE'");
	}

	// Close file
	delete epaidbfile;

	// Initialize register 0x01 with values from database
	if (_epEpisodeIdx >= ARRAYSIZE(kTlcEpQuestToPlay)) {
		error("TLC:EpInit: EposdeIdx out of range for init data of reg0x01");
	}
	setScriptVar(2, kTlcEpQuestToPlay[_epEpisodeIdx] + 0x30);

	// Return code
	setScriptVar(0, 0x09);

	debugC(1, kDebugLogic, "TLC:EpInit: For episode %d loaded %d question scores. Will play %d questions", _epEpisodeIdx+1, _epQuestionsInEpisode, kTlcEpQuestToPlay[_epEpisodeIdx]);
}


void TlcGame::epSelectNextQuestion() {
	_epQuestionIdx++;
	_epQuestionNumOfPool = -1;

	/* check if there is a dedicated quesition at this position */
	switch (_epEpisodeIdx) {    // _epEpsiodeIdx: 0..14
	case  1:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool =  5; break;
		case  7: _epQuestionNumOfPool = 16; break;
		case 11: _epQuestionNumOfPool = 18; break;
		}
		break;
	case  2:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool =  7; break;
		case  7: _epQuestionNumOfPool = 13; break;
		case 11: _epQuestionNumOfPool = 16; break;
		}
		break;
	case  3:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool =  8; break;
		case 11: _epQuestionNumOfPool = 17; break;
		}
		break;
	case  4:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 18; break;
		case  7: _epQuestionNumOfPool = 22; break;
		case 11: _epQuestionNumOfPool = 21; break;
		}
		break;
	case  5:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 22; break;
		case 11: _epQuestionNumOfPool = 24; break;
		}
		break;
	case  6:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 12; break;
		case  7: _epQuestionNumOfPool = 14; break;
		case 11: _epQuestionNumOfPool = 18; break;
		}
		break;
	case  7:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 15; break;
		case  7: _epQuestionNumOfPool = 16; break;
		case 11: _epQuestionNumOfPool = 17; break;
		}
		break;
	case  8:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 11; break;
		case  7: _epQuestionNumOfPool = 14; break;
		case  9: _epQuestionNumOfPool = 15; break;
		}
		break;
	case  9:
		switch (_epQuestionIdx) {
		case  2: _epQuestionNumOfPool =  2; break;
		case  4: _epQuestionNumOfPool =  8; break;
		case  6: _epQuestionNumOfPool = 11; break;
		}
		break;
	case 10:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 15; break;
		case  7: _epQuestionNumOfPool = 16; break;
		case 11: _epQuestionNumOfPool = 17; break;
		}
		break;
	case 11:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 14; break;
		case  7: _epQuestionNumOfPool = 15; break;
		case 11: _epQuestionNumOfPool = 17; break;
		}
		break;
	case 12:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 12; break;
		case  7: _epQuestionNumOfPool = 14; break;
		case 11: _epQuestionNumOfPool = 16; break;
		}
		break;
	case 13:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool = 15; break;
		case  7: _epQuestionNumOfPool = 18; break;
		case 11: _epQuestionNumOfPool = 19; break;
		}
		break;
	case 14:
		switch (_epQuestionIdx) {
		case  3: _epQuestionNumOfPool =  1; break;
		case  7: _epQuestionNumOfPool = 14; break;
		case 11: _epQuestionNumOfPool = 16; break;
		}
		break;
	}

	// get a random question if there was no predefined
	if (_epQuestionNumOfPool == -1) {
		do {
			_epQuestionNumOfPool = _random.getRandomNumber(32767) / 2000;
		} while (_epQuestionNumOfPool < 1 || _epQuestionNumOfPool > _epQuestionsInEpisode);

		debugC(1, kDebugLogic, "TLC:EpSelNextQuest: Question %d: Selected question %d/%d by random.", _epQuestionIdx, _epQuestionNumOfPool, _epQuestionsInEpisode);

	} else {
		debugC(1, kDebugLogic, "TLC:EpSelNextQuest: Question %d: Selected question %d/%d by predefined data.", _epQuestionIdx, _epQuestionNumOfPool, _epQuestionsInEpisode);
	}

	// Choose next question, if question was already played
	while (_epQuestionsData[_epQuestionNumOfPool - 1].questionUsed) {
		_epQuestionNumOfPool++;
		if (_epQuestionNumOfPool > _epQuestionsInEpisode) {
			_epQuestionNumOfPool = 1;
		}
	}
	_epQuestionsData[_epQuestionNumOfPool - 1].questionUsed = true;
	debugC(1, kDebugLogic, "TLC:EpSelNextQuest: Question %d: Forward to question %d/%d. (used-flag)", _epQuestionIdx, _epQuestionNumOfPool, _epQuestionsInEpisode);

	// write selected episode and question to script variables
	setScriptVar(4, (_epEpisodeIdx + 1) / 10);
	setScriptVar(5, (_epEpisodeIdx + 1) % 10);
	setScriptVar(6, _epQuestionNumOfPool / 10);
	setScriptVar(7, _epQuestionNumOfPool % 10);

	// Set return value
	setScriptVar(0, 9);

	// Debug output
	{
		uint32 dbgQScore = _epQuestionsData[_epQuestionNumOfPool - 1].questionScore;
		debugC(1, kDebugLogic, "TLC:EpSelNextQuest: Bins for Answers: %d %d %d %d %d %d %d %d",
			(dbgQScore >> 28) & 0xf, (dbgQScore >> 24) & 0xf, (dbgQScore >> 20) & 0xf, (dbgQScore >> 16) & 0xf,
			(dbgQScore >> 12) & 0xf, (dbgQScore >> 8) & 0xf, (dbgQScore >> 4) & 0xf, (dbgQScore) & 0xf);
	}

}


void TlcGame::epResultQuestion() {
	int answerIdx, shift, scoreBinId;
	int specialReg;
	uint32 questionScore;

	// Add special question results here
	specialReg = -1;
	switch (_epEpisodeIdx) {    // _epEpsiodeIdx: 0..14
	case  1:
		switch (_epQuestionNumOfPool) {
		case  5: specialReg = 0x09; break;
		case 16: specialReg = 0x0A; break;
		case 18: specialReg = 0x0B; break;
		}
		break;
	case  2:
		switch (_epQuestionNumOfPool) {
		case  7: specialReg = 0x09; break;
		case 13: specialReg = 0x0A; break;
		case 16: specialReg = 0x0B; break;
		}
		break;
	case  3:
		switch (_epQuestionNumOfPool) {
		case  8: specialReg = 0x09; break;
		case 17: specialReg = 0x0A; break;
		}
		break;
	case  4:
		switch (_epQuestionNumOfPool) {
		case 18: specialReg = 0x09; break;
		case 22: specialReg = 0x0A; break;
		case 21: specialReg = 0x0B; break;
		}
		break;
	case  5:
		switch (_epQuestionNumOfPool) {
		case 22: specialReg = 0x09; break;
		case 24: specialReg = 0x0A; break;
		}
		break;
	case  6:
		switch (_epQuestionNumOfPool) {
		case 12: specialReg = 0x09; break;
		case 14: specialReg = 0x0A; break;
		case 18: specialReg = 0x0B; break;
		}
		break;
	case  7:
		switch (_epQuestionNumOfPool) {
		case 15: specialReg = 0x09; break;
		case 16: specialReg = 0x0A; break;
		case 17: specialReg = 0x0B; break;
		}
		break;
	case  8:
		switch (_epQuestionNumOfPool) {
		case 11: specialReg = 0x09; break;
		case 14: specialReg = 0x0A; break;
		case 15: specialReg = 0x0B; break;
		}
		break;
	case  9:
		switch (_epQuestionNumOfPool) {
		case  2: specialReg = 0x09; break;
		case  8: specialReg = 0x0A; break;
		case 11: specialReg = 0x0B; break;
		}
		break;
	case 10:
		switch (_epQuestionNumOfPool) {
		case 15: specialReg = 0x09; break;
		case 16: specialReg = 0x0A; break;
		case 17: specialReg = 0x0B; break;
		}
		break;
	case 11:
		switch (_epQuestionNumOfPool) {
		case 14: specialReg = 0x09; break;
		case 15: specialReg = 0x0A; break;
		case 17: specialReg = 0x0B; break;
		}
		break;
	case 12:
		switch (_epQuestionNumOfPool) {
		case 12: specialReg = 0x09; break;
		case 14: specialReg = 0x0A; break;
		case 16: specialReg = 0x0B; break;
		}
		break;
	case 13:
		switch (_epQuestionNumOfPool) {
		case 15: specialReg = 0x09; break;
		case 18: specialReg = 0x0A; break;
		case 19: specialReg = 0x0B; break;
		}
		break;
	case 14:
		switch (_epQuestionNumOfPool) {
		case  1: specialReg = 0x09; break;
		case 14: specialReg = 0x0A; break;
		case 16: specialReg = 0x0B; break;
		}
		break;
	}

	// Add value of register 3 (answer register) to spezial register
	if (specialReg >= 0) {
		setScriptVar(specialReg, _scriptVariables[specialReg] + _scriptVariables[3]);
		debugC(1, kDebugLogic, "TLC:EpResultQuest: Question: %d vars[0x%02x] += %d. New Value: %d", _epQuestionIdx, specialReg, _scriptVariables[3], _scriptVariables[specialReg]);
	}


	// Process info from score database
	answerIdx = _scriptVariables[3];
	shift = (7 - answerIdx) * 4;
	questionScore = _epQuestionsData[_epQuestionNumOfPool - 1].questionScore;
	scoreBinId = (questionScore >> shift) & 0xF;
	if (scoreBinId > 5) {
		error("TLC:EpResultQuest: Invalid score bin %d (0..5 allowed)", scoreBinId);
	}

	_epScoreBin[scoreBinId] = _epScoreBin[scoreBinId] + 1;

	debugC(1, kDebugLogic, "TLC:EpResultQuest: Answer: %d -> Inc bin[%d] -> bin[0..5] = %d, %d, %d, %d, %d, %d",
		answerIdx+1, scoreBinId, _epScoreBin[0], _epScoreBin[1], _epScoreBin[2], _epScoreBin[3], _epScoreBin[4], _epScoreBin[5]);
}
/*
 * Processes the result of the questions for this episode.
 * _epScoreBin[   0]: Ignored. Used if this answer for a question has no influence. Reset with each new Exit Poll.
 * _epScoreBin[1..3]: Seems to be used to select alternative video for this episode. Reset with each new Exit Poll.
 * _epScoreBin[4..5]: Seems to be used over the whole game. (Values are kept over the episodes in the script variables.)
 */
void TlcGame::epResultEpisode() {

	uint16 maxBinValue;
	int    i;

	/* keep only the maxium scores of bin[1], bin[2], bin[3]. -> Set all other to 0 */
	debugCN(1, kDebugLogic, "TLC:EpResultEpisode: bins[1..3] = %d, %d, %d ", _epScoreBin[1], _epScoreBin[2], _epScoreBin[3]);
	maxBinValue = _epScoreBin[1];
	for (i = 2; i < 4; i++) {
		if (maxBinValue < _epScoreBin[i]) {
			maxBinValue = _epScoreBin[i];
		}
	}
	for (i = 1; i < 4; i++) {
		if (_epScoreBin[i] < maxBinValue) {
			_epScoreBin[i] = 0;
		}
	}
	debugC(1, kDebugLogic, "-> bins[1..3] = %d, %d, %d ", _epScoreBin[1], _epScoreBin[2], _epScoreBin[3]);

	/* Select next stream according to which bin(s) are still >0. */
	if (_epScoreBin[1] != 0 && _epScoreBin[2] == 0 && _epScoreBin[3] == 0) {
		setScriptVar(3, 1);
	} else if (_epScoreBin[1] == 0 && _epScoreBin[2] != 0 && _epScoreBin[3] == 0) {
		setScriptVar(3, 2);
	} else if (_epScoreBin[1] == 0 && _epScoreBin[2] == 0 && _epScoreBin[3] != 0) {
		setScriptVar(3, 3);
	} else if (_epScoreBin[1] != 0 && _epScoreBin[2] != 0 && _epScoreBin[3] == 0) {
		setScriptVar(3, _random.getRandomNumberRng(1, 2));
	} else if (_epScoreBin[1] != 0 && _epScoreBin[2] == 0 && _epScoreBin[3] != 0) {
		setScriptVar(3, (_random.getRandomNumberRng(0, 1) * 2) + 1);
	} else if (_epScoreBin[1] == 0 && _epScoreBin[2] != 0 && _epScoreBin[3] != 0) {
		setScriptVar(3, _random.getRandomNumberRng(2, 3));
	} else if (_epScoreBin[1] != 0 && _epScoreBin[2] != 0 && _epScoreBin[3] != 0) {
		setScriptVar(3, _random.getRandomNumberRng(1, 3));
	} else {
		error("Tlc:EpResultEpisode: Stream selection failed. bins[0..5] = %d, %d, %d, %d, %d, %d",
			  _epScoreBin[0], _epScoreBin[1], _epScoreBin[2], _epScoreBin[3], _epScoreBin[4], _epScoreBin[5]);
	}
	debugC(1, kDebugLogic, "Selected stream [1..3] = %d ", _scriptVariables[3]);

	/* save bin values of bin[4..5] to script variables */
	setScriptVar(1, _epScoreBin[4]);
	setScriptVar(2, _epScoreBin[5]);

	/* return values */
	setScriptVar(0, 9);
}


void TlcGame::opFlags() {
	int x;
	int y;

	switch (_scriptVariables[0]) {

	// Initialize the flags all to 0. Done at the beginning of a new TAT
	case 0x00:
		for (x = 0; x < 0x0E; x++) {
			for (y = 0; y < 0x09; y++) {
				_tatFlags[x][y] = 0;
			}
		}
		debugC(0, kDebugLogic, "Tlc:TatFlags: Initialized fields (%d, %d)", x, y);
		break;

	// Get and set flags
	case 0x01:
		// Calculate position in flag field
		x = 10 * _scriptVariables[0x04] + _scriptVariables[0x05];
		y = _scriptVariables[0x06];

		if (x >= 0x0E) {
			warning("Tlc:TatFlags: x=%d out of range (0...13).", x);
			x = 0x0E;
		}
		if (y >= 0x09) {
			warning("Tlc:TatFlags: y=%d out of range (0...8).", x);
			x = 0x0E;
		}

		// Check flags in field
		if (_tatFlags[x][y] == 0) {
			setScriptVar(0x01, 0);
			_tatFlags[x][y] = 1;

			debugC(1, kDebugLogic, "Tlc:TatFlags: Set x=%d, y=%d to 1", x, y);

			debugTatFlags(0, 1);
			debugTatFlags(2, 3);
			debugTatFlags(4, 5);
			debugTatFlags(6, 7);
		}
		else {
			setScriptVar(0x01, 1);
		}
		break;
	}
}


void TlcGame::debugTatFlags(int y1, int y2) {
	Common::String s1, s2;
	for (int x = 0; x < 14; x++) {
		s1 += Common::String::format("%d", _tatFlags[x][y1]);
		s2 += Common::String::format("%d", _tatFlags[x][y2]);
	}

	debugC(0, kDebugLogic, "Tlc:TatFlags: %s  %s", s1.c_str(), s2.c_str());
}


void TlcGame::opTat() {
	switch (_scriptVariables[0x40]) {
	case 1:
		tatInitRegs();
		setScriptVar(0x40, 0);
		break;

	case 2:
		tatLoadDB();
		setScriptVar(0x40, 0);
		break;

	case 3:
		tatResultQuest();
		setScriptVar(0x40, 0);
		break;

	case 4:
		tatResultEpisode();
		setScriptVar(0x40, 0);
		break;

	case 9:
		tatGetProfile();
		setScriptVar(0x40, 0);
		break;
	}
}


void TlcGame::tatInitRegs() {
	for (int i = 0; i < 0x10; i++) {
		setScriptVar(0x4D + i, 0);
		setScriptVar16(0x5D + i*2, 0);
	}

	// TODO:
	// memset(_tatUnkData0_14, 0, 15);
}


void TlcGame::tatLoadDBHeaders() {
	// Load tat headers if not already done
	if (_tatHeaders == NULL) {
		// Open tataidb.rle
		Common::SeekableReadStream *tataidbfile = SearchMan.createReadStreamForMember("SYSTEM/TATAIDB.RLE");
		if (!tataidbfile) {
			error("TLC:TatLoadDB: Could not open 'SYSTEM/TATAIDB.RLE'");
		}

		_tatEpisodes = tataidbfile->readUint32LE();
		_tatHeaders = new TlcTatHeader[_tatEpisodes];

		for (int iEpisode = 0; iEpisode < _tatEpisodes; iEpisode++) {
			_tatHeaders[iEpisode].questionsNum = tataidbfile->readUint32LE();
			_tatHeaders[iEpisode].questionsOffset = tataidbfile->readUint32LE();
			for (int iBin = 0; iBin < 16; iBin++) {
				_tatHeaders[iEpisode].binDividends[iBin] = tataidbfile->readByte();
			}
		}
		if (tataidbfile->eos()) {
			error("TLC:TatLoadDB: Error reading headers from 'TATAIDB.RLE'");
		}
	}
}


void TlcGame::tatLoadDB() {
	Common::SeekableReadStream *tataidbfile = 0;
	int episode;
	uint32 questOffset;

	for (int iBin = 0; iBin < 0x10; iBin++) {
		setScriptVar(0x4D + iBin, 0);
	}

	// Load TAT headers
	tatLoadDBHeaders();

	// Load questions for the requested episode
	episode = _scriptVariables[0x47] - 0x31;    // -'1'
	_tatQuestCount = _tatHeaders[episode].questionsNum;
	questOffset = _tatHeaders[episode].questionsOffset;

	delete[] _tatQuestions;
	_tatQuestions = new TlcTatQuestions[_tatQuestCount];

	// Open tataidb.rle and seek correct position
	tataidbfile = SearchMan.createReadStreamForMember("SYSTEM/TATAIDB.RLE");
	if (!tataidbfile) {
		error("TLC:TatLoadDB: Could not open 'SYSTEM/TATAIDB.RLE'");
	}
	tataidbfile->seek(questOffset, SEEK_SET);

	for (int iQuest = 0; iQuest < _tatQuestCount; iQuest++) {
		tataidbfile->read(_tatQuestions[iQuest].name, 5);
		_tatQuestions[iQuest].name[5] = '\0';
		_tatQuestions[iQuest].answerCount = tataidbfile->readByte();

		for (int iAns = 0; iAns < _tatQuestions[iQuest].answerCount; iAns++) {
			for (int iBin = 0; iBin < 16; iBin++) {
				_tatQuestions[iQuest].answerData[iAns].binScore[iBin] = tataidbfile->readByte();
			}
		}
	}

	if (tataidbfile->eos()) {
		error("TLC:TatLoadDB: Error reading questions from 'TATAIDB.RLE'");
	}

	// tatData[iQ*232  ] <= 5 Bytes Name
	// tatData[iQ*232+6] <= 1 Byte iAnswers

}

void TlcGame::tatResultQuest() {
	char questName[6];
	int  questIdx;
	int  selectedAns;
	if (_tatQuestions == NULL) {
		error("TLC:ResultQuest: Error, TATAIDB not loaded.");
	}

	// Get name of current question from script variables
	for (int i = 0; i < 5; i++) {
		questName[i] = _scriptVariables[0x41 + i] + 0x30;
	}
	questName[5] = '\0';

	// search for question in the database
	questIdx = -1;
	do {
		questIdx++;
		if (questIdx >= _tatQuestCount) {
			error("TLC:ResultQuest: Could not find question '%s' in TATAIDB. Count: %d", questName, _tatQuestCount);
		}
	} while (scumm_stricmp(questName, _tatQuestions[questIdx].name) != 0);

	// Get selected answer. Range: 0..7
	selectedAns = _scriptVariables[0x46];
	if (selectedAns >= _tatQuestions[questIdx].answerCount) {
		error("TLC:ResultQuest: Chosen answer out of range for question: '%s'. Answer: %d/%d, questIdx: %d", questName, selectedAns+1, _tatQuestions[questIdx].answerCount, questIdx);
	}

	// Add answer score for each bin to the dedicated script variables
	for (int iBin = 0; iBin < 16; iBin++) {
		int score = _tatQuestions[questIdx].answerData[selectedAns].binScore[iBin];
		setScriptVar(0x4d + iBin, _scriptVariables[0x4D + iBin] + score);
	}
}

void TlcGame::tatResultEpisode() {

	int episode = _scriptVariables[0x47] - 0x31;
	float ratioCur = 0;
	float ratioA = 0;
	float ratioB = 0;
	int idxA = 0xff;
	int idxB = 0xff;
	int product;
	char resultStrA[5];
	char resultStrB[5];

	// Process the bin scores
	for (int iBin = 0; iBin < 16; iBin++) {
		int binScoreSum = _scriptVariables[0x4D + iBin];

		// increment 16 Bit interpreted variables with score sum of the current episode
		setScriptVar16(0x5D + iBin * 2, getScriptVar16(0x5D + iBin * 2) + binScoreSum);

		// Find the two biggest bin ratios. Remeber idx and ratio, A is biggest, B second
		if (binScoreSum != 0) {
			ratioCur = (float)binScoreSum / (float)_tatHeaders[episode].binDividends[iBin];
			if (ratioCur > ratioA) {
				ratioB = ratioA;
				idxB = idxA;
				ratioA = ratioCur;
				idxA = iBin;
			} else {
				if (ratioCur > ratioB) {
					ratioB = ratioCur;
					idxB = iBin;
				}
			}
		}
	}

	// Process result according to biggest ratio (ratioA)
	switch (idxA) {
	case 0:
		product = ratioA * 18.0;
		if (product <= 3) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 4);
		}
		break;

	case 1:
		product = ratioA * 13.0;
		if (product <= 3) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 4);
		}
		break;

	case 2:
		product = ratioA * 12.0;
		if (product <= 3) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 4);
		}
		break;

	case 3:
		product = ratioA * 13.0;
		if (product <= 8) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 9);
		}
		break;

	case 4:
		product = ratioA * 11.0;
		if (product <= 3) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 4);
		}
		break;

	case 5:
		product = ratioA * 11.0;
		if (product >= 4) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product - 4);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		}
		break;

	case 6:
		product = ratioA * 9.0;
		if (product <= 4) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 5);
		}
		break;

	case 7:
		product = ratioA * 10.0;
		if (product <= 3) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 4);
		}
		break;

	case 8:
		product = ratioA * 12.0;
		if (product <= 4) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product - 5);
		}
		break;

	case 9:
		product = ratioA * 10.0;
		sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		break;

	case 10:
		product = ratioA * 7.0;
		sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		break;

	case 11:
		product = ratioA * 10.0;
		if (product >= 4) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product - 4);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		}
		break;

	case 12:
		product = ratioA * 9.0;
		if (product >= 4) {
			sprintf(resultStrA, "%cP%02d", idxA + 'A', product - 4);
		} else {
			sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		}
		break;

	case 13:
		product = ratioA * 6.0;
		sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		break;

	case 14:
		product = ratioA * 7.0;
		sprintf(resultStrA, "%cP%02d", idxA + 'A', product);
		break;

	case 15:
		product = ratioA * 8.0;
		sprintf(resultStrA, "%cN%02d", idxA + 'A', product);
		break;
	}


	// The same for the second biggest ratio (ratioB)
	switch (idxB) {
	case 0:
		product = ratioB * 18.0;
		if (product <= 3) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 4);
		}
		break;

	case 1:
		product = ratioB * 13.0;
		if (product <= 3) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 4);
		}
		break;

	case 2:
		product = ratioB * 12.0;
		if (product <= 3) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 4);
		}
		break;

	case 3:
		product = ratioB * 13.0;
		if (product <= 8) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 9);
		}
		break;

	case 4:
		product = ratioB * 11.0;
		if (product <= 3) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 4);
		}
		break;

	case 5:
		product = ratioB * 11.0;
		if (product >= 4) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product - 4);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		}
		break;

	case 6:
		product = ratioB * 9.0;
		if (product <= 4) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 5);
		}
		break;

	case 7:
		product = ratioB * 10.0;
		if (product <= 3) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 4);
		}
		break;

	case 8:
		product = ratioB * 12.0;
		if (product <= 4) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product - 5);
		}
		break;

	case 9:
		product = ratioB * 10.0;
		sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		break;

	case 10:
		product = ratioB * 7.0;
		sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		break;

	case 11:
		product = ratioB * 10.0;
		if (product >= 4) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product - 4);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		}
		break;

	case 12:
		product = ratioB * 9.0;
		if (product >= 4) {
			sprintf(resultStrB, "%cP%02d", idxB + 'A', product - 4);
		} else {
			sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		}
		break;

	case 13:
		product = ratioB * 6.0;
		sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		break;

	case 14:
		product = ratioB * 7.0;
		sprintf(resultStrB, "%cP%02d", idxB + 'A', product);
		break;

	case 15:
		product = ratioB * 8.0;
		sprintf(resultStrB, "%cN%02d", idxB + 'A', product);
		break;
	}

	// Write result to script variables
	setScriptVar(0x4D, resultStrA[0] - 0x30);
	setScriptVar(0x4E, resultStrA[1] - 0x30);
	setScriptVar(0x4F, resultStrA[2] - 0x30);
	setScriptVar(0x50, resultStrA[3] - 0x30);
	setScriptVar(0x51, resultStrB[0] - 0x30);
	setScriptVar(0x52, resultStrB[1] - 0x30);
	setScriptVar(0x53, resultStrB[2] - 0x30);
	setScriptVar(0x54, resultStrB[3] - 0x30);
	setScriptVar(0x55, '\0');
}


void TlcGame::tatGetProfile() {
	uint16 sumBinDivs[16];
	float  binRatios[16];
	int iBin, iEpisode;

	for (iBin = 0; iBin < 16; iBin++) {
		sumBinDivs[iBin] = 0;
	}

	// Load scoretable by summing all dividends for each episode
	tatLoadDBHeaders();
	for (iEpisode = 0; iEpisode < _tatEpisodes; iEpisode++) {
		for (iBin = 0; iBin < 16; iBin++) {
			sumBinDivs[iBin] += _tatHeaders[iEpisode].binDividends[iBin];
		}
	}

	// Calculate ratio of each bin
	for (iBin = 0; iBin < 16; iBin++) {
		binRatios[iBin] = (float)getScriptVar16(0x5D + 2 * iBin) / (float)sumBinDivs[iBin];
	}

	// Select higher ratio of each pair (A=iBin and B=iBin+1) and 1 or 2 accoring to threshold
	for (iBin = 0; iBin < 16; iBin += 2) {
		if (binRatios[iBin] > binRatios[iBin + 1]) {

			setScriptVar(0x4d + iBin, 'A' + iBin);
			if (binRatios[iBin] > 0.5) {
				setScriptVar(0x4e + iBin, '1');
			} else {
				setScriptVar(0x4e + iBin, '2');
			}
		} else {

			setScriptVar(0x4d + iBin, 'B' + iBin);
			if (binRatios[iBin + 1] > 0.5) {
				setScriptVar(0x4e + iBin, '1');
			} else {
				setScriptVar(0x4e + iBin, '2');
			}
		}
	}

	// Adapt former set script variables (all -0x30)
	for (iBin = 0; iBin < 16; iBin++) {
		setScriptVar(0x4d + iBin, _scriptVariables[0x4d + iBin] - '0');
	}
}

} // End of Namespace Groovie
