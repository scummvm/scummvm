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

//===================================================================================================================
//
// File - tony_gsdk.cpp
//
//===================================================================================================================


//general odds and ends

#include <sys/stat.h>
#include <fcntl.h>
//#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "stdafx.h"
//#include "src\driver96.h"
#include "debug.h"
#include "header.h"
#include "layers.h"
#include "memory.h"
#include "protocol.h"
#include "resman.h"
#include "tony_gsdk.h"

// TODO replace with file class

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
uint32	Read_file(const char	*name,	mem	**membloc, uint32	uid)	//Tony25Apr96
{
//read the file in and place into an allocated MEM_float block
//used for non resources manager files - stuff like fonts, etc.
//returns bytes read or 0 for error

	FILE	*fh=0;	//file pointer
	uint32	end;



	fh = fopen(name, "rb");	//open for binary reading

	if	(fh==NULL)
	{	Zdebug("Read_file cannot open %s", name);
		return(0);
	}

//ok, find the length and read the file in
	fseek(fh, 0, SEEK_END);	//get size of file
   end = ftell(fh);	//finally got the end

	*membloc= Twalloc(end, MEM_float, uid);	//reserve enough floating memory for the file
	
	fseek( fh, 0, SEEK_SET );	//back to beginning of file

	if	(fread( (*membloc)->ad, sizeof(char), end,fh) < end)
	{	Zdebug("Read_file read fail %d", name);
		return(0);
	}

	fclose(fh);

	return(end);	//ok, done it - return bytes read
}
//-----------------------------------------------------------------------------------------------------------------------
int32	Direct_read_file(const char	*name,	char	*ad)	//Tony1May96
{
//load the file directly into the memory location passed
//memory must be pre-allocated

	FILE	*fh=0;	//file pointer
	uint32	end;


	fh = fopen(name, "rb");	//open for binary reading

	if	(fh==NULL)
	{	Zdebug("Direct_read_file cannot open %s", name);
		return(0);
	}

//ok, find the length and read the file in
	fseek(fh, 0, SEEK_END);	//get size of file
   end = ftell(fh);	//finally got the end
	fseek( fh, 0, SEEK_SET );	//back to beginning of file

	if	(fread( ad, sizeof(char), end,fh) < end)
	{	Zdebug("Direct_read_file read fail %d", name);
		return(0);
	}

	fclose(fh);


	return(end);	//ok, done it - return bytes read
}
//-----------------------------------------------------------------------------------------------------------------------
int32	Direct_write_file(const char	*name,	char	*ad, uint32 total_bytes)	//Tony1May96
{
//load the file directly into the memory location passed
	int	fh;

	//fh = open(name, _O_RDWR | _O_CREAT);	//open for reading
	fh = open(name, O_RDWR | O_CREAT);	//open for reading

	if	(fh==-1)
	{	Zdebug("Direct_write_file open fail %d", name);
		return(-1);
	}

	if	(write( fh, ad, total_bytes)==-1)
	{	Zdebug("Direct_write_file write fail %d", name);
		return(-1);
	}

	close(fh);

	return(0);	//ok, done it
}

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

