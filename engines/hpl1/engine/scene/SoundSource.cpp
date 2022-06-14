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

#include "hpl1/engine/scene/SoundSource.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundHandler.h"
#include "hpl1/engine/system/String.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSoundSource::cSoundSource(const tString &asName, const tString &asSoundName, cSound *apSound, bool abVolatile)
	: iEntity2D(asName) {
	UpdateBoundingBox();

	mpSound = apSound;
	msSoundName = asSoundName;
	mpSoundChannel = NULL;
	mbVolatile = abVolatile;

	msSoundName = asSoundName;

	mlInterval = 0;
	mbLoop = true;
	mbRelative = false;
	mfMaxDist = 1000;
	mfMinDist = 1;
	mlRandom = 0;
	mfVolume = 1;
	mlCounter = 0;

	mbPlaying = false;
}

//-----------------------------------------------------------------------

cSoundSource::~cSoundSource() {
	Stop();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSoundSource::UpdateLogic(float afTimeStep) {
	if (mbIsActive && !mbPlaying) {
		if (mlRandom == 0) {
			Play();
		} else {
			if (mlCounter >= mlInterval) {
				if (cMath::RandRectl(0, mlRandom) == 0) {
					Play();
					mlCounter = 0;
				}
			} else {
				mlCounter++;
			}
		}

		// Update position
		if (mbRelative == false) {
			if (mpSound->GetSoundHandler()->IsValid(mpSoundChannel)) {
				mpSoundChannel->SetPosition(GetWorldPosition());
			}
		}
	}
}

//-----------------------------------------------------------------------

bool cSoundSource::IsDead() {
	if (mbVolatile) {
		if (mbIsActive == false && mbPlaying == false)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSoundSource::LoadData(TiXmlElement *apRootElem) {
	mlInterval = cString::ToInt(apRootElem->Attribute("Interval"), 0);
	mbLoop = cString::ToBool(apRootElem->Attribute("Loop"), false);
	mbRelative = cString::ToBool(apRootElem->Attribute("Relative"), false);
	mfMaxDist = cString::ToFloat(apRootElem->Attribute("MaxDist"), 100);
	mfMinDist = cString::ToFloat(apRootElem->Attribute("MinDist"), 100);
	mlRandom = cString::ToInt(apRootElem->Attribute("Random"), 100);
	mfVolume = cString::ToFloat(apRootElem->Attribute("Volume"), 1);

	if (!mbRelative) {
		cVector3f vPos;
		vPos.x = cString::ToFloat(apRootElem->Attribute("X"), 0);
		vPos.y = cString::ToFloat(apRootElem->Attribute("Y"), 0);
		vPos.z = cString::ToFloat(apRootElem->Attribute("Z"), 0);

		SetPosition(vPos);
	} else {
		cVector3f vPos;
		vPos.x = cString::ToFloat(apRootElem->Attribute("RelX"), 0);
		vPos.y = cString::ToFloat(apRootElem->Attribute("RelY"), 0);
		vPos.z = cString::ToFloat(apRootElem->Attribute("RelZ"), 0);

		SetPosition(vPos);
	}

	return true;
}

//-----------------------------------------------------------------------

const cRect2f &cSoundSource::GetBoundingBox() {
	return mBoundingBox;
}

//-----------------------------------------------------------------------

bool cSoundSource::UpdateBoundingBox() {
	return true;
}

//-----------------------------------------------------------------------

void cSoundSource::Stop() {
	mpSound->GetSoundHandler()->Stop(msSoundName);

	mbIsActive = false;
	mbPlaying = false;
	mlCounter = 0;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSoundSource::Play() {
	cVector3f vPos;
	if (mbRelative)
		vPos = GetWorldPosition();
	else
		vPos = GetLocalPosition();

	mpSoundChannel = mpSound->GetSoundHandler()->Play(msSoundName, mbLoop, mfVolume, vPos, mfMinDist, mfMaxDist,
													  eSoundDest_World, mbRelative);

	mbPlaying = true;

	if (!mbLoop)
		mbIsActive = false;
}

//-----------------------------------------------------------------------

} // namespace hpl
