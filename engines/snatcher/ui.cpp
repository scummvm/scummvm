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
#include "snatcher/resource.h"
#include "snatcher/script.h"
#include "snatcher/snatcher.h"
#include "snatcher/ui.h"
#include "snatcher/util.h"
#include "common/stream.h"


namespace Snatcher {

UI::UI(GraphicsEngine *gfx, CmdQueue *que, ResourcePointer *scd) : _gfx(gfx), _que(que), _scd(scd), _sceneId(0), _textLineBreak(0), _dialogTextBuffer(nullptr), _textColor(0),
	_scriptTextResource(nullptr),/*_makestrbt1(0),*/ _textY(0), _headLineYOffset(0), _sceneTextOffsCur(0), _textLineEnd(0), _sceneInfo(0), _sceneTextOffset(0), _flashLight(),
		_sceneTextOffsStart(0), _waitCursorFrame(0), _waitCursorAnimDelay(0), _progress(-1), _progress2(-1), _controllerCfg(0), _verbInterfaceMode(0), _verbsInterpreterMode(0),
			_lastVerbFirstPage(0), _numVerbsFirstPage(0), _numVerbsLastPage(0), _lastVerbLastPage(0), _numVerbsMax(0), _verbsTabCurPage(0), _lastVerbDrawn(0), _verbTextBuffer(nullptr), _scriptVerbsArray(nullptr),
				_scriptSentenceArray(nullptr), _prevSelectedVerb(0), _selectedVerb(0), _lastHiliteVerb(0), _hiliteVerb(0), _textInputMarginLeft(0), _videoPhoneMode(0),
					_textInputColumnCur(0), _textInputColumnMax(0), _textInputCursorState(0), _textInputCursorBlinkCnt(0), _underscoreStr(nullptr), _textInputStr(nullptr), _vkeybPosTotal(0),
						_vkeybColumnWidth(0), _verbsTabLayoutMap(nullptr), _verbsTabOffsX(0), _verbsTabOffsY(0), _hilitePosX(0), _hilitePosY(0), _vkeybVisible(false) {

	_dialogTextBuffer = new uint8[256]();
	_verbTextBuffer = new uint8[50]();
	_underscoreStr = new uint8[12]();
	_textInputStr = new uint8[30]();
}

UI::~UI() {
	delete[] _dialogTextBuffer;
	delete[] _verbTextBuffer;
	delete[] _underscoreStr;
	delete[] _textInputStr;
}

bool UI::displayDialog(int sceneInfo, int sceneTextOffset, uint16 inputFlags) {
	if (_progress == -1)
		_progress = 0;

	if (inputFlags & 0x100)
		_gfx->setTextPrintDelay(0);

	if (_gfx->isTextInQueue())
		return false;

	switch (_progress) {
	case 0:
		for (int i = 5; i < 15; ++i)
			_gfx->setAnimParameter(i, GraphicsEngine::kAnimParaEnable, 0);
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaEnable, 0);
		_gfx->enqueuePaletteEvent(_scd->makePtr(0x13622));
		++_progress;
		break;

	case 1:
		_gfx->setVar(11, 0xFF);
		_progress = 9;
		break;

	case 2:
		if (_sceneId == 0) {
			_gfx->transitionCommand(21);
			//_c13Valu = 0xFF;
			_textY = _headLineYOffset;
		} else if (_sceneId == 0xFF) {
			_textY = _headLineYOffset;
		} else {
			printDialogStringHead();
		}
		++_progress;
		break;

	case 3:
		printDialogStringBody();
		if (checkStringPrintProgress())
			++_progress;
		break;

	case 4:
		_waitCursorFrame = 0;
		_waitCursorAnimDelay = 1;
		if (!_sceneId)
			_gfx->setAnimParameter(31, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide);
		++_progress;
		break;

	case 5:
		if (!waitWithCursorAnim(inputFlags))
			return false;
		_gfx->transitionCommand(0xFE);
		if (_textLineEnd) {
			_progress = 6;
			if (!_sceneId)
				_gfx->setAnimParameter(31, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		} else {
			_progress = 7;
		}
		break;

	case 6:
		_sceneTextOffset = _sceneInfo = 0;
		//if (!_sceneId)
		//	_c13Valu = 0xFF;
		_progress = -1;
		break;

	case 7:
		_gfx->resetTextFields();
		++_progress;
		break;

	case 8:
		_gfx->transitionCommand(_gfx->getVerbAreaType() == 0 ? 15 : (_gfx->getVerbAreaType() == 1 ? 20 : 22));
		_progress = 2;
		break;

	case 9:
		_sceneInfo = sceneInfo;
		_sceneTextOffset = sceneTextOffset;
		_gfx->enqueueDrawCommands(_scd->makePtr(0xECB6));
		++_progress;
		break;

	case 10:
		_sceneTextOffsStart = _sceneTextOffset;
		_gfx->resetTextFields();
		_sceneId = (_sceneInfo & 0xFF) - 1;

		_sceneTextOffsCur = _sceneTextOffsStart;
		_textLineEnd = 0;
		//_gfxOps10_sub_974B = _gfxOps10_sub_974E = 0;
		//_gfxOps10_subword_974C = 0;
		_textColor = 1;

		_gfx->transitionCommand(_gfx->getVerbAreaType() == 0 ? 15 : (_gfx->getVerbAreaType() == 1 ? 20 : 22));
		//_c13Valu = 0;
		_progress = 2;
		break;

	default:
		error("%s(): Unknown state %d", __FUNCTION__, _progress);
		break;
	}

	return (_progress == -1);
}

bool UI::drawVerbs() {
	if (_progress2 == -1)
		_progress2 = 0;

	if (_verbsInterpreterMode == 2)
		_progress2 |= 0x80;

	if (_progress2 == 0) {
		if (_gfx->getVerbAreaType()) {
			_lastVerbFirstPage = 5;
			_numVerbsFirstPage = 4;
			_numVerbsLastPage = 3;
			_lastVerbLastPage = 4;
			_numVerbsMax = 6;
		} else {
			_lastVerbFirstPage = 6;
			_numVerbsFirstPage = 5;
			_numVerbsLastPage = 2;
			_lastVerbLastPage = 3;
			_numVerbsMax = 7;
		}

		if (_scriptVerbsArray->pos() >= 9) {
			if (_scriptTextResource[_scriptVerbsArray->read(0)] < 'A') {
				_verbInterfaceMode = 2;
				_que->writeUInt16(0x22);
				_que->writeUInt32(0x1138C);
			} else {
				_verbInterfaceMode = 3;
				_que->writeUInt16(0x22);
				_que->writeUInt32(0x11382);
			}
			_que->start();
			_progress2 = 1;
			return false;
		}
	}

	if (_progress2 <= 1) {
		_que->writeUInt16(0x16);
		_que->start();
		_progress2 = 2;
		return false;
	}

	if (_verbInterfaceMode < 2) {
		if (_progress2 == 2) {
			static const uint8 strHead[] = { 0xFC, 0x00, 0xF9, 0x01, 0xFE, 0x04, 0xFB };
			memcpy(_verbTextBuffer, strHead, sizeof(strHead));
			_verbTextBuffer[41] = 0xFF;
			_headLineYOffset = 0;
			_verbsTabCurPage = 0;
			bool sp7 = (_scriptVerbsArray->pos() == 7 && _gfx->getVerbAreaType() != 0);
			if (sp7) {
				_numVerbsLastPage = 2;
				_lastVerbLastPage = 3;
			}
			if (_scriptVerbsArray->pos() == 8 || sp7) {
				_verbsTabCurPage = 1;
				if (_scriptSentenceArray->pos() == 0) {
					if (_lastVerbFirstPage <= _prevSelectedVerb) {
						_verbsTabCurPage = 2;
						_prevSelectedVerb -= _numVerbsFirstPage;
					}
				}
			}
			_progress2 = 3;
		}

		if (_progress2 >= 3 && _progress2 < 97) {
			uint8 res = (_progress2 - 3 < _numVerbsMax) ? drawVerb(_progress2 - 3) : 0xFF;
			if (res == 0xFF) {
				_lastVerbDrawn = _progress2 - 3;
				_progress2 = 97;
			} else if (res != 0x80) {
				_verbTextBuffer[7] = 0x10;
				_verbTextBuffer[8] = res * 9;
				_gfx->printText(_verbTextBuffer);
			} else {
				_que->start();
			}
			if (_progress2 < 97) {
				++_progress2;
				return false;
			}
		}

		if (_progress2 == 97) {
			if (_verbsTabCurPage == 2)
				_lastVerbDrawn = _numVerbsMax;

			_selectedVerb = 0;
			uint8 v = 0;
			if (_scriptSentenceArray->pos() == 0) {
				if (_prevSelectedVerb <= _lastVerbDrawn - 1)
					v = _prevSelectedVerb;
			}
			_selectedVerb = 0;
			_hiliteVerb = _lastHiliteVerb = v;
			++_progress2;
		}

		if (_progress2 == 98) {
			drawHiliteVerb(0);
			++_progress2;
			return false;
		}

		if (_progress2 == 99) {
			_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaPosX, 16);
			drawHiliteVerb(1);
			++_progress2;
			return false;
		}
	} else {
		if (_progress2 == 2) {
			if (_verbInterfaceMode == 2) {
				_que->writeUInt16(0x05);
				_que->writeUInt32(0x11454);
				_que->start();
			}
			++_progress2;
			return false;
		}
		if (_progress2 == 3) {
			if (_verbInterfaceMode == 2) {
				_gfx->transitionCommand(15);
			} else {
				_gfx->transitionCommand(16);
				_vkeybVisible = true;
			}
			++_progress2;
			return false;
		}
		if (_progress2 == 4) {
			_textInputMarginLeft = 64;
			_textInputColumnCur = 0;
			if (_textInputColumnMax == 0) {
				_textInputColumnMax = (_verbInterfaceMode == 2) ? 10 : 17;
				_textInputMarginLeft = (_verbInterfaceMode == 2) ? 24 : 48;
				_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaFrame, _verbInterfaceMode == 2 ? 2 : 0);
			}
			++_progress2;
		}
		if (_progress2 == 5) {
			if (drawUnderscoreString())
				++_progress2;
			return false;
		}
		if (_progress2 == 6) {
			_textInputCursorState = 0;
			_textInputColumnCur = 0;
			drawUnderscoreCursor();
			++_progress2;
			return false;
		}
		if (_progress2 == 7) {
			const uint8 *src = _scd->makePtr(0x11D12)();
			uint8 *dst = _dialogTextBuffer;
			for (int i = 0; i < 256; ++i) {
				*dst = *src++;
				if (*dst++ == 0xFF)
					break;
			}
			_dialogTextBuffer[5] = _dialogTextBuffer[46] = _textInputMarginLeft;
			_dialogTextBuffer[6] = _dialogTextBuffer[47] = 24;
			memset(_textInputStr, 0, 30);
			_selectedVerb = 0;

			static const uint8 offs[] = { 0x2D, 0x06, 0x1B, 0x1A, 0xC0, 0x87, 0x10, 0x90 };

			_vkeybPosTotal = offs[((_verbInterfaceMode - 2) << 1)];
			_vkeybColumnWidth = offs[((_verbInterfaceMode - 2) << 1) + 1];
			_verbsTabOffsX = offs[((_verbInterfaceMode - 2) << 1) + 4];
			_verbsTabOffsY = offs[((_verbInterfaceMode - 2) << 1) + 5];
			adjustHilitePos();

			_progress2 = 100;
			return false;
		}
	}

	return true;
}

bool UI::verbsTabInputPrompt(const SnatcherEngine::Input &input) {
	assert (_progress2 >= 100);
	if (_verbsInterpreterMode == 2) {
		if (_progress2 & 0x80) {
			verbSelect2setResult();
			_progress2 &= ~0x80;
			if (_progress2 < 104)
				_progress2 = 104;
		}
	} else {
		if (_progress2 == 100) {
			if (_verbInterfaceMode < 2)
				_gfx->transitionCommand(_gfx->getVerbAreaType() ? 18 : 13);
			++_progress2;
		}

		if (_progress2 == 101) {
			if (_verbInterfaceMode < 2 && !_gfx->isVerbsTabActive())
				return false;
			++_progress2;
		}

		if (_progress2 == 102) {
			_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaEnable, 1);
			if (_verbInterfaceMode >= 2) {
				_verbsTabLayoutMap = _scd->makePtr(_verbInterfaceMode == 3 ? 0x11512 : 0x114DC)();
				_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
				_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
				_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaFrame, _verbInterfaceMode == 2 ? 2 : 0);
			} else {
				_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, 0);
			}
			++_progress2;
			return false;
		}

		if ((_progress2 & 0xFF) == 103) {
			verbsTabHandleInput(input);
			if (_selectedVerb == 0)
				return false;
			++_progress2;
		}

		if (_progress2 & 0x100) {
			if (verbSelect2())
				virtKeybPrint();
			else if (_selectedVerb != 0)
				++_progress2;
			return false;
		}

		if (_progress2 & 0x200) {
			virtKeybPrint();
			return false;
		}
	}

	if (_progress2 == 104) {
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
		++_progress2;
	}

	if (_verbInterfaceMode < 2) {
		if (_progress2 == 105) {
			_gfx->transitionCommand(_gfx->getVerbAreaType() ? 19 : 14);
			++_progress2;
		}

		if (_progress2 == 106) {
			if (_gfx->transitionStateBusy())
				return false;
			++_progress2;
		}
		if (_progress2 == 107)
			_videoPhoneMode = _textInputColumnMax = 0;

		if (_progress2 >= 109 && _progress <= 127) {
			verbTabSwapPage();
			if (_progress2 == 103) {
				drawHiliteVerb(0);
				_progress2 |= 0x100;
			}
			return false;
		}

	} else {
		if (_progress2 == 105) {
			_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
			_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
			if (_verbInterfaceMode == 2)
				_gfx->transitionCommand(0xFE);
			else if (_vkeybVisible) {
				_vkeybVisible = false;
				_gfx->transitionCommand(17);
			}
			++_progress2;
			return false;
		}

		if (_progress2 == 106) {
			if (_gfx->transitionStateBusy())
				return false;
			++_progress2;
		}

		if (_progress2 == 107) {
			_que->writeUInt16(0x16);
			_que->start();
			++_progress2;
			return false;
		}

		if (_progress2 == 108) {
			if (++_verbsInterpreterMode != 2) {
				_verbsInterpreterMode = 0;
				_verbInterfaceMode = 0;
			}
			_videoPhoneMode = 0;
			_textInputColumnMax = 0;
		}
	}

	_progress2 = -1;
	return true;
}

void UI::moveFlashLight() {
	if (_verbInterfaceMode != 1)
		return;
	_gfx->transitionCommand(1);
	_gfx->setScrollStep(GraphicsEngine::kSingleStep | GraphicsEngine::kHorzA, (_flashLight.posX >> 16) + ((Util::rngMakeNumber() & 0x8000) ? 1 : 0));
	_gfx->setScrollStep(GraphicsEngine::kSingleStep | GraphicsEngine::kVertA, (_flashLight.posY >> 16) - 40 + ((Util::rngMakeNumber() & 0x8000) ? 1 : 0));
}

void UI::setVerbInterfaceMode(uint16 mode) {
	_verbInterfaceMode = mode;
	if (mode == 1)
		_flashLight.posX = _flashLight.posY = 0;
}

void UI::loadState(Common::SeekableReadStream *in) {
	if (in->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		error("%s(): Save file invalid or corrupt", __FUNCTION__);
	_verbInterfaceMode = in->readUint16BE();
	_verbsInterpreterMode = in->readUint16BE();
	_progress = _progress2 = -1;
}

void UI::saveState(Common::SeekableWriteStream *out) {
	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));
	out->writeUint16BE(_verbInterfaceMode);
	out->writeUint16BE(_verbsInterpreterMode);
}

void UI::printDialogStringHead() {
	static const uint8 strHead[] = { 0xFC, 0x00, 0xF9, 0x01, 0xFB, 0x10, 0x00, 0xFE };
	const uint8 *s = _scd->makePtr(0x13552).getDataFromTable(_sceneId)();
	memcpy(_dialogTextBuffer, strHead, sizeof(strHead));
	_dialogTextBuffer[6] = _headLineYOffset;
	_dialogTextBuffer[8] = (*s & 0x80) ? 3 : (*s++ & 7);
	uint8 *d = _dialogTextBuffer + 9;

	for (int i = 0; i < 25; ++i) {
		*d++ = *s;
		if (*s++ == 0xFF)
			break;
	}

	assert(*(d - 1) == 0xFF);

	_gfx->printText(_dialogTextBuffer);
	_textY = _headLineYOffset + 9;
}

void UI::printDialogStringBody() {
	static const uint8 strHead[] = { 0xFC, 0x01, 0xF9, 0x01, 0xFE, 0x00, 0xFB };
	static const uint8 buttonTable[3][6] = {
		{ 'A', 'A', 'B', 'C', 'C', 'B' },
		{ 'B', 'C', 'A', 'A', 'B', 'C' },
		{ 'C', 'B', 'C', 'B', 'A', 'A' }
	};

	memcpy(_dialogTextBuffer, strHead, sizeof(strHead));
	_dialogTextBuffer[5] = _textColor;
	uint8 *d = _dialogTextBuffer + sizeof(strHead);
	//_makestrbt1 = 0xFF;
	*d++ = _sceneId ? 24 : 40;
	*d++ = _textY;

	_textLineBreak = 0;
	assert(_scriptTextResource);
	const uint8 *s = _scriptTextResource + _sceneTextOffsCur;
	for (int i = 0; i < 50;++i) {
		uint8 in = *s++;
		++_sceneTextOffsCur;
		switch (in) {
		case 0xFF:
			_textLineEnd = 1;
			i = 49;
			break;
		case 0xF6:
			_textLineBreak = 3;
			i = 49;
			break;
		case 0xF4:
			_sceneInfo = (_sceneInfo & 0xFF) | 0x200;
			break;
		case 0xF2:
			_textLineBreak = 1;
			i = 49;
			break;
		case 0xF0:
		case 0xEE:
		case 0xEC:
			*d++ = buttonTable[(in - 0xEC) >> 1][_controllerCfg];
			break;
		default:
			if (in < 0xF8) {
				*d++ = in;
			} else {
				*d++ = 0xFE;
				*d++ = _textColor = in & 7;
			}
			break;
		}
	}

	*d++ = 0xFF;
	_gfx->printText(_dialogTextBuffer);
}

bool UI::checkStringPrintProgress() {
	if (_textLineEnd || _textLineBreak == 3)
		return true;
	_textY += 9;
	return (_textY > 56);
}

bool UI::waitWithCursorAnim(uint16 inputFlags) {
	if (inputFlags & 0x20)
		return true;
	if ((_waitCursorAnimDelay && --_waitCursorAnimDelay) || _sceneId == 0)
		return false;
	const uint8 *s = _scd->makePtr(0x1391C)() + _waitCursorFrame;

	ResourcePointer drw = _scd->makePtr(_gfx->getVerbAreaType() == 0 ? 0x1387C : (_gfx->getVerbAreaType() == 1 ? 0x13894 : 0x138AC));

	uint32 frm = 0x138C4 + _waitCursorFrame * 4;
	(drw + 2).writeUINT32(frm);
	(drw + 12).writeUINT32(frm + 4);

	_gfx->enqueueDrawCommands(drw);

	_waitCursorFrame += 2;
	_waitCursorAnimDelay = s[1];
	if (s[3])
		return false;
	if (_sceneInfo >> 8)
		return true;
	_waitCursorFrame = 0;
	return false;
}

uint8 UI::drawVerb(uint8 id) {
	uint8 cid = id;
	if (_verbsTabCurPage == 1 && _lastVerbFirstPage == id) {
		_que->writeUInt16(0x05);
		_que->writeUInt32(_gfx->getVerbAreaType() ? 0x10F26 : 0x10F10);
		return 0x80;
	} else if (_verbsTabCurPage > 1) {
		if (id == 0) {
			_que->writeUInt16(0x05);
			_que->writeUInt32(0x10EFA);
			return 0x80;
		} else {
			id += _numVerbsFirstPage;
		}
	}

	if (id >= _scriptVerbsArray->pos())
		return 0xFF;

	uint16 offs = _scriptVerbsArray->read(id);
	const uint8 *s = _scriptTextResource + offs;
	uint8 *d = _verbTextBuffer + 9;
	for (int i = 0; i < 33; ++i) {
		*d = *s++;
		if (*d++ == 0xFF)
			break;
	}
	Common::String dbg((const char*)_verbTextBuffer + 9, (const char*)_verbTextBuffer + 32);
	debug("VERB %d: %s", id, dbg.substr(0,  dbg.findFirstOf((char)0xFF, 0)).c_str());
	_verbTextBuffer[47] = 0xFF;

	return cid;
}

void UI::drawHiliteVerb(uint8 id) {
	uint8 res = 0;
	if (id == 0) {
		_verbTextBuffer[5] = 4;
		res = drawVerb(_lastHiliteVerb);
	} else {
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaPosX, 16);
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaPosY, _hiliteVerb * 9 + 143 + (_gfx->getVerbAreaType() ? 16 : 0));
		_verbTextBuffer[5] = 1;
		res = drawVerb(_hiliteVerb);
	}
	if (res != 0x80) {
		_verbTextBuffer[7] = 0x10;
		_verbTextBuffer[8] = res * 9;
		_gfx->printText(_verbTextBuffer);
	} else {
		_que->start();
	}
	if (id == 1) {
		_lastHiliteVerb = _hiliteVerb;
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, 0);
	}
}

bool UI::drawUnderscoreString() {
	static const uint8 underscoreStr[] = { 0xFC, 0x00, 0xF9, 0x01, 0xFB, 0x00, 0x8, 0xFE, 0x01, 0x5F, 0xFF };
	memcpy(_underscoreStr, underscoreStr, sizeof(underscoreStr));
	_underscoreStr[5] = _textInputColumnCur * 9 + _textInputMarginLeft;
	_underscoreStr[6] = 33;
	_gfx->printText(_underscoreStr);
	return (++_textInputColumnCur == _textInputColumnMax);
}

void UI::drawUnderscoreCursor() {
	if (_textInputCursorState) {
		_underscoreStr[8] = 1;
		_textInputCursorState = 0;
	} else {
		_underscoreStr[5] = _textInputColumnCur * 9 + _textInputMarginLeft;
		_underscoreStr[8] = 0;
		++_textInputCursorState;
	}
	_gfx->printText(_underscoreStr);
	_textInputCursorBlinkCnt = 32;
}

void UI::verbSelect2setResult() {
	int pos = _textInputColumnCur;
	if (_textInputStr[pos])
		++pos;

	uint8 *d = &_dialogTextBuffer[9];
	d[pos] = 0xFF;
	int i = 0;
	for (; i < 247; ++i) {
		if (d[i] == 0xFF || d[i] != ' ')
			break;
	}
	if (d[i] == 0xFF || i == 247) {
		d[0] = ' ';
		d[1] = 0xFF;
	}

	for (i = 0; i < _scriptVerbsArray->size(); ++i) {
		uint16 val = _scriptVerbsArray->read(i);
		if (verbSelect2checkSelection(val))
			break;
	}

	_selectedVerb = (i == _scriptVerbsArray->size()) ? 1 : i + 1;
}

void UI::verbsTabHandleInput(const SnatcherEngine::Input &input) {
	if (_progress2 & 0x100) {
		if (_verbInterfaceMode < 2)
			drawHiliteVerb(1);
		else
			virtKeybBackspace();
		_progress2 &= ~0x100;
		return;
	}
	if (_progress2 & 0x400) {
		if (_gfx->getAnimParameter(62, GraphicsEngine::kAnimParaControlFlags) != (GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause))
			return;
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosX, _hilitePosX);
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosY, _hilitePosY);
		_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
		if (_progress2 & 0x800)
			verbSelect2setResult();
		_progress2 &= ~0xC00;
		return;
	}

	if (_verbInterfaceMode < 2) {
		if (_verbInterfaceMode == 1) {
			if (_scriptSentenceArray->pos() == 0) {
				if (input.sustainedControllerFlagsRemapped & 0x10) {
					//_c13Valu = 0xFF;
					uint8 in = _flashLight.inputFlags = input.sustainedControllerFlagsRemapped;
					_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
					if (in & 1)
						moveFlashLight(0, -0x20000);
					if (in & 2)
						moveFlashLight(0, 0x20000);
					if (in & 8)
						moveFlashLight(0x20000, 0);
					if (in & 4)
						moveFlashLight(-0x20000, 0);
				} else {
					//_c13Valu = 0;
					_flashLight.posData = checkFlashLightPos(((_flashLight.posX >> 16) + 72) >> 2, ((_flashLight.posY >> 16) + 40) >> 2);
					_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
				}
			}
			if (_scriptSentenceArray->pos() == 0 && (input.sustainedControllerFlagsRemapped & 0x10))
				return; // No handling of the verbs section when moving around the flashlight
		}

		uint8 in = input.singleFrameControllerFlagsRemapped;
		if (_scriptVerbsArray->pos() == 1) {
			if (in == 0x10) {
				if (_scriptSentenceArray->pos() != 0)
					_selectedVerb = 0xFF;
			} else if (in == 0x20) {
				verbSelect();
			}
		} else {
			if (in == 1 || in == 4) {
				for (bool lp = true; lp; ) {
					lp = false;
					if (--_hiliteVerb < 0) {
						_hiliteVerb = _lastVerbDrawn - 1;
						if (_verbsTabCurPage) {
							if (_verbsTabCurPage == 1)
								_hiliteVerb = _numVerbsLastPage;
							else
								--_hiliteVerb;
							verbTabSwapPage();
						}
					} else if (_hiliteVerb == 0 && _verbsTabCurPage == 2) {
						lp = true;
						continue;
					}
				}
				if (_progress2 == 103) {
					drawHiliteVerb(0);
					_progress2 |= 0x100;
				}

			} else if (in == 2 || in == 8) {
				for (bool lp = true; lp; ) {
					lp = false;
					if ((++_hiliteVerb + 1 == _lastVerbDrawn) && (_verbsTabCurPage == 1)) {
						lp = true;
						continue;
					}
				}
				if ((_hiliteVerb == _lastVerbDrawn) || (_verbsTabCurPage == 2 && _hiliteVerb == _lastVerbLastPage)) {
					_hiliteVerb = 0;
					if (_verbsTabCurPage) {
						if (_verbsTabCurPage == 1)
							_hiliteVerb = 1;
						verbTabSwapPage();
					}
				}
				if (_progress2 == 103) {
					drawHiliteVerb(0);
					_progress2 |= 0x100;
				}
			} else if (in == 0x10) {
				if (_scriptSentenceArray->pos() != 0)
					_selectedVerb = 0xFF;
			} else if (in == 0x20) {
				verbSelect();
			}
		}
	} else {
		if (--_textInputCursorBlinkCnt == 0)
			drawUnderscoreCursor();

		uint8 in = input.singleFrameControllerFlagsRemapped;

		if (in & 1) {
			for (int cr = _vkeybPosTotal - _vkeybColumnWidth; verbsTabMoveHilite(cr); )
				cr -= _vkeybColumnWidth;

		} else if (in & 2) {
			for (int cr = _vkeybPosTotal + _vkeybColumnWidth; verbsTabMoveHilite(cr); )
				cr += _vkeybColumnWidth;

		} else if (in & 8) {
			for (int cr = _vkeybPosTotal + 1; verbsTabMoveHilite(cr); )
				++cr;

		} else if (in & 4) {
			for (int cr = _vkeybPosTotal - 1; verbsTabMoveHilite(cr); )
				--cr;

		} else if (in & 0x10) {
			virtKeybBackspace();
		} else if ((_videoPhoneMode == 0) && (in & 0x80)) {
			virtKeybStart();
		} else if (in & 0x20) {
			if (verbSelect2())
				virtKeybPrint();
		}
	}
}

bool UI::verbsTabMoveHilite(int &pos) {
	switch (_verbsTabLayoutMap[pos]) {
	case 0:
		break;
	case 1:
		return true;
	case 2:
		while (verbsTabMoveHilite(++pos)) {}
		break;
	case 3:
		while(verbsTabMoveHilite(--pos)) {}
		break;
	case 4:
		do {
			pos += _vkeybColumnWidth;
		} while(verbsTabMoveHilite(pos));
		break;
	case 5:
		do {
			pos -= _vkeybColumnWidth;
		} while(verbsTabMoveHilite(pos));
		break;
	case 6:
		pos += _vkeybColumnWidth;
		while(verbsTabMoveHilite(--pos)) {}
		break;
	case 7:
		pos -= _vkeybColumnWidth;
		while(verbsTabMoveHilite(++pos)) {}
		break;
	default:
		_vkeybPosTotal = pos;
		adjustHilitePos();
	}
	return false;
}

void UI::adjustHilitePos() {
	_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
	_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaFrame, _verbInterfaceMode == 2 ? 2 : 0);
	_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);

	int rows = 1;
	int remX = _vkeybPosTotal;
	while (_vkeybColumnWidth <= remX) {
		remX -= _vkeybColumnWidth;
		++rows;
	}

	_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosX, (remX << 3) + _verbsTabOffsX);
	_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaPosX, (remX << 3) + _verbsTabOffsX);
	_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosY, (rows << 3) + _verbsTabOffsY);
	_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaPosY, (rows << 3) + _verbsTabOffsY);
}

void UI::verbTabSwapPage() {
	if (_progress2 == 103) {
		_progress2 = 109;
		_gfx->setAnimParameter(0, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
		_que->writeUInt16(0x16);
		_que->start();
		_verbTextBuffer[5] = 4;
		_verbsTabCurPage ^= 3;
	} else {
		uint8 res = (_progress2 - 109 < _numVerbsMax) ? drawVerb(_progress2 - 109) : 0xFF;
		if (res == 0xFF) {
			_progress2 = 103;
		} else if (res != 0x80) {
			_verbTextBuffer[7] = 0x10;
			_verbTextBuffer[8] = res * 9;
			_gfx->printText(_verbTextBuffer);
		} else {
			_que->start();
		}
		if (_progress2 == 103)
			_lastHiliteVerb = _hiliteVerb;
		else
			++_progress2;
	}
}

void UI::verbSelect() {
	if ((_verbsTabCurPage == 1 && _hiliteVerb == _lastVerbFirstPage) || (_verbsTabCurPage == 2 && _hiliteVerb == 0))
		return;

	_selectedVerb = _hiliteVerb + 1;

	if (_verbsTabCurPage == 2)
		_selectedVerb += _numVerbsFirstPage;

	if (_scriptSentenceArray->pos() == 0)
		_prevSelectedVerb = _selectedVerb - 1;

	_que->writeUInt16(0x1C);
	_que->writeUInt16(0x1D);
	_que->start();
}

bool UI::verbSelect2() {
	uint8 r = _verbsTabLayoutMap[_vkeybPosTotal];

	if (_videoPhoneMode) {
		if (!(_progress2 & 0x100)) {
			if (r == '*')
				_gfx->setAnimParameter(2, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
			else if (r == '#')
				_gfx->setAnimParameter(3, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
			else
				_gfx->setAnimParameter(r & 0x1F, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
			_progress2 = 0x101;
		}

	} else {
		if (!(_progress2 & 0x100)) {
			_que->writeUInt16(0x1C);
			_que->writeUInt16(0x1D);
			_que->start();
			_progress2 = 0x100;
			return false;
		}	

		if (_progress2 == 0x100) {
			if (_verbInterfaceMode == 3) {
				uint8 frm = _scd->makePtr(0x115CA)[_vkeybPosTotal];
				_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaFrame, frm);
				_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
			}
			++_progress2;
			if (r == 8 || r == 9) {
				_que->writeUInt16(0x1C);
				_que->writeUInt16(0x1D);
				_que->start();
				return false;
			}
		}
	}

	bool reslt = true;
	if (_progress2 == 0x101) {
		if (r == 8) {
			_progress2 |= 0xC00;
			reslt = false;

		} else if (r == 9) {
			if (_textInputColumnCur != 0) {
				if (!_textInputStr[_textInputColumnCur])
					_textInputColumnCur--;
				_textInputStr[_textInputColumnCur] = 0;
			} else {
				reslt = false;
			}
		} else if (_textInputColumnCur < _textInputColumnMax) {
			_textInputStr[_textInputColumnCur] = r;
			if (_textInputColumnCur + 1 < _textInputColumnMax)
				++_textInputColumnCur;
		} else {
			reslt = false;
		}
		_progress2 = reslt ? 0x200 : (_progress2 & 0xC00) | 103;
	}
	return reslt;
}

void UI::virtKeybBackspace() {
	if (!(_progress2 & 0x100)) {
		if (_verbInterfaceMode == 3) {
			_hilitePosX = _gfx->getAnimParameter(62, GraphicsEngine::kAnimParaPosX);
			_hilitePosY = _gfx->getAnimParameter(62, GraphicsEngine::kAnimParaPosY);
			_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosX, 88);
			_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosY, 192);
			_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaFrame, 104);
			_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
			if (_vkeybPosTotal != 139)
				_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
		}
		_que->writeUInt16(0x1C);
		_que->writeUInt16(0x1D);
		_que->start();
		_progress2 |= 0x100;
		return;
	} else {
		if (_textInputColumnCur != 0) {
			if (!_textInputStr[_textInputColumnCur])
				_textInputColumnCur--;
			_textInputStr[_textInputColumnCur] = 0;
		}
		_progress2 |= 0x400;
		virtKeybPrint();
	}
}

void UI::virtKeybStart() {
	if (_verbInterfaceMode == 3) {
		_hilitePosX = _gfx->getAnimParameter(62, GraphicsEngine::kAnimParaPosX);
		_hilitePosY = _gfx->getAnimParameter(62, GraphicsEngine::kAnimParaPosY);
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosX, 200);
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaPosY, 192);
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaFrame, 108);
		_gfx->setAnimParameter(62, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimNone);
		_gfx->setAnimParameter(63, GraphicsEngine::kAnimParaControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
	}
	_que->writeUInt16(0x1C);
	_que->writeUInt16(0x1D);
	_que->start();

	_progress2 |= 0xC00;
}

bool UI::verbSelect2checkSelection(uint16 val) {
	const uint8 *s1 = &_scriptTextResource[val];
	const uint8 *s2 = &_dialogTextBuffer[9];

	if (_verbsInterpreterMode == 1) {
		for (int i = 0; i < 247; ++i) {
			if (*s1 == 0xFF)
				return false;
			if (*s1++ == *s2)
				break;
		}
	} else {
		for (int i = 0; i < 247; ++i) {
			if (*s1 != *s2++)
				return false;
			if (*s1++ == 0xFF)
				break;
		}
	}
	return true;
}

void UI::virtKeybPrint() {
	int pr = _progress2 & ~0x400;

	if (!(_progress2 & 0x200)) {
		_progress2 = (_progress2 & ~0xFF) | 0x200;
		return;
	}
	if (pr == 0x200) {
		_que->writeUInt16(0x23);
		_que->start();
		++_progress2;
		return;
	}
	if (pr == 0x201) {
		uint8 *d = &_dialogTextBuffer[9];
		for (int i = 0; i < 18; ++i) {
			if (_textInputStr[i] != 0)
				*d++ = _textInputStr[i];
		}
		if (d == &_dialogTextBuffer[9])
			*d++ = ' ';
		*d++ = 0xFF;

		_que->writeUInt16(0x05);
		_que->writeUInt32(0x11946);
		_que->start();
		++_progress2;
		return;
	}
	if (pr == 0x202) {
		_gfx->printText(_dialogTextBuffer);
		++_progress2;
		return;
	}
	if (pr == 0x203) {
		_que->writeUInt16(0x05);
		_que->writeUInt32(0x11942);
		_que->start();
		++_progress2;
			return;
	}
	if (pr == 0x204) {
		drawUnderscoreCursor();
		_progress2 = (_progress2 & ~0x2FF) | 103;

		if (_videoPhoneMode != 0) {
			uint8 v = _textInputStr[0];
			if (_textInputStr[_scd->makePtr(0x11938)[(v == '*' || v == '#') ? 2 : (v & 0x0F)] - 1])
				verbSelect2setResult();
		}
	}
}

uint8 UI::checkFlashLightPos(int16 x, int16 y) {
	return _scd->makePtr(0x109E8)[y * 36 + x];
}

void UI::moveFlashLight(int32 offsX, int32 offsY) {
	if (offsX == 0) {
		assert(offsY != 0);
		int32 nY = _flashLight.posY + offsY;
		if (nY >= -0x270000 && nY < 0x250000)
			_flashLight.posY = nY;

	} else if (offsY == 0) {
		assert(offsX != 0);
		int32 nX = _flashLight.posX + offsX;
		if (nX >= -0x470000 && nX < 0x450000)
			_flashLight.posX = nX;
	} else {
		error("%s(): Invalid parameters", __FUNCTION__);
	}
	debug ("FLASHLIGHT: X: %d, Y: %d", _flashLight.posX >> 16, _flashLight.posY >> 16);
}

} // End of namespace Snatcher
