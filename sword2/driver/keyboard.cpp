/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

//=============================================================================
//
//	Filename	:	keyboard.c
//	Created		:	19th September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the interface to the keyboard
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		19-Sep-96	PRP		Keyboard functions.  Simple logging of
//								previous 32 keys pressed.
//
//	1.1		19-Sep-96	PRP		Fixed bug, ReadKey did not return RD_OK.
//
//	1.2		13-Aug-97	PSJ		Added GetKeyStatus
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//  BOOL KeyWaiting(void)
//
//	This function returns TRUE if there is an unprocessed key waiting in the
//	queue, FALSE otherwise.
//
//	--------------------------------------------------------------------------
//
//	int32 ReadKey(char *key)
//
//	Sets the value of key passed in to the current waiting key.  If there is
//	no key waiting, an error code is returned.
//	
//	--------------------------------------------------------------------------
//
//	void GetKeyStatus(_drvKeyStatus *s)
//
//	Retrieves the status of the keyboard handler.
//	
//=============================================================================


#include "stdafx.h"
#include "driver96.h"

uint8	keyBacklog = 0;				// The number of key presses waiting to be processed.
uint8	keyPointer = 0;				// Index of the next key to read from the buffer.
char    keyBuffer[MAX_KEY_BUFFER];	// The keyboard buffer



void WriteKey(char key)
{
	if (keyBuffer && keyBacklog < MAX_KEY_BUFFER)
	{
		keyBuffer[(keyPointer + keyBacklog) % MAX_KEY_BUFFER] = key;
		keyBacklog += 1;
	}
}



BOOL KeyWaiting(void)

{

	if (keyBacklog)
		return(TRUE);
	else
		return(FALSE);

}



int32 ReadKey(char *key)

{
	if (!keyBacklog)
		return(RDERR_NOKEYWAITING);

	if (key == NULL)
		return(RDERR_INVALIDPOINTER);

	*key = keyBuffer[keyPointer++];
	if (keyPointer == MAX_KEY_BUFFER)
		keyPointer = 0;

	keyBacklog -= 1;

	return(RD_OK);

}

void GetKeyStatus(_drvKeyStatus *s)

{
	// Flush key buffer
	s->pBacklog = &keyBacklog;
	s->pPointer = &keyPointer;
	s->pBuffer =  keyBuffer;
}

