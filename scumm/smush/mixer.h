/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SMUSH_MIXER_H
#define SMUSH_MIXER_H

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_MIXER
#  define DEBUG_MIXER
# endif
#else
# ifdef DEBUG_MIXER
#  error DEBUG_MIXER defined without DEBUG
# endif
#endif

class _Channel;

class SoundRenderer;

/*! 	@brief The class for the player's sound mixer

	This class is used for sound mixing.
	It contains a list of current track and request them to mix.
	It then sends the mixed sound samples to the sound renderer.

*/
class Mixer {
public:
	virtual ~Mixer() {};
	virtual bool init() = 0;
	virtual _Channel * findChannel(int track) = 0;
	virtual bool addChannel(_Channel * c) = 0;
	virtual bool handleFrame() = 0;
	virtual bool stop() = 0;
};

#endif
