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
 */

#ifndef HPL_OPENAL_SOUND_DATA_H
#define HPL_OPENAL_SOUND_DATA_H

#include "hpl1/engine/sound/SoundData.h"

#include "audio/audiostream.h"

namespace hpl {

class cOpenALSoundData : public iSoundData {
public:
	cOpenALSoundData(tString asName, bool abStream);
	~cOpenALSoundData();

	bool CreateFromFile(const tString &asFile);

	iSoundChannel *CreateChannel(int alPriority);

	bool IsStream() { return mbStream; }

	//cOAL_Sample *GetSample() { return (mpSample); } // static_cast<cOAL_Sample*> (mpSoundData));}
	//cOAL_Stream *GetStream() { return (mpStream); } // static_cast<cOAL_Stream*> (mpSoundData));}

private:
	//cOAL_Sample *mpSample;
	//cOAL_Stream *mpStream;
	Audio::SeekableAudioStream* _stream;

	// iOAL_Loadable*	mpSoundData;
};
};     // namespace hpl
#endif // HPL_OPENAL_SOUND_DATA_H
