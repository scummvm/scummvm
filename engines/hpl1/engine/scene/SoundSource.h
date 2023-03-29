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

#ifndef HPL_SOUNDSOURCE_H
#define HPL_SOUNDSOURCE_H

#include "common/list.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/system/SystemTypes.h"

class TiXmlElement;

namespace hpl {

class cSound;
class iSoundChannel;

class cSoundSource : public iEntity2D {
public:
	cSoundSource(const tString &asName, const tString &asSoundName, cSound *apSound, bool abVolatile);
	~cSoundSource();

	tString GetEntityType() { return "SoundSource"; }

	void UpdateLogic(float afTimeStep);

	bool LoadData(TiXmlElement *apRootElem);

	bool IsDead();

	void SetInterval(int alX) { mlInterval = alX; }
	void SetLoop(bool abX) { mbLoop = abX; }
	void SetRelative(bool abX) { mbRelative = abX; }
	void SetMaxDist(float afX) { mfMaxDist = afX; }
	void SetMinDist(float afX) { mfMinDist = afX; }
	void SetRandom(int alX) { mlRandom = alX; }
	void SetVolume(float afX) { mfVolume = afX; }

	iSoundChannel *GetSoundChannel() { return mpSoundChannel; }

	const cRect2f &GetBoundingBox();
	bool UpdateBoundingBox();

	void Stop();

private:
	cSound *mpSound;
	bool mbVolatile;

	int mlInterval;
	bool mbLoop;
	bool mbRelative;
	float mfMaxDist;
	float mfMinDist;
	int mlRandom;
	tString msSoundName;
	float mfVolume;
	int mlCounter;

	bool mbPlaying;

	iSoundChannel *mpSoundChannel;

	void Play();
};

typedef Common::List<cSoundSource *> tSoundSourceList;
typedef tSoundSourceList::iterator tSoundSourceListIt;

} // namespace hpl

#endif // HPL_SOUNDSOURCE_H
