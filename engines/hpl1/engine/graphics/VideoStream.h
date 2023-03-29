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

#ifndef HPL_VIDEO_STREAM_H
#define HPL_VIDEO_STREAM_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"

namespace hpl {

//-----------------------------------------

class iTexture;

class iVideoStream;

//-----------------------------------------

class iVideoStreamLoader {
public:
	virtual ~iVideoStreamLoader() {}

	virtual iVideoStream *Create(const tString &asName) = 0;

	tStringVec &GetExtensions() { return mvExtensions; }

protected:
	tStringVec mvExtensions;
};

//-----------------------------------------

class iVideoStream : public iResourceBase {
public:
	iVideoStream(tString asName) : iResourceBase(asName, 0) {}
	virtual ~iVideoStream() {}

	virtual bool LoadFromFile(tString asFilePath) = 0;

	virtual void Update(float afTimeStep) = 0;

	virtual void Play() = 0;
	virtual void Stop() = 0;

	virtual void Pause(bool abX) = 0;
	virtual bool IsPaused() = 0;

	virtual void SetLoop(bool abX) = 0;
	virtual bool IsLooping() = 0;

	virtual void CopyToTexture(iTexture *apTexture) = 0;

	const tString &GetFileName() { return msFilePath; }
	const cVector2l &GetSize() { return mvSize; }

	//////////////////////////////////
	// ResourceBase implementation
	bool reload() { return false; }
	void unload() {}
	void destroy() {}

protected:
	tString msFilePath;
	cVector2l mvSize;
};

} // namespace hpl

#endif // HPL_VIDEO_STREAM_H
