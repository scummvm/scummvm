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

#include "yslib.h"

namespace Saga {

static int MusicInitialized = 0;

int SYSMUSIC_Init(int enabled)
{
	YS_IGNORE_PARAM(enabled);

	if (MusicInitialized) {
		return R_FAILURE;
	}

	MusicInitialized = 1;
	return R_SUCCESS;
}

int SYSMUSIC_Shutdown(void)
{
	if (!MusicInitialized) {
		return R_FAILURE;
	}

	MusicInitialized = 0;
	return R_SUCCESS;
}

int SYSMUSIC_Play(ulong music_rn, uint flags)
{
	if (!MusicInitialized) {
		return R_FAILURE;
	}

	YS_IGNORE_PARAM(music_rn);
	YS_IGNORE_PARAM(flags);

	return R_SUCCESS;

}

int SYSMUSIC_Pause(void)
{
	if (!MusicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;

}

int SYSMUSIC_Resume(void)
{
	if (!MusicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;

}

int SYSMUSIC_Stop(void)
{

	if (!MusicInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;

}

} // End of namespace Saga

