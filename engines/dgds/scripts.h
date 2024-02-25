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
 */

#ifndef DGDS_SCRIPTS_H
#define DGDS_SCRIPTS_H

#include "common/rect.h"
#include "dgds/parser.h"
#include "dgds/scene.h"

namespace Dgds {

class DgdsEngine;
class DgdsChunkReader;


class ScriptParserData : public ParserData {
public:
	ScriptParserData() : scr(nullptr) {}
	Common::String filename;
	Common::SeekableReadStream *scr;
	Common::HashMap<uint16, Common::String> _tags;
};

class TTMEnviro : public ScriptParserData {
public:
	TTMEnviro() : _totalFrames(330), _enviro(0), ScriptParserData() {
		ARRAYCLEAR(_scriptPals);
	}
	uint16 _enviro;
	uint16 _totalFrames;
	Common::Array<int> _frameOffsets;
	Common::String _scriptShapes[6];
	int _scriptPals[6];
};

enum TTMRunType {
	kRunTypeStopped = 0,
	kRunType1 = 1,
	kRunTypeMulti = 2,
	kRunTypeTimeLimited = 3,
	kRunTypeFinished = 4,
	kRunType5 = 5,
};


struct TTMSeq {
	TTMSeq() : _enviro(0), _seqNum(0), _startFrame(0), _currentFrame(0),
	_lastFrame(0), _selfLoop(false), _executed(false), _timeNext(0),
	_timeCut(0), _currentBmpId(0), _brushNum(0), _currentSongId(-1),
	_currentPalId(0), _currentGetPutId(0), _timeInterval(0) {
		reset();
	}

	void reset();

	int16 _enviro;
	int16 _seqNum;
	int16 _startFrame;
	int16 _gotoFrame;
	int16 _currentFrame;
	int16 _lastFrame;
	bool _selfLoop;
	bool _executed;
	int16 _slot[6];
	uint32 _timeNext;
	uint32 _timeCut;
	Common::Rect _drawWin;
	int16 _currentBmpId;
	int16 _brushNum;
	int16 _currentSongId;
	int16 _currentPalId;
	int16 _currentGetPutId;
	byte _drawColFG;
	byte _drawColBG;
	int16 _runPlayed;
	int16 _runCount;
	int16 _timeInterval;
	TTMRunType _runFlag;
	int16 _scriptFlag;
};

class ADSData : public ScriptParserData {
public:
	ADSData() : _initFlag(0), _maxSegments(0), _scriptDelay(-1),
			_hitTTMOp0110(false), _hitBranchOp(false), _gotoTarget(-1) {
		for (int i = 0; i < ARRAYSIZE(_state); i++) {
			_state[i] = 8;
		}
		ARRAYCLEAR(_countdown);
		ARRAYCLEAR(_segments);
		ARRAYCLEAR(_charWhile);
	}
	Common::Array<Common::String> _scriptNames;
	Common::Array<TTMEnviro> _scriptEnvs;
	Common::Array<TTMSeq> _ttmSeqs;
	int _initFlag;
	int _maxSegments;
	// TODO: replace these with dynamic arrays - fixed arrays inherited from original.
	int _state[80];
	int _countdown[80];
	// note: originals uses char * but we use offsets into script for less pointers..
	int32 _segments[80];
	int32 _charWhile[80];
	Common::Array<struct TTMSeq *> _usedSeqs[80];
	int32 _scriptDelay;
	int32 _gotoTarget;
	bool _hitTTMOp0110;
	bool _hitBranchOp;
};

class TTMInterpreter {
public:
	TTMInterpreter(DgdsEngine *vm);

	bool load(const Common::String &filename, TTMEnviro &env);
	void unload();
	bool run(TTMEnviro &env, struct TTMSeq &seq);
	void findAndAddSequences(TTMEnviro &scriptData, Common::Array<TTMSeq> &seqArray);

protected:
	void handleOperation(TTMEnviro &env, struct TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval);
	void updateScreen(struct TTMSeq &seq);
	int32 findGOTOTarget(TTMEnviro &env, TTMSeq &seq);

	DgdsEngine *_vm;
};

class ADSInterpreter {
public:
	ADSInterpreter(DgdsEngine *vm);
	~ADSInterpreter();

	bool load(const Common::String &filename);
	void unload();
	bool run();
	int numArgs(uint16 opcode) const;
	void segmentOrState(int16 seg, uint16 val);
	void segmentSetState(int16 seg, uint16 val);
	
	void setHitTTMOp0110(); // TODO: What is this global?
	int16 getStateForSceneOp(uint16 segnum);

protected:
	bool handleOperation(uint16 code, Common::SeekableReadStream *scr);
	bool playScene();
	void skipToEndIf(Common::SeekableReadStream *scr);
	TTMSeq *findTTMSeq(int16 enviro, int16 seq);
	TTMEnviro *findTTMEnviro(int16 enviro);
	bool runUntilBranchOpOrEnd();
	void findUsedSequencesForSegment(int segno);
	void findEndOrInitOp();
	bool updateSeqTimeAndFrame(TTMSeq &seq);
	DgdsEngine *_vm;
	TTMInterpreter *_ttmInterpreter;

	ADSData _adsData;

	TTMSeq *_currentTTMSeq;
};

} // End of namespace Dgds

#endif // DGDS_SCRIPTS_H
