/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/fmv/movieplayer.h"
#include "sword25/fmv/theora_decoder.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define BS_LOG_PREFIX "MOVIEPLAYER"

Service *OggTheora_CreateObject(Kernel *pKernel) {
	return new MoviePlayer(pKernel);
}

MoviePlayer::MoviePlayer(Kernel *pKernel) : Service(pKernel) {
	if (!_RegisterScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");

	_decoder = new TheoraDecoder();
}

bool MoviePlayer::LoadMovie(const Common::String &filename, unsigned int Z) {
	Common::SeekableReadStream *in = Kernel::GetInstance()->GetPackage()->GetStream(filename);

	if (!in) {
		BS_LOG_ERRORLN("Could not open movie file \"%s\".", filename.c_str());
		return false;
	}

	if (!_decoder->load(in)) {
		BS_LOG_ERRORLN("Could not load movie file \"%s\".", filename.c_str());
		return false;
	}

	return true;
}

bool MoviePlayer::UnloadMovie() {
	_decoder->close();

	return true;
}

bool MoviePlayer::Play() {
	return true;
}

bool MoviePlayer::Pause() {
	return true;
}

void MoviePlayer::Update() {
}

bool MoviePlayer::IsMovieLoaded() {
	return true;
}

bool MoviePlayer::IsPaused() {
	return true;
}

float MoviePlayer::GetScaleFactor() {
	return 1.0f;
}

void MoviePlayer::SetScaleFactor(float ScaleFactor) {
}

double MoviePlayer::GetTime() {
	return 1.0;
}


} // End of namespace Sword25
