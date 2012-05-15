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

#ifndef TONY_GFXENGINE_H
#define TONY_GFXENGINE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "tony/mpal/memory.h"
#include "tony/game.h"
#include "tony/gfxcore.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/tonychar.h"
#include "tony/utils.h"

namespace Tony {

class RMGfxEngine {
private:
	RMGfxTargetBuffer m_bigBuf;
	RMInput m_input;
	RMPointer m_point;
	RMLocation m_loc;
	RMOptionScreen m_opt;
	RMTony m_tony;
	RMInventory m_inv;
	RMInterface m_inter;
	RMTextItemName m_itemName;

	bool m_bOption;
	bool m_bLocationLoaded;

	bool m_bInput;
	bool m_bAlwaysDrawMouse;

	int m_nCurLoc;
	RMTonyAction m_curAction;
	int m_curActionObj;
	OSystem::MutexRef csMainLoop;

	int m_nWipeType;
	uint32 m_hWipeEvent;
	int m_nWipeStep;

	bool m_bMustEnterMenu;
protected:
	static void ItemIrq(uint32 dwItem, int nPattern, int nStatus);
	void InitForNewLocation(int nLoc, RMPoint ptTonyStart, RMPoint start);
public:
	bool m_bWiping;
	Common::Rect m_rcWipeEllipse;
	bool m_bGUIOption;
	bool m_bGUIInterface;
	bool m_bGUIInventory;
public:
	RMGfxEngine();
	virtual ~RMGfxEngine();

	// Draw the next frame
	void DoFrame(CORO_PARAM, bool bDrawLocation);

	// Initialises the graphics engine
	void Init();

	// Closes the graphics engine
	void Close(void);

	// Warns when changing
	void SwitchFullscreen(bool bFull);

	// Warn that we are guided by the GDI
	void GDIControl(bool bCon);

	// Warns when entering or exits the options menu
	void OpenOptionScreen(CORO_PARAM, int type);

	// Enables or disables mouse input
	void EnableInput(void);
	void DisableInput(void);

	// Enables and disables mouse draw
	void EnableMouse(void);
	void DisableMouse(void);

	operator byte *() {
		return (byte *)m_bigBuf;
	}
	RMInput &GetInput() {
		return m_input;
	}

	// Link to the custom function list
	void InitCustomDll(void);

	// Link to graphic task
	void LinkGraphicTask(RMGfxTask *task) {
		m_bigBuf.AddPrim(new RMGfxPrimitive(task));
	};

	// Manage a location
	uint32 LoadLocation(int nLoc, RMPoint ptTonyStart, RMPoint start);
	void UnloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result);

	// Freeze and unfreeze
	void Freeze(void);
	void Unfreeze(void);

	// State management
	void SaveState(const Common::String &fn, byte *curThumb, const Common::String &name);
	void LoadState(CORO_PARAM, const Common::String &fn);

	// Selects a location
	void SelectLocation(const RMPoint &ptTonyStart = RMPoint(-1, -1), const RMPoint &start = RMPoint(-1, -1));

	// Pauses sound
	void PauseSound(bool bPause);

	// Wipe
	void InitWipe(int type);
	void CloseWipe(void);
	void WaitWipeEnd(CORO_PARAM);

	void SetPalesati(bool bpal) {
		m_inter.SetPalesati(bpal);
	}
	bool CanLoadSave();
};

} // End of namespace Tony

#endif
