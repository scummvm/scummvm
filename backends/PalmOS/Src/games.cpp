/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <PalmOS.h>
#include <VFSMgr.h>
#include <ctype.h>

#include "start.h"
#include "games.h"
#include "skin.h"

#include "extend.h"
#include "StarterRsc.h"

DmOpenRef gameDB = NULL;

static Err GamUpdateList() {
	if (gameDB) {
		UInt16 numRecs = DmNumRecords(gameDB);
		
		if (numRecs > 0) {
			MemHandle tmpH;
			UInt32 version, size;
			UInt32 *versionP;

			// get record size and version
			tmpH = DmQueryRecord(gameDB, 0);
			size = MemHandleSize(tmpH);
			versionP = (UInt32 *)MemHandleLock(tmpH);
			version = *versionP;
			MemHandleUnlock(tmpH);

			// check record
			if (version != curItemVersion && size != sizeof(GameInfoType)) {
				UInt16 index;
				GameInfoType gitCur;
				void *tmpP;
				FormPtr ofmP, frmP;
				
				// show dialog
				ofmP = FrmGetActiveForm();
				frmP = FrmInitForm(ConvertForm);
				FrmSetActiveForm(frmP);
				FrmDrawForm(frmP);
				SysTaskDelay(200);

				if (version == itemVersion_2) {
					// need conversion from V2 -> V2.5
					GameInfoTypeV2 git0;

					for (index = 0; index < numRecs; index++) {
						
						// get old data
						tmpH = DmQueryRecord(gameDB, index);
						tmpP = MemHandleLock(tmpH);
						MemMove(&git0, tmpP, sizeof(GameInfoTypeV2));
						MemHandleUnlock(tmpH);

						// convert to new format
						gitCur.version = curItemVersion;
						gitCur.icnID = 0xFFFF;
						gitCur.selected = git0.selected;
						StrCopy(gitCur.nameP, git0.nameP);
						StrCopy(gitCur.pathP, git0.pathP);
						StrCopy(gitCur.gameP, git0.gameP);
						gitCur.gfxMode = git0.gfxMode;
						
						gitCur.autoLoad = git0.autoLoad;
						gitCur.bootParam = git0.bootParam;
						gitCur.setPlatform = git0.setPlatform;
						gitCur.subtitles = git0.subtitles;
						gitCur.talkSpeed = git0.talkSpeed;

						gitCur.loadSlot = git0.loadSlot;
						gitCur.bootValue = git0.bootValue;
						gitCur.talkValue = git0.talkValue;
						gitCur.platform = git0.platform;
						gitCur.language = git0.language;
						
						tmpH = DmResizeRecord(gameDB, index, sizeof(GameInfoType));	// TODO : check error on resize tmpH==NULL
						tmpP = MemHandleLock(tmpH);
						DmWrite(tmpP, 0, &gitCur, sizeof(GameInfoType));
						MemPtrUnlock(tmpP);
					}
				} else {
					// need conversion from V0 -> V2.5
					GameInfoTypeV0 git0;

					for (index = 0; index < numRecs; index++) {
						
						// get old data
						tmpH = DmQueryRecord(gameDB, index);
						tmpP = MemHandleLock(tmpH);
						MemMove(&git0, tmpP, sizeof(GameInfoTypeV0));
						MemHandleUnlock(tmpH);

						// convert to new format
						gitCur.version = curItemVersion;
						gitCur.icnID = 0xFFFF;
						gitCur.selected = git0.selected;
						StrCopy(gitCur.nameP, git0.nameP);
						StrCopy(gitCur.pathP, git0.pathP);
						StrCopy(gitCur.gameP, git0.gameP);
						gitCur.gfxMode = git0.gfxMode;
						
						gitCur.autoLoad = git0.autoLoad;
						gitCur.bootParam = git0.bootParam;
						gitCur.setPlatform = git0.amiga;	// amiga become platform amiga/atari-st/machintosh
						gitCur.subtitles = git0.subtitles;
						gitCur.talkSpeed = git0.talkSpeed;

						gitCur.loadSlot = git0.loadSlot;
						gitCur.bootValue = git0.bootValue;
						gitCur.talkValue = git0.talkValue;
						gitCur.platform = 0;	// default to amiga
						gitCur.language = git0.language;
						
						tmpH = DmResizeRecord(gameDB, index, sizeof(GameInfoType));	// TODO : check error on resize tmpH==NULL
						tmpP = MemHandleLock(tmpH);
						DmWrite(tmpP, 0, &gitCur, sizeof(GameInfoType));
						MemPtrUnlock(tmpP);
					}
				}

				FrmEraseForm(frmP);
				FrmDeleteForm(frmP);
				if (ofmP)
					FrmReturnToForm(MainForm);
			}	
		}
	}
	
	return errNone;
}

Err GamOpenDatabase() {
	Err err = errNone;

	gameDB = DmOpenDatabaseByTypeCreator( 'DATA', appFileCreator, dmModeReadWrite);

	if (!gameDB) {
		err = DmCreateDatabase(0, "ScummVM-Data", appFileCreator, 'DATA', false);
		if (!err) {
			gameDB = DmOpenDatabaseByTypeCreator( 'DATA', appFileCreator, dmModeReadWrite);
			
			if (!gameDB)
				err = DmGetLastErr();
		}
	}

	if (err)
		FrmCustomAlert(FrmErrorAlert,"Cannot open/create games list DB !",0,0);
	else
		err = GamUpdateList();

	return err;
}

void GamImportDatabase() {
	if (gPrefs->card.volRefNum != sysInvalidRefNum && gPrefs->card.moveDB) {
		FileRef file;
		Err e;
		
		e = VFSFileOpen(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata.pdb", vfsModeRead, &file);
		if (!e) {
			UInt16 oCardNo, nCardNo;
			LocalID oDbID, nDbID;
			UInt32 type = 'ODAT';	// change the type to avoid the old db to be loaded in case of crash

			VFSFileClose(file);
			if (gPrefs->card.confirmMoveDB)
				if (FrmCustomAlert(FrmConfirmAlert, "Do you want to import games database from memory card ?", 0, 0) == FrmConfirmNo) {
					// prevent to replace the file on memory card
					gPrefs->card.moveDB = false;
					return;
				}
 			
 			// get current db info and rename it
 			DmOpenDatabaseInfo(gameDB, &oDbID, 0, 0, &oCardNo, 0);
			GamCloseDatabase(true);
			DmSetDatabaseInfo(oCardNo, oDbID, "ScummVM-Data-old.pdb", 0, 0, 0, 0, 0, 0, 0, 0, &type, 0);

	
			e = VFSImportDatabaseFromFile(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata.pdb", &nCardNo, &nDbID);
			if (e) {
				type = 'DATA';
				FrmCustomAlert(FrmErrorAlert, "Failed to import games database from memory card.", 0, 0);
				DmSetDatabaseInfo(oCardNo, oDbID, "ScummVM-Data.pdb", 0, 0, 0, 0, 0, 0, 0, 0, &type, 0);
			} else {
				// in OS5 the localID may change ... ? (cause Free Handle error) TODO : check if this is still required, crash now with tapwave !!!
//				oDbID = DmFindDatabase (oCardNo, "ScummVM-Data-old.pdb");
				e = DmDeleteDatabase(oCardNo, oDbID);
			}
			GamOpenDatabase();
		}
	}
}

void GamCloseDatabase(Boolean ignoreCardParams) {
	if (gameDB) {
		LocalID dbID;
		UInt16 cardNo;
		
		DmOpenDatabaseInfo(gameDB, &dbID, 0, 0, &cardNo, 0);
		DmCloseDatabase(gameDB);

		if (!ignoreCardParams) {
			if (gPrefs->card.moveDB && gPrefs->card.volRefNum != sysInvalidRefNum) {
				VFSFileRename(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata.pdb", "listdata-old.pdb");
				Err e = VFSExportDatabaseToFile(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata.pdb", cardNo, dbID);
				if (!e) {
					VFSFileDelete(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata-old.pdb");
					if (gPrefs->card.deleteDB)
						DmDeleteDatabase(cardNo, dbID);
				} else {
					VFSFileRename(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata-old.pdb", "listdata.pdb");
				}
			}
		}
	}
	gameDB = NULL;
}

static Int16 GamCompare(GameInfoType *a, GameInfoType *b, SortRecordInfoPtr, SortRecordInfoPtr, MemHandle) {
	return StrCaselessCompare(a->nameP, b->nameP);
}

Err GamSortList() {
	return DmQuickSort (gameDB, (DmComparF *)GamCompare, 0);
}

void GamUnselect() {
	GameInfoType *game;

	MemHandle recordH;
	UInt16 index;

	index = GamGetSelected();
	
	if (index != dmMaxRecordIndex) {
		Boolean newValue;
		
		recordH = DmGetRecord(gameDB, index);
		game = (GameInfoType *)MemHandleLock(recordH);
		
		newValue = false;
		DmWrite(game, OffsetOf(GameInfoType,selected), &newValue, sizeof(Boolean));

		MemHandleUnlock(recordH);
		DmReleaseRecord (gameDB, index, 0);
	}
}

UInt16 GamGetSelected() {
	MemHandle record;
	GameInfoType *game;
	Boolean selected;
	UInt16 index = DmNumRecords(gameDB)-1;
	
	while (index != (UInt16)-1) {
		record = DmQueryRecord(gameDB, index);
		game = (GameInfoType *)MemHandleLock(record);
		selected = game->selected;
		MemHandleUnlock(record);

		if (selected)
			return index;

		index--;
	}

	return dmMaxRecordIndex;
}

Boolean GamJumpTo(Char letter) {
	MemHandle record;
	GameInfoType *game;
	Boolean found = false;
	UInt16 index = 0;
	UInt16 maxIndex = DmNumRecords(gameDB);
	UInt16 active = GamGetSelected();

	while (index < maxIndex) {
		record = DmGetRecord(gameDB, index);
		game = (GameInfoType *)MemHandleLock(record);
		
		if (tolower(game->nameP[0]) == tolower(letter)) {
			found = true;

			if (index != active) {
				RectangleType rArea;
				UInt16 maxView;
				Boolean newValue = true;

				SknGetListBounds(&rArea, NULL);
				maxView = rArea.extent.y / sknInfoListItemSize;

				GamUnselect();
				DmWrite(game, OffsetOf(GameInfoType,selected), &newValue, sizeof(Boolean));
				
				if (index < gPrefs->listPosition || index >= (gPrefs->listPosition + maxView))
					gPrefs->listPosition = index;
			}
		}

		MemHandleUnlock(record);
		DmReleaseRecord (gameDB, index, 0);

		index++;
		
		if (found)
			return found;
	}

	return found;
}

