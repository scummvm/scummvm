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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/inventory.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {


/****************************************************************************\
*           RMInventory Methods
\****************************************************************************/

RMInventory::RMInventory() {
	_items = NULL;
	_state = CLOSED;
	_bCombining = false;
	_csModifyInterface = g_system->createMutex();
	_nItems = 0;

	Common::fill(_inv, _inv + 256, 0);
	_nInv = 0;
	_curPutY = 0;
	_curPutTime = 0;
	_curPos = 0;
	_bHasFocus = false;
	_nSelectObj = 0;
	_nCombine = 0;
	_bBlinkingRight = false;
	_bBlinkingLeft = false;
	miniAction = 0;
}

RMInventory::~RMInventory() {
	close();
	g_system->deleteMutex(_csModifyInterface);
}

bool RMInventory::checkPointInside(const RMPoint &pt) {
	if (!GLOBALS.bCfgInvUp)
		return pt.y > RM_SY - 70;
	else
		return pt.y < 70;
}


void RMInventory::init(void) {
	int i, j;
	int curres;

	// Create the main buffer
	Create(RM_SX, 68);
	setPriority(185);

	// Setup the inventory
	_nInv = 0;
	_curPos = 0;
	_bCombining = false;

	// New items
	_nItems = 78;  // @@@ Number of takeable items
	_items = new RMInventoryItem[_nItems + 1];

	curres = 10500;

	// Loop through the items
	for (i = 0; i <= _nItems; i++) {
		// Load the items from the resource
		RMRes res(curres);
		RMDataStream ds;

		assert(res.IsValid());

		// Initialise the MPAL inventory item by reading it in.
		_items[i].icon.SetInitCurPattern(false);
		ds.OpenBuffer(res);
		ds >> _items[i].icon;
		ds.Close();

		// Puts in the default pattern 1
		_items[i].pointer = NULL;
		_items[i].status = 1;
		_items[i].icon.SetPattern(1);
		_items[i].icon.doFrame(this, false);

		curres++;
		if (i == 0 || i == 28 || i == 29)
			continue;

		_items[i].pointer = new RMGfxSourceBuffer8RLEByteAA[_items[i].icon.NumPattern()];

		for (j = 0; j < _items[i].icon.NumPattern(); j++) {
			RMResRaw raw(curres);

			assert(raw.IsValid());

			_items[i].pointer[j].init((const byte *)raw, raw.Width(), raw.Height(), true);
			curres++;
		}
	}

	_items[28].icon.SetPattern(1);
	_items[29].icon.SetPattern(1);

	// Download interface
	RMDataStream ds;
	RMRes res(RES_I_MINIINTER);
	assert(res.IsValid());
	ds.OpenBuffer(res);
	ds >> miniInterface;
	miniInterface.SetPattern(1);
	ds.Close();

	// Create the text for hints on the mini interface
	_hints[0].setAlignType(RMText::HCENTER, RMText::VTOP);
	_hints[1].setAlignType(RMText::HCENTER, RMText::VTOP);
	_hints[2].setAlignType(RMText::HCENTER, RMText::VTOP);

	// The text is taken from MPAL for translation
	RMMessage msg1(15);
	RMMessage msg2(13);
	RMMessage msg3(14);

	_hints[0].writeText(msg1[0], 1);       // Examine
	_hints[1].writeText(msg2[0], 1);       // Take
	_hints[2].writeText(msg3[0], 1);       // Use


	// Prepare initial inventory
	prepare();
	drawOT(Common::nullContext);
	clearOT();
}

void RMInventory::close(void) {
	// Has memory
	if (_items != NULL) {
		// Delete the item pointers
		for (int i = 0; i <= _nItems; i++)
			delete[] _items[i].pointer;

		// Delete the items array
		delete[] _items;
		_items = NULL;
	}

	destroy();
}

void RMInventory::reset(void) {
	_state = CLOSED;
	endCombine();
}

void RMInventory::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	RMPoint pos;
	RMPoint pos2;
	RMGfxPrimitive *p;
	RMGfxPrimitive *p2;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_state == OPENING || _state == CLOSING)
		prim->setDst(RMPoint(0, _curPutY));
	else
		prim->setDst(RMPoint(0, _curPutY));

	g_system->lockMutex(_csModifyInterface);
	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);
	g_system->unlockMutex(_csModifyInterface);

	if (_state == SELECTING) {

		if (!GLOBALS.bCfgInvUp) {
			_ctx->pos.Set((_nSelectObj + 1) * 64 - 20, RM_SY - 113);
			_ctx->pos2.Set((_nSelectObj + 1) * 64 + 34, RM_SY - 150);
		} else {
			_ctx->pos.Set((_nSelectObj + 1) * 64 - 20, 72 - 4); // The brown part is at the top :(
			_ctx->pos2.Set((_nSelectObj + 1) * 64 + 34, 119 - 4);
		}

		_ctx->p = new RMGfxPrimitive(prim->_task, _ctx->pos);
		_ctx->p2 = new RMGfxPrimitive(prim->_task, _ctx->pos2);

		// Draw the mini interface
		CORO_INVOKE_2(miniInterface.draw, bigBuf, _ctx->p);

		if (GLOBALS.bCfgInterTips) {
			if (miniAction == 1) // Examine
				CORO_INVOKE_2(_hints[0].draw, bigBuf, _ctx->p2);
			else if (miniAction == 2) // Talk
				CORO_INVOKE_2(_hints[1].draw, bigBuf, _ctx->p2);
			else if (miniAction == 3) // Use
				CORO_INVOKE_2(_hints[2].draw, bigBuf, _ctx->p2);
		}

		delete _ctx->p;
		delete _ctx->p2;
	}

	CORO_END_CODE;
}

void RMInventory::removeThis(CORO_PARAM, bool &result) {
	if (_state == CLOSED)
		result = true;
	else
		result = false;
}

void RMInventory::removeItem(int code) {
	int i;

	for (i = 0; i < _nInv; i++)
		if (_inv[i] == code - 10000) {
			g_system->lockMutex(_csModifyInterface);

			Common::copy(&_inv[i + 1], &_inv[i + 1] + (_nInv - i), &_inv[i]);
			_nInv--;

			prepare();
			drawOT(Common::nullContext);
			clearOT();
			g_system->unlockMutex(_csModifyInterface);
			return;
		}
}

void RMInventory::addItem(int code) {
	if (code <= 10000 && code >= 10101) {
		// If we are here, it means that we are adding an item that should not be in the inventory
		warning("Cannot find a valid icon for this item, and then it will not be added to the inventory");
	} else {
		g_system->lockMutex(_csModifyInterface);
		if (_curPos + 8 == _nInv) {
			// Break through the inventory! On the flashing pattern
			_items[28].icon.SetPattern(2);
		}

		_inv[_nInv++] = code - 10000;

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}

void RMInventory::changeItemStatus(uint32 code, uint32 dwStatus) {
	if (code <= 10000 && code >= 10101) {
		error("Specified object code is not valid");
	} else {
		g_system->lockMutex(_csModifyInterface);
		_items[code - 10000].icon.SetPattern(dwStatus);
		_items[code - 10000].status = dwStatus;

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}


void RMInventory::prepare(void) {
	int i;

	for (i = 1; i < RM_SX / 64 - 1; i++) {
		if (i - 1 + _curPos < _nInv)
			addPrim(new RMGfxPrimitive(&_items[_inv[i - 1 + _curPos]].icon, RMPoint(i * 64, 0)));
		else
			addPrim(new RMGfxPrimitive(&_items[0].icon, RMPoint(i * 64, 0)));
	}

	// Frecce
	addPrim(new RMGfxPrimitive(&_items[29].icon, RMPoint(0, 0)));
	addPrim(new RMGfxPrimitive(&_items[28].icon, RMPoint(640 - 64, 0)));
}

bool RMInventory::miniActive(void) {
	return _state == SELECTING;
}

bool RMInventory::haveFocus(const RMPoint &mpos) {
	// When we combine, have the focus only if we are on an arrow (to scroll)
	if (_state == OPENED && _bCombining && checkPointInside(mpos) && (mpos.x < 64 || mpos.x > RM_SX - 64))
		return true;

	// If the inventory is open, focus we we go over it
	if (_state == OPENED && !_bCombining && checkPointInside(mpos))
		return true;

	// If we are selecting a verb (and then right down), we always focus
	if (_state == SELECTING)
		return true;

	return false;
}

void RMInventory::endCombine(void) {
	_bCombining = false;
}

bool RMInventory::leftClick(const RMPoint &mpos, int &nCombineObj) {
	int n;

	// The left click picks an item from your inventory to use it with the background
	n = mpos.x / 64;

	if (_state == OPENED) {
		if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0) {
			_bCombining = true; //m_state = COMBINING;
			_nCombine = _inv[n - 1 + _curPos];
			nCombineObj = _nCombine + 10000;

			_vm->playUtilSFX(1);
			return true;
		}
	}

	// Click the right arrow
	if ((_state == OPENED) && _bBlinkingRight) {
		g_system->lockMutex(_csModifyInterface);
		_curPos++;

		if (_curPos + 8 >= _nInv) {
			_bBlinkingRight = false;
			_items[28].icon.SetPattern(1);
		}

		if (_curPos > 0) {
			_bBlinkingLeft = true;
			_items[29].icon.SetPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
	// Click the left arrow
	else if ((_state == OPENED) && _bBlinkingLeft) {
		assert(_curPos > 0);
		g_system->lockMutex(_csModifyInterface);
		_curPos--;

		if (_curPos == 0) {
			_bBlinkingLeft = false;
			_items[29].icon.SetPattern(1);
		}

		if (_curPos + 8 < _nInv) {
			_bBlinkingRight = true;
			_items[28].icon.SetPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}


	return false;
}

void RMInventory::rightClick(const RMPoint &mpos) {
	int n;

	assert(checkPointInside(mpos));

	if (_state == OPENED && !_bCombining) {
		// Open the context interface
		n = mpos.x / 64;

		if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0) {
			_state = SELECTING;
			miniAction = 0;
			_nSelectObj = n - 1;

			_vm->playUtilSFX(0);
		}
	}

	if ((_state == OPENED) && _bBlinkingRight) {
		g_system->lockMutex(_csModifyInterface);
		_curPos += 7;
		if (_curPos + 8 > _nInv)
			_curPos = _nInv - 8;

		if (_curPos + 8 <= _nInv) {
			_bBlinkingRight = false;
			_items[28].icon.SetPattern(1);
		}

		if (_curPos > 0) {
			_bBlinkingLeft = true;
			_items[29].icon.SetPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	} else if ((_state == OPENED) && _bBlinkingLeft) {
		assert(_curPos > 0);
		g_system->lockMutex(_csModifyInterface);
		_curPos -= 7;
		if (_curPos < 0)
			_curPos = 0;

		if (_curPos == 0) {
			_bBlinkingLeft = false;
			_items[29].icon.SetPattern(1);
		}

		if (_curPos + 8 < _nInv) {
			_bBlinkingRight = true;
			_items[28].icon.SetPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}

bool RMInventory::rightRelease(const RMPoint &mpos, RMTonyAction &curAction) {
	if (_state == SELECTING) {
		_state = OPENED;

		if (miniAction == 1) { // Esamina
			curAction = TA_EXAMINE;
			return true;
		} else if (miniAction == 2) { // Parla
			curAction = TA_TALK;
			return true;
		} else if (miniAction == 3) { // Usa
			curAction = TA_USE;
			return true;
		}
	}

	return false;
}

#define INVSPEED    20

void RMInventory::doFrame(RMGfxTargetBuffer &bigBuf, RMPointer &ptr, RMPoint mpos, bool bCanOpen) {
	int i;
	bool bNeedRedraw = false;

	if (_state != CLOSED) {
		// Clean up the OT list
		g_system->lockMutex(_csModifyInterface);
		clearOT();

		// DoFrame makes all the objects currently in the inventory be displayed
		// @@@ Maybe we should do all takeable objects? Please does not help
		for (i = 0; i < _nInv; i++)
			if (_items[_inv[i]].icon.doFrame(this, false) && (i >= _curPos && i <= _curPos + 7))
				bNeedRedraw = true;

		if ((_state == CLOSING || _state == OPENING || _state == OPENED) && checkPointInside(mpos)) {
			if (mpos.x > RM_SX - 64) {
				if (_curPos + 8 < _nInv && !_bBlinkingRight) {
					_items[28].icon.SetPattern(3);
					_bBlinkingRight = true;
					bNeedRedraw = true;
				}
			} else if (_bBlinkingRight) {
				_items[28].icon.SetPattern(2);
				_bBlinkingRight = false;
				bNeedRedraw = true;
			}

			if (mpos.x < 64) {
				if (_curPos > 0 && !_bBlinkingLeft) {
					_items[29].icon.SetPattern(3);
					_bBlinkingLeft = true;
					bNeedRedraw = true;
				}
			} else if (_bBlinkingLeft) {
				_items[29].icon.SetPattern(2);
				_bBlinkingLeft = false;
				bNeedRedraw = true;
			}
		}

		if (_items[28].icon.doFrame(this, false))
			bNeedRedraw = true;

		if (_items[29].icon.doFrame(this, false))
			bNeedRedraw = true;

		if (bNeedRedraw)
			prepare();

		g_system->unlockMutex(_csModifyInterface);
	}

	if (_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_i)) {
		GLOBALS.bCfgInvLocked = !GLOBALS.bCfgInvLocked;
	}

	if (_bCombining) {//m_state == COMBINING)
		ptr.setCustomPointer(&_items[_nCombine].pointer[_items[_nCombine].status - 1]);
		ptr.setSpecialPointer(RMPointer::PTR_CUSTOM);
	}

	if (!GLOBALS.bCfgInvUp) {
		if ((_state == CLOSED) && (mpos.y > RM_SY - 10 || GLOBALS.bCfgInvLocked) && bCanOpen) {
			if (!GLOBALS.bCfgInvNoScroll) {
				_state = OPENING;
				_curPutY = RM_SY - 1;
				_curPutTime = _vm->getTime();
			} else {
				_state = OPENED;
				_curPutY = RM_SY - 68;
			}
		} else if (_state == OPENED) {
			if ((mpos.y < RM_SY - 70 && !GLOBALS.bCfgInvLocked) || !bCanOpen) {
				if (!GLOBALS.bCfgInvNoScroll) {
					_state = CLOSING;
					_curPutY = RM_SY - 68;
					_curPutTime = _vm->getTime();
				} else {
					_state = CLOSED;
				}
			}
		} else if (_state == OPENING) {
			while (_curPutTime + INVSPEED < _vm->getTime()) {
				_curPutY -= 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY <= RM_SY - 68) {
				_state = OPENED;
				_curPutY = RM_SY - 68;
			}

		} else if (_state == CLOSING) {
			while (_curPutTime + INVSPEED < _vm->getTime()) {
				_curPutY += 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY > 480)
				_state = CLOSED;
		}
	} else {
		if ((_state == CLOSED) && (mpos.y < 10 || GLOBALS.bCfgInvLocked) && bCanOpen) {
			if (!GLOBALS.bCfgInvNoScroll) {
				_state = OPENING;
				_curPutY = - 68;
				_curPutTime = _vm->getTime();
			} else {
				_state = OPENED;
				_curPutY = 0;
			}
		} else if (_state == OPENED) {
			if ((mpos.y > 70 && !GLOBALS.bCfgInvLocked) || !bCanOpen) {
				if (!GLOBALS.bCfgInvNoScroll) {
					_state = CLOSING;
					_curPutY = -2;
					_curPutTime = _vm->getTime();
				} else {
					_state = CLOSED;
				}
			}
		} else if (_state == OPENING) {
			while (_curPutTime + INVSPEED < _vm->getTime()) {
				_curPutY += 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY >= 0) {
				_state = OPENED;
				_curPutY = 0;
			}
		} else if (_state == CLOSING) {
			while (_curPutTime + INVSPEED < _vm->getTime()) {
				_curPutY -= 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY < -68)
				_state = CLOSED;
		}
	}

	if (_state == SELECTING) {
		int startx = (_nSelectObj + 1) * 64 - 20;
		int starty;

		if (!GLOBALS.bCfgInvUp)
			starty = RM_SY - 109;
		else
			starty = 70;

		// Make sure it is on one of the verbs
		if (mpos.y > starty && mpos.y < starty + 45) {
			if (mpos.x > startx && mpos.x < startx + 40) {
				if (miniAction != 1) {
					miniInterface.SetPattern(2);
					miniAction = 1;
					_vm->playUtilSFX(1);
				}
			} else if (mpos.x >= startx + 40 && mpos.x < startx + 80) {
				if (miniAction != 2) {
					miniInterface.SetPattern(3);
					miniAction = 2;
					_vm->playUtilSFX(1);
				}
			} else if (mpos.x >= startx + 80 && mpos.x < startx + 108) {
				if (miniAction != 3) {
					miniInterface.SetPattern(4);
					miniAction = 3;
					_vm->playUtilSFX(1);
				}
			} else {
				miniInterface.SetPattern(1);
				miniAction = 0;
			}
		} else  {
			miniInterface.SetPattern(1);
			miniAction = 0;
		}

		// Update the mini-interface
		miniInterface.doFrame(&bigBuf, false);
	}

	if ((_state != CLOSED) && !_nInList) {
		bigBuf.addPrim(new RMGfxPrimitive(this));
	}
}


bool RMInventory::itemInFocus(const RMPoint &mpt) {
	if ((_state == OPENED || _state == OPENING) && checkPointInside(mpt))
		return true;
	else
		return false;
}

RMItem *RMInventory::whichItemIsIn(const RMPoint &mpt) {
	int n;

	if (_state == OPENED) {
		if (checkPointInside(mpt)) {
			n = mpt.x / 64;
			if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0 && (!_bCombining || _inv[n - 1 + _curPos] != _nCombine))
				return &_items[_inv[n - 1 + _curPos]].icon;
		}
	}

	return NULL;
}



int RMInventory::getSaveStateSize(void) {
	//     m_inv   pattern   m_nInv
	return 256 * 4 + 256 * 4   +  4     ;
}

void RMInventory::saveState(byte *state) {
	int i, x;

	WRITE_LE_UINT32(state, _nInv);
	state += 4;
	Common::copy(_inv, _inv + 256, (uint32 *)state);
	state += 256 * 4;

	for (i = 0; i < 256; i++) {
		if (i < _nItems)
			x = _items[i].status;
		else
			x = 0;

		WRITE_LE_UINT32(state, x);
		state += 4;
	}
}

int RMInventory::loadState(byte *state) {
	int i, x;

	_nInv = READ_LE_UINT32(state);
	state += 4;
	Common::copy((uint32 *)state, (uint32 *)state + 256, _inv);
	state += 256 * 4;

	for (i = 0; i < 256; i++) {
		x = READ_LE_UINT32(state);
		state += 4;

		if (i < _nItems) {
			_items[i].status = x;
			_items[i].icon.SetPattern(x);
		}
	}

	_curPos = 0;
	_bCombining = false;
	
	_items[29].icon.SetPattern(1);

	if (_nInv > 8)
		_items[28].icon.SetPattern(2);
	else
		_items[28].icon.SetPattern(1);

	prepare();
	drawOT(Common::nullContext);
	clearOT();

	return getSaveStateSize();
}


/****************************************************************************\
*           RMInterface methods
\****************************************************************************/

RMInterface::~RMInterface() {

}

bool RMInterface::active() {
	return _bActive;
}

int RMInterface::onWhichBox(RMPoint pt) {
	int max, i;

	pt -= _openStart;

	// Check how many verbs you have to consider
	max = 4;
	if (_bPalesati) max = 5;

	// Find the verb
	for (i = 0; i < max; i++)
		if (_hotbbox[i].PtInRect(pt))
			return i;

	// Found no verb
	return -1;
}

void RMInterface::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	prim->Dst().TopLeft() = _openStart;
	CORO_INVOKE_2(RMGfxSourceBuffer8RLEByte::draw, bigBuf, prim);

	// Check if there is a draw hot zone
	_ctx->h = onWhichBox(_mpos);
	if (_ctx->h != -1) {
		prim->Dst().TopLeft() = _openStart;
		CORO_INVOKE_2(_hotzone[_ctx->h].draw, bigBuf, prim);

		if (_lastHotZone != _ctx->h) {
			_lastHotZone = _ctx->h;
			_vm->playUtilSFX(1);
		}

		if (GLOBALS.bCfgInterTips) {
			prim->Dst().TopLeft() = _openStart + RMPoint(70, 177);
			CORO_INVOKE_2(_hints[_ctx->h].draw, bigBuf, prim);
		}
	} else
		_lastHotZone = -1;

	CORO_END_CODE;
}


void RMInterface::doFrame(RMGfxTargetBuffer &bigBuf, RMPoint mousepos) {
	// If needed, add to the OT schedule list
	if (!_nInList && _bActive)
		bigBuf.addPrim(new RMGfxPrimitive(this));

	_mpos = mousepos;
}

void RMInterface::clicked(const RMPoint &mousepos) {
	_bActive = true;
	_openPos = mousepos;

	// Calculate the top left corner of the interface
	_openStart = _openPos - RMPoint(_dimx / 2, _dimy / 2);
	_lastHotZone = -1;

	// Keep it inside the screen
	if (_openStart.x < 0)
		_openStart.x = 0;
	if (_openStart.y < 0)
		_openStart.y = 0;
	if (_openStart.x + _dimx > RM_SX)
		_openStart.x = RM_SX - _dimx;
	if (_openStart.y + _dimy > RM_SY)
		_openStart.y = RM_SY - _dimy;

	// Play the sound effect
	_vm->playUtilSFX(0);
}

bool RMInterface::released(const RMPoint &mousepos, RMTonyAction &action) {
	if (!_bActive)
		return false;

	_bActive = false;

	switch (onWhichBox(mousepos)) {
	case 0:
		action = TA_TAKE;
		break;

	case 1:
		action = TA_TALK;
		break;

	case 2:
		action = TA_USE;
		break;

	case 3:
		action = TA_EXAMINE;
		break;

	case 4:
		action = TA_PALESATI;
		break;

	default:        // No verb
		return false;
	}

	return true;
}

void RMInterface::reset(void) {
	_bActive = false;
}

void RMInterface::setPalesati(bool bOn) {
	_bPalesati = bOn;
}

bool RMInterface::getPalesati(void) {
	return _bPalesati;
}

void RMInterface::init(void) {
	int i;
	RMResRaw inter(RES_I_INTERFACE);
	RMRes pal(RES_I_INTERPPAL);

	setPriority(191);

	RMGfxSourceBuffer::init(inter, inter.Width(), inter.Height());
	loadPaletteWA(RES_I_INTERPAL);

	for (i = 0; i < 5; i++) {
		RMResRaw part(RES_I_INTERP1 + i);

		_hotzone[i].init(part, part.Width(), part.Height());
		_hotzone[i].loadPaletteWA(pal);
	}

	_hotbbox[0].SetRect(126, 123, 159, 208);   // Take
	_hotbbox[1].SetRect(90, 130, 125, 186);    // About
	_hotbbox[2].SetRect(110, 60, 152, 125);
	_hotbbox[3].SetRect(56, 51, 93, 99);
	_hotbbox[4].SetRect(51, 105, 82, 172);

	_hints[0].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[1].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[2].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[3].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[4].setAlignType(RMText::HRIGHT, RMText::VTOP);

	// The text is taken from MPAL for translation
	RMMessage msg0(12);
	RMMessage msg1(13);
	RMMessage msg2(14);
	RMMessage msg3(15);
	RMMessage msg4(16);

	_hints[0].writeText(msg0[0], 1);   // Take
	_hints[1].writeText(msg1[0], 1);   // Talk
	_hints[2].writeText(msg2[0], 1);   // Use
	_hints[3].writeText(msg3[0], 1);   // Examine
	_hints[4].writeText(msg4[0], 1);   // Show Yourself

	_bActive = false;
	_bPalesati = false;
	_lastHotZone = 0;
}

void RMInterface::close(void) {
	int i;

	destroy();

	for (i = 0; i < 5; i++)
		_hotzone[i].destroy();
}

} // End of namespace Tony
