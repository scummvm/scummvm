/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/Notebook.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

//////////////////////////////////////////////////////////////////////////
// FRONT STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNotebookState_Front::cNotebookState_Front(cInit *apInit, cNotebook *apNotebook) : iNotebookState(apInit, apNotebook) {
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpTextBack = mpDrawer->CreateGfxObject("notebook_textback.bmp", "diffalpha2d");

	mpFrontFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	mvOptions.resize(2);

	mvOptions[0].msText = kTranslate("Notebook", "TaskList");
	mvOptions[0].mvPos = cVector3f(400, 220, 70);

	mvOptions[1].msText = kTranslate("Notebook", "PickedNotes");
	mvOptions[1].mvPos = cVector3f(400, 280, 70);

	mlSelected = -1;

	mvFontSize = 21;

	for (size_t i = 0; i < mvOptions.size(); ++i) {
		cRect2f optionRect;
		optionRect.w = mpFrontFont->getLength(mvFontSize, mvOptions[i].msText.c_str());
		optionRect.h = mvFontSize.y + 8;
		optionRect.x = mvOptions[i].mvPos.x - optionRect.w / 2;
		optionRect.y = mvOptions[i].mvPos.y;

		mvOptions[i].mRect = optionRect;
	}
}

//-----------------------------------------------------------------------

void cNotebookState_Front::OnEnterState(int alLastState) {
	mvOptions[0].msText = kTranslate("Notebook", "TaskList");
	mvOptions[1].msText = kTranslate("Notebook", "PickedNotes");

	mpNotebook->SetBookType(eNotebookType_Front);
}

//-----------------------------------------------------------------------

void cNotebookState_Front::OnLeaveState(int alNextState) {
}

//-----------------------------------------------------------------------

void cNotebookState_Front::OnUpdate(float afTime) {
	bool bFound = false;
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (cMath::PointBoxCollision(mpNotebook->GetMousePos(), mvOptions[i].mRect)) {
			if (mlSelected != (int)i) {
				mlSelected = (int)i;
				mvOptions[i].mfAlpha = 0;
			}
			bFound = true;

			mvOptions[i].mfAlpha += 1.8f * afTime;
			if (mvOptions[i].mfAlpha > 1.0f)
				mvOptions[i].mfAlpha = 1.0f;
		} else {
			mvOptions[i].mfAlpha -= 1.3f * afTime;
			if (mvOptions[i].mfAlpha < 0.0f)
				mvOptions[i].mfAlpha = 0.0f;
		}
	}

	if (bFound == false)
		mlSelected = -1;
}

//-----------------------------------------------------------------------

void cNotebookState_Front::OnDraw() {
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (mvOptions[i].mfAlpha > 0) {
			mpFrontFont->draw(mvOptions[i].mvPos + cVector3f(0, 0, 1), mvFontSize,
							  cColor(0.9f, 0.9f, 0.9f, 1.0f * mpNotebook->GetAlpha() * mvOptions[i].mfAlpha),
							  eFontAlign_Center, mvOptions[i].msText);

			mpDrawer->DrawGfxObject(mpTextBack, mvOptions[i].mvPos + cVector3f(-160, -2, -1),
									cVector2f(320, mvFontSize.y + 15), cColor(1, mvOptions[i].mfAlpha * 0.7f));
		}

		mpFrontFont->draw(mvOptions[i].mvPos, mvFontSize, cColor(0.7f, 0.7f, 0.7f, 0.8f * mpNotebook->GetAlpha()),
						  eFontAlign_Center, mvOptions[i].msText);
		mpFrontFont->draw(mvOptions[i].mvPos + cVector3f(1, 1, -1), mvFontSize, cColor(0.0f, 0.0f, 0.0f, 0.8f * mpNotebook->GetAlpha()),
						  eFontAlign_Center, mvOptions[i].msText);
	}
}

//-----------------------------------------------------------------------

void cNotebookState_Front::OnMouseDown(eMButton aButton) {
	if (mlSelected == 0) {
		mpStateMachine->ChangeState(eNotebookState_TaskList);
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_open", false, 1);
	} else if (mlSelected == 1) {
		mpStateMachine->ChangeState(eNotebookState_NoteList);
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_open", false, 1);
	}
}
//-----------------------------------------------------------------------

void cNotebookState_Front::OnExit() {
	mpNotebook->SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// TASK LIST STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNotebookState_TaskList::cNotebookState_TaskList(cInit *apInit, cNotebook *apNotebook) : iNotebookState(apInit, apNotebook) {
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpTextFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("cour.fnt", 14);

	mfFontSize = 15;

	//////////////////
	// Options
	mlSelected = -1;
	mpOptionsImage[0] = mpDrawer->CreateGfxObject("notebook_nextpage.bmp", "diffalpha2d");
	mpOptionsImage[1] = mpDrawer->CreateGfxObject("notebook_prevpage.bmp", "diffalpha2d");

	mvOptions.resize(2);

	cVector2f vBookSize(350, 460);
	cVector3f vBookPos(400 - vBookSize.x / 2, 300 - vBookSize.y / 2, 60);

	mvOptions[0].mvPos = cVector3f((vBookPos.x + vBookSize.x) - 119, (vBookPos.y + vBookSize.y) - 120, 15);
	mvOptions[1].mvPos = cVector3f(vBookPos.x, vBookPos.y + vBookSize.y - 120, 15);

	for (size_t i = 0; i < mvOptions.size(); ++i) {
		cRect2f optionRect;
		if (i == 0) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[0].mvPos.x;
			optionRect.y = mvOptions[0].mvPos.y;
		} else if (i == 1) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[1].mvPos.x;
			optionRect.y = mvOptions[1].mvPos.y;
		}

		mvOptions[i].mRect = optionRect;
	}
}

//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnEnterState(int alLastState) {
	mvTaskPages.clear();
	mpNotebook->SetBookType(eNotebookType_Open);

	mlCurrentPage = 0;
	mlSelected = -1;

	int lStart = 0;
	int lCount = 0;
	float fYPos = 0;
	tWStringVec vTempStrings;

	tNotebook_BookTaskList_Iterator it = mpNotebook->GetTaskIterator();
	while (it.HasNext()) {
		cNotebook_BookTask *pTask = it.Next();

		mpTextFont->getWordWrapRows(308, mfFontSize, mfFontSize - 1, _W("- ") + pTask->msText, &vTempStrings);

		fYPos += ((float)vTempStrings.size()) * mfFontSize;

		// Log("%d pos: %f\n",lCount,fYPos);

		if (fYPos > 360.0f) {
			cNotebook_TaskPage taskPage;

			taskPage.mlStartTask = lStart;
			taskPage.mlEndTask = lCount - 1;
			lStart = lCount;

			// Log("Too far! page: %d - %d\n",taskPage.mlStartTask,taskPage.mlEndTask);

			mvTaskPages.push_back(taskPage);

			fYPos = ((float)vTempStrings.size()) * mfFontSize;
		}

		// Space between tasks
		fYPos += 30;

		lCount++;
		vTempStrings.clear();
	}

	if (lStart < mpNotebook->GetTaskNum()) {
		cNotebook_TaskPage taskPage;

		taskPage.mlStartTask = lStart;
		taskPage.mlEndTask = lCount - 1;

		mvTaskPages.push_back(taskPage);
	}
}

//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnLeaveState(int alNextState) {
}

//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnUpdate(float afTime) {
	//////////////////////////////////
	// Next/Prev Page
	bool bFound = false;
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (cMath::PointBoxCollision(mpNotebook->GetMousePos(), mvOptions[i].mRect)) {
			if (mlSelected != (int)i) {
				mlSelected = (int)i;
				mvOptions[i].mfAlpha = 0;
			}
			bFound = true;

			mvOptions[i].mfAlpha += 2.5f * afTime;
			if (mvOptions[i].mfAlpha > 1.0f)
				mvOptions[i].mfAlpha = 1.0f;
		} else {
			mvOptions[i].mfAlpha -= 1.3f * afTime;
			if (mvOptions[i].mfAlpha < 0.0f)
				mvOptions[i].mfAlpha = 0.0f;
		}
	}

	if (bFound == false)
		mlSelected = -1;
}

//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnDraw() {
	cVector3f vPos(254, 110, 70);

	int lCount = 0;

	tNotebook_BookTaskList_Iterator it = mpNotebook->GetTaskIterator();
	while (it.HasNext()) {
		cNotebook_BookTask *pTask = it.Next();

		if (lCount < mvTaskPages[mlCurrentPage].mlStartTask ||
			lCount > mvTaskPages[mlCurrentPage].mlEndTask) {
			++lCount;
			continue;
		}

		mpTextFont->draw(vPos + cVector3f(-12, 0, 0), 14, cColor(0.0f, 0.0f, 0.0f, 0.9f), eFontAlign_Left,
						 _W("-"));
		int lRows = mpTextFont->drawWordWrap(vPos, 308, 15, 14, cColor(0.0f, 0.0f, 0.0f, 0.9f), eFontAlign_Left, pTask->msText);

		vPos.y += 30.0f + (float)lRows * 15.0f;

		++lCount;
	}

	//////////////////////////////////
	// Page number
	mpTextFont->draw(cVector3f(400, 490, 16), 15, cColor(0.05f, mpNotebook->GetAlpha()), eFontAlign_Center,
					 Common::U32String::format("%d", mlCurrentPage + 1));

	//////////////////////////////////
	// Draw arrows back and forward
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (i == 0 && mlCurrentPage >= (int)mvTaskPages.size() - 1)
			continue;
		if (i == 1 && mlCurrentPage == 0)
			continue;

		mpDrawer->DrawGfxObject(mpOptionsImage[i], mvOptions[i].mvPos,
								cVector2f(mvOptions[i].mRect.w, mvOptions[i].mRect.h),
								cColor(1, mpNotebook->GetAlpha()));
	}
}

//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnMouseDown(eMButton aButton) {
	// Forward
	if (mlSelected == 0) {
		if (mlCurrentPage < (int)mvTaskPages.size() - 1) {
			mlCurrentPage++;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
	// Backward
	else if (mlSelected == 1) {
		if (mlCurrentPage > 0) {
			mlCurrentPage--;
			if (mlCurrentPage < 0)
				mlCurrentPage = 0;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
}
//-----------------------------------------------------------------------

void cNotebookState_TaskList::OnExit() {
	mpStateMachine->ChangeState(eNotebookState_Front);
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_close", false, 1);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NOTE LIST STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNotebookState_NoteList::cNotebookState_NoteList(cInit *apInit, cNotebook *apNotebook) : iNotebookState(apInit, apNotebook) {
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpTextBack = mpDrawer->CreateGfxObject("notebook_listtextback.bmp", "diffalpha2d");
	mpTab = mpDrawer->CreateGfxObject("notebook_tab.bmp", "diffalpha2d");

	mpTextFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt", 14);

	msUnread = kTranslate("Notebook", "Unread");

	mvSize = cVector2f(14, 14);

	mlMaxNotesPerPage = 11;
	mlFirstNote = 0;
	mlSelected = -1;

	mpOptionsImage[0] = mpDrawer->CreateGfxObject("notebook_nextpage.bmp", "diffalpha2d");
	mpOptionsImage[1] = mpDrawer->CreateGfxObject("notebook_prevpage.bmp", "diffalpha2d");

	mvOptions.resize(2);

	cVector2f vBookSize(350, 460);
	cVector3f vBookPos(400 - vBookSize.x / 2, 300 - vBookSize.y / 2, 60);

	mvOptions[0].mvPos = cVector3f((vBookPos.x + vBookSize.x) - 119, (vBookPos.y + vBookSize.y) - 120, 15);

	mvOptions[1].mvPos = cVector3f(vBookPos.x, vBookPos.y + vBookSize.y - 120, 15);

	for (size_t i = 0; i < mvOptions.size(); ++i) {
		cRect2f optionRect;
		if (i == 0) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[0].mvPos.x;
			optionRect.y = mvOptions[0].mvPos.y;
		} else if (i == 1) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[1].mvPos.x;
			optionRect.y = mvOptions[1].mvPos.y;
		}

		mvOptions[i].mRect = optionRect;
	}
}

//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnEnterState(int alLastState) {
	mpNotebook->SetBookType(eNotebookType_Open);

	mpSelectedNote = NULL;

	if (alLastState != eNotebookState_Note)
		mlFirstNote = (mpNotebook->GetNoteNum() / 12) * 11;

	mlSelected = -1;

	//////////////////////////////////////////
	// Setup pos and size of all entries
	int lCount = 0;
	cVector3f vPos(242, 111, 70);
	tNotebook_NoteList_Iterator it = mpNotebook->GetNoteIterator();
	while (it.HasNext()) {
		if (lCount == mlMaxNotesPerPage) {
			lCount = 0;
			vPos = cVector3f(242, 111, 70);
		}

		cNotebook_Note *pNote = it.Next();

		pNote->mRect.x = vPos.x;
		pNote->mRect.y = vPos.y + 6;
		pNote->mRect.w = 300;
		pNote->mRect.h = mvSize.y + 12;

		vPos.y += mvSize.y + 15;

		lCount++;
	}
}

//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnLeaveState(int alNextState) {
}

//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnUpdate(float afTime) {
	mpSelectedNote = NULL;

	//////////////////////////////////
	// Notes
	int lCount = 0;
	tNotebook_NoteList_Iterator it = mpNotebook->GetNoteIterator();
	while (it.HasNext()) {
		cNotebook_Note *pNote = it.Next();
		if (lCount < mlFirstNote || lCount >= mlFirstNote + mlMaxNotesPerPage) {
			lCount++;
			continue;
		}

		if (cMath::PointBoxCollision(mpNotebook->GetMousePos(), pNote->mRect)) {
			mpSelectedNote = pNote;
		}

		if (pNote == mpSelectedNote) {
			pNote->mfAlpha += 2.5f * afTime;
			if (pNote->mfAlpha > 1.0f)
				pNote->mfAlpha = 1.0f;
		} else {
			pNote->mfAlpha -= 2.9f * afTime;
			if (pNote->mfAlpha < 0.0f)
				pNote->mfAlpha = 0.0f;
		}

		lCount++;
	}

	//////////////////////////////////
	// Next/Prev Page
	bool bFound = false;
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (cMath::PointBoxCollision(mpNotebook->GetMousePos(), mvOptions[i].mRect)) {
			if (mlSelected != (int)i) {
				mlSelected = (int)i;
				mvOptions[i].mfAlpha = 0;
			}
			bFound = true;

			mvOptions[i].mfAlpha += 2.5f * afTime;
			if (mvOptions[i].mfAlpha > 1.0f)
				mvOptions[i].mfAlpha = 1.0f;
		} else {
			mvOptions[i].mfAlpha -= 1.3f * afTime;
			if (mvOptions[i].mfAlpha < 0.0f)
				mvOptions[i].mfAlpha = 0.0f;
		}
	}

	if (bFound == false)
		mlSelected = -1;
}

//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnDraw() {
	int lCount = 0;
	tNotebook_NoteList_Iterator it = mpNotebook->GetNoteIterator();
	while (it.HasNext()) {
		cNotebook_Note *pNote = it.Next();
		if (lCount < mlFirstNote || lCount >= mlFirstNote + mlMaxNotesPerPage) {
			lCount++;
			continue;
		}

		cVector3f vTabPos(400 - 175, pNote->mRect.y, 68);
		cVector2f vTabSize(14, pNote->mRect.h + 3);
		mpDrawer->DrawGfxObject(mpTab, vTabPos, vTabSize, cColor(1, mpNotebook->GetAlpha()));

		cVector3f vTextPos(pNote->mRect.x, pNote->mRect.y, 70);

		if (pNote->mfAlpha < 1) {
			float fAlpha = 0.9f * mpNotebook->GetAlpha() * (1 - pNote->mfAlpha);
			mpTextFont->draw(vTextPos, mvSize, cColor(0, fAlpha), eFontAlign_Left, pNote->msName);
		}

		if (pNote->mfAlpha > 0) {
			float fAlpha = pNote->mfAlpha * mpNotebook->GetAlpha();
			float fShadowWidth = mpTextFont->getLength(mvSize, pNote->msName.c_str());

			cVector3f vPos(pNote->mRect.x - 10, pNote->mRect.y, 69);
			cVector2f vSize(fShadowWidth + 20, pNote->mRect.h - 2);

			mpDrawer->DrawGfxObject(mpTextBack, vPos, vSize, cColor(1, fAlpha * 0.7f));

			mpTextFont->draw(vTextPos, mvSize, cColor(0.85f, fAlpha), eFontAlign_Left, pNote->msName);
		}

		lCount++;
	}

	//////////////////////////////////
	// Page number
	mpTextFont->draw(cVector3f(400, 490, 16), 17, cColor(0.05f, mpNotebook->GetAlpha()), eFontAlign_Center,
					 Common::U32String::format("%d", (mlFirstNote / 11) + 1));

	//////////////////////////////////
	// Draw arrows back and forward
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (i == 0 && mlFirstNote >= mpNotebook->GetNoteNum() - mlMaxNotesPerPage)
			continue;
		if (i == 1 && mlFirstNote == 0)
			continue;

		mpDrawer->DrawGfxObject(mpOptionsImage[i], mvOptions[i].mvPos,
								cVector2f(mvOptions[i].mRect.w, mvOptions[i].mRect.h),
								cColor(1, mpNotebook->GetAlpha()));
	}
}

//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnMouseDown(eMButton aButton) {
	if (mpSelectedNote != NULL) {
		mpNotebook->SetSelectedNote(mpSelectedNote);
		mpStateMachine->ChangeState(eNotebookState_Note);
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_open", false, 1);
	}

	// Forward
	if (mlSelected == 0) {
		if (mlFirstNote < mpNotebook->GetNoteNum() - mlMaxNotesPerPage) {
			mlFirstNote += mlMaxNotesPerPage;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
	// Backward
	else if (mlSelected == 1) {
		if (mlFirstNote > 0) {
			mlFirstNote -= mlMaxNotesPerPage;
			if (mlFirstNote < 0)
				mlFirstNote = 0;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
}
//-----------------------------------------------------------------------

void cNotebookState_NoteList::OnExit() {
	mpStateMachine->ChangeState(eNotebookState_Front);
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_close", false, 1);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NOTE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNotebookState_Note::cNotebookState_Note(cInit *apInit, cNotebook *apNotebook) : iNotebookState(apInit, apNotebook) {
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpOptionsImage[0] = mpDrawer->CreateGfxObject("notebook_nextpage.bmp", "diffalpha2d");
	mpOptionsImage[1] = mpDrawer->CreateGfxObject("notebook_prevpage.bmp", "diffalpha2d");

	mpTextFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("cour.fnt", 14);
	mpNavigateFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	mvSize = cVector2f(14, 14);
	mlMaxRows = 22;

	mvOptions.resize(2);

	cVector2f vBookSize(350, 460);
	cVector3f vBookPos(400 - vBookSize.x / 2, 300 - vBookSize.y / 2, 60);

	mvOptions[0].msText = _W(">>>");
	mvOptions[0].mvPos = cVector3f((vBookPos.x + vBookSize.x) - 119,
								   (vBookPos.y + vBookSize.y) - 120,
								   15);

	mvOptions[1].msText = _W("<<<");
	mvOptions[1].mvPos = cVector3f(vBookPos.x,
								   vBookPos.y + vBookSize.y - 120,
								   15);

	for (size_t i = 0; i < mvOptions.size(); ++i) {
		cRect2f optionRect;
		if (i == 0) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[0].mvPos.x;
			optionRect.y = mvOptions[0].mvPos.y;
		} else if (i == 1) {
			optionRect.w = 119;
			optionRect.h = 120;
			optionRect.x = mvOptions[1].mvPos.x;
			optionRect.y = mvOptions[1].mvPos.y;
		}

		mvOptions[i].mRect = optionRect;
	}
}

//-----------------------------------------------------------------------

void cNotebookState_Note::OnEnterState(int alLastState) {
	mpNotebook->SetBookType(eNotebookType_Open);
	cNotebook_Note *pNote = mpNotebook->GetSelectedNote();

	mvRows.clear();
	mvPages.clear();
	mpTextFont->getWordWrapRows(320, mvSize.y + 5, mvSize, pNote->msText, &mvRows);

	int lStartRow = 0;
	int lRowCount = 0;
	for (size_t i = 0; i < mvRows.size(); ++i) {
		// Log("Row%d: %s\n",i, mvRows[i].c_str());

		lRowCount++;

		// Log("Row: %d\n",lRowCount);
		if (cString::GetLastStringPosW(mvRows[i], _W("[new_page]")) >= 0) {
			// There some text on this page, make a newer one
			if (lRowCount > 1) {
				cNotePage page;
				page.mlStart = lStartRow;
				page.mlEnd = lStartRow + lRowCount - 1;
				lStartRow = page.mlEnd + 1;
				mvPages.push_back(page);
			}
			// No text on page, skip the newer page.
			else {
				lStartRow++;
			}
			lRowCount = 0;
			// Log("NEWPAGE Added page: %d - %d\n",page.mlStart,page.mlEnd);
		} else if (lRowCount >= mlMaxRows) {
			cNotePage page;
			page.mlStart = lStartRow;
			page.mlEnd = lStartRow + lRowCount;
			lStartRow = page.mlEnd;
			mvPages.push_back(page);
			lRowCount = 0;
			// Log("MAX ROW Added page: %d - %d\n",page.mlStart,page.mlEnd);
		}
	}
	if (lStartRow < (int)mvRows.size()) {
		cNotePage page;
		page.mlStart = lStartRow;
		page.mlEnd = (int)mvRows.size();
		mvPages.push_back(page);
		// Log("Added page: %d - %d\n",page.mlStart,page.mlEnd);
	}

	mlCurrentPage = 0;

	mlSelected = -1;
}

//-----------------------------------------------------------------------

void cNotebookState_Note::OnLeaveState(int alNextState) {
}

//-----------------------------------------------------------------------

void cNotebookState_Note::OnUpdate(float afTime) {
	bool bFound = false;
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (cMath::PointBoxCollision(mpNotebook->GetMousePos(), mvOptions[i].mRect)) {
			if (mlSelected != (int)i) {
				mlSelected = (int)i;
				mvOptions[i].mfAlpha = 0;
			}
			bFound = true;

			mvOptions[i].mfAlpha += 2.5f * afTime;
			if (mvOptions[i].mfAlpha > 1.0f)
				mvOptions[i].mfAlpha = 1.0f;
		} else {
			mvOptions[i].mfAlpha -= 1.3f * afTime;
			if (mvOptions[i].mfAlpha < 0.0f)
				mvOptions[i].mfAlpha = 0.0f;
		}
	}

	if (bFound == false)
		mlSelected = -1;
}

//-----------------------------------------------------------------------

void cNotebookState_Note::OnDraw() {
	//////////////////////////////////
	// Draw page text
	cVector3f vPos(235, 107, 70);
	for (int i = mvPages[mlCurrentPage].mlStart; i < mvPages[mlCurrentPage].mlEnd; ++i) {
		mpTextFont->draw(vPos, mvSize, cColor(0, 1.0f * mpNotebook->GetAlpha()),
						 eFontAlign_Left, mvRows[i]);

		vPos.y += mvSize.y + 2;
	}

	//////////////////////////////////
	// Page number
	mpTextFont->draw(cVector3f(400, 490, 16), 15, cColor(0.05f, mpNotebook->GetAlpha()), eFontAlign_Center,
					 Common::U32String::format("%d", mlCurrentPage + 1));

	//////////////////////////////////
	// Draw arrows back and forward
	for (size_t i = 0; i < mvOptions.size(); ++i) {
		if (i == 0 && mlCurrentPage == (int)mvPages.size() - 1)
			continue;
		if (i == 1 && mlCurrentPage == 0)
			continue;

		mpDrawer->DrawGfxObject(mpOptionsImage[i], mvOptions[i].mvPos,
								cVector2f(mvOptions[i].mRect.w, mvOptions[i].mRect.h),
								cColor(1, mpNotebook->GetAlpha()));
	}
}

//-----------------------------------------------------------------------

void cNotebookState_Note::OnMouseDown(eMButton aButton) {
	// Forward
	if (mlSelected == 0) {
		if (mlCurrentPage < (int)mvPages.size() - 1) {
			mlCurrentPage++;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
	// Backward
	else if (mlSelected == 1) {
		if (mlCurrentPage > 0) {
			mlCurrentPage--;
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_page", false, 1);
		}
	}
}
//-----------------------------------------------------------------------

void cNotebookState_Note::OnExit() {
	mpStateMachine->ChangeState(eNotebookState_NoteList);
	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_open", false, 1);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cNotebook::cNotebook(cInit *apInit) : iUpdateable("Notebook") {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mbInventoryWasActive = false;

	Reset();

	mpGfxBackground = mpDrawer->CreateGfxObject("notebook_background.bmp", "diffalpha2d");

	mStateMachine.AddState(hplNew(cNotebookState_Front, (mpInit, this)), "Front", eNotebookState_Front, mpInit->mpGame->GetStepSize());
	mStateMachine.AddState(hplNew(cNotebookState_TaskList, (mpInit, this)), "TaskList", eNotebookState_TaskList, mpInit->mpGame->GetStepSize());
	mStateMachine.AddState(hplNew(cNotebookState_NoteList, (mpInit, this)), "NoteList", eNotebookState_NoteList, mpInit->mpGame->GetStepSize());
	mStateMachine.AddState(hplNew(cNotebookState_Note, (mpInit, this)), "Note", eNotebookState_Note, mpInit->mpGame->GetStepSize());

	///////////////////////
	// Book types
	mvBookTypes.resize(2);

	// front
	mvBookTypes[0].mpGfxImage = mpDrawer->CreateGfxObject("notebook_front.bmp", "diffalpha2d");
	mvBookTypes[0].mfAlpha = 1;
	mvBookTypes[0].mType = eNotebookType_Front;

	// open
	mvBookTypes[1].mpGfxImage = mpDrawer->CreateGfxObject("notebook_open.bmp", "diffalpha2d");
	mvBookTypes[1].mfAlpha = 0;
	mvBookTypes[1].mType = eNotebookType_Open;
}

//-----------------------------------------------------------------------

cNotebook::~cNotebook(void) {
	STLDeleteAll(mlstTasks);
	STLDeleteAll(mlstNotes);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cNotebook::OnStart() {
}

//-----------------------------------------------------------------------

void cNotebook::Update(float afTimeStep) {
	////////////////////////////////
	// Check active and fade
	if (mbActive == false) {
		mfAlpha -= 2.5f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
		return;
	} else {
		mfAlpha += 2.3f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	}

	////////////////////////////////
	// Book types
	for (size_t i = 0; i < mvBookTypes.size(); ++i) {
		if (mBookType == mvBookTypes[i].mType) {
			mvBookTypes[i].mfAlpha += 3.5f * afTimeStep;
			if (mvBookTypes[i].mfAlpha > 1.0f)
				mvBookTypes[i].mfAlpha = 1.0f;
		} else {
			mvBookTypes[i].mfAlpha -= 3.6f * afTimeStep;
			if (mvBookTypes[i].mfAlpha < 0.0f)
				mvBookTypes[i].mfAlpha = 0.0f;
		}
	}

	//////////////////////////////
	// Update state machine
	mStateMachine.Update(afTimeStep);
}

//-----------------------------------------------------------------------

void cNotebook::Reset() {
	mbActive = false;
	mfAlpha = 0;

	STLDeleteAll(mlstTasks);
	STLDeleteAll(mlstNotes);
}

//-----------------------------------------------------------------------

void cNotebook::OnDraw() {
	if (mfAlpha == 0)
		return;

	mpDrawer->DrawGfxObject(mpGfxBackground, cVector3f(0, 0, 0), cVector2f(800, 600), cColor(1, mfAlpha));

	////////////////////////////////////
	// Book types
	cVector2f vBookSize(350, 460);
	cVector3f vBookPos(400 - vBookSize.x / 2, 300 - vBookSize.y / 2, 10);

	for (size_t i = 0; i < mvBookTypes.size(); ++i) {
		mpDrawer->DrawGfxObject(mvBookTypes[i].mpGfxImage, vBookPos, vBookSize, cColor(1, mfAlpha * mvBookTypes[i].mfAlpha));
	}

	////////////////////////////////////
	// Current state
	static_cast<iNotebookState *>(mStateMachine.CurrentState())->OnDraw();
}

//-----------------------------------------------------------------------

void cNotebook::SetMousePos(const cVector2f &avPos) {
	mvMousePos = avPos;
	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

void cNotebook::AddMousePos(const cVector2f &avRel) {
	mvMousePos += avRel;

	if (mvMousePos.x < 0)
		mvMousePos.x = 0;
	if (mvMousePos.x >= 800)
		mvMousePos.x = 800;
	if (mvMousePos.y < 0)
		mvMousePos.y = 0;
	if (mvMousePos.y >= 600)
		mvMousePos.y = 600;

	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

//-----------------------------------------------------------------------

void cNotebook::OnMouseDown(eMButton aButton) {
	// Check if click is outside of book
	cRect2f bookRect;
	bookRect.w = 350;
	bookRect.h = 460;
	bookRect.x = 400 - bookRect.w / 2;
	bookRect.y = 300 - bookRect.h / 2;
	if (cMath::PointBoxCollision(mvMousePos, bookRect) == false) {
		OnExit();
	} else {
		static_cast<iNotebookState *>(mStateMachine.CurrentState())->OnMouseDown(aButton);
	}
}

//-----------------------------------------------------------------------

void cNotebook::OnMouseUp(eMButton aButton) {
}

//-----------------------------------------------------------------------

void cNotebook::OnExit() {
	static_cast<iNotebookState *>(mStateMachine.CurrentState())->OnExit();
}

//-----------------------------------------------------------------------

tNotebook_NoteList_Iterator cNotebook::GetNoteIterator() {
	return tNotebook_NoteList_Iterator(&mlstNotes);
}

cNotebook_Note *cNotebook::AddNote(const tWString &asName, const tString &asTextCat, const tString &asTextEntry) {
	cNotebook_Note *pNote = hplNew(cNotebook_Note, ());
	pNote->msName = asName;

	pNote->msTextCat = asTextCat;
	pNote->msTextEntry = asTextEntry;

	pNote->msText = kTranslate(asTextCat, asTextEntry);

	mlstNotes.push_back(pNote);

	return pNote;
}

void cNotebook::RemoveNote(const tString &asName) {
	/*tNotebook_NoteListIt it = mlstNotes.begin();
	for(; it != mlstNotes.end(); ){
		cNotebook_Note *pNote = *it;
		if(pNote->msName == asName)	{
			it = mlstNotes.erase(it);
			hplDelete( pNote );
		}
		else{
			++it;
		}
	}*/
}
//-----------------------------------------------------------------------

tNotebook_BookTaskList_Iterator cNotebook::GetTaskIterator() {
	return tNotebook_BookTaskList_Iterator(&mlstTasks);
}

void cNotebook::AddTask(const tString &asName, const tWString &asText) {
	if (mpInit->mbDisablePersonalNotes)
		return;

	tNotebook_BookTaskListIt it = mlstTasks.begin();
	for (; it != mlstTasks.end(); ++it) {
		cNotebook_BookTask *pTask = *it;
		if (pTask->msName == asName) {
			Warning("Notebook task '%s' already exists\n", asName.c_str());
			return;
		}
	}

	cNotebook_BookTask *pTask = hplNew(cNotebook_BookTask, ());
	pTask->msName = asName;
	pTask->msText = asText;

	mlstTasks.push_back(pTask);

	mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_add_note", false, 1);
	mpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate("Notebook", "AddTask"), 2.2f, false);
}

void cNotebook::RemoveTask(const tString &asName) {
	tNotebook_BookTaskListIt it = mlstTasks.begin();
	for (; it != mlstTasks.end();) {
		cNotebook_BookTask *pTask = *it;
		if (pTask->msName == asName) {
			it = mlstTasks.erase(it);
			hplDelete(pTask);
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------

void cNotebook::SetBookType(eNotebookType aType) {
	if (mBookType == aType)
		return;

	mBookType = aType;
}

//-----------------------------------------------------------------------

void cNotebook::SetActive(bool abX) {
	mbActive = abX;
	if (mbActive) {
		mLastCrossHairState = mpInit->mpPlayer->GetCrossHairState();

		if (mpInit->mpInventory->IsActive()) {
			mbInventoryWasActive = true;
			mvMousePos = mpInit->mpInventory->GetMousePos();
			mpInit->mpInventory->SetActive(false);
		} else {
			mbInventoryWasActive = false;
		}
		mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);

		mBookType = eNotebookType_Front;
		mStateMachine.ChangeState(eNotebookState_Front);
		mvBookTypes[0].mfAlpha = 1;
	} else {
		if (mbInventoryWasActive) {
			mpInit->mpInventory->SetActive(true);
			mpInit->mpInventory->SetMousePos(mvMousePos);
		} else {
			mpInit->mpPlayer->SetCrossHairState(mLastCrossHairState);
			mpInit->mpPlayer->SetCrossHairPos(cVector2f(400, 300));
		}
	}
}

//-----------------------------------------------------------------------

void cNotebook::SaveToGlobal(cNotebook_GlobalSave *apSave) {
	//////////////////////
	// Tasks
	{
		tNotebook_BookTaskListIt it = mlstTasks.begin();
		for (; it != mlstTasks.end(); ++it) {
			cNotebook_BookTask *pTask = *it;
			cNotebookTask_GlobalSave saveTask;

			saveTask.msName = pTask->msName;
			saveTask.msText = pTask->msText;

			apSave->mlstTasks.Add(saveTask);
		}
	}

	//////////////////////
	// Notes
	{
		tNotebook_NoteListIt it = mlstNotes.begin();
		for (; it != mlstNotes.end(); ++it) {
			cNotebook_Note *pNote = *it;
			cNotebookNote_GlobalSave saveNote;

			saveNote.mbRead = pNote->mbRead;
			saveNote.msName = pNote->msName;
			saveNote.msTextCat = pNote->msTextCat;
			saveNote.msTextEntry = pNote->msTextEntry;

			apSave->mlstNotes.Add(saveNote);
		}
	}
}

//-----------------------------------------------------------------------

void cNotebook::LoadFromGlobal(cNotebook_GlobalSave *apSave) {
	//////////////////////
	// Tasks
	{
		cContainerListIterator<cNotebookTask_GlobalSave> it = apSave->mlstTasks.GetIterator();
		while (it.HasNext()) {
			cNotebook_BookTask *pTask = hplNew(cNotebook_BookTask, ());
			cNotebookTask_GlobalSave &saveTask = it.Next();

			pTask->msName = saveTask.msName;
			pTask->msText = saveTask.msText;

			mlstTasks.push_back(pTask);
		}
	}

	//////////////////////
	// Notes
	{
		cContainerListIterator<cNotebookNote_GlobalSave> it = apSave->mlstNotes.GetIterator();
		while (it.HasNext()) {
			cNotebook_Note *pNote = hplNew(cNotebook_Note, ());
			cNotebookNote_GlobalSave &saveNote = it.Next();

			pNote->mbRead = saveNote.mbRead;
			pNote->msName = saveNote.msName;
			pNote->msTextCat = saveNote.msTextCat;
			pNote->msTextEntry = saveNote.msTextEntry;

			pNote->msText = kTranslate(pNote->msTextCat, pNote->msTextEntry);

			mlstNotes.push_back(pNote);
		}
	}
}

//-----------------------------------------------------------------------
