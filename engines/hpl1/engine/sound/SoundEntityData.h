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

#ifndef HPL_SOUND_ENTITY_DATA_H
#define HPL_SOUND_ENTITY_DATA_H

#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cSoundEntityData : public iResourceBase {
	friend class cSoundEntity;

public:
	cSoundEntityData(tString asName);
	~cSoundEntityData();

	bool CreateFromFile(const tString &asFile);

	const tString &GetMainSoundName() { return msMainSound; }
	void SetMainSoundName(const tString &asName) { msMainSound = asName; }
	const tString &GetStartSoundName() { return msStartSound; }
	void SetStartSoundName(const tString &asName) { msStartSound = asName; }
	const tString &GetStopSoundName() { return msStopSound; }
	void SetStopSoundName(const tString &asName) { msStopSound = asName; }

	void SetFadeStart(bool abX) { mbFadeStart = abX; }
	bool GetFadeStart() { return mbFadeStart; }
	void SetFadeStop(bool abX) { mbFadeStop = abX; }
	bool GetFadeStop() { return mbFadeStop; }

	void SetVolume(float afX) { mfVolume = afX; }
	float GetVolume() { return mfVolume; }
	void SetMaxDistance(float afX) { mfMaxDistance = afX; }
	float GetMaxDistance() { return mfMaxDistance; }
	void SetMinDistance(float afX) { mfMinDistance = afX; }
	float GetMinDistance() { return mfMinDistance; }

	void SetStream(bool abX) { mbStream = abX; }
	bool GetStream() { return mbStream; }
	void SetLoop(bool abX) { mbLoop = abX; }
	bool GetLoop() { return mbLoop; }
	void SetUse3D(bool abX) { mbUse3D = abX; }
	bool GetUse3D() { return mbUse3D; }

	void SetBlockable(bool abX) { mbBlockable = abX; }
	bool GetBlockable() { return mbBlockable; }
	void SetBlockVolumeMul(float afX) { mfBlockVolumeMul = afX; }
	float GetBlockVolumeMul() { return mfBlockVolumeMul; }

	void SetRandom(float afX) { mfRandom = afX; }
	float GetRandom() { return mfRandom; }
	void SetInterval(float afX) { mfInterval = afX; }
	float GetInterval() { return mfInterval; }

	void SetPriority(int alX) { mlPriority = alX; }
	int GetPriority() { return mlPriority; }

	// Resource implementation
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

protected:
	tString msMainSound;
	tString msStartSound;
	tString msStopSound;

	bool mbFadeStart;
	bool mbFadeStop;

	float mfVolume;
	float mfMaxDistance;
	float mfMinDistance;

	bool mbStream;
	bool mbLoop;
	bool mbUse3D;

	bool mbBlockable;
	float mfBlockVolumeMul;

	float mfRandom;
	float mfInterval;

	int mlPriority;
};

} // namespace hpl

#endif // HPL_SOUND_DATA_H
