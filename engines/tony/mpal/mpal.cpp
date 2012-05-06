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
 *                                     ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *                        ...                  Spyral Software snc        *
 *        .             x#""*$Nu       -= We create much MORE than ALL =- *
 *      d*#R$.          R     ^#$o     ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *    .F    ^$k         $        "$b                                      *
 *   ."       $b      u "$         #$L                                    *
 *   P         $c    :*$L"$L        '$k  Project: MPAL................... *
 *  d    @$N.   $.   d ^$b^$k         $c                                  *
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  Mpal Query Library..... *
 * 4    4k   *N  #b .>    '$N'*$u      *                                  *
 * M     $L   #$  $ 8       "$c'#$b.. .@ Author:  Giovanni Bajo.......... *
 * M     '$u   "$u :"         *$. "#*#"                                   *
 * M      '$N.  "  F           ^$k       Desc:    Libreria principale di  *
 * 4>       ^R$oue#             d                 MPAL, contenente il     *
 * '$          ""              @                  codice per le query.... *
 *  #b                       u#                                           *
 *   $b                    .@"           OS: [ ] DOS  [X] WIN95  [ ] OS/2 *
 *    #$u                .d"                                              *
 *     '*$e.          .zR".@           ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *        "*$$beooee$*"  @"M                  This source code is         *
 *             """      '$.?              Copyright (C) Spyral Software   *
 *                       '$d>                 ALL RIGHTS RESERVED         *
 *                        '$>          ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *                                                                        *
 **************************************************************************/

#include "common/scummsys.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "tony/sched.h"
#include "tony/tony.h"
#include "tony/mpal/lzo.h"	
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpaldll.h"
#include "tony/mpal/stubs.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Copyright
\****************************************************************************/

const char *mpalCopyright =
  "\n\nMPAL - MultiPurpose Adventure Language for Windows 95\n"
  "Copyright 1997-98 Giovanni Bajo and Luca Giusti\n"
  "ALL RIGHTS RESERVED\n"
  "\n"
  "\n";

/****************************************************************************\
*       Structures
\****************************************************************************/

/****************************************************************************\
*       typedef CFCALL
*       --------------
* Description: Descrizione di una chiamata a una custom function
\****************************************************************************/

typedef struct {
  int nCf;

  int arg1, arg2, arg3, arg4;
} CFCALL;
typedef CFCALL*         LPCFCALL;
typedef LPCFCALL*       LPLPCFCALL;


/****************************************************************************\
*       Global variables
\****************************************************************************/

uint32                    mpalError;

static byte *           lpMpcImage;

LPITEMIRQFUNCTION        lpiifCustom=NULL;

LPLPCUSTOMFUNCTION      lplpFunctions = NULL;
Common::String *		lplpFunctionStrings = NULL;
uint16                   nObjs;

uint16                   nVars;
HGLOBAL                  hVars;
LPMPALVAR                lpmvVars;

uint16                   nMsgs;
HGLOBAL                  hMsgs;
LPMPALMSG                lpmmMsgs;

uint16                   nDialogs;
HGLOBAL                  hDialogs;
LPMPALDIALOG             lpmdDialogs;

uint16                   nItems;
HGLOBAL                  hItems;
LPMPALITEM               lpmiItems;

uint16                   nLocations;
HGLOBAL                  hLocations;
LPMPALLOCATION           lpmlLocations;

uint16                   nScripts;
HGLOBAL                  hScripts;
LPMPALSCRIPT             lpmsScripts;

Common::File             hMpr;
uint16                   nResources;
uint32 *                 lpResources;

bool                     bExecutingAction;
bool                     bExecutingDialog;

uint32                    nPollingLocations[MAXPOLLINGLOCATIONS];
HANDLE                   hEndPollingLocations[MAXPOLLINGLOCATIONS];
uint32                   PollingThreads[MAXPOLLINGLOCATIONS];

HANDLE                   hAskChoice;
HANDLE                   hDoneChoice;

uint32                    nExecutingAction;

uint32                    nExecutingDialog;
uint32                    nExecutingChoice;
uint32                    nSelectedChoice;


/****************************************************************************\
*       Internal functions
\****************************************************************************/

/****************************************************************************\
*
* Function:     void LockVar(void);
*
* Description:  Locka le variabili per accederci
*
\****************************************************************************/

void LockVar(void) {
	lpmvVars=(LPMPALVAR)GlobalLock(hVars);
}

/****************************************************************************\
*
* Function:     void UnlockVar(void);
*
* Description:  Unlocka le variabili dopo l'uso
*
\****************************************************************************/

void UnlockVar(void) {
	GlobalUnlock(hVars);
}


/****************************************************************************\
*
* Function:     void LockMsg(void);
*
* Description:  Locka i messaggi per accederci
*
\****************************************************************************/

static void LockMsg(void) {
#ifdef NEED_LOCK_MSGS
	lpmmMsgs=(LPMPALMSG)GlobalLock(hMsgs);
#endif
}


/****************************************************************************\
*
* Function:     void UnlockMsg(void);
*
* Description:  Unlocka i messaggi dopo l'uso
*
\****************************************************************************/

static void UnlockMsg(void) {
#ifdef NEED_LOCK_MSGS
	GlobalUnlock(hMsgs);
#endif
}


/****************************************************************************\
*
* Function:     void LockDialogs(void);
*
* Description:  Locka i dialoghi per accederci
*
\****************************************************************************/

static void LockDialogs(void) {
	lpmdDialogs=(LPMPALDIALOG)GlobalLock(hDialogs);
}


/****************************************************************************\
*
* Function:     void UnlockDialogs(void);
*
* Description:  Unlocka i dialoghi dopo l'uso
*
\****************************************************************************/

static void UnlockDialogs(void) {
	GlobalUnlock(hDialogs);
}


/****************************************************************************\
*
* Function:     void LockLocations(void);
*
* Description:  Locka le strutture di dati sulle locazioni
*
\****************************************************************************/

static void LockLocations(void) {
	lpmlLocations=(LPMPALLOCATION)GlobalLock(hLocations);
}


/****************************************************************************\
*
* Function:     void UnlockLocations(void);
*
* Description:  Unlocka le strutture di dati sulle locazioni
*
\****************************************************************************/

static void UnlockLocations(void) {
	GlobalUnlock(hLocations);
}


/****************************************************************************\
*
* Function:     void LockItems(void);
*
* Description:  Locka le strutture di dati sugli item
*
\****************************************************************************/

static void LockItems(void) {
	lpmiItems=(LPMPALITEM)GlobalLock(hItems);
}


/****************************************************************************\
*
* Function:     void UnlockItems(void);
*
* Description:  Unlocka le strutture di dati sugli item
*
\****************************************************************************/

static void UnlockItems(void) {
	GlobalUnlock(hItems);
}

/****************************************************************************\
*
* Function:     void LockScripts(void);
*
* Description:  Locka le strutture di dati sugli script
*
\****************************************************************************/

static void LockScripts(void) {
	lpmsScripts=(LPMPALSCRIPT)GlobalLock(hScripts);
}


/****************************************************************************\
*
* Function:     void UnlockScripts(void);
*
* Description:  Unlocka le strutture di dati sugli script
*
\****************************************************************************/

static void UnlockScripts(void) {
	GlobalUnlock(hScripts);
}


/****************************************************************************\
*
* Function:     int varGetValue(char * lpszVarName);
*
* Description:  Restituisce il valore corrente di una variabile globale
*
* Input:        char * lpszVarName       Nome della variabile
*
* Return:       Valore corrente
*
* Note:         Prima di questa funzione, bisogna richiamare LockVar() che
*               locka le variabili globali per l'utilizzo. Dopo inoltre bi-
*               sogna ricordarsi di chiamare UnlockVar()
*
\****************************************************************************/

int32 varGetValue(const char *lpszVarName) {
	int i;
	LPMPALVAR v=lpmvVars;

	for (i = 0; i < nVars; v++, i++)
		if (strcmp(lpszVarName, v->lpszVarName) == 0)
			return v->dwVal;

	mpalError = 1;
	return 0;
}


/****************************************************************************\
*
* Function:     void varSetValue(char * lpszVarName, int val);
*
* Description:  Setta un nuovo valore per una variabile globale di MPAL
*
* Input:        char * lpszVarName       Nome della variabile
*               int val                 Valore da settare
*
\****************************************************************************/

void varSetValue(const char *lpszVarName, int32 val) {
	uint i;
	LPMPALVAR v = lpmvVars;

	for (i = 0; i < nVars; v++, i++)
		if (strcmp(lpszVarName, v->lpszVarName) == 0) {
			v->dwVal = val;
			if (lpiifCustom != NULL && strncmp(v->lpszVarName, "Pattern.", 8) == 0) {
				i = 0;
				sscanf(v->lpszVarName, "Pattern.%u", &i);
				lpiifCustom(i, val, -1);
			} else if (lpiifCustom != NULL && strncmp(v->lpszVarName, "Status.", 7) == 0) {
				i = 0;
				sscanf(v->lpszVarName,"Status.%u", &i);
				lpiifCustom(i, -1, val);
			}
			return;
		}

	mpalError = 1;
	return;
}


/****************************************************************************\
*
* Function:     int locGetOrderFromNum(uint32 nLoc);
*
* Description:  Trova l'indice della locazione #nLoc all'interno dell'array
*               delle strutture delle locazioni
*
* Input:        uint32 nLoc              Numero della locazione da cercare
*
* Return:       Indice, o -1 se la locazione non e' presente
*
* Note:         Per funzionare, la funzione necessita che le locazioni siano
*               state lockate con LockLoc()
*
\****************************************************************************/

static int locGetOrderFromNum(uint32 nLoc) {
	int i;
	LPMPALLOCATION loc = lpmlLocations;

	for (i = 0; i < nLocations; i++,loc++)
		if (loc->nObj == nLoc)
			return i;

	return -1;
}

/****************************************************************************\
*
* Function:     int msgGetOrderFromNum(uint32 nMsg);
*
* Description:  Trova l'indice del messaggio #nMsg all'interno dell'array
*               delle strutture dei messaggi
*
* Input:        uint32 nMsg              Numero del messaggio da cercare
*
* Return:       Indice, o -1 se il messaggio non e' presente
*
* Note:         Per funzionare, la funzione necessita che i messaggi siano
*               stati lockati con LockMsg()
*
\****************************************************************************/

static int msgGetOrderFromNum(uint32 nMsg) {
	int i;
	LPMPALMSG msg=lpmmMsgs;

	for (i = 0; i < nMsgs; i++, msg++)
		if (msg->wNum == nMsg)
			return i;

	return -1;
}


/****************************************************************************\
*
* Function:     int itemGetOrderFromNum(uint32 nItem);
*
* Description:  Trova l'indice dell'item #nItem all'interno dell'array delle
*               strutture degli item
*
* Input:        uint32 nItem             Numero dell'item da cercare
*
* Return:       Indice, o -1 se l'item non e' presente
*
* Note:         Per funzionare, questa funzione necessita che gli item siano
*               stati lockati tramite LockItem()
*
\****************************************************************************/

static int itemGetOrderFromNum(uint32 nItem) {
	int i;
	LPMPALITEM item=lpmiItems;

	for (i = 0; i < nItems; i++, item++)
		if (item->nObj == nItem)
			return i;

	return -1;
}


/****************************************************************************\
*
* Function:     int scriptGetOrderFromNum(uint32 nScript);
*
* Description:  Trova l'indice dello script #nScript all'interno dell'array
*               delle strutture degli script
*
* Input:        uint32 nScript           Numero dello script da cercare
*
* Return:       Indice, o -1 se lo script non e' presente
*
* Note:         Per funzionare, questa funzione necessita che gli script siano
*               stati lockati tramite LockScript()
*
\****************************************************************************/

static int scriptGetOrderFromNum(uint32 nScript) {
	int i;
	LPMPALSCRIPT script = lpmsScripts;

	for (i = 0; i < nScripts; i++,script++)
		if (script->nObj == nScript)
			return i;

	return -1;
}


/****************************************************************************\
*
* Function:     int dialogGetOrderFromNum(uint32 nDialog);
*
* Description:  Trova l'indice del dialog #nDialog all'interno dell'array
*               delle strutture dei dialog
*
* Input:        uint32 nDialog           Numero del dialog da cercare
*
* Return:       Indice, o -1 se il dialog non e' presente
*
* Note:         Per funzionare, questa funzione necessita che i dialog siano
*               stati lockati tramite LockDialogs()
*
\****************************************************************************/

static int dialogGetOrderFromNum(uint32 nDialog) {
	int i;
	LPMPALDIALOG dialog=lpmdDialogs;

	for (i = 0; i < nDialogs; i++, dialog++)
		if (dialog->nObj == nDialog)
			return i;

	return -1;
}



/****************************************************************************\
*
* Function:     char * DuplicateMessage(uint32 nMsgOrd);
*
* Description:  Duplica un messaggio
*
* Input:        uint32 nMsgOrd           Indice del messaggio dentro l'array
*                                       di strutture dei messaggi
*
* Return:       Pointer al messaggio duplicato (che puo' essere liberato
*               con GlobalFree()).
*
\****************************************************************************/

static char *DuplicateMessage(uint32 nMsgOrd) {
	const char *origmsg;
	char *clonemsg;
	int j;

	if (nMsgOrd == (uint32)-1)
		return NULL;

	origmsg = (const char *)GlobalLock(lpmmMsgs[nMsgOrd].hText);

	j = 0;
	while (origmsg[j] != '\0' || origmsg[j+1] != '\0')
		j++;
	j += 2;

	clonemsg=(char *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, j);
	if (clonemsg == NULL)
		return NULL;

	CopyMemory(clonemsg, origmsg, j);
	GlobalUnlock(lpmmMsgs[nMsgOrd].hText);

	return clonemsg;
}


/****************************************************************************\
*
* Function:     char * DuplicateDialogPeriod(uint32 nDlgOrd, uint32 nPeriod);
*
* Description:  Duplica una frase di un dialog
*
* Input:        uint32 nDlgOrd           Indice del dialogo dentro l'array di
*                                       strutture dei dialoghi
*
*               uint32 nPeriod           Numero della frase da duplicare
*
* Return:       Pointer alla frase duplicata (che puo' essere liberata con
*               GlobalFree()).
*
\****************************************************************************/

static char *DuplicateDialogPeriod(uint32 nPeriod) {
	const char *origmsg;
	char *clonemsg;
	LPMPALDIALOG dialog=lpmdDialogs+nExecutingDialog;
	int i,j;

	for (j = 0; dialog->Periods[j] != NULL; j++)
		if (dialog->PeriodNums[j] == nPeriod) {
			/* Trovata la frase, va duplicata */
			origmsg = (const char *)GlobalLock(dialog->Periods[j]);

			/* Calcola la lunghezza e alloca la memoria */
			i = 0;
			while (origmsg[i] != '\0') i++;

			clonemsg = (char *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, i + 1);
			if (clonemsg == NULL)
				return NULL;

			CopyMemory(clonemsg, origmsg, i);

			GlobalUnlock(dialog->Periods[j]);

			return clonemsg;
		}

	return NULL;
}



/****************************************************************************\
*
* Function:     HGLOBAL resLoad(uint32 dwId);
*
* Description:  Carica una risorsa dal file MPR
*
* Input:        uint32 dwId              ID della risorsa da caricare
*
* Return:       Handle alla memoria in cui si trova la risorsa
*
\****************************************************************************/

HGLOBAL resLoad(uint32 dwId) {
	int i;
	HGLOBAL h;
	char head[4];
	uint32 nBytesRead;
	uint32 nSizeComp, nSizeDecomp;
	byte *temp, *buf;

	for (i = 0; i < nResources; i++)
		if (lpResources[i * 2] == dwId) {
			hMpr.seek(lpResources[i * 2 + 1]);
			nBytesRead = hMpr.read(head, 4);
			if (nBytesRead != 4)
				return NULL;
			if (head[0] != 'R' || head[1] != 'E' || head[2] != 'S' || head[3] != 'D')
				return NULL;

			nSizeDecomp = hMpr.readUint32LE();
			if (hMpr.err())
				return NULL;

			nSizeComp = hMpr.readUint32LE();
			if (hMpr.err())
				return NULL;

			h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, nSizeDecomp + (nSizeDecomp / 1024) * 16);
			buf = (byte *)GlobalLock(h);
			temp = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,nSizeComp);

			nBytesRead = hMpr.read(temp, nSizeComp);
			if (nBytesRead != nSizeComp)
				return NULL;

			lzo1x_decompress(temp, nSizeComp, buf, &nBytesRead);
			if (nBytesRead != nSizeDecomp)
				return NULL;

			GlobalFree(temp);
			GlobalUnlock(h);
			return h;
		}

	return NULL;
}

static uint32 *GetSelectList(uint32 i) {
	uint32 *sl;
	int j,k,num;
	LPMPALDIALOG dialog=lpmdDialogs+nExecutingDialog;

	/* Conta quanti select attivi ci sono */
	num = 0;
	for (j = 0; dialog->Choice[i].Select[j].dwData != 0; j++)
		if (dialog->Choice[i].Select[j].curActive)
			num++;

	/* Se sono 0, e' un errore */
	if (num == 0)
		return NULL;

	sl= (uint32 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (sl == NULL)
		return NULL;

	/* Copia il dato di ogni select attivo dentro la lista */
	k = 0;
	for (j = 0; dialog->Choice[i].Select[j].dwData != 0; j++)
		if (dialog->Choice[i].Select[j].curActive)
			sl[k++] = dialog->Choice[i].Select[j].dwData;

	sl[k] = (uint32)NULL;
	return sl;
}

static uint32 *GetItemList(uint32 nLoc) {
	uint32 *il;
	uint32 num,i,j;
	LPMPALVAR v = lpmvVars;

	num = 0;
	for (i = 0; i < nVars; i++,v++) {
		if (strncmp(v->lpszVarName,"Location",8) == 0 && v->dwVal == nLoc)
			num++;
	}

	il=(uint32 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (il == NULL)
		return NULL;

	v = lpmvVars;
	j = 0;
	for (i = 0; i < nVars; i++,v++) {
		if (strncmp(v->lpszVarName,"Location",8) == 0 && v->dwVal == nLoc) {
			sscanf(v->lpszVarName, "Location.%u", &il[j]);
			j++;
		}
	}

	il[j] = (uint32)NULL;
	return il;
}

static LPITEM GetItemData(uint32 nOrdItem) {
	LPMPALITEM curitem = lpmiItems+nOrdItem;
	LPITEM ret;
	HGLOBAL hDat;
	char *dat;
	int i, j;
	char *patlength;
	uint32 dim;

	// Lo zeroinit e' obbligatorio!!!!
	ret = (LPITEM)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(ITEM));
	if (ret == NULL)
		return NULL;
	ret->speed = 150;

	hDat = resLoad(curitem->dwRes);
	dat = (char *)GlobalLock(hDat);

	if (dat[0] == 'D' && dat[1] == 'A' && dat[2] == 'T') {
		i = dat[3];            // Versione!! Per ora 1.0
		dat += 4;

		if (i >= 0x10) { // Dalla 1.0 c'e' il punto di destinazione per ogni oggetto
			ret->destX = (int16)READ_LE_UINT16(dat);
			ret->destY = (int16)READ_LE_UINT16(dat + 2);
			dat+=4;
		}

		if (i >= 0x11) {// Dalla 1.1 c'e' la velocita' di animazione
			ret->speed = READ_LE_UINT16(dat);
			dat += 2;
		} else
			ret->speed = 150;
	}

	ret->numframe=*dat++;
	ret->numpattern=*dat++;
	ret->Zvalue=*dat++;

	// Carica le coordinate left&top di ogni frame
	for (i = 0; i < ret->numframe; i++) {
		ret->frameslocations[i].left = (int16)READ_LE_UINT16(dat);
		ret->frameslocations[i].top = (int16)READ_LE_UINT16(dat + 2);
		dat += 4;
	}

	// Carica le dimensioni di ogni frame e calcola right&bottom
	for (i = 0; i < ret->numframe; i++) {
		ret->frameslocations[i].right = (int16)READ_LE_UINT16(dat) + ret->frameslocations[i].left;
		ret->frameslocations[i].bottom = (int16)READ_LE_UINT16(dat + 2) + ret->frameslocations[i].top;
		dat+=4;
	}

	// Carica i bounding box di ogni frame
	for (i = 0; i < ret->numframe; i++) {
		ret->bbox[i].left = (int16)READ_LE_UINT16(dat);
		ret->bbox[i].top = (int16)READ_LE_UINT16(dat + 2);
		ret->bbox[i].right = (int16)READ_LE_UINT16(dat + 4);
		ret->bbox[i].bottom = (int16)READ_LE_UINT16(dat + 6);
		dat+=8;
	}

	// Carica i pattern di animazione
	patlength = dat;
	dat+=ret->numpattern;

	for (i = 1; i < ret->numpattern; i++) {
		for (j = 0; j < patlength[i]; j++)
			ret->pattern[i][j] = dat[j];
		ret->pattern[i][(int)patlength[i]] = 255;   // Termina i pattern
		dat += patlength[i];
	}

	// Carica i singoli frame di animazione
	for (i = 1; i < ret->numframe; i++) {
		dim=(uint32)(ret->frameslocations[i].right-ret->frameslocations[i].left) *
			(uint32)(ret->frameslocations[i].bottom-ret->frameslocations[i].top);
		ret->frames[i]=(char *)GlobalAlloc(GMEM_FIXED,dim);
   
		if (ret->frames[i] == NULL)
			return NULL;
		CopyMemory(ret->frames[i], dat, dim);
		dat += dim;
	}

	// Controlla se siamo arrivati fino alla fine del file
	i = READ_LE_UINT16(dat);
	if (i != 0xABCD)
		return NULL;

	GlobalUnlock(hDat);
	GlobalFree(hDat);

	return ret;
}


/****************************************************************************\
*
* Function:     void PASCAL CustomThread(LPCFCALL p);
*
* Description:  Thread che richiama una funzione custom. Viene usato negli
*               script, in modo che ciascuna funzione venga eseguita senza
*               ritardare le altre
*
* Input:        LPCFCALL p              Struttura che definisce la chiamata
*
* Note:         La struttura passata come parametro viene freeata con
*               GlobalFree() alla fine dell'esecuzione.
*
\****************************************************************************/

void PASCAL CustomThread(LPCFCALL p) {
	// FIXME: Convert to proper corotuine call
	warning("FIXME: CustomThread call");

	lplpFunctions[p->nCf](nullContext, p->arg1, p->arg2, p->arg3, p->arg4);
	GlobalFree(p);
	ExitThread(1);
//	_endthread();
}


/****************************************************************************\
*
* Function:     void PASCAL ScriptThread(LPMPALSCRIPT s);
*
* Description:  Esegue uno script. Questa funzione e' pensata come starting
*               point per un thread
*
* Input:        LPMPALSCRIPT s          Script da eseguire
*
* Note:         Lo script passato come parametro viene, alla fine dell'ese-
*               cuzione, freeato con una GlobalFree()
*
\****************************************************************************/

void PASCAL ScriptThread(LPMPALSCRIPT s) {
	uint i,j,k;
	uint32 dwStartTime = timeGetTime();
	uint32 dwCurTime;
	uint32 dwId;
	static HANDLE cfHandles[MAX_COMMANDS_PER_MOMENT];
	int numHandles = 0;
	LPCFCALL p;

// warning("PlayScript(): Moments: %u\n",s->nMoments);
	for (i = 0; i < s->nMoments; i++) {
		// Dorme il tempo necessario per arrivare al momento successivo
		if (s->Moment[i].dwTime == -1) {
			WaitForMultipleObjects(numHandles, cfHandles, true, INFINITE);
			dwStartTime = timeGetTime();
		} else {
			dwCurTime = timeGetTime();
			if (dwCurTime < dwStartTime + (s->Moment[i].dwTime * 100)) {
  //     warning("PlayScript(): Sleeping %lums\n",dwStartTime+(s->Moment[i].dwTime*100)-dwCurTime);
				Sleep(dwStartTime+(s->Moment[i].dwTime * 100) - dwCurTime);
			}
		}

		numHandles = 0;
		for (j = 0;j<s->Moment[i].nCmds; j++) {
			k=s->Moment[i].CmdNum[j];

			switch (s->Command[k].type) {
			case 1:
				p=(LPCFCALL)GlobalAlloc(GMEM_FIXED, sizeof(CFCALL));
				if (p == NULL) {
					mpalError = 1;
					ExitThread(0);
//					_endthread();
				}

				p->nCf=s->Command[k].nCf;
				p->arg1=s->Command[k].arg1;
				p->arg2=s->Command[k].arg2;
				p->arg3=s->Command[k].arg3;
				p->arg4=s->Command[k].arg4;

					 // !!! Nuova gestione dei thread
				if ((cfHandles[numHandles++] = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)CustomThread,(void *)p, 0, &dwId)) == NULL) {
			 //if ((cfHandles[numHandles++]=(void*)_beginthread(CustomThread, 10240, (void *)p))==(void*)-1)
					mpalError = 1;
					ExitThread(0);
//					_endthread();
				}
				break;

			case 2:
				LockVar();
				varSetValue(
					s->Command[k].lpszVarName,
					EvaluateExpression(s->Command[k].expr)
				);
				UnlockVar();
				break;

			default:
				mpalError = 1;
				GlobalFree(s);
				ExitThread(0);
//				_endthread();
			}
		}
	}

	GlobalFree(s);
	ExitThread(1);
	//_endthread();
}


/****************************************************************************\
*
* Function:     void PASCAL ActionThread(LPMPALITEM item);
*
* Description:  Thread che esegue una azione su un item. Il thread
*               esegue sempre l'azione 0, per cui e' necessario creare
*               un item nuovo in cui l'azione 0 sia quella richiesta.
*               Inoltre non viene controllata l'espressione when, ma viene
*               sempre eseguita l'azione.
*
* Input:        LPMPALITEM item         Item che contiene l'azione
*
\****************************************************************************/

void ActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int j, k;
	CORO_END_CONTEXT(_ctx);

	const LPMPALITEM item = *(const LPMPALITEM *)param;

	CORO_BEGIN_CODE(_ctx);

	mpalError = 0;
	for (_ctx->j = 0; _ctx->j < item->Action[item->dwRes].nCmds; _ctx->j++) {
		_ctx->k = item->Action[item->dwRes].CmdNum[_ctx->j];

		if (item->Command[_ctx->k].type == 1) {
			// Custom function
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Call=%s params=%d,%d,%d,%d",
				_vm->_scheduler.getCurrentPID(), lplpFunctionStrings[item->Command[_ctx->k].nCf].c_str(),
				item->Command[_ctx->k].arg1, item->Command[_ctx->k].arg2,
				item->Command[_ctx->k].arg3, item->Command[_ctx->k].arg4
			);

			CORO_INVOKE_4(lplpFunctions[item->Command[_ctx->k].nCf],
				item->Command[_ctx->k].arg1,
				item->Command[_ctx->k].arg2,
				item->Command[_ctx->k].arg3,
				item->Command[_ctx->k].arg4

			);
		} else if (item->Command[_ctx->k].type == 2) {
			// Variable assign
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Variable=%s",
				_vm->_scheduler.getCurrentPID(), item->Command[_ctx->k].lpszVarName);

			LockVar();
			varSetValue(item->Command[_ctx->k].lpszVarName, EvaluateExpression(item->Command[_ctx->k].expr));
			UnlockVar();
			break;

		} else {
			mpalError = 1;
			break;
		}
	}

	GlobalFree(item);
	
	debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d ended", _vm->_scheduler.getCurrentPID());

	CORO_KILL_SELF();

	CORO_END_CODE;
}

/**
 * This thread monitors a created action to detect when it ends.
 * @remarks				Since actions can spawn sub-actions, this needs to be a
 *						separate thread to determine when the outer action is done
 */
void ShutUpActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(_vm->_scheduler.waitForSingleObject, pid, INFINITE);

	bExecutingAction = false;

	CORO_KILL_SELF();

	CORO_END_CODE;
}

/****************************************************************************\
*
* Function:     void PASCAL LocationPollThread(uint32 id);
*
* Description:  Esegue il polling di una locazione (starting point di un
*               thread).
*
* Input:        uint32 id                Indice per gli array relativi ai
*                                       polling degli item delle locazioni
*
\****************************************************************************/

void LocationPollThread(CORO_PARAM, const void *param) {
	typedef struct {
		uint32 nItem, nAction;

		uint16 wTime;
		byte perc;
		HGLOBAL when;
		byte nCmds;
		uint16 CmdNum[MAX_COMMANDS_PER_ACTION];

		uint32 dwLastTime;
	} MYACTION;

	typedef struct {
		uint32 nItem;
		uint32 hThread;
	} MYTHREAD;

	CORO_BEGIN_CONTEXT;
		uint32 *il;
		int i, j, k;
		int numitems;
		int nRealItems;
		LPMPALITEM curItem,newItem;
		int nIdleActions;
		uint32 curTime;
		uint32 dwSleepTime;
		uint32 dwId;
		int ord;
		bool delayExpired;

		MYACTION *MyActions;
		MYTHREAD *MyThreads;
	CORO_END_CONTEXT(_ctx);

	uint32 id = *((const uint32 *)param);

	CORO_BEGIN_CODE(_ctx);

 /* Tanto per cominciare, e' necessario richiedere la lista degli item
    presenti nella locazione. */
	_ctx->il = mpalQueryItemList(nPollingLocations[id]);

 /* Contiamo gli items */
	for (_ctx->numitems = 0; _ctx->il[_ctx->numitems] != 0; _ctx->numitems++)
		;

 /* Cerchiamo gli items della locazione senza idle actions e li eliminiamo
    dalla lista */
	LockItems();
	_ctx->nIdleActions = 0;
	_ctx->nRealItems = 0;
	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		_ctx->ord = itemGetOrderFromNum(_ctx->il[_ctx->i]);

		if (_ctx->ord == -1) continue;
	 
		_ctx->curItem = lpmiItems + _ctx->ord;

		_ctx->k = 0;
		for (_ctx->j = 0; _ctx->j < _ctx->curItem->nActions; _ctx->j++)
			if (_ctx->curItem->Action[_ctx->j].num == 0xFF)
				_ctx->k++;

		_ctx->nIdleActions += _ctx->k;

		if (_ctx->k == 0)
			/* Possiamo eliminare questo item dalla lista */
			_ctx->il[_ctx->i] = (uint32)NULL;
		else
			_ctx->nRealItems++;
	}
	UnlockItems();

	/* Se non e' rimasto nessuno possiamo uscire */
	if (_ctx->nRealItems == 0) {
		GlobalFree(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	_ctx->MyThreads = (MYTHREAD *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nRealItems * sizeof(MYTHREAD));
	if (_ctx->MyThreads == NULL) {
		GlobalFree(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	/* Inizializziamo le routine random */
	//curTime = timeGetTime();
	//srand(curTime);


 /* Abbiamo appurato che esiste almeno un item che contiene idle actions.
    Ora creaiamo le copie speculari delle idle actions */
	_ctx->MyActions = (MYACTION *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nIdleActions * sizeof(MYACTION));
	if (_ctx->MyActions == NULL) {
		GlobalFree(_ctx->MyThreads);
		GlobalFree(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	LockItems();
	_ctx->k = 0;

	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		if (_ctx->il[_ctx->i] == 0)
			continue;

		_ctx->curItem = lpmiItems + itemGetOrderFromNum(_ctx->il[_ctx->i]);

		for (_ctx->j = 0; _ctx->j < _ctx->curItem->nActions; _ctx->j++)
			if (_ctx->curItem->Action[_ctx->j].num == 0xFF) {
				_ctx->MyActions[_ctx->k].nItem = _ctx->il[_ctx->i];
				_ctx->MyActions[_ctx->k].nAction = _ctx->j;

				_ctx->MyActions[_ctx->k].wTime = _ctx->curItem->Action[_ctx->j].wTime;
				_ctx->MyActions[_ctx->k].perc = _ctx->curItem->Action[_ctx->j].perc;
				_ctx->MyActions[_ctx->k].when = _ctx->curItem->Action[_ctx->j].when;
				_ctx->MyActions[_ctx->k].nCmds = _ctx->curItem->Action[_ctx->j].nCmds;
				CopyMemory(_ctx->MyActions[_ctx->k].CmdNum, _ctx->curItem->Action[_ctx->j].CmdNum,
				MAX_COMMANDS_PER_ACTION * sizeof(uint16));

				_ctx->MyActions[_ctx->k].dwLastTime = timeGetTime();
				_ctx->k++;
			}
	}

	UnlockItems();

	/* La item list non ci serve piu' */
	GlobalFree(_ctx->il);


	/* Eccoci al ciclo principale. */
	while (1) {
		/* Cerchiamo tra tutte le idle actions quella a cui manca meno tempo per
			l'esecuzione */
		_ctx->curTime = timeGetTime();
		_ctx->dwSleepTime = (uint32)-1L;

		for (_ctx->k = 0;_ctx->k<_ctx->nIdleActions;_ctx->k++)
			if (_ctx->curTime >= _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime) {
				_ctx->dwSleepTime = 0;
				break;
		     } else
				_ctx->dwSleepTime = MIN(_ctx->dwSleepTime, _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime - _ctx->curTime);

		/* Ci addormentiamo, ma controllando sempre l'evento che viene settato
			quando viene richiesta la nostra chiusura */
		_ctx->k = WaitForSingleObject(hEndPollingLocations[id], _ctx->dwSleepTime);
		if (_ctx->k == WAIT_OBJECT_0)
			break;

		for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
			if (_ctx->MyThreads[_ctx->i].nItem != 0) {
				CORO_INVOKE_3(_vm->_scheduler.waitForSingleObject, _ctx->MyThreads[_ctx->i].hThread, 0, &_ctx->delayExpired);

				// if result ) == WAIT_OBJECT_0)
				if (!_ctx->delayExpired)
					_ctx->MyThreads[_ctx->i].nItem = 0;
			}

		_ctx->curTime = timeGetTime();

		/* Cerchiamo all'interno delle idle actions quale e' necessario eseguire */
		for (_ctx->k = 0; _ctx->k < _ctx->nIdleActions; _ctx->k++)
			if (_ctx->curTime >= _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime) {
				_ctx->MyActions[_ctx->k].dwLastTime += _ctx->MyActions[_ctx->k].wTime;

			   /* E' _ctx->il momento di tirare _ctx->il nostro dado virtuale, e controllare
				  se la sorte e' dalla parte della idle action */
				byte randomVal = (byte)_vm->_randomSource.getRandomNumber(99);
				if (randomVal < _ctx->MyActions[_ctx->k].perc) {
					/* Controlliamo se c'e' una action in esecuzione sull'item */
					if ((bExecutingAction) && (nExecutingAction == _ctx->MyActions[_ctx->k].nItem))
						continue;

					/* Controlliamo se c'e' gia' un'altra idle function in esecuzione
						sullo stesso item */
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
						if (_ctx->MyThreads[_ctx->i].nItem == _ctx->MyActions[_ctx->k].nItem)
							break;

					if (_ctx->i < _ctx->nRealItems)
						continue;

					/* Ok, siamo gli unici :) */
					LockItems();
					_ctx->curItem=lpmiItems+itemGetOrderFromNum(_ctx->MyActions[_ctx->k].nItem);

					/* Controlliamo se c'e' un esperessione WhenExecute */
					_ctx->j=_ctx->MyActions[_ctx->k].nAction;
					if (_ctx->curItem->Action[_ctx->j].when != NULL)
						if (!EvaluateExpression(_ctx->curItem->Action[_ctx->j].when)) {
							UnlockItems();
							continue;
						}

					/* Ok, possiamo eseguire la azione. Per comodita' lo facciamo in
						un nuovo thread */
					_ctx->newItem = (LPMPALITEM)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALITEM));
					if (_ctx->newItem == false) {
						GlobalFree(_ctx->MyThreads);
						GlobalFree(_ctx->MyActions);
						
						CORO_KILL_SELF();
						return;
					}

					CopyMemory(_ctx->newItem,_ctx->curItem, sizeof(MPALITEM));
					UnlockItems();

					/* Copiamo l'azione nella #0 */
//					_ctx->newItem->Action[0].nCmds = _ctx->curItem->Action[_ctx->j].nCmds;
//					CopyMemory(_ctx->newItem->Action[0].CmdNum,_ctx->curItem->Action[_ctx->j].CmdNum,_ctx->newItem->Action[0].nCmds*sizeof(_ctx->newItem->Action[0].CmdNum[0]));
					_ctx->newItem->dwRes=_ctx->j;

					/* Creaiamo l'action thread. Provvedera' lui a liberare la memoria
						allocata per _ctx->il nuovo item */
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
						if (_ctx->MyThreads[_ctx->i].nItem == 0)
							break;

					_ctx->MyThreads[_ctx->i].nItem = _ctx->MyActions[_ctx->k].nItem;

					// !!! Nuova gestione dei thread
					if ((_ctx->MyThreads[_ctx->i].hThread = _vm->_scheduler.createProcess(ActionThread, &_ctx->newItem, sizeof(LPMPALITEM))) == 0) {
					//if ((_ctx->MyThreads[_ctx->i].hThread=(void*)_beginthread(ActionThread, 10240,(void *)_ctx->newItem))==(void*)-1)
						GlobalFree(_ctx->newItem);
						GlobalFree(_ctx->MyThreads);
						GlobalFree(_ctx->MyActions);
						
						CORO_KILL_SELF();
						return;
					}

					/* Skippa tutte le idle action dello stesso item */
				}
			}
	}

	/* Chiude tutti _ctx->i thread interni */

	/*

		CODICE OBSOLETO: ANDIAMO DI SKIP CHE RULLA

 for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
   if (_ctx->MyThreads[_ctx->i].nItem != 0) {
     TerminateThread(_ctx->MyThreads[_ctx->i].hThread, 0);
     CloseHandle(_ctx->MyThreads[_ctx->i].hThread);
   }
*/

	// Set idle skip on
	// FIXME: Convert to co-routine
	lplpFunctions[200](nullContext, 0, 0, 0, 0);

	for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
		if (_ctx->MyThreads[_ctx->i].nItem != 0) {
			CORO_INVOKE_3(_vm->_scheduler.waitForSingleObject, _ctx->MyThreads[_ctx->i].hThread, 5000, &_ctx->delayExpired);

/*
			//if (result != WAIT_OBJECT_0)
			if (_ctx->delayExpired)
				TerminateThread(_ctx->MyThreads[_ctx->i].hThread, 0);
*/
			_vm->_scheduler.killMatchingProcess(_ctx->MyThreads[_ctx->i].hThread);
		}

	// Set idle skip off
	CORO_INVOKE_4(lplpFunctions[201], 0, 0, 0, 0);

	/* Abbiamo finito */
	GlobalFree(_ctx->MyThreads);
	GlobalFree(_ctx->MyActions);
	
	CORO_KILL_SELF();

	CORO_END_CODE;
}



/****************************************************************************\
*
* Function:     void ShutUpDialogThread(HANDLE hThread);
*
* Description:  Aspetta la fine dell'esecuzione del dialog thread e ripri-
*               stina le variabili globali indicando che il dialogo e' finito.
*
* Input:        HANDLE hThread          Handle del dialog thread
*
* Note:         Si ricorre a questo thread aggiuntivo, al posto di azzerare
*               le variabili all'interno del dialog thread stesso, poiche',
*               a causa della natura ricorsiva di un dialogo, non e' sempre
*               possibile sapere quando esattamente finisce un dialogo.
*
\****************************************************************************/

void ShutUpDialogThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(_vm->_scheduler.waitForSingleObject, pid, INFINITE);

	bExecutingDialog = false;
	nExecutingDialog = 0;
	nExecutingChoice = 0;

	SetEvent(hAskChoice);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void DoChoice(CORO_PARAM, uint32 nChoice);

/**
 * Executes a group of the current dialog. Can 'be the Starting point of a process.
 * @parm nGroup					Number of the group to perform
 */
void GroupThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		LPMPALDIALOG dialog;
		int i, j, k;
		int type;
	CORO_END_CONTEXT(_ctx);

	uint32 nGroup = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	// Lock the _ctx->dialog
	LockDialogs();

	// Find the pointer to the current _ctx->dialog
	_ctx->dialog = lpmdDialogs + nExecutingDialog;

	// Search inside the group requesting the _ctx->dialog
	for (_ctx->i = 0; _ctx->dialog->Group[_ctx->i].num != 0; _ctx->i++) {
		if (_ctx->dialog->Group[_ctx->i].num == nGroup) {
			// Cycle through executing the commands of the group
			for (_ctx->j = 0; _ctx->j < _ctx->dialog->Group[_ctx->i].nCmds; _ctx->j++) {
				_ctx->k = _ctx->dialog->Group[_ctx->i].CmdNum[_ctx->j];

				_ctx->type = _ctx->dialog->Command[_ctx->k].type;
				if (_ctx->type == 1) {
					// Call custom function
					CORO_INVOKE_4(lplpFunctions[_ctx->dialog->Command[_ctx->k].nCf],
						_ctx->dialog->Command[_ctx->k].arg1, 
						_ctx->dialog->Command[_ctx->k].arg2,
						_ctx->dialog->Command[_ctx->k].arg3, 
						_ctx->dialog->Command[_ctx->k].arg4
					);

				} else if (_ctx->type == 2) {
					// Set a variable
					LockVar();
					varSetValue(_ctx->dialog->Command[_ctx->k].lpszVarName, EvaluateExpression(_ctx->dialog->Command[_ctx->k].expr));
					UnlockVar();
					
				} else if (_ctx->type == 3) {
					// DoChoice: call the chosen function
					CORO_INVOKE_1(DoChoice, (uint32)_ctx->dialog->Command[_ctx->k].nChoice);
					
				} else {
					mpalError = 1;
					UnlockDialogs();
					
					CORO_KILL_SELF();
					return;
				}
			}

			/* The gruop is finished, so we can return to the calling function.
			 * If the group was the first called, then the process will automatically 
			 * end. Otherwise it returns to the caller method
			 */
			return;
		}
	}

	/* Se siamo qui, vuol dire che non abbiamo trovato il gruppo richiesto */
	mpalError = 1;
	UnlockDialogs();
	
	CORO_KILL_SELF();

	CORO_END_CODE;
}


/****************************************************************************\
*
* Function:     void DoChoice(uint32 nChoice);
*
* Description:  Esegue una scelta nel dialogo corrente
*
* Input:        uint32 nChoice           Numero della scelta da eseguire
*
\****************************************************************************/

void DoChoice(CORO_PARAM, uint32 nChoice) {
	CORO_BEGIN_CONTEXT;
		LPMPALDIALOG dialog;
		int i, j, k;
		uint32 nGroup;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	/* Locka _ctx->i dialoghi */
	LockDialogs();

	/* Trova il puntatore al dialogo corrente */
	_ctx->dialog=lpmdDialogs+nExecutingDialog;

	/* Cerca la scelta richiesta tra quelle nel dialogo */
	for (_ctx->i = 0; _ctx->dialog->Choice[_ctx->i].nChoice != 0; _ctx->i++)
		if (_ctx->dialog->Choice[_ctx->i].nChoice == nChoice)
			break;

	/* Se non l'ha trovata, esce con errore */
	if (_ctx->dialog->Choice[_ctx->i].nChoice == 0) {
		/* Se siamo qui, non abbiamo trovato la choice richiesta */
		mpalError = 1;
		UnlockDialogs();

		CORO_KILL_SELF();
		return;
	}

	/* Abbiamo trova la choice richiesta. Ricordiamoci qual e' nella
		variabile globale */
	nExecutingChoice = _ctx->i;

	while (1) {
		nExecutingChoice = _ctx->i;

		_ctx->k = 0;
		/* Calcoliamo le when expression di ciascun select, per vedere se sono
			attivi o disattivi */
		for (_ctx->j = 0; _ctx->dialog->Choice[_ctx->i].Select[_ctx->j].dwData != 0; _ctx->j++)
			if (_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].when == NULL) {
				_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].curActive = 1;
				_ctx->k++;
			} else if (EvaluateExpression(_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].when)) {
				_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].curActive = 1;
				_ctx->k++;
			} else
				_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].curActive = 0;

		/* Se non ci sono scelte attivate, la scelta e' finita */
		if (_ctx->k == 0) {
			UnlockDialogs();
			break;
		}

		/* Avvertiamo il gioco che c'e' una scelta da far fare all'utente,
			e restiamo in attesa della risposta */
		ResetEvent(hDoneChoice);
		SetEvent(hAskChoice);
		WaitForSingleObject(hDoneChoice, INFINITE);

		/* Ora che la scelta e' stata effettuata, possiamo eseguire _ctx->i gruppi
			associati con la scelta */
		_ctx->j = nSelectedChoice;
		for (_ctx->k = 0; _ctx->dialog->Choice[_ctx->i].Select[_ctx->j].wPlayGroup[_ctx->k] != 0; _ctx->k++) {
			_ctx->nGroup = _ctx->dialog->Choice[_ctx->i].Select[_ctx->j].wPlayGroup[_ctx->k];
			CORO_INVOKE_1(GroupThread, &_ctx->nGroup);
		}

		/* Controllo sugli attributi */
		if (_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].attr & (1 << 0)) {
			/* Bit 0 settato: fine della scelta */
			UnlockDialogs();
			break;
		}

		if (_ctx->dialog->Choice[_ctx->i].Select[_ctx->j].attr & (1 << 1)) {
			/* Bit 1 settato: fine del dialogo */
			UnlockDialogs();
			
			CORO_KILL_SELF();
			return;
		}

		/* Fine della scelta senza attributi: bisogna rifarla */
	}

	// If we're here, we found an end choice. Return to the caller group
	return;

	CORO_END_CODE;
}



/****************************************************************************\
*
* Function:     HANDLE DoAction(uint32 nAction, uint32 ordItem, uint32 dwParam);
*
* Description:  Esegue una azione su un certo item
*
* Input:        uint32 nAction           Numero dell'azione
*               uint32 ordItem           Indice dell'item nelle strutture
*                                       degli item
*               uint32 dwParam           Eventuale parametro per l'azione
*
* Return:       Handle del thread che sta eseguendo l'azione, oppure
*               INVALID_HANDLE_VALUE se l'azione non e' definita, o l'item
*               e' disattivato.
*
* Note:         Si puo' ottenere l'indice dell'item a partire dal suo numero
*               tramite la funzione itemGetOrderFromNum().
*               Gli item devono essere lockati, perche' questa funzione
*               funzioni, tramite LockItem();
*
\****************************************************************************/

static HANDLE DoAction(uint32 nAction, uint32 ordItem, uint32 dwParam) {
	LPMPALITEM item = lpmiItems;
	int i;
	LPMPALITEM newitem;
	uint32 h;

	item+=ordItem;
	Common::String buf = Common::String::format("Status.%u", item->nObj);
	if (varGetValue(buf.c_str()) <= 0)
		return INVALID_HANDLE_VALUE;

	for (i = 0; i < item->nActions; i++) {
		if (item->Action[i].num != nAction)
			continue;

		if (item->Action[i].wParm != dwParam)
			continue;

		if (item->Action[i].when != NULL) {
			if (!EvaluateExpression(item->Action[i].when))
				continue;
		}

		// Ora abbiamo trova l'azione giusta che deve essere eseguita.
		// Duplichiamo l'item corrente e copiamo la azione #i nella #0
		newitem = (LPMPALITEM)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALITEM));
		if (newitem == NULL)
			return INVALID_HANDLE_VALUE;

		// Nella nuova versione scriviamo il numero dell'azione in dwRes
		Common::copy((byte *)item, (byte *)item + sizeof(MPALITEM), (byte *)newitem);
/*   newitem->Action[0].nCmds=item->Action[i].nCmds;
   CopyMemory(newitem->Action[0].CmdNum,item->Action[i].CmdNum,newitem->Action[0].nCmds*sizeof(newitem->Action[0].CmdNum[0]));
*/
		newitem->dwRes = i;

		// E finalmente possiamo richiamare il thread, che eseguira' l'azione
		// 0 dell'item, e poi liberera' la memoria con la GlobalFree()

		// !!! New thread management
		if ((h = g_scheduler->createProcess(ActionThread, &newitem, sizeof(LPMPALITEM))) == NULL)
			return INVALID_HANDLE_VALUE;

		if ((h = g_scheduler->createProcess(ShutUpActionThread, &h, sizeof(uint32))) == NULL)
			return INVALID_HANDLE_VALUE;

/*
	 if ((h=(void*)_beginthread(ActionThread, 10240,(void *)newitem))==(void*)-1)
     return INVALID_HANDLE_VALUE;
		
	 if ((h=(void*)_beginthread(ShutUpActionThread, 10240,(void *)h))==(void*)-1)
     return INVALID_HANDLE_VALUE;

*/
		nExecutingAction = item->nObj;
		bExecutingAction = true;

		return (HANDLE)h;
	}

	return INVALID_HANDLE_VALUE;
}

/**
 * Shows a dialog in a separate process.
 *
 * @param nDlgOrd				The index of the dialog in the dialog list
 * @param nGroup				Number of the group to perform
 * @returns						The process Id of the process running the dialog
 *								or INVALID_HANDLE_VALUE on error
 * @remarks						The dialogue runs in a thread created on purpose, 
 * so that must inform through an event and when 'necessary to you make a choice. 
 * The data on the choices may be obtained through various queries.
 */
static uint32 DoDialog(uint32 nDlgOrd, uint32 nGroup) {
	uint32 h;

	// Store the running dialog in a global variable
	nExecutingDialog = nDlgOrd;

	// Enables the flag to indicate that there is' a running dialogue
	bExecutingDialog = true;

	ResetEvent(hAskChoice);
	ResetEvent(hDoneChoice);

	// Create a thread that performs the dialogue group

	// Create the process
	if ((h = _vm->_scheduler.createProcess(GroupThread, &nGroup, sizeof(uint32))) == 0)
		return 0;

	// Create a thread that waits until the end of the dialog process, and will restore the global variables
	if (_vm->_scheduler.createProcess(ShutUpDialogThread, &h, sizeof(uint32)) == 0) {
		// Something went wrong, so kill the previously started dialog process
		_vm->_scheduler.killMatchingProcess(h);
		return 0;
	}

	return h;
}


/****************************************************************************\
*
* Function:     bool DoSelection(uint32 nChoice, uint32 dwData);
*
* Description:  Prende nota del select scelto dall'utente, e avverte il
*               thread che stava eseguendo il dialogo che puo' continuare.
*
* Input:        uint32 nChoice           Numero della scelta che era in corso
*               uint32 dwData            Dato abbinato al select selezionato
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool DoSelection(uint32 i, uint32 dwData) {
	LPMPALDIALOG dialog=lpmdDialogs+nExecutingDialog;
	int j;

	for (j = 0; dialog->Choice[i].Select[j].dwData != 0; j++)
		if (dialog->Choice[i].Select[j].dwData == dwData && dialog->Choice[i].Select[j].curActive != 0)
			break;

	if (dialog->Choice[i].Select[j].dwData == 0)
		return false;

	nSelectedChoice = j;
	SetEvent(hDoneChoice);
	return true;
}



/****************************************************************************\
*       Exported functions
\****************************************************************************/

/****************************************************************************\
*
* Function:     bool mpalInit(LPSTR lpszMpcFileName, LPSTR lpszMprFileName,
*                 LPLPCUSTOMFUNCTION lplpcfArray);
*
* Description:  Inizializza la libreria MPAL, e apre un file .MPC, che
*               verra' utilizzato per tutte le query
*
* Input:        char * lpszMpcFileName   Nome del file .MPC, comprensivo di
*                                       estensione
*               char * lpszMprFileName   Nome del file .MPR, comprensivo di
*                                       estensione
*               LPLPCUSTOMFUNCTION
*                 lplpcfArray           Array di pointer a funzioni custom
*
* Return:       true se tutto OK, false in caso di errore
*
\****************************************************************************/

bool mpalInit(const char *lpszMpcFileName, const char *lpszMprFileName, 
			  LPLPCUSTOMFUNCTION lplpcfArray, Common::String *lpcfStrings) {
	Common::File hMpc;
	byte buf[5];
	uint32 nBytesRead;
	bool bCompress;
	uint32 dwSizeDecomp, dwSizeComp;
	byte *cmpbuf;

 //printf("Item: %lu\n", sizeof(MPALITEM));
 //printf("Script: %lu\n", sizeof(MPALSCRIPT));
 //printf("Dialog: %lu\n", sizeof(MPALDIALOG));

	/* Si salva l'array delle funzioni custom */
	lplpFunctions = lplpcfArray;
	lplpFunctionStrings = lpcfStrings;

	/* Apre il file MPC in lettura */
	if (!hMpc.open(lpszMpcFileName))
		return false;

	/* Legge e controlla l'header */
	nBytesRead = hMpc.read(buf, 5);
	if (nBytesRead != 5)
		return false;

	if (buf[0] != 'M' || buf[1] != 'P' || buf[2] != 'C' || buf[3] != 0x20)
		return false;

	bCompress = buf[4];

	/* Legge la dimensione del file decompresso, e alloca la memoria */
	dwSizeDecomp = hMpc.readUint32LE();
	if (hMpc.err())
		return false;

	lpMpcImage = (byte *)GlobalAlloc(GMEM_FIXED,dwSizeDecomp+16);
	if (lpMpcImage == NULL)
		return false;

	if (bCompress) {
		/* Se il file e' compresso, guarda quanto e' grande e alloca la
			memoria temporanea per la decompressione */
		dwSizeComp = hMpc.readUint32LE();
		if (hMpc.err())
			return false;

		cmpbuf = (byte *)GlobalAlloc(GMEM_FIXED,dwSizeComp);
		if (cmpbuf == NULL)
			return false;

		nBytesRead = hMpc.read(cmpbuf, dwSizeComp);
		if (nBytesRead != dwSizeComp)
			return false;

		/* Decomprime l'immagine */
		lzo1x_decompress(cmpbuf, dwSizeComp, lpMpcImage, &nBytesRead);
		if (nBytesRead != dwSizeDecomp)
			return false;

		GlobalFree(cmpbuf);
	} else {
		/* Se il file non e' compresso, lo legge all'interno della memoria gia'
			allocata */
		nBytesRead = hMpc.read(lpMpcImage, dwSizeDecomp);
		if (nBytesRead != dwSizeDecomp)
			return false;
	}

	/* Chiude il file */
	hMpc.close();

	/* Parsa l'immagine */
	if (ParseMpc(lpMpcImage) == false)
		return false;

	GlobalFree(lpMpcImage);

	/* Calcola utilizzo di memoria */
	/*
 {
	 char errbuf[256];
	 wsprintf(errbuf,"Utilizzo in RAM: VAR %lu, MSG %lu, DLG %lu, ITM %lu, LOC %lu, SCR %lu",
	   nVars*sizeof(MPALVAR),
		 nMsgs*sizeof(MPALMSG),
		 nDialogs*sizeof(MPALDIALOG),
		 nItems*sizeof(MPALITEM),
		 nLocations*sizeof(MPALLOCATION),
		 nScripts*sizeof(MPALSCRIPT));
	 MessageBox(NULL,errbuf,"Dump",MB_OK);
 }
*/

	/* Apre il file MPR in lettura */
	if (!hMpr.open(lpszMprFileName))
		return false;

	/* Si posiziona a 8 byte dalla fine del file */
	hMpr.seek(-12, SEEK_END);

	dwSizeComp = hMpr.readUint32LE();
	if (hMpr.err())
		return false;

	nResources = hMpr.readUint32LE();
	if (hMpr.err())
		return false;

	nBytesRead = hMpr.read(buf, 4);
	if (hMpr.err())
		return false;

	if (buf[0] !='E' || buf[1] != 'N' || buf[2] != 'D' || buf[3] != '0')
		return false;

	/* Si posiziona all'inizio dell'header delle risorse */
	hMpr.seek(-(12 + (int)dwSizeComp), SEEK_END);

	lpResources = (uint32 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nResources * 8);
	if (lpResources == NULL)
		return false;

	cmpbuf = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSizeComp);
	if (cmpbuf == NULL)
		return false;

	nBytesRead = hMpr.read(cmpbuf, dwSizeComp);
	if (nBytesRead != dwSizeComp)
		return false;

	lzo1x_decompress((const byte *)cmpbuf, dwSizeComp, (byte *)lpResources, (uint32 *)&nBytesRead);
	if (nBytesRead != (uint32)nResources*8)
		return false;

	GlobalFree(cmpbuf);

	/* Si riposiziona all'inizio lasciando il file di risorse aperto */
	hMpr.seek(0, SEEK_SET);

	/* Non c'e' nessuna azione ne' dialogo in esecuzione */
	bExecutingAction = false;
	bExecutingDialog = false;

	/* Non c'e' nessuna locazione in polling */
	Common::fill(nPollingLocations, nPollingLocations + MAXPOLLINGLOCATIONS, 0);

	/* Crea l'evento che verra' utilizzato per avvertire il gioco che c'e'
		da effettuare una scelta */
	hAskChoice = CreateEvent(NULL, true, false, NULL);
	hDoneChoice = CreateEvent(NULL, true, false, NULL);

	return true;
}

/****************************************************************************\
*
* Function:     uint32 mpalQuery(uint16 wQueryType, ...);
*
* Description:  Questa e' la funzione generale per comunicare con la libreria,
*               per richiedere informazioni riguardo a quanto si trova nel
*               file .MPC
*
* Input:        uint16 wQueryType         Tipo di query. La lista e' in
*                                       enum QueryTypes
*
* Return:       4 bytes che dipendono dal tipo di query
*
* Note:         E' _FORTEMENTE_ consigliato utilizzare le macro
*               definite sopra per utilizzare le query, dato che
*               permettono di evitare spiacevoli bug dovuti a dimenticanze
*               di parametri.
*
\****************************************************************************/

#define GETARG(type)   va_arg(v,type)

void mpalQueryInner(CORO_PARAM, uint16 wQueryType, uint32 *dwRet, va_list v) {
	CORO_BEGIN_CONTEXT;
		int x, y, z;
		char *n;
		Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	mpalError = OK;

	if (wQueryType == MPQ_VERSION) {
		/*
		 *  uint32 mpalQuery(MPQ_VERSION);
		 */
		*dwRet = HEX_VERSION;

	} else if (wQueryType == MPQ_GLOBAL_VAR) {
		/*
		 *  uint32 mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName);
		 */
		LockVar();
		*dwRet = (uint32)varGetValue(GETARG(char *));
		UnlockVar();

	} else if (wQueryType == MPQ_MESSAGE) {
		/*
		 *  char * mpalQuery(MPQ_MESSAGE, uint32 nMsg);
		 */
		LockMsg();
		*dwRet = (uint32)DuplicateMessage(msgGetOrderFromNum(GETARG(uint32)));
		UnlockMsg();
		
	} else if (wQueryType == MPQ_ITEM_PATTERN) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem);
		 */
		LockVar();
		_ctx->buf = Common::String::format("Pattern.%u", GETARG(uint32));
		*dwRet = (uint32)varGetValue(_ctx->buf.c_str());
		UnlockVar();
		
	} else if (wQueryType == MPQ_LOCATION_SIZE) {
		/*
		 *  uint32 mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord);
		 */
		LockLocations();
		_ctx->x = locGetOrderFromNum(GETARG(uint32));
		_ctx->y = GETARG(uint32);
		if (_ctx->x != -1) {
			if (_ctx->y == MPQ_X)
				*dwRet = lpmlLocations[_ctx->x].dwXlen;
			else if (_ctx->y == MPQ_Y)
				*dwRet = lpmlLocations[_ctx->x].dwYlen;
			else
				mpalError = 1;
		} else
			mpalError = 1;
		UnlockLocations();
		
	} else if (wQueryType == MPQ_LOCATION_IMAGE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc);
		 */
		LockLocations();
		_ctx->x = locGetOrderFromNum(GETARG(uint32));
		*dwRet = (uint32)resLoad(lpmlLocations[_ctx->x].dwPicRes);
		UnlockLocations();

	} else if (wQueryType == MPQ_RESOURCE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_RESOURCE, uint32 dwRes);
		 */
		*dwRet = (uint32)resLoad(GETARG(uint32));

	} else if (wQueryType == MPQ_ITEM_LIST) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_LIST, uint32 nLoc);
		 */
		LockVar();
		*dwRet = (uint32)GetItemList(GETARG(uint32));
		LockVar();

	} else if (wQueryType == MPQ_ITEM_DATA) {
		/*
		 *  LPITEM mpalQuery(MPQ_ITEM_DATA, uint32 nItem);
		 */
		LockItems();
		*dwRet = (uint32)GetItemData(itemGetOrderFromNum(GETARG(uint32)));
		UnlockItems();

	} else if (wQueryType == MPQ_ITEM_IS_ACTIVE) {
		/*
		 *  bool mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem);
		 */
		LockVar();
		_ctx->x = GETARG(uint32);
		_ctx->buf = Common::String::format("Status.%u", _ctx->x);
		if (varGetValue(_ctx->buf.c_str()) <= 0)
			*dwRet = (uint32)false;
		else
			*dwRet = (uint32)true;
		UnlockVar();

	} else if (wQueryType == MPQ_ITEM_NAME) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_NAME, uint32 nItem, char * lpszName);
		 */
		LockVar();
		_ctx->x = GETARG(uint32);
		_ctx->n = GETARG(char *);
		_ctx->buf = Common::String::format("Status.%u", _ctx->x);
		if (varGetValue(_ctx->buf.c_str()) <= 0)
			_ctx->n[0]='\0';
		else {
			LockItems();
			_ctx->y = itemGetOrderFromNum(_ctx->x);
			CopyMemory(_ctx->n, (char *)(lpmiItems+_ctx->y)->lpszDescribe, MAX_DESCRIBE_SIZE);
			UnlockItems();
		}

		UnlockVar();

	} else if (wQueryType == MPQ_DIALOG_PERIOD) {
		/*
		 *  char * mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod);
		 */
		LockDialogs();
		_ctx->y = GETARG(uint32);
		*dwRet = (uint32)DuplicateDialogPeriod(_ctx->y);
		UnlockDialogs();

	} else if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		WaitForSingleObject(hAskChoice, INFINITE);
		ResetEvent(hAskChoice);

		if (bExecutingDialog)
			*dwRet = (uint32)nExecutingChoice;
		else
			*dwRet = (uint32)((int)-1);

	} else if (wQueryType == MPQ_DIALOG_SELECTLIST) {
		/*
		 *  uint32 *mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice);
		 */
		LockDialogs();
		*dwRet = (uint32)GetSelectList(GETARG(uint32));
		UnlockDialogs();
		break;

	} else if (wQueryType == MPQ_DIALOG_SELECTION) {
		/*
		 *  bool mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData);
		 */
		LockDialogs();
		_ctx->x = GETARG(uint32);
		_ctx->y = GETARG(uint32);
		*dwRet = (uint32)DoSelection(_ctx->x,_ctx->y);
		UnlockDialogs();

	} else if (wQueryType == MPQ_DO_ACTION) {
		/*
		 *  int mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam);
		 */
		LockItems();
		LockVar();
		_ctx->x = GETARG(uint32);
		_ctx->z = GETARG(uint32);
		_ctx->y = itemGetOrderFromNum(_ctx->z);
		if (_ctx->y!=-1) {
			*dwRet = (uint32)DoAction(_ctx->x, _ctx->y, GETARG(uint32));
		} else {
			*dwRet = (uint32)INVALID_HANDLE_VALUE;
			mpalError = 1;
		}
		UnlockVar();
		UnlockItems();

	} else if (wQueryType == MPQ_DO_DIALOG) {
		/*
		 *  int mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup);
		 */
		if (!bExecutingDialog) {
			LockDialogs();

			_ctx->x = dialogGetOrderFromNum(GETARG(uint32));
			_ctx->y = GETARG(uint32);
			*dwRet = DoDialog(_ctx->x, _ctx->y);
			UnlockDialogs();
		}
	} else {
		/*
		 *  DEFAULT -> ERROR
		 */
		mpalError = 1;
	}

	CORO_END_CODE;
}

uint32 mpalQuery(uint16 wQueryType, ...) {
	uint32 dwRet;
	va_list v;
	va_start(v, wQueryType);

	mpalQueryInner(nullContext, wQueryType, &dwRet, v);

	va_end(v);

	return dwRet;
}

void mpalQueryCoro(CORO_PARAM, uint32 *dwRet, uint16 wQueryType, ...) {
	va_list v;
	va_start(v, wQueryType);

	mpalQueryInner(coroParam, wQueryType, dwRet, v);

	va_end(v);
}


/****************************************************************************\
*
* Function:     uint32 mpalGetError(void);
*
* Description:  Ritorna il codice di errore corrente di MPAL
*
* Return:       Codice di errore
*
\****************************************************************************/

uint32 mpalGetError(void) {
	return mpalError;
}


/****************************************************************************\
*
* Function:     void mpalExecuteScript(int nScript);
*
* Description:  Esegue uno script. Lo script viene eseguito in multitasking
*               tramite un thread.
*
* Input:        int nScript             Numero dello script da eseguire
*
* Return:       true se lo script e' stato avviato, false in caso di errore
*
\****************************************************************************/

bool EXPORT mpalExecuteScript(int nScript) {
	int n;
	LPMPALSCRIPT s;
	uint32 dwId;

	LockScripts();
	n = scriptGetOrderFromNum(nScript);
	s = (LPMPALSCRIPT)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALSCRIPT));
	if (s == NULL)
		return false;

	CopyMemory(s, lpmsScripts+n, sizeof(MPALSCRIPT));
	UnlockScripts();

// !!! Nuova gestione dei thread
	if (CreateThread(NULL, 10240,(LPTHREAD_START_ROUTINE)ScriptThread,(void *)s, 0, &dwId) == NULL)
 //if ((void*)_beginthread(ScriptThread, 10240,(void *)s)==(void*)-1)
		return false;

	return true;
}


/****************************************************************************\
*
* Function:     void mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCustom);
*
* Description:  Install a custom routine that will be called by MPAL every
*               time the pattern of an item has been changed.
*
* Input:        LPITEMIRQFUNCTION lpiifCustom   Custom function to install
*
\****************************************************************************/

void EXPORT mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCus) {
	lpiifCustom = lpiifCus;
}


/****************************************************************************\
*
* Function:     bool mpalStartIdlePoll(int nLoc);
*
* Description:  Process the idle actions of the items on one location.
*
* Input:        int nLoc                Number of the location whose items
*                                       must be processed for idle actions.
*
* Return:       true se tutto OK, false se si e' superato il limite massimo.
*
* Note:         Il numero massimo delle locazione che e' possibile pollare
*               contemporaneamente e' contenuto nel define MAXPOLLINGFUNCIONS
*
\****************************************************************************/

bool mpalStartIdlePoll(int nLoc) {
	uint32 i;

	for (i = 0; i < MAXPOLLINGLOCATIONS; i++)
		if (nPollingLocations[i] == (uint32)nLoc)
			return false;

	for (i = 0; i < MAXPOLLINGLOCATIONS; i++) {
		if (nPollingLocations[i] == 0) {
			nPollingLocations[i]=nLoc;

			hEndPollingLocations[i] = CreateEvent(NULL, true, false, NULL);
// !!! Nuova gestione dei thread
			if ((PollingThreads[i] = _vm->_scheduler.createProcess(LocationPollThread, &i, sizeof(uint32))) == 0)
//			 if ((hEndPollingLocations[i]=(void*)_beginthread(LocationPollThread, 10240,(void *)i))==(void*)-1)
				return false;

			return true;
		}
	}

	return false;
}



/****************************************************************************\
*
* Function:     bool mpalEndIdlePoll(int nLoc);
*
* Description:  Stop processing the idle actions of the items on one location.
*
* Input:        int nLoc                Number of the location
*
* Return:       true se tutto OK, false se la locazione specificata non era
*               in fase di polling
*
\****************************************************************************/

void mpalEndIdlePoll(CORO_PARAM, int nLoc, bool *result) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (_ctx->i = 0; _ctx->i < MAXPOLLINGLOCATIONS; _ctx->i++) {
		if (nPollingLocations[_ctx->i] == (uint32)nLoc) {
			SetEvent(hEndPollingLocations[_ctx->i]);

			CORO_INVOKE_2(_vm->_scheduler.waitForSingleObject, PollingThreads[_ctx->i], INFINITE);

			CloseHandle(hEndPollingLocations[_ctx->i]);
			nPollingLocations[_ctx->i] = 0;

			if (result)
				*result = true;
			return;
		}
	}

	if (result)
		*result = false;

	CORO_END_CODE;
}



/****************************************************************************\
*
* Function:     int mpalGetSaveStateSize(void);
*
* Description:  Acquire the length of a save state
*
* Return:       Length in bytes
*
\****************************************************************************/

int mpalGetSaveStateSize(void) {
	return nVars * sizeof(MPALVAR) + 4;
}


/****************************************************************************\
*
* Function:     void mpalSaveState(byte *buf);
*
* Description:  Store the save state into a buffer. The buffer must be
*								length at least the size specified with mpalGetSaveStateSize
*
* Input:				byte *buf							Buffer where to store the state
*
\****************************************************************************/

void mpalSaveState(byte *buf) {
	LockVar();
	WRITE_LE_UINT32(buf, nVars);
	CopyMemory(buf + 4, (byte *)lpmvVars, nVars * sizeof(MPALVAR));
	UnlockVar();	
}

/****************************************************************************\
*
* Function:     int mpalLoadState(byte *buf);
*
* Description:  Load a save state from a buffer. 
*
* Input:				byte *buf							Buffer where to store the state
*
*	Return:				Length of the state in bytes
*
\****************************************************************************/

int mpalLoadState(byte *buf) {
	// Dobbiamo distruggere tutte le variabili e ricrearle
	GlobalFree(hVars);

	nVars = READ_LE_UINT32(buf);
	
	hVars = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, nVars * sizeof(MPALVAR));
	LockVar();
	CopyMemory((byte *)lpmvVars, buf + 4, nVars * sizeof(MPALVAR));
	UnlockVar();

	return nVars*sizeof(MPALVAR)+4;
}

bool bDontOutput;

struct MsgCommentsStruct {
	uint16 wStart;
	uint16 wEnd;
	const char *pComment;	
};
const MsgCommentsStruct MsgComments[] = {
	{ 10, 16, "###" },
	{ 560, 563, "@@@ BUTCH & DUDLEY:" },
	{ 551, 553, "@@@ JACK'S LETTER (JACK'S VOICE):" },
	{ 679, 679, "@@@ OFF-SCREEN VOICE:" },
	{ 799, 799, "###" },
	{ 830, 838, "RE-HASHING (FROM MACBETH):" },
	{ 890, 894, "@@@ BEARDED LADY FROM WITHIN HER ROOM:" },
	{ 1175, 1175, "###" },
	{ 1210, 1210, "###" },
	{ 1347, 1349, "###" },
	{ 1175, 1175, "###" },
	{ 1342, 1343, "###" },
	{ 1742, 1742, "@@@ OFF-SCREEN VOICE:" },
	{ 1749, 1749, "###" },
	{ 1759, 1759, "###" },
	{ 2165, 2166, "@@@ MORTIMER:" },
	{ 2370, 2372, "@@@ ELECTRONIC VOICE FROM AN AUTOMATIC PICTURE MACHINE:" },
	{ 2580, 2589, "@@@ BIFF:" },
	{ 2590, 2593, "@@@ BARTENDER:" },
	{ 2596, 2596, "@@@ SAD PIRATE:" },
	{ 2760, 2767, "@@@ EGGHEAD:" },
	{ 2959, 2959, "@@@ MONSTROUS VOICE FROM BEHIND A LOCKED DOOR:" },
	{ 3352, 3352, "@@@ POLLY:" },
	{ 3378, 3379, "@@@ POLLY:" },
	{ 3461, 3469, "@@@ RANDALL:" },
	{ 3571, 3574, "@@@ CAPTAIN'S JOURNAL (CAPTAIN'S VOICE):" },
	{ 3646, 3646, "NOTE: THIS SENTENCE ENDS THE STORY TOLD IN SENTENCES 03640 - 03643:" },
	{ 3647, 3648, "TONY SPEAKS TRYING TO IMITATE CAPTAIN CORNELIUS' VOICE:" },
	{ 3670, 3671, "###" },
	{ 3652, 3652, "###" },
	{ 3656, 3657, "@@@ GATEKEEPER:" },
	{ 3658, 3659, "@@@ GATEKEEPER (FAR AWAY):" },
	{ 3790, 3795, "@@@ GATEKEEPER:" },
	{ 3798, 3799, "@@@ OFF-SCREEN VOICE:" },
	{ 4384, 4384, "###" },
	{ 4394, 4395, "###" },
	{ 4780, 4780, "###" },
	{ 5089, 5089, "TONY PLAYING SOMEONE ELSE, WITH A DEEPER TONE:" },
	{ 5090, 5090, "NORMAL TONY:" },
	{ 5091, 5091, "TONY PLAYING SOMEONE ELSE, WITH A DEEPER TONE:" },
	{ 5262, 5262, "@@@ OFF-SCREEN VOICE" },
	{ 5276, 5277, "###" },
	{ 5326, 5326, "###" },
	{ 5472, 5472, "LYRICS FROM THE SONG \"I AM ONE\", BY SMASHING PUMPKINS:" },
	{ 5488, 5488, "LYRICS FROM THE SONG \"I AM ONE\", BY SMASHING PUMPKINS:" },
	{ 5652, 5653, "###" },
//bernie	{ 11000, 15000, "###" },
	{ 11000, 11111, "###" },


	{ 0, 0, NULL }
};

void OutputStartMsgComment(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wStart == wNum) {
			warning("Start: %d\n", wNum);

			f->writeString("</TABLE>\n<P>\n<P>\n");

			if (strcmp(MsgComments[i].pComment, "###") != 0 && strncmp(MsgComments[i].pComment, "@@@", 3) != 0) {
				f->writeString(Common::String::format("%s\n", MsgComments[i].pComment));
				f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
			} else
				bDontOutput = true;
			return;
		}
}

void OutputEndMsgComment(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wEnd != 0; i++)
		if (MsgComments[i].wEnd == wNum) {
warning("End: %d\n", wNum);

			if (strcmp(MsgComments[i].pComment, "###") != 0 && strncmp(MsgComments[i].pComment, "@@@", 3) != 0) {
				f->writeString("</TABLE>\n<P>\n");
			} else
				bDontOutput = false;
	
			f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
			return;
		}
}


int OutputStartOther(uint16 wNum, Common::OutSaveFile *f) {
	int i;
	
	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wStart <= wNum && MsgComments[i].wEnd >= wNum) {
			if (strncmp(MsgComments[i].pComment, "@@@", 3) == 0) {
				if (MsgComments[i].wStart == wNum) {
					f->writeString(Common::String::format("%s\n", MsgComments[i].pComment + 4));
					f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
				}
				
				return 1;				
			}
		}

	return 0;
}


void OutputEndOther(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wEnd == wNum && strncmp(MsgComments[i].pComment, "@@@", 3) == 0) {
			f->writeString("</TABLE>\n<P>\n");	
			break;
		}
}


void mpalDumpMessages(void) {
	int i, j;
	char *lpMessage;
	char *p;
	char *lpPeriods[30];
	char fname[64];
	char frase[2048];
	int nPeriods;
	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");

	LockMsg();

	bDontOutput = false;

	warning("Dumping MESSAGES.HTM...\n");
	
	f = g_system->getSavefileManager()->openForSaving("Messages.htm");
	f->writeString("<HTML>\n<BODY>\n<TABLE WIDTH = 100%% BORDER = 1>\n");

	for (i = 0; i < nMsgs; i++) {
		lpMessage = (char*)GlobalLock(lpmmMsgs[i].hText);
		if (*lpMessage != '\0') {
			// bernie: debug
			/*if (lpmmMsgs[i].wNum == 1950) {
				int a = 1;
			}*/

			nPeriods = 1;
			p=lpPeriods[0] = lpMessage;

			OutputStartMsgComment(lpmmMsgs[i].wNum, f);

			while (1) {
				// Trova la fine del periodo corrente
				while (*p != '\0')
					p++;

				// Se c'e' un altro '\0' siamo alla fine del messaggio
				p++;
				if (*p == '\0')
					break;

				// Altrimenti c'e' un altro periodo, e ci ricordiamo il suo inizio
				lpPeriods[nPeriods++] = p;
			}

			// Ora fa un ciclo su tutti i periodi
			for (j = 0;j<nPeriods; j++) {
				if (nPeriods == 1)
					sprintf(fname, "000-%05d.WAV", lpmmMsgs[i].wNum);
				else
					sprintf(fname, "000-%05d-%02d.WAV", lpmmMsgs[i].wNum,j);
			
				strcpy(frase, lpPeriods[j]);

				while ((p = strchr(frase,'^')) != NULL)
					*p = '\"';

				p = frase;
				while (*p == ' ') p++;
				if (*p == '\0')
					continue;

				if (!bDontOutput) {
					v1->writeString(Common::String::format("%s\n", fname));
					f->writeString("\t<TR>\n");
					f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </B></TD>\n", fname));
					f->writeString(Common::String::format("\t\t<TD> %s </TD>\n", frase));
					f->writeString("\t</TR>\n");
				}
			}

			OutputEndMsgComment(lpmmMsgs[i].wNum, f);

			GlobalUnlock(lpmmMsgs[i].hText);
		}
	}

	f->writeString("</TABLE>\n</BODY>\n</HTML>\n");

	f->finalize();
	v1->finalize();
	delete f;
	delete v1;

	UnlockMsg();
}



void mpalDumpOthers(void) {
	int i,j;
	char *lpMessage;
	char *p;
	char *lpPeriods[30];
	char fname[64];
	char frase[2048];
	int nPeriods;

	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");
	f = g_system->getSavefileManager()->openForSaving("Others.htm");
	LockMsg();
	
	bDontOutput = false;
	
	warning("Dumping OTHERS.HTM...\n");

	f->writeString("<HTML>\n<BODY>\n");
	
	for (i = 0; i < nMsgs; i++) {
		lpMessage = (char*)GlobalLock(lpmmMsgs[i].hText);
		if (*lpMessage != '\0') {
			nPeriods = 1;
			p=lpPeriods[0] = lpMessage;
			
			if (OutputStartOther(lpmmMsgs[i].wNum, f)) {	
				while (1) {
					// Trova la fine del periodo corrente
					while (*p!='\0')
						p++;
					
					// Se c'e' un altro '\0' siamo alla fine del messaggio
					p++;
					if (*p == '\0')
						break;
					
					// Altrimenti c'e' un altro periodo, e ci ricordiamo il suo inizio
					lpPeriods[nPeriods++] = p;
				}
				
				// Ora fa un ciclo su tutti i periodi
				for (j = 0; j < nPeriods; j++) {
					if (nPeriods == 1)
						sprintf(fname, "000-%05d.WAV", lpmmMsgs[i].wNum);
					else
						sprintf(fname, "000-%05d-%02d.WAV", lpmmMsgs[i].wNum,j);				

					strcpy(frase,lpPeriods[j]);
					
					while ((p = strchr(frase,'^')) != NULL)
						*p = '\"';

					p = frase;
					while (*p == ' ') p++;
					if (*p == '\0')
						continue;		
					
					if (!bDontOutput) {
						v1->writeString(Common::String::format("%s\n", fname));
						f->writeString("\t<TR>\n");
						f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </B></TD>\n", fname));
						f->writeString(Common::String::format("\t\t<TD> %s </TD>\n", frase));
						f->writeString("\t</TR>\n");
					}
				}
			}

			OutputEndOther(lpmmMsgs[i].wNum, f);
			
			GlobalUnlock(lpmmMsgs[i].hText);
		}
	}
	
	f->writeString("</BODY>\n</HTML>\n");
	
	f->finalize();
	v1->finalize();

	delete f;
	delete v1;
	UnlockMsg();
}


#if 0 // English names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Old lady 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Convict", NULL, NULL, "Jack (with microphone)", "Old lady 1", NULL };
const char *DLG90[] = { "Tony", "Bearded lady", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piranha", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Beast", NULL };
const char *DLG190[] = { "Tony", "Beast", NULL };
const char *DLG191[] = { "Tony", "Beast", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Bartender", "Sad pirate", "Anchorman", "Biff", NULL };
const char *DLG251[] = { "Tony", "Bartender", "Sad pirate", "Anchorman", "Biff", NULL };
const char *DLG260[] = { "Tony", "Captain", "Captain (tale)", NULL };
const char *DLG270[] = { "Tony", "Egghead", NULL };
const char *DLG271[] = { "Tony", "Egghead", NULL };
const char *DLG272[] = { "Tony", "Egghead", NULL };
const char *DLG290[] = { "Tony", "Old lady 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Captain (off scene)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Gatekeeper", NULL };
const char *DLG371[] = { "Tony", "Gatekeeper", NULL };
const char *DLG372[] = { "Tony", "Gatekeeper", NULL };
const char *DLG373[] = { "Tony", "Gatekeeper", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Mr. Wishing Well", "Tony (from the top of the well)", NULL  };
const char *DLG560[] = { "Tony", "Superintendent", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Storyteller", "Mr. Wishing Well", NULL };
#endif

#if 0 // Polish names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Staruszka 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Skazaniec", NULL, NULL, "£ebster (przez mikrofon)", "Staruszka 1", NULL };
const char *DLG90[] = { "Tony", "Kobieta z Brod¹", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Pirania", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG160[] = { "Tony", "Œmiechozol", NULL };
const char *DLG161[] = { "Tony", "Œmiechozol", NULL };
const char *DLG162[] = { "Tony", "Œmiechozol", NULL };
const char *DLG163[] = { "Tony", "Œmiechozol", NULL };
const char *DLG180[] = { "Tony", "Wycz", NULL };
const char *DLG190[] = { "Tony", "Wycz", NULL };
const char *DLG191[] = { "Tony", "Wycz", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG211[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG212[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barman", "Smutny Pirat", "Wodzirej", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barman", "Smutny Pirat", "Wodzirej", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitan", "Captain (opowieœæ)", NULL };
const char *DLG270[] = { "Tony", "Jajog³owy", NULL };
const char *DLG271[] = { "Tony", "Jajog³owy", NULL };
const char *DLG272[] = { "Tony", "Jajog³owy", NULL };
const char *DLG290[] = { "Tony", "Staruszka 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Kapitan (zza sceny)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Stra¿nik", NULL };
const char *DLG371[] = { "Tony", "Stra¿nik", NULL };
const char *DLG372[] = { "Tony", "Stra¿nik", NULL };
const char *DLG373[] = { "Tony", "Stra¿nik", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Gospodyni (zza sceny)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Pan Studnia ¯yczeñ", "Tony (nad studni¹)", NULL  };
const char *DLG560[] = { "Tony", "Inspektor", NULL };
const char *DLG590[] = { "Tony", "Pantaloniarz", NULL };
const char *DLG600[] = { "Tony", "£ebster", "£ebster", NULL, "£ebster", NULL, NULL, NULL, "Narrator", "Pan Studnia ¯yczeñ", NULL };
#endif // Polish


#if 0 // Russian
const char *DLG10[] = { "Òîíè", NULL };
const char *DLG51[] = { "Òîíè", "Áó÷", "Äàäëè" };
const char *DLG52[] = { "Òîíè", NULL };
const char *DLG61[] = { "Òîíè", "Ñòàðóøêà 1", NULL };
const char *DLG71[] = { "Òîíè", "Òèìîòè", "Îñóæäåííûé", NULL, NULL, "Äæåê (ñ ìèêðîôîíîì)", "Ñòàðóøêà 1", NULL };
const char *DLG90[] = { "Òîíè", "Áîðîäàòàÿ æåíùèíà", NULL };
const char *DLG110[] = { "Òîíè", "Ëîðåíö", NULL };
const char *DLG111[] = { "Òîíè", "Ëîðåíö", NULL };
const char *DLG130[] = { "Òîíè", "Ïèðàíüÿ", NULL };
const char *DLG150[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG151[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG152[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG153[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG154[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG160[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG161[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG162[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG163[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG180[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG190[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG191[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG201[] = { "Òîíè", NULL };
const char *DLG210[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG211[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG212[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG240[] = { "Òîíè", "Èçàáåëëà", NULL };
const char *DLG250[] = { "Òîíè", "Áàðìåí", "Ãðóñòíûé ïèðàò", "Âåäóùèé", "Áèôô", NULL };
const char *DLG251[] = { "Òîíè", "Áàðìåí", "Ãðóñòíûé ïèðàò", "Âåäóùèé", "Áèôô", NULL };
const char *DLG260[] = { "Òîíè", "Êàïèòàí", "Êàïèòàí (ðàññêàç)", NULL };
const char *DLG270[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG271[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG272[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG290[] = { "Òîíè", "Ñòàðóøêà 2", NULL };
const char *DLG310[] = { "Òîíè", "Óîëëè", NULL };
const char *DLG330[] = { "Òîíè", "Ïîëëè", "Êàïèòàí (çà ñöåíîé)", NULL };
const char *DLG340[] = { "Òîíè", "Ðýíäàë", NULL };
const char *DLG360[] = { "Òîíè", NULL };
const char *DLG361[] = { "Òîíè", NULL };
const char *DLG370[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG371[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG372[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG373[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG380[] = { "Òîíè", NULL };
const char *DLG410[] = { "Òîíè", "Ãâåíäåëü", NULL };
const char *DLG430[] = { "Òîíè", "Ãàðîëüä", "×àê", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Òîíè", NULL };
const char *DLG470[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Òîíè", "Pin-up", NULL };
const char *DLG490[] = { "Òîíè", "Ãâåíäåëü", NULL };	
const char *DLG530[] = { "Òîíè", "Ãàðîëüä", "×àê", NULL };
const char *DLG550[] = { "Òîíè", "Ãîñïîäèí Êîëîäåö æåëàíèé", "Òîíè (ñ âåðøèíû êîëîäöà)", NULL  };
const char *DLG560[] = { "Òîíè", "Íà÷àëüíèê îõðàíû", NULL };
const char *DLG590[] = { "Òîíè", "Ïàíòàãðþýëü", NULL };
const char *DLG600[] = { "Òîíè", "Äæåê", "Äæåê", NULL, "Äæåê", NULL, NULL, NULL, "Ðàññêàç÷èê", "Ãîñïîäèí Êîëîäåö æåëàíèé", NULL };
#endif // Russian


#if 0 // Czech names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Stará paní 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Trestanec", NULL, NULL, "Jack (s mikrofonem)", "Stará paní 1", NULL };
const char *DLG90[] = { "Tony", "Vousatá žena", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piraòa", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Zvíøe", NULL };
const char *DLG190[] = { "Tony", "Zvíøe", NULL };
const char *DLG191[] = { "Tony", "Zvíøe", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barman", "Smutný pirát", "Moderátor", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barman", "Smutný pirát", "Moderátor", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitán", "Kapitán (pøíbìh)", NULL };
const char *DLG270[] = { "Tony", "Intelektuál", NULL };
const char *DLG271[] = { "Tony", "Intelektuál", NULL };
const char *DLG272[] = { "Tony", "Intelektuál", NULL };
const char *DLG290[] = { "Tony", "Stará paní 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Lóra", "Kapitán (mimo scénu)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Strážný", NULL };
const char *DLG371[] = { "Tony", "Strážný", NULL };
const char *DLG372[] = { "Tony", "Strážný", NULL };
const char *DLG373[] = { "Tony", "Strážný", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Pan Studna pøání", "Tony (z vrcholu studny)", NULL  };
const char *DLG560[] = { "Tony", "Správce", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Vypravìè", "Pan Studna pøání", NULL };
#endif // Czech names

#if 1 // Deutsch names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Alte Dame 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Sträfling", NULL, NULL, "Jack (mit Mikrofon)", "Alte Dame 1", NULL };
const char *DLG90[] = { "Tony", "Bärtige Dame", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piranha", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Biest", NULL };
const char *DLG190[] = { "Tony", "Biest", NULL };
const char *DLG191[] = { "Tony", "Biest", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barmann", "Trauriger Pirat", "Chefanimateur", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barmann", "Trauriger Pirat", "Chefanimateur", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitän", "Kapitän (Erzählung)", NULL };
const char *DLG270[] = { "Tony", "Eierkopf", NULL };
const char *DLG271[] = { "Tony", "Eierkopf", NULL };
const char *DLG272[] = { "Tony", "Eierkopf", NULL };
const char *DLG290[] = { "Tony", "Alte Dame 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Kapitän (im Off)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Pförtner", NULL };
const char *DLG371[] = { "Tony", "Pförtner", NULL };
const char *DLG372[] = { "Tony", "Pförtner", NULL };
const char *DLG373[] = { "Tony", "Pförtner", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Herr Wunschbrunnen", "Tony (über dem Brunnen)", NULL  };
const char *DLG560[] = { "Tony", "Verwalter", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Erzähler", "Herr Wunschbrunnen", NULL };
#endif


#define HANDLE_DIALOG(num)	\
case num:	\
	if (nPers >= (int)(sizeof(DLG##num) / sizeof(const char *)) || DLG##num[nPers] == NULL)	\
	{	\
		warning("ERROR: Il personaggio #%d non esiste nel dialogo %d!\n", nPers, nDlg);	\
		return "ERROR";	\
	}	\
	else	\
		return DLG##num[nPers];


const char *GetPersonName(uint16 nDlg, int nPers) {
	switch (nDlg) {
	HANDLE_DIALOG(10);
	HANDLE_DIALOG(51);
	HANDLE_DIALOG(52);
	HANDLE_DIALOG(61);
	HANDLE_DIALOG(71);
	HANDLE_DIALOG(90);
	HANDLE_DIALOG(110);
	HANDLE_DIALOG(111);
	HANDLE_DIALOG(130);
	HANDLE_DIALOG(150);
	HANDLE_DIALOG(151);
	HANDLE_DIALOG(152);
	HANDLE_DIALOG(153);
	HANDLE_DIALOG(154);
	HANDLE_DIALOG(160);
	HANDLE_DIALOG(161);
	HANDLE_DIALOG(162);
	HANDLE_DIALOG(163);
	HANDLE_DIALOG(180);
	HANDLE_DIALOG(190);
	HANDLE_DIALOG(191);
	HANDLE_DIALOG(201);
	HANDLE_DIALOG(210);
	HANDLE_DIALOG(211);
	HANDLE_DIALOG(212);
	HANDLE_DIALOG(240);
	HANDLE_DIALOG(250);
	HANDLE_DIALOG(251);
	HANDLE_DIALOG(260);
	HANDLE_DIALOG(270);
	HANDLE_DIALOG(271);
	HANDLE_DIALOG(272);
	HANDLE_DIALOG(290);
	HANDLE_DIALOG(310);
	HANDLE_DIALOG(330);
	HANDLE_DIALOG(340);
	HANDLE_DIALOG(360);
	HANDLE_DIALOG(361);
	HANDLE_DIALOG(370);
	HANDLE_DIALOG(371);
	HANDLE_DIALOG(372);
	HANDLE_DIALOG(373);
	HANDLE_DIALOG(380);
	HANDLE_DIALOG(410);
	HANDLE_DIALOG(430);
	HANDLE_DIALOG(460);
	HANDLE_DIALOG(470);
	HANDLE_DIALOG(471);
	HANDLE_DIALOG(472);
	HANDLE_DIALOG(473);
	HANDLE_DIALOG(474);
	HANDLE_DIALOG(480);
	HANDLE_DIALOG(490);
	HANDLE_DIALOG(530);
	HANDLE_DIALOG(550);
	HANDLE_DIALOG(560);
	HANDLE_DIALOG(590);
	HANDLE_DIALOG(600);

	default:
		warning("ERROR: Il dialogo %d non esiste!\n", (int)nDlg);
		return "ERROR";
	}
}

void mpalDumpDialog(LPMPALDIALOG dlg) {
	char dfn[64];
	char fname[64];
	int g,c,j;
	struct command* curCmd;
	char *frase; char *p;
	char copia[2048];
	bool bAtLeastOne;
	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");
	
	sprintf(dfn,"DIALOG%03d.HTM", dlg->nObj);
	warning("Dumping %s...\n", dfn);

	f = g_system->getSavefileManager()->openForSaving(dfn);

	f->writeString("<HTML>\n<BODY>\n");

	for (g = 0;dlg->Group[g].num != 0; g++) {
		bAtLeastOne = false;

		for (c = 0; c<dlg->Group[g].nCmds; c++) {
			curCmd = &dlg->Command[dlg->Group[g].CmdNum[c]];
			if (curCmd->type == 1 && curCmd->nCf == 71) {
				bAtLeastOne = true;
				break;
			}
		}
		
		if (!bAtLeastOne)
			continue;
		
		f->writeString(Common::String::format("<P>\n<H3>Group %d</H3>\n<P>\n", g));
		f->writeString("<TABLE WIDTH = 100%% BORDER = 1>\n");

		for (c = 0;c<dlg->Group[g].nCmds; c++) {
			curCmd = &dlg->Command[dlg->Group[g].CmdNum[c]];

			// Se è una funzione custom, e richiama la SendDialogMessage(nPers, nMsg)
			if (curCmd->type == 1 && curCmd->nCf == 71) {
				sprintf(fname, "%03d-%05d.WAV", dlg->nObj, curCmd->arg2);
				
				for (j = 0; dlg->Periods[j] != NULL; j++)
					if (dlg->PeriodNums[j] == curCmd->arg2)
						break;
						
				if (dlg->Periods[j] == NULL)
					warning("ERROR: Dialogo %d, Periodo %d non trovato!\n", (int)dlg->nObj, (int)curCmd->arg2);
				else {	
					frase = (char *)GlobalLock(dlg->Periods[j]);
					strcpy(copia, frase);
					GlobalUnlock(dlg->Periods[j]);

					while ((p = strchr(copia,'^')) != NULL)
						*p = '\"';

					p = frase;
					while (*p == ' ') p++;
					if (*p == '\0')
						continue;				

					v1->writeString(Common::String::format("%s\n", fname));
					f->writeString("\t<TR>\n");
					f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </TD>\n", fname));
					f->writeString(Common::String::format("\t\t<TD WIDTH = 13%%> <B> %s </B> </TD>\n", 
						GetPersonName(dlg->nObj, curCmd->arg1)));
					f->writeString(Common::String::format("\t\t<TD> %s </TD>\n",copia));
					f->writeString("\t</TR>\n");
					//fprintf(f, "(%s) <%s> %s\n", fname, GetPersonName(dlg->nObj, curCmd->arg1), copia);
				}
			}
		}

		f->writeString("</TABLE><P>\n");
		//fprintf(f,"\n\n\n\n");
	}

	f->finalize();
	v1->finalize();
	delete f;
	delete v1;
}

void mpalDumpDialogs(void) {
	int i;

	LockDialogs();

	for (i = 0; i < nDialogs; i++)
		mpalDumpDialog(&lpmdDialogs[i]);

	UnlockDialogs();
}

} // end of namespace MPAL

} // end of namespace Tony
