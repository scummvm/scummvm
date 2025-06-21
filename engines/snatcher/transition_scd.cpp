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
#include "common/endian.h"
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
	uint8 _lineScrollOp;
	uint8 _nextStepExt;
	uint8 _lastStepExt;
	int16 _restoreDlgTab;
	int16 _hint_proc;
	int16 _transitionType;
	int16 _transitionState;
	int16 _transitionState2;
	int16 _transitionStep;
	int16 _tmpScrollOffset;
	int16 _subPara;
	int16 _lineScrollOpState;
	int16 _lineScrollTimer1;
	int16 _lineScrollTimer2;
	int16 _lineScrollTimer3;
	int16 _lineScrollTimer4;
	int16 _lineScrollTimer5;
	uint16 _trs__DB;
	bool _useEngineScrollOffsets;

private:
	typedef Common::Functor1Mem<int, void, TransitionManager_SCD> TrsFunc;
	Common::Array<TrsFunc*> _trsProcs;

	void makeFunctions();
	void doCommandIntern(int cmd, int arg);

	void trsUpdt_dummy(int arg);
	void trsUpdt_engineScroll(int arg);
	void trsUpdt_skyParallaxScroll(int arg);
	void trsUpdt_04(int arg);
	void trsUpdt_videoPhoneInterference(int arg);
	void trsUpdt_06(int arg);
	void trsUpdt_screenShutter(int arg);
	void trsUpdt_revealShutter(int arg);
	void trsUpdt_showStdVerbsTab(int arg);
	void trsUpdt_hideStdVerbsTab(int arg);
	void trsUpdt_showDialogueTab1(int arg);
	void trsUpdt_showVirtKeybTab(int arg);
	void trsUpdt_hideVirtKeybTab(int arg);
	void trsUpdt_showSmallVerbsTab(int arg);
	void trsUpdt_hideSmallVerbsTab(int arg);
	void trsUpdt_showDialogueTab2(int arg);
	void trsUpdt_showMonitorTextField(int arg);
	void trsUpdt_showDefaultTextScreen(int arg);
	void trsUpdt_videoPhoneCallStart(int arg);
	void trsUpdt_24(int arg);
	void trsUpdt_videoPhoneCallEnd1(int arg);
	void trsUpdt_videoPhoneCallEnd2(int arg);
	void trsUpdt_27(int arg);
	void trsUpdt_28(int arg);
	void trsUpdt_29(int arg);

	void lineScrollInit();
	void lineScrollTableFill(uint16 start, uint16 len, uint16 val);
	void lineScrollTableDecr(uint16 start, uint16 len);
	void textScreenTrsUpdate(int type, int hIntHandlerNo, int state);

	typedef Common::Functor1Mem<Graphics::SegaRenderer*, void, TransitionManager_SCD> HINTFunc;
	Common::Array<HINTFunc*> _hINTProcs;
	const HINTFunc *_hINTHandler;

	void setHINTHandler(uint8 num);

	void hIntHandler_dummy(Graphics::SegaRenderer*);
	void hIntHandler_showVirtKeybTabStep1(Graphics::SegaRenderer *sr);
	void hIntHandler_showVirtKeybTabStep2(Graphics::SegaRenderer *sr);
	void hIntHandler_hideVirtKeybTabStep1(Graphics::SegaRenderer *sr);
	void hIntHandler_hideVirtKeybTabStep2(Graphics::SegaRenderer *sr);
	void hIntHandler_screenShutter(Graphics::SegaRenderer *sr);
	void hIntHandler_revealShutter(Graphics::SegaRenderer *sr);
	void hIntHandler_toggleStdVerbsTab(Graphics::SegaRenderer *sr);
	void hIntHandler_showDialogueTab1(Graphics::SegaRenderer *sr);
	void hIntHandler_toggleSmallVerbsTab(Graphics::SegaRenderer *sr);
	void hIntHandler_showDialogueTab2(Graphics::SegaRenderer *sr);
	void hIntHandler_showMonitorTextField(Graphics::SegaRenderer *sr);
	void hIntHandler_showDefaultTextScreen(Graphics::SegaRenderer *sr);
	void hIntHandler_videoPhoneCallStart(Graphics::SegaRenderer *sr);
	void hIntHandler_videoPhoneCallEnd(Graphics::SegaRenderer *sr);
};

TransitionManager_SCD::TransitionManager_SCD(GraphicsEngine::GfxState &state) : _gfxState(state), _hScrollTable(nullptr), _hScrollTableLen(0), _internalState(nullptr), _scrollType(0), _hint_proc(0), _transitionStep(0),
	_trsCommandExt(0), _trsCommand(0), _resetCommand(0), _nextStepExt(0), _nextStep(0), _lastStepExt(0), _lastStep(0), _lineScrollOpState(0), _lineScrollTimer1(0), _lineScrollTimer2(0), _lineScrollTimer3(0), _lineScrollTimer4(0),
		_lineScrollTimer5(0), _trs__DB(0), _lineScrollOp(0), _restoreDlgTab(0), _transitionType(0), _transitionState(0), _transitionState2(0), _tmpScrollOffset(0), _subPara(0), _useEngineScrollOffsets(false), _hINTHandler(nullptr) {
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
	}

	if (_nextStepExt != _lastStepExt || _lastStepExt == 0) {
		if (_nextStepExt == 0xFE) {
			if (_lastStepExt == 23) {
				_internalState[kVertA].setNextOffset(0);
				_internalState[kVertB].setNextOffset(0);
				if (_restoreDlgTab) {
					resetVars(0x30);
					_nextStepExt = _lastStepExt = 15;
				} else {
					resetVars(0x18);
				}
			} else {
				resetVars(0x18);
			}
		} else if (_nextStepExt == 15 && _lastStepExt == 23) {
			_nextStepExt = 23;
			_restoreDlgTab = 1;
		} else if (_nextStepExt != 0) {
			_lastStepExt = _nextStepExt;
			resetVars(0x40);
		}
	}

	processTransition();

	bool changed = false;
	bool reset = (_nextStep == 0xFF || _nextStep == 0xFC);

	if (_scrollType && !reset) {
		for (int i = 0; i < 4; ++i) {
			if (_internalState[i].recalc(_scrollType & 0x10))
				changed = true;
			_result.realOffsets[i] = _internalState[i].getRealOffset(!(changed || _useEngineScrollOffsets));
			_result.unmodifiedOffsets[i] = _internalState[i].getUnmodifiedOffset();
			_internalState[i].setNextOffset(_result.realOffsets[i]);
		}
		_scrollType &= ~0x10;
	}

	processCmdInternal();
	_result.busy = _lastStepExt;

	if (_result.hScrollTable != _hScrollTable || _result.hScrollTableNumEntries != _hScrollTableLen) {
		_result.hScrollTable = _hScrollTable;
		_result.hScrollTableNumEntries = _hScrollTableLen;
		_result.disableVScroll = (_lineScrollOp != 0);
	}

	return changed || reset || _lastStep || _lastStepExt || _result.hInt.needUpdate;
}

void TransitionManager_SCD::hINTCallback(void *segaRenderer) {
	if (_result.hInt.enable && _hINTHandler && _hINTHandler->isValid())
		(*_hINTHandler)(static_cast<Graphics::SegaRenderer *>(segaRenderer));
}

void TransitionManager_SCD::processCmdInternal() {
	if ((_nextStep != _lastStep || _lastStep == 0) && _nextStep != 0) {
		if (_nextStep == 0xFF || _nextStep == 0xFC) {
			resetVars(_nextStep == 0xFF ? 0x0F : 0x0E);
			return;
		} else if (_nextStep == 0xFD) {
			_lineScrollOp |= 0x80;
		} else {
			_lastStep = _nextStep;
			if (_nextStep < 3) {
				resetVars(0x08);
			} else {
				_lineScrollOpState = 0;
				_lineScrollOp = 0;
			}
		}
	}

	bool b = false;
	if (_lineScrollOp != 0) {
		if (_lineScrollOp & 0x80)
			b = true;
		else
			doCommandIntern(_lineScrollOp, 0);
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
			_result.realOffsets[kVertA] = 0;
			_result.realOffsets[kVertB] = 0;
			_internalState[kVertA].setNextOffset(0);
			_internalState[kVertB].setNextOffset(0);
			return;
		}
		_result.realOffsets[kVertA] = _transitionState > 1 ? 0x30 : 0x10;
		_result.realOffsets[kVertB] = _transitionState > 1 ? 0x30 : 0x10;
		break;
	case 2: case 3: case 4: case 5: case 6:
		_result.hInt.counter = 7;
		break;
	case 7: case 8:
		_result.hInt.counter = 0;
		break;
	case 9:
		_result.hInt.counter = 7;
		break;
	case 10: case 11:
		_result.hInt.counter = 0;
		break;
	case 12: case 13: case 14: case 15: case 16:
		_result.hInt.counter = 7;
		break;
	case 17:
		_result.hInt.counter = 7;
		_result.realOffsets[kVertA] = 0;
		_result.realOffsets[kVertB] = _transitionState2;
		break;
	case 18: case 19: case 20: case 21:
		_result.hInt.counter = 7;
		_result.realOffsets[kVertA] = 0;
		_result.realOffsets[kVertB] = _transitionState;
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
		_lineScrollOp = 0;
		_lineScrollOpState = 0;
		_trs__DB = 0;
		_nextStepExt = 0;
		_transitionType = _transitionState = _tmpScrollOffset = _subPara = 0;
		_result.verbsTabVisible = false;
		_restoreDlgTab = 0;

	}
	if (groupFlags & 0x04) {
		_nextStep = _lastStep = 0;
		_result.lineScrollMode = false;
		for (int i = 0; i < 4; ++i) {
			if (_scrollType == 0)
				_result.realOffsets[i] = 0;
			else
				_internalState[i].setNextOffset(0);
		}
	}
	if (groupFlags & 0x08) {
		_useEngineScrollOffsets = false;
		//_trsoa = 0;
		//_trsob = 0;
	}
	if (groupFlags & 0x10) {
		_nextStepExt = _lastStepExt = 0;
		_transitionType = 0;
		_result.verbsTabVisible = false;
	}
	if (groupFlags & 0x20) {
		_restoreDlgTab = 0;
		if (_scrollType == 0) {
			_result.realOffsets[kVertA] = _result.realOffsets[kVertB] = 0;
		} else {
			_internalState[kVertA].setNextOffset(0);
			_internalState[kVertB].setNextOffset(0);
		}
	}
	if (groupFlags & 0x40) {
		_transitionState = _tmpScrollOffset = _subPara = 0;
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
		TF(skyParallaxScroll),
		TF(04),
		TF(videoPhoneInterference),
		TF(06),
		TF(screenShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(revealShutter),
		TF(showStdVerbsTab),
		TF(hideStdVerbsTab),
		TF(showDialogueTab1),
		TF(showVirtKeybTab),
		TF(hideVirtKeybTab),
		TF(showSmallVerbsTab),
		TF(hideSmallVerbsTab),
		TF(showDialogueTab2),
		TF(showMonitorTextField),
		TF(showDefaultTextScreen),
		TF(videoPhoneCallStart),
		TF(24),
		TF(videoPhoneCallEnd1),
		TF(videoPhoneCallEnd2),
		TF(27),
		TF(28),
		TF(29)
	};

	for (uint i = 0; i < ARRAYSIZE(funcTbl); ++i) \
		_trsProcs.push_back(new TrsFunc(this, funcTbl[i]));

	typedef void (TransitionManager_SCD::*HIFunc)(Graphics::SegaRenderer*);
	static const HIFunc funcTbl2[] = {
		HF(showVirtKeybTabStep1),
		HF(showVirtKeybTabStep2),
		HF(hideVirtKeybTabStep1),
		HF(hideVirtKeybTabStep2),
		HF(dummy),
		HF(dummy),
		HF(dummy),
		HF(screenShutter),
		HF(revealShutter),
		HF(dummy),
		HF(dummy),
		HF(dummy),
		HF(dummy),
		HF(toggleStdVerbsTab),
		HF(dummy),
		HF(showDialogueTab1),
		HF(dummy),
		HF(dummy),
		HF(toggleSmallVerbsTab),
		HF(dummy),
		HF(showDialogueTab2),
		HF(showMonitorTextField),
		HF(showDefaultTextScreen),
		HF(videoPhoneCallStart),
		HF(videoPhoneCallEnd)
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
	_useEngineScrollOffsets = true;
}

void TransitionManager_SCD::trsUpdt_skyParallaxScroll(int arg) {
	if (!_lineScrollOp) {
		lineScrollInit();
		return;
	}

	++_lineScrollTimer1;
	++_lineScrollTimer2;
	++_lineScrollTimer3;
	++_lineScrollTimer4;
	++_lineScrollTimer5;

	uint16 f = _gfxState.frameCount();

	if (!(f & 1)) {
		lineScrollTableDecr(0, 8);
		_hScrollTableLen = 8;
	}
	if (!(f & 3)) {
		lineScrollTableDecr(8, 9);
		_hScrollTableLen = 17;
	}
	if (_lineScrollTimer1 == 6) {
		_lineScrollTimer1 = 0;
		lineScrollTableDecr(17, 12);
		_hScrollTableLen = 29;
	}
	if (!(f & 7)) {
		lineScrollTableDecr(29, 10);
		_hScrollTableLen = 39;
	}
	if (_lineScrollTimer2 == 10) {
		_lineScrollTimer2 = 0;
		lineScrollTableDecr(39, 8);
		_hScrollTableLen = 47;
	}
	if (_lineScrollTimer3 == 12) {
		_lineScrollTimer3 = 0;
		lineScrollTableDecr(47, 6);
		_hScrollTableLen += 53;
	}
	if (_lineScrollTimer4 == 14) {
		_lineScrollTimer4 = 0;
		lineScrollTableDecr(53, 5);
		_hScrollTableLen = 58;
	}
	if (!(f & 0x0F)) {
		lineScrollTableDecr(58, 4);
		_hScrollTableLen = 62;
	}
	if (_lineScrollTimer5 == 18) {
		_lineScrollTimer5 = 0;
		lineScrollTableDecr(62, 3);
		_hScrollTableLen = 65;
	}
}

void TransitionManager_SCD::trsUpdt_04(int arg) {
}

void TransitionManager_SCD::trsUpdt_videoPhoneInterference(int arg) {
	switch (_lineScrollOpState) {
		case 0:
			++_lineScrollOpState;
			lineScrollInit();
			break;
		case 1:
			++_lineScrollOpState;
			_hScrollTableLen = 112;
			lineScrollTableFill(48, 3, 0x3F8);
			lineScrollTableFill(51, 2, 0x3F4);
			lineScrollTableFill(53, 3, 0x3F8);
			lineScrollTableFill(72, 24, 2);
			lineScrollTableFill(96, 16, 4);
			break;
		case 2:
			if (++_lineScrollTimer1 < 8)
				return;
			_result.lineScrollMode = false;
			++_lineScrollOpState;
			break;
		case 3:
			if (++_lineScrollTimer2 < 13)
				return;
			_result.lineScrollMode = true;
			++_lineScrollOpState;
			break;
		case 4:
			if (++_lineScrollTimer3 == 7)
				resetVars(0x0F);
			break;
		default:
			error("%s(): Invalid state %d", __FUNCTION__, _lineScrollOpState);
			break;
	}
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

void TransitionManager_SCD::trsUpdt_showStdVerbsTab(int arg) {
	if (arg == 0) {
		_transitionType = 7;
		setHINTHandler(13);
		_transitionState = 252;
		_tmpScrollOffset = 144;
		++_subPara;
	} else if (arg == 1) {
		_transitionState -= 19;
		_tmpScrollOffset += 19;
		if (_tmpScrollOffset > 255) {
			_transitionState = 136;
			_tmpScrollOffset = 256;
			_result.verbsTabVisible = true;
			++_subPara;
		}
	}
}

void TransitionManager_SCD::trsUpdt_hideStdVerbsTab(int arg) {
	if (arg == 0) {
		_transitionType = 8;
		setHINTHandler(13);
		_transitionState = 140;
		_tmpScrollOffset = 256;
		++_subPara;
	} else if (arg == 1) {
		_transitionState += 19;
		_tmpScrollOffset -= 19;
		if (_tmpScrollOffset < 144) {
			_tmpScrollOffset = 144;
			++_subPara;
		}
	} else {
		resetVars(0x18);
	}
}

void TransitionManager_SCD::trsUpdt_showDialogueTab1(int arg) {
	if (arg == 0)
		textScreenTrsUpdate(9, 15, 256);
}

void TransitionManager_SCD::trsUpdt_showVirtKeybTab(int arg) {
	switch (arg) {
	case 0:
		_transitionType = 10;
		setHINTHandler(0);
		_transitionState = 100;
		++_subPara;
		break;
	case 1:
		_transitionState += 3;
		if (_transitionState < 117)
			return;
		_transitionState = 117;
		setHINTHandler(1);
		++_subPara;
		break;
	case 2:
		_transitionState = 0;
		++_subPara;
		break;
	case 3:
		_transitionState += 10;
		if (_transitionState < 112)
			return;
		_transitionState = 112;
		++_subPara;
		break;
	case 4:
		_result.verbsTabVisible = true;
		++_subPara;
		break;
	default:
		break;
	}
}

void TransitionManager_SCD::trsUpdt_hideVirtKeybTab(int arg) {
	switch (arg) {
	case 0:
		_transitionType = 11;
		setHINTHandler(2);
		_transitionState = 112;
		_transitionState2 = 117;
		_tmpScrollOffset = 320;
		++_subPara;
		break;
	case 1:
		_transitionState2 -= 3;
		_tmpScrollOffset += 3;
		if (_transitionState2 < 101) {
			_transitionState2 = 101;
			++_subPara;
		}
		break;
	case 2:
		_transitionState = 112;
		++_subPara;
		break;
	case 3:
		setHINTHandler(3);
		++_subPara;
		break;
	case 4:
		_transitionState -= 10;
		if (_transitionState < 0) {
			_transitionState = 0;
			++_subPara;
		}
		break;
	default:
		resetVars(0x18);
		break;
	}
}

void TransitionManager_SCD::trsUpdt_showSmallVerbsTab(int arg) {
	if (arg == 0) {
		_transitionType = 12;
		setHINTHandler(18);
		_transitionState = 144;
		++_subPara;
	} else if (arg == 1) {
		_transitionState += 19;
		if (_transitionState > 239) {
			_transitionState = 240;
			_result.verbsTabVisible = true;
			++_subPara;
		}
	}
}

void TransitionManager_SCD::trsUpdt_hideSmallVerbsTab(int arg) {
	if (arg == 0) {
		_transitionType = 13;
		setHINTHandler(18);
		_transitionState = 240;
		++_subPara;
	} else if (arg == 1) {
		_transitionState -= 19;
		if (_transitionState < 144) {
			_transitionState = 144;
			++_subPara;
		}
	} else {
		resetVars(0x18);
	}
}

void TransitionManager_SCD::trsUpdt_showDialogueTab2(int arg) {
	if (arg == 0)
		textScreenTrsUpdate(14, 20, 240);
}

void TransitionManager_SCD::trsUpdt_showMonitorTextField(int arg) {
	if (arg == 0)
		textScreenTrsUpdate(15, 21, 328);
}

void TransitionManager_SCD::trsUpdt_showDefaultTextScreen(int arg) {
	if (arg == 0)
		textScreenTrsUpdate(16, 22, 232);
}

void TransitionManager_SCD::trsUpdt_videoPhoneCallStart(int arg) {
	if (_tmpScrollOffset == 0) {
		_transitionState2 = 48;
		++_tmpScrollOffset;
	} else {
		if (_tmpScrollOffset == 1) {
			_transitionState2 += 8;
			if (_transitionState2 > 96) {
				_transitionState2 = 96;
				++_tmpScrollOffset;
			}
		}
		_transitionType = 17;
		setHINTHandler(23);
	}
}

void TransitionManager_SCD::trsUpdt_24(int arg) {
}

void TransitionManager_SCD::trsUpdt_videoPhoneCallEnd1(int arg) {
	if (arg != 0)
		return;
	_transitionType = 19;
	setHINTHandler(24);
	_transitionState = 96;
	++_subPara;
}

void TransitionManager_SCD::trsUpdt_videoPhoneCallEnd2(int arg) {
	if (arg == 0) {
		_transitionType = 20;
		setHINTHandler(24);
		_transitionState = 96;
		++_subPara;
	} else if (arg == 1) {
		_transitionState -= 8;
		if (_transitionState < 48) {
			_transitionState = 48;
			++_subPara;
		}
	} else {
		resetVars(0x30);
	}
}

void TransitionManager_SCD::trsUpdt_27(int arg) {
}

void TransitionManager_SCD::trsUpdt_28(int arg) {
}

void TransitionManager_SCD::trsUpdt_29(int arg) {
}

void TransitionManager_SCD::lineScrollInit() {
	_lineScrollOp = _nextStep;
	_nextStep = 0;
	_hScrollTableLen = 128;
	_useEngineScrollOffsets = false;
	_lineScrollTimer1 = _lineScrollTimer3 = _lineScrollTimer5 = 0;
	Common::fill<int16*>(_hScrollTable, &_hScrollTable[0x100], 0);
	_result.lineScrollMode = true;
}

void TransitionManager_SCD::lineScrollTableFill(uint16 start, uint16 len, uint16 val) {
	Common::fill<int16*>(&_hScrollTable[start], &_hScrollTable[start + len], val);
}

void TransitionManager_SCD::lineScrollTableDecr(uint16 start, uint16 len) {
	for (int16 *d = &_hScrollTable[start]; d < &_hScrollTable[start + len]; ++d)
		*d = (*d - 1) & 0x3FF;
}

void TransitionManager_SCD::textScreenTrsUpdate(int type, int hIntHandlerNo, int state) {
	_transitionType = type;
	setHINTHandler(hIntHandlerNo);
	_transitionState = state;
	++_subPara;
}

void TransitionManager_SCD::setHINTHandler(uint8 num) {
	if (num < _hINTProcs.size())
		_hINTHandler = _hINTProcs[num];
	 else
		error("%s(): Invalid HINT handler %d", __FUNCTION__, num);
}

void TransitionManager_SCD::hIntHandler_dummy(Graphics::SegaRenderer*) {
}

void TransitionManager_SCD::hIntHandler_showVirtKeybTabStep1(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 99) {
		sr->writeUint16VSRAM(0, TO_BE_16(0x140));
		sr->writeUint16VSRAM(2, TO_BE_16(0x140));
	} else if (_transitionStep == _transitionState) {
		sr->writeUint16VSRAM(0, TO_BE_16(_result.realOffsets[kVertA]));
		sr->writeUint16VSRAM(2, TO_BE_16(_result.realOffsets[kVertB]));
		_result.hInt.enable = false;
	}
	++_transitionStep;
}

void TransitionManager_SCD::hIntHandler_showVirtKeybTabStep2(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 99) {
		sr->writeUint16VSRAM(0, TO_BE_16(0x140));
		sr->writeUint16VSRAM(2, TO_BE_16(0x140));
	} else if (_transitionStep == 117) {
		sr->writeUint16VSRAM(0, TO_BE_16(_result.realOffsets[kVertA]));
		sr->writeUint16VSRAM(2, TO_BE_16(_result.realOffsets[kVertB]));
	} else if (_transitionStep == 143) {
		sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
		sr->writeUint16VSRAM(2, TO_BE_16(_transitionState));
		_result.hInt.enable = false;
	}
	++_transitionStep;
}

void TransitionManager_SCD::hIntHandler_hideVirtKeybTabStep1(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 99) {
		sr->writeUint16VSRAM(0, TO_BE_16(_tmpScrollOffset));
		sr->writeUint16VSRAM(2, TO_BE_16(_tmpScrollOffset));
	} else if (_transitionStep == 143) {
		sr->writeUint16VSRAM(0, TO_BE_16(0x70));
		sr->writeUint16VSRAM(2, TO_BE_16(0x70));
		_result.hInt.enable = false;
	} else if (_transitionStep == _transitionState2) {
		sr->writeUint16VSRAM(0, TO_BE_16(_result.realOffsets[kVertA]));
		sr->writeUint16VSRAM(2, TO_BE_16(_result.realOffsets[kVertB]));
	}
	++_transitionStep;
}

void TransitionManager_SCD::hIntHandler_hideVirtKeybTabStep2(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 143) {
		sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
		sr->writeUint16VSRAM(2, TO_BE_16(_transitionState));
		_result.hInt.enable = false;
	}
	++_transitionStep;
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

void TransitionManager_SCD::hIntHandler_toggleStdVerbsTab(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != _transitionState)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_tmpScrollOffset));
	sr->writeUint16VSRAM(2, TO_BE_16(_tmpScrollOffset));
	sr->writeUint16VRAM(0xF800, 0);
	sr->writeUint16VRAM(0xF802, 0);
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_showDialogueTab1(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 17)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
	sr->writeUint16VSRAM(2, TO_BE_16(_transitionState));
	sr->writeUint16VRAM(0xF800, 0);
	sr->writeUint16VRAM(0xF802, 0);
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_toggleSmallVerbsTab(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 19)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
	sr->writeUint16VSRAM(2, TO_BE_16(_transitionState));
	sr->writeUint16VRAM(0xF800, 0);
	sr->writeUint16VRAM(0xF802, 0);
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_showDialogueTab2(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 19)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_showMonitorTextField(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 8)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_showDefaultTextScreen(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 20)
		return;
	sr->writeUint16VSRAM(0, TO_BE_16(_transitionState));
	_result.hInt.enable = false;
}

void TransitionManager_SCD::hIntHandler_videoPhoneCallStart(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 9) {
		sr->writeUint16VSRAM(0, 0);
		sr->writeUint16VSRAM(2, TO_BE_16((-_transitionState2) & 0x3FF));
	} else if (_transitionStep == 17) {
		sr->writeUint16VSRAM(0, TO_BE_16(_result.realOffsets[kVertA]));
		sr->writeUint16VSRAM(2, TO_BE_16(_result.realOffsets[kVertB]));
		_result.hInt.enable = false;
	}
	++_transitionStep;
}

void TransitionManager_SCD::hIntHandler_videoPhoneCallEnd(Graphics::SegaRenderer *sr) {
	if (_transitionStep == 9) {
		sr->writeUint16VSRAM(0, 0);
		sr->writeUint16VSRAM(2, TO_BE_16((-_transitionState) & 0x3FF));
		_result.hInt.enable = false;
	}
	++_transitionStep;
}

TransitionManager *TransitionManager::createSegaTransitionManager(GraphicsEngine::GfxState &state) {
	return new TransitionManager_SCD(state);
}

} // End of namespace Snatcher
