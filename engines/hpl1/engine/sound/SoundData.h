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

#ifndef HPL_SOUND_DATA_H
#define HPL_SOUND_DATA_H

#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cSoundManager;
class iSoundChannel;

class iSoundData : public iResourceBase {
public:
	iSoundData(tString asName, bool abStream) : iResourceBase(asName, 0),
												mpSoundManger(NULL), mbStream(abStream) {}

	virtual ~iSoundData() {}

	virtual bool CreateFromFile(const tString &asFile) = 0;

	virtual iSoundChannel *CreateChannel(int alPriority) = 0;

	bool IsStream() { return mbStream; }
	void SetLoopStream(bool abX) { mbLoopStream = abX; }
	bool GetLoopStream() { return mbLoopStream; }

	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	void SetSoundManager(cSoundManager *apSoundManager) {
		mpSoundManger = apSoundManager;
	}

protected:
	bool mbStream;
	bool mbLoopStream;
	cSoundManager *mpSoundManger;
};

} // namespace hpl

#endif // HPL_SOUND_DATA_H
