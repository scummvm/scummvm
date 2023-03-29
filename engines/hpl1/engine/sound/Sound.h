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

#ifndef HPL_SOUND_H
#define HPL_SOUND_H

#include "hpl1/engine/game/Updateable.h"

namespace hpl {

class iLowLevelSound;
class cResources;
class cSoundHandler;
class cMusicHandler;

class cSound : public iUpdateable {
public:
	cSound(iLowLevelSound *apLowLevelSound);
	~cSound();

	void Init(cResources *apResources, bool abUseHardware, bool abForceGeneric, bool abUseEnvAudio, int alMaxChannels,
			  int alStreamUpdateFreq, bool abUseThreading, bool abUseVoiceManagement,
			  int alMaxMonoSourceHint, int alMaxStereoSourceHint,
			  int alStreamingBufferSize, int alStreamingBufferCount, bool abEnableLowLevelLog, tString asDeviceName);

	void Update(float afTimeStep);

	iLowLevelSound *GetLowLevel() { return mpLowLevelSound; }
	cSoundHandler *GetSoundHandler() { return mpSoundHandler; }
	cMusicHandler *GetMusicHandler() { return mpMusicHandler; }

private:
	iLowLevelSound *mpLowLevelSound;
	cResources *mpResources;
	cSoundHandler *mpSoundHandler;
	cMusicHandler *mpMusicHandler;
};

} // namespace hpl
#endif // HPL_SOUND_H
