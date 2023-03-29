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
#include "hpl1/engine/system/SystemTypes.h"

#include "audio/audiostream.h"
#include "common/ptr.h"
#include "common/str.h"

namespace hpl {

class cLowLevelSoundOpenAL;

class cOpenALSoundData : public iSoundData {
public:
	cOpenALSoundData(tString asName, bool abStream, cLowLevelSoundOpenAL *lowLevelSound);
	~cOpenALSoundData();

	bool CreateFromFile(const tString &asFile);

	iSoundChannel *CreateChannel(int alPriority);

	bool IsStream() { return mbStream; }

private:
	Common::SharedPtr<byte> _audioData = nullptr;
	uint32 _audioDataSize = 0;
	uint _format = 0;
	cLowLevelSoundOpenAL *_lowLevelSound;
};

} // namespace hpl

#endif // HPL_OPENAL_SOUND_DATA_H
