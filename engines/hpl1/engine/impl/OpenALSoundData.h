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
#ifndef HPL_OPENAL_SOUND_DATA_H
#define HPL_OPENAL_SOUND_DATA_H

#include "hpl1/engine/sound/SoundData.h"

//#include "OALWrapper/OAL_Funcs.h"
#include "temp-types.h"

namespace hpl {

	class cOpenALSoundData : public iSoundData
	{
	public:
		cOpenALSoundData(tString asName, bool abStream);
		~cOpenALSoundData();

		bool CreateFromFile(const tString &asFile);

		iSoundChannel* CreateChannel(int alPriority);

		bool IsStream(){ return mbStream;}

		bool IsStereo();

		cOAL_Sample*	GetSample(){ return ( mpSample ); } //static_cast<cOAL_Sample*> (mpSoundData));}
		cOAL_Stream*	GetStream(){ return ( mpStream ); } //static_cast<cOAL_Stream*> (mpSoundData));}

	private:
		cOAL_Sample*	mpSample;
		cOAL_Stream*	mpStream;

//iOAL_Loadable*	mpSoundData;
	};
};
#endif // HPL_OPENAL_SOUND_DATA_H
