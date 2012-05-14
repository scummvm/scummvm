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

#include "common/savefile.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/mpalutils.h"
#include "tony/custom.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {


/****************************************************************************\
*       Metodi di RMGfxEngine
\****************************************************************************/

void ExitAllIdles(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	int nCurLoc = *(const int *)param;

	CORO_BEGIN_CODE(_ctx);

	// Chiude le idle
	GLOBALS.bSkipSfxNoLoop = true;

	CORO_INVOKE_2(mpalEndIdlePoll, nCurLoc, NULL);

	GLOBALS.bIdleExited = true;
	GLOBALS.bSkipSfxNoLoop = false;

	CORO_END_CODE;
}

RMGfxEngine::RMGfxEngine() {
	// Crea il big buffer dove verranno disegnati i frame
	m_bigBuf.Create(RM_BBX, RM_BBY, 16);
	m_bigBuf.OffsetY(RM_SKIPY);

	csMainLoop = NULL;
	m_nCurLoc = 0;
	m_curAction = TA_GOTO;
	m_curActionObj = 0;
	m_nWipeType	= 0;
	m_hWipeEvent = 0;
	m_nWipeStep	= 0;
	m_bMustEnterMenu = false;
	m_bWiping = false;
	m_bGUIOption = false;
	m_bGUIInterface = false;
	m_bGUIInventory = false;
	m_bAlwaysDrawMouse = false;
	m_bOption = false;
	m_bLocationLoaded = false;
	m_bInput = false;
}

RMGfxEngine::~RMGfxEngine() {
	// Chiude il buffer
	m_bigBuf.Destroy();
	g_system->deleteMutex(csMainLoop);
}

void RMGfxEngine::OpenOptionScreen(CORO_PARAM, int type) {
	CORO_BEGIN_CONTEXT;
		bool bRes;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bRes = false;

	if (type == 0)
		CORO_INVOKE_2(m_opt.Init, m_bigBuf, _ctx->bRes);
	else if (type == 1)
		CORO_INVOKE_3(m_opt.InitLoadMenuOnly, m_bigBuf, true, _ctx->bRes);
	else if (type == 2)
		CORO_INVOKE_2(m_opt.InitNoLoadSave, m_bigBuf, _ctx->bRes);
	else if (type == 3)
		CORO_INVOKE_3(m_opt.InitLoadMenuOnly, m_bigBuf, false, _ctx->bRes);
	else if (type == 4)
		CORO_INVOKE_3(m_opt.InitSaveMenuOnly, m_bigBuf, false, _ctx->bRes);

	if (_ctx->bRes) {
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
			GLOBALS.bIdleExited = true;
		} else {
			CORO_INVOKE_0(m_tony.StopNoAction);

			GLOBALS.bIdleExited = false;

			CoroScheduler.createProcess(ExitAllIdles, &m_nCurLoc, sizeof(int));
		}
	}

	CORO_END_CODE;
}

void RMGfxEngine::DoFrame(CORO_PARAM, bool bDrawLocation) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	g_system->lockMutex(csMainLoop);
	
	// Poll dei dispositivi di input
	m_input.Poll();

	if (m_bMustEnterMenu && GLOBALS.bIdleExited) {
		m_bOption = true;
		m_bMustEnterMenu = false;
		GLOBALS.bIdleExited = false;
	}
	
  if (m_bOption) {
		CORO_INVOKE_1(m_opt.DoFrame, &m_input);
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
			// Se siamo sull'inventario, è lui che controlla tutti gli input
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
						CORO_INVOKE_3(m_tony.MoveAndDoAction, m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), m_curAction);

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
							CORO_INVOKE_1(OpenOptionScreen, 0);
							goto SKIPCLICKSINISTRO;
						} else if (m_input.GetAsyncKeyState(Common::KEYCODE_ESCAPE))
							CORO_INVOKE_1(OpenOptionScreen, 0);
						else if (!_vm->getIsDemo()) {
							if (m_input.GetAsyncKeyState(Common::KEYCODE_F3) || m_input.GetAsyncKeyState(Common::KEYCODE_F5))
								// Save game screen
								CORO_INVOKE_1(OpenOptionScreen, 3);
							else if (m_input.GetAsyncKeyState(Common::KEYCODE_F2) || m_input.GetAsyncKeyState(Common::KEYCODE_F7))
								// Load game screen
								CORO_INVOKE_1(OpenOptionScreen, 4);
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
							CORO_INVOKE_3(m_tony.MoveAndDoAction, m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), m_point.CurAction());
						else if (m_itemName.GetSelectedItem() != NULL)
							CORO_INVOKE_4(m_tony.MoveAndDoAction, m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), TA_COMBINE, m_curActionObj);
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
							CORO_INVOKE_3(m_tony.MoveAndDoAction, m_itemName.GetHotspot(), m_itemName.GetSelectedItem(), m_curAction);

							m_curAction = TA_GOTO;
							m_point.SetAction(m_curAction);
						}
					}
				}
			}

			// Aggiorna il nome sotto il puntatore del mouse
			m_itemName.SetMouseCoord(m_input.MousePos());
			if (!m_inter.Active() && !m_inv.MiniActive())
				CORO_INVOKE_4(m_itemName.DoFrame, m_bigBuf, m_loc, m_point, m_inv);	
		}

		// Inventario & interfaccia
		m_inter.DoFrame(m_bigBuf, m_input.MousePos());
		m_inv.DoFrame(m_bigBuf, m_point, m_input.MousePos(), (!m_tony.InAction() && !m_inter.Active() && m_bGUIInventory));
	}

	// Anima Tony
	CORO_INVOKE_2(m_tony.DoFrame, &m_bigBuf, m_nCurLoc);
	
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
	CORO_INVOKE_0(m_bigBuf.DrawOT);

#define FSTEP (480/32)

	// Wipe
	if (m_bWiping) {
		switch (m_nWipeType) {
			case 1:
				if (!(m_rcWipeEllipse.bottom - m_rcWipeEllipse.top >= FSTEP * 2)) {
					CoroScheduler.setEvent(m_hWipeEvent);
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
					CoroScheduler.setEvent(m_hWipeEvent);
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

	CORO_END_CODE;
}


void RMGfxEngine::InitCustomDll(void) {
	SetupGlobalVars(&m_tony, &m_point, &_vm->_theBoxes, &m_loc, &m_inv, &m_input);
}

void RMGfxEngine::ItemIrq(uint32 dwItem, int nPattern, int nStatus) {
	RMItem *item;
	assert(GLOBALS.GfxEngine);

	if (GLOBALS.GfxEngine->m_bLocationLoaded) {
		item = GLOBALS.GfxEngine->m_loc.GetItemFromCode(dwItem);
		if (item != NULL) {
			if (nPattern != -1) {
				if (GLOBALS.bPatIrqFreeze)
					MainFreeze();
				item->SetPattern(nPattern,true);
				if (GLOBALS.bPatIrqFreeze)
					MainUnfreeze();
			}
			if (nStatus != -1)
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


void RMGfxEngine::SelectLocation(const RMPoint &ptTonyStart, const RMPoint &start) {
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

uint32 RMGfxEngine::LoadLocation(int nLoc, RMPoint ptTonyStart, RMPoint start) {
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
	return CORO_INVALID_PID_VALUE; //mpalQueryDoAction(0,m_nCurLoc,0);
}

void RMGfxEngine::UnloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result) {
	CORO_BEGIN_CONTEXT;
		uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	
	// Scarica tutta la memoria della locazione
	CORO_INVOKE_2(mpalEndIdlePoll, m_nCurLoc, NULL);

	// On Exit?
	if (bDoOnExit) {
		_ctx->h = mpalQueryDoAction(1, m_nCurLoc, 0);
		if (_ctx->h != CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
	}

	MainFreeze();

	m_bLocationLoaded = false;
	
	m_bigBuf.ClearOT();
	m_loc.Unload();

	if (result != NULL)
		*result = CORO_INVALID_PID_VALUE;

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
	m_bigBuf.DrawOT(Common::nullContext);
	m_bigBuf.ClearOT();
	delete load;
	_vm->_window.GetNewFrame(*this, NULL);



	GLOBALS.bPatIrqFreeze = true;

	// GUI attivabile
	m_bGUIOption = true;
	m_bGUIInterface = true;
	m_bGUIInventory = true;

	GLOBALS.bSkipSfxNoLoop = false;
	m_bMustEnterMenu = false;
	GLOBALS.bIdleExited = false;
	m_bOption = false;
	m_bWiping = false;
	m_hWipeEvent = CoroScheduler.createEvent(false, false);

	// Crea l'evento di freeze
	csMainLoop = g_system->createMutex();

	// Inizializza la funzione di IRQ di Item per l'MPAL
	GLOBALS.GfxEngine = this;
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

	// Carica la locazione e setta le priorità		@@@@@
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

void RestoreMusic(CORO_PARAM);
void SaveMusic(Common::OutSaveFile *f);
void LoadMusic(Common::InSaveFile *f);

#define TONY_SAVEGAME_VERSION 8

void RMGfxEngine::SaveState(const char *fn, byte *curThumb, const char *name, bool bFastCompress) {
	Common::OutSaveFile *f;
	byte *state;
	uint thumbsize;
	uint size;
	int i;
	char buf[4];
	RMPoint tp = m_tony.Position();

	// Saving: mpal variables, current location, + tony inventory position

	// For now, we only save the MPAL state
	size = mpalGetSaveStateSize();
	state = new byte[size];
	mpalSaveState(state);

	thumbsize = 160 * 120 * 2;
	
	buf[0] = 'R';
	buf[1] = 'M';
	buf[2] = 'S';
	buf[3] = TONY_SAVEGAME_VERSION;

	f = g_system->getSavefileManager()->openForSaving(fn);
	if (f == NULL) 
		return;

	f->write(buf, 4);
	f->writeUint32LE(thumbsize);
	f->write(curThumb, thumbsize);

	// Difficulty level
	i = mpalQueryGlobalVar("VERSIONEFACILE");
	f->writeByte(i);

	i = strlen(name);
	f->writeByte(i);
	f->write(name, i);
	f->writeUint32LE(m_nCurLoc);
	f->writeUint32LE(tp.x);
	f->writeUint32LE(tp.y);

	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// Inventory
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
	
	// Saves the state of the shepherdess and show yourself
	bStat = m_tony.GetPastorella();
	f->writeByte(bStat);
	bStat = m_inter.GetPalesati();
	f->writeByte(bStat);
	
	// Save the chars
	CharsSaveAll(f);

	// Save the options
	f->writeByte(GLOBALS.bCfgInvLocked);
	f->writeByte(GLOBALS.bCfgInvNoScroll);
	f->writeByte(GLOBALS.bCfgTimerizedText);
	f->writeByte(GLOBALS.bCfgInvUp);
	f->writeByte(GLOBALS.bCfgAnni30);
	f->writeByte(GLOBALS.bCfgAntiAlias);
	f->writeByte(GLOBALS.bCfgSottotitoli);
	f->writeByte(GLOBALS.bCfgTransparence);
	f->writeByte(GLOBALS.bCfgInterTips);
	f->writeByte(GLOBALS.bCfgDubbing);
	f->writeByte(GLOBALS.bCfgMusic);
	f->writeByte(GLOBALS.bCfgSFX);
	f->writeByte(GLOBALS.nCfgTonySpeed);
	f->writeByte(GLOBALS.nCfgTextSpeed);
	f->writeByte(GLOBALS.nCfgDubbingVolume);
	f->writeByte(GLOBALS.nCfgMusicVolume);
	f->writeByte(GLOBALS.nCfgSFXVolume);

	// Save the hotspots
	SaveChangedHotspot(f);

	// Save the music
	SaveMusic(f);

	f->finalize();
	delete f;
}

void RMGfxEngine::LoadState(CORO_PARAM, const char *fn) {
	// PROBLEMA: Bisognerebbe caricare la locazione in un thread a parte per fare la OnEnter ...
	CORO_BEGIN_CONTEXT;
		Common::InSaveFile *f;
		byte *state, *statecmp;
		uint size, sizecmp;
		char buf[4];
		RMPoint tp;
		int loc;
		int ver;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->f = g_system->getSavefileManager()->openForLoading(fn);
	if (_ctx->f == NULL) 
		return;
	_ctx->f->read(_ctx->buf, 4);

	if (_ctx->buf[0] != 'R' || _ctx->buf[1] != 'M' || _ctx->buf[2] != 'S') {
		delete _ctx->f;
		return;
	}
	
	_ctx->ver = _ctx->buf[3];
	
	if (_ctx->ver == 0 || _ctx->ver > TONY_SAVEGAME_VERSION) {
		delete _ctx->f;
		return;
	}
	
	if (_ctx->ver >= 0x3) {
		// There is a thumbnail. If the version is between 5 and 7, it's compressed
		if ((_ctx->ver >= 0x5) && (_ctx->ver <= 0x7)) {
			_ctx->i = 0;
			_ctx->i = _ctx->f->readUint32LE();
			_ctx->f->seek(_ctx->i);
		} else {
			if (_ctx->ver >= 8)
				// Skip thumbnail size
				_ctx->f->skip(4);

			_ctx->f->seek(160 * 120 * 2, SEEK_CUR);
		}
	}

	if (_ctx->ver >= 0x5) {
		// Skip the difficulty level
		_ctx->f->seek(1, SEEK_CUR);
	}

	if (_ctx->ver >= 0x4) {	// Skip the savegame name, which serves no purpose
		_ctx->i = _ctx->f->readByte();
		_ctx->f->seek(_ctx->i, SEEK_CUR);
	}

	_ctx->loc = _ctx->f->readUint32LE();
	_ctx->tp.x = _ctx->f->readUint32LE();
	_ctx->tp.y = _ctx->f->readUint32LE();
	_ctx->size = _ctx->f->readUint32LE();

	if ((_ctx->ver >= 0x5) && (_ctx->ver <= 7)) {
		// MPAL was packed!
		_ctx->sizecmp = _ctx->f->readUint32LE();
		_ctx->state = new byte[_ctx->size];
		_ctx->statecmp = new byte[_ctx->sizecmp];
		_ctx->f->read(_ctx->statecmp, _ctx->sizecmp);
		lzo1x_decompress(_ctx->statecmp,_ctx->sizecmp,_ctx->state,&_ctx->size);
		delete[] _ctx->statecmp;
	} else {
		// Read uncompressed MPAL data
		_ctx->state = new byte[_ctx->size];
		_ctx->f->read(_ctx->state, _ctx->size);
	}

	mpalLoadState(_ctx->state);
	delete[] _ctx->state;

	// Inventory
	_ctx->size = _ctx->f->readUint32LE();
	_ctx->state = new byte[_ctx->size];
	_ctx->f->read(_ctx->state, _ctx->size);
	m_inv.LoadState(_ctx->state);
	delete[] _ctx->state;

	if (_ctx->ver >= 0x2) {	  // Versione 2: box please
		_ctx->size = _ctx->f->readUint32LE();
		_ctx->state = new byte[_ctx->size];
		_ctx->f->read(_ctx->state, _ctx->size);
		_vm->_theBoxes.LoadState(_ctx->state);
		delete[] _ctx->state;
	}

	if (_ctx->ver >= 5) {
	  // Versione 5: 
		bool bStat = false;
		
		bStat = _ctx->f->readByte();
		m_tony.SetPastorella(bStat);		
		bStat = _ctx->f->readByte();
		m_inter.SetPalesati(bStat);		

		CharsLoadAll(_ctx->f);
	}

	if (_ctx->ver >= 6) {
		// Load options
		GLOBALS.bCfgInvLocked = _ctx->f->readByte();
		GLOBALS.bCfgInvNoScroll = _ctx->f->readByte();
		GLOBALS.bCfgTimerizedText = _ctx->f->readByte();
		GLOBALS.bCfgInvUp = _ctx->f->readByte();
		GLOBALS.bCfgAnni30 = _ctx->f->readByte();
		GLOBALS.bCfgAntiAlias = _ctx->f->readByte();
		GLOBALS.bCfgSottotitoli = _ctx->f->readByte();
		GLOBALS.bCfgTransparence = _ctx->f->readByte();
		GLOBALS.bCfgInterTips = _ctx->f->readByte();
		GLOBALS.bCfgDubbing = _ctx->f->readByte();
		GLOBALS.bCfgMusic = _ctx->f->readByte();
		GLOBALS.bCfgSFX = _ctx->f->readByte();
		GLOBALS.nCfgTonySpeed = _ctx->f->readByte();
		GLOBALS.nCfgTextSpeed = _ctx->f->readByte();
		GLOBALS.nCfgDubbingVolume = _ctx->f->readByte();
		GLOBALS.nCfgMusicVolume = _ctx->f->readByte();
		GLOBALS.nCfgSFXVolume = _ctx->f->readByte();

		// Load hotspots
		LoadChangedHotspot(_ctx->f);
	}

	if (_ctx->ver >= 7) {
		LoadMusic(_ctx->f);
	}

	delete _ctx->f;

	CORO_INVOKE_2(UnloadLocation, false, NULL);
	LoadLocation(_ctx->loc,_ctx->tp,RMPoint(-1, -1));
	m_tony.SetPattern(RMTony::PAT_STANDRIGHT);
	MainUnfreeze();
	
	// On older versions, need to an enter action
	if (_ctx->ver < 5)
		mpalQueryDoAction(0, _ctx->loc, 0);
	else {
		// In the new ones, we just reset the mcode
		MCharResetCodes();
	}

	if (_ctx->ver >= 6)
		ReapplyChangedHotspot();

	CORO_INVOKE_0(RestoreMusic);

	m_bGUIInterface = true;
	m_bGUIInventory = true;
	m_bGUIOption = true;

	CORO_END_CODE;
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

void RMGfxEngine::WaitWipeEnd(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, m_hWipeEvent, CORO_INFINITE);
}

} // End of namespace Tony
