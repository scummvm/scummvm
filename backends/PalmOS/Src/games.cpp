/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include <PalmOS.h>
#include <VFSMgr.h>
#include <ctype.h>

#include "globals.h"
#include "palmdefs.h"
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
				SysTaskDelay(1 * SysTicksPerSecond());

				MemSet(&gitCur, sizeof(GameInfoType), 0);

				if (version == itemVersion_33 ||
					version == itemVersion_32 ||
					version == itemVersion_31 ||
					version == itemVersion_30 ||
					version == itemVersion_27 ||
					version == itemVersion_26 ||
					version == itemVersion_25) {

					for (index = 0; index < numRecs; index++) {
						// get old data
						tmpH = DmQueryRecord(gameDB, index);
						tmpP = MemHandleLock(tmpH);
						MemMove(&gitCur, tmpP, MemHandleSize(tmpH));
						MemHandleUnlock(tmpH);
						
						if (version < itemVersion_30) {
							gitCur.musicInfo.volume.palm = 50;
							gitCur.musicInfo.volume.music = 192;
							gitCur.musicInfo.volume.sfx = 192;
							gitCur.musicInfo.volume.speech = 192;
							gitCur.musicInfo.volume.audiocd = 50;

							gitCur.musicInfo.sound.tempo = 100;
							gitCur.musicInfo.sound.defaultTrackLength = 10;
							gitCur.musicInfo.sound.firstTrack = 1;
						}

						if (version < itemVersion_31)
							gitCur.engine = 0;
						
						if (version < itemVersion_32)
							gitCur.renderMode = 0;

						gitCur.renderMode = 0;
						gitCur.fmQuality = 0;
						gitCur.gfxMode = (gitCur.gfxMode == 3 ? 1 : 0); // v3.4 only 2 modes

						if (version <= itemVersion_33) {
									if (gitCur.engine == 0) gitCur.engine = 8;
							else	if (gitCur.engine == 1) gitCur.engine = 7;
							else	if (gitCur.engine == 2) gitCur.engine = 3;
							else	if (gitCur.engine == 3) gitCur.engine = 1;
							else	if (gitCur.engine == 4) gitCur.engine = 0;
							else	if (gitCur.engine == 5) gitCur.engine = 6;
							else	if (gitCur.engine == 6) gitCur.engine = 4;
							else	if (gitCur.engine == 7) gitCur.engine = 5;
							else	if (gitCur.engine == 8) gitCur.engine = 2;
							
									if (gitCur.renderMode == 1) gitCur.renderMode = 4;
							else	if (gitCur.renderMode == 2) gitCur.renderMode = 5;
							else	if (gitCur.renderMode == 3) gitCur.renderMode = 2;
							else	if (gitCur.renderMode == 4) gitCur.renderMode = 3;
							else	if (gitCur.renderMode == 5) gitCur.renderMode = 1;

									if (gitCur.platform == 2) gitCur.platform = 6;
							else	if (gitCur.platform == 5) gitCur.platform = 8;
							else	if (gitCur.platform == 6) gitCur.platform = 2;

									if (gitCur.language ==  1) gitCur.language = 4;
							else	if (gitCur.language ==  2) gitCur.language = 6;
							else	if (gitCur.language ==  3) gitCur.language = 5;
							else	if (gitCur.language ==  4) gitCur.language = 8;
							else	if (gitCur.language ==  5) gitCur.language = 11;
							else	if (gitCur.language ==  6) gitCur.language = 13;
							else	if (gitCur.language ==  7) gitCur.language = 9;
							else	if (gitCur.language ==  8) gitCur.language = 1;
							else	if (gitCur.language ==  9) gitCur.language = 10;
							else	if (gitCur.language == 10) gitCur.language = 7;
							else	if (gitCur.language == 11) gitCur.language = 12;
							else	if (gitCur.language == 12) gitCur.language = 2;
							else	if (gitCur.language == 13) gitCur.language = 3;

									if (gitCur.musicInfo.sound.drvMusic == 1) gitCur.musicInfo.sound.drvMusic = 4;
							else	if (gitCur.musicInfo.sound.drvMusic == 2) gitCur.musicInfo.sound.drvMusic = 5;
							else	if (gitCur.musicInfo.sound.drvMusic == 4) gitCur.musicInfo.sound.drvMusic = 2;
							else	if (gitCur.musicInfo.sound.drvMusic == 5) gitCur.musicInfo.sound.drvMusic = 1;
						}
						
						if (gitCur.musicInfo.volume.palm > 100)
							gitCur.musicInfo.volume.palm = 50;

						// simply resize the old record
						tmpH = DmResizeRecord(gameDB, index, sizeof(GameInfoType));	// TODO : check error on resize tmpH==NULL
						tmpP = MemHandleLock(tmpH);
						DmWrite(tmpP, 0, &gitCur, sizeof(GameInfoType));
						MemPtrUnlock(tmpP);
					}

				} else if (version == itemVersion_20) {
					// need conversion from V2 -> V3.4
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
						gitCur.gfxMode = (git0.gfxMode == 3 ? 1 : 0); // v3.4 only 2 modes

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

						gitCur.musicInfo.volume.palm = 50;
						gitCur.musicInfo.volume.music = 192;
						gitCur.musicInfo.volume.sfx = 192;
						gitCur.musicInfo.volume.speech = 192;
						gitCur.musicInfo.volume.audiocd = 50;

						gitCur.musicInfo.sound.tempo = 100;
						gitCur.musicInfo.sound.defaultTrackLength = 10;
						gitCur.musicInfo.sound.firstTrack = 1;

						// to V3.4
								if (gitCur.platform == 2) gitCur.platform = 6;
						else	if (gitCur.platform == 5) gitCur.platform = 8;
						else	if (gitCur.platform == 6) gitCur.platform = 2;

								if (gitCur.language ==  1) gitCur.language = 4;
						else	if (gitCur.language ==  2) gitCur.language = 6;
						else	if (gitCur.language ==  3) gitCur.language = 5;
						else	if (gitCur.language ==  4) gitCur.language = 8;
						else	if (gitCur.language ==  5) gitCur.language = 11;
						else	if (gitCur.language ==  6) gitCur.language = 13;
						else	if (gitCur.language ==  7) gitCur.language = 9;
						else	if (gitCur.language ==  8) gitCur.language = 1;
						else	if (gitCur.language ==  9) gitCur.language = 10;
						else	if (gitCur.language == 10) gitCur.language = 7;
						else	if (gitCur.language == 11) gitCur.language = 12;
						else	if (gitCur.language == 12) gitCur.language = 2;
						else	if (gitCur.language == 13) gitCur.language = 3;

								if (gitCur.musicInfo.sound.drvMusic == 1) gitCur.musicInfo.sound.drvMusic = 4;
						else	if (gitCur.musicInfo.sound.drvMusic == 2) gitCur.musicInfo.sound.drvMusic = 5;
						else	if (gitCur.musicInfo.sound.drvMusic == 4) gitCur.musicInfo.sound.drvMusic = 2;
						else	if (gitCur.musicInfo.sound.drvMusic == 5) gitCur.musicInfo.sound.drvMusic = 1;

						gitCur.engine = ENGINE_SCUMM;

						tmpH = DmResizeRecord(gameDB, index, sizeof(GameInfoType));	// TODO : check error on resize tmpH==NULL
						tmpP = MemHandleLock(tmpH);
						DmWrite(tmpP, 0, &gitCur, sizeof(GameInfoType));
						MemPtrUnlock(tmpP);
					}
				} else {
					// need conversion from V0 -> V3.4
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
						gitCur.gfxMode = (git0.gfxMode == 3 ? 1 : 0); // v3.4 only 2 modes

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

						gitCur.musicInfo.volume.palm = 50;
						gitCur.musicInfo.volume.music = 192;
						gitCur.musicInfo.volume.sfx = 192;
						gitCur.musicInfo.volume.speech = 192;
						gitCur.musicInfo.volume.audiocd = 50;

						gitCur.musicInfo.sound.tempo = 100;
						gitCur.musicInfo.sound.defaultTrackLength = 10;
						gitCur.musicInfo.sound.firstTrack = 1;

						gitCur.engine = ENGINE_SCUMM;

						tmpH = DmResizeRecord(gameDB, index, sizeof(GameInfoType));	// TODO : check error on resize tmpH==NULL
						tmpP = MemHandleLock(tmpH);
						DmWrite(tmpP, 0, &gitCur, sizeof(GameInfoType));
						MemPtrUnlock(tmpP);
					}
				}

				FrmEraseForm(frmP);
				FrmDeleteForm(frmP);
				if (ofmP)
					FrmSetActiveForm(ofmP);
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
			e = DmDeleteDatabase(oCardNo, oDbID);

			if (!e)
				if (e = VFSImportDatabaseFromFile(gPrefs->card.volRefNum, "/Palm/Programs/ScummVM/listdata.pdb", &nCardNo, &nDbID))
					FrmCustomAlert(FrmErrorAlert, "Failed to import games database from memory card.", 0, 0);

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
