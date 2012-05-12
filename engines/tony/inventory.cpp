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

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/inventory.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {


/****************************************************************************\
* 			RMInventory Methods
\****************************************************************************/

RMInventory::RMInventory() {
	m_items = NULL;
	m_state = CLOSED;
	m_bCombining = false;
	m_csModifyInterface = g_system->createMutex();
	m_nItems = 0;

	Common::fill(m_inv, m_inv + 256, 0);
	m_nInv = 0;
	m_curPutY = 0;
	m_curPutTime = 0;
	m_curPos = 0;
	m_bHasFocus = false;
	m_nSelectObj = 0;
	m_nCombine = 0;
	m_bBlinkingRight = false;
	m_bBlinkingLeft = false;
	miniAction = 0;
}

RMInventory::~RMInventory() {
	Close();
	g_system->deleteMutex(m_csModifyInterface);
}

bool RMInventory::CheckPointInside(RMPoint &pt) {
	if (!bCfgInvUp)
		return pt.y > RM_SY - 70;
	else
		return pt.y < 70;
}



void RMInventory::Init(void) {
	int i,j;
	int curres;

	// Crea il buffer principale
	Create(RM_SX, 68);
	SetPriority(185);

	// Pulisce l'inventario
	m_nInv = 0;
	m_curPos = 0;
	m_bCombining = false;

	// Nuovi oggetti
	m_nItems = 78;	// @@@ Numero di oggetti prendibili
	m_items = new RMInventoryItem[m_nItems + 1];

	curres = 10500;

	// Loop sugli oggetti
	for (i = 0; i <= m_nItems; i++) {
		// Carica l'oggetto da risorsa
		RMRes res(curres);
		RMDataStream ds;

		assert(res.IsValid());

		// Non deve inizializzare il cur pattern dell'oggetto dell'inventario leggendolo da MPAL!
		// Glelo mettiamo noi a MANO, e non c'entra nulla con l'oggetto in MPAL
		m_items[i].icon.SetInitCurPattern(false);
		ds.OpenBuffer(res);
		ds >> m_items[i].icon;
		ds.Close();

		// Mette di default a pattern 1
		m_items[i].pointer = NULL;
		m_items[i].status = 1;
		m_items[i].icon.SetPattern(1);
		m_items[i].icon.DoFrame(this, false);

		curres++;
		if (i == 0 || i == 28 || i == 29) continue;
		
		m_items[i].pointer = new RMGfxSourceBuffer8RLEByteAA[m_items[i].icon.NumPattern()];

		for (j = 0; j < m_items[i].icon.NumPattern(); j++) {
			RMResRaw raw(curres);

			assert(raw.IsValid());
			
			m_items[i].pointer[j].Init((const byte *)raw, raw.Width(), raw.Height(), true);
			curres++;
		}
	}

	m_items[28].icon.SetPattern(1);
	m_items[29].icon.SetPattern(1);

	// Carica l'interfaccina
	RMDataStream ds;
	RMRes res(RES_I_MINIINTER);
	assert(res.IsValid());
	ds.OpenBuffer(res);
	ds >> miniInterface;
	miniInterface.SetPattern(1);
	ds.Close();

	// Crea il testo per gli hints sulla mini interfaccia
	m_hints[0].SetAlignType(RMText::HCENTER,RMText::VTOP);
	m_hints[1].SetAlignType(RMText::HCENTER,RMText::VTOP);
	m_hints[2].SetAlignType(RMText::HCENTER,RMText::VTOP);

	// Il testo viene preso da MPAL per la traduzione
	RMMessage msg1(15);
	RMMessage msg2(13);
	RMMessage msg3(14);

	m_hints[0].WriteText(msg1[0],1);
	m_hints[1].WriteText(msg2[0],1);
	m_hints[2].WriteText(msg3[0],1);

/*
	m_hints[0].WriteText("Examine",1);
	m_hints[1].WriteText("Talk",1);
	m_hints[2].WriteText("Use",1);
*/

	// Prepara il primo inventario
	Prepare();
	DrawOT();
	ClearOT();
}

void RMInventory::Close(void) {
	// Ciao memoria 
	if (m_items != NULL) {
		// Delete the item pointers
		for (int i = 0; i <= m_nItems; i++)
			delete[] m_items[i].pointer;

		// Delete the items array
		delete[] m_items;
		m_items = NULL;
	}

	Destroy();
}

void RMInventory::Reset(void) {
	m_state = CLOSED;
	EndCombine();
}

void RMInventory::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_state == OPENING || m_state == CLOSING)
		prim->SetDst(RMPoint(0, m_curPutY));
	else
		prim->SetDst(RMPoint(0, m_curPutY));

	g_system->lockMutex(m_csModifyInterface);
	RMGfxWoodyBuffer::Draw(bigBuf, prim);
	g_system->unlockMutex(m_csModifyInterface);

	if (m_state == SELECTING) {
		RMPoint pos;
		RMPoint pos2;

		if (!bCfgInvUp) {
			pos.Set((m_nSelectObj+1)*64 - 20,RM_SY - 113);
			pos2.Set((m_nSelectObj+1)*64 + 34,RM_SY - 150);
		} else {
			pos.Set((m_nSelectObj+1)*64 - 20, 72 - 4); // la parte marrone sta in alto :(
			pos2.Set((m_nSelectObj+1)*64 + 34, 119 - 4);
		}
		
		RMGfxPrimitive p(prim->m_task, pos);
		RMGfxPrimitive p2(prim->m_task, pos2);

		// Disegna l'interfaccina stupida
		miniInterface.Draw(bigBuf,&p);

		if (bCfgInterTips) {
			if (miniAction == 1) // Esamina
				m_hints[0].Draw(bigBuf, &p2);
			else if (miniAction == 2) // Parla
				m_hints[1].Draw(bigBuf, &p2);
			else if (miniAction == 3) // Usa
				m_hints[2].Draw(bigBuf, &p2);
		}
	}
}

bool RMInventory::RemoveThis(void) {
	if (m_state == CLOSED)
		return true;

	return false;
}

void RMInventory::RemoveItem(int code) {
	int i;

	for (i=0;i<m_nInv;i++)
		if (m_inv[i] == code - 10000) {
			g_system->lockMutex(m_csModifyInterface);

			Common::copy_backward(&m_inv[i + 1], &m_inv[i + 1] + (m_nInv - i), &m_inv[i]);
//			m_inv[m_nInv-1]=0;
			m_nInv--;
	
			Prepare();
			DrawOT();
			ClearOT();
			g_system->unlockMutex(m_csModifyInterface);
			return;
		}

	//MessageBox(NULL,"Specified object is not in the inventory","INTERNAL ERROR",MB_OK|MB_ICONEXCLAMATION);
}

void RMInventory::AddItem(int code) {
	if (code <= 10000 && code >= 10101) {
		// Se siamo qui, vuol dire che stiamo aggiungendo un oggetto che non dovrebbe essere
		// nell'inventario
		warning("Cannot find a valid icon for this item, and then it will not be added to the inventory");
	} else {
		g_system->lockMutex(m_csModifyInterface);
		if (m_curPos + 8 == m_nInv)	{
			// Sfondiamo l'inventario! Attivo il pattern di lampeggio
			m_items[28].icon.SetPattern(2);
		}

		m_inv[m_nInv++]=code-10000;

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	}
}

void RMInventory::ChangeItemStatus(uint32 code, uint32 dwStatus) {
	if (code <= 10000 && code >= 10101) {
		error("Specified object code is not valid");
	} else {
		g_system->lockMutex(m_csModifyInterface);
		m_items[code - 10000].icon.SetPattern(dwStatus);
		m_items[code - 10000].status = dwStatus;

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	}
}


void RMInventory::Prepare(void)
{
	int i;

	for (i = 1; i < RM_SX / 64 - 1; i++) {
		if (i - 1 + m_curPos < m_nInv)
			AddPrim(new RMGfxPrimitive(&m_items[m_inv[i - 1 + m_curPos]].icon, RMPoint(i * 64, 0)));
		else
			AddPrim(new RMGfxPrimitive(&m_items[0].icon, RMPoint(i * 64, 0)));
	}

	// Frecce
	AddPrim(new RMGfxPrimitive(&m_items[29].icon, RMPoint(0, 0)));
	AddPrim(new RMGfxPrimitive(&m_items[28].icon, RMPoint(640 - 64, 0)));

	//AddPrim(new RMGfxPrimitive(&m_items[0].icon,RMPoint(0,0)));
}

bool RMInventory::MiniActive(void) {
	return m_state == SELECTING;
}

bool RMInventory::HaveFocus(RMPoint mpos) {
	// In fase di combine abbiamo il focus solo se siamo su una freccia (per poter scrollare)
	if (m_state == OPENED && m_bCombining && CheckPointInside(mpos) && (mpos.x < 64 || mpos.x > RM_SX - 64))
		return true;

	// Se l'inventario ่ aperto, abbiamo il focus quando ci andiamo sopra
	if (m_state == OPENED && !m_bCombining && CheckPointInside(mpos))
		return true;
 
	// Se stiamo selezionando un verbo (quindi tasto destro premuto), abbiamo il focus alltime
	if (m_state == SELECTING)
		return true;

	return false;
}

void RMInventory::EndCombine(void) {
	m_bCombining = false;
}

bool RMInventory::LeftClick(RMPoint mpos, int& nCombineObj) {
	int n;

	// Il click sinistro prende in mano un oggetto dell'inventario per utilizzarlo con lo sfondo
	n=mpos.x / 64;

	if (m_state == OPENED) {
		if (n > 0 && n < RM_SX / 64 - 1 && m_inv[n - 1 + m_curPos] != 0) {
			m_bCombining = true; //m_state=COMBINING;
			m_nCombine = m_inv[n - 1 + m_curPos];
			nCombineObj = m_nCombine + 10000;

			_vm->PlayUtilSFX(1);
			return true;
		}
	}
	
	// Click sulla freccia destra
	if ((m_state == OPENED) && m_bBlinkingRight) {
		g_system->lockMutex(m_csModifyInterface);
		m_curPos++;

		if (m_curPos+8 >= m_nInv) {
			m_bBlinkingRight = false;
			m_items[28].icon.SetPattern(1);
		}

		if (m_curPos > 0) {
			m_bBlinkingLeft = true;
			m_items[29].icon.SetPattern(2);
		}

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	}
	// Click sulla freccia sinistra
	else if ((m_state == OPENED) && m_bBlinkingLeft) {
		assert(m_curPos>0);
		g_system->lockMutex(m_csModifyInterface);
		m_curPos--;

		if (m_curPos == 0) {
			m_bBlinkingLeft = false;
			m_items[29].icon.SetPattern(1);
		}

		if (m_curPos + 8 < m_nInv) {
			m_bBlinkingRight = true;
			m_items[28].icon.SetPattern(2);
		}

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	}


	return false;
}


void RMInventory::RightClick(RMPoint mpos) {
	int n;

	assert(CheckPointInside(mpos));

	if (m_state == OPENED && !m_bCombining) {
		// Apre l'interfaccina contestuale
		n = mpos.x / 64;

		if (n > 0 && n < RM_SX / 64 - 1 && m_inv[n - 1 + m_curPos] != 0) {	
			m_state = SELECTING;
			miniAction = 0;
			m_nSelectObj = n - 1;

			_vm->PlayUtilSFX(0);
		}
	}

	if ((m_state == OPENED) && m_bBlinkingRight) {
		g_system->lockMutex(m_csModifyInterface);
		m_curPos += 7;
		if (m_curPos + 8 > m_nInv)
			m_curPos = m_nInv - 8;

		if (m_curPos + 8 <= m_nInv) {
			m_bBlinkingRight = false;
			m_items[28].icon.SetPattern(1);
		}

		if (m_curPos>0) {
			m_bBlinkingLeft = true;
			m_items[29].icon.SetPattern(2);
		}

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	} else if ((m_state == OPENED) && m_bBlinkingLeft) {
		assert(m_curPos > 0);
		g_system->lockMutex(m_csModifyInterface);
		m_curPos -= 7;
		if (m_curPos < 0) m_curPos = 0;

		if (m_curPos == 0) {
			m_bBlinkingLeft = false;
			m_items[29].icon.SetPattern(1);
		}

		if (m_curPos + 8 < m_nInv) {
			m_bBlinkingRight = true;
			m_items[28].icon.SetPattern(2);
		}

		Prepare();
		DrawOT();
		ClearOT();
		g_system->unlockMutex(m_csModifyInterface);
	}
}

bool RMInventory::RightRelease(RMPoint mpos, RMTonyAction& curAction) {
	if (m_state == SELECTING) {
		m_state = OPENED;

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

#define INVSPEED	20

void RMInventory::DoFrame(RMGfxTargetBuffer &bigBuf, RMPointer &ptr, RMPoint mpos, bool bCanOpen) {
	int i;
	bool bNeedRedraw = false;

	if (m_state != CLOSED) {
		// Pulisce l'OTList
		g_system->lockMutex(m_csModifyInterface);
		ClearOT();

		// Fa il DoFrame di tutti gli oggetti contenuti attualmente nell'inventario
		// @@@ forse bisognerebbe farlo di tutti gli oggetti takeable? Probabilmente non serve a nulla
		for (i = 0; i < m_nInv; i++)
			if (m_items[m_inv[i]].icon.DoFrame(this, false) && (i >= m_curPos && i <= m_curPos + 7))
				bNeedRedraw = true;

		if ((m_state == CLOSING || m_state == OPENING || m_state == OPENED) && CheckPointInside(mpos)) {
			if (mpos.x > RM_SX - 64) {
				if (m_curPos + 8 < m_nInv && !m_bBlinkingRight) {
					m_items[28].icon.SetPattern(3);
					m_bBlinkingRight = true;
					bNeedRedraw = true;
				}
			} else if (m_bBlinkingRight) {
				m_items[28].icon.SetPattern(2);
				m_bBlinkingRight = false;
				bNeedRedraw = true;
			}

			if (mpos.x < 64) {
				if (m_curPos > 0 && !m_bBlinkingLeft) {
					m_items[29].icon.SetPattern(3);
					m_bBlinkingLeft = true;
					bNeedRedraw = true;
   				}
			} else if (m_bBlinkingLeft) {
				m_items[29].icon.SetPattern(2);
				m_bBlinkingLeft = false;
				bNeedRedraw = true;
			}
		}

		if (m_items[28].icon.DoFrame(this, false))
			bNeedRedraw = true;

		if (m_items[29].icon.DoFrame(this, false))
			bNeedRedraw = true;

		if (bNeedRedraw)
			Prepare();

		g_system->unlockMutex(m_csModifyInterface);
	}
 
	if ((GetAsyncKeyState(Common::KEYCODE_i) & 0x8001) == 0x8001) {
		bCfgInvLocked = !bCfgInvLocked;
	}

	if (m_bCombining) {//m_state == COMBINING) 
		ptr.SetCustomPointer(&m_items[m_nCombine].pointer[m_items[m_nCombine].status - 1]);
		ptr.SetSpecialPointer(RMPointer::PTR_CUSTOM);
	}

	if (!bCfgInvUp) {
		if ((m_state == CLOSED) && (mpos.y > RM_SY - 10 || bCfgInvLocked) && bCanOpen) {
			if (!bCfgInvNoScroll) {
				m_state = OPENING;
				m_curPutY = RM_SY - 1;
				m_curPutTime=_vm->GetTime();
			} else {
				m_state = OPENED;
				m_curPutY = RM_SY - 68;
			}
		} else if (m_state == OPENED) { 
			if ((mpos.y < RM_SY - 70 && !bCfgInvLocked) || !bCanOpen) {
				if (!bCfgInvNoScroll) {
					m_state = CLOSING;
					m_curPutY = RM_SY - 68;
					m_curPutTime = _vm->GetTime();
				} else {
					m_state = CLOSED;
				}
			}
		} else if (m_state == OPENING) {
			while (m_curPutTime + INVSPEED < _vm->GetTime()) {
				m_curPutY -= 3;
				m_curPutTime += INVSPEED;
			}

			if (m_curPutY <= RM_SY - 68) {
				m_state = OPENED;
				m_curPutY = RM_SY - 68;
			}

		} else if (m_state == CLOSING) {
			while (m_curPutTime + INVSPEED < _vm->GetTime()) {
				m_curPutY += 3;
				m_curPutTime += INVSPEED;
			}

			if (m_curPutY > 480)
				m_state = CLOSED;
		}
	} else {
		if ((m_state == CLOSED) && (mpos.y<10 || bCfgInvLocked) && bCanOpen) {
			if (!bCfgInvNoScroll) {
				m_state = OPENING;
				m_curPutY =- 68;
				m_curPutTime = _vm->GetTime();
			} else {
				m_state = OPENED;
				m_curPutY = 0;
			}
		}
		else if (m_state == OPENED) { 
			if ((mpos.y>70 && !bCfgInvLocked) || !bCanOpen) {
				if (!bCfgInvNoScroll) {
					m_state = CLOSING;
					m_curPutY = -2;
					m_curPutTime = _vm->GetTime();
				} else {
					m_state = CLOSED;
				}
			}
		} else if (m_state == OPENING) {
			while (m_curPutTime + INVSPEED < _vm->GetTime()) {
				m_curPutY += 3;
				m_curPutTime += INVSPEED;
			}

			if (m_curPutY >= 0) {
				m_state = OPENED;
				m_curPutY = 0;
			}
		} else if (m_state == CLOSING) {
			while (m_curPutTime + INVSPEED < _vm->GetTime()) {
				m_curPutY -= 3;
				m_curPutTime += INVSPEED;
			}

			if (m_curPutY < -68)
				m_state = CLOSED;
		}
	}

	if (m_state == SELECTING) {
		int startx = (m_nSelectObj + 1) * 64 - 20;
		int starty;
		
		if (!bCfgInvUp)
			starty=RM_SY-109;
		else
			starty=70;
		
		// Controlla se si trova su uno dei verbi
		if (mpos.y > starty && mpos.y < starty + 45) {
			if (mpos.x > startx && mpos.x < startx + 40) { 	
				if (miniAction !=1 ) {
					miniInterface.SetPattern(2);
					miniAction = 1;
	  				_vm->PlayUtilSFX(1);
				}
			} else if (mpos.x >= startx + 40 && mpos.x < startx + 80) {
				if (miniAction != 2) {
					miniInterface.SetPattern(3);
					miniAction = 2;
					_vm->PlayUtilSFX(1);
     			}
			} else if (mpos.x >= startx + 80 && mpos.x < startx + 108) {
				if (miniAction != 3) {
					miniInterface.SetPattern(4);
					miniAction = 3;
    				_vm->PlayUtilSFX(1);
				}
			} else {
				miniInterface.SetPattern(1);
				miniAction = 0;
			}
		} else  {
			miniInterface.SetPattern(1);
			miniAction = 0;
		}
	
		// Aggiorna la miniinterface
		miniInterface.DoFrame(&bigBuf, false);
	}

	if ((m_state!= CLOSED) && !m_nInList) {
		bigBuf.AddPrim(new RMGfxPrimitive(this));
	}
}


bool RMInventory::ItemInFocus(RMPoint mpt) {
	if ((m_state == OPENED || m_state == OPENING) && CheckPointInside(mpt))
		return true;
	else
		return false;
}

RMItem *RMInventory::WhichItemIsIn(RMPoint mpt) {
	int n;

	if (m_state == OPENED) {
		if (CheckPointInside(mpt)) {
			n=mpt.x / 64;
			if (n>0 && n < RM_SX / 64 - 1 && m_inv[n - 1 + m_curPos] != 0 && (!m_bCombining || m_inv[n - 1 + m_curPos] != m_nCombine))
				return &m_items[m_inv[n - 1 + m_curPos]].icon;
		} 
	}

	return NULL;
}



int RMInventory::GetSaveStateSize(void) {
	//     m_inv   pattern   m_nInv
	return 256*4 + 256*4   +  4     ;
}

void RMInventory::SaveState(byte *state) {
	int i, x;

	WRITE_LE_UINT32(state, m_nInv); state += 4;
	Common::copy(m_inv, m_inv + 256, (uint32 *)state); state += 256 * 4;

	for (i = 0; i < 256; i++) {
		if (i < m_nItems)
			x = m_items[i].status;
		else
			x = 0;

		WRITE_LE_UINT32(state, x); state += 4;
	}
}

int RMInventory::LoadState(byte *state) {
	int i, x;

	m_nInv = READ_LE_UINT32(state); state += 4;
	Common::copy((uint32 *)state, (uint32 *)state + 256, m_inv); state += 256 * 4;

	for (i = 0; i < 256; i++) {
		x = READ_LE_UINT32(state);	state += 4;
		
		if (i < m_nItems) {
			m_items[i].status = x;
			m_items[i].icon.SetPattern(x);
		}
	}

	m_curPos = 0;
	m_bCombining = false;

	m_items[29].icon.SetPattern(1);
	
	if (m_nInv > 8)
		m_items[28].icon.SetPattern(2);
	else
		m_items[28].icon.SetPattern(1);

	Prepare();
	DrawOT();
	ClearOT();

	return GetSaveStateSize();
}


/****************************************************************************\
* 			RMInterface methods
\****************************************************************************/

RMInterface::~RMInterface() {

}

bool RMInterface::Active() {
	return m_bActive;
}

int RMInterface::OnWhichBox(RMPoint pt) {
	int max, i;

	pt -= m_openStart;

	// Controlla quanti verbi bisogna considerare
	max = 4; if (m_bPalesati) max = 5;
	
	// Cerca il verbo
	for (i = 0; i < max; i++)
		if (m_hotbbox[i].PtInRect(pt))
			return i;

	// Nessun verbo trovato
	return -1;
}

void RMInterface::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	int h;

	prim->Dst().TopLeft() = m_openStart;
	RMGfxSourceBuffer8RLEByte::Draw(bigBuf, prim);

	// Controlla se c'e' da disegnare una zona calda
	h = OnWhichBox(m_mpos);
	if (h != -1) {
		prim->Dst().TopLeft() = m_openStart;
		m_hotzone[h].Draw(bigBuf, prim);
		
		if (m_lastHotZone != h) {
			m_lastHotZone = h;
			_vm->PlayUtilSFX(1);
		}

		if (bCfgInterTips) {
			prim->Dst().TopLeft() = m_openStart + RMPoint(70, 177);
			m_hints[h].Draw(bigBuf,prim);
		}
	} else
		m_lastHotZone = -1;

}


void RMInterface::DoFrame(RMGfxTargetBuffer &bigBuf, RMPoint mousepos) {
	// Se c'่ bisogna, schedula nella OT list
	if (!m_nInList && m_bActive)
		bigBuf.AddPrim(new RMGfxPrimitive(this));

	m_mpos = mousepos;
}

void RMInterface::Clicked(RMPoint mousepos) {
	m_bActive = true;
	m_openPos = mousepos;

	// Calcola l'angolo in alto a sinistra dell'interfaccia
	m_openStart = m_openPos - RMPoint(m_dimx / 2, m_dimy / 2);
	m_lastHotZone = -1;

	// La tiene dentro lo schermo
	if (m_openStart.x < 0) m_openStart.x = 0;
	if (m_openStart.y < 0) m_openStart.y = 0;
	if (m_openStart.x+m_dimx > RM_SX) m_openStart.x = RM_SX - m_dimx;
	if (m_openStart.y+m_dimy > RM_SY) m_openStart.y = RM_SY - m_dimy;

	// Play dell'effetto sonoro
	_vm->PlayUtilSFX(0);
}

bool RMInterface::Released(RMPoint mousepos, RMTonyAction &action) {
	if (!m_bActive)
		return false;

	m_bActive = false;

	switch (OnWhichBox(mousepos)) {
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

	default:		// Nessun verbo
		return false;
	} 
	
	return true;	
}

void RMInterface::Reset(void) {
	m_bActive = false;
}

void RMInterface::SetPalesati(bool bOn) {
	m_bPalesati=bOn;
}

bool RMInterface::GetPalesati(void) {
	return m_bPalesati;
}

void RMInterface::Init(void) {
	int i;
	RMResRaw inter(RES_I_INTERFACE);	
	RMRes pal(RES_I_INTERPPAL);

	SetPriority(191);

	RMGfxSourceBuffer::Init(inter, inter.Width(), inter.Height());
	LoadPaletteWA(RES_I_INTERPAL);

	for (i = 0; i < 5; i++) {
		RMResRaw part(RES_I_INTERP1 + i);

		m_hotzone[i].Init(part, part.Width(), part.Height());
		m_hotzone[i].LoadPaletteWA(pal);
	}

	m_hotbbox[0].SetRect(126, 123, 159, 208);	// prendi
	m_hotbbox[1].SetRect(90, 130, 125, 186);	// parla
	m_hotbbox[2].SetRect(110, 60, 152, 125);
	m_hotbbox[3].SetRect(56, 51, 93, 99);
	m_hotbbox[4].SetRect(51, 105, 82, 172);

	m_hints[0].SetAlignType(RMText::HRIGHT, RMText::VTOP);
	m_hints[1].SetAlignType(RMText::HRIGHT, RMText::VTOP);
	m_hints[2].SetAlignType(RMText::HRIGHT, RMText::VTOP);
	m_hints[3].SetAlignType(RMText::HRIGHT, RMText::VTOP);
	m_hints[4].SetAlignType(RMText::HRIGHT, RMText::VTOP);

	// Il testo viene preso da MPAL per la traduzione
	RMMessage msg0(12);
	RMMessage msg1(13);
	RMMessage msg2(14);
	RMMessage msg3(15);
	RMMessage msg4(16);

	m_hints[0].WriteText(msg0[0], 1);
	m_hints[1].WriteText(msg1[0], 1);
	m_hints[2].WriteText(msg2[0], 1);
	m_hints[3].WriteText(msg3[0], 1);
	m_hints[4].WriteText(msg4[0], 1);
/*
	m_hints[0].WriteText("Take",1);
	m_hints[1].WriteText("Talk",1);
	m_hints[2].WriteText("Use",1);
	m_hints[3].WriteText("Examine",1);
	m_hints[4].WriteText("Palesati",1);
*/
	m_bActive = false;
	m_bPalesati = false;
	m_lastHotZone = 0;
}

void RMInterface::Close(void) {
	int i;

	Destroy();

	for (i = 0; i < 5; i++)
		m_hotzone[i].Destroy();
}

} // End of namespace Tony
