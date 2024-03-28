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
#include "graphics/managed_surface.h"

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
	Common::Array<Common::Rect> _getPutAreas;
	Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> _getPutSurfaces;
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


// Note: this object needs to be safely copy-able - ADS opcodes 0x4000 and 0x4010 require it.
struct TTMSeq {
	TTMSeq() : _enviro(0), _seqNum(0), _startFrame(0), _lastFrame(0), _timeCut(0) {
		// Other members are initialized in the reset function.
		reset();
	}

	void reset();

	int16 _enviro;
	int16 _seqNum;
	int16 _startFrame;	// first frame in this sequence
	int16 _gotoFrame;   // frame to GOTO (or -1 if not currently set)
	int16 _currentFrame; // currently executing frame
	int16 _lastFrame;	// previous frame processed (-1 if none)
	bool _selfLoop;		// does the script frame loop back on itself
	bool _executed;		// has the current frame already been run
	uint32 _timeNext;	// time the next frame should be run
	uint32 _timeCut;	// time to finish execution
	Common::Rect _drawWin;
	// these current ids are called "slot"s in the original
	int16 _currentFontId; 	// aka slot 0
	int16 _currentPalId;	// aka slot 1
	int16 _currentSongId;	// aka slot 3
	int16 _currentBmpId;	// aka slot 4
	int16 _currentGetPutId;	// aka slot 5
	int16 _brushNum;
	byte _drawColFG;
	byte _drawColBG;
	int16 _runPlayed;		// number of times the sequence has been started from ADS
	int16 _runCount;		// number of times to play the sequence before stopping
	int16 _timeInterval;	// interval between frames
	TTMRunType _runFlag;
	int16 _scriptFlag;
};

class ADSData : public ScriptParserData {
public:
	ADSData() : _initFlag(false), _maxSegments(0), _scriptDelay(-1),
			_hitTTMOp0110(false), _hitBranchOp(false), _gotoTarget(-1),
			_runningSegmentIdx(0) {
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
	bool _initFlag;
	int _maxSegments;
	// TODO: replace these with dynamic arrays - fixed arrays inherited from original.
	int _state[80];
	int _countdown[80];
	// note: originals uses char * but we use offsets into script for less pointers. -1 is nullptr
	int32 _segments[80];
	int32 _charWhile[80];
	Common::Array<struct TTMSeq *> _usedSeqs[80];
	int32 _scriptDelay;
	int32 _gotoTarget;
	bool _hitTTMOp0110;
	bool _hitBranchOp;
	int16 _runningSegmentIdx;
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

	void setHitTTMOp0110(); // TODO: better name for this global?
	void setGotoTarget(int32 target);
	int16 getStateForSceneOp(uint16 segnum);
	void setScriptDelay(int16 delay) { _adsData._scriptDelay = delay; }

protected:
	bool handleOperation(uint16 code, Common::SeekableReadStream *scr);
	void handleRandomOp(uint16 code, Common::SeekableReadStream *scr);
	int16 randomOpGetProportion(uint16 code, Common::SeekableReadStream *scr);
	bool playScene();
	bool skipToEndIf();
	bool skipSceneLogicBranch();
	TTMSeq *findTTMSeq(int16 enviro, int16 seq);
	TTMEnviro *findTTMEnviro(int16 enviro);
	bool runUntilBranchOpOrEnd();
	void findUsedSequencesForSegment(int segno);
	void findEndOrInitOp();
	bool updateSeqTimeAndFrame(TTMSeq &seq);
	int getArrIndexOfSegNum(uint16 segnum);
	DgdsEngine *_vm;
	TTMInterpreter *_ttmInterpreter;

	ADSData _adsData;

	TTMSeq *_currentTTMSeq;
};

} // End of namespace Dgds

#endif // DGDS_SCRIPTS_H
