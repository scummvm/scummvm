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

#ifndef TONY_INVENTORY_H
#define TONY_INVENTORY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "tony/font.h"
#include "tony/game.h"
#include "tony/gfxcore.h"
#include "tony/loc.h"

namespace Tony {

struct RMInventoryItem {
	RMItem icon;
	RMGfxSourceBuffer8RLEByteAA *pointer;
	int status;
};

class RMInventory : public RMGfxWoodyBuffer {
private:
	enum STATE {
		CLOSED,
		OPENING,
		OPENED,
		CLOSING,
		SELECTING
	};

protected:
	int m_nItems;
	RMInventoryItem *m_items;

	int m_inv[256];
	int m_nInv;
	int m_curPutY;
	uint32 m_curPutTime;

	int m_curPos;
	STATE m_state;
	bool m_bHasFocus;
	int m_nSelectObj;
	int m_nCombine;
	bool m_bCombining;

	bool m_bBlinkingRight, m_bBlinkingLeft;

	int miniAction;
	RMItem miniInterface;
	RMText m_hints[3];

	OSystem::MutexRef m_csModifyInterface;

protected:
	// Prepare the image inventory. It should be recalled whenever the inventory changes
	void Prepare(void);

	// Check if the mouse Y position is conrrect, even under the inventory portion of the screen
	bool CheckPointInside(const RMPoint &pt);

public:
	RMInventory();
	virtual ~RMInventory();

	// Prepare a frame
	void DoFrame(RMGfxTargetBuffer &bigBuf, RMPointer &ptr, RMPoint mpos, bool bCanOpen);

	// Initialisation and closing
	void Init(void);
	void Close(void);
	void Reset(void);

	// Overload test for removal from OT list
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Overload the drawing of the inventory
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Method for determining whether the inventory currently has the focus
	bool HaveFocus(const RMPoint &mpos);

	// Method for determining if the mini interface is active
	bool MiniActive(void);

	// Handle the left mouse click (only when the inventory has the focus)
	bool LeftClick(const RMPoint &mpos, int &nCombineObj);

	// Handle the right mouse button (only when the inventory has the focus)
	void RightClick(const RMPoint &mpos);
	bool RightRelease(const RMPoint &mpos, RMTonyAction &curAction);

	// Warn that an item combine is over
	void EndCombine(void);

public:
	// Add an item to the inventory
	void AddItem(int code);
	RMInventory &operator+=(RMItem *item) {
		AddItem(item->MpalCode());
		return *this;
	}
	RMInventory &operator+=(RMItem &item) {
		AddItem(item.MpalCode());
		return *this;
	}
	RMInventory &operator+=(int code) {
		AddItem(code);
		return *this;
	}

	// Removes an item
	void RemoveItem(int code);

	// We are on an object?
	RMItem *WhichItemIsIn(const RMPoint &mpt);
	bool ItemInFocus(const RMPoint &mpt);

	// Change the icon of an item
	void ChangeItemStatus(uint32 dwCode, uint32 dwStatus);

	// Save methods
	int GetSaveStateSize(void);
	void SaveState(byte *state);
	int LoadState(byte *state);
};


class RMInterface : public RMGfxSourceBuffer8RLEByte {
private:
	bool m_bActive;
	RMPoint m_mpos;
	RMPoint m_openPos;
	RMPoint m_openStart;
	RMText m_hints[5];
	RMGfxSourceBuffer8RLEByte m_hotzone[5];
	RMRect m_hotbbox[5];
	bool m_bPalesati;
	int m_lastHotZone;

protected:
	// Return which box a given point is in
	int OnWhichBox(RMPoint pt);

public:
	virtual ~RMInterface();

	// The usual DoFrame (poll the graphics engine)
	void DoFrame(RMGfxTargetBuffer &bigBuf, RMPoint mousepos);

	// TRUE if it is active (you can select items)
	bool Active();

	// Initialisation
	void Init(void);
	void Close(void);

	// Reset the interface
	void Reset(void);

	// Warns of mouse clicks and releases
	void Clicked(const RMPoint &mousepos);
	bool Released(const RMPoint &mousepos, RMTonyAction &action);

	// Enalbes or disables the fifth verb
	void SetPalesati(bool bOn);
	bool GetPalesati(void);

	// Overloaded Draw
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

} // End of namespace Tony

#endif
