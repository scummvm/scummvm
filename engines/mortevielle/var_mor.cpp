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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/*---------------------------------------------------------------------------*/
/*------------------------------   VARIABLES   ------------------------------*/
/*---------------------------------------------------------------------------*/

int g_x,
        g_y,
        g_t,
        g_jh,
        g_mh,
        g_cs,
        g_hdb,
        g_hfb,
        g_key,
        g_num,
        g_res,
        g_ment,
        g_haut,
        g_caff,
        g_maff,
        g_crep,
        g_ades,
        g_iouv,
		g_ctrm,
        g_dobj,
        g_mlec,
        g_mchai,
        g_mpers,
        g_perdep,
        g_prebru;

/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  AND  FONCTIONS   -----------------------*/
/*---------------------------------------------------------------------------*/

void musyc(tablint &tb, int nbseg, int att) {
	warning("TODO: musyc");
}

} // End of namespace Mortevielle
