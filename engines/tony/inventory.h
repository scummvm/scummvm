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
/**************************************************************************
 *                                     ออออออออออออออออออออออออออออออออออ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ออออออออออออออออออออออออออออออออออ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Inventory.CPP........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

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
	RMGfxSourceBuffer8RLEByteAA	*pointer;
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
	// Prepara l'immagine dell'inventario. Va richiamato ogni volta
	//  che l'inventario cambia
	void Prepare(void);

	// Controlla se la posizione Y del mouse ่ corretta, anche in base
	// alla posizione dell'inventario su schermo
	bool CheckPointInside(const RMPoint &pt);

public:
	RMInventory();
	virtual ~RMInventory();

	// Prepara un frame
	void DoFrame(RMGfxTargetBuffer& bigBuf, RMPointer &ptr, RMPoint mpos, bool bCanOpen);

	// Inizializzazione e chiusura
	void Init(void);
	void Close(void);
	void Reset(void);

	// Overload per la rimozione da otlist
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Overload per il disegno (per la posizione x&y e l'interfaccina)
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Metodo per determinare se l'inventario sta comandando gli input
	bool HaveFocus(const RMPoint &mpos);

	// Metodo per determinare se la mini interfaccia ่ attiva
	bool MiniActive(void);

	// Gestisce il click sinistro del mouse (solo quando c'ha focus)
	bool LeftClick(const RMPoint &mpos, int &nCombineObj);
	
	// Gestisce il tasto destro del mouse (solo quando c'ha focus)
	void RightClick(const RMPoint &mpos);
	bool RightRelease(const RMPoint &mpos, RMTonyAction &curAction);

	// Avverte che ่ finito il combine
	void EndCombine(void);
	
public:		
	// Aggiunta di un oggetto all'inventario
	void AddItem(int code);
	RMInventory& operator+=(RMItem *item) { AddItem(item->MpalCode()); return *this; }
	RMInventory& operator+=(RMItem &item) { AddItem(item.MpalCode()); return *this; }
	RMInventory& operator+=(int code) { AddItem(code); return *this; }

	// Rimozione di oggetto
	void RemoveItem(int code);

	// Siamo sopra un oggetto?
	RMItem *WhichItemIsIn(const RMPoint &mpt);
	bool ItemInFocus(const RMPoint &mpt);

	// Cambia l'icona di un oggetto
	void ChangeItemStatus(uint32 dwCode, uint32 dwStatus);

	// Salvataggio
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
	// Dice su quale zona calda si trova il punto	
	int OnWhichBox(RMPoint pt);

public:
	virtual ~RMInterface();

	// Il solito DoFrame (polling del motore grafico)
	void DoFrame(RMGfxTargetBuffer& bigBuf, RMPoint mousepos);		

	// TRUE se ่ attiva (non si pu๒ selezionare oggetti)		
	bool Active(); 

	// Inizializzazione
	void Init(void);
	void Close(void);

	// Resetta l'interfaccia
	void Reset(void);

	// Avverte dei click e rilasci del mouse
	void Clicked(const RMPoint &mousepos);
	bool Released(const RMPoint &mousepos, RMTonyAction &action);

	// Attiva o disattiva il quinto verbo
	void SetPalesati(bool bOn);
	bool GetPalesati(void);

	// Overloading del Draw per il posizionamente
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

} // End of namespace Tony

#endif
