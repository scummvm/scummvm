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

#ifndef	_SPEECH
#define	_SPEECH

//#include "src\driver96.h"
#include "header.h"


#define	MAX_SUBJECT_LIST	30	//is that enough?



typedef	struct	//array of these for subject menu build up
{
	uint32	res;
	uint32	ref;
} _subject_unit;

extern uint32	speech_text_bloc_no;	// so speech text cleared when running a new start-script
extern int16	officialTextNumber;

extern int32	speechScriptWaiting;

extern	int	choosing;	//could alternately use logic->looping of course
extern	uint32	unpause_zone;
#endif
