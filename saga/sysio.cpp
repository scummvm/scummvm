/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "reinherit.h"

#include "SDL.h"

namespace Saga {

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/

/*
 * Begin module component
\*--------------------------------------------------------------------------*/

int SYSIO_Init(void)
{

	int result;

	/* Initialize SDL library */
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);

	if (result != 0) {

		R_printf(R_STDERR, "SDL library initialization failed.\n");

		return R_FAILURE;
	}

	R_printf(R_STDOUT, "SDL library initialized.\n");

	return R_SUCCESS;
}

int SYSIO_Shutdown(void)
{

	return R_SUCCESS;
}

} // End of namespace Saga

