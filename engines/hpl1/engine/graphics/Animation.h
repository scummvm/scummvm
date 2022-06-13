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
#ifndef HPL_ANIMATION_H
#define HPL_ANIMATION_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"

namespace hpl {

	class cAnimationTrack;

	typedef std::vector<cAnimationTrack*> tAnimationTrackVec;
	typedef tAnimationTrackVec::iterator tAnimationTrackVecIt;

	class cAnimation : public iResourceBase
	{
	public:
		cAnimation(const tString &asName, const tString &asFile);
		~cAnimation();

		float GetLength();
		void SetLength(float afTime);

		cAnimationTrack* CreateTrack(const tString &asName, tAnimTransformFlag aFlags);
		cAnimationTrack* GetTrack(int alIndex);
		cAnimationTrack* GetTrackByName(const tString &asName);
		void ResizeTracks(int alNum);
		int GetTrackNum();

		const char* GetAnimationName(){ return msAnimName.c_str();}
		void SetAnimationName(const tString &asName){ msAnimName =asName;}

		tString& GetFileName(){ return msFileName;}

		//Resources implementation
		bool Reload(){ return false;}
		void Unload(){}
		void Destroy(){}

	private:
		tString msAnimName;
		tString msFileName;

		float mfLength;

		tAnimationTrackVec mvTracks;
	};

};
#endif // HPL_ANIMATION_H
