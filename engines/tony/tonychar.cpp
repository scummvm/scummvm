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
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  TonyChar.CPP.........  *
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

#include "tony/mpal/memory.h"
#include "tony/mpal/mpalutils.h"
#include "tony/game.h"
#include "tony/tonychar.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       Metodi di RMTony
\****************************************************************************/

bool RMTony::m_bAction = false;

uint32 RMTony::WaitEndOfAction(HANDLE hThread) {
	WaitForSingleObject(hThread, INFINITE);
	m_bAction = false;

	return 1;
}

RMGfxSourceBuffer *RMTony::NewItemSpriteBuffer(int dimx, int dimy, bool bPreRLE) {
	RMGfxSourceBuffer8RLE *spr;
	
	assert(m_cm == CM_256);
	spr = new RMGfxSourceBuffer8RLEByteAA;
	spr->SetAlphaBlendColor(1);
	if (bPreRLE)
		spr->SetAlreadyCompressed();
	return spr;
}

void RMTony::Init(void) {
	RMRes tony(0);
	RMRes body(9999);
	RMDataStream ds;

	// Apre il buffer
	ds.OpenBuffer(tony);

	// Legge dallo stream l'oggetto corrente (cio่ Tony)
	ReadFromStream(ds, true);	// da OGX
	
	// Chiude il buffer
	ds.Close();

	// Legge il corpo di Tony
	ds.OpenBuffer(body);
	m_body.ReadFromStream(ds, true); // da OGX
	ds.Close();
	m_body.SetPattern(0);

	// Mostra Tony di default
	m_bShow=m_bShowOmbra = true;

  // Nessuna azione in attesa
	m_bActionPending = false;
	m_bAction = false;

	m_bPastorella = false;
	m_bIsTalking = false;
	m_bIsStaticTalk = false;

	m_nTimeLastStep = _vm->GetTime();
}


void RMTony::Close(void) {
	// Disalloca @@@ Manca la disallocazione di un item
	m_ombra.Destroy();
}

void RMTony::DoFrame(RMGfxTargetBuffer *bigBuf, int curLoc) {
	if (!m_nInList && m_bShow)
		bigBuf->AddPrim(new RMGfxPrimitive(this));
	
	SetSpeed(nCfgTonySpeed);	

	// Esegue il movimento normale del personaggio
	int time = _vm->GetTime();

	do 	{
		m_nTimeLastStep += (1000 / 40);
		RMCharacter::DoFrame(bigBuf, curLoc);

	} while (time > m_nTimeLastStep + (1000 / 40));

	// Controlla se siamo alla fine del percorso
	if (EndOfPath() && m_bActionPending) {
		// Bisogna eseguire l'azione sulla quale abbiamo clickato
		m_bActionPending = false;	
	}

	if (m_bIsTalking || m_bIsStaticTalk)
		m_body.DoFrame(bigBuf, false);
}

void RMTony::Show(void) {
	m_bShow = true;
	m_bShowOmbra = true;
}

void RMTony::Hide(bool bShowOmbra) {
	m_bShow = false;
	m_bShowOmbra = bShowOmbra;
}


void RMTony::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Richiama il Draw() della classe madre se tony ่ visibile
	if (m_bShow && bDrawNow) 	{
		if (m_bCorpoDavanti) 		{
			prim->Dst().SetEmpty();
			prim->Dst().Offset(-44, -134);
			if (m_bPastorella)
				prim->Dst().Offset(1, 4);
			RMCharacter::Draw(bigBuf, prim);
		}

		if (m_bIsTalking || m_bIsStaticTalk) {
			// Offset inverso per lo scrolling
			prim->Dst().SetEmpty();
			prim->Dst().Offset(-m_curScroll);
    		prim->Dst().Offset(m_pos);
			prim->Dst().Offset(-44, -134);
			prim->Dst() += m_nBodyOffset;	
			m_body.Draw(bigBuf, prim);
		}

		if (!m_bCorpoDavanti) {
			prim->Dst().SetEmpty();
			prim->Dst().Offset(-44, -134);
			if (m_bPastorella)
				prim->Dst().Offset(0, 3);
			RMCharacter::Draw(bigBuf, prim);
		}
	}
}

void RMTony::MoveAndDoAction(RMPoint dst, RMItem *item, int nAction, int nActionParm) {
	// Fa il movimento normale, ma si ricorda se deve poi eseguire un azione
	if (item == NULL) {
		m_bActionPending = false;
		m_ActionItem = NULL;
	} else {
		m_ActionItem = item;
		m_Action = nAction;
		m_ActionParm = nActionParm;
		m_bActionPending = true;
	}

	if (!RMCharacter::Move(dst)) {
		m_bActionPending = false;
		m_ActionItem = NULL;
	}
}


void RMTony::ExecuteAction(int nAction, int nActionItem, int nParm) {
	HANDLE hThread;
	
	if (nAction == TA_COMBINE) {
		hThread = mpalQueryDoAction(TA_COMBINE, nParm, nActionItem);
		
		// Se ่ fallito il combine, proviamo con il ReceiveCombine
		if (hThread == INVALID_HANDLE_VALUE) {
			hThread = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, nParm); 
			
			// Se ่ fallito il receive, andiamo con quelli generici
			// @@@ CombineGive!
			if (hThread == INVALID_HANDLE_VALUE) {
				hThread = mpalQueryDoAction(TA_COMBINE, nParm, 0);
				
				if (hThread == INVALID_HANDLE_VALUE){
					hThread = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, 0);
				}
			}
		}
	} else {
		// Esegue l'azione
		hThread = mpalQueryDoAction(nAction, nActionItem, 0); 
	}
					
	if (hThread != INVALID_HANDLE_VALUE) {
		uint32 id;
		m_bAction = true;
		CreateThread(NULL, 10240,(LPTHREAD_START_ROUTINE)WaitEndOfAction, (void *)hThread, 0, &id);
		hActionThread = hThread;
	} else if (nAction != TA_GOTO) {
 		uint32 id;

		if (nAction == TA_TALK) {
			hThread = mpalQueryDoAction(6, 1, 0); 
			m_bAction = true;
			CreateThread(NULL,10240,(LPTHREAD_START_ROUTINE)WaitEndOfAction, (void *)hThread,0,&id);
  			hActionThread=hThread;
		} else if (nAction == TA_PALESATI) {
			hThread = mpalQueryDoAction(7, 1, 0);
			m_bAction = true; 
			CreateThread(NULL,10240,(LPTHREAD_START_ROUTINE)WaitEndOfAction,(void *)hThread, 0, &id);
  			hActionThread=hThread;
		} else {
			hThread = mpalQueryDoAction(5, 1, 0); 
			m_bAction = true;
			CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)WaitEndOfAction, (void *)hThread, 0, &id);
			hActionThread = hThread;
		}
	}
}


void RMTony::StopNoAction(void) {
	if (m_bAction)
		WaitForSingleObject(hActionThread, INFINITE);

	m_bActionPending = false;
	m_ActionItem = NULL;
	Stop();
}

void RMTony::Stop(void) {
	HANDLE hThread;

	if (m_ActionItem != NULL) {
		// Richiama l'MPAL per scegliere la direzione
		hThread = mpalQueryDoAction(21, m_ActionItem->MpalCode(), 0);

		if (hThread==INVALID_HANDLE_VALUE)
			RMCharacter::Stop();
		else {
			bNeedToStop = false;	// Se facciamo la OnWhichDirection, almeno dopo non dobbiamo fare la Stop()
			bMoving = false;
			WaitForSingleObject(hThread, INFINITE); // @@@ Mettere un assert dopo 10 secondi
		}
	} else {
		RMCharacter::Stop();
	}

	if (!m_bActionPending)
		return;

	m_bActionPending = false;

	ExecuteAction(m_Action, m_ActionItem->MpalCode(), m_ActionParm);
	
	m_ActionItem=NULL;
}


int RMTony::GetCurPattern(void) {
	int nPatt=RMCharacter::GetCurPattern();

	if (!m_bPastorella)
		return nPatt;
	
	switch (nPatt) {
	case PAT_PAST_STANDUP:
		return PAT_STANDUP;
	case PAT_PAST_STANDDOWN:
		return PAT_STANDDOWN;
	case PAT_PAST_STANDLEFT:
		return PAT_STANDLEFT;
	case PAT_PAST_STANDRIGHT:
		return PAT_STANDRIGHT;

	case PAT_PAST_WALKUP:
		return PAT_WALKUP;
	case PAT_PAST_WALKDOWN:
		return PAT_WALKDOWN;
	case PAT_PAST_WALKLEFT:
		return PAT_WALKLEFT;
	case PAT_PAST_WALKRIGHT:
		return PAT_WALKRIGHT;
	}

	return nPatt;
}

void RMTony::SetPattern(int nPatt, bool bPlayP0) {
	if (m_bPastorella) {
		switch (nPatt) {
		case PAT_STANDUP:
			nPatt=PAT_PAST_STANDUP;
			break;
		case PAT_STANDDOWN:
			nPatt=PAT_PAST_STANDDOWN;
			break;
		case PAT_STANDLEFT:
			nPatt=PAT_PAST_STANDLEFT;
			break;
		case PAT_STANDRIGHT:
			nPatt=PAT_PAST_STANDRIGHT;
			break;
		case PAT_WALKUP:
			nPatt=PAT_PAST_WALKUP;
			break;
		case PAT_WALKDOWN:
			nPatt=PAT_PAST_WALKDOWN;
			break;
		case PAT_WALKLEFT:
			nPatt=PAT_PAST_WALKLEFT;
			break;
		case PAT_WALKRIGHT:
			nPatt=PAT_PAST_WALKRIGHT;
			break;
		}
	}

	RMCharacter::SetPattern(nPatt, bPlayP0);
}


void RMTony::Take(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (GetCurPattern()) {
		case PAT_STANDDOWN:
			assert(0);	// Non esiste il prende mentre sei in StandDown
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_TAKEUP_UP1);
				break;
			case 1:
				SetPattern(PAT_TAKEUP_MID1);
				break;
			case 2:
				SetPattern(PAT_TAKEUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_TAKERIGHT_UP1);
				break;
			case 1:
				SetPattern(PAT_TAKERIGHT_MID1);
				break;
			case 2:
				SetPattern(PAT_TAKERIGHT_DOWN1);
				break;
			}
			break;


		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_TAKELEFT_UP1);
				break;
			case 1:
				SetPattern(PAT_TAKELEFT_MID1);
				break;
			case 2:
				SetPattern(PAT_TAKELEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		SetPattern(GetCurPattern()+1);
	} else if (nPart == 2) {
		switch (GetCurPattern()) {
		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
			SetPattern(PAT_STANDUP);
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
			SetPattern(PAT_STANDLEFT);
			break;

		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
			SetPattern(PAT_STANDRIGHT);
			break;
		}
	}
}


void RMTony::Put(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (GetCurPattern()) {
		case PAT_STANDDOWN:
			//assert(0);	// Non esiste il prende mentre sei in StandDown
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_PUTUP_UP1);
				break;
			case 1:
				SetPattern(PAT_PUTUP_MID1);
				break;
			case 2:
				SetPattern(PAT_PUTUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_PUTRIGHT_UP1);
				break;
			case 1:
				SetPattern(PAT_PUTRIGHT_MID1);
				break;
			case 2:
				SetPattern(PAT_PUTRIGHT_DOWN1);
				break;
			}
			break;


		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				SetPattern(PAT_PUTLEFT_UP1);
				break;
			case 1:
				SetPattern(PAT_PUTLEFT_MID1);
				break;
			case 2:
				SetPattern(PAT_PUTLEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		SetPattern(GetCurPattern()+1);
	} else if (nPart == 2) {
		switch (GetCurPattern()) {
		case PAT_PUTUP_UP2:
		case PAT_PUTUP_MID2:
		case PAT_PUTUP_DOWN2:
			SetPattern(PAT_STANDUP);
			break;

		case PAT_PUTLEFT_UP2:
		case PAT_PUTLEFT_MID2:
		case PAT_PUTLEFT_DOWN2:
			SetPattern(PAT_STANDLEFT);
			break;

		case PAT_PUTRIGHT_UP2:
		case PAT_PUTRIGHT_MID2:
		case PAT_PUTRIGHT_DOWN2:
			SetPattern(PAT_STANDRIGHT);
			break;
		}
	}
}


void RMTony::StartTalk(TALKTYPE nTalkType) {
	int headStartPat = 0, bodyStartPat = 0;
	int headLoopPat = 0, bodyLoopPat = 0;
	
	assert(!m_bIsTalking);

	m_bIsTalking = true;
	m_nPatB4Talking = GetCurPattern();
	m_nTalkType = nTalkType;

	// Setta la direzione di parlata SOLO se non siamo in una static animation (perch้ l'ha giเ fatto)
	if (!m_bIsStaticTalk) {
		switch (m_nPatB4Talking) {
		case PAT_STANDDOWN:
			m_TalkDirection=DOWN;
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
		case PAT_SIRIALZALEFT:
		case PAT_STANDLEFT:
			m_TalkDirection=LEFT;
			break;
		
		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
		case PAT_SIRIALZARIGHT:
		case PAT_STANDRIGHT:
			m_TalkDirection=RIGHT;
			break;

		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
		case PAT_STANDUP:
			m_TalkDirection=UP;
			break;
		}

		// Mette davanti il corpo di default
		m_bCorpoDavanti = true;
	}

	if (m_bPastorella) {
		// Da pastorella, c'่ un solo parlato
		MainFreeze();
		switch (m_TalkDirection) {
		case UP:
			SetPattern(PAT_PAST_TALKUP);
			break;

		case DOWN:
			SetPattern(PAT_PAST_TALKDOWN);
			break;
			
		case LEFT:
			SetPattern(PAT_PAST_TALKLEFT);
			break;

		case RIGHT:
			SetPattern(PAT_PAST_TALKRIGHT);
			break;
		}
		MainUnfreeze();
		return;
	}

	headStartPat = bodyStartPat = 0;
	bodyLoopPat = 0;

	switch (nTalkType) {
	case TALK_NORMAL: 
		m_bCorpoDavanti = false;
		headStartPat = 0;
		bodyStartPat = 0;

		switch (m_TalkDirection) {
		case DOWN:
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			m_nBodyOffset.Set(4, 53);
			break;

		case LEFT:
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			m_nBodyOffset.Set(6, 56);
			break;

		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			m_nBodyOffset.Set(6, 56);
			break;

		case UP:
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_STANDUP;
			m_nBodyOffset.Set(6, 53);
			break;
		}
		break;

	case TALK_FIANCHI:	
		m_bCorpoDavanti = false;
		switch (m_TalkDirection) {
		case UP:
			m_nBodyOffset.Set(2, 42);
			headStartPat = PAT_TESTA_UP;
			bodyStartPat = BPAT_FIANCHIUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_FIANCHIUP_LOOP;
			break;

		case DOWN:
			m_nBodyOffset.Set(2, 48);
			headStartPat = PAT_TESTA_DOWN;
			bodyStartPat = BPAT_FIANCHIDOWN_START;
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_FIANCHIDOWN_LOOP;
			break;

		case LEFT:
			m_nBodyOffset.Set(-3, 53);
			headStartPat = PAT_TESTA_LEFT;
			bodyStartPat = BPAT_FIANCHILEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_FIANCHILEFT_LOOP;
			break;

		case RIGHT:
			m_nBodyOffset.Set(2, 53);
			headStartPat = PAT_TESTA_RIGHT;
			bodyStartPat = BPAT_FIANCHIRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_FIANCHIRIGHT_LOOP;
			break;
		}
		break;


	case TALK_CANTA:
		m_nBodyOffset.Set(-10, 25);
		headStartPat = PAT_TESTA_LEFT;
		bodyStartPat = BPAT_CANTALEFT_START;
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_CANTALEFT_LOOP;
		break;

	case TALK_RIDE:
		m_bCorpoDavanti = false;
		switch (m_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_RIDELEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_RIDELEFT_LOOP;
			bodyLoopPat = BPAT_RIDELEFT;
			break;

		case RIGHT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_RIDERIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_RIDERIGHT_LOOP;
			bodyLoopPat = BPAT_RIDERIGHT;
			break;
		}
		break;

	case TALK_RIDE2:
		m_bCorpoDavanti = false;
		switch (m_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_RIDELEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_RIDELEFT_LOOP;
			break;

		case RIGHT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_RIDERIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_RIDERIGHT_LOOP;
			bodyLoopPat = BPAT_RIDERIGHT;
			break;
		}
		break;


	case TALK_SIINDICA:
		switch (m_TalkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			m_nBodyOffset.Set(-4, 40);
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_SIINDICALEFT;
			break;

		case RIGHT:
			m_nBodyOffset.Set(5, 40);
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_SIINDICARIGHT;
			break;					
		}
		break;

	case TALK_SPAVENTATO:
		switch (m_TalkDirection) {
		case UP:
			m_nBodyOffset.Set(-4, -11);
			headStartPat = PAT_TESTA_UP;
			bodyStartPat = BPAT_SPAVENTOUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_SPAVENTOUP_LOOP;
			break;

		case DOWN:
			m_nBodyOffset.Set(-5, 45);
			headStartPat = PAT_SPAVENTODOWN_START;
			bodyStartPat = BPAT_SPAVENTODOWN_START;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			bodyLoopPat = BPAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			m_nBodyOffset.Set(-4, 41);
			headStartPat = PAT_SPAVENTORIGHT_START;
			bodyStartPat = BPAT_SPAVENTORIGHT_START;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			bodyLoopPat = BPAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			m_nBodyOffset.Set(-10, 41);
			headStartPat = PAT_SPAVENTOLEFT_START;
			bodyStartPat = BPAT_SPAVENTOLEFT_START;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			bodyLoopPat = BPAT_SPAVENTOLEFT_LOOP;
			break;
		}
		break;

	case TALK_SPAVENTATO2:
		m_bCorpoDavanti = false;
		switch (m_TalkDirection) {
		case UP:
			bodyStartPat = BPAT_STANDUP;
			bodyLoopPat = BPAT_STANDUP;
			m_nBodyOffset.Set(6, 53);

			headStartPat = PAT_TESTA_UP;
			headLoopPat = PAT_TALK_UP;
			break;

		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			m_nBodyOffset.Set(4, 53);

			headStartPat = PAT_SPAVENTODOWN_START;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			m_nBodyOffset.Set(6, 56);

			headStartPat = PAT_SPAVENTORIGHT_START;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			m_nBodyOffset.Set(6, 56);

			headStartPat = PAT_SPAVENTOLEFT_START;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			break;
		}
		break;

	case TALK_CONBICCHIERE:
		m_nBodyOffset.Set(4, 53);
		headLoopPat = PAT_TALK_DOWN;
		bodyLoopPat = BPAT_BICCHIERE;
		break;
	case TALK_CONVERME:
		m_nBodyOffset.Set(9, 56);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_VERME;
		break;
	case TALK_CONMARTELLO:
		m_nBodyOffset.Set(6, 56);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MARTELLO;
		break;
	case TALK_CONCORDA:
		m_nBodyOffset.Set(-3, 38);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_CORDA;
		break;
	case TALK_CONSEGRETARIA:
		m_nBodyOffset.Set(-17, 12);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_CONSEGRETARIA;
		break;

	case TALK_CONCONIGLIO:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(-21, -5);
			bodyStartPat = BPAT_CONCONIGLIOLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONCONIGLIOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-4, -5);
			bodyStartPat = BPAT_CONCONIGLIORIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONCONIGLIORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONRICETTA:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(-61, -7);
			bodyStartPat = BPAT_CONRICETTALEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONRICETTALEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-5, -7);
			bodyStartPat = BPAT_CONRICETTARIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONRICETTARIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONCARTE:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(-34, -2);
			bodyStartPat = BPAT_CONCARTELEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONCARTELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-4, -2);
			bodyStartPat = BPAT_CONCARTERIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONCARTERIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZO:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(-35, 2);
			bodyStartPat = BPAT_CONPUPAZZOLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_CONPUPAZZOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-14, 2);
			bodyStartPat = BPAT_CONPUPAZZORIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_CONPUPAZZORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZOSTATIC:
	case TALK_CONRICETTASTATIC:
	case TALK_CONCONIGLIOSTATIC:
	case TALK_CONCARTESTATIC:
	case TALK_CONTACCUINOSTATIC:
	case TALK_CONMEGAFONOSTATIC:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALK_LEFT;
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			break;
		}
		break;

	// La barba ่ l'unico caso in cui la testa ่ animata a parte
	//  mentre il corpo ่ quello standard
	case TALK_CONBARBASTATIC:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALKBARBA_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			m_nBodyOffset.Set(6, 56);
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALKBARBA_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			m_nBodyOffset.Set(6, 56);
			break;
		}
		break;
	
	case TALK_SCHIFATO:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_SCHIFATOLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SCHIFATOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_SCHIFATORIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SCHIFATORIGHT_LOOP;
			break;
		}
		break;

	case TALK_NAAH:
		switch (m_TalkDirection) {
		case LEFT:
		case UP:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_NAAHLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_NAAHLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(6, 56);
			headStartPat = PAT_NAAHRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_NAAHRIGHT_LOOP;
			break;
		}
		break;

	case TALK_MACBETH1:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH1;
		break;
	case TALK_MACBETH2:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH2;
		break;
	case TALK_MACBETH3:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH3;
		break;
	case TALK_MACBETH4:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH4;
		break;
	case TALK_MACBETH5:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH5;
		break;
	case TALK_MACBETH6:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH6;
		break;
	case TALK_MACBETH7:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH7;
		break;
	case TALK_MACBETH8:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH8;
		break;
	case TALK_MACBETH9:
		m_nBodyOffset.Set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH9;
		break;

	case TALK_SPAVENTATOSTATIC:
		m_bCorpoDavanti = false;
		switch (m_TalkDirection) {
		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			m_nBodyOffset.Set(4, 53);

			headStartPat = PAT_SPAVENTODOWN_STAND;
			headLoopPat = PAT_SPAVENTODOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			m_nBodyOffset.Set(6, 56);

			headStartPat = PAT_SPAVENTORIGHT_STAND;
			headLoopPat = PAT_SPAVENTORIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			m_nBodyOffset.Set(6, 56);

			headStartPat = PAT_SPAVENTOLEFT_STAND;
			headLoopPat = PAT_SPAVENTOLEFT_LOOP;
			break;
		}
		break;
	}			

	// Esegue il set dei pattern vero e proprio
	if (headStartPat != 0 || bodyStartPat != 0) {
		MainFreeze();
		SetPattern(headStartPat);
		m_body.SetPattern(bodyStartPat);
		MainUnfreeze();
	
		if (bodyStartPat!=0)
			m_body.WaitForEndPattern();
		if (headStartPat!=0)
			WaitForEndPattern();
	}
	
	MainFreeze();
	SetPattern(headLoopPat);
	if (bodyLoopPat)
		m_body.SetPattern(bodyLoopPat);
	MainUnfreeze();
}


void RMTony::EndTalk(void) {
	int headStandPat = 0;
	int headEndPat = 0;
	int bodyEndPat = 0;
	int finalPat = 0;
	bool bStatic = false;

	bodyEndPat = 0;
	headEndPat = 0;
	
	switch (m_TalkDirection) {
	case UP:
		finalPat = PAT_STANDUP;
		headStandPat = PAT_TESTA_UP;
		break;

	case DOWN:
		finalPat = PAT_STANDDOWN;
		headStandPat = PAT_TESTA_DOWN;
		break;

	case LEFT:
		finalPat = PAT_STANDLEFT;
		headStandPat = PAT_TESTA_LEFT;
		break;

	case RIGHT:
		finalPat = PAT_STANDRIGHT;
		headStandPat = PAT_TESTA_RIGHT;
		break;
	}

	if (m_bPastorella) {
		MainFreeze();
		SetPattern(finalPat);
		MainUnfreeze();
  		m_bIsTalking = false; 
		return;
	}


	bStatic = false;
	switch (m_nTalkType) {
	case TALK_NORMAL:
		bodyEndPat = 0;
		break;

	case TALK_FIANCHI:
		switch (m_TalkDirection) {
		case UP:
			bodyEndPat = BPAT_FIANCHIUP_END;
			break;

		case DOWN:
			bodyEndPat = BPAT_FIANCHIDOWN_END;
			break;

		case LEFT:
			bodyEndPat = BPAT_FIANCHILEFT_END;
			break;

		case RIGHT:	
			bodyEndPat = BPAT_FIANCHIRIGHT_END;
			break;
		}
		break;

	case TALK_CANTA:
		bodyEndPat = BPAT_CANTALEFT_END;
		break;

	case TALK_RIDE:
	case TALK_RIDE2:
		if (m_TalkDirection==LEFT)
			headEndPat = PAT_RIDELEFT_END;
		else if (m_TalkDirection==RIGHT)
			headEndPat = PAT_RIDERIGHT_END;
			
		bodyEndPat = 0;
		break;

	case TALK_SCHIFATO:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			headEndPat = PAT_SCHIFATOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_SCHIFATORIGHT_END;
			break;
		}
			
		bodyEndPat = 0;
		break;

	case TALK_NAAH:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			headEndPat = PAT_NAAHLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_NAAHRIGHT_END;
			break;
		}
			
		bodyEndPat = 0;
		break;

	case TALK_SIINDICA:
		break;

	case TALK_SPAVENTATO:
		switch (m_TalkDirection) {	
		case UP:
			bodyEndPat = BPAT_SPAVENTOUP_END;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			bodyEndPat = BPAT_SPAVENTODOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			bodyEndPat = BPAT_SPAVENTORIGHT_END;
			break;

		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			bodyEndPat = BPAT_SPAVENTOLEFT_END;
			break;
		}
		break;

	case TALK_SPAVENTATO2:
		switch (m_TalkDirection) {	
		case UP:
			bodyEndPat = 0;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			bodyEndPat = 0;
			break;

		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			bodyEndPat = 0;
			break;
		}
		break;

	case TALK_CONCONIGLIO:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONCONIGLIOLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONCONIGLIORIGHT_END;
			break;
		}
		break;

	case TALK_CONRICETTA:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONRICETTALEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONRICETTARIGHT_END;
			break;
		}
		break;

	case TALK_CONCARTE:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONCARTELEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONCARTERIGHT_END;
			break;
		}
		break;

	case TALK_CONPUPAZZO:
		switch (m_TalkDirection) {	
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_CONPUPAZZOLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_CONPUPAZZORIGHT_END;
			break;
		}
		break;

	case TALK_CONVERME:
		finalPat = PAT_CONVERME;
		break;
	case TALK_CONCORDA:
		finalPat = PAT_CONCORDA;
		break;
	case TALK_CONSEGRETARIA:
		finalPat = PAT_CONSEGRETARIA;
		break;
	case TALK_CONMARTELLO:
		finalPat = PAT_CONMARTELLO;
		break;
	case TALK_CONBICCHIERE:
		finalPat = PAT_CONBICCHIERE;
		break;

	case TALK_MACBETH1:
	case TALK_MACBETH2:
	case TALK_MACBETH3:
	case TALK_MACBETH4:
	case TALK_MACBETH5:
	case TALK_MACBETH6:
	case TALK_MACBETH7:
	case TALK_MACBETH8:
		finalPat = 0;
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (m_TalkDirection) {	
		case DOWN:
			headStandPat = PAT_SPAVENTODOWN_STAND;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headStandPat = PAT_SPAVENTORIGHT_STAND;
			bodyEndPat = 0;
			break;

		case LEFT:
			headStandPat = PAT_SPAVENTOLEFT_STAND;
			bodyEndPat = 0;
			break;
		}
		break;
	}

	// Gestisce la fine di una animazione static lasciando tutto invariato
	if (m_bIsStaticTalk) {
		if (m_nTalkType == TALK_CONBARBASTATIC) {
			MainFreeze();
			SetPattern(0);
			if (m_TalkDirection == UP || m_TalkDirection == LEFT) {
				m_body.SetPattern(BPAT_CONBARBALEFT_STATIC);
				m_nBodyOffset.Set(-41, -14);
			} else if (m_TalkDirection == DOWN || m_TalkDirection == RIGHT) {
				m_body.SetPattern(BPAT_CONBARBARIGHT_STATIC);
				m_nBodyOffset.Set(-26, -14);
			}
			MainUnfreeze();
		} else {
			MainFreeze();
			SetPattern(headStandPat);
			MainUnfreeze();

			m_body.WaitForEndPattern();
		}

		m_bIsTalking = false;  
		return;
	}

	// Set dei pattern
	if (headEndPat != 0 && bodyEndPat != 0) {
		MainFreeze();
		SetPattern(headEndPat);
		MainUnfreeze();

		m_body.WaitForEndPattern();

		MainFreeze();
		m_body.SetPattern(bodyEndPat);
		MainUnfreeze();
		
		WaitForEndPattern();
		m_body.WaitForEndPattern();
	} else if (bodyEndPat != 0) {
		MainFreeze();
		SetPattern(headStandPat);
		MainUnfreeze();

		m_body.WaitForEndPattern();

		MainFreeze();
		m_body.SetPattern(bodyEndPat);
		MainUnfreeze();
		
		m_body.WaitForEndPattern();
	} else if (headEndPat != 0) {
		m_body.WaitForEndPattern();

		MainFreeze();
		SetPattern(headEndPat);
		MainUnfreeze();

		WaitForEndPattern();
	} else {
		m_body.WaitForEndPattern();
	}

	if (finalPat != 0) {
		MainFreeze();
		m_body.SetPattern(0);
		SetPattern(finalPat);
		MainUnfreeze();
	}

	m_bIsTalking = false;  
}

void RMTony::StartStatic(TALKTYPE nTalk) {
	int nPat = 0;
	int headPat = 0, headLoopPat = 0;
	int bodyStartPat = 0;
	int bodyLoopPat = 0;

	nPat = GetCurPattern();

	headLoopPat = -1;

	switch (nPat) {
	case PAT_STANDDOWN:
		m_TalkDirection=DOWN;
		headPat = PAT_TESTA_RIGHT;
		break;

	case PAT_TAKELEFT_UP2:
	case PAT_TAKELEFT_MID2:
	case PAT_TAKELEFT_DOWN2:
	case PAT_SIRIALZALEFT:
	case PAT_STANDLEFT:
		m_TalkDirection=LEFT;
		headPat = PAT_TESTA_LEFT;
		break;
		
	case PAT_TAKERIGHT_UP2:
	case PAT_TAKERIGHT_MID2:
	case PAT_TAKERIGHT_DOWN2:
	case PAT_SIRIALZARIGHT:
	case PAT_STANDRIGHT:
		m_TalkDirection=RIGHT;
		headPat = PAT_TESTA_RIGHT;
		break;

	case PAT_TAKEUP_UP2:
	case PAT_TAKEUP_MID2:
	case PAT_TAKEUP_DOWN2:
	case PAT_STANDUP:
		m_TalkDirection=UP;
		headPat = PAT_TESTA_LEFT;
		break;
	}

	m_bCorpoDavanti = true;

	switch (nTalk) {
	case TALK_CONCONIGLIOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-21, -5);
			bodyStartPat = BPAT_CONCONIGLIOLEFT_START;
			bodyLoopPat = BPAT_CONCONIGLIOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-4, -5);
			bodyStartPat = BPAT_CONCONIGLIORIGHT_START;
			bodyLoopPat = BPAT_CONCONIGLIORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONCARTESTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-34, -2);
			bodyStartPat = BPAT_CONCARTELEFT_START;
			bodyLoopPat = BPAT_CONCARTELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-4, -2);
			bodyStartPat = BPAT_CONCARTERIGHT_START;
			bodyLoopPat = BPAT_CONCARTERIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONRICETTASTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-61, -7);
			bodyStartPat = BPAT_CONRICETTALEFT_START;
			bodyLoopPat = BPAT_CONRICETTALEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-5, -7);
			bodyStartPat = BPAT_CONRICETTARIGHT_START;
			bodyLoopPat = BPAT_CONRICETTARIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONPUPAZZOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-35, 2);
			bodyStartPat = BPAT_CONPUPAZZOLEFT_START;
			bodyLoopPat = BPAT_CONPUPAZZOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-14, 2);
			bodyStartPat = BPAT_CONPUPAZZORIGHT_START;
			bodyLoopPat = BPAT_CONPUPAZZORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONTACCUINOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-16, -9);
			bodyStartPat = BPAT_CONTACCUINOLEFT_START;
			bodyLoopPat = BPAT_CONTACCUINOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-6, -9);
			bodyStartPat = BPAT_CONTACCUINORIGHT_START;
			bodyLoopPat = BPAT_CONTACCUINORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONMEGAFONOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-41, -8);
			bodyStartPat = BPAT_CONMEGAFONOLEFT_START;
			bodyLoopPat = BPAT_CONMEGAFONOLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-14, -8);
			bodyStartPat = BPAT_CONMEGAFONORIGHT_START;
			bodyLoopPat = BPAT_CONMEGAFONORIGHT_LOOP;
			break;
		}
		break;

	case TALK_CONBARBASTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			m_nBodyOffset.Set(-41, -14);
			bodyStartPat = BPAT_CONBARBALEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			headLoopPat = PAT_TALKBARBA_LEFT;
			headPat = 0;
			break;

		case DOWN:
		case RIGHT:
			m_nBodyOffset.Set(-26, -14);
			bodyStartPat = BPAT_CONBARBARIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_TALKBARBA_RIGHT;
			headPat = 0;
			break;
		}
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (m_TalkDirection) {
		case DOWN:
			headPat = PAT_SPAVENTODOWN_START;
			bodyLoopPat = BPAT_STANDDOWN;
			bodyStartPat = BPAT_STANDDOWN;
			headLoopPat = PAT_SPAVENTODOWN_STAND;
			m_nBodyOffset.Set(4, 53);
			break;

		case LEFT:
			headPat = PAT_SPAVENTOLEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SPAVENTOLEFT_STAND;
			m_nBodyOffset.Set(6, 56);
			break;

		case RIGHT:
			headPat = PAT_SPAVENTORIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SPAVENTORIGHT_STAND;
			m_nBodyOffset.Set(6, 56);
			break;
		}
	}

	// e vai con i pattern
	m_bIsStaticTalk = true;

	MainFreeze();
	SetPattern(headPat);
	m_body.SetPattern(bodyStartPat);
	MainUnfreeze();

	m_body.WaitForEndPattern();	
	WaitForEndPattern();

	MainFreeze();
	if (headLoopPat != -1)
		SetPattern(headLoopPat);
	m_body.SetPattern(bodyLoopPat);
	MainUnfreeze();
}


void RMTony::EndStatic(TALKTYPE nTalk) {
	int bodyEndPat = 0;
	int finalPat = 0;
	int headEndPat = 0;

	switch (m_TalkDirection) {
	case UP:
	case LEFT:
		finalPat = PAT_STANDLEFT;
		break;

	case RIGHT:
	case DOWN:
		finalPat = PAT_STANDRIGHT;
		break;
	}

	switch (nTalk) {
	case TALK_CONPUPAZZOSTATIC:
		switch (m_TalkDirection) {
		case UP: case LEFT:
			bodyEndPat = BPAT_CONPUPAZZOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONPUPAZZORIGHT_END;
			break;
		}
		break;

	case TALK_CONRICETTASTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONRICETTALEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONRICETTARIGHT_END;
			break;
		}
		break;

	case TALK_CONCONIGLIOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONCONIGLIOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONCONIGLIORIGHT_END;
			break;
		}
		break;
	
	case TALK_CONCARTESTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONCARTELEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONCARTERIGHT_END;
			break;
		}
		break;

	case TALK_CONTACCUINOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONTACCUINOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONTACCUINORIGHT_END;
			break;
		}
		break;

	case TALK_CONMEGAFONOSTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONMEGAFONOLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONMEGAFONORIGHT_END;
			break;
		}
		break;

	case TALK_CONBARBASTATIC:
		switch (m_TalkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_CONBARBALEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_CONBARBARIGHT_END;
			break;
		}
		break;

	case TALK_SPAVENTATOSTATIC:
		switch (m_TalkDirection) {
		case LEFT:
			headEndPat = PAT_SPAVENTOLEFT_END;
			break;

		case DOWN:
			headEndPat = PAT_SPAVENTODOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SPAVENTORIGHT_END;
			break;
		}
		break;
	}

	if (headEndPat != 0) {
		MainFreeze();
		SetPattern(headEndPat);
		MainUnfreeze();

		WaitForEndPattern();	
	} else {
		// Play please
		MainFreeze();
		m_body.SetPattern(bodyEndPat);
		MainUnfreeze();

		m_body.WaitForEndPattern();	
	}

	MainFreeze();
	SetPattern(finalPat);
	m_body.SetPattern(0);
	MainUnfreeze();
	
	m_bIsStaticTalk = false;
}

} // End of namespace Tony
