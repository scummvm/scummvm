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

#include "hpl1/engine/impl/OpenALSoundData.h"
#include "audio/audiostream.h"
#include "hpl1/debug.h"
#include "hpl1/engine/impl/OpenALSoundChannel.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/system/low_level_system.h"
namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cOpenALSoundData::cOpenALSoundData(tString asName, bool abStream)
	: iSoundData(asName, abStream) {
}

//-----------------------------------------------------------------------

cOpenALSoundData::~cOpenALSoundData() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cOpenALSoundData::CreateFromFile(const tString &filename) {
	if (_filename != "")
		error("trying to load a sample"); // FIXME: remove this if its not needed
	// FIXME: string types
	_filename = filename.c_str();
	return true;
}

//-----------------------------------------------------------------------

iSoundChannel *cOpenALSoundData::CreateChannel(int alPriority) {
	IncUserCount();
	return hplNew(cOpenALSoundChannel, (this, Audio::SeekableAudioStream::openStreamFile(_filename.substr(0, _filename.size() - 4)), mpSoundManger));
}

} // namespace hpl
