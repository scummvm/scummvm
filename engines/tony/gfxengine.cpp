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
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  GfxEngine.CPP........  *
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

#include "common/savefile.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/mpalutils.h"
#include "tony/custom.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

extern bool bIdleExited;
extern bool bPatIrqFreeze;
extern bool bSkipSfxNoLoop;


/****************************************************************************\
*       Metodi di RMGfxEngine
\****************************************************************************/

bool bIdleExited;

void ExitAllIdles(CORO_PARAM, int nCurLoc) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Chiude le idle
	bSkipSfxNoLoop = true;

	CORO_INVOKE_2(mpalEndIdlePoll, nCurLoc, NULL);

	bIdleExited = true;
	bSkipSfxNoLoop = false;

	CORO_END_CODE;
}

RMGfxEngine::RMGfxEngine() {
	// Crea il big buffer dove verranno disegnati i frame
	m_bigBuf.Create(RM_BBX, RM_BBY, 16);
	m_bigBuf.OffsetY(RM_SKIPY);

	csMainLoop = NULL;
}

RMGfxEngine::~RMGfxEngine() {
	// Chiude il buffer
	m_bigBuf.Destroy();
	g_system->deleteMutex(csMainLoop);
}

void RMGfxEngine::OpenOptionScreen(int type) {
	bool bRes = false;

	switch (type) {
		case 0:
			bRes = m_opt.Init(m_bigBuf);
			break;
		case 1:
			bRes = m_opt.InitLoadMenuOnly(m_bigBuf,true);
			break;
		case 2:
			bRes = m_opt.InitNoLoadSave(m_bigBuf);
			break;
		case 3:
			bRes = m_opt.InitLoadMenuOnly(m_bigBuf,false);
			break;
		case 4:
			bRes = m_opt.InitSaveMenuOnly(m_bigBuf,false);
			break;
	}

	if (bRes) {
		_vm->PauseSound(true);

		DisableInput();
		m_inv.EndCombine();
		m_curActionObj = 0;
		m_curAction = TA_GOTO;
		m_point.SetAction(m_curAction);
		m_point.SetSpecialPointer(RMPointer::PTR_NONE);
		m_point.SetCustomPointer(NULL);
		EnableMouse();
		_vm->GrabThumbnail();
		
		// Esce la IDLE onde evitare la morte prematura in caricamento
		m_bMustEnterMenu = true;							
		if (type == 1 || type == 2) {
			bIdleExited = true;
		} else {
			m_tony.StopNoAction();

			uint32 id;
			bIdleExited = false;
			CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)ExitAllIdles, (void *)m_nCurLoc, 0, &id);
		}
	}
}

void RMGfxEngine::DoFrame(bool bDrawLocation) {
	g_system->lockMutex(csMainLoop);
	
	// Poll dei dispositivi di input
	m_input.Poll();

	if (m_bMustEnterMenu && bIdleExited) {
		m_bOption = true;
		m_bMustEnterMenu = false;
		bIdleExited = false;
	}
	
  if (m_bOption) {
		m_opt.DoFrame(&m_input);
		m_bOption = !m_opt.IsClosing();
		if (!m_bOption) {
			DisableMouse();
			EnableInput();
			mpalStartIdlePoll(m_nCurLoc);
			_vm->PauseSound(false);
		}
	}

	if (bDrawLocation && m_bLocationLoaded) {
		// Locazione e oggetti
		m_loc.DoFrame(&m_bigBuf);

		// Controlla gli input del mouse
		if (m_bInput && !m_tony.InAction()) {
			// Se siamo sull'inventario, ่ lui che controlla tutti gli input
			if (m_inv.HaveFocus(m_input.MousePos()) && !m_inter.Active()) {
				// CLICK SINISTRO
				// **************
				if (m_input.MouseLeftClicked()/* && m_itemName.IsItemSelected()*/) {
					// Left click attiva il combine, se siamo su un oggetto
					if (m_inv.LeftClick(m_input.MousePos(),m_curActionObj)) {
						m_curAction = TA_COMBINE;
						m_point.SetAction(m_curAction);
					}
				}
				else			

				// CLICK DESTRO
				// ************
				if (m_input.MouseRightClicked()) {
					if(m_itemName.IsItemSelected()) {
						m_curActionObj=0;
						m_inv.RightClick(m_input.MousePos());
					} else
						m_inv.RightClick(m_input.MousePos());
				} else

				// RILASCIO DESTRO
				// ***************
				if (m_input.MouseRightReleased()) {
					if (m_inv.RightRelease(m_input.MousePos(), m_curAction)) {
						m_tony.MoveAndDoAction(m_itemName.GetHotspot(),m_itemName.GetSelectedItem(),m_curAction);

						m_curAction = TA_GOTO;
						m_point.SetAction(m_curAction);
					}
				}
			} else {
				// Menu Opzioni
				// ************
				if (m_bGUIOption) {
					if (!m_tony.InAction() && m_bInput) {
						if ((m_input.MouseLeftClicked() && m_input.MousePos().x < 3 && m_input.MousePos().y < 3)) {
							OpenOptionScreen(0);
							goto SKIPCLICKSINISTRO;
						} else if ((GetAsyncKeyState(Common::KEYCODE_ESCAPE)&0x8001) == 0x8001)
							OpenOptionScreen(0);
						else if (_vm->getIsDemo()) {
							if ((GetAsyncKeyState(Common::KEYCODE_F3) & 0x8001) == 0x8001)
								OpenOptionScreen(3);
							else if ((GetAsyncKeyState(Common::KEYCODE_F2) & 0x8001) == 0x8001)
								OpenOptionScreen(4);
						}
					}
				}

				// CLICK SINISTRO
				// **************
				if (m_input.MouseLeftClicked() && !m_inter.Active()) {
					// Se clicko dentro un oggetto, esegui l'azione
					//if (m_itemName.IsItemSelected())
					{
						if (m_curAction != TA_COMBINE)
							m_tony.MoveAndDoAction(m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), m_point.CurAction());
						else if (m_itemName.GetSelectedItem() != NULL)
							m_tony.MoveAndDoAction(m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), TA_COMBINE, m_curActionObj);
					}
					
					if (m_curAction == TA_COMBINE) {
						m_inv.EndCombine();
						m_point.SetSpecialPointer(RMPointer::PTR_NONE);
					}

					m_curAction = TA_GOTO;
					m_point.SetAction(m_curAction);
				}

SKIPCLICKSINISTRO:
				// CLICK DESTRO
				// ************
				if (m_curAction == TA_COMBINE) {
					// Durante il combine, lo annulla.
					if (m_input.MouseRightClicked()) {
						m_inv.EndCombine();
						m_curActionObj = 0;
						m_curAction = TA_GOTO;
						m_point.SetAction(m_curAction);
						m_point.SetSpecialPointer(RMPointer::PTR_NONE);
					}
				} else if (m_input.MouseRightClicked() && m_itemName.IsItemSelected() && m_point.GetSpecialPointer() == RMPointer::PTR_NONE) {
					if (m_bGUIInterface) {
						// Prima di aprire l'interfaccia, rimette GOTO
						m_curAction = TA_GOTO;
						m_curActionObj = 0;
						m_point.SetAction(m_curAction);
						m_inter.Clicked(m_input.MousePos());
					}
				}
				

				// RILASCIO DESTRO
				// ***************
				if (m_input.MouseRightReleased()) {
					if (m_bGUIInterface) {
						if (m_inter.Released(m_input.MousePos(),m_curAction)) {
							m_point.SetAction(m_curAction);
							m_tony.MoveAndDoAction(m_itemName.GetHotspot(),m_itemName.GetSelectedItem(), m_curAction);

							m_curAction = TA_GOTO;
							m_point.SetAction(m_curAction);
						}
					}
				}
			}

			// Aggiorna il nome sotto il puntatore del mouse
			m_itemName.SetMouseCoord(m_input.MousePos());
			if (!m_inter.Active() && !m_inv.MiniActive())
				m_itemName.DoFrame(m_bigBuf,m_loc,m_point,m_inv);	
		}

		// Inventario & interfaccia
		m_inter.DoFrame(m_bigBuf, m_input.MousePos());
		m_inv.DoFrame(m_bigBuf, m_point, m_input.MousePos(), (!m_tony.InAction() && !m_inter.Active() && m_bGUIInventory));
	}

	// Anima Tony
	m_tony.DoFrame(&m_bigBuf, m_nCurLoc);
	
	// Aggiorna lo scrolling per tenere Tony dentro lo schermo
	if (m_tony.MustUpdateScrolling() && m_bLocationLoaded) {
		RMPoint showThis = m_tony.Position();
		showThis.y -= 60;
		m_loc.UpdateScrolling(showThis);
	}

	if (m_bLocationLoaded)
		m_tony.SetScrollPosition(m_loc.ScrollPosition());

	if ((!m_tony.InAction() && m_bInput) || m_bAlwaysDrawMouse) {
		m_point.SetCoord(m_input.MousePos());
		m_point.DoFrame(&m_bigBuf);
	}

	// **********************
	// Disegna la lista di OT
	// **********************
	m_bigBuf.DrawOT();

#define FSTEP (480/32)

	// Wipe
	if (m_bWiping) {
		switch (m_nWipeType) {
			case 1:
				if (!(m_rcWipeEllipse.bottom - m_rcWipeEllipse.top >= FSTEP * 2)) {
					SetEvent(m_hWipeEvent);
					m_nWipeType = 3;
					break;
				}
				
				m_rcWipeEllipse.top += FSTEP;
				m_rcWipeEllipse.left += FSTEP;
				m_rcWipeEllipse.right -= FSTEP;
				m_rcWipeEllipse.bottom -= FSTEP;
				break;

			case 2:
				if (!(m_rcWipeEllipse.bottom - m_rcWipeEllipse.top < 480 - FSTEP)) {
					SetEvent(m_hWipeEvent);
					m_nWipeType = 3;
					break;
				}

				m_rcWipeEllipse.top -= FSTEP;
				m_rcWipeEllipse.left -= FSTEP;
				m_rcWipeEllipse.right += FSTEP;
				m_rcWipeEllipse.bottom += FSTEP;
				break;
		}
	}

	g_system->unlockMutex(csMainLoop);
}


void RMGfxEngine::InitCustomDll(void) {
	SetupGlobalVars(&m_tony, &m_point, &_vm->_theBoxes, &m_loc, &m_inv, &m_input);
}

void RMGfxEngine::ItemIrq(uint32 dwItem, int nPattern, int nStatus) {
	static RMGfxEngine *This = NULL;
	RMItem *item;

	// Inizializzazione!
	if ((int)dwItem == -1) {
		This = (RMGfxEngine*)nPattern;
		return;
	}

	if (This->m_bLocationLoaded) {
		item=This->m_loc.GetItemFromCode(dwItem);
		if (item != NULL) {
			if (nPattern!=-1) {
				if (bPatIrqFreeze)
					MainFreeze();
				item->SetPattern(nPattern,true);
				if (bPatIrqFreeze)
					MainUnfreeze();
			}
			if (nStatus!=-1)
				item->SetStatus(nStatus);
		}
	}
}

/*
	// WINBUG: This is a special case for the file open/save dialog,
	//  which sometimes pumps while it is coming up but before it has
	//  disabled the main window.
	HWND hWndFocus = ::GetFocus();
	bool bEnableParent = false;
	m_ofn.hwndOwner = PreModal();
	AfxUnhookWindowCreate();
	if (m_ofn.hwndOwner != NULL && ::IsWindowEnabled(m_ofn.hwndOwner)) {
		bEnableParent = true;
		::EnableWindow(m_ofn.hwndOwner, false);
	}

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	ASSERT(pThreadState->m_pAlternateWndInit == NULL);

	if (m_ofn.Flags & OFN_EXPLORER)
		pThreadState->m_pAlternateWndInit = this;
	else
		AfxHookWindowCreate(this);

	int nResult;
	if (m_bOpenFileDialog)
		nResult = ::GetOpenFileName(&m_ofn);
	else
		nResult = ::GetSaveFileName(&m_ofn);

	if (nResult)
		ASSERT(pThreadState->m_pAlternateWndInit == NULL);
	pThreadState->m_pAlternateWndInit = NULL;

	// WINBUG: Second part of special case for file open/save dialog.
	if (bEnableParent)
		::EnableWindow(m_ofn.hwndOwner, true);
	if (::IsWindow(hWndFocus))
		::SetFocus(hWndFocus);
*/


void RMGfxEngine::SelectLocation(RMPoint ptTonyStart, RMPoint start) {
#if 0
	OPENFILENAME ofn;
	char lpszFileName[512];

	// @@@ Con TonyStart=-1,-1 allora usa la posizione scritta nella locazione

  // Sceglie la locazione
	ZeroMemory(lpszFileName,512);
	ZeroMemory(&ofn,sizeof(ofn));
  ofn.lStructSize=sizeof(ofn);
  ofn.hwndOwner=NULL;
  ofn.lpstrFilter="Locazione (*.LOC)\0*.LOC\0Locazione ottimizzata (*.LOX)\0*.LOX\0Tutti i files (*.*)\0*.*\0";
  ofn.lpstrCustomFilter=NULL;
  ofn.nFilterIndex=1;
  ofn.lpstrFile=lpszFileName;
  ofn.nMaxFile=512;
  ofn.lpstrInitialDir=NULL;
  ofn.lpstrTitle="Load Location";
  ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;

  if (!GetOpenFileName(&ofn))
		ASSERT(0);

	// Carica la locazione
	m_loc.Load(lpszFileName);
	m_bLocationLoaded = true;
	m_nCurLoc=m_loc.TEMPGetNumLoc();
	
	if (ptTonyStart.x==-1 && ptTonyStart.y==-1)
		InitForNewLocation(m_loc.TEMPGetNumLoc(),m_loc.TEMPGetTonyStart(),RMPoint(-1,-1));
	else
		InitForNewLocation(m_loc.TEMPGetNumLoc(),ptTonyStart,start);
#endif
}

void RMGfxEngine::InitForNewLocation(int nLoc, RMPoint ptTonyStart, RMPoint start) {
	if (start.x == -1 || start.y == -1) {
		start.x = ptTonyStart.x-RM_SX / 2;
		start.y = ptTonyStart.y-RM_SY / 2;
	}

	m_loc.SetScrollPosition(start);
	
	if (ptTonyStart.x==0 && ptTonyStart.y == 0) {
	} else {
		m_tony.SetPosition(ptTonyStart,nLoc);
		m_tony.SetScrollPosition(start);
	}

	m_curAction = TA_GOTO;
	m_point.SetCustomPointer(NULL);
	m_point.SetSpecialPointer(RMPointer::PTR_NONE);
	m_point.SetAction(m_curAction);
	m_inter.Reset();
	m_inv.Reset();

	mpalStartIdlePoll(m_nCurLoc);
}

HANDLE RMGfxEngine::LoadLocation(int nLoc, RMPoint ptTonyStart, RMPoint start) {
	bool bLoaded;
	int i;

	m_nCurLoc=nLoc;

	bLoaded = false;
	for (i=0; i < 5; i++) {
		// Retry sul loading della locazione
		RMRes res(m_nCurLoc);
		if (!res.IsValid())
			continue;
#if 0
		// codice per dumpare una locazione in caso serva una modifica
		if (nLoc == 106) {
			FILE *f = fopen("loc106.lox", "wb");
			fwrite(res.DataPointer(), res.Size(), 1, f);
			fclose(f);
		}
#endif
		m_loc.Load(res);
		InitForNewLocation(nLoc,ptTonyStart,start);
		bLoaded = true;
		break;
	}
	
	if (!bLoaded)
		SelectLocation(ptTonyStart,start);

	if (m_bOption)
		m_opt.ReInit(m_bigBuf);

	m_bLocationLoaded = true;

	// On Enter per la locazion
	return INVALID_HANDLE_VALUE; //mpalQueryDoAction(0,m_nCurLoc,0);
}

void RMGfxEngine::UnloadLocation(CORO_PARAM, bool bDoOnExit, HANDLE *result) {
	CORO_BEGIN_CONTEXT;
		HANDLE h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	
	// Scarica tutta la memoria della locazione
	CORO_INVOKE_2(mpalEndIdlePoll, m_nCurLoc, NULL);

	// On Exit?
	if (bDoOnExit) {
		_ctx->h = mpalQueryDoAction(1, m_nCurLoc, 0);
		if (_ctx->h != INVALID_HANDLE_VALUE)
			WaitForSingleObject(_ctx->h, INFINITE);
	}

	MainFreeze();

	m_bLocationLoaded = false;
	
	m_bigBuf.ClearOT();
	m_loc.Unload();

	if (result != NULL)
		*result = INVALID_HANDLE_VALUE;

	CORO_END_CODE;
}

void RMGfxEngine::Init(/*HINSTANCE hInst*/) {
/*
	//RECUPERARE UNA LOCAZIONE:

	RMRes res(5);
	ASSERT(res.IsValid());
	FILE *f;
	f=fopen("c:\\code\\rm\\new\\pippo.loc","wb");
	fwrite(res,1,5356900,f);
	fclose(f);
*/

	// Schermata di loading
	RMResRaw *raw; 
	RMGfxSourceBuffer16 *load = NULL;
	INIT_GFX16_FROMRAW(20038, load);
	m_bigBuf.AddPrim(new RMGfxPrimitive(load));
	m_bigBuf.DrawOT();
	m_bigBuf.ClearOT();
	delete load;
	_vm->_window.GetNewFrame(*this, NULL);



	bPatIrqFreeze = true;

	// GUI attivabile
	m_bGUIOption = true;
	m_bGUIInterface = true;
	m_bGUIInventory = true;

	bSkipSfxNoLoop = false;
	m_bMustEnterMenu = false;
	bIdleExited = false;
	m_bOption = false;
	m_bWiping = false;
	m_hWipeEvent = CreateEvent(NULL, false, false, NULL);

	// Crea l'evento di freeze
	csMainLoop = g_system->createMutex();

	// Inizializza la funzione di IRQ di Item per l'MPAL
	ItemIrq((uint32)-1, (uint32)this, 0);
	mpalInstallItemIrq(ItemIrq);
	
	// Inizializza DirectInput	
	m_input.Init(/*hInst*/);

	// Inizializza il puntatore del mouse
	m_point.Init();

	// Inizializza Tony
	m_tony.Init();
	m_tony.LinkToBoxes(&_vm->_theBoxes);

	// Inizializza l'inventario e l'interfaccia
	m_inv.Init();
	m_inter.Init();

	// Carica la locazione e setta le prioritเ		@@@@@
	m_bLocationLoaded = false;
/*
	m_nCurLoc=1;
	RMRes res(m_nCurLoc);
	m_loc.Load(res);
	m_loc.SetPriority(1);
	m_tony.SetPosition(RMPoint(201,316),1);
	//m_tony.SetPosition(RMPoint(522,305),2);
	//m_tony.SetPosition(RMPoint(158,398),4);
	m_tony.SetPattern(m_tony.PAT_STANDDOWN);
	m_curAction=TA_GOTO;
*/ 
	EnableInput();

	// Inizio del gioco
	//m_tony.ExecuteAction(4,1,0);    //PREGAME
	
	m_tony.ExecuteAction(20,1,0);

//	theLog << "Seleziona la locazione\n";
	//LoadLocation(1,RMPoint(201,316),RMPoint(-1,-1));
	//SelectLocation();
	//LoadLocation(5,RMPoint(685,338),RMPoint(-1,-1));
	//LoadLocation(7,RMPoint(153,424),RMPoint(-1,-1));
	//LoadLocation(70,RMPoint(10,10),RMPoint(-1,-1));
	//LoadLocation(20,RMPoint(112,348),RMPoint(-1,-1));
	//LoadLocation(26,RMPoint(95,456),RMPoint(-1,-1));
	//LoadLocation(12,RMPoint(221,415),RMPoint(-1,-1));
	//LoadLocation(25,RMPoint(221,415),RMPoint(-1,-1));
	//LoadLocation(16,RMPoint(111,438),RMPoint(-1,-1));
	//LoadLocation(60,RMPoint(18,302),RMPoint(-1,-1));
	
	// CASTELLO
	
	//LoadLocation(40,RMPoint(233,441),RMPoint(-1,-1));
}

void RMGfxEngine::Close(void) {
	m_bigBuf.ClearOT();

	m_inter.Close();
	m_inv.Close();
	m_tony.Close();
	m_point.Close();
	m_input.Close();
}

void RMGfxEngine::SwitchFullscreen(bool bFull) {
}

void RMGfxEngine::GDIControl(bool bCon) {
}

void RMGfxEngine::EnableInput(void) {
	m_bInput = true;
}

void RMGfxEngine::DisableInput(void) {
	m_bInput = false;
	m_inter.Reset();
}

void RMGfxEngine::EnableMouse(void) {
	m_bAlwaysDrawMouse = true;
}

void RMGfxEngine::DisableMouse(void) {
	m_bAlwaysDrawMouse = false;
}

void RMGfxEngine::Freeze(void) {
	g_system->lockMutex(csMainLoop);
}

void RMGfxEngine::Unfreeze(void) {
	g_system->unlockMutex(csMainLoop);
}

void CharsSaveAll(Common::OutSaveFile *f);
void CharsLoadAll(Common::InSaveFile *f);
void MCharResetCodes(void);
void SaveChangedHotspot(Common::OutSaveFile *f);
void LoadChangedHotspot(Common::InSaveFile *f);
void ReapplyChangedHotspot(void);

void RestoreMusic(void);
void SaveMusic(Common::OutSaveFile *f);
void LoadMusic(Common::InSaveFile *f);

unsigned char wrkmem[LZO1X_999_MEM_COMPRESS];

void RMGfxEngine::SaveState(const char *fn, byte *curThumb, const char *name, bool bFastCompress) {
	Common::OutSaveFile *f;
	byte *state, *statecmp;
	byte *thumbcmp; 
	uint thumbsizecmp, thumbsize;
	uint size, sizecmp;
	int i;
	char buf[4];
	RMPoint tp = m_tony.Position();

	// Salvataggio: variabili mpal + locazione corrente + posizione di tony + inventario

	// Per ora salviamo solo lo stato MPAL
	size=mpalGetSaveStateSize();
	state = new byte[size];
	statecmp = new byte[size*2];
	mpalSaveState(state);

	thumbcmp = new byte[160 * 120 * 4];
	thumbsize = 160 * 120 * 2;
	
	if (bFastCompress) {
		lzo1x_1_compress(state,size,statecmp,&sizecmp,wrkmem);
		lzo1x_1_compress(curThumb,thumbsize,thumbcmp,&thumbsizecmp,wrkmem);
	} else {
		lzo1x_999_compress(state,size,statecmp,&sizecmp,wrkmem);
		lzo1x_999_compress(curThumb,thumbsize,thumbcmp,&thumbsizecmp,wrkmem);
	}

	buf[0] = 'R';
	buf[1] = 'M';
	buf[2] = 'S';
	buf[3] = 0x7;

	f = g_system->getSavefileManager()->openForSaving(fn);
	if (f==NULL) return;
	f->write(buf, 4);
	f->writeUint32LE(thumbsizecmp);
	f->write(thumbcmp, thumbsizecmp);

	// Livello di difficoltเ
	i = mpalQueryGlobalVar("VERSIONEFACILE");
	f->writeByte(i);

	i=strlen(name);
	f->writeByte(i);
	f->write(name, i);
	f->writeUint32LE(m_nCurLoc);
	f->writeUint32LE(tp.x);
	f->writeUint32LE(tp.y);
	f->writeUint32LE(size);
	f->writeUint32LE(sizecmp);
	f->write(statecmp, sizecmp);
	delete[] state;
	delete[] statecmp;
	delete[] thumbcmp;

	// inventario
	size = m_inv.GetSaveStateSize();
	state = new byte[size];
	m_inv.SaveState(state);
	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// boxes
	size = _vm->_theBoxes.GetSaveStateSize();
	state = new byte[size];
	_vm->_theBoxes.SaveState(state);
	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// New Ver5
	bool bStat;
	
	// Salva lo stato della pastorella e del palesati
	bStat = m_tony.GetPastorella();
	f->writeByte(bStat);
	bStat = m_inter.GetPalesati();
	f->writeByte(bStat);
	
	// Salva gli mchar
	CharsSaveAll(f);

	// Salva le opzioni
	f->writeByte(bCfgInvLocked);
	f->writeByte(bCfgInvNoScroll);
	f->writeByte(bCfgTimerizedText);
	f->writeByte(bCfgInvUp);
	f->writeByte(bCfgAnni30);
	f->writeByte(bCfgAntiAlias);
	f->writeByte(bCfgSottotitoli);
	f->writeByte(bCfgTransparence);
	f->writeByte(bCfgInterTips);
	f->writeByte(bCfgDubbing);
	f->writeByte(bCfgMusic);
	f->writeByte(bCfgSFX);
	f->writeByte(nCfgTonySpeed);
	f->writeByte(nCfgTextSpeed);
	f->writeByte(nCfgDubbingVolume);
	f->writeByte(nCfgMusicVolume);
	f->writeByte(nCfgSFXVolume);

	// Salva gli hotspot
	SaveChangedHotspot(f);

	// Salva la musica
	SaveMusic(f);

	f->finalize();
	delete f;
}

void RMGfxEngine::LoadState(const char *fn) {
	// PROBLEMA: Bisognerebbe caricare la locazione in un thread a parte per fare la OnEnter ...
	Common::InSaveFile *f;
	byte *state, *statecmp;
	uint size, sizecmp;
	char buf[4];
	RMPoint tp;
	int loc;
	int ver;
	int i;

	f = g_system->getSavefileManager()->openForLoading(fn);
	if (f == NULL) return;
	f->read(buf, 4);
	if (buf[0] != 'R' || buf[1] != 'M' || buf[2] != 'S') {
		delete f;
		return;
	}
	
	ver = buf[3];
	
	if (ver != 0x1 && ver != 0x2 && ver != 0x3 && ver != 0x4 && ver != 0x5 && ver != 0x6 && ver != 0x7) {
		delete f;
		return;
	}
	
	if (ver >= 0x3) {
		// C'่ il thumbnail. Se ver >= 5, ่ compresso
		if (ver >= 0x5) {
			i = 0;
			i = f->readUint32LE();
			f->seek(i);
		} else
			f->seek(160 * 120 * 2, SEEK_CUR);
	}

	if (ver >= 0x5) {
		// Skip del livello di difficoltเ
		f->seek(1, SEEK_CUR);
	}

	if (ver >= 0x4) {	// Skippa il nome, che non serve a nessuno
		i = f->readByte();
		f->seek(i, SEEK_CUR);
	}

	loc = f->readUint32LE();
	loc = f->readUint32LE();
	tp.x = f->readUint32LE();
	tp.y = f->readUint32LE();
	size = f->readUint32LE();

	if (ver >= 0x5) {
		// Stato MPAL compresso!
		sizecmp = f->readUint32LE();
		state = new byte[size];
		statecmp = new byte[sizecmp];
		f->read(statecmp, sizecmp);
		lzo1x_decompress(statecmp,sizecmp,state,&size);
		delete[] statecmp;
	} else {
		state = new byte[size];
		f->read(state, size);
	}

	mpalLoadState(state);
	delete[] state;


	// inventario
	size = f->readUint32LE();
	state = new byte[size];
	f->read(state, size);
	m_inv.LoadState(state);
	delete[] state;

	if (ver >= 0x2) {	  // Versione 2: box please
		size = f->readUint32LE();
		state = new byte[size];
		f->read(state, size);
		_vm->_theBoxes.LoadState(state);
		delete[] state;
	}

	if (ver >= 5) {
	  // Versione 5: 
		bool bStat = false;
		
		bStat = f->readByte();
		m_tony.SetPastorella(bStat);		
		bStat = f->readByte();
		m_inter.SetPalesati(bStat);		

		CharsLoadAll(f);
	}

	if (ver >= 6) {
		// Carica le opzioni
		bCfgInvLocked = f->readByte();
		bCfgInvNoScroll = f->readByte();
		bCfgTimerizedText = f->readByte();
		bCfgInvUp = f->readByte();
		bCfgAnni30 = f->readByte();
		bCfgAntiAlias = f->readByte();
		bCfgSottotitoli = f->readByte();
		bCfgTransparence = f->readByte();
		bCfgInterTips = f->readByte();
		bCfgDubbing = f->readByte();
		bCfgMusic = f->readByte();
		bCfgSFX = f->readByte();
		nCfgTonySpeed = f->readByte();
		nCfgTextSpeed = f->readByte();
		nCfgDubbingVolume = f->readByte();
		nCfgMusicVolume = f->readByte();
		nCfgSFXVolume = f->readByte();

		// Carica gli hotspot
		LoadChangedHotspot(f);
	}

	if (ver >= 7) {
		LoadMusic(f);
	}

	delete f;

	UnloadLocation(nullContext, false, NULL);
	LoadLocation(loc,tp,RMPoint(-1, -1));
	m_tony.SetPattern(RMTony::PAT_STANDRIGHT);
	MainUnfreeze();
	
	// Le versioni vecchie necessitano di On enter
	if (ver < 5)
		mpalQueryDoActionU32(0, loc, 0);
	else {
		// In quelle nuove, ci basta resettare gli mcode
		MCharResetCodes();
	}

	if (ver >= 6)
		ReapplyChangedHotspot();

	RestoreMusic();

	m_bGUIInterface = true;
	m_bGUIInventory = true;
	m_bGUIOption = true;
}

void RMGfxEngine::PauseSound(bool bPause) {
	if (m_bLocationLoaded)
		m_loc.PauseSound(bPause);
}

void RMGfxEngine::InitWipe(int type) {
	m_bWiping = true;
	m_nWipeType=type;
	m_nWipeStep=0;

	if (m_nWipeType == 1)
		m_rcWipeEllipse = Common::Rect(80, 0, 640 - 80, 480);
	else if (m_nWipeType == 2)
		m_rcWipeEllipse = Common::Rect(320 - FSTEP, 240 - FSTEP, 320 + FSTEP, 240 + FSTEP);
}

void RMGfxEngine::CloseWipe(void) {
	m_bWiping = false;
}

void RMGfxEngine::WaitWipeEnd(void) {
	WaitForSingleObject(m_hWipeEvent,INFINITE);
}

} // End of namespace Tony
