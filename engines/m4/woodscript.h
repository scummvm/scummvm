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

#ifndef M4_WOODSCRIPT_H
#define M4_WOODSCRIPT_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/str.h"
#include "common/array.h"
#include "common/stream.h"
#include "graphics/surface.h"

#include "m4/globals.h"
#include "m4/assets.h"
#include "m4/resource.h"
#include "m4/sprite.h"
#include "m4/m4.h"
#include "m4/graphics.h"
#include "m4/viewmgr.h"

namespace M4 {

class MadsM4Engine;
class WoodScript;
class Machine;
class Sequence;
class AssetManager;
class View;

struct Instruction {
	int32 instr;
	long *argp[3];
	long argv[3];
	int argc;
	// Helper method; many opcode functions can get either a defined value or a random value
	long getValue() {
		if (argc == 3)
			return _vm->imath_ranged_rand16(argv[1], argv[2]);
		else
			return argv[1];
	}
};

class Bytecode {
public:
	Bytecode(WoodScript *ws, byte *code, int32 codeSize, Sequence *seq);
	~Bytecode();
	int loadInstruction(Instruction &instruction);
	void jumpAbsolute(int32 ofs);
	void jumpRelative(int32 ofs);
	void setSequence(Sequence *sequence);
	void setCode(byte *code, int32 codeSize);
	Sequence *sequence() const;
	uint32 pos() const { return _code->pos() / 4; }
protected:
	WoodScript *_ws;
	Common::SeekableReadStream *_code;
	Sequence *_sequence;
	static int32 _dataFormats[];
	bool decodeArgument(int32 format, int32 data, long *&arg, long &value);
};

struct EndOfSequenceRequestItem {
	int32 codeOffset, count;
	EndOfSequenceRequestItem() : codeOffset(-1) {}
	bool isValid() const { return codeOffset >= 0; }
};

typedef Common::Array<EndOfSequenceRequestItem> EndOfSequenceRequestList;

class Sequence {
public:
	Sequence(WoodScript *ws, Machine *machine, int32 sequenceHash);
	~Sequence();

	void pause();
	void resume();
	void issueEndOfSequenceRequest(int32 codeOffset, int32 count);
	void cancelEndOfSequenceRequest();

	bool runProgram();

	bool changeProgram(int32 sequenceHash);

	void clearVars();

	long *getVarPtr(int index);
	long *getParentVarPtr(int index);
	long *getDataPtr(int index);

	void setActive(bool active) { _active = active; }
	bool isActive() const { return _active; }

	bool isTerminated() const { return _terminated; }

	void draw(M4Surface *surface, const Common::Rect &clipRect, Common::Rect &updateRect);

	bool s1_end(Instruction &instruction);
	bool s1_clearVars(Instruction &instruction);
	bool s1_set(Instruction &instruction);
	bool s1_compare(Instruction &instruction);
	bool s1_add(Instruction &instruction);
	bool s1_sub(Instruction &instruction);
	bool s1_mul(Instruction &instruction);
	bool s1_div(Instruction &instruction);
	bool s1_and(Instruction &instruction);
	bool s1_or(Instruction &instruction);
	bool s1_not(Instruction &instruction);
	bool s1_sin(Instruction &instruction);
	bool s1_cos(Instruction &instruction);
	bool s1_abs(Instruction &instruction);
	bool s1_min(Instruction &instruction);
	bool s1_max(Instruction &instruction);
	bool s1_mod(Instruction &instruction);
	bool s1_floor(Instruction &instruction);
	bool s1_round(Instruction &instruction);
	bool s1_ceil(Instruction &instruction);
	bool s1_point(Instruction &instruction);
	bool s1_dist2d(Instruction &instruction);
	bool s1_crunch(Instruction &instruction);
	bool s1_branch(Instruction &instruction);
	bool s1_setFrame(Instruction &instruction);
	bool s1_sendMessage(Instruction &instruction);
	bool s1_push(Instruction &instruction);
	bool s1_pop(Instruction &instruction);
	bool s1_jumpSub(Instruction &instruction);
	bool s1_return(Instruction &instruction);
	bool s1_getFrameCount(Instruction &instruction);
	bool s1_getFrameRate(Instruction &instruction);
	bool s1_getCelsPixSpeed(Instruction &instruction);
	bool s1_setIndex(Instruction &instruction);
	bool s1_setLayer(Instruction &instruction);
	bool s1_setDepth(Instruction &instruction);
	bool s1_setData(Instruction &instruction);
	bool s1_openStream(Instruction &instruction);
	bool s1_streamNextFrame(Instruction &instruction);
	bool s1_closeStream(Instruction &instruction);

	int32 indexReg() const { return _indexReg; }

	EndOfSequenceRequestItem getEndOfSequenceRequestItem() const { return _endOfSequenceRequest; }
	bool hasEndOfSequenceRequestPending() const { return _endOfSequenceRequest.isValid(); }
	void resetEndOfSequenceRequest() { _endOfSequenceRequest.codeOffset = -1; }

	Machine *getMachine() const { return _machine; }


protected:
	WoodScript *_ws;
	Bytecode *_code;

	long *_vars;
	bool _active, _terminated;
	Machine *_machine;
	Sequence *_parentSequence;
	int32 _layer;
	int32 _startTime, _switchTime;
	long *_dataRow;
	int32 _localVarCount;
	int32 _cmpFlags;

	EndOfSequenceRequestItem _endOfSequenceRequest;

	int32 _indexReg;

	M4Sprite *_curFrame;

	int32 _sequenceHash;

	int32 _returnHashes[8]; //FIXME: Use constant instead of 8
	uint32 _returnOffsets[8];
	int32 _returnStackIndex;

	Common::SeekableReadStream *_stream;
	SpriteAsset *_streamSpriteAsset;

	bool streamOpen();
	bool streamNextFrame();
	void streamClose();

};

class Machine {
public:
	Machine(WoodScript *ws, int32 machineHash, Sequence *parentSeq, int32 dataHash,
		int32 dataRowIndex, int callbackHandler, Common::String machineName, int32 id);
	~Machine();

	void clearMessages();
	void clearPersistentMessages();
	void restorePersistentMessages();
	void sendMessage(uint32 messageHash, long messageValue, Machine *sender);
	void resetSwitchTime();
	bool changeSequenceProgram(int32 sequenceHash);

	bool searchMessages(uint32 messageHash, uint32 messageValue, Machine *sender);
	bool searchPersistentMessages(uint32 messageHash, uint32 messageValue, Machine *sender);

	void enterState();
	int32 execInstruction();
	void execBlock(int32 offset, int32 count);
	int32 getState() { return _currentState; }

	int32 getId() const { return _id; }

	bool m1_gotoState(Instruction &instruction);
	bool m1_jump(Instruction &instruction);
	bool m1_terminate(Instruction &instruction);
	bool m1_startSequence(Instruction &instruction);
	bool m1_pauseSequence(Instruction &instruction);
	bool m1_resumeSequence(Instruction &instruction);
	bool m1_storeValue(Instruction &instruction);
	bool m1_sendMessage(Instruction &instruction);
	bool m1_broadcastMessage(Instruction &instruction);
	bool m1_replyMessage(Instruction &instruction);
	bool m1_sendSystemMessage(Instruction &instruction);
	bool m1_createMachine(Instruction &instruction);
	bool m1_createMachineEx(Instruction &instruction);
	bool m1_clearVars(Instruction &instruction);

	void m1_onEndSequence(Instruction &instruction);
	void m1_onMessage(Instruction &instruction);
	void m1_switchLt(Instruction &instruction);
	void m1_switchLe(Instruction &instruction);
	void m1_switchEq(Instruction &instruction);
	void m1_switchNe(Instruction &instruction);
	void m1_switchGe(Instruction &instruction);
	void m1_switchGt(Instruction &instruction);

	long *dataRow() const { return _dataRow; }
	Sequence *parentSequence() const { return _parentSequence; }
	Common::String name() const { return _name; }

protected:
	WoodScript *_ws;
	Bytecode *_code;

	Common::String _name;
	Sequence *_sequence, *_parentSequence;
	byte *_mach;
	int32 _machHash, _machineCodeOffset;
	int32 _stateCount, _stateTableOffset;
	long *_dataRow;
	int32 _id, _recursionLevel, _currentState, _targetCount;
	/* TODO:
	m->msgReplyXM = NULL;
	m->CintrMsg = CintrMsg;
	_walkPath
	_messages
	_persistentMessages
	_usedPersistentMessages
	*/
};

class WoodScript {
public:

	WoodScript(MadsM4Engine *vm);
	~WoodScript();

	Machine *createMachine(int32 machineHash, Sequence *parentSeq, int32 dataHash, int32 dataRowIndex, int callbackHandler, const char *machineName);
	Sequence *createSequence(Machine *machine, int32 sequenceHash);

	void runSequencePrograms();
	void runEndOfSequenceRequests();
	void runTimerSequenceRequests();

	/* Series */
	// Move to own class, e.g. SeriesPlayer
	int32 loadSeries(const char* seriesName, int32 hash, RGB8* palette);
	void unloadSeries(int32 hash);
	void setSeriesFramerate(Machine *machine, int32 frameRate);
	Machine *playSeries(const char *seriesName, long layer, uint32 flags, int32 triggerNum,
		int32 frameRate, int32 loopCount, int32 s, int32 x, int32 y,
		int32 firstFrame, int32 lastFrame);
	Machine *showSeries(const char *seriesName, long layer, uint32 flags, int32 triggerNum,
		int32 duration, int32 index, int32 s, int32 x, int32 y);
	Machine *streamSeries(const char *seriesName, int32 frameRate, long layer, int32 triggerNum);

	void update();
	void clear();

	/* Misc */
	void setDepthTable(int16 *depthTable);

	long *getGlobalPtr(int index);
	long getGlobal(int index);
	void setGlobal(int index, long value);

	AssetManager *assets() const { return _assets; }

	// Sets the untouched, clean surface which contains the room background
	void setBackgroundSurface(M4Surface *backgroundSurface);
	// Sets the view which is used for drawing
	void setSurfaceView(View *view);

	RGB8 *getMainPalette() const;

	void setInverseColorTable(byte *inverseColorTable) { _inverseColorTable = inverseColorTable; }
	byte *getInverseColorTable() const { return _inverseColorTable; }

protected:
	MadsM4Engine *_vm;
	AssetManager *_assets;

	Common::Array<Sequence*> _sequences, _layers;
	Common::Array<Machine*> _machines;
	int32 _machineId;

	long *_globals;

	Common::Array<Sequence*> _endOfSequenceRequestList;

	int32 _indexReg;

	/* Misc */
	int16 *_depthTable;
	byte *_inverseColorTable;
	M4Surface *_backgroundSurface;
	View *_surfaceView;

};


} // End of namespace M4


#endif
