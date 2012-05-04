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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                        ...                  Spyral Software snc        *
 *        .             x#""*$Nu       -= We create much MORE than ALL =- *
 *      d*#R$.          R     ^#$o     様様様様様様様様様様様様様様様様様 *
 *    .F    ^$k         $        "$b                                      *
 *   ."       $b      u "$         #$L                                    *
 *   P         $c    :*$L"$L        '$k  Project: MPAL................... *
 *  d    @$N.   $.   d ^$b^$k         $c                                  *
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  MPC Loader............. *
 * 4    4k   *N  #b .>    '$N'*$u      *                                  *
 * M     $L   #$  $ 8       "$c'#$b.. .@ Author:  Giovanni Bajo.......... *
 * M     '$u   "$u :"         *$. "#*#"                                   *
 * M      '$N.  "  F           ^$k       Desc:    Legge un file compilato *
 * 4>       ^R$oue#             d                 di MPAL................ *
 * '$          ""              @                  ....................... *
 *  #b                       u#                                           *
 *   $b                    .@"           OS: [ ] DOS  [X] WIN95  [ ] OS/2 *
 *    #$u                .d"                                              *
 *     '*$e.          .zR".@           様様様様様様様様様様様様様様様様様 *
 *        "*$$beooee$*"  @"M                  This source code is         *
 *             """      '$.?              Copyright (C) Spyral Software   *
 *                       '$d>                 ALL RIGHTS RESERVED         *
 *                        '$>          様様様様様様様様様様様様様様様様様 *
 *                                                                        *
 **************************************************************************/

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

static bool CompareCommands(struct command *cmd1, struct command *cmd2) {
	if (cmd1->type == 2 && cmd2->type == 2) {
		if (strcmp(cmd1->lpszVarName,cmd2->lpszVarName)==0 &&
			CompareExpressions(cmd1->expr,cmd2->expr))
			return true;
		else
			return false;
	} else
		return (memcmp(cmd1,cmd2,sizeof(struct command))==0);
}


/****************************************************************************\
*
* Function:     LPBTYE ParseScript(byte *lpBuf, LPMPALSCRIPT lpmsScript);
*
* Description:  Esegue il parsing da file .MPC di uno script e inserisce il
*               tutto dentro una struttura
*
* Input:        byte *lpBuf            Buffer contenente lo script compilato
*               LPMPALSCRIPT lpmsScript Puntatore a una struttura che verra'
*                                       riempita con i dati dello script
*                                       lato
*
* Return:       Puntatore al buffer dopo l'item, o NULL in caso di errore
*
\****************************************************************************/

static byte *ParseScript(byte *lpBuf, LPMPALSCRIPT lpmsScript) {
	int curCmd,j,len;
	uint i;

	lpmsScript->nObj=*(int *)lpBuf;
	lpBuf+=4;

	lpmsScript->nMoments=*(uint16 *)lpBuf;
	lpBuf+=2;

	curCmd=0;

	for (i=0;i<lpmsScript->nMoments;i++) {
		lpmsScript->Moment[i].dwTime=*(int *)lpBuf; lpBuf+=4;
		lpmsScript->Moment[i].nCmds=*lpBuf;       lpBuf++;

		for (j=0;j<lpmsScript->Moment[i].nCmds;j++) {
			lpmsScript->Command[curCmd].type=*lpBuf; lpBuf++;
			switch (lpmsScript->Command[curCmd].type) {
			case 1:
				lpmsScript->Command[curCmd].nCf =*(uint16 *)(lpBuf); lpBuf+=2;
				lpmsScript->Command[curCmd].arg1=*(int *)(lpBuf); lpBuf+=4;
				lpmsScript->Command[curCmd].arg2=*(int *)(lpBuf); lpBuf+=4;
				lpmsScript->Command[curCmd].arg3=*(int *)(lpBuf); lpBuf+=4;
				lpmsScript->Command[curCmd].arg4=*(int *)(lpBuf); lpBuf+=4;
				break;

			case 2:          // Variable assign
				len=*lpBuf; lpBuf++;
				lpmsScript->Command[curCmd].lpszVarName=(char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,len+1);
				if (lpmsScript->Command[curCmd].lpszVarName==NULL)
					return NULL;
				CopyMemory(lpmsScript->Command[curCmd].lpszVarName, lpBuf, len);
				lpBuf+=len;

				lpBuf=ParseExpression(lpBuf,&lpmsScript->Command[curCmd].expr);
				if (lpBuf==NULL)
				return NULL;
				break;

			default:
				return NULL;
			}

			lpmsScript->Moment[i].CmdNum[j]=curCmd;
			curCmd++;
		}
	}

	return lpBuf;
}


/****************************************************************************\
*
* Function:     byte *ParseDialog(byte *lpBuf, LPMPALDIALOG lpmdDialog);
*
* Description:  Esegue il parsing da file .MPC di un dialog, e inserisce il
*               tutto dentro una struttura
*
* Input:        byte *lpBuf            Buffer contenente il dialogo compi-
*                                       lato
*               LPMPALDIALOG lpmdDialog Puntatore a una struttura che verra'
*                                       riempita con i dati del dialogo
*                                       compilato
*
* Return:       Puntatore al buffer dopo il dialogo, o NULL in caso di errore
*
\****************************************************************************/

static byte *ParseDialog(byte *lpBuf, LPMPALDIALOG lpmdDialog) {
	uint32 i,j,z,kk;
	uint32 num,num2,num3;
	byte *lpLock;
	uint32 curCmd;
	uint32 len;

	lpmdDialog->nObj=*(int *)lpBuf;
	lpBuf+=4;

	/* Periodi */
	num=*(uint16 *)lpBuf; lpBuf+=2;
	
	if (num >= MAX_PERIODS_PER_DIALOG-1) {
		Common::String msg = Common::String::format("Too much periods in dialog #%d",lpmdDialog->nObj);
		MessageBox(msg);
	}

	for (i=0;i<num;i++) {
		lpmdDialog->PeriodNums[i]=*(uint16 *)lpBuf; lpBuf+=2;
		lpmdDialog->Periods[i]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,*lpBuf+1);
		lpLock = (byte *)GlobalLock(lpmdDialog->Periods[i]);
		CopyMemory(lpLock,lpBuf+1,*lpBuf);
		GlobalUnlock(lpmdDialog->Periods[i]);
		lpBuf+=(*lpBuf)+1;
	}

	lpmdDialog->PeriodNums[i]=0;
	lpmdDialog->Periods[i]=NULL;

	/* Gruppi */
	num=*(uint16 *)lpBuf; lpBuf+=2;
	curCmd=0;

	if (num >= MAX_GROUPS_PER_DIALOG) {
		Common::String msg = Common::String::format("Too much groups in dialog #%d",lpmdDialog->nObj);
		MessageBox(msg);
	}

	for (i=0;i<num;i++) {
		lpmdDialog->Group[i].num=*(uint16 *)lpBuf; lpBuf+=2;
		lpmdDialog->Group[i].nCmds=*lpBuf; lpBuf++;

		if (lpmdDialog->Group[i].nCmds >= MAX_COMMANDS_PER_GROUP) {
			Common::String msg = Common::String::format("Too much commands in group #%d in dialog #%d",lpmdDialog->Group[i].num,lpmdDialog->nObj);
			MessageBox(msg);
		}

		for (j=0;j<lpmdDialog->Group[i].nCmds;j++) {
			lpmdDialog->Command[curCmd].type=*lpBuf;
			lpBuf++;

			switch (lpmdDialog->Command[curCmd].type) {
			// Call custom function
			case 1:
				lpmdDialog->Command[curCmd].nCf =*(uint16 *)(lpBuf); lpBuf+=2;
				lpmdDialog->Command[curCmd].arg1=*(int *)(lpBuf); lpBuf+=4;
				lpmdDialog->Command[curCmd].arg2=*(int *)(lpBuf); lpBuf+=4;
				lpmdDialog->Command[curCmd].arg3=*(int *)(lpBuf); lpBuf+=4;
				lpmdDialog->Command[curCmd].arg4=*(int *)(lpBuf); lpBuf+=4;
				break;

			// Variable assign
			case 2:
				len=*lpBuf;
				lpBuf++;
				lpmdDialog->Command[curCmd].lpszVarName=(char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,len+1);
				if (lpmdDialog->Command[curCmd].lpszVarName==NULL)
					return NULL;
				CopyMemory(lpmdDialog->Command[curCmd].lpszVarName,lpBuf,len);
				lpBuf+=len;

				lpBuf=ParseExpression(lpBuf,&lpmdDialog->Command[curCmd].expr);
				if (lpBuf==NULL)
					return NULL;
				break;

			// Do Choice
			case 3:
				lpmdDialog->Command[curCmd].nChoice=*(uint16 *)lpBuf; lpBuf+=2;
				break;

			default:
				return NULL;
			}

			for (kk=0;kk<curCmd;kk++) {
				if (CompareCommands(&lpmdDialog->Command[kk],&lpmdDialog->Command[curCmd])) {
					lpmdDialog->Group[i].CmdNum[j]=kk;
					break;
				}
			}

			if (kk==curCmd) {	
				lpmdDialog->Group[i].CmdNum[j]=curCmd;
				curCmd++;
			}
		}
	}

	if (curCmd >= MAX_COMMANDS_PER_DIALOG) {
		Common::String msg = Common::String::format("Too much commands in dialog #%d",lpmdDialog->nObj);
		MessageBox(msg);
	}

	/* Choices */
	num=*(uint16 *)lpBuf; lpBuf+=2;

	if (num >= MAX_CHOICES_PER_DIALOG) {
		Common::String msg = Common::String::format("Too much choices in dialog #%d",lpmdDialog->nObj);
		MessageBox(msg);
	}

	for (i=0;i<num;i++) {
		lpmdDialog->Choice[i].nChoice=*(uint16 *)lpBuf; lpBuf+=2;

		num2=*lpBuf++;

		if (num2 >= MAX_SELECTS_PER_CHOICE) {
			Common::String msg = Common::String::format("Too much selects in choice #%d in dialog #%d",lpmdDialog->Choice[i].nChoice,lpmdDialog->nObj);
			MessageBox(msg);
		}

		for (j=0;j<num2;j++) {
			// When
			switch (*lpBuf++) {
			case 0:
				lpmdDialog->Choice[i].Select[j].when=NULL;
				break;

			case 1:
				lpBuf=ParseExpression(lpBuf,&lpmdDialog->Choice[i].Select[j].when);
				if (lpBuf==NULL)
					return NULL;
				break;

			case 2:
				return NULL;
			}

			// Attrib
			lpmdDialog->Choice[i].Select[j].attr=*lpBuf++;

			// Data
			lpmdDialog->Choice[i].Select[j].dwData=*(uint32 *)lpBuf; lpBuf+=4;

			// PlayGroup
			num3=*lpBuf; *lpBuf++;

  			if (num3 >= MAX_PLAYGROUPS_PER_SELECT) {
				Common::String msg = Common::String::format("Too much playgroups in select #%d in choice #%d in dialog #%d",j,lpmdDialog->Choice[i].nChoice,lpmdDialog->nObj);
				MessageBox(msg);
			}

			for (z=0;z<num3;z++) {
				lpmdDialog->Choice[i].Select[j].wPlayGroup[z]=*(uint16 *)lpBuf; lpBuf+=2;
			}

			lpmdDialog->Choice[i].Select[j].wPlayGroup[num3]=0;
		}

		// Segna l'ultimo select
		lpmdDialog->Choice[i].Select[num2].dwData=0;
	}

	lpmdDialog->Choice[num].nChoice=0;

	return lpBuf;
}

/****************************************************************************\
*
* Function:     byte *ParseItem(byte *lpBuf, LPMPALITEM lpmiItem);
*
* Description:  Esegue il parsing da file .MPC di un item, e inserisce il
*               tutto dentro una struttura
*
* Input:        byte *lpBuf            Buffer contenete l'item compilato
*               LPMPALITEM lpmiItem     Puntatore a una struttura che verra'
*                                       riempita con i dati dell'item
*                                       compilato
*
* Return:       Puntatore al buffer dopo l'item, o NULL in caso di errore
*
* Note:         E' necessario che la struttura passata come parametro sia
*               stata completamente inizializzata a 0 (con una ZeroMemory,
*               ad esempio).
*
\****************************************************************************/

static byte *ParseItem(byte *lpBuf, LPMPALITEM lpmiItem) {
	byte len;
	uint32 i,j,kk;
	uint32 curCmd;

	lpmiItem->nObj=*(int *)lpBuf;
	lpBuf+=4;

	len=*lpBuf;
	lpBuf++;
	CopyMemory(lpmiItem->lpszDescribe,lpBuf, MIN((byte)127, len));
	lpBuf+=len;

	if (len >= MAX_DESCRIBE_SIZE) {
		Common::String msg = Common::String::format("Describe too long in item #%d",lpmiItem->nObj);
		MessageBox(msg);
	}

	lpmiItem->nActions=*lpBuf;
	lpBuf++;

	/* Alloca le azioni */
	if (lpmiItem->nActions>0)
		lpmiItem->Action = (ItemAction *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,sizeof(struct ItemAction)*(int)lpmiItem->nActions);

	curCmd=0;

	for (i=0;i<lpmiItem->nActions;i++) {
		lpmiItem->Action[i].num=*lpBuf;
		lpBuf++;

		lpmiItem->Action[i].wParm=*(uint16 *)lpBuf;
		lpBuf+=2;

		if (lpmiItem->Action[i].num==0xFF) {
			lpmiItem->Action[i].wTime=*(uint16 *)lpBuf;
			lpBuf+=2;

			lpmiItem->Action[i].perc=*lpBuf;
			lpBuf++;
		}


		if (*lpBuf==0) {
			lpBuf++;
			lpmiItem->Action[i].when=NULL;
		} else {
			lpBuf++;
			lpBuf=ParseExpression(lpBuf,&lpmiItem->Action[i].when);
			if (lpBuf==NULL)
				return NULL;
		}

		lpmiItem->Action[i].nCmds=*lpBuf;
		lpBuf++;

		if (lpmiItem->Action[i].nCmds >= MAX_COMMANDS_PER_ACTION) {
			Common::String msg = Common::String::format("Too much commands in action #%d in item #%d",lpmiItem->Action[i].num,lpmiItem->nObj);
			MessageBox(msg);
		}

		for (j=0;j<lpmiItem->Action[i].nCmds;j++) {
			lpmiItem->Command[curCmd].type=*lpBuf;
			lpBuf++;
			switch (lpmiItem->Command[curCmd].type) {
			case 1:          // Call custom function
				lpmiItem->Command[curCmd].nCf =*(uint16 *)(lpBuf); lpBuf+=2;
				lpmiItem->Command[curCmd].arg1=*(int *)(lpBuf); lpBuf+=4;
				lpmiItem->Command[curCmd].arg2=*(int *)(lpBuf); lpBuf+=4;
				lpmiItem->Command[curCmd].arg3=*(int *)(lpBuf); lpBuf+=4;
				lpmiItem->Command[curCmd].arg4=*(int *)(lpBuf); lpBuf+=4;
				break;

			case 2:          // Variable assign
				len=*lpBuf;
				lpBuf++;
				lpmiItem->Command[curCmd].lpszVarName=(char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,len+1);
				if (lpmiItem->Command[curCmd].lpszVarName==NULL)
					return NULL;
				CopyMemory(lpmiItem->Command[curCmd].lpszVarName,lpBuf,len);
				lpBuf+=len;

				lpBuf=ParseExpression(lpBuf,&lpmiItem->Command[curCmd].expr);
				if (lpBuf==NULL)
					return NULL;
				break;

			default:
				return NULL;
			}

			for (kk=0;kk<curCmd;kk++) {
				if (CompareCommands(&lpmiItem->Command[kk],&lpmiItem->Command[curCmd])) {
					lpmiItem->Action[i].CmdNum[j]=kk;
					break;
				}
			}

			if (kk==curCmd) {	
				lpmiItem->Action[i].CmdNum[j]=curCmd;
				curCmd++;

				if (curCmd >= MAX_COMMANDS_PER_ITEM) {
					Common::String msg = Common::String::format("Too much commands in item #%d",lpmiItem->nObj);
					MessageBox(msg);
					curCmd=0;
				}
			}
		}
	}

	lpmiItem->dwRes=*(uint32 *)lpBuf; lpBuf+=4;

	return lpBuf;
}


/****************************************************************************\
*
* Function:     byte *ParseLocation(byte *buf, LPMPALLOCATIONN lpmlLocation)
*
* Description:  Esegue il parsing da file .MPC di una locazione, riempendo
*               una struttura
*
* Input:        byte *buf              Buffer contenente la locazione
*                                       compilata
*               LPMPALLOCATION
*                lpmlLocation           Pointer alla struttura che verra'
*                                       riempita con i dati sulla locazione
*
* Return:       Puntatore al buffer dopo l'item, o NULL in caso di errore
*
\****************************************************************************/

static byte *ParseLocation(byte *lpBuf, LPMPALLOCATION lpmlLocation) {
	lpmlLocation->nObj=*(int *)lpBuf;
	lpBuf+=4;
	lpmlLocation->dwXlen=*(uint16 *)lpBuf;
	lpBuf+=2;
	lpmlLocation->dwYlen=*(uint16 *)lpBuf;
	lpBuf+=2;
	 lpmlLocation->dwPicRes=*(uint32 *)lpBuf;
	lpBuf+=4;

	return lpBuf;
}

/*static int CompareMoments(int * a, int * b) {
	if (*a<*b)
		return -1;
	else if (*a>*b)
		return 1;
	else
		return 0;
}*/

/****************************************************************************\
*       Funzioni globali
\****************************************************************************/

/****************************************************************************\
*
* Function:     bool ParseMpc(byte *lpBuf);
*
* Description:  Legge e interpreta un file MPC, e crea le strutture per le
*               varie direttive nelle variabili globali
*
* Input:        byte *lpBuf            Immagine in memoria del file MPC,
*                                       escluso l'header
*
* Return:       true se tutto OK, false in caso di errore.
*
\****************************************************************************/

bool ParseMpc(byte *lpBuf) {
	uint16 i, j;
	uint16 wLen;
	byte *lpTemp, *lpTemp2;

	/* 1. Variabili */
	if (lpBuf[0]!='V' || lpBuf[1]!='A' || lpBuf[2]!='R' || lpBuf[3]!='S')
		return false;

	lpBuf+=4;
	nVars=*(uint16 *)lpBuf;
	lpBuf+=2;

	hVars=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(MPALVAR)*(uint32)nVars);
	if (hVars==NULL)
		return false;

	lpmvVars=(LPMPALVAR)GlobalLock(hVars);

	for (i=0;i<nVars;i++) {
		wLen=*(byte *)lpBuf;
		lpBuf++;
		CopyMemory(lpmvVars->lpszVarName,lpBuf,MIN(wLen, (uint16)32));
		lpBuf+=wLen;
		lpmvVars->dwVal=*(int *)lpBuf;
		lpBuf+=4;

		lpBuf++;             // Salta 'ext'
		lpmvVars++;
	}

	GlobalUnlock(hVars);

	/* 2. Messaggi */
	if (lpBuf[0]!='M' || lpBuf[1]!='S' || lpBuf[2]!='G' || lpBuf[3]!='S')
		return false;

	lpBuf+=4;
	nMsgs=*(uint16 *)lpBuf;
	lpBuf+=2;

#ifdef NEED_LOCK_MSGS
	hMsgs=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(MPALMSG)*(uint32)nMsgs);
	if (hMsgs==NULL)
		return false;

	lpmmMsgs=(LPMPALMSG)GlobalLock(hMsgs);
#else
	lpmmMsgs=(LPMPALMSG)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,sizeof(MPALMSG)*(uint32)nMsgs);
	if (lpmmMsgs==NULL)
		return false;
#endif

	for (i=0;i<nMsgs;i++) {
		lpmmMsgs->wNum=*(uint16 *)lpBuf;
		lpBuf+=2;

		for (j=0;lpBuf[j]!=0;)
			j+=lpBuf[j]+1;

		lpmmMsgs->hText=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,j+1);
		lpTemp2=lpTemp=(byte *)GlobalLock(lpmmMsgs->hText);

		for (j=0;lpBuf[j]!=0;) {
			CopyMemory(lpTemp,&lpBuf[j+1],lpBuf[j]);
			lpTemp+=lpBuf[j];
			*lpTemp++='\0';
			j+=lpBuf[j]+1;
		}

		lpBuf+=j+1;
		*lpTemp='\0';

		GlobalUnlock(lpmmMsgs->hText);
		lpmmMsgs++;
	}

#ifdef NEED_LOCK_MSGS
	GlobalUnlock(hMsgs);
#endif

	/* 3. Oggetti */
	if (lpBuf[0]!='O' || lpBuf[1]!='B' || lpBuf[2]!='J' || lpBuf[3]!='S')
		return false;

	lpBuf+=4;
	nObjs=*(uint16 *)lpBuf;
	lpBuf+=2;

	// Controlla i dialoghi
	nDialogs=0;
	hDialogs=lpmdDialogs=NULL;
	if (*((const char *)lpBuf+2)==6 && strncmp((const char *)lpBuf+3,"Dialog",6)==0) {
		nDialogs=*(uint16 *)lpBuf; lpBuf+=2;

		hDialogs=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,(uint32)nDialogs*sizeof(MPALDIALOG));
		if (hDialogs==NULL)
			return false;

		lpmdDialogs=(LPMPALDIALOG)GlobalLock(hDialogs);

		for (i=0;i<nDialogs;i++)
			if ((lpBuf=ParseDialog(lpBuf+7,&lpmdDialogs[i]))==NULL)
				return false;

		GlobalUnlock(hDialogs);
	}

	// Controlla gli item
	nItems=0;
	hItems=lpmiItems=NULL;
	if (*(lpBuf+2)==4 && strncmp((const char *)lpBuf+3,"Item",4)==0) {
		nItems=*(uint16 *)lpBuf; lpBuf+=2;

		// Alloca la memoria e li legge
		hItems=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,(uint32)nItems*sizeof(MPALITEM));
		if (hItems==NULL)
			return false;

		lpmiItems=(LPMPALITEM)GlobalLock(hItems);

		for (i=0;i<nItems;i++)
			if ((lpBuf=ParseItem(lpBuf+5,&lpmiItems[i]))==NULL)
				return false;

		GlobalUnlock(hItems);
	}

	// Controlla le locazioni
	nLocations=0;
	hLocations=lpmlLocations=NULL;
	if (*(lpBuf+2)==8 && strncmp((const char *)lpBuf+3,"Location",8)==0) {
		nLocations=*(uint16 *)lpBuf; lpBuf+=2;

		// Alloca la memoria e li legge
		hLocations=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,(uint32)nLocations*sizeof(MPALLOCATION));
		if (hLocations==NULL)
			return false;

		lpmlLocations=(LPMPALLOCATION)GlobalLock(hLocations);

		for (i=0;i<nLocations;i++)
			if ((lpBuf=ParseLocation(lpBuf+9,&lpmlLocations[i]))==NULL)
				return false;

		GlobalUnlock(hLocations);
	}

	// Controlla gli script
	nScripts=0;
	hScripts=lpmsScripts=NULL;
	if (*(lpBuf+2)==6 && strncmp((const char *)lpBuf+3,"Script",6)==0) {
		nScripts=*(uint16 *)lpBuf; lpBuf+=2;

		// Alloca la memoria
		hScripts=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,(uint32)nScripts*sizeof(MPALSCRIPT));
		if (hScripts==NULL)
			return false;

		lpmsScripts=(LPMPALSCRIPT)GlobalLock(hScripts);

		for (i=0;i<nScripts;i++) {
			if ((lpBuf=ParseScript(lpBuf+7,&lpmsScripts[i]))==NULL)
			return false;

			// Ordina i vari moments dello script
			//qsort(
			//lpmsScripts[i].Moment,
			//lpmsScripts[i].nMoments,
			//sizeof(lpmsScripts[i].Moment[0]),
			//(int (*)(const void *, const void *))CompareMoments
			//);

		}

		GlobalUnlock(hScripts);
	}

	if (lpBuf[0]!='E' || lpBuf[1]!='N' || lpBuf[2]!='D' || lpBuf[3]!='0')
		return false;

	return true;
}


} // end of namespace MPAL

} // end of namespace Tony
