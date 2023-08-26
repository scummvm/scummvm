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

/***************************************************************************/
/*                                                                         */
/*  ahoptim.h                                                              */
/*                                                                         */
/*    FreeType auto hinting outline optimization (declaration).            */
/*                                                                         */
/*  Copyright 2000-2001 Catharon Productions Inc.                          */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  `CatharonLicense.txt'.  By continuing to use, modify, or distribute    */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license.        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_AHOPTIM_H
#define AGS_LIB_FREETYPE_AHOPTIM_H


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/modules/autohint/ahtypes.h"

namespace AGS3 {
namespace FreeType213 {

// FT_BEGIN_HEADER


/* the maximal number of stem configurations to record */
/* during optimization                                 */
#define AH_MAX_CONFIGS  8


typedef struct  AH_Stem_ {
	FT_Pos   pos;       /* current position        */
	FT_Pos   velocity;  /* current velocity        */
	FT_Pos   force;     /* sum of current forces   */
	FT_Pos   width;     /* normalized width        */

	FT_Pos   min_pos;   /* minimum grid position */
	FT_Pos   max_pos;   /* maximum grid position */

	AH_Edge  edge1;     /* left/bottom edge */
	AH_Edge  edge2;     /* right/top edge   */

	FT_Pos   opos;      /* original position */
	FT_Pos   owidth;    /* original width    */

	FT_Pos   min_coord; /* minimum coordinate */
	FT_Pos   max_coord; /* maximum coordinate */
} AH_Stem;


/* A spring between two stems */
typedef struct  AH_Spring_ {
	AH_Stem   *stem1;
	AH_Stem   *stem2;
	FT_Pos    owidth;   /* original width  */
	FT_Pos    tension;  /* current tension */
} AH_Spring;


/* A configuration records the position of each stem at a given time  */
/* as well as the associated distortion                               */
typedef struct AH_Configuration_ {
	FT_Pos   *positions;
	FT_Long  distortion;
} AH_Configuration;


typedef struct  AH_Optimizer_ {
	FT_Memory         memory;
	AH_Outline        outline;

	FT_Int            num_hstems;
	AH_Stem           *horz_stems;

	FT_Int            num_vstems;
	AH_Stem           *vert_stems;

	FT_Int            num_hsprings;
	FT_Int            num_vsprings;
	AH_Spring         *horz_springs;
	AH_Spring         *vert_springs;

	FT_Int            num_configs;
	AH_Configuration  configs[AH_MAX_CONFIGS];
	FT_Pos            *positions;

	/* during each pass, use these instead */
	FT_Int            num_stems;
	AH_Stem           *stems;

	FT_Int            num_springs;
	AH_Spring         *springs;
	FT_Bool           vertical;

	FT_Fixed          tension_scale;
	FT_Pos            tension_threshold;
} AH_Optimizer;


/* loads the outline into the optimizer */
int AH_Optimizer_Init(AH_Optimizer *optimizer, AH_Outline outline, FT_Memory memory);

/* compute optimal outline */
void AH_Optimizer_Compute(AH_Optimizer *optimizer);

/* release the optimization data */
void AH_Optimizer_Done(AH_Optimizer *optimizer);


// FT_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AHOPTIM_H */
