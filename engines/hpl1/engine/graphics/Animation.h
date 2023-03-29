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

#ifndef HPL_ANIMATION_H
#define HPL_ANIMATION_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cAnimationTrack;

typedef Common::Array<cAnimationTrack *> tAnimationTrackVec;
typedef tAnimationTrackVec::iterator tAnimationTrackVecIt;

class cAnimation : public iResourceBase {
public:
	cAnimation(const tString &asName, const tString &asFile);
	~cAnimation();

	float GetLength();
	void SetLength(float afTime);

	cAnimationTrack *CreateTrack(const tString &asName, tAnimTransformFlag aFlags);
	cAnimationTrack *GetTrack(int alIndex);
	cAnimationTrack *GetTrackByName(const tString &asName);
	void ResizeTracks(int alNum);
	int GetTrackNum();

	const char *GetAnimationName() { return msAnimName.c_str(); }
	void SetAnimationName(const tString &asName) { msAnimName = asName; }

	tString &GetFileName() { return msFileName; }

	// Resources implementation
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

private:
	tString msAnimName;
	tString msFileName;

	float mfLength;

	tAnimationTrackVec mvTracks;
};

} // namespace hpl

#endif // HPL_ANIMATION_H
