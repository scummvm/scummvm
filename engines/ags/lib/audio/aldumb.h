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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * aldumb.h - The user header file for DUMB with      / / \  \
 *            Allegro.                               | <  /   \_
 *                                                   |  \/ /\   /
 * Include this file if you wish to use DUMB          \_  /  > /
 * with Allegro. It will include dumb.h for you,        | \ / /
 * and provide extra functionality such as audio        |  ' /
 * stream and datafile integration.                      \__/
 */

#ifndef AGS_LIB_ALDUMB_H
#define AGS_LIB_ALDUMB_H

#include "ags/lib/allegro.h"
#include "ags/lib/dumb-0.9.2/dumb.h"

namespace AGS3 {

/* Packfile Support */

void dumb_register_packfiles(void);

DUMBFILE *dumbfile_open_packfile(PACKFILE *p);
DUMBFILE *dumbfile_from_packfile(PACKFILE *p);


/* Datafile Registration Functions */

#define DUMB_DAT_DUH DAT_ID('D','U','H',' ')
#define DUMB_DAT_IT  DAT_ID('I','T',' ',' ')
#define DUMB_DAT_XM  DAT_ID('X','M',' ',' ')
#define DUMB_DAT_S3M DAT_ID('S','3','M',' ')
#define DUMB_DAT_MOD DAT_ID('M','O','D',' ')

void dumb_register_dat_duh(long type);
void dumb_register_dat_it(long type);
void dumb_register_dat_xm(long type);
void dumb_register_dat_s3m(long type);
void dumb_register_dat_mod(long type);
void dumb_register_dat_it_quick(long type);
void dumb_register_dat_xm_quick(long type);
void dumb_register_dat_s3m_quick(long type);
void dumb_register_dat_mod_quick(long type);


/* DUH Playing Functions */

typedef struct AL_DUH_PLAYER AL_DUH_PLAYER;

AL_DUH_PLAYER *al_start_duh(DUH *duh, int n_channels, long pos, float volume, long bufsize, int freq);
void al_stop_duh(AL_DUH_PLAYER *dp);
void al_pause_duh(AL_DUH_PLAYER *dp);
void al_resume_duh(AL_DUH_PLAYER *dp);
void al_duh_set_priority(AL_DUH_PLAYER *dp, int priority);
void al_duh_set_volume(AL_DUH_PLAYER *dp, float volume);
float al_duh_get_volume(AL_DUH_PLAYER *dp);
int al_poll_duh(AL_DUH_PLAYER *dp);
long al_duh_get_position(AL_DUH_PLAYER *dp);

AL_DUH_PLAYER *al_duh_encapsulate_sigrenderer(DUH_SIGRENDERER *sigrenderer, float volume, long bufsize, int freq);
DUH_SIGRENDERER *al_duh_get_sigrenderer(AL_DUH_PLAYER *dp);

/* IMPORTANT: This function will return NULL if the music has ended. */
DUH_SIGRENDERER *al_duh_decompose_to_sigrenderer(AL_DUH_PLAYER *dp);

#ifdef DUMB_DECLARE_DEPRECATED

	AL_DUH_PLAYER *al_duh_encapsulate_renderer(DUH_SIGRENDERER *dr, float volume, long bufsize, int freq) DUMB_DEPRECATED;
	DUH_SIGRENDERER *al_duh_get_renderer(AL_DUH_PLAYER *dp) DUMB_DEPRECATED;
	DUH_SIGRENDERER *al_duh_decompose_to_renderer(AL_DUH_PLAYER *dp) DUMB_DEPRECATED;
	/* Replace 'renderer' with 'sigrenderer' in each case where you called one of
	 * these functions.
	 */

#endif

} // namespace AGS3

#endif
