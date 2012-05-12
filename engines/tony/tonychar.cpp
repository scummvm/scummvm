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

RMTony::RMTony() {
	m_bShow = false;
	m_bShowOmbra = false;
	m_bCorpoDavanti = false;
	m_bActionPending = false;
	m_ActionItem = NULL;
	m_Action = 0;
	m_ActionParm = 0;
	m_bPastorella = false;
	m_bIsStaticTalk = false;
	m_bIsTalking = false;
	m_nPatB4Talking = 0;
	m_nTalkType = TALK_NORMAL;
	m_TalkDirection = UP;
	m_nTimeLastStep = 0;
	hActionThread = CORO_INVALID_PID_VALUE;
}

void RMTony::WaitEndOfAction(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	m_bAction = false;

	CORO_END_CODE;
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

	// Mostra Tony di default
	m_bShow=m_bShowOmbra = true;

	// Nessuna azione in attesa
	m_bActionPending = false;
	m_bAction = false;

	m_bPastorella = false;
	m_bIsTalking = false;
	m_bIsStaticTalk = false;

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

	m_nTimeLastStep = _vm->GetTime();
}


void RMTony::Close(void) {
	// Disalloca @@@ Manca la disallocazione di un item
	m_ombra.Destroy();
}

void RMTony::DoFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int curLoc) {
	CORO_BEGIN_CONTEXT;
		int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!m_nInList && m_bShow)
		bigBuf->AddPrim(new RMGfxPrimitive(this));
	
	SetSpeed(nCfgTonySpeed);	

	// Esegue il movimento normale del personaggio
	_ctx->time = _vm->GetTime();

	do {
		m_nTimeLastStep += (1000 / 40);
		CORO_INVOKE_2(RMCharacter::DoFrame, bigBuf, curLoc);

	} while (_ctx->time > m_nTimeLastStep + (1000 / 40));

	// Controlla se siamo alla fine del percorso
	if (EndOfPath() && m_bActionPending) {
		// Bisogna eseguire l'azione sulla quale abbiamo clickato
		m_bActionPending = false;	
	}

	if (m_bIsTalking || m_bIsStaticTalk)
		m_body.DoFrame(bigBuf, false);

	CORO_END_CODE;
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
	if (m_bShow && bDrawNow) {
		if (m_bCorpoDavanti) {
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

void RMTony::MoveAndDoAction(CORO_PARAM, RMPoint dst, RMItem *item, int nAction, int nActionParm) {
	CORO_BEGIN_CONTEXT;
		bool result;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

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

	CORO_INVOKE_2(RMCharacter::Move, dst, &_ctx->result);
	if (!_ctx->result) {
		m_bActionPending = false;
		m_ActionItem = NULL;
	}

	CORO_END_CODE;
}


void RMTony::ExecuteAction(int nAction, int nActionItem, int nParm) {
	// fixme: See if hThread can be converted to uint32
	uint32 pid;
	
	if (nAction == TA_COMBINE) {
		pid = mpalQueryDoAction(TA_COMBINE, nParm, nActionItem);
		
		// Se ่ fallito il combine, proviamo con il ReceiveCombine
		if (pid == CORO_INVALID_PID_VALUE) {
			pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, nParm); 
			
			// Se ่ fallito il receive, andiamo con quelli generici
			// @@@ CombineGive!
			if (pid == CORO_INVALID_PID_VALUE) {
				pid = mpalQueryDoAction(TA_COMBINE, nParm, 0);
				
				if (pid == CORO_INVALID_PID_VALUE){
					pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, 0);
				}
			}
		}
	} else {
		// Perform the action
		pid = mpalQueryDoAction(nAction, nActionItem, 0); 
	}
					
	if (pid != CORO_INVALID_PID_VALUE) {
		m_bAction = true;
		CoroScheduler.createProcess(WaitEndOfAction, &pid, sizeof(uint32));
		hActionThread = pid;
	} else if (nAction != TA_GOTO) {
		if (nAction == TA_TALK) {
			pid = mpalQueryDoAction(6, 1, 0); 
			m_bAction = true;
			CoroScheduler.createProcess(WaitEndOfAction, &pid, sizeof(uint32));
  			hActionThread = pid;
		} else if (nAction == TA_PALESATI) {
			pid = mpalQueryDoAction(7, 1, 0);
			m_bAction = true; 
			CoroScheduler.createProcess(WaitEndOfAction, &pid, sizeof(uint32));
  			hActionThread = pid;
		} else {
			pid = mpalQueryDoAction(5, 1, 0); 
			m_bAction = true;
			CoroScheduler.createProcess(WaitEndOfAction, &pid, sizeof(uint32));
			hActionThread = pid;
		}
	}
}


void RMTony::StopNoAction(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_bAction)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, hActionThread, CORO_INFINITE);

	m_bActionPending = false;
	m_ActionItem = NULL;
	CORO_INVOKE_0(Stop);

	CORO_END_CODE;
}

void RMTony::Stop(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
		uint32 pid;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (m_ActionItem != NULL) {
		// Richiama l'MPAL per scegliere la direzione
		_ctx->pid = mpalQueryDoAction(21, m_ActionItem->MpalCode(), 0);

		if (_ctx->pid == CORO_INVALID_PID_VALUE)
			CORO_INVOKE_0(RMCharacter::Stop);
		else {
			bNeedToStop = false;	// Se facciamo la OnWhichDirection, almeno dopo non dobbiamo fare la Stop()
			bMoving = false;
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->pid, CORO_INFINITE); // @@@ Mettere un assert dopo 10 secondi
		}
	} else {
		CORO_INVOKE_0(RMCharacter::Stop);
	}

	if (!m_bActionPending)
		return;

	m_bActionPending = false;

	ExecuteAction(m_Action, m_ActionItem->MpalCode(), m_ActionParm);
	
	m_ActionItem=NULL;

	CORO_END_CODE;
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


bool RMTony::StartTalkCalculate(TALKTYPE nTalkType, int &headStartPat, int &bodyStartPat,
								int &headLoopPat, int &bodyLoopPat) {
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
		return false;
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

		default:
			break;
		}
		break;
	}

	return true;
}

void RMTony::StartTalk(CORO_PARAM, TALKTYPE nTalkType) {
	CORO_BEGIN_CONTEXT;
		int headStartPat, bodyStartPat;
		int headLoopPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStartPat = _ctx->bodyStartPat = 0;
	_ctx->headLoopPat = _ctx->bodyLoopPat = 0;

	if (!StartTalkCalculate(nTalkType, _ctx->headStartPat, _ctx->bodyStartPat,
			_ctx->headLoopPat, _ctx->bodyLoopPat))
		return;

	// Esegue il set dei pattern vero e proprio
	if (_ctx->headStartPat != 0 || _ctx->bodyStartPat != 0) {
		MainFreeze();
		SetPattern(_ctx->headStartPat);
		m_body.SetPattern(_ctx->bodyStartPat);
		MainUnfreeze();
	
		if (_ctx->bodyStartPat != 0)
			CORO_INVOKE_0(m_body.WaitForEndPattern);
		if (_ctx->headStartPat != 0)
			CORO_INVOKE_0(WaitForEndPattern);
	}
	
	MainFreeze();
	SetPattern(_ctx->headLoopPat);
	if (_ctx->bodyLoopPat)
		m_body.SetPattern(_ctx->bodyLoopPat);
	MainUnfreeze();

	CORO_END_CODE;
}


bool RMTony::EndTalkCalculate(int &headStandPat, int &headEndPat, int &bodyEndPat, int &finalPat, bool &bStatic) {
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
		return false;
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


		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void RMTony::EndTalk(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
		int headStandPat, headEndPat;
		int bodyEndPat, finalPat;
		bool bStatic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStandPat = _ctx->headEndPat = 0;
	_ctx->bodyEndPat = _ctx->finalPat = 0;
	_ctx->bStatic = false;

	_ctx->bodyEndPat = 0;
	_ctx->headEndPat = 0;

	if (!EndTalkCalculate(_ctx->headStandPat, _ctx->headEndPat, _ctx->bodyEndPat, _ctx->finalPat, _ctx->bStatic))
		return;

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
			SetPattern(_ctx->headStandPat);
			MainUnfreeze();

			CORO_INVOKE_0(m_body.WaitForEndPattern);
		}

		m_bIsTalking = false;  
		return;
	}

	// Set dei pattern
	if (_ctx->headEndPat != 0 && _ctx->bodyEndPat != 0) {
		MainFreeze();
		SetPattern(_ctx->headEndPat);
		MainUnfreeze();

		CORO_INVOKE_0(m_body.WaitForEndPattern);

		MainFreeze();
		m_body.SetPattern(_ctx->bodyEndPat);
		MainUnfreeze();
		
		CORO_INVOKE_0(WaitForEndPattern);
		CORO_INVOKE_0(m_body.WaitForEndPattern);
	} else if (_ctx->bodyEndPat != 0) {
		MainFreeze();
		SetPattern(_ctx->headStandPat);
		MainUnfreeze();

		CORO_INVOKE_0(m_body.WaitForEndPattern);

		MainFreeze();
		m_body.SetPattern(_ctx->bodyEndPat);
		MainUnfreeze();
		
		CORO_INVOKE_0(m_body.WaitForEndPattern);
	} else if (_ctx->headEndPat != 0) {
		CORO_INVOKE_0(m_body.WaitForEndPattern);

		MainFreeze();
		SetPattern(_ctx->headEndPat);
		MainUnfreeze();

		CORO_INVOKE_0(WaitForEndPattern);
	} else {
		CORO_INVOKE_0(m_body.WaitForEndPattern);
	}

	if (_ctx->finalPat != 0) {
		MainFreeze();
		m_body.SetPattern(0);
		SetPattern(_ctx->finalPat);
		MainUnfreeze();
	}

	m_bIsTalking = false;  

	CORO_END_CODE;
}

void RMTony::StartStaticCalculate(TALKTYPE nTalk, int &headPat, int &headLoopPat,
						  int &bodyStartPat, int &bodyLoopPat) {
	int nPat = GetCurPattern();

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

		default:
			break;
		}

	default:
		break;
	}
}

void RMTony::StartStatic(CORO_PARAM, TALKTYPE nTalk) {
	CORO_BEGIN_CONTEXT;
		int headPat, headLoopPat;
		int bodyStartPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headPat = _ctx->headLoopPat = 0;
	_ctx->bodyStartPat = _ctx->bodyLoopPat = 0;

	// e vai con i pattern
	m_bIsStaticTalk = true;

	MainFreeze();
	SetPattern(_ctx->headPat);
	m_body.SetPattern(_ctx->bodyStartPat);
	MainUnfreeze();

	CORO_INVOKE_0(m_body.WaitForEndPattern);	
	CORO_INVOKE_0(WaitForEndPattern);

	MainFreeze();
	if (_ctx->headLoopPat != -1)
		SetPattern(_ctx->headLoopPat);
	m_body.SetPattern(_ctx->bodyLoopPat);
	MainUnfreeze();

	CORO_END_CODE;
}


void RMTony::EndStaticCalculate(TALKTYPE nTalk, int &bodyEndPat, int &finalPat, int &headEndPat) {
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

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void RMTony::EndStatic(CORO_PARAM, TALKTYPE nTalk) {
	CORO_BEGIN_CONTEXT;
		int bodyEndPat;
		int finalPat;
		int headEndPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bodyEndPat = 0;
	_ctx->finalPat = 0;
	_ctx->headEndPat = 0;

	EndStaticCalculate(nTalk, _ctx->bodyEndPat, _ctx->finalPat, _ctx->headEndPat);

	if (_ctx->headEndPat != 0) {
		MainFreeze();
		SetPattern(_ctx->headEndPat);
		MainUnfreeze();

		CORO_INVOKE_0(WaitForEndPattern);	
	} else {
		// Play please
		MainFreeze();
		m_body.SetPattern(_ctx->bodyEndPat);
		MainUnfreeze();

		CORO_INVOKE_0(m_body.WaitForEndPattern);	
	}

	MainFreeze();
	SetPattern(_ctx->finalPat);
	m_body.SetPattern(0);
	MainUnfreeze();
	
	m_bIsStaticTalk = false;

	CORO_END_CODE;
}

} // End of namespace Tony
