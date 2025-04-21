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


#include "snatcher/graphics.h"
#include "snatcher/transition.h"
#include "graphics/segagfx.h"

namespace Snatcher {

class ScrollInternalState {
public:
	ScrollInternalState() : _offsSingle(0), _offsRep(0), _curPos(0), _curResult(0), _factor(1), _override(0) {}

	void singleStep(int16 step) {
		_offsSingle = step << 16;
	}

	void setDirectionAndSpeed(int16 incr) {
		_offsRep = incr << 16;
	}

	void setFactor(int16 factor) {
		_factor = factor;
	}

	void setNextOffset(int16 offs) {
		_override = offs;
	}

	void clear() {
		_offsSingle = 0;
		_offsRep = 0;
		_curPos = 0;
		_curResult = 0;
	}

	bool recalc(bool singleStepABS) {
		_curPos = singleStepABS ? _offsSingle : _offsSingle + _curPos;
		_offsSingle = 0;
		int32 t = (_offsRep >> 16);
		_curPos += (t << 8);
		int16 r = _curPos >> 16;
		bool changed = (_curResult != r);
		_curResult = r;
		return changed;
	}

	int16 getRealOffset(bool override) const {
		return override ? _override : _curResult * _factor;
	}

	int16 getUnmodifiedOffset() const {
		return _curResult;
	}

private:
	int32 _offsSingle;
	int32 _offsRep;
	int32 _curPos;
	int16 _curResult;
	int16 _factor;
	int16 _override;
};

class TransitionManager_SCD final : public TransitionManager {
public:
	enum {
		kHorzA = 0,
		kVertA = 1,
		kHorzB = 2,
		kVertB = 3
	};

	TransitionManager_SCD(GraphicsEngine::GfxState &state);
	~TransitionManager_SCD() override;

	void doCommand(int cmd) override;

	void scroll_setSingleStep(int mode, int16 step) override {
		assert(mode < 4);
		_internalState[mode].singleStep(step);
	}

	void scroll_setDirectionAndSpeed(int mode, int16 incr) override {
		assert(mode < 4);
		_internalState[mode].setDirectionAndSpeed(incr);
	}

	void clear() override;
	bool nextFrame() override;

	void hINTCallback(void *segaRenderer) override;

private:
	void processCmdInternal();
	void processTransition();
	void resetVars(int groupFlags);

	GraphicsEngine::GfxState &_gfxState;
	ScrollInternalState *_internalState;
	int16 *_hScrollTable;
	uint16 _hScrollTableLen;

	uint8 _scrollType;
	uint8 _trsCommandExt;
	uint8 _trsCommand;
	uint8 _resetCommand;

	uint8 _nextStep;
	uint8 _lastStep;
	uint8 _trsCmd_0xff_0xfd_0xfc_or0to6_last;
	uint8 _nextStepExt;
	uint8 _lastStepExt;
	int16 _trsFlagttt;
	int16 _trsu_7;
	int16 _hint_proc;
	int16 _transitionType;
	int16 _transitionState;
	int16 _transitionStep;
	int16 _trsu_5;
	int16 _subPara;
	uint32 _trs__DA;
	uint16 _trs__DB;
	bool useEngineScrollOffsets;

private:
	typedef Common::Functor1Mem<int, void, TransitionManager_SCD> TrsFunc;
	Common::Array<TrsFunc*> _trsProcs;

	void makeFunctions();
	void doCommandIntern(int cmd, int arg);

	void trsUpdt_dummy(int arg);
	void trsUpdt_engineScroll(int arg);
	void trsUpdt_03(int arg);
	void trsUpdt_04(int arg);
	void trsUpdt_05(int arg);
	void trsUpdt_06(int arg);
	void trsUpdt_screenShutter(int arg);
	void trsUpdt_revealShutter(int arg);
	void trsUpdt_13(int arg);
	void trsUpdt_14(int arg);
	void trsUpdt_15(int arg);
	void trsUpdt_16(int arg);
	void trsUpdt_17(int arg);
	void trsUpdt_18(int arg);
	void trsUpdt_19(int arg);
	void trsUpdt_20(int arg);
	void trsUpdt_21(int arg);
	void trsUpdt_22(int arg);
	void trsUpdt_23(int arg);
	void trsUpdt_24(int arg);
	void trsUpdt_25(int arg);
	void trsUpdt_26(int arg);
	void trsUpdt_27(int arg);
	void trsUpdt_28(int arg);
	void trsUpdt_29(int arg);

	typedef Common::Functor1Mem<Graphics::SegaRenderer*, void, TransitionManager_SCD> HINTFunc;
	Common::Array<HINTFunc*> _hINTProcs;
	const HINTFunc *_hINTHandler;

	void setHINTHandler(uint8 num);

	void hIntHandler_screenShutter(Graphics::SegaRenderer *sr);
	void hIntHandler_revealShutter(Graphics::SegaRenderer *sr);
};

TransitionManager_SCD::TransitionManager_SCD(GraphicsEngine::GfxState &state) : _gfxState(state), _hScrollTable(nullptr), _hScrollTableLen(0), _internalState(nullptr), _scrollType(0), _hint_proc(0), _transitionStep(0),
	_trsCommandExt(0), _trsCommand(0), _resetCommand(0), _nextStepExt(0), _nextStep(0), _lastStepExt(0), _lastStep(0), _trs__DA(0), _trs__DB(0), _trsCmd_0xff_0xfd_0xfc_or0to6_last(0), _trsFlagttt(0),
	_trsu_7(0), _transitionType(0), _transitionState(0), _trsu_5(0), _subPara(0), useEngineScrollOffsets(false), _hINTHandler(nullptr) {
	_internalState = new ScrollInternalState[4];
	assert(_internalState);
	_internalState[kVertA].setFactor(-1);
	_internalState[kVertB].setFactor(-1);
	_hScrollTable = new int16[0x200]();
	assert(_hScrollTable);
	makeFunctions();
}

TransitionManager_SCD::~TransitionManager_SCD() {
	delete[] _internalState;
	delete[] _hScrollTable;
	for (Common::Array<TrsFunc*>::const_iterator i = _trsProcs.begin(); i != _trsProcs.end(); ++i)
		delete *i;
	for (Common::Array<HINTFunc*>::const_iterator i = _hINTProcs.begin(); i != _hINTProcs.end(); ++i)
		delete *i;
}

void TransitionManager_SCD::doCommand(int cmd) {
	_gfxState.setVar(3, 1);

	if (cmd == 0xFF || cmd == 0xFC) {
		clear();
		_resetCommand = cmd;
	} else {
		if (cmd >= 7 && cmd != 0xFD) {
			_trsCommandExt = cmd;
		} else {
			_trsCommand = cmd;
			_scrollType = (cmd < 3) ? cmd | ((cmd & 1) << 4) : 0;
		}
	}
}

void TransitionManager_SCD::clear() {
	for (int i = 0; i < 4; ++i) 
		_internalState[i].clear();
	memset(&_result, 0, sizeof(_result));
	_scrollType = 0;
	_trsCommandExt = 0;
	_trsCommand = 0;
	_resetCommand = 0;
}

bool TransitionManager_SCD::nextFrame() {
	if (_resetCommand == 0) {
		if (_trsCommandExt)
			_nextStepExt = _trsCommandExt;
		if (_trsCommand)
			_nextStep = _trsCommand;
		_trsCommand = _trsCommandExt = 0;
	} else {
		_nextStep = _resetCommand;
		_resetCommand = 0;
		_trsCommandExt = 0;
		// _wordRAM__TABLE48__09 = 0;
		// _wordRAM__TABLE48__0A = 0;
	}

	if (_nextStepExt != _lastStepExt || _lastStepExt == 0) {
		if (_nextStepExt == 0xFE) {
			if (_lastStepExt == 23) {
				_internalState[kVertA].setNextOffset(0);
				_internalState[kVertB].setNextOffset(0);
				if (_trsFlagttt) {
					resetVars(0x30);
					_nextStepExt = _lastStepExt = 15;
				} else {
					resetVars(0x18);
				}
			}
		} else if (_nextStepExt != 0 && (_nextStepExt != 15 || _lastStepExt != 23)) {
			_lastStepExt = _nextStepExt;
			resetVars(0x40);
		} else if (_nextStepExt != 0) {
			_nextStepExt = 23;
			_trsFlagttt = 1;
		}
	}

	processTransition();

	bool changed = false;
	bool reset = _nextStep == 0xFF || _nextStep == 0xFC;

	if (_scrollType != 0 && !reset) {
		for (int i = 0; i < 4; ++i) {
			if (_internalState[i].recalc(_scrollType & 0x10))
				changed = true;
			_result.realOffsets[i] = _internalState[i].getRealOffset(!(changed || useEngineScrollOffsets));
			_result.unmodifiedOffsets[i] = _internalState[i].getUnmodifiedOffset();
			_internalState[i].setNextOffset(_result.realOffsets[i]);
		}
		_result.hScrollTable = _hScrollTable;
		_result.hScrollTableNumEntries = _hScrollTableLen;
		_result.disableVScroll = (_trsCmd_0xff_0xfd_0xfc_or0to6_last != 0);
		_scrollType &= ~0x10;
	}

	processCmdInternal();

	return changed || reset || _result.hInt.needUpdate;
}

void TransitionManager_SCD::hINTCallback(void *segaRenderer) {
	if (_hINTHandler && _hINTHandler->isValid())
		(*_hINTHandler)(static_cast<Graphics::SegaRenderer *>(segaRenderer));
}

void TransitionManager_SCD::processCmdInternal() {
	if ((_nextStep != _lastStep || _lastStep == 0) && _nextStep != 0) {
		if (_nextStep == 0xFF || _nextStep == 0xFC) {
			resetVars(_nextStep == 0xFF ? 0x0F : 0x0E);
			return;
		} else if (_nextStep == 0xFD) {
			_trsCmd_0xff_0xfd_0xfc_or0to6_last |= 0x80;
		} else {
			_lastStep = _nextStep;
			if (_nextStep < 3) {
				resetVars(0x08);
			} else {
				_trs__DA = 0;
				_trsCmd_0xff_0xfd_0xfc_or0to6_last = 0;
			}
		}
	}

	bool b = false;
	if (_trsCmd_0xff_0xfd_0xfc_or0to6_last != 0) {
		if (_trsCmd_0xff_0xfd_0xfc_or0to6_last & 0x80)
			b = true;
		else
			doCommandIntern(_trsCmd_0xff_0xfd_0xfc_or0to6_last, 0);
	}

	if (!b && _nextStep != 0)
		doCommandIntern(_nextStep, _subPara);

	doCommandIntern(_nextStepExt, _subPara);
}

void TransitionManager_SCD::processTransition() {
	_result.hInt.enable = false;
	if (_transitionType == 0)
		return;

	_transitionStep = 0;

	switch (_transitionType) {
	case 1:
		_result.hInt.counter = 15;
		if (--_transitionState <= 0) {
			_internalState[kVertA].setNextOffset(0);
			_internalState[kVertB].setNextOffset(0);
			return;
		}
		_internalState[kVertA].setNextOffset(_transitionState > 1 ? 0x30 : 0x10);
		_internalState[kVertB].setNextOffset(_transitionState > 1 ? 0x30 : 0x10);
		break;
	case 2: case 3: case 4: case 5: case 6:
		_result.hInt.counter = 7;
		break;
	case 7: case 8:
		break;
	case 9:
		break;
	case 10: case 11:
		break;
	case 12: case 13: case 14: case 15: case 16:
		break;
	case 17:
		break;
	case 18: case 19: case 20: case 21:
		break;
	case 22: case 23:
		break;
	default:
		break;
	}
	_result.hInt.enable = true;
	_result.hInt.needUpdate = true;
}

void TransitionManager_SCD::resetVars(int groupFlags) {
	if (groupFlags & 0x01) {
		_result.hInt.enable = false;
		_result.hInt.needUpdate = true;
	}
	if (groupFlags & 0x02) {
		_trsCmd_0xff_0xfd_0xfc_or0to6_last = 0;
		_trs__DA = 0;
		_trs__DB = 0;
		_nextStepExt = 0;
		_transitionType = _transitionState = _trsu_5 = _subPara = _trsu_7 = 0;
		_trsFlagttt = 0;

	}
	if (groupFlags & 0x04) {
		_nextStep = _lastStep = 0;
		_result.lineScrollMode = false;
		for (int i = 0; i < 4; ++i)
			_internalState[i].setNextOffset(0);
	}
	if (groupFlags & 0x08) {
		useEngineScrollOffsets = false;
		//_trsoa = 0;
		//_trsob = 0;
	}
	if (groupFlags & 0x10) {
		_nextStepExt = _lastStepExt = 0;
		_transitionType = _trsu_7 = 0;
	}
	if (groupFlags & 0x20) {
		_trsFlagttt = 0;
		_internalState[kVertA].setNextOffset(0);
		_internalState[kVertB].setNextOffset(0);
	}
	if (groupFlags & 0x40) {
		_transitionState = _trsu_5 = _subPara = 0;
	}
}

#define TF(x) &TransitionManager_SCD::trsUpdt_##x
#define HF(x) &TransitionManager_SCD::hIntHandler_##x
void TransitionManager_SCD::makeFunctions() {
	typedef void (TransitionManager_SCD::*ScrFunc)(int);
	static const ScrFunc funcTbl[] = {
		TF(dummy),
		TF(engineScroll),
		TF(engineScroll),
		TF(03),
		TF(04),
		TF(05),
		TF(06),
		TF(screenShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(13),
		TF(14),
		TF(15),
		TF(16),
		TF(17),
		TF(18),
		TF(19),
		TF(20),
		TF(21),
		TF(22),
		TF(23),
		TF(24),
		TF(25),
		TF(26),
		TF(27),
		TF(28),
		TF(29)
	};

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i) \
		_trsProcs.push_back(new TrsFunc(this, funcTbl[i]));

	typedef void (TransitionManager_SCD::*HIFunc)(Graphics::SegaRenderer*);
	static const HIFunc funcTbl2[] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		HF(screenShutter),
		HF(revealShutter),
		nullptr,
		nullptr
	};

	for (uint i = 0; i < ARRAYSIZE(funcTbl2); ++i) \
		_hINTProcs.push_back(new HINTFunc(this, funcTbl2[i]));
}
#undef HF
#undef TF

void TransitionManager_SCD::doCommandIntern(int cmd, int arg) {
	if (_trsProcs[cmd]->isValid())
		(*_trsProcs[cmd])(arg);
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, cmd);
}

void TransitionManager_SCD::trsUpdt_dummy(int arg) {
}

void TransitionManager_SCD::trsUpdt_engineScroll(int arg) {
	_result.lineScrollMode = false;
	useEngineScrollOffsets = true;
}

void TransitionManager_SCD::trsUpdt_03(int arg) {
}

void TransitionManager_SCD::trsUpdt_04(int arg) {
}

void TransitionManager_SCD::trsUpdt_05(int arg) {
}

void TransitionManager_SCD::trsUpdt_06(int arg) {
}

void TransitionManager_SCD::trsUpdt_screenShutter(int arg) {
	if (arg == 0) {
		_transitionType = 1;
		setHINTHandler(7);
		_transitionState = 5;
		++_subPara;
		resetVars(0x0C);
	} else if (arg == 1) {
		if (_transitionState < 0)
			resetVars(0x0F);
	} else {
		error("%s(): Invalid arg %d", __FUNCTION__, arg);
	}
}

void TransitionManager_SCD::trsUpdt_revealShutter(int arg) {
	if (arg == 0) {
		_transitionType = _nextStepExt - 6;
		setHINTHandler(8);
		_transitionState = 23;
		++_subPara;
		resetVars(0x0C);
		_transitionState |= ((_transitionState & 0xFC) << 6);
	} else {
		--_transitionState;
		_transitionState &= 0xFF;
		if ((int8)(_transitionState) >= 0) {
			_transitionState |= ((_transitionState & 0xFC) << 6);
		} else {
			_transitionState = 0;
			if (_nextStepExt == 12)
				resetVars(0x0F);
		}
	}
}

void TransitionManager_SCD::trsUpdt_13(int arg) {
}

void TransitionManager_SCD::trsUpdt_14(int arg) {
}

void TransitionManager_SCD::trsUpdt_15(int arg) {
}

void TransitionManager_SCD::trsUpdt_16(int arg) {
}

void TransitionManager_SCD::trsUpdt_17(int arg) {
}

void TransitionManager_SCD::trsUpdt_18(int arg) {
}

void TransitionManager_SCD::trsUpdt_19(int arg) {
}

void TransitionManager_SCD::trsUpdt_20(int arg) {
}

void TransitionManager_SCD::trsUpdt_21(int arg) {
}

void TransitionManager_SCD::trsUpdt_22(int arg) {
}

void TransitionManager_SCD::trsUpdt_23(int arg) {
}

void TransitionManager_SCD::trsUpdt_24(int arg) {
}

void TransitionManager_SCD::trsUpdt_25(int arg) {
}

void TransitionManager_SCD::trsUpdt_26(int arg) {
}

void TransitionManager_SCD::trsUpdt_27(int arg) {
}

void TransitionManager_SCD::trsUpdt_28(int arg) {
}

void TransitionManager_SCD::trsUpdt_29(int arg) {
}

void TransitionManager_SCD::setHINTHandler(uint8 num) {
	if (num < _hINTProcs.size())
		_hINTHandler = _hINTProcs[num];
	 else
		error("%s(): Invalid HINT handler %d", __FUNCTION__, num);
}

void TransitionManager_SCD::hIntHandler_screenShutter(Graphics::SegaRenderer *sr) {
	if (_transitionState <= 0 || _transitionStep == (_transitionState == 1 ? 8 : 10)) {
		_result.hInt.enable = false;
		sr->hINT_enable(false);
		sr->enableDisplay(true);
	} else if (_transitionStep == (_transitionState == 1 ? 6 : 4)) {
		sr->enableDisplay(false);
	} else if (_transitionStep == 7) {
		sr->writeUint16VSRAM(0, TO_BE_16(_transitionState == 1 ? 0x3F0 : 0x3D0));
	}
	++_transitionStep;
}

void TransitionManager_SCD::hIntHandler_revealShutter(Graphics::SegaRenderer *sr) {
	static const int8 actions[5][8] = {
		{  0,  0,  7,  1,  8,  2, -1, -1 },
		{  0,  4,  9,  3, 10,  1, 11,  2 },
		{  0,  4, 12,  3, 13,  1, 14,  2 },
		{  0,  4, 15,  3, 16,  1, 17,  2 },
		{  0,  4, 18,  3, 19,  1, 20,  2 }
	};

	if (_transitionType < 2 || _transitionType > 6)
		error("%s(): Invalid transition type %d", __FUNCTION__, _transitionType);

	int a = -1;
	for (const int8 *it = actions[_transitionType - 2]; it < &actions[_transitionType - 2][8] && a == -1; it += 2) {
		if (_transitionStep == it[0])
			a = it[1];
	}

	switch (a) {
	case 0:
	case 4:
		sr->enableDisplay(true);
		if (a == 4)
			break;
	// fall through
	case 3:
		sr->writeUint16VSRAM(0, TO_BE_16(-(_transitionState >> 8)));
		break;
	case 1:
		sr->writeUint16VSRAM(0, TO_BE_16(_transitionState >> 8));
		break;
	case 2:
		sr->enableDisplay(false);
		sr->hINT_enable(false);
		_result.hInt.enable = false;
		break;
	default:
		break;
	}

	++_transitionStep;
}

TransitionManager *TransitionManager::createSegaTransitionManager(GraphicsEngine::GfxState &state) {
	return new TransitionManager_SCD(state);
}

} // End of namespace Snatcher
