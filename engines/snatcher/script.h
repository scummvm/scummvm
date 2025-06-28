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

#include "common/array.h"
#include "common/func.h"
#include "common/ptr.h"

#define		SNATCHER_SCRIPT_DEBUG

namespace Common {
class SeekableReadStream;
class SeekableWriteStream;
} // namespace Common

namespace Snatcher {

class ActionSequenceHandler;
class ResourcePointer;
class SnatcherEngine;
class UI;

class CmdQueue {
public:
	CmdQueue(SnatcherEngine *vm);
	~CmdQueue();
	void reset();
	void writeUInt16(uint16 val);
	void writeUInt32(uint32 val);
	void start() { _enable = (_writePos != _data); }
	bool enabled() const { return _enable; }
	void run(GameState &state);

	void loadState(Common::SeekableReadStream *in);
	void saveState(Common::SeekableWriteStream *out);

private:
	const uint16 *_readPos;
	uint16 *_data;
	uint16 *_writePos;
	uint16 _currentOpcode;
	int16 _progress;
	uint16 _counter;
	bool _enable;

	SnatcherEngine *_vm;
	GameState *_state;

private:
	void makeFunctions();

	void m_end(const uint16 *&data);
	void m_initAnimations(const uint16 *&data);
	void m_animPauseAll(const uint16 *&data);
	void m_drawCommands(const uint16 *&data);
	void m_fmSfxBlock(const uint16 *&data);
	void m_printText(const uint16 *&data);
	void m_06(const uint16 *&data);
	void m_fmMusicStart(const uint16 *&data);
	void m_displayDialog(const uint16 *&data);
	void m_animOps(const uint16 *&data);
	void m_pcmWait(const uint16 *&data);
	void m_cdaWait(const uint16 *&data);
	void m_pcmSound(const uint16 *&data);
	void m_chatWithPortraitAnim(const uint16 *&data);
	void m_cdaPlay(const uint16 *&data);
	void m_gfxReset(const uint16 *&data);
	void m_waitFrames(const uint16 *&data);
	void m_loadResource(const uint16 *&data);
	void m_gfxStart(const uint16 *&data);
	void m_fmMusicWait(const uint16 *&data);
	void m_nop(const uint16 *&data);
	void m_invItemCloseUp(const uint16 *&data);
	void m_resetTextFields(const uint16 *&data);
	void m_unused(const uint16 *&data);
	void m_shooterSequenceRun(const uint16 *&data);
	void m_fmSfxWait(const uint16 *&data);
	void m_nop2(const uint16 *&data);
	void m_cdaSync(const uint16 *&data);
	void m_fmSoundEffect(const uint16 *&data);
	void m_pcmBlock(const uint16 *&data);
	void m_saveGame(const uint16 *&data);
	void m_shooterSequenceResetScore(const uint16 *&data);
	void m_gfxPostLoadProcess(const uint16 *&data);
	void m_waitForStartButton(const uint16 *&data);
	void m_palOps(const uint16 *&data);
	void m_clearTextInputLine(const uint16 *&data);

	typedef Common::Functor1Mem<const uint16*&, void, CmdQueue> CmdQueOpcode;
	Common::Array<CmdQueOpcode*> _opcodes;
};

class ScriptArray {
public:
	ScriptArray() : _data(nullptr) {}
	ScriptArray(uint8 *data, uint8 size) : _data(data) {
		*data = size;
	}
	uint8 size() const { return _data[0]; }
	uint8 pos() const {	return _data[1]; }
	uint8 &pos() { return _data[1]; }
	uint16 read(uint8 entry) const {
		if (entry >= size())
			error("ScriptArrayEntry::read: entry %d out of bounds", entry);
		return *((uint16*)&_data[2 + entry * 2]);
	}
	void write(uint8 entry, uint16 val) {
		if (entry >= size())
			error("ScriptArrayEntry::write: entry %d out of bounds", entry);
		*((uint16*)&_data[2 + entry * 2]) = val;
	}
private:
	uint8 *_data;
};

class MemAccessHandler;
class ScriptEngine {
public:
	ScriptEngine(CmdQueue *que, UI *ui, ActionSequenceHandler *aseq, MemAccessHandler *mem, ResourcePointer *scd);
	~ScriptEngine();

	void resetArrays();
	void run(Script &script);
	bool postProcess(Script &script);
	void processInput();

	void loadState(Common::SeekableReadStream *in, Script &script, bool onlyTempData);
	void saveState(Common::SeekableWriteStream *out, Script &script, bool onlyTempData);

private:
	CmdQueue *_que;
	MemAccessHandler *_mem;
	ActionSequenceHandler *_aseq;
	UI *_ui;
	Script *_script;

	uint8 *_arrayData;
	ScriptArray _arrays[6];
	uint8 *_flagsTable;
	int _pos1;
	int _pos2;
	uint16 _result;
	uint _op;

private:
	void runOpcode();
	void getOpcodeProperties(uint16 &m1, uint16 &m2, uint16 &u);
	void runOpcodeOrReadVar(uint16 mode, uint16 &result);
	void getFlags(uint16 sel, uint16 &result);
	void setFlags(uint16 sel, uint32 flags);

	uint8 countFunctionOps();
	void seekToFunctionOp(int num);
	bool isExecFuncOpcode();
	bool stackOps_checkFor52(uint16 &result);
	bool opcodeHasFlag8();
	bool evalFinished();
	void skipOnFlag8();
	void jump();

	bool getArrayLastEntry(uint16 arrNo, uint16 &result);
	bool popArrayLastEntry(uint16 arrNo, uint16 &result);
	bool getArrayEntry(uint16 arrNo, uint8 pos, uint16 &result);
	bool setArrayLastEntry(uint16 arrNo, uint16 val);

	uint8 getOpcode(int offset);

private:
	void makeOpcodeTable(ResourcePointer *scd);

	void o_nop();
	void o_animOps();
	void o_evalAdd();
	void o_chatWithPortraitAnim();
	void o_pcmSound();
	void o_evalAnd();
	void o_fmMusicStart();
	void o_runSubScript();
	void o_callSubroutine();
	void o_cdaPlay();
	void o_incrCounter();
	void o_fmSoundEffect();
	void o_evalEquals();
	void o_clearFlags();
	void o_setFlags();
	void o_getPrevPosition();
	void o_evalLess();
	void o_evalLessOrEqual();
	void o_doEitherOr();
	void o_doIf();
	void o_setResult();
	void o_execFunc();
	void o_evalGreater();
	void o_evalGreaterOrEqual();
	void o_break();
	void o_evalTrue();
	void o_start();
	void o_evalOr();
	void o_condExec();
	void o_sentenceRecurseBegin();
	void o_verbOps();
	void o_sentenceRecurseEnd();
	void o_returnFromSubScript();
	void o_checkSubRecurseState();
	void o_loadModuleAndStartGfx();
	void o_randomSwitch();
	void o_sequentialSwitch();
	void o_64();
	void o_sysOps();
	void o_displayDialog();
	void o_animWait();
	void o_pcmSoundWait();
	void o_fmMusicWait();
	void o_cdaWait();
	void o_fmSfxWait();
	void o_waitFrames();

#ifndef SNATCHER_SCRIPT_DEBUG
	typedef Common::Functor0Mem<void, ScriptEngine> ScriptEngineProc;
#else
	class ScriptEngineProc : public Common::Functor0Mem<void, ScriptEngine> {
	public:
		typedef void (ScriptEngine::*ScrFunc)();
		ScriptEngineProc(ScriptEngine *se, ScrFunc func, const char *desc, uint8 len, uint8 flg) : Common::Functor0Mem<void, ScriptEngine>(se, func), _desc(Common::String("o_") + desc) {
			_desc += Common::String::format("() [len: %d, flags: 0x%02x]", len, flg);
			_recursion = 0;
		}
		void operator()(int pos) {
			Common::String spc;
			for (int i = 0; i < _recursion; ++i)
				spc += "|--";
			debug("0x%04x: %s%s", pos, spc.c_str(), _desc.c_str());
			++_recursion;
			Functor0Mem<void, ScriptEngine>::operator()();
			--_recursion;
		}
	private:
		static int _recursion;
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
