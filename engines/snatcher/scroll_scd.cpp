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
#include "snatcher/scroll.h"

namespace Snatcher {

class ScrollInternalState {
public:
	ScrollInternalState() : _offsSingle(0), _offsRep(0), _curPos(0), _curResult(0), _factor(1) {}

	void singleStep(int16 step) {
		_offsSingle = step << 16;
	}

	void setDirectionAndSpeed(int16 incr) {
		_offsRep = incr << 16;
	}

	void setFactor(int16 factor) {
		_factor = factor;
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

	int16 getOffset() const {
		return _curResult * _factor;
	}

private:
	int32 _offsSingle;
	int32 _offsRep;
	int32 _curPos;
	int16 _curResult;
	int16 _factor;
};

class ScrollManager_SCD : public ScrollManager {
public:
	enum {
		kHorzA = 0,
		kVertA = 1,
		kHorzB = 2,
		kVertB = 3
	};

	ScrollManager_SCD(GraphicsEngine::GfxState &state);
	~ScrollManager_SCD() override;

	void doCommand(int cmd) override;

	void setSingleStep(int mode, int16 step) override {
		assert(mode < 4);
		_internalState[mode].singleStep(step);
	}

	void setDirectionAndSpeed(int mode, int16 incr) override {
		assert(mode < 4);
		_internalState[mode].setDirectionAndSpeed(incr);
	}

	void clear() override;
	bool nextFrame() override;

private:
	bool processCmdInternal();
	void startTransition();

	GraphicsEngine::GfxState &_gfxState;
	ScrollInternalState *_internalState;
	int16 *_hScrollTable;
	uint16 _hScrollTableLen;

	uint8 _scrollType;
	uint8 _scrollCommandExt;
	uint8 _scrollCommand;
	uint8 _resetCommand;

	uint8 _nextStep;
	uint8 _lastStep;
	uint8 _scrollCmd_0xff_0xfd_0xfc_or0to6_last;
	uint8 _nextStepExt;
	uint8 _lastStepExt;
	int16 _lastVScrollOffset;
	int16 _lastHScrollOffset;
	int16 _scrollFlagttt;
	int16 _scrollu_7;
	int16 _hint_proc;
	int16 _transitionType;
	int16 _transitionState;
	int16 _scrollu_5;
	int16 _subPara;
	uint32 _scroll__DA;
	uint16 _scroll__DB;
	bool _needRefresh;

private:
	typedef Common::Functor1Mem<int, void, ScrollManager_SCD> ScrollFunc;
	Common::Array<ScrollFunc*> _scrollProcs;

	void makeFunctions();
	void doCommandIntern(int cmd, int arg);

	void scrUpdt_dummy(int arg);
	void scrUpdt_setHScrollFullScreen(int arg);
	void scrUpdt_03(int arg);
	void scrUpdt_04(int arg);
	void scrUpdt_05(int arg);
	void scrUpdt_06(int arg);
	void scrUpdt_07(int arg);
	void scrUpdt_08(int arg);
	void scrUpdt_13(int arg);
	void scrUpdt_14(int arg);
	void scrUpdt_15(int arg);
	void scrUpdt_16(int arg);
	void scrUpdt_17(int arg);
	void scrUpdt_18(int arg);
	void scrUpdt_19(int arg);
	void scrUpdt_20(int arg);
	void scrUpdt_21(int arg);
	void scrUpdt_22(int arg);
	void scrUpdt_23(int arg);
	void scrUpdt_24(int arg);
	void scrUpdt_25(int arg);
	void scrUpdt_26(int arg);
	void scrUpdt_27(int arg);
	void scrUpdt_28(int arg);
	void scrUpdt_29(int arg);
};


ScrollManager_SCD::ScrollManager_SCD(GraphicsEngine::GfxState &state) : _gfxState(state), _hScrollTable(nullptr), _hScrollTableLen(0), _internalState(nullptr), _scrollType(0), _hint_proc(0),
	_scrollCommandExt(0), _scrollCommand(0), _resetCommand(0), _nextStepExt(0), _nextStep(0), _lastStepExt(0), _lastStep(0), _scroll__DA(0), _scroll__DB(0), _scrollCmd_0xff_0xfd_0xfc_or0to6_last(0),
	_lastVScrollOffset(0), _scrollFlagttt(0), _scrollu_7(0), _transitionType(0), _transitionState(0), _scrollu_5(0), _subPara(0), _lastHScrollOffset(0), _needRefresh(false) {
	_internalState = new ScrollInternalState[4];
	assert(_internalState);
	_internalState[kVertA].setFactor(-1);
	_internalState[kVertB].setFactor(-1);
	_hScrollTable = new int16[0x200]();
	assert(_hScrollTable);
	makeFunctions();
}

ScrollManager_SCD::~ScrollManager_SCD() {
	delete[] _internalState;
	delete[] _hScrollTable;
	for (uint i = 0; i < _scrollProcs.size(); ++i) \
		delete _scrollProcs[i];
}

void ScrollManager_SCD::doCommand(int cmd) {
	_gfxState.setVar(3, 1);

	if (cmd == 0xFF || cmd == 0xFC) {
		clear();
		_resetCommand = cmd;
	} else {
		if (cmd >= 7 && cmd != 0xFD) {
			_scrollCommandExt = cmd;
		} else {
			_scrollCommand = cmd;
			_scrollType = (cmd < 3) ? cmd | ((cmd & 1) << 4) : 0;
		}
	}
}

void ScrollManager_SCD::clear() {
	for (int i = 0; i < 4; ++i) 
		_internalState[i].clear();
	memset(&_result, 0, sizeof(_result));
	_scrollType = 0;
	_scrollCommandExt = 0;
	_scrollCommand = 0;
	_resetCommand = 0;
}

bool ScrollManager_SCD::nextFrame() {
	if (_resetCommand == 0) {
		if (_scrollCommandExt)
			_nextStepExt = _scrollCommandExt;
		if (_scrollCommand)
			_nextStep = _scrollCommand;
		_scrollCommandExt = 0;
	} else {
		_nextStep = _resetCommand;
		_resetCommand = 0;
		_scrollCommandExt = 0;
		// _wordRAM__TABLE48__09 = 0;
		// _wordRAM__TABLE48__0A = 0;
	}

	if (_nextStepExt != _lastStepExt || _lastStepExt == 0) {
		if (_nextStepExt == 0xFE) {
			_scrollu_7 = 0;
			_transitionType = 0;
			_nextStepExt = 0;
			if (_lastStepExt == 23) {
				_lastVScrollOffset = 0;
				if (_scrollFlagttt) {
					_scrollFlagttt = 0;
					_nextStepExt = _lastStepExt = 15;
				}
			}
		} else if (_nextStepExt != 0 && (_nextStepExt != 15 || _lastStepExt != 23)) {
			_lastStepExt = _nextStepExt;
			_transitionState = _scrollu_5 = _subPara = 0;

		} else if (_nextStepExt != 0) {
			_nextStepExt = 23;
			_scrollFlagttt = 1;
		}
	}

	startTransition();

	bool changed = false;
	bool reset = _nextStep == 0xFF || _nextStep == 0xFC;
	bool needRefresh = processCmdInternal();

	if (needRefresh && _scrollType != 0 && !reset) {
		for (int i = 0; i < 4; ++i) {
			if (_internalState[i].recalc(_scrollType & 0x10))
				changed = true;
			_result.offsets[i] = _internalState[i].getOffset();
		}
		_result.hScrollTable = _hScrollTable;
		_result.hScrollTableNumEntries = _hScrollTableLen;
		_result.disableVScroll = (_scrollCmd_0xff_0xfd_0xfc_or0to6_last != 0);
		_scrollType &= ~0x10;
	}

	return changed || reset;
}

bool ScrollManager_SCD::processCmdInternal() {
	if ((_nextStep != _lastStep || _lastStep == 0) && _nextStep != 0) {
		if (_nextStep == 0xFF || _nextStep == 0xFC) {
			if (_nextStep == 0xFF)
				_result.hINTEnable = false;
			_nextStep = _lastStep = 0;
			_nextStepExt = 0;
			_transitionType = 0;
			_transitionState = _scrollu_5 = _subPara = _scrollu_7 = 0;
			_scrollFlagttt = 0;
			_scroll__DA = 0;
			_scroll__DB = 0;
			_lastHScrollOffset = 0;
			_lastVScrollOffset = 0;
			_needRefresh = false;
			_scrollCmd_0xff_0xfd_0xfc_or0to6_last = 0;
			_result.lineScrollMode = false;
			//_scrolloa = 0;
			//_scrollob = 0;
			return _needRefresh;
		} else if (_nextStep == 0xFD) {
			_scrollCmd_0xff_0xfd_0xfc_or0to6_last |= 0x80;
		} else {
			_lastStep = _nextStep;
			if (_nextStep < 3) {
				_needRefresh = false;
				//_scrolloa = 0;
				//_scrollob = 0;
			} else {
				_scroll__DA = 0;
				_scrollCmd_0xff_0xfd_0xfc_or0to6_last = 0;
			}
		}
	}

	bool b = false;
	if (_scrollCmd_0xff_0xfd_0xfc_or0to6_last != 0) {
		if (_scrollCmd_0xff_0xfd_0xfc_or0to6_last & 0x80)
			b = true;
		else
			doCommandIntern(_scrollCmd_0xff_0xfd_0xfc_or0to6_last, 0);
	}

	if (!b && _nextStep != 0)
		doCommandIntern(_nextStep, _subPara);

	doCommandIntern(_nextStepExt, _subPara);

	return _needRefresh;
}

void ScrollManager_SCD::startTransition() {
	if (_transitionType == 0)
		return;

	switch (_transitionType) {
	case 1:
		_result.hINTCounter = 15;
		//_result.hINTProc = (void *)_hint_proc7;
		if (--_transitionState <= 0)
			return;
		_result.offsets[kVertA] = _result.offsets[kVertB] = (_transitionState > 1) ? 0x30 : 0x10;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		_result.hINTCounter = 7;
		break;
	case 7:
	case 8:
		break;
	case 9:
		break;
	case 10:
	case 11:
		break;
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		break;
	case 17:
		break;
	case 18:
	case 19:
	case 20:
	case 21:
		break;
	case 22:
	case 23:
		break;
	default:
		break;
	}
	_result.hINTEnable = true;
}

#define SF(x) &ScrollManager_SCD::scrUpdt_##x
void ScrollManager_SCD::makeFunctions() {
	typedef void (ScrollManager_SCD::*ScrFunc)(int);
	static const ScrFunc funcTbl[] = {
		SF(dummy),
		SF(setHScrollFullScreen),
		SF(setHScrollFullScreen),
		SF(03),
		SF(04),
		SF(05),
		SF(06),
		SF(07),
		SF(08),
		SF(08),
		SF(08),
		SF(08),
		SF(08),
		SF(13),
		SF(14),
		SF(15),
		SF(16),
		SF(17),
		SF(18),
		SF(19),
		SF(20),
		SF(21),
		SF(22),
		SF(23),
		SF(24),
		SF(25),
		SF(26),
		SF(27),
		SF(28),
		SF(29)
	};

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i) \
		_scrollProcs.push_back(new ScrollFunc(this, funcTbl[i]));
}
#undef SF

void ScrollManager_SCD::doCommandIntern(int cmd, int arg) {
	if (_scrollProcs[cmd]->isValid())
		(*_scrollProcs[cmd])(arg);
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, cmd);
}

void ScrollManager_SCD::scrUpdt_dummy(int arg) {
}

void ScrollManager_SCD::scrUpdt_setHScrollFullScreen(int arg) {
	_result.lineScrollMode  = false;
	_needRefresh = true;
}

void ScrollManager_SCD::scrUpdt_03(int arg) {
}

void ScrollManager_SCD::scrUpdt_04(int arg) {
}

void ScrollManager_SCD::scrUpdt_05(int arg) {
}

void ScrollManager_SCD::scrUpdt_06(int arg) {
}

void ScrollManager_SCD::scrUpdt_07(int arg) {
	if (arg == 0) {
		_transitionType = 1;
		_hint_proc = 3278;
		_transitionState = 5;
		++_subPara;
		_nextStep = 0;
		_lastHScrollOffset = 0;
		_lastVScrollOffset = 0;
		_needRefresh = false;
		//_scrolloa = 0;
		//_scrollob = 0;
	} else if (arg == 1) {
		if (_transitionState < 0) {
			_nextStep = _lastStep = 0;
			_nextStepExt = 0;
			_transitionType = 0;
			_transitionState = _scrollu_5 = _subPara = _scrollu_7 = 0;
			_scrollFlagttt = 0;
			_scroll__DA = 0;
			_scroll__DB = 0;
			_lastHScrollOffset = 0;
			_lastVScrollOffset = 0;
			_needRefresh = false;
			_scrollCmd_0xff_0xfd_0xfc_or0to6_last = 0;
			_result.lineScrollMode = false;
			//_scrolloa = 0;
			//_scrollob = 0;
		}
	} else {
		error("%s(): Invalid arg %d", __FUNCTION__, arg);
	}
}

void ScrollManager_SCD::scrUpdt_08(int arg) {
}

void ScrollManager_SCD::scrUpdt_13(int arg) {
}

void ScrollManager_SCD::scrUpdt_14(int arg) {
}

void ScrollManager_SCD::scrUpdt_15(int arg) {
}

void ScrollManager_SCD::scrUpdt_16(int arg) {
}

void ScrollManager_SCD::scrUpdt_17(int arg) {
}

void ScrollManager_SCD::scrUpdt_18(int arg) {
}

void ScrollManager_SCD::scrUpdt_19(int arg) {
}

void ScrollManager_SCD::scrUpdt_20(int arg) {
}

void ScrollManager_SCD::scrUpdt_21(int arg) {
}

void ScrollManager_SCD::scrUpdt_22(int arg) {
}

void ScrollManager_SCD::scrUpdt_23(int arg) {
}

void ScrollManager_SCD::scrUpdt_24(int arg) {
}

void ScrollManager_SCD::scrUpdt_25(int arg) {
}

void ScrollManager_SCD::scrUpdt_26(int arg) {
}

void ScrollManager_SCD::scrUpdt_27(int arg) {
}

void ScrollManager_SCD::scrUpdt_28(int arg) {
}

void ScrollManager_SCD::scrUpdt_29(int arg) {
}

ScrollManager *ScrollManager::createSegaScrollManager(GraphicsEngine::GfxState &state) {
	return new ScrollManager_SCD(state);
}

} // End of namespace Snatcher
