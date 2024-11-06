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

#include "dgds/ads.h"

namespace Dgds {


Common::Error ADSData::syncState(Common::Serializer &s) {
	uint16 arrSize = ARRAYSIZE(_state);
	s.syncAsUint16LE(arrSize);
	if (arrSize != ARRAYSIZE(_state))
		error("Expected fixed size state array");
	for (uint i = 0; i < arrSize; i++)
		s.syncAsSint16LE(_state[i]);

	uint16 nenvs = _scriptEnvs.size();
	s.syncAsUint16LE(nenvs);
	// This should be the same on load as the data comes from the ADS/TTM files.
	if (nenvs != _scriptEnvs.size())
		error("Unexpected number of script envs (%d in save vs %d in ADS)", nenvs, _scriptEnvs.size());

	for (auto &env : _scriptEnvs)
		env.syncState(s);

	uint16 nseqs = _ttmSeqs.size();
	s.syncAsUint16LE(nseqs);
	if (nseqs != _ttmSeqs.size())
		error("Unexpected number of ttm seqeunces (%d in save vs %d in ADS)", nseqs, _ttmSeqs.size());

	for (auto &seq : _ttmSeqs)
		seq->syncState(s);

	return Common::kNoError;
}


ADSInterpreter::ADSInterpreter(DgdsEngine *vm) : _vm(vm), _currentTTMSeq(nullptr), _adsData(nullptr) {
	_ttmInterpreter = new TTMInterpreter(_vm);
}

ADSInterpreter::~ADSInterpreter() {
	delete _ttmInterpreter;
	_ttmInterpreter = nullptr;
	for (auto &data : _adsTexts)
		delete data._value.scr;
}

bool ADSInterpreter::load(const Common::String &filename) {
	// Don't clear current _adsData, we reset that below.
	_currentTTMSeq = nullptr;

	// For high detail, replace extension ADS with ADH.  Low detail is ADL.
	Common::String detailfile = filename.substr(0, filename.size() - 1);
	if (_vm->getDetailLevel() == kDgdsDetailLow)
		detailfile += "L";
	else
		detailfile += "H";

	if (!_vm->getResourceManager()->hasResource(detailfile))
		detailfile = filename;

	debug("ADSInterpreter: load %s", detailfile.c_str());

	// Reset the state
	_adsTexts.setVal(detailfile, ADSData());
	_adsData = &(_adsTexts.getVal(detailfile));

	ADSParser dgds(_vm->getResourceManager(), _vm->getDecompressor());
	dgds.parse(_adsData, detailfile);

	for (const auto &file : _adsData->_scriptNames) {
		if (file.empty())
			continue;
		_adsData->_scriptEnvs.resize(_adsData->_scriptEnvs.size() + 1);
		debug("   load TTM %s to env %d", file.c_str(), _adsData->_scriptEnvs.size());
		TTMEnviro &data = _adsData->_scriptEnvs.back();
		data._enviro = _adsData->_scriptEnvs.size();
		_ttmInterpreter->load(file, data);
		_ttmInterpreter->findAndAddSequences(data, _adsData->_ttmSeqs);
	}

	_adsData->scr->seek(0);

	uint16 opcode = 0;
	int segcount = 0;
	findUsedSequencesForSegment(0);
	_adsData->_segments[0] = 0;
	opcode = _adsData->scr->readUint16LE();
	while (_adsData->scr->pos() < _adsData->scr->size()) {
		if (opcode == 0xffff) {
			segcount++;
			_adsData->_segments[segcount] = _adsData->scr->pos();
			findUsedSequencesForSegment(segcount);
		} else {
			_adsData->scr->skip(numArgs(opcode) * 2);
		}
		opcode = _adsData->scr->readUint16LE();
	}

	for (uint i = segcount + 1; i < ARRAYSIZE(_adsData->_segments); i++)
		_adsData->_segments[i] = -1;

	_adsData->_maxSegments = segcount + 1;
	_adsData->_filename = filename;

	for (uint i = 0; i < ARRAYSIZE(_adsData->_state); i++)
		_adsData->_state[i] = 8;
	for (auto &seq : _adsData->_ttmSeqs)
		seq->reset();

	return true;
}

static const uint16 ADS_SEQ_OPCODES[] = {
	0x2000, 0x2005, 0x2010, 0x2015, 0x4000, 0x4010, 0x1330,
	0x1340, 0x1360, 0x1370, 0x1320, 0x1310, 0x1350
};

bool ADSInterpreter::updateSeqTimeAndFrame(const TTMEnviro *env, Common::SharedPtr<TTMSeq> seq) {
	if (seq->_timeInterval != 0) {
		uint32 now = DgdsEngine::getInstance()->getThisFrameMs();
		if (now < seq->_timeNext) {
			debug(10, "env %d seq %d (%s) not advancing from frame %d (now %d timeNext %d interval %d)", seq->_enviro,
					seq->_seqNum, env->_tags.getValOrDefault(seq->_seqNum).c_str(), seq->_currentFrame, now, seq->_timeNext, seq->_timeInterval);
			return false;
		}
		seq->_timeNext = now + seq->_timeInterval;
	}

	seq->_executed = false;
	if (seq->_gotoFrame == -1) {
		debug(10, "env %d seq %d (%s) advance to frame %d->%d (start %d last %d)", seq->_enviro, seq->_seqNum,
				env->_tags.getValOrDefault(seq->_seqNum).c_str(), seq->_currentFrame, seq->_currentFrame + 1, seq->_startFrame, seq->_lastFrame);
		seq->_currentFrame++;
	} else {
		debug(10, "env %d seq %d (%s) goto to frame %d->%d (start %d last %d)", seq->_enviro, seq->_seqNum,
				env->_tags.getValOrDefault(seq->_seqNum).c_str(), seq->_currentFrame, seq->_gotoFrame, seq->_startFrame, seq->_lastFrame);
		seq->_currentFrame = seq->_gotoFrame;
		seq->_gotoFrame = -1;
	}

	return true;
}

void ADSInterpreter::findUsedSequencesForSegment(int idx) {
	_adsData->_usedSeqs[idx].clear();
	int64 startoff = _adsData->scr->pos();
	uint16 opcode = 0;
	// Skip the segment number.
	int16 segno = _adsData->scr->readUint16LE();
	while (opcode != 0xffff && _adsData->scr->pos() < _adsData->scr->size()) {
		opcode = _adsData->scr->readUint16LE();
		for (uint16 o : ADS_SEQ_OPCODES) {
			if (opcode == o) {
				int16 envno = _adsData->scr->readSint16LE();
				int16 seqno = _adsData->scr->readSint16LE();
				Common::SharedPtr<TTMSeq> seq = findTTMSeq(envno, seqno);
				if (!seq) {
					warning("ADS opcode %04x at offset %d references unknown seq %d %d",
							opcode, (int)_adsData->scr->pos(), envno, seqno);
				} else {
					bool already_added = false;
					for (const Common::SharedPtr<TTMSeq> &s : _adsData->_usedSeqs[idx]) {
						if (s == seq) {
							already_added = true;
							break;
						}
					}
					if (!already_added) {
						debug(10, "ADS seg no %d (idx %d) uses seq %d %d", segno, idx, envno, seqno);
						_adsData->_usedSeqs[idx].push_back(seq);
					}
				}
				// Rewind as we will go forward again outside this loop.
				_adsData->scr->seek(-4, SEEK_CUR);
				break;
			}
		}
		_adsData->scr->skip(numArgs(opcode) * 2);
	}
	_adsData->scr->seek(startoff);
}


void ADSInterpreter::unload() {
	_adsData = nullptr;
	_currentTTMSeq = nullptr;
	_adsTexts.clear();
}

bool ADSInterpreter::playScene() {
	if (!_currentTTMSeq)
		return false;

	TTMEnviro *env = findTTMEnviro(_currentTTMSeq->_enviro);
	if (!env)
		error("Couldn't find environment num %d", _currentTTMSeq->_enviro);

	_adsData->_gotoTarget = -1;
	return _ttmInterpreter->run(*env, *_currentTTMSeq);
}

bool ADSInterpreter::skipSceneLogicBranch() {
	Common::SeekableReadStream *scr = _adsData->scr;
	bool result = true;
	while (scr->pos() < scr->size()) {
		uint16 op = scr->readUint16LE();
		if (op == 0x1510 || op == 0x1500) { // else or endif
			scr->seek(-2, SEEK_CUR);
			return true;
		} else if (op == 0 || op == 0xffff) {
			// end of segment
			return false;
		} else if ((op & 0xff0f) == 0x1300) {
			// A nested IF (0x13x0) block. Skip to endif ignoring else.
			result = skipToEndIf();
		} else {
			scr->skip(numArgs(op) * 2);
		}
	}
	return result && scr->pos() < scr->size();
}

bool ADSInterpreter::skipToElseOrEndif() {
	Common::SeekableReadStream *scr = _adsData->scr;
	bool result = skipSceneLogicBranch();
	if (result) {
		uint16 op = scr->readUint16LE();
		if (op == 0x1500)
			// Hit else block - we want to run that
			result = runUntilBranchOpOrEnd();
		// don't rewind - the calls to this should always return ptr+2
	}
	return result;
}

bool ADSInterpreter::skipToEndIf() {
	Common::SeekableReadStream *scr = _adsData->scr;
	while (scr->pos() < scr->size()) {
		uint16 op = scr->readUint16LE();
		// don't rewind - the calls to this should always return after the last op.
		if (op == 0x1510) // ENDIF
			return true;
		else if (op == 0 || op == 0xffff)
			return false;

		scr->skip(numArgs(op) * 2);
	}
	return false;
}

bool ADSInterpreter::skipToEndWhile() {
	Common::SeekableReadStream *scr = _adsData->scr;
	while (scr->pos() < scr->size()) {
		uint16 op = scr->readUint16LE();
		// don't rewind - the calls to this should always return after the last op.
		if (op == 0x1520)
			return true;
		else if (op == 0 || op == 0xffff)
			return false;

		scr->skip(numArgs(op) * 2);
	}
	return false;
}

TTMEnviro *ADSInterpreter::findTTMEnviro(int16 enviro) {
	for (auto & env : _adsData->_scriptEnvs) {
		if (env._enviro == enviro)
			return &env;
	}
	return nullptr;
}

Common::SharedPtr<TTMSeq> ADSInterpreter::findTTMSeq(int16 enviro, int16 seqno) {
	for (auto &seq : _adsData->_ttmSeqs) {
		if (seq->_enviro == enviro && seq->_seqNum == seqno)
			return seq;
	}
	return nullptr;
}

void ADSInterpreter::segmentOrState(int16 seg, uint16 val) {
	int idx = getArrIndexOfSegNum(seg);
	if (idx >= 0) {
		_adsData->_charWhile[idx] = 0;
		_adsData->_state[idx] = (_adsData->_state[idx] & 8) | val;
	}
}


void ADSInterpreter::segmentSetState(int16 seg, uint16 val) {
	int idx = getArrIndexOfSegNum(seg);
	if (idx >= 0) {
		_adsData->_charWhile[idx] = 0;
		if (_adsData->_state[idx] != 8)
			_adsData->_state[idx] = val;
	}
}

void ADSInterpreter::findEndOrInitOp() {
	Common::SeekableReadStream *scr = _adsData->scr;
	int32 startoff = scr->pos();
	while (scr->pos() < scr->size()) {
		uint16 opcode = scr->readUint16LE();
		// on FFFF return the original offset
		if (opcode == 0xffff) {
			scr->seek(startoff);
			return;
		}
		// on 5 (init) return the next offset (don't rewind)
		if (opcode == 0x0005)
			return;
		// everything else just go forward.
		scr->skip(numArgs(opcode) * 2);
	}
}

bool ADSInterpreter::logicOpResult(uint16 code, const TTMEnviro *env, const TTMSeq *seq, uint16 arg) {
	const char *tag = (seq && env) ? env->_tags.getValOrDefault(seq->_seqNum).c_str() : "";
	int16 envNum = env ? env->_enviro : 0;
	int16 seqNum = seq ? seq->_seqNum : 0;
	const char *optype = (code < 0x1300 ? "while" : "if");

	assert(seq || code == 0x1380 || code == 0x1390);

	switch (code) {
	case 0x1010: // WHILE paused
	case 0x1310: // IF paused, 2 params
		debugN(10, "ADS 0x%04x: %s paused env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return seq->_runFlag == kRunTypePaused;
	case 0x1020: // WHILE not paused
	case 0x1320: // IF not paused, 2 params
		debugN(10, "ADS 0x%04x: %s not paused env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return seq->_runFlag != kRunTypePaused;
	case 0x1030: // WHILE NOT PLAYED
	case 0x1330: // IF_NOT_PLAYED, 2 params
		debugN(10, "ADS 0x%04x: %s not played env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return !seq->_runPlayed;
	case 0x1040: // WHILE PLAYED
	case 0x1340: // IF_PLAYED, 2 params
		debugN(10, "ADS 0x%04x: %s played env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return seq->_runPlayed;
	case 0x1050: // WHILE FINISHED
	case 0x1350: // IF_FINISHED, 2 params
		debugN(10, "ADS 0x%04x: %s finished env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return seq->_runFlag == kRunTypeFinished;
	case 0x1060: // WHILE NOT RUNNING
	case 0x1360: { // IF_NOT_RUNNING, 2 params
		debugN(10, "ADS 0x%04x: %s not running env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		// Dragon only checks kRunTypeStopped, HoC onward also check for kRunTypeFinished
		bool isDragon = _vm->getGameId() == GID_DRAGON;
		return seq->_runFlag == kRunTypeStopped || (!isDragon && seq->_runFlag == kRunTypeFinished);
	}
	case 0x1070: // WHILE RUNNING
	case 0x1370: // IF_RUNNING, 2 params
		debugN(10, "ADS 0x%04x: %s running env %d seq %d (%s)", code, optype, envNum, seqNum, tag);
		return seq->_runFlag == kRunType1 || seq->_runFlag == kRunTypeMulti || seq->_runFlag == kRunTypeTimeLimited;
	case 0x1080:
	case 0x1090:
		warning("Unimplemented IF/WHILE operation 0x%x", code);
		return true;
	case 0x1380: // IF_DETAIL_LTE, 1 param
		debugN(10, "ADS 0x%04x: if detail <= %d", code, arg);
		// FIXME: This should be right but we only have detail 0/1 and maybe HOC onward use
		// different numbers?  HOC intro checks for >= 4.
		return false;
		//return ((int)DgdsEngine::getInstance()->getDetailLevel() <= arg);
	case 0x1390: // IF_DETAIL_GTE, 1 param
		debugN(10, "ADS 0x%04x: if detail >= %d", code, arg);
		return true;
		//return ((int)DgdsEngine::getInstance()->getDetailLevel() >= arg);
	default:
		error("Not an ADS logic op: %04x, how did we get here?", code);
	}
}

bool ADSInterpreter::handleLogicOp(uint16 code, Common::SeekableReadStream *scr) {
	bool testval = true;
	uint16 andor = 0x1420; // start with "true" AND..
	int32 startPos = scr->pos() - 2;
	while (scr->pos() < scr->size()) {
		uint16 enviro;
		uint16 seqnum;
		Common::SharedPtr<TTMSeq> seq;
		TTMEnviro *env = nullptr;

		if (code != 0x1380 && code != 0x1390) {
			enviro = scr->readUint16LE();
			seqnum = scr->readUint16LE();
			seq = findTTMSeq(enviro, seqnum);
			env = findTTMEnviro(enviro);
			if (!seq) {
				warning("ADS if op referenced non-existent env %d seq %d", enviro, seqnum);
				return false;
			}
		} else {
			// TODO: this value is not actually enviro? for now just read it.
			enviro = scr->readUint16LE();
		}

		bool logicResult = logicOpResult(code, env, seq.get(), enviro);

		if (andor == 0x1420) // AND
			testval &= logicResult;
		else // OR
			testval |= logicResult;

		debug(10, "  -> %s (overall %s)", logicResult ? "true" : "false", testval ? "true" : "false");
		bool isWhile = code < 0x1300;

		code = scr->readUint16LE();

		if (code == 0x1420 || code == 0x1430) {
			andor = code;
			debug(10, "  ADS 0x%04x: %s", code, code == 0x1420 ? "AND" : "OR");
			code = scr->readUint16LE();
			// The next op should be another logic op
		} else {
			// No AND or OR, next op is just what to do.
			scr->seek(-2, SEEK_CUR);
			if (testval) {
				if (isWhile) {
					_adsData->_countdown[_adsData->_runningSegmentIdx]++;
					_adsData->_charWhile[_adsData->_runningSegmentIdx] = startPos;
				}
				bool runResult = runUntilBranchOpOrEnd();
				// WHILE (10x0) series always return false
				return (!isWhile) && runResult;
			} else {
				if (isWhile) {
					_adsData->_countdown[_adsData->_runningSegmentIdx] = 0;
					_adsData->_charWhile[_adsData->_runningSegmentIdx] = 0;
					return skipToEndWhile();
				} else {
					return skipToElseOrEndif();
				}
			}
		}
	}
	error("didn't return from ADS logic test");
}

int16 ADSInterpreter::randomOpGetProportion(uint16 code, Common::SeekableReadStream *scr) {
	// Leaves the pointer at the same place it started
	int argsize = numArgs(code) * 2;
	if (argsize == 0)
		error("Unexpected 0-arg ADS opcode 0x%04x inside random block", code);
	// skip args before the random proportion
	if (argsize > 2)
		scr->seek(argsize - 2, SEEK_CUR);
	int16 result = scr->readSint16LE();
	scr->seek(-argsize, SEEK_CUR);
	return result;
}

void ADSInterpreter::handleRandomOp(Common::SeekableReadStream *scr) {
	int16 max = 0;
	int64 startpos = scr->pos();

	// Collect the random proportions
	uint16 code = scr->readUint16LE();
	while (code != 0 && code != 0x30FF && scr->pos() < scr->size()) {
		int16 val = randomOpGetProportion(code, scr);
		// leaves pointer at beginning of next op
		max += val;
		scr->skip(numArgs(code) * 2);
		if (scr->pos() >= scr->size())
			break;
		code = scr->readUint16LE();
	}
	if (!max)
		return;

	int64 endpos = scr->pos();

	int16 randval = _vm->getRandom().getRandomNumber(max - 1) + 1; // Random from 1-max.
	scr->seek(startpos, SEEK_SET);

	// Now find the random bit to jump to
	code = scr->readUint16LE();
	do {
		int16 val = randomOpGetProportion(code, scr);
		randval -= val;
		if (randval < 1) {
			// This is the opcode we want to execute
			break;
		}
		scr->skip(numArgs(code) * 2);
		if (scr->pos() >= scr->size())
			break;
		code = scr->readUint16LE();
	} while (code != 0 && scr->pos() < scr->size());
	if (code && code != 0x3020)
		handleOperation(code, scr);

	scr->seek(endpos, SEEK_SET);
}

bool ADSInterpreter::handleOperation(uint16 code, Common::SeekableReadStream *scr) {
	uint16 enviro, seqnum;

	switch (code) {
	case 0x0001:
	case 0x0005:
		debug(10, "ADS 0x%04x: init", code);
		// "init".  0x0005 can be used for searching for next thing.
		break;
	case 0x1010: // WHILE runtype, 2 params
	case 0x1020: // WHILE not runtype, 2 params
	case 0x1030: // WHILE not played, 2 params
	case 0x1040: // WHILE played, 2 params
	case 0x1050: // WHILE finished, 2 params
	case 0x1060: // WHILE not running, 2 params
	case 0x1070: // WHILE running, 2 params
	case 0x1080: // WHILE countdown <= , 1 param (HOC+ only)
	case 0x1090: // WHILE ??, 1 param (HOC+ only)
	case 0x1310: // IF paused, 2 params
	case 0x1320: // IF not paused, 2 params
	case 0x1330: // IF NOT_PLAYED, 2 params
	case 0x1340: // IF PLAYED, 2 params
	case 0x1350: // IF FINISHED, 2 params
	case 0x1360: // IF NOT_RUNNING, 2 params
	case 0x1370: // IF RUNNING, 2 params
	case 0x1380: // IF DETAIL LEVEL <= x, 1 param (HOC+ only)
	case 0x1390: // IF DETAIL LEVEL >= x, 1 param (HOC+ only)
		return handleLogicOp(code, scr);
	case 0x1500: // ELSE / Skip to end-if, 0 params
		debug(10, "ADS 0x%04x: else (skip to end if)", code);
		skipToElseOrEndif();
		_adsData->_hitBranchOp = true;
		return true;
	case 0x1510: // END IF 0 params
		debug(10, "ADS 0x%04x: hit branch op endif", code);
		_adsData->_hitBranchOp = true;
		return true;
	case 0x1520: // END WHILE 0 params
		debug(10, "ADS 0x%04x: hit branch op endwhile", code);
		_adsData->_hitBranchOp = true;
		return false;

	case 0x2000:
	case 0x2005: { // ADD sequence
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		int16 runCount = scr->readSint16LE();
		uint16 unk = scr->readUint16LE(); // proportion

		Common::SharedPtr<TTMSeq> seq = findTTMSeq(enviro, seqnum);
		TTMEnviro *env = findTTMEnviro(enviro);
		if (!seq || !env) {
			// This happens in Willy Beamish FDD scene 24
			warning("ADS op %04x invalid env + seq requested %d %d", code, enviro, seqnum);
			break;
		}

		debug(10, "ADS 0x%04x: add scene - env %d seq %d (%s) runCount %d prop %d", code,
					enviro, seqnum, env->_tags.getValOrDefault(seqnum).c_str(), runCount, unk);

		if (code == 0x2000)
			seq->_currentFrame = seq->_startFrame;

		_currentTTMSeq = seq;
		if (runCount == 0) {
			seq->_runFlag = kRunType1;
		} else if (runCount < 0) {
			// Negative run count sets the cut time
			seq->_timeCut = DgdsEngine::getInstance()->getThisFrameMs() + (-runCount * MS_PER_FRAME);
			seq->_runFlag = kRunTypeTimeLimited;
		} else {
			seq->_runFlag = kRunTypeMulti;
			seq->_runCount = runCount - 1;
		}
		seq->_runPlayed++;
		break;
	}
	case 0x2010: { // STOP SCENE, 3 params (ttmenv, ttmseq, proportion)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		_currentTTMSeq = findTTMSeq(enviro, seqnum);
		const TTMEnviro *env = findTTMEnviro(enviro);
		debug(10, "ADS 0x2010: stop seq env %d seq %d (%s) prop %d", enviro, seqnum,
				env->_tags.getValOrDefault(seqnum).c_str(), unk);
		if (_currentTTMSeq)
			_currentTTMSeq->_runFlag = kRunTypeStopped;
		break;
	}
	case 0x2015: { // PAUSE SEQ, 3 params (ttmenv, ttmseq, proportion)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		_currentTTMSeq = findTTMSeq(enviro, seqnum);
		const TTMEnviro *env = findTTMEnviro(enviro);
		debug(10, "ADS 0x2015: set paused env %d seq %d (%s) prop %d", enviro, seqnum,
				env->_tags.getValOrDefault(seqnum).c_str(), unk);
		if (_currentTTMSeq)
			_currentTTMSeq->_runFlag = kRunTypePaused;
		break;
	}
	case 0x2020: { // RESET SEQ, 2 params (env, seq, proportion)
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		uint16 unk = scr->readUint16LE();
		_currentTTMSeq = findTTMSeq(enviro, seqnum);
		const TTMEnviro *env = findTTMEnviro(enviro);
		debug(10, "ADS 0x2020: reset scene env %d seq %d (%s) prop %d", enviro, seqnum,
				env->_tags.getValOrDefault(seqnum).c_str(), unk);
		if (_currentTTMSeq)
			_currentTTMSeq->reset();
		break;
	}

	case 0x3020: {// RANDOM_NOOP, 1 param (proportion)
		uint16 unk = scr->readUint16LE();
		debug(10, "ADS 0x3020: random noop? prop %d", unk);
		return true;
	}
	case 0x3010: // RANDOM_START, 0 params
		debug(10, "ADS 0x3010: random start");
		handleRandomOp(scr);
		break;

	case 0x30FF: // RANDOM_END, 0 params
		debug(10, "ADS 0x30FF: random end");
		error("Unexpected RANDOM END mid-stream (no RANDOM START?).");

	case 0x4000: { // MOVE SEQ TO BACK
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		debug(10, "ADS 0x%04x: mov seq to back env %d seq %d", code, enviro, seqnum);
		/*uint16 unk = */scr->readUint16LE();
		// This is O(N) but the N is small and it's not called often.
		Common::SharedPtr<TTMSeq> seq;
		for (uint i = 0; i < _adsData->_ttmSeqs.size(); i++) {
			if (_adsData->_ttmSeqs[i]->_enviro == enviro && _adsData->_ttmSeqs[i]->_seqNum == seqnum) {
				seq = _adsData->_ttmSeqs[i];
				_adsData->_ttmSeqs.remove_at(i);
				break;
			}
		}

		if (seq)
			_adsData->_ttmSeqs.push_back(seq);
		else
			warning("ADS: 0x4000 Request to move env %d seq %d which doesn't exist", enviro, seqnum);

		break;
	}

	case 0x4010: { // MOVE SEQ TO FRONT
		enviro = scr->readUint16LE();
		seqnum = scr->readUint16LE();
		debug(10, "ADS 0x%04x: mov seq to front env %d seq %d", code, enviro, seqnum);
		/*uint16 unk = */scr->readUint16LE();
		// This is O(N) but the N is small and it's not called often.
		Common::SharedPtr<TTMSeq> seq;
		for (uint i = 0; i < _adsData->_ttmSeqs.size(); i++) {
			if (_adsData->_ttmSeqs[i]->_enviro == enviro && _adsData->_ttmSeqs[i]->_seqNum == seqnum) {
				seq = _adsData->_ttmSeqs[i];
				_adsData->_ttmSeqs.remove_at(i);
				break;
			}
		}

		if (seq)
			_adsData->_ttmSeqs.insert_at(0, seq);
		else
			warning("ADS: 0x4010 Request to move env %d seq %d which doesn't exist", enviro, seqnum);

		break;
	}

	case 0x1420: // AND, 0 params - should not hit this here.
	case 0x1430: // OR, 0 params - should not hit this here.
		warning("ADS: Unexpected logic opcode 0x%04x - should be after IF/WHILE", code);
		break;

	case 0xF000:
		debug(10, "ADS 0x%04x: set state 2, current idx (%d)", code, _adsData->_runningSegmentIdx);
		if (_adsData->_runningSegmentIdx != -1)
			_adsData->_state[_adsData->_runningSegmentIdx] = 2;
		return false;

	case 0xF010: { // FADE_OUT, 1 param
		int16 segment = scr->readSint16LE();
		int16 idx = _adsData->_runningSegmentIdx;
		if (segment >= 0)
			idx = getArrIndexOfSegNum(segment);
		debug(10, "ADS 0x%04x: set state 2, segment %d (idx %d)", code, segment, idx);
		if (idx >= 0)
			_adsData->_state[idx] = 2;
		if (idx == _adsData->_runningSegmentIdx)
			return false;
		else
			return true;
	}

	case 0xF200: { // RUN_SCRIPT, 1 param
		int16 segment = scr->readSint16LE();
		int16 idx = getArrIndexOfSegNum(segment);
		debug(10, "ADS 0x%04x: run seg %d idx %d", code, segment, idx);
		if (segment >= 0 && idx >= 0) {
			int state = (_adsData->_state[idx] & 8) | 4;
			_adsData->_state[idx] = state;
		}
		return true;
	}

	case 0xF210: { // RESTART_SCRIPT, 1 param
		int16 segment = scr->readSint16LE();
		int16 idx = getArrIndexOfSegNum(segment);
		debug(10, "ADS 0x%04x: restart seg %d idx %d", code, segment, idx);
		if (segment >= 0 && idx >= 0) {
			int state = (_adsData->_state[idx] & 8) | 3;
			_adsData->_state[idx] = state;
		}
		return true;
	}

	case 0xffff:	// END
		debug(10, "ADS 0xFFFF: end");
		return false;

	//// unknown / to-be-implemented
	// The next 6 are in HoC code but maybe never used?
	case 0x13A0: // IF some_ads_variable[0] <=
	case 0x13A1: // IF some_ads_variable[1] <=
	case 0x13B0: // IF some_ads_variable[0] >
	case 0x13B1: // IF some_ads_variable[1] >
	case 0x13C0: // IF some_ads_variable[0] ==
	case 0x13C1: // IF some_ads_variable[1] ==

	case 0xFF10:
	case 0xFFF0: // END_IF, 0 params
	default: {
		int nops = numArgs(code);
		warning("ADS 0x%04x: Unimplemented opcode: (skip %d args)", code, nops);
		for (int i = 0; i < nops; i++)
			scr->readUint16LE();
		break;
	}
	}

	return true;
}

int16 ADSInterpreter::getStateForSceneOp(uint16 segnum) {
	int idx = getArrIndexOfSegNum(segnum);
	if (idx < 0)
		return 0;
	if (!(_adsData->_state[idx] & 4)) {
		for (const Common::SharedPtr<TTMSeq> &seq: _adsData->_usedSeqs[idx]) {
			if (!seq)
				return 0;
			if (seq->_runFlag != kRunTypeStopped && !seq->_selfLoop)
				return 1;
		}
		return 0;
	}
	return 1;
}


int ADSInterpreter::getArrIndexOfSegNum(uint16 segnum) {
	int32 startoff = _adsData->scr->pos();
	int result = -1;
	for (int i = 0; i < _adsData->_maxSegments; i++) {
		_adsData->scr->seek(_adsData->_segments[i]);
		int16 seg = _adsData->scr->readSint16LE();
		if (seg == segnum) {
			result = i;
			break;
		}
	}
	_adsData->scr->seek(startoff);
	return result;
}


bool ADSInterpreter::run() {
	if (!_adsData || _adsData->_ttmSeqs.empty())
		return false;

	for (int idx = 0; idx < _adsData->_maxSegments; idx++) {
		int16 flag = _adsData->_state[idx] & 0xfff7;
		for (auto seq : _adsData->_usedSeqs[idx]) {
			if (flag == 3) {
				debug(10, "ADS: Segment idx %d, Reset seq %d", idx, seq->_seqNum);
				seq->reset();
			} else {
				if (flag != seq->_scriptFlag) {
					//debug(10, "ADS: Segment idx %d, update seq %d scriptflag %d -> %d",
					//	idx, seq->_seqNum, seq->_scriptFlag, flag);
					seq->_scriptFlag = flag;
				}
			}
		}
	}

	assert(_adsData->scr || !_adsData->_maxSegments);
	for (int idx = 0; idx < _adsData->_maxSegments; idx++) {
		int16 state = _adsData->_state[idx];
		int32 offset = _adsData->_segments[idx];
		_adsData->scr->seek(offset);
		// skip over the segment num
		offset += 2;
		int16 segnum = _adsData->scr->readSint16LE();
		if (state & 8) {
			state &= 0xfff7;
			_adsData->_state[idx] = state;
		} else {
			findEndOrInitOp();
			offset = _adsData->scr->pos();
		}

		if (_adsData->_charWhile[idx])
			offset = _adsData->_charWhile[idx];

		if (state == 3 || state == 4) {
			_adsData->_state[idx] = 1;
			state = 1;
		}

		_adsData->_runningSegmentIdx = idx;
		if (state == 1) {
			_adsData->scr->seek(offset);
			debug(10, "ADS: Run segment %d idx %d/%d", segnum, idx, _adsData->_maxSegments);
			runUntilBranchOpOrEnd();
		}
	}

	bool result = false;
	for (Common::SharedPtr<TTMSeq> seq : _adsData->_ttmSeqs) {
		_currentTTMSeq = seq;
		seq->_lastFrame = -1;
		int sflag = seq->_scriptFlag;
		TTMRunType rflag = seq->_runFlag;
		if (sflag == 6 || (rflag != kRunType1 && rflag != kRunTypeTimeLimited && rflag != kRunTypeMulti && rflag != kRunTypePaused)) {
			if (sflag != 6 && sflag != 5 && rflag == kRunTypeFinished) {
				seq->_runFlag = kRunTypeStopped;
			}
		} else {
			int16 curframe = seq->_currentFrame;
			TTMEnviro *env = findTTMEnviro(seq->_enviro);
			_adsData->_hitTTMOp0110 = false;
			_adsData->_scriptDelay = -1;
			bool scriptresult = false;
			// Next few lines of code in a separate function in the original..
			if (curframe < env->_totalFrames && curframe > -1 && env->_frameOffsets[curframe] > -1) {
				env->scr->seek(env->_frameOffsets[curframe]);
				scriptresult = playScene();
			}

			if (scriptresult && sflag != 5) {
				seq->_executed = true;
				seq->_lastFrame = seq->_currentFrame;
				result = true;
				if (_adsData->_scriptDelay != -1 && seq->_timeInterval != _adsData->_scriptDelay) {
					uint32 now = DgdsEngine::getInstance()->getThisFrameMs();
					seq->_timeNext = now + _adsData->_scriptDelay;
					seq->_timeInterval = _adsData->_scriptDelay;
				}

				if (!_adsData->_hitTTMOp0110) {
					if (_adsData->_gotoTarget != -1) {
						seq->_gotoFrame = _adsData->_gotoTarget;
						if (seq->_currentFrame == _adsData->_gotoTarget)
							seq->_selfLoop = true;
					}
					if (seq->_runFlag != kRunTypePaused)
						updateSeqTimeAndFrame(env, seq);
				} else {
					seq->_gotoFrame = seq->_startFrame;
					if (seq->_runFlag == kRunTypeMulti && seq->_runCount != 0) {
						bool updated = updateSeqTimeAndFrame(env, seq);
						if (updated) {
							seq->_runCount--;
						}
					} else if (seq->_runFlag == kRunTypeTimeLimited && seq->_timeCut != 0) {
						updateSeqTimeAndFrame(env, seq);
					} else {
						bool updated = updateSeqTimeAndFrame(env, seq);
						if (updated) {
							seq->_runFlag = kRunTypeFinished;
							seq->_timeInterval = 0;
						}
					}
				}
			} else if (sflag != 5) {
				seq->_gotoFrame = seq->_startFrame;
				seq->_runFlag = kRunTypeFinished;
			}
		}

		if (rflag == kRunTypeTimeLimited && seq->_timeCut <= DgdsEngine::getInstance()->getThisFrameMs()) {
			seq->_runFlag = kRunTypeFinished;
		}
	}
	return result;
}

bool ADSInterpreter::runUntilBranchOpOrEnd() {
	Common::SeekableReadStream *scr = _adsData->scr;
	if (!scr || scr->pos() >= scr->size())
		return false;

	bool more = true;
	do {
		uint16 code = scr->readUint16LE();
		if (code == 0xffff)
			return false;
		more = handleOperation(code, scr);
	} while (!_adsData->_hitBranchOp && more && scr->pos() < scr->size());

	_adsData->_hitBranchOp = false;

	return more;
}

void ADSInterpreter::setHitTTMOp0110(bool val /* = true */) {
	_adsData->_hitTTMOp0110 = val;
}

void ADSInterpreter::setGotoTarget(int32 target) {
	_adsData->_gotoTarget = target;
}

int ADSInterpreter::numArgs(uint16 opcode) const {
	// TODO: This list is from DRAGON, there may be more entries in newer games.
	switch (opcode) {
	case 0x1080:
	case 0x1090:
	case 0x1380:
	case 0x1390:
	case 0x13A0:
	case 0x13A1:
	case 0x13B0:
	case 0x13B1:
	case 0x13C0:
	case 0x13C1:
	case 0x3020:
	case 0xF010:
	case 0xF200:
	case 0xF210:
		return 1;

	case 0x1010:
	case 0x1020:
	case 0x1030:
	case 0x1040:
	case 0x1050:
	case 0x1060:
	case 0x1070:
	case 0x1310:
	case 0x1320:
	case 0x1330:
	case 0x1340:
	case 0x1350:
	case 0x1360:
	case 0x1370:
		return 2;

	case 0x2010:
	case 0x2015:
	case 0x2020:
	case 0x4000:
	case 0x4010:
		return 3;

	case 0x2000:
	case 0x2005:
		return 4;

	default:
		return 0;
	}
}

Common::Error ADSInterpreter::syncState(Common::Serializer &s) {
	//TODO: Currently sync all states then set the active one,
	// do we need to load/save all?
	uint32 numTexts = _adsTexts.size();
	s.syncAsUint32LE(numTexts);

	Common::Array<Common::String> scriptNames;
	Common::String activeScript;

	if (s.isLoading()) {
		for (uint32 i = 0; i < numTexts; i++) {
			Common::String txtName;
			s.syncString(txtName);
			load(txtName);
			scriptNames.push_back(txtName);
		}
	} else {
		for (const auto &node : _adsTexts) {
			Common::String txtName = node._key;
			s.syncString(txtName);
			scriptNames.push_back(txtName);
			if (&node._value == _adsData)
				activeScript = txtName;
		}
	}

	// Text order should be the same
	if (s.getVersion() < 3) {
		for (const Common::String &name : scriptNames) {
			_adsTexts[name].syncState(s);
		}
	}

	for (const Common::String &name : scriptNames) {
		load(name);
	}

	s.syncString(activeScript);
	assert(activeScript.empty() || _adsTexts.contains(activeScript));
	_adsData = activeScript.empty() ? nullptr : &_adsTexts[activeScript];

	return Common::kNoError;
}

} // end namespace Dgds
