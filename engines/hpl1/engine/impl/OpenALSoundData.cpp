/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/impl/OpenALSoundData.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/impl/OpenALSoundChannel.h"
namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cOpenALSoundData::cOpenALSoundData(tString asName, bool abStream) : iSoundData(asName,abStream)
	{
		mpSample = NULL;
		mpStream = NULL;
//		mpSoundData = NULL;
	}

	//-----------------------------------------------------------------------

	cOpenALSoundData::~cOpenALSoundData()
	{
#if 0
  		if (mbStream)
		{
			if(mpStream)
				OAL_Stream_Unload ( mpStream );//static_cast<cOAL_Stream*>(mpSoundData) );
		}
		else
		{
			if(mpSample)
				OAL_Sample_Unload ( mpSample );//static_cast<cOAL_Sample*>(mpSoundData) );
		}
#endif

	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cOpenALSoundData::CreateFromFile(const tString &asFile)
	{
#if 0
  		int lFlags=0;

		unsigned int lCaps = 0;
//		FSOUND_GetDriverCaps(0, &lCaps);//Current driver here instead of 0

		//Get the load flags
//		if(lCaps & FSOUND_CAPS_HARDWARE)	lFlags |= FSOUND_HW3D;
		//if(mbStream)						lFlags |= FSOUND_STREAMABLE;

		if(mbStream)
		{
			//mpSoundData
			mpStream = OAL_Stream_Load ( asFile.c_str() );



			if(mpStream == NULL )//mpSoundData==NULL){
			{
				Error("Couldn't load sound stream '%s'\n", asFile.c_str());
				return false;
			}
			else
				OAL_Stream_SetLoop(mpStream,mbLoopStream);
				//mpStream->SetLoop(mbLoopStream);
				//mpSoundData->SetLoop(mbLoopStream);

		}
		else
		{
			mpSample = OAL_Sample_Load ( asFile.c_str() );
//			mpSoundData = OAL_Sample_Load ( asFile.c_str() );
			if(mpSample == NULL)//mpSoundData==NULL){
			{
				Error("Couldn't load sound data '%s'\n", asFile.c_str());
				return false;
			}
			else
				OAL_Sample_SetLoop(mpSample,true);
				//mpSample->SetLoop ( true );

		}

#endif
		return true;

	}

	//-----------------------------------------------------------------------

	iSoundChannel* cOpenALSoundData::CreateChannel(int alPriority)
	{
#if 0
  		//if(mpSoundData==NULL)return NULL;
		if ( (mpSample == NULL) && (mpStream == NULL) ) return NULL;

		int lHandle;
		iSoundChannel *pSoundChannel=NULL;
		if(mbStream)
		{
			lHandle = OAL_Stream_Play ( OAL_FREE, GetStream(), 1.0f, true );
			if(lHandle==-1)return NULL;

			pSoundChannel = hplNew( cOpenALSoundChannel, (this,lHandle, mpSoundManger) );
			IncUserCount();
		}
		else
		{
			lHandle = OAL_Sample_Play ( OAL_FREE, GetSample(), 1.0f, true, alPriority);
			if(lHandle==-1)return NULL;

			pSoundChannel = hplNew( cOpenALSoundChannel, (this,lHandle, mpSoundManger) );
			IncUserCount();
		}

		return pSoundChannel;
#endif
		return NULL; 
	}

	//-----------------------------------------------------------------------

	bool cOpenALSoundData::IsStereo()
	{
#if 0
  		if (mbStream)
			return (OAL_Stream_GetChannels(mpStream)==2);
		if (mpSample)
			return (OAL_Sample_GetChannels(mpSample)==2);

#endif
		return false;

	}

	//-----------------------------------------------------------------------

}
