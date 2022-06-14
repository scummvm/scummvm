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

#include "hpl1/engine/graphics/Animation.h"

#include "hpl1/engine/graphics/AnimationTrack.h"
#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAnimation::cAnimation(const tString &asName, const tString &asFile) : iResourceBase(asName, 0) {
	msAnimName = "";
	msFileName = asFile;
}

//-----------------------------------------------------------------------

cAnimation::~cAnimation() {
	STLDeleteAll(mvTracks);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

float cAnimation::GetLength() {
	return mfLength;
}

//-----------------------------------------------------------------------

void cAnimation::SetLength(float afTime) {
	mfLength = afTime;
}

//-----------------------------------------------------------------------

cAnimationTrack *cAnimation::CreateTrack(const tString &asName, tAnimTransformFlag aFlags) {
	cAnimationTrack *pTrack = hplNew(cAnimationTrack, (asName, aFlags, this));

	mvTracks.push_back(pTrack);

	return pTrack;
}

//-----------------------------------------------------------------------

cAnimationTrack *cAnimation::GetTrack(int alIndex) {
	return mvTracks[alIndex];
}

//-----------------------------------------------------------------------

cAnimationTrack *cAnimation::GetTrackByName(const tString &asName) {
	for (size_t i = 0; i < mvTracks.size(); ++i) {
		if (asName == tString(mvTracks[i]->GetName())) {
			return mvTracks[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cAnimation::ResizeTracks(int alNum) {
	mvTracks.reserve(alNum);
}

//-----------------------------------------------------------------------

int cAnimation::GetTrackNum() {
	return (int)mvTracks.size();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
