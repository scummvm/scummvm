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

#ifndef SNATCHER_SCRIPT_H
#define SNATCHER_SCRIPT_H


#include "common/ptr.h"
#include "snatcher/resource.h"

#define		SNATCHER_SCRIPT_DEBUG

namespace Snatcher {

class FIO;
class SnatcherEngine;

class CmdQueue {
public:
	CmdQueue(SnatcherEngine *vm);
	~CmdQueue();
	void reset();
	void writeUInt16(uint16 val);
	void writeUInt32(uint32 val);
	void start() { _enable = true; }	
	bool enabled() const { return _enable; }
	void run(GameState &state);

private:
	const uint16 *_readPos;
	uint16 *_data;
	uint16 *_writePos;
	uint16 _currentOpcode;
	int16 _progress;
	uint16 _counter;
	bool _enable;

	SnatcherEngine *_vm;

private:
	void makeFunctions();

	void m_end(GameState &state, const uint16 *&data);
	void m_initAnimations(GameState &state, const uint16 *&data);
	void m_02(GameState &state, const uint16 *&data);
	void m_drawCommands(GameState &state, const uint16 *&data);
	void m_04(GameState &state, const uint16 *&data);
	void m_printText(GameState &state, const uint16 *&data);
	void m_06(GameState &state, const uint16 *&data);
	void m_fmMusicStart(GameState &state, const uint16 *&data);
	void m_displayDialog(GameState &state, const uint16 *&data);
	void m_animOps(GameState &state, const uint16 *&data);
	void m_pcmWait(GameState &state, const uint16 *&data);
	void m_11(GameState &state, const uint16 *&data);
	void m_12(GameState &state, const uint16 *&data);
	void m_mmSeq(GameState &state, const uint16 *&data);
	void m_14(GameState &state, const uint16 *&data);
	void m_gfxReset(GameState &state, const uint16 *&data);
	void m_waitFrames(GameState &state, const uint16 *&data);
	void m_loadResource(GameState &state, const uint16 *&data);
	void m_gfxStart(GameState &state, const uint16 *&data);
	void m_fmMusicWait(GameState &state, const uint16 *&data);
	void m_20(GameState &state, const uint16 *&data);
	void m_21(GameState &state, const uint16 *&data);
	void m_resetTextFields(GameState &state, const uint16 *&data);
	void m_23(GameState &state, const uint16 *&data);
	void m_24(GameState &state, const uint16 *&data);
	void m_25(GameState &state, const uint16 *&data);
	void m_26(GameState &state, const uint16 *&data);
	void m_27(GameState &state, const uint16 *&data);
	void m_28(GameState &state, const uint16 *&data);
	void m_29(GameState &state, const uint16 *&data);
	void m_30(GameState &state, const uint16 *&data);
	void m_31(GameState &state, const uint16 *&data);
	void m_32(GameState &state, const uint16 *&data);
	void m_33(GameState &state, const uint16 *&data);
	void m_palOps(GameState &state, const uint16 *&data);
	void m_35(GameState &state, const uint16 *&data);

	typedef Common::Functor2Mem<GameState&, const uint16*&, void, CmdQueue> CmdQueOpcode;
	Common::Array<CmdQueOpcode*> _opcodes;

private:
	void printSceneEntryStringHead();
	void printSceneEntryStringBody(GameState &state);
	bool checkStringProgress();
	bool waitWithCursorAnim();

	uint8 *_textBuffer;
	uint8 _textColor;
	uint8 _textY;
	uint8 _textY2;
	//uint8 _makestrbt1;
	uint8 _sceneId;
	uint8 _textLineBreak;
	uint8 _textLineEnd;
	uint16 _sceneTextOffsCur;
	uint16 _sceneInfo;
	uint16 _sceneTextOffset;
	uint16 _sceneTextOffsStart;
	uint8 _waitCursorFrame;
	uint8 _waitCursorAnimDelay;
};

struct Script {
	Script() : scriptStateByteUnk(0), unkislp(0), data(nullptr), dataSize(0), curPos(0), newPos(-1), curFileNo(0), curGfxScript(-1), stack(nullptr), stackSize(0x600), sp(nullptr), bp(nullptr), res() {
		stack = new uint8[stackSize]();
		sp = bp = &stack[stackSize];
	}
	~Script() {
		delete[] stack;
	}
	uint8 scriptStateByteUnk;
	uint8 unkislp;
	uint8 curFileNo;
	int16 curGfxScript;
	ResourcePointer res;
	const uint8 *data;
	uint32 dataSize;
	int curPos;
	int newPos;
	uint8 *stack;
	const uint32 stackSize;
	uint8 *sp;
	uint8 *bp;
};

class ScriptEngine {
public:
	ScriptEngine(SnatcherEngine *vm, CmdQueue *que, ResourcePointer *scd);
	~ScriptEngine();

	//void writeVar(uint16 arrayNo, uint16 pos, uint8 val);
	void resetArrays();
	void run(Script *script);
	bool postProcess(Script *script);

private:
	SnatcherEngine *_vm;
	CmdQueue *_que;
	Script *_script;

	uint8 *_var;
	uint8 *_arrays[6];
	uint8 *_buf352;
	int _curPos;
	int /*_v1, _v2, */_v3, _v5;
	uint _op;

private:
	void runOpcode(int offset);
	void getOpcodeProps(int offset, int &len, int &f1, int &f2);
	void runOpcode(int &offset, int len, int &result);
	void getFlag(int arg, int &result);
	void setFlag(int arg, int val);

	uint8 countFunctionOps(int offset);
	void seekToFunctionOp(int &offset, int arg);
	bool isExecFuncOpcode(int offset);
	bool stackOps_checkFor52(int offset, int &result);
	bool opcodeHasFlag8(int offset);
	bool OPC_4_ss();
	void skipOnFlag8(int &offset);
	void stackOps_unkParseTo(int &offset);

	bool getArrayLastEntry(int arrNo, int &result);
	bool popArrayLastEntry(int arrNo, int &result);
	bool getArrayEntry(int arrNo, int val, int &result);
	bool setArrayLastEntry(int arrNo, int val);

	uint8 getOpcode(int offset);

private:
	void makeOpcodeTable(ResourcePointer *scd);

	void o_nop();
	void o_animOps();
	void o_01();
	void o_mmSequence();
	void o_05();
	void o_06();
	void o_fmMusicStart();
	void o_11();
	void o_12();
	void o_13();
	void o_14();
	void o_15();
	void o_16();
	void o_19();
	void o_20();
	void o_21();
	void o_22();
	void o_23();
	void o_24();
	void o_26();
	void o_28();
	void o_executeFunction();
	void o_40();
	void o_42();
	void o_break();
	void o_46();
	void o_startFunction();
	void o_49();
	void o_52();
	void o_54();
	void o_56();
	void o_57();
	void o_loadScript();
	void o_59();
	void o_loadModuleAndStartGfx();
	void o_62();
	void o_63();
	void o_64();
	void o_subOps();
	void o_displayDialog();
	void o_animWait();
	void o_pcmSoundWait();
	void o_fmMusicWait();
	void o_72();
	void o_73();
	void o_waitFrames();

#ifndef SNATCHER_SCRIPT_DEBUG
	typedef Common::Functor0Mem<void, ScriptEngine> ScriptEngineProc;
#else
	class ScriptEngineProc : public Common::Functor0Mem<void, ScriptEngine> {
	public:
		typedef void (ScriptEngine::*ScrFunc)();
		ScriptEngineProc(ScriptEngine *se, ScrFunc func, const char *desc, uint8 len, uint8 flg) : Common::Functor0Mem<void, ScriptEngine>(se, func), _desc(Common::String("o_") + desc) {
			_desc += Common::String::format("() [len: %d, flags: 0x%02x]", len, flg);
		}
		void operator()() {
			debug("%s", _desc.c_str());
			Functor0Mem<void, ScriptEngine>::operator()();
		}
	private:
		Common::String _desc;
	};
#endif

	struct ScriptEngineOpcode {
		ScriptEngineOpcode() : len(0), flags(0), proc(nullptr) {}
		ScriptEngineOpcode(uint8 v1, uint8 v2, ScriptEngineProc *p) : len(v1), flags(v2), proc(Common::SharedPtr<ScriptEngineProc>(p)) {}
		~ScriptEngineOpcode() {}
		uint8 len;
		uint8 flags;
		Common::SharedPtr<ScriptEngineProc> proc;
	};
	Common::Array<ScriptEngineOpcode> _opcodes;
};

} // End of namespace Snatcher

#endif // SNATCHER_SCRIPT
