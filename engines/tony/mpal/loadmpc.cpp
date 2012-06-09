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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

/*
#include "lzo1x.h"
*/
#include "mpal.h"
#include "mpaldll.h"
#include "memory.h"
#include "tony/tony.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Funzioni statiche
\****************************************************************************/

static bool compareCommands(struct command *cmd1, struct command *cmd2) {
	if (cmd1->type == 2 && cmd2->type == 2) {
		if (strcmp(cmd1->lpszVarName, cmd2->lpszVarName) == 0 &&
			compareExpressions(cmd1->expr, cmd2->expr))
			return true;
		else
			return false;
	} else
		return (memcmp(cmd1, cmd2, sizeof(struct command)) == 0);
}

/**
 * Parses a script from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled script.
 * @param lpmsScript		Pointer to a structure that will be filled with the
 * data of the script.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 */
static const byte *ParseScript(const byte *lpBuf, LPMPALSCRIPT lpmsScript) {
	int curCmd, j, len;
	uint i;

	lpmsScript->nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	lpmsScript->nMoments = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	curCmd = 0;

	for (i = 0; i < lpmsScript->nMoments; i++) {
		lpmsScript->Moment[i].dwTime = (int32)READ_LE_UINT32(lpBuf);
		lpBuf += 4;
		lpmsScript->Moment[i].nCmds = *lpBuf;
		lpBuf++;

		for (j = 0; j < lpmsScript->Moment[i].nCmds; j++) {
			lpmsScript->_command[curCmd].type = *lpBuf;
			lpBuf++;
			switch (lpmsScript->_command[curCmd].type) {
			case 1:
				lpmsScript->_command[curCmd].nCf = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmsScript->_command[curCmd].arg1 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd].arg2 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd].arg3 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd].arg4 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			case 2:          // Variable assign
				len = *lpBuf;
				lpBuf++;
				lpmsScript->_command[curCmd].lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmsScript->_command[curCmd].lpszVarName == NULL)
					return NULL;
				copyMemory(lpmsScript->_command[curCmd].lpszVarName, lpBuf, len);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmsScript->_command[curCmd].expr);
				if (lpBuf == NULL)
					return NULL;
				break;

			default:
				return NULL;
			}

			lpmsScript->Moment[i].CmdNum[j] = curCmd;
			curCmd++;
		}
	}
	return lpBuf;
}

/**
 * Frees a script allocated via a previous call to ParseScript
 *
 * @param lpmsScript		Pointer to a script structure
 */
static void FreeScript(LPMPALSCRIPT lpmsScript) {
	for (int i = 0; i < MAX_COMMANDS_PER_SCRIPT && (lpmsScript->_command[i].type); ++i, ++lpmsScript) {
		if (lpmsScript->_command[i].type == 2) {
			// Variable Assign
			globalDestroy(lpmsScript->_command[i].lpszVarName);
			freeExpression(lpmsScript->_command[i].expr);
		}
	}
}

/**
 * Parses a dialog from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled dialog.
 * @param lpmdDialog		Pointer to a structure that will be filled with the
 * data of the dialog.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 */
static const byte *parseDialog(const byte *lpBuf, LPMPALDIALOG lpmdDialog) {
	uint32 i, j, z, kk;
	uint32 num, num2, num3;
	byte *lpLock;
	uint32 curCmd;
	uint32 len;

	lpmdDialog->nObj = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	/* Periodi */
	num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	
	if (num >= MAX_PERIODS_PER_DIALOG - 1)
		error("Too much periods in dialog #%d", lpmdDialog->nObj);

	for (i = 0; i < num; i++) {
		lpmdDialog->_periodNums[i] = READ_LE_UINT16(lpBuf);
		lpBuf += 2;
		lpmdDialog->_periods[i] = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, *lpBuf + 1);
		lpLock = (byte *)globalLock(lpmdDialog->_periods[i]);
		Common::copy(lpBuf + 1, lpBuf + 1 + *lpBuf, lpLock);
		globalUnlock(lpmdDialog->_periods[i]);
		lpBuf += (*lpBuf) + 1;
	}

	lpmdDialog->_periodNums[i] = 0;
	lpmdDialog->_periods[i] = NULL;

	/* Gruppi */
	num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	curCmd = 0;

	if (num >= MAX_GROUPS_PER_DIALOG)
		error("Too much groups in dialog #%d", lpmdDialog->nObj);

	for (i = 0; i < num; i++) {
		lpmdDialog->_group[i].num = READ_LE_UINT16(lpBuf);
		lpBuf += 2;
		lpmdDialog->_group[i].nCmds = *lpBuf; lpBuf++;

		if (lpmdDialog->_group[i].nCmds >= MAX_COMMANDS_PER_GROUP)
			error("Too much commands in group #%d in dialog #%d",lpmdDialog->_group[i].num,lpmdDialog->nObj);

		for (j = 0; j < lpmdDialog->_group[i].nCmds; j++) {
			lpmdDialog->_command[curCmd].type = *lpBuf;
			lpBuf++;

			switch (lpmdDialog->_command[curCmd].type) {
			// Call custom function
			case 1:
				lpmdDialog->_command[curCmd].nCf = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmdDialog->_command[curCmd].arg1 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd].arg2 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd].arg3 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd].arg4 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			// Variable assign
			case 2:
				len = *lpBuf;
				lpBuf++;
				lpmdDialog->_command[curCmd].lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmdDialog->_command[curCmd].lpszVarName == NULL)
					return NULL;

				Common::copy(lpBuf, lpBuf + len, lpmdDialog->_command[curCmd].lpszVarName);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmdDialog->_command[curCmd].expr);
				if (lpBuf == NULL)
					return NULL;
				break;

			// Do Choice
			case 3:
				lpmdDialog->_command[curCmd].nChoice = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				break;

			default:
				return NULL;
			}

			for (kk = 0;kk < curCmd; kk++) {
				if (compareCommands(&lpmdDialog->_command[kk], &lpmdDialog->_command[curCmd])) {
					lpmdDialog->_group[i].CmdNum[j] = kk;
					break;
				}
			}

			if (kk == curCmd) {	
				lpmdDialog->_group[i].CmdNum[j] = curCmd;
				curCmd++;
			}
		}
	}

	if (curCmd >= MAX_COMMANDS_PER_DIALOG)
		error("Too much commands in dialog #%d",lpmdDialog->nObj);

	/* Choices */
	num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	if (num >= MAX_CHOICES_PER_DIALOG)
		error("Too much choices in dialog #%d",lpmdDialog->nObj);

	for (i = 0; i < num; i++) {
		lpmdDialog->_choice[i].nChoice = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		num2 = *lpBuf++;

		if (num2 >= MAX_SELECTS_PER_CHOICE)
			error("Too much selects in choice #%d in dialog #%d", lpmdDialog->_choice[i].nChoice, lpmdDialog->nObj);

		for (j = 0; j < num2; j++) {
			// When
			switch (*lpBuf++) {
			case 0:
				lpmdDialog->_choice[i]._select[j].when = NULL;
				break;

			case 1:
				lpBuf = parseExpression(lpBuf, &lpmdDialog->_choice[i]._select[j].when);
				if (lpBuf == NULL)
					return NULL;
				break;

			case 2:
				return NULL;
			}

			// Attrib
			lpmdDialog->_choice[i]._select[j].attr = *lpBuf++;

			// Data
			lpmdDialog->_choice[i]._select[j].dwData = READ_LE_UINT32(lpBuf);
			lpBuf += 4;

			// PlayGroup
			num3 = *lpBuf++;

			if (num3 >= MAX_PLAYGROUPS_PER_SELECT)
				error("Too much playgroups in select #%d in choice #%d in dialog #%d", j, lpmdDialog->_choice[i].nChoice, lpmdDialog->nObj);

			for (z = 0; z < num3; z++) {
				lpmdDialog->_choice[i]._select[j].wPlayGroup[z] = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
			}

			lpmdDialog->_choice[i]._select[j].wPlayGroup[num3] = 0;
		}

		// Mark the last selection
		lpmdDialog->_choice[i]._select[num2].dwData = 0;
	}

	lpmdDialog->_choice[num].nChoice = 0;

	return lpBuf;
}


/**
 * Parses an item from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled dialog.
 * @param lpmiItem			Pointer to a structure that will be filled with the
 * data of the item.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 * @remarks		It's necessary that the structure that is passed  has been
 * completely initialised to 0 beforehand.
 */
static const byte *parseItem(const byte *lpBuf, LPMPALITEM lpmiItem) {
	byte len;
	uint32 i, j, kk;
	uint32 curCmd;

	lpmiItem->nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	len = *lpBuf;
	lpBuf++;
	copyMemory(lpmiItem->lpszDescribe, lpBuf, MIN((byte)127, len));
	lpBuf += len;

	if (len >= MAX_DESCRIBE_SIZE)
		error("Describe too long in item #%d", lpmiItem->nObj);

	lpmiItem->nActions=*lpBuf;
	lpBuf++;

	/* Alloca le azioni */
	if (lpmiItem->nActions > 0)
		lpmiItem->Action = (ItemAction *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(struct ItemAction) * (int)lpmiItem->nActions);

	curCmd = 0;

	for (i = 0; i < lpmiItem->nActions; i++) {
		lpmiItem->Action[i].num = *lpBuf;
		lpBuf++;

		lpmiItem->Action[i].wParm = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		if (lpmiItem->Action[i].num == 0xFF) {
			lpmiItem->Action[i].wTime = READ_LE_UINT16(lpBuf);
			lpBuf += 2;

			lpmiItem->Action[i].perc = *lpBuf;
			lpBuf++;
		}


		if (*lpBuf == 0) {
			lpBuf++;
			lpmiItem->Action[i].when = NULL;
		} else {
			lpBuf++;
			lpBuf = parseExpression(lpBuf,&lpmiItem->Action[i].when);
			if (lpBuf == NULL)
				return NULL;
		}

		lpmiItem->Action[i].nCmds=*lpBuf;
		lpBuf++;

		if (lpmiItem->Action[i].nCmds >= MAX_COMMANDS_PER_ACTION)
			error("Too much commands in action #%d in item #%d",lpmiItem->Action[i].num,lpmiItem->nObj);

		for (j = 0; j < lpmiItem->Action[i].nCmds; j++) {
			lpmiItem->_command[curCmd].type = *lpBuf;
			lpBuf++;
			switch (lpmiItem->_command[curCmd].type) {
			case 1:          // Call custom function
				lpmiItem->_command[curCmd].nCf  = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmiItem->_command[curCmd].arg1 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd].arg2 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd].arg3 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd].arg4 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			case 2:          // Variable assign
				len = *lpBuf;
				lpBuf++;
				lpmiItem->_command[curCmd].lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmiItem->_command[curCmd].lpszVarName == NULL)
					return NULL;
				copyMemory(lpmiItem->_command[curCmd].lpszVarName, lpBuf, len);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmiItem->_command[curCmd].expr);
				if (lpBuf == NULL)
					return NULL;
				break;

			default:
				return NULL;
			}

			for (kk = 0; kk < curCmd; kk++) {
				if (compareCommands(&lpmiItem->_command[kk], &lpmiItem->_command[curCmd])) {
					lpmiItem->Action[i].CmdNum[j] = kk;
					break;
				}
			}

			if (kk == curCmd) {	
				lpmiItem->Action[i].CmdNum[j] = curCmd;
				curCmd++;

				if (curCmd >= MAX_COMMANDS_PER_ITEM) {
					error("Too much commands in item #%d",lpmiItem->nObj);
					//curCmd=0;
				}
			}
		}
	}

	lpmiItem->dwRes = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	return lpBuf;
}

/**
 * Frees an item parsed from a prior call to ParseItem
 *
 * @param lpmiItem			Pointer to an item structure
 */
static void freeItem(LPMPALITEM lpmiItem) {
	// Free the actions
	if (lpmiItem->Action)
		globalDestroy(lpmiItem->Action);

	// Free the commands
	for (int i = 0; i < MAX_COMMANDS_PER_ITEM && (lpmiItem->_command[i].type); ++i, ++lpmiItem) {
		if (lpmiItem->_command[i].type == 2) {
			// Variable Assign
			globalDestroy(lpmiItem->_command[i].lpszVarName);
			freeExpression(lpmiItem->_command[i].expr);
		}
	}
}

/**
 * Parses a location from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled location.
 * @param lpmiLocation		Pointer to a structure that will be filled with the
 * data of the location.
 * @returns		Pointer to the buffer after the location, or NULL on failure.
 */
static const byte *ParseLocation(const byte *lpBuf, LPMPALLOCATION lpmlLocation) {
	lpmlLocation->nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;
	lpmlLocation->dwXlen = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	lpmlLocation->dwYlen = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	lpmlLocation->dwPicRes = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	return lpBuf;
}


/****************************************************************************\
*       Exported functions
\****************************************************************************/
/**
 * @defgroup Exported functions
 */

/**
 * Reads and interprets the MPC file, and create structures for various directives 
 * in the global variables
 *
 * @param lpBuf				Buffer containing the MPC file data, excluding the header.
 * @returns		True if succeeded OK, false if failure.
 */
bool ParseMpc(const byte *lpBuf) {
	uint16 i, j;
	uint16 wLen;
	byte *lpTemp, *lpTemp2;

	/* 1. Variables */
	if (lpBuf[0] != 'V' || lpBuf[1] != 'A' || lpBuf[2] != 'R' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS.nVars = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	GLOBALS.hVars = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(MPALVAR) * (uint32)GLOBALS.nVars);
	if (GLOBALS.hVars == NULL)
		return false;

	GLOBALS.lpmvVars = (LPMPALVAR)globalLock(GLOBALS.hVars);

	for (i = 0; i < GLOBALS.nVars; i++) {
		wLen = *(const byte *)lpBuf;
		lpBuf++;
		copyMemory(GLOBALS.lpmvVars->lpszVarName, lpBuf, MIN(wLen, (uint16)32));
		lpBuf += wLen;
		GLOBALS.lpmvVars->dwVal = READ_LE_UINT32(lpBuf);
		lpBuf += 4;

		lpBuf++;             // Salta 'ext'
		GLOBALS.lpmvVars++;
	}

	globalUnlock(GLOBALS.hVars);

	/* 2. Messages */
	if (lpBuf[0] != 'M' || lpBuf[1] != 'S' || lpBuf[2] != 'G' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS.nMsgs = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

#ifdef NEED_LOCK_MSGS
	GLOBALS.hMsgs = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(MPALMSG) * (uint32)GLOBALS.nMsgs);
	if (GLOBALS.hMsgs == NULL)
		return false;

	GLOBALS.lpmmMsgs = (LPMPALMSG)globalLock(GLOBALS.hMsgs);
#else
	GLOBALS.lpmmMsgs=(LPMPALMSG)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,sizeof(MPALMSG)*(uint32)GLOBALS.nMsgs);
	if (GLOBALS.lpmmMsgs==NULL)
		return false;
#endif

	for (i = 0; i < GLOBALS.nMsgs; i++) {
		GLOBALS.lpmmMsgs->wNum = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		for (j = 0; lpBuf[j] != 0;)
			j += lpBuf[j] + 1;

		GLOBALS.lpmmMsgs->hText = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, j + 1);
		lpTemp2 = lpTemp = (byte *)globalLock(GLOBALS.lpmmMsgs->hText);

		for (j = 0; lpBuf[j] != 0;) {
			copyMemory(lpTemp, &lpBuf[j + 1], lpBuf[j]);
			lpTemp += lpBuf[j];
			*lpTemp ++= '\0';
			j += lpBuf[j] + 1;
		}

		lpBuf += j + 1;
		*lpTemp = '\0';

		globalUnlock(GLOBALS.lpmmMsgs->hText);
		GLOBALS.lpmmMsgs++;
	}

#ifdef NEED_LOCK_MSGS
	globalUnlock(GLOBALS.hMsgs);
#endif

	/* 3. Objects */
	if (lpBuf[0] != 'O' || lpBuf[1] != 'B' || lpBuf[2] != 'J' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS.nObjs = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	// Check out the dialogs
	GLOBALS.nDialogs = 0;
	GLOBALS.hDialogs = GLOBALS.lpmdDialogs = NULL;
	if (*((const byte *)lpBuf + 2) == 6 && strncmp((const char *)lpBuf + 3, "Dialog", 6) == 0) {
		GLOBALS.nDialogs = READ_LE_UINT16(lpBuf); lpBuf += 2;

		GLOBALS.hDialogs = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS.nDialogs * sizeof(MPALDIALOG));
		if (GLOBALS.hDialogs == NULL)
			return false;

		GLOBALS.lpmdDialogs = (LPMPALDIALOG)globalLock(GLOBALS.hDialogs);

		for (i = 0;i < GLOBALS.nDialogs; i++)
			if ((lpBuf = parseDialog(lpBuf + 7, &GLOBALS.lpmdDialogs[i])) == NULL)
				return false;

		globalUnlock(GLOBALS.hDialogs);
	}

	// Check the items
	GLOBALS.nItems = 0;
	GLOBALS.hItems = GLOBALS.lpmiItems = NULL;
	if (*(lpBuf + 2) == 4 && strncmp((const char *)lpBuf + 3, "Item", 4)==0) {
		GLOBALS.nItems = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory and read them in
		GLOBALS.hItems = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS.nItems * sizeof(MPALITEM));
		if (GLOBALS.hItems == NULL)
			return false;

		GLOBALS.lpmiItems = (LPMPALITEM)globalLock(GLOBALS.hItems);

		for (i = 0; i < GLOBALS.nItems; i++)
			if ((lpBuf = parseItem(lpBuf + 5, &GLOBALS.lpmiItems[i])) == NULL)
				return false;

		globalUnlock(GLOBALS.hItems);
	}

	// Check the locations
	GLOBALS.nLocations = 0;
	GLOBALS.hLocations = GLOBALS.lpmlLocations = NULL;
	if (*(lpBuf + 2) == 8 && strncmp((const char *)lpBuf + 3, "Location", 8) == 0) {
		GLOBALS.nLocations = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory and read them in
		GLOBALS.hLocations = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS.nLocations*sizeof(MPALLOCATION));
		if (GLOBALS.hLocations == NULL)
			return false;

		GLOBALS.lpmlLocations = (LPMPALLOCATION)globalLock(GLOBALS.hLocations);

		for (i = 0; i < GLOBALS.nLocations; i++)
			if ((lpBuf = ParseLocation(lpBuf + 9, &GLOBALS.lpmlLocations[i])) == NULL)
				return false;

		globalUnlock(GLOBALS.hLocations);
	}

	// Check the scripts
	GLOBALS.nScripts = 0;
	GLOBALS.hScripts = GLOBALS.lpmsScripts = NULL;
	if (*(lpBuf + 2) == 6 && strncmp((const char *)lpBuf + 3, "Script", 6) == 0) {
		GLOBALS.nScripts = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory
		GLOBALS.hScripts = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS.nScripts * sizeof(MPALSCRIPT));
		if (GLOBALS.hScripts == NULL)
			return false;

		GLOBALS.lpmsScripts = (LPMPALSCRIPT)globalLock(GLOBALS.hScripts);

		for (i = 0; i < GLOBALS.nScripts; i++) {
			if ((lpBuf = ParseScript(lpBuf + 7, &GLOBALS.lpmsScripts[i])) == NULL)
			return false;

			// Sort the various moments of the script
			//qsort(
			//GLOBALS.lpmsScripts[i].Moment,
			//GLOBALS.lpmsScripts[i].nMoments,
			//sizeof(GLOBALS.lpmsScripts[i].Moment[0]),
			//(int (*)(const void *, const void *))CompareMoments
			//);
		}

		globalUnlock(GLOBALS.hScripts);
	}

	if (lpBuf[0] != 'E' || lpBuf[1] != 'N' || lpBuf[2] != 'D' || lpBuf[3] != '0')
		return false;

	return true;
}

/**
 * Free the given dialog
 */
static void freeDialog(LPMPALDIALOG lpmdDialog) {
	// Free the periods
	int i, j;

	for (i = 0; i < MAX_PERIODS_PER_DIALOG && (lpmdDialog->_periods[i]); ++i)
		globalFree(lpmdDialog->_periods[i]);

	for (i = 0; i < MAX_COMMANDS_PER_GROUP && (lpmdDialog->_command[i].type); i++) {
		if (lpmdDialog->_command[i].type == 2) {
			// Variable assign
			globalDestroy(lpmdDialog->_command[i].lpszVarName);
			freeExpression(lpmdDialog->_command[i].expr);
		}
	}

	// Free the choices
	for (i = 0; i < MAX_CHOICES_PER_DIALOG; ++i) {
		for (j = 0; j < MAX_SELECTS_PER_CHOICE; j++) {
			if (lpmdDialog->_choice[i]._select[j].when)
				freeExpression(lpmdDialog->_choice[i]._select[j].when);
		}
	}
}

/**
 * Frees any data allocated from the parsing of the MPC file
 */
void FreeMpc() {
	int i;

	// Free variables
	globalFree(GLOBALS.hVars);

	// Free messages
	LPMPALMSG lpmmMsgs = (LPMPALMSG)globalLock(GLOBALS.hMsgs);
	for (i = 0; i < GLOBALS.nMsgs; i++, ++lpmmMsgs)
		globalFree(lpmmMsgs->hText);

	globalUnlock(GLOBALS.hMsgs);
	globalFree(GLOBALS.hMsgs);

	// Free objects
	if (GLOBALS.hDialogs) {
		LPMPALDIALOG lpmdDialogs = (LPMPALDIALOG)globalLock(GLOBALS.hDialogs);

		for (i = 0; i < GLOBALS.nDialogs; i++, ++lpmdDialogs)
			freeDialog(lpmdDialogs);

		globalFree(GLOBALS.hDialogs);
	}

	// Free items
	if (GLOBALS.hItems) {
		LPMPALITEM lpmiItems = (LPMPALITEM)globalLock(GLOBALS.hItems);

		for (i = 0; i < GLOBALS.nItems; ++i, ++lpmiItems)
			freeItem(lpmiItems);

		globalUnlock(GLOBALS.hItems);
		globalFree(GLOBALS.hItems);
	}

	// Free the locations
	if (GLOBALS.hLocations) {
		globalFree(GLOBALS.hLocations);
	}

	// Free the scripts
	if (GLOBALS.hScripts) {
		LPMPALSCRIPT lpmsScripts = (LPMPALSCRIPT)globalLock(GLOBALS.hScripts);

		for (i = 0; i < GLOBALS.nScripts; ++i, ++lpmsScripts) {
			FreeScript(lpmsScripts);
		}

		globalUnlock(GLOBALS.hScripts);
	}
}

} // end of namespace MPAL

} // end of namespace Tony
