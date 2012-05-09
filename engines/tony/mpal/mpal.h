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
 *  F   4  "$c  '$   $   #$u#$u       '$ Module:  MPAL Main Include file. *
 * 4    4k   *N  #b .>    '$N'*$u      *                                  *
 * M     $L   #$  $ 8       "$c'#$b.. .@ Author:  Giovanni Bajo.......... *
 * M     '$u   "$u :"         *$. "#*#"                                   *
 * M      '$N.  "  F           ^$k       Desc:    Main Include file for   *
 * 4>       ^R$oue#             d                 using MPAL.DLL......... *
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


/****************************************************************************\
*       Copyright Notice
\****************************************************************************/

/*
 * A Spyral Software Production:
 *
 * MPAL - MultiPurpose Adventure Language
 * (C) 1997 Giovanni Bajo and Luca Giusti
 * ALL RIGHTS RESERVED
 *
 *
 */


/****************************************************************************\
*       General Introduction
\****************************************************************************/

/*
 * MPAL (MultiPurpose Adventure Language) is a high level language
 * for the definition of adventure. Through the use of MPAL you can describe
 * storyboard the adventure, and then use it with any user interface. 
 * In fact, unlike many other similar products, MPAL is not programmed through 
 * the whole adventure, but are defined only the locations, objects, as they may 
 * interact with each other, etc.. thus making MPAL useful for any type of adventure.
 */

/****************************************************************************\
*       Structure
\****************************************************************************/

/*
 * MPAL consists of two main files: MPAL.DLL and MPAL.H
 * The first is the DLL that contains the code to interface with MPAL
 * adventures, the second is the header that defines the prototypes
 * functions. MPAL is compiled for Win32, and it can therefore be used with
 * any compiler that supports Win32 DLL (Watcom C++, Visual C++,
 * Delphi, etc.), and therefore compatible with both Windows 95 and Windows NT.
 *
 * To use the DLL, and 'obviously need to create a library for symbols to export.
 *
 */


/****************************************************************************\
*       Custom Functions
\****************************************************************************/

/*
 * A custom function and a function specified by the program that uses the
 * library, to perform the particular code. The custom functions are
 * retrieved from the library as specified in the source MPAL, and in particular 
 * in defining the behavior of an item with some action.
 *
 * To use the custom functions, you need to prepare an array of
 * pointers to functions (such as using the type casting LPCUSTOMFUNCTION, 
 * (defined below), and pass it as second parameter to mpalInit (). Note you
 * must specify the size of the array, as elements of pointers and which do not 
 * contain the same: the library will call it only those functions specified in 
 * the source MPAL. It can be useful, for debugging reasons, do not bet 
 * the shares of arrays used to debugging function, to avoid unpleasant crash, 
 * if it has been made an error in source and / or some oversight in the code.
 *
 */

#ifndef TONY_MPAL_H
#define TONY_MPAL_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "tony/coroutine.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Macro definitions and structures
\****************************************************************************/

/* OK value for the error codes */
#define OK      0

#define MAXFRAMES  400        // frame animation of an object
#define MAXPATTERN 40         // pattern of animation of an object

#define MAXPOLLINGLOCATIONS   64

#define EXPORT
#define LPSTR char *

/****************************************************************************\
*       enum QueryCoordinates
*       ---------------------
* Description: Macro for use with queries that may refer to X and Y co-ordinates
\****************************************************************************/

enum QueryCoordinates {
  MPQ_X,
  MPQ_Y
};


/****************************************************************************\
*       enum QueryTypes
*       ---------------
* Description: Query can be used with mpalQuery methods. In practice corresponds
*              all claims that can do at the library
\****************************************************************************/

enum QueryTypes {
  /* General Query */
  MPQ_VERSION=10,

  MPQ_GLOBAL_VAR=50,
  MPQ_RESOURCE,
  MPQ_MESSAGE,

  /* Query on leases */
  MPQ_LOCATION_IMAGE=100,
  MPQ_LOCATION_SIZE,

  /* Queries about items */
  MPQ_ITEM_LIST=200,
  MPQ_ITEM_DATA,
  MPQ_ITEM_PATTERN,
  MPQ_ITEM_NAME,
	MPQ_ITEM_IS_ACTIVE,

  /* Query dialog */
  MPQ_DIALOG_PERIOD=300,
  MPQ_DIALOG_WAITFORCHOICE,
  MPQ_DIALOG_SELECTLIST,
  MPQ_DIALOG_SELECTION,

  /* Query execution */
  MPQ_DO_ACTION=400,
  MPQ_DO_DIALOG
};


/****************************************************************************\
*       typedef ITEM
*       ------------
* Description: Framework to manage the animation of an item
\****************************************************************************/

typedef struct {
  char *frames[MAXFRAMES];
  Common::Rect frameslocations[MAXFRAMES];
  Common::Rect bbox[MAXFRAMES];
  short pattern[MAXPATTERN][MAXFRAMES];
  short speed;
  char numframe;
  char numpattern;
  char curframe;
  char curpattern;
  short destX, destY;
  signed char Zvalue;
  short objectID;
  char TAG;
} ITEM;
typedef ITEM *LPITEM;


/****************************************************************************\
*       typedef LPCUSTOMFUNCTION
*       ------------------------
* Description: Define a custom function, to use the language MPAL
*              to perform various controls as a result of an action
\****************************************************************************/

typedef void (*LPCUSTOMFUNCTION)(CORO_PARAM, uint32, uint32, uint32, uint32);
typedef LPCUSTOMFUNCTION *LPLPCUSTOMFUNCTION;


/****************************************************************************\
*       typedef LPITEMIRQFUNCTION
*       -------------------------
* Description: Define an IRQ of an item that is called when the 
*              pattern changes or the status of an item
\****************************************************************************/

typedef void (*LPITEMIRQFUNCTION)(uint32, int, int);
typedef LPITEMIRQFUNCTION* LPLPITEMIRQFUNCTION;


/****************************************************************************\
*       Macrofunctions query
\****************************************************************************/

/****************************************************************************\
*
* Function:     uint32 mpalQueryVersion(void);
*
* Description:  Gets the current version of MPAL
*
* Return:       Version number (0x1232 = 1.2.3b)
*
\****************************************************************************/

#define mpalQueryVersion()                              \
        (uint16)mpalQueryDWORD(MPQ_VERSION)



/****************************************************************************\
*
* Function:     uint32 mpalQueryGlobalVar(LPSTR lpszVarName);
*
* Description:  Gets the numerical value of a global variable
*
* Input:        LPSTR lpszVarName       Nome della variabile (ASCIIZ)
*
* Return:       Valore della variabile
*
* Note:         This query was implemented for debugging. The program, 
*				if well designed, should not need to access variables from 
*				within the library.
*
\****************************************************************************/

#define mpalQueryGlobalVar(lpszVarName)                 \
        mpalQueryDWORD(MPQ_GLOBAL_VAR, (const char *)(lpszVarName))



/****************************************************************************\
*
* Function:     HGLOBAL mpalQueryResource(uint32 dwResId);
*
* Description:  Provides access to a resource inside the .MPC file
*
* Input:        uint32 dwResId           ID della risorsa
*
* Return:       Handle to a memory area containing the resource, 
*				ready for use.
*
\****************************************************************************/

#define mpalQueryResource(dwResId)                      \
        mpalQueryHANDLE(MPQ_RESOURCE, (uint32)(dwResId))



/****************************************************************************\
*
* Function:     LPSTR mpalQueryMessage(uint32 nMsg);
*
* Description:  Returns a message.
*
* Input:        uint32 nMsg               Message number
*
* Return:       ASCIIZ message
*
* Note:         The returned pointer must be freed with GlobalFree()
*				after use. The message will be in ASCIIZ format.
*
\****************************************************************************/

#define mpalQueryMessage(nMsg)                          \
        (LPSTR)mpalQueryHANDLE(MPQ_MESSAGE, (uint32)(nMsg))



/****************************************************************************\
*
* Function:     HGLOBAL mpalQueryLocationImage(uint32 nLoc);
*
* Description:  Provides a image image
*
* Input:        uint32 nLoc              Locazion number
*
* Return:       Returns a picture handle
*
\****************************************************************************/

#define mpalQueryLocationImage(nLoc)                    \
        mpalQueryHANDLE(MPQ_LOCATION_IMAGE, (uint32)(nLoc))



/****************************************************************************\
*
* Function:     uint32 mpalQueryLocationSize(uint32 nLoc, uint32 dwCoord);
*
* Description:  Request the x or y size of a location in pixels
*
* Input:        uint32 nLoc              Location number
*               uint32 dwCoord           MPQ_Xr o MPQ_Y
*
* Return:       Size
*
\****************************************************************************/

#define mpalQueryLocationSize(nLoc,dwCoord)             \
        mpalQueryDWORD(MPQ_LOCATION_SIZE,(uint32)(nLoc),(uint32)(dwCoord))



/****************************************************************************\
*
* Function:     uint32 * mpalQueryItemList(uint32 nLoc);
*
* Description:  Provides the list of objects in the lease.
*
* Input:        uint32 nLoc              Location number
*
* Return:       List of objects (accessible by Item [0], Item [1], etc.)
*
\****************************************************************************/
// TODO: Check if the results of this are endian safe
#define mpalQueryItemList(nLoc)                         \
        (uint32 *)mpalQueryHANDLE(MPQ_ITEM_LIST,(uint32)(nLoc))



/****************************************************************************\
*
* Function:     LPBKGANIM mpalQueryItemData(uint32 nItem);
*
* Description:  Provides information on an item
*e
* Input:        uint32 nItem             Item number
*
* Return:       structure filled with requested information
*
\****************************************************************************/

#define mpalQueryItemData(nItem)                          \
        (LPITEM)mpalQueryHANDLE(MPQ_ITEM_DATA,(uint32)(nItem))



/****************************************************************************\
*
* Function:     uint32 mpalQueryItemPattern(uint32 nItem);
*
* Description:  Provides the current pattern of an item
*
* Input:        uint32 nItem             Item number
*
* Return:       Number of animation patterns to be executed.
*
* Note:         By default, the pattern of 0 indicates that we should 
*				do nothing.
*
\****************************************************************************/

#define mpalQueryItemPattern(nItem)                  \
        mpalQueryDWORD(MPQ_ITEM_PATTERN,(uint32)(nItem))



/****************************************************************************\
*
* Function:     bool mpalQueryItemIsActive(uint32 nItem);
*
* Description:  Returns true if an item is active
*
* Input:        uint32 nItem             Item number
*
* Return:       TRUE if the item is active, FALSE otherwise
*
\****************************************************************************/

#define mpalQueryItemIsActive(nItem)                  \
        (bool)mpalQueryDWORD(MPQ_ITEM_IS_ACTIVE,(uint32)(nItem))


/****************************************************************************\
*
* Function:     void mpalQueryItemName(uint32 nItem, LPSTR lpszName);
*
* Description:  Returns the name of an item
*
* Input:        uint32 nItem             Item number
*               LPSTR lpszName          Pointer to a buffer of at least 33 bytes
*                                       that will be filled with the name
*
* Note:         If the item is not active (ie. if its status or number
*				is less than or equal to 0), the string will be empty.
*
\****************************************************************************/

#define mpalQueryItemName(nItem, lpszName)             \
        mpalQueryHANDLE(MPQ_ITEM_NAME,(uint32)(nItem), (LPSTR)(lpszName))



/****************************************************************************\
*
* Function:     LPSTR mpalQueryDialogPeriod(uint32 nDialog, uint32 nPeriod);
*
* Description:  Returns a sentence of dialog.
*
* Input:        uint32 nDialog           Dialog number
*               uint32 nPeriod           Number of words
*
* Return:       A pointer to the string of words, or NULL on failure.
*
* Note:         The string must be freed after use by GlobalFree ().
*
*               Unlike normal messages, the sentences of dialogue
*				are formed by a single string terminated with 0.
*
\****************************************************************************/

#define mpalQueryDialogPeriod(nPeriod)                  \
        (LPSTR)mpalQueryHANDLE(MPQ_DIALOG_PERIOD, (uint32)(nPeriod))



/****************************************************************************\
*
* Function:     int mpalQueryDialogWaitForChoice(void);
*
* Description:  Wait until the moment in which the need is signaled 
*				to make a choice by the user.
*
* Return:       Number of choice to be made, or -1 if the dialogue is finished.
*
\****************************************************************************/

#define mpalQueryDialogWaitForChoice(dwRet)                  \
        CORO_INVOKE_2(mpalQueryCORO, MPQ_DIALOG_WAITFORCHOICE, dwRet)

/****************************************************************************\
*
* Function:     uint32 * mpalQueryDialogSelectList(uint32 nChoice);
*
* Description:  Requires a list of various options for some choice within 
*				the current dialog.

* Input:        uint32 nChoice           Choice number
*
* Return:       A pointer to an array containing the data matched to each option.
*
* Note:         The figure 'a uint32 specified in the source to which MPAL
*				You can 'assign meaning that the more' suits.
*
*               The pointer msut be freed after use by GlobalFree().
*
\****************************************************************************/

#define mpalQueryDialogSelectList(nChoice)              \
        (uint32 *)mpalQueryHANDLE(MPQ_DIALOG_SELECTLIST,(uint32)(nChoice))



/****************************************************************************\
*
* Function:     bool mpalQueryDialogSelection(uint32 nChoice, uint32 dwData);
*
* Description:  Warns the library that the user has selected, in a certain 
*				choice of the current dialog, corresponding option
*				at a certain given.
*
* Input:        uint32 nChoice           Choice number of the choice that 
*										was in progress
*               uint32 dwData            Option that was selected by the user.
*
* Return:       TRUE if all OK, FALSE on failure.
*
* Note:         After execution of this query, MPAL continue 
* Groups according to the execution of the dialogue. And necessary so the game 
* remains on hold again for another  Chosen by mpalQueryDialogWaitForChoice ().
*
\****************************************************************************/

#define mpalQueryDialogSelection(nChoice,dwData)        \
        (bool)mpalQueryDWORD(MPQ_DIALOG_SELECTION,(uint32)(nChoice),(uint32)(dwData))


/****************************************************************************\
*
* Function:     HANDLE mpalQueryDoAction(uint32 nAction, uint32 nItem,
*                 uint32 dwParam);
*
* Description:  Warns the library an action was performed on a Object. 
* The library will call 'custom functions, if necessary.
*
* Input:        uint32 nAction           Action number
*               uint32 nItem             Item number
*               uint32 dwParam           Action parameter
*
* Return:       Handle to the thread that is performing the action, or
*				INVALID_PID_VALUE if the action is not 'defined for 
*				the item, or the item and 'off.
*
* Note:         The parameter is used primarily to implement actions 
* as "U.S." involving two objects together. The action will be executed only
* if the item is active, ie if its status is a positive number greater than 0.
*
\****************************************************************************/

#define mpalQueryDoAction(nAction, nItem, dwParam)      \
        mpalQueryDWORD(MPQ_DO_ACTION, (uint32)(nAction), (uint32)(nItem), (uint32)(dwParam))



/****************************************************************************\
*
* Function:     HANDLE mpalQueryDoDialog(uint32 nDialog, uint32 nGroup);
*
* Description:  Warns the library a dialogue was required.
*
* Input:        uint32 nDialog           Dialog number
*               uint32 nGroup            Group number to use
*
* Return:       Handle to the thread that is running the box, or
*				INVALID_PID_VALUE if the dialogue does not exist.
*
\****************************************************************************/

#define mpalQueryDoDialog(nDialog,nGroup)               \
        mpalQueryHANDLE(MPQ_DO_DIALOG, (uint32)(nDialog),(uint32)(nGroup))


/****************************************************************************\
*       Functions exported DLL
\****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************\
*
* Function:     bool mpalInit(LPSTR lpszMpcFileName, LPSTR lpszMprFileName,
*                 LPLPCUSTOMFUNCTION lplpcfArray);
*
* Description:  Initializes the MPAL library, and opens an .MPC file, which
*				will be 'used for all queries
*
* Input:        LPSTR lpszMpcFileName   Name of the .MPC file, including extension
*               LPSTR lpszMprFileName   Name of the .MPR file, including extension
*               LPLPCUSTOMFUNCTION		Array of pointers to custom functions
*
* Return:       TRUE if all OK, FALSE on failure
*
\****************************************************************************/

bool EXPORT mpalInit(const char *lpszFileName, const char *lpszMprFileName,
					 LPLPCUSTOMFUNCTION lplpcfArray, Common::String *lpcfStrings);



/****************************************************************************\
*
* Function:     uint32 mpalQuery(uint16 wQueryType, ...);
*
* Description:  This is the general function to communicate with the library,
*				To request information about what is in the .MPC file
*
* Input:        uint16 wQueryType       Type of query. The list is in
*										the QueryTypes enum.
*
* Return:       4 bytes depending on the type of query
*
* Note:         I _strongly_ recommended to use macros defined above to use 
*				the query, since it helps avoid any unpleasant bugs due to 
*				forgeting parameters.
*
\****************************************************************************/

typedef void *HANDLE;

uint32 mpalQueryDWORD(uint16 wQueryType, ...);

HANDLE mpalQueryHANDLE(uint16 wQueryType, ...);

void mpalQueryCORO(CORO_PARAM, uint16 wQueryType, uint32 *dwRet, ...);

/****************************************************************************\
*
* Function:     bool mpalExecuteScript(int nScript);
*
* Description:  Execute a script. The script runs on multitasking by a thread.
*
* Input:        int nScript             Script number to run
*
* Return:       TRUE if the script 'was launched, FALSE on failure
*
\****************************************************************************/

bool EXPORT mpalExecuteScript(int nScript);



/****************************************************************************\
*
* Function:     uint32 mpalGetError(void);
*
* Description:  Returns the current MPAL error code
*
* Return:       Error code
*
\****************************************************************************/

uint32 EXPORT mpalGetError(void);



/****************************************************************************\
*
* Function:     void mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCustom);
*
* Description:  Install a custom routine That will be called by MPAL
*				every time the pattern of an item has-been changed.
*
* Input:        LPITEMIRQFUNCTION lpiifCustom   Custom function to install
*
\****************************************************************************/

void EXPORT mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCustom);


/****************************************************************************\
*
* Function:     bool mpalStartIdlePoll(int nLoc);
*
* Description:  Process the idle actions of the items on one location.
*
* Input:        int nLoc                Number of the location whose items
*                                       must be processed for idle actions.
*
* Return:       TRUE if all OK, and FALSE if it exceeded the maximum limit.
*
* Note:         The maximum number of locations that can be polled
*				simultaneously is defined defined by MAXPOLLINGFUNCIONS
*
\****************************************************************************/

bool EXPORT mpalStartIdlePoll(int nLoc);


/****************************************************************************\
*
* Function:     bool mpalEndIdlePoll(int nLoc);
*
* Description:  Stop processing the idle actions of the items on one location.
*
* Input:        int nLoc                Number of the location
*
* Return:       TRUE if all OK, FALSE if the specified location was not
*				in the process of polling
*
\****************************************************************************/

void mpalEndIdlePoll(CORO_PARAM, int nLoc, bool *result);


/****************************************************************************\
*
* Function:     int mpalLoadState(LPBYTE buf);
*
* Description:  Load a save state from a buffer. 
*
* Input:				LPBYTE buf		Buffer where to store the state
*
*	Return:				Length of the state in bytes
*
\****************************************************************************/

int EXPORT mpalLoadState(byte *buf);



/****************************************************************************\
*
* Function:     void mpalSaveState(LPBYTE buf);
*
* Description:  Store the save state into a buffer. The buffer must be
*				length at least the size specified with mpalGetSaveStateSize
*
* Input:		LPBYTE buf				Buffer where to store the state
*
\****************************************************************************/

void EXPORT mpalSaveState(byte *buf);



/****************************************************************************\
*
* Function:     int mpalGetSaveStateSize(void);
*
* Description:  Acquire the length of a save state
*
* Return:       Length in bytes
*
\****************************************************************************/

int EXPORT mpalGetSaveStateSize(void);

#ifdef __cplusplus
}
#endif

/****************************************************************************\
*
* Function:     void LockVar(void);
*
* Description:  Locka le variabili per accederci
*
\****************************************************************************/

extern void LockVar(void);

/****************************************************************************\
*
* Function:     void UnlockVar(void);
*
* Description:  Unlocka le variabili dopo l'uso
*
\****************************************************************************/

extern void UnlockVar(void);

} // end of namespace MPAL

} // end of namespace Tony

#endif

