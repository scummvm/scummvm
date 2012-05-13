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
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  MPAL DLL Header........ *
 * 4    4k   *N  #b .>    '$N'*$u      *                                  *
 * M     $L   #$  $ 8       "$c'#$b.. .@ Author:  Giovanni Bajo.......... *
 * M     '$u   "$u :"         *$. "#*#"                                   *
 * M      '$N.  "  F           ^$k       Desc:    Header per i moduli per *
 * 4>       ^R$oue#             d                 la DLL di query di MPAL *
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

#ifndef __MPALDLL_H
#define __MPALDLL_H

#include "common/file.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/stubs.h"
#include "tony/mpal/loadmpc.h"
#include "tony/mpal/expr.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Defines
\****************************************************************************/

#define HEX_VERSION             0x0170

/*
  SICURA

#define MAX_ACTIONS_PER_ITEM    40
#define MAX_COMMANDS_PER_ITEM   256
#define MAX_COMMANDS_PER_ACTION 64
#define MAX_DESCRIBE_SIZE				128
*/

#define MAX_ACTIONS_PER_ITEM    40
#define MAX_COMMANDS_PER_ITEM   128
#define MAX_COMMANDS_PER_ACTION 128
#define MAX_DESCRIBE_SIZE				64



#define MAX_MOMENTS_PER_SCRIPT  256
#define MAX_COMMANDS_PER_SCRIPT 256
#define MAX_COMMANDS_PER_MOMENT 32





/*
   Versione sicura!

#define MAX_GROUPS_PER_DIALOG   128
#define MAX_COMMANDS_PER_DIALOG 640
#define MAX_COMMANDS_PER_GROUP  64
#define MAX_CHOICES_PER_DIALOG  64
#define MAX_SELECTS_PER_CHOICE  33
#define MAX_PLAYGROUPS_PER_SELECT 9
#define MAX_PERIODS_PER_DIALOG  640

*/

#define MAX_GROUPS_PER_DIALOG   128
#define MAX_COMMANDS_PER_DIALOG 480
#define MAX_COMMANDS_PER_GROUP  64
#define MAX_CHOICES_PER_DIALOG  64
#define MAX_SELECTS_PER_CHOICE  64
#define MAX_PLAYGROUPS_PER_SELECT 9
#define MAX_PERIODS_PER_DIALOG  400

/*
  Prima di Rufus:

#define MAX_GROUPS_PER_DIALOG   128
#define MAX_COMMANDS_PER_DIALOG 512
#define MAX_COMMANDS_PER_GROUP  32
#define MAX_CHOICES_PER_DIALOG  64
#define MAX_SELECTS_PER_CHOICE  32
#define MAX_PLAYGROUPS_PER_SELECT 4
#define MAX_PERIODS_PER_DIALOG  512
*/

#define NEED_LOCK_MSGS


/****************************************************************************\
*       Strutture
\****************************************************************************/

#include "common/pack-start.h"

/****************************************************************************\
*       typedef MPALVAR
*       ---------------
* Description: Variabile globale di MPAL
\****************************************************************************/

struct MPALVAR {
	uint32 dwVal;                   // Valore della variabile
	char lpszVarName[33];			// Nome della variabile
} PACKED_STRUCT;
typedef MPALVAR*        LPMPALVAR;
typedef LPMPALVAR*      LPLPMPALVAR;


/****************************************************************************\
*       typedef MPALMSG
*       ---------------
* Description: Messaggio di MPAL
\****************************************************************************/

struct MPALMSG {
  HGLOBAL hText;                // Handle al testo del messaggio
  uint16 wNum;                    // Numero del messaggio
} PACKED_STRUCT;
typedef MPALMSG*        LPMPALMSG;
typedef LPMPALMSG*      LPLPMPALMSG;


/****************************************************************************\
*       typedef MPALLOCATION
*       --------------------
* Description: Locazione di MPAL
\****************************************************************************/

struct MPALLOCATION {
	uint32 nObj;						// Numero della locazione
	uint32 dwXlen, dwYlen;			// Dimensione
	uint32 dwPicRes;				// Risorsa che contiene l'immagine
} PACKED_STRUCT;
typedef MPALLOCATION*   LPMPALLOCATION;
typedef LPMPALLOCATION* LPLPMPALLOCATION;


/****************************************************************************\
*       struct command
*       --------------
* Description: Gestisce un comando, cioe' le tag utilizzate dalle OnAction
*   negli item, dalle Time negli script e dai Group nei Dialog
\****************************************************************************/

struct command {
  /*
   * Tipi di comandi riconosciuti:
   *
   *   #1 -> Chiamata a funzione custom         (ITEM, SCRIPT, DIALOG)
   *   #2 -> Assegnazione di variabile          (ITEM, SCRIPT, DIALOG)
   *   #3 -> Esecuzione di una scelta           (DIALOG)
   *
   */
	byte  type;						// Tipo di comando

	union {
		int32 nCf;                  // Numero funzione custom         [#1]
		char *lpszVarName;			// Nome variabile                 [#2]
		int32 nChoice;              // Numero di scelta da fare       [#3]
	};

	union {
		int32 arg1;                 // Argomento 1 funzione custom    [#1]
		HGLOBAL expr;				// Espressione da assegnare alla
									//   variabile                    [#2]
	};

	int32 arg2,arg3,arg4;			// Argomenti per funzione custom  [#1]
} PACKED_STRUCT;

/****************************************************************************\
*       typedef MPALDIALOG
*       ------------------
* Description: Dialog di MPAL
\****************************************************************************/

struct MPALDIALOG {
	uint32 nObj;                    // Numero dialog

	struct command Command[MAX_COMMANDS_PER_DIALOG];

	struct {
		uint16 num;

		byte nCmds;
		uint16 CmdNum[MAX_COMMANDS_PER_GROUP];

	} Group[MAX_GROUPS_PER_DIALOG];

	struct {
		// L'ultima choice ha nChoice==0
		uint16 nChoice;

		// Non c'e' il numero di Select (siamo abbastanza avari di RAM). L'ultimo
		// select ha dwData==0
		struct {
			HGLOBAL when;
			uint32 dwData;
			uint16 wPlayGroup[MAX_PLAYGROUPS_PER_SELECT];

			// Bit 0=endchoice   Bit 1=enddialog
			byte attr;

			// Modificata a run-time: 0 se il select e' correntemente disabilitato,
			// 1 se e' correntemente attivato
			byte curActive;
		} Select[MAX_SELECTS_PER_CHOICE];

	} Choice[MAX_CHOICES_PER_DIALOG];

	uint16 PeriodNums[MAX_PERIODS_PER_DIALOG];
	HGLOBAL Periods[MAX_PERIODS_PER_DIALOG];

} PACKED_STRUCT;
typedef MPALDIALOG*     LPMPALDIALOG;
typedef LPMPALDIALOG*   LPLPMPALDIALOG;

/****************************************************************************\
*       typedef MPALITEM
*       ----------------
* Description: Item di MPAL
\****************************************************************************/

struct ItemAction {
	byte    num;                // Numero dell'azione
	uint16    wTime;              // In caso di idle, il tempo che deve passare
    byte    perc;               // Percentuale di eseguire l'idle
    HGLOBAL when;               // Espressione da calcolare: se !=0, allora
                                //  l'azione puo' essere eseguita
    uint16    wParm;              // Parametro per l'azione

    byte nCmds;                 // Numero comandi da eseguire
    uint32 CmdNum[MAX_COMMANDS_PER_ACTION]; // Comando da eseguire
} PACKED_STRUCT;

struct MPALITEM {
	uint32 nObj;                    // Numero item

	byte lpszDescribe[MAX_DESCRIBE_SIZE]; // Nome
	byte nActions;                // Numero delle azioni gestite
	uint32 dwRes;                  // Risorsa che contiene frame e pattern

	struct command Command[MAX_COMMANDS_PER_ITEM];

  // Array di strutture contenenti le varie azioni gestite. In pratica, di
  // ogni azione sappiamo quali comandi eseguire, tra quelli definiti nella
  // struttura qui sopra
/*
  struct
  {
    byte    num;                // Numero dell'azione
    uint16    wTime;              // In caso di idle, il tempo che deve passare
    byte    perc;               // Percentuale di eseguire l'idle
    HGLOBAL when;               // Espressione da calcolare: se !=0, allora
                                //  l'azione puo' essere eseguita
    uint16    wParm;              // Parametro per l'azione

    byte nCmds;                 // Numero comandi da eseguire
    uint32 CmdNum[MAX_COMMANDS_PER_ACTION]; // Comando da eseguire

  } Action[MAX_ACTIONS_PER_ITEM];
	*/
	struct ItemAction *Action;

} PACKED_STRUCT;
typedef MPALITEM*       LPMPALITEM;
typedef LPMPALITEM*     LPLPMPALITEM;


/****************************************************************************\
*       typedef MPALSCRIPT
*       ------------------
* Description: Script di MPAL
\****************************************************************************/

struct MPALSCRIPT {
	uint32 nObj;

	uint32 nMoments;

	struct command Command[MAX_COMMANDS_PER_SCRIPT];

	struct {
		int32 dwTime;

		byte nCmds;
		uint32 CmdNum[MAX_COMMANDS_PER_MOMENT];
  
	} Moment[MAX_MOMENTS_PER_SCRIPT];

} PACKED_STRUCT;
typedef MPALSCRIPT*     LPMPALSCRIPT;
typedef LPMPALSCRIPT*   LPLPMPALSCRIPT;

#include "common/pack-end.h"


/****************************************************************************\
*       Prototipi di funzione
\****************************************************************************/

/****************************************************************************\
*
* Function:     int32 varGetValue(const char *lpszVarName);
*
* Description:  Restituisce il valore corrente di una variabile globale
*
* Input:        const char *lpszVarName       Nome della variabile
*
* Return:       Valore corrente
*
* Note:         Prima di questa funzione, bisogna richiamare LockVar() che
*               locka le variabili globali per l'utilizzo. Dopo inoltre bi-
*               sogna ricordarsi di chiamare UnlockVar()
*
\****************************************************************************/

int32 varGetValue(const char *lpszVarName);


/****************************************************************************\
*
* Function:     void varSetValue(const char *lpszVarName, int32 val);
*
* Description:  Setta un nuovo valore per una variabile globale di MPAL
*
* Input:        const char *lpszVarName       Nome della variabile
*               int32 val                 Valore da settare
*
\****************************************************************************/

void varSetValue(const char *lpszVarName, int32 val);

/****************************************************************************\
*       Includes the various modules
\****************************************************************************/

} // end of namespace MPAL

} // end of namespace Tony

#endif

