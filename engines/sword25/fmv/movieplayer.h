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

#ifndef SWORD25_MOVIEPLAYER_H
#define SWORD25_MOVIEPLAYER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "graphics/surface.h"

namespace Sword25 {

class TheoraDecoder;

class MoviePlayer : public Service {
public:
	// -----------------------------------------------------------------------------
	// Constructor / Destructor
	// -----------------------------------------------------------------------------

	MoviePlayer(Kernel *pKernel);
	~MoviePlayer() {};

	// -----------------------------------------------------------------------------
	// Abstract interface must be implemented by each Movie Player
	// -----------------------------------------------------------------------------

	/**
	 * Loads a movie file
	 *
	 * This method loads a movie file and prepares it for playback.
	 * There can be oly one movie file loaded at a time. If you already have loaded a
	 * movie file, it will be unloaded and, if necessary, stopped playing.
	 * @param Filename      The filename of the movie file to be loaded
	 * @param Z             Z indicates the position of the film on the main graphics layer
	 * @return              Returns false if an error occured while loading, otherwise true.
	*/
	bool LoadMovie(const Common::String &Filename, unsigned int Z);

	/**
	 * Unloads the currently loaded movie file.
	 * @return              Returns false if an error occurred while unloading, otherwise true.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	bool UnloadMovie();

	/**
	 * Plays the loaded movie.
	 *
	 * The film will be keeping the aspect ratio of the screen.
	 * If the film was previously paused with Pause(), then the film will resume playing.
	 * @return              Returns false if an error occurred while starting, otherwise true.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	bool Play();

	/**
	 * Pauses movie playback.
	 *
	 * A paused movie can later be resumed by calling the Play() method again.
	 * @return              Returns false if an error occurred while pausing, otherwise true.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	bool Pause();

	/**
	 * This function must be called once per frame.
	 */
	void Update();

	/**
	 * Returns whether a film is loaded for playback.
	 */
	bool IsMovieLoaded();

	/**
	 * Returns whether the movie playback is paused.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	*/
	bool IsPaused();

	/**
	 * Returns the scaling factor for the loaded film.
	 *
	 * When a movie is loaded, the scaling factor is automatically selected so that the film
	 * takes the maximum screen space, without the film being distorted.
	 * @return              Returns the scaling factor of the film.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	float GetScaleFactor();

	/**
	 * Sets the factor by which the loaded film is to be scaled.
	 * @param ScaleFactor   The desired scale factor.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	void SetScaleFactor(float ScaleFactor);

	/**
	 * Returns the current playing position in seconds.
	 * @remark              This method can only be called when IsMovieLoaded() returns true.
	 */
	double GetTime();

private:
	bool _RegisterScriptBindings();

	TheoraDecoder *_decoder;
	Graphics::Surface *_backSurface;
};

} // End of namespace Sword25

#endif
