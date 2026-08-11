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
 */

#ifndef DGDS_ADS_H
#define DGDS_ADS_H

#include "dgds/scripts.h"
#include "dgds/ttm.h"

namespace Dgds {

class ADSData : public ScriptParserData {
public:
	ADSData() : _maxSegments(0), _scriptDelay(-1), _hitTTMOp0110(false), _hitBranchOp(false),
			_gotoTarget(-1), _runningSegmentIdx(0) {
		for (int i = 0; i < ARRAYSIZE(_state); i++)
			_state[i] = 8;

		for (int i = 0; i < ARRAYSIZE(_segments); i++)
			_segments[i] = -1;

		ARRAYCLEAR(_countdown);
		ARRAYCLEAR(_charWhile);
	}
	Common::Array<Common::String> _scriptNames;
	Common::Array<TTMEnviro> _scriptEnvs;
	Common::Array<Common::SharedPtr<TTMSeq>> _ttmSeqs;	// Pointers as we need to shuffle them but keep _usedSeqs below valid
	int _maxSegments;
	// TODO: replace these with dynamic arrays - fixed arrays inherited from original.
	int _state[80];
	int _countdown[80];
	// note: originals uses char * but we use offsets into script for less pointers. -1 is nullptr
	int32 _segments[80];
	int32 _charWhile[80];
	Common::Array<Common::SharedPtr<TTMSeq>> _usedSeqs[80];
	int32 _scriptDelay;
	int32 _gotoTarget;
	bool _hitTTMOp0110;
	bool _hitBranchOp;
	int16 _runningSegmentIdx;

	Common::Error syncState(Common::Serializer &s);
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

	void setHitTTMOp0110(bool val = true); // TODO: better name for this global?
	bool getHitTTMOp0110() const { return _adsData->_hitTTMOp0110; }
	void setGotoTarget(int32 target);
	int16 getStateForSceneOp(uint16 segnum);
	void setScriptDelay(int16 delay) { _adsData->_scriptDelay = delay; }

	Common::Error syncState(Common::Serializer &s);

protected:
	bool handleOperation(uint16 code, Common::SeekableReadStream *scr);
	void handleRandomOp(Common::SeekableReadStream *scr);
	bool handleLogicOp(uint16 code,  Common::SeekableReadStream *scr);
	bool logicOpResult(uint16 code, const TTMEnviro *env, const TTMSeq *seq, uint16 arg);
	int16 randomOpGetProportion(uint16 code, Common::SeekableReadStream *scr);
	bool playScene();
	bool skipToElseOrEndif();
	bool skipToEndIf();
	bool skipToEndWhile();
	bool skipSceneLogicBranch();
	Common::SharedPtr<TTMSeq> findTTMSeq(int16 enviro, int16 seq);
	TTMEnviro *findTTMEnviro(int16 enviro);
	bool runUntilBranchOpOrEnd();
	void findUsedSequencesForSegment(int segno);
	void findEndOrInitOp();
	bool updateSeqTimeAndFrame(const TTMEnviro *env, Common::SharedPtr<TTMSeq> seq);
	int getArrIndexOfSegNum(uint16 segnum);

	DgdsEngine *_vm;
	TTMInterpreter *_ttmInterpreter;

	Common::HashMap<Common::String, ADSData> _adsTexts;
	ADSData *_adsData;

	Common::SharedPtr<TTMSeq> _currentTTMSeq;
};

} // end namespace Dgds

#endif // DGDS_ADS_H
