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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

namespace Trecision {
/*------------------------------------------------
                    RegenInventory()
--------------------------------------------------*/
void TrecisionEngine::RegenInventory(uint8 StartIcon, uint8 StartLine) {
	if (StartLine > ICONDY)
		StartLine = ICONDY;

	for (uint16 b = 0; b < ICONDY; b++)
		wordset(g_vm->_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx, 0, SCREENLEN);

	for (uint16 a = 0; a < ICONSHOWN; a++) {
		if (g_vm->_inventory[a + StartIcon] >= LASTICON) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(g_vm->_video2 + (FIRSTLINE + b) * CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
					  Icone + (g_vm->_inventory[a + StartIcon] - LASTICON + READICON + 1) * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		} else if (g_vm->_inventory[a + StartIcon] != g_vm->_lightIcon) {
			for (uint16 b = 0; b < (ICONDY - StartLine); b++)
				MCopy(g_vm->_video2 + (FIRSTLINE + b) * CurRoomMaxX + a * (ICONDX) + ICONMARGSX + CurScrollPageDx,
					  Icone + g_vm->_inventory[a + StartIcon] * ICONDX * ICONDY + (b + StartLine) * ICONDX, ICONDX);
		}
	}

	// frecce
	if (StartIcon != 0) { // COPIA LA SINISTRA
		LeftArrow = ICONMARGSX * ICONDY * 3;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(g_vm->_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx,
				  Arrows + LeftArrow + (b + StartLine) * ICONMARGSX, ICONMARGSX);
		}
	}

	if ((StartIcon + ICONSHOWN) < g_vm->_inventorySize) { // COPIA LA DESTRA
		RightArrow = ICONMARGDX * ICONDY * 2;
		for (uint16 b = 0; b < (ICONDY - StartLine); b++) {
			MCopy(g_vm->_video2 + (FIRSTLINE + b) * CurRoomMaxX + CurScrollPageDx + SCREENLEN - ICONMARGDX,
				  Arrows + RightArrow + ICONMARGSX * ICONDY * 2 + (b + StartLine) * ICONMARGSX, ICONMARGSX);
		}
	}
	LeftArrow = 0;
	RightArrow = 0;

	//refreshSmkIcon( StartIcon, 2 );

	VMouseCopy();
	for (uint16 a = 0; a < ICONDY; a++) {
		_graphicsMgr->vCopy((FIRSTLINE + a) * VirtualPageLen + VideoScrollPageDx,
								  g_vm->_video2 + (FIRSTLINE + a) * CurRoomMaxX + CurScrollPageDx, SCREENLEN);
	}
	VMouseRestore();
	//VMouseON();
	_graphicsMgr->unlock();
}

/*-------------------------------------------------------------------------*/
/*                                 REGENINV            					   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::RegenInv(uint8 StartIcon, uint8 StartLine) {
	_regenInvStartIcon = StartIcon;
	_regenInvStartLine = StartLine;
}

/*-------------------------------------------------------------------------*/
/*                            ONELEFTINVENTORY          				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::OneLeftInventory() {
	if (_iconBase < _inventorySize - ICONSHOWN)
		_iconBase++;
	RegenInv(_iconBase, INVENTORY_SHOW);
	RightArrow = 1;
}

/*-------------------------------------------------------------------------*/
/*                            ONERIGHTINVENTORY          				   */
/*-------------------------------------------------------------------------*/
void TrecisionEngine::OneRightInventory() {
	if (_iconBase > 0)
		_iconBase--;
	RegenInv(_iconBase, INVENTORY_SHOW);
	LeftArrow = 1;
}

} // End of namespace Trecision
