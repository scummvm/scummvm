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


#ifndef __AHOPTIM_H__
#define __AHOPTIM_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "ahtypes.h"


FT2_1_3_BEGIN_HEADER


/* the maximal number of stem configurations to record */
/* during optimization                                 */
#define AH_MAX_CONFIGS  8


typedef struct  AH_Stem_ {
	FT2_1_3_Pos   pos;       /* current position        */
	FT2_1_3_Pos   velocity;  /* current velocity        */
	FT2_1_3_Pos   force;     /* sum of current forces   */
	FT2_1_3_Pos   width;     /* normalized width        */

	FT2_1_3_Pos   min_pos;   /* minimum grid position */
	FT2_1_3_Pos   max_pos;   /* maximum grid position */

	AH_Edge  edge1;     /* left/bottom edge */
	AH_Edge  edge2;     /* right/top edge   */

	FT2_1_3_Pos   opos;      /* original position */
	FT2_1_3_Pos   owidth;    /* original width    */

	FT2_1_3_Pos   min_coord; /* minimum coordinate */
	FT2_1_3_Pos   max_coord; /* maximum coordinate */

} AH_Stem;


/* A spring between two stems */
typedef struct  AH_Spring_ {
	AH_Stem*  stem1;
	AH_Stem*  stem2;
	FT2_1_3_Pos    owidth;   /* original width  */
	FT2_1_3_Pos    tension;  /* current tension */

} AH_Spring;


/* A configuration records the position of each stem at a given time  */
/* as well as the associated distortion                               */
typedef struct AH_Configuration_ {
	FT2_1_3_Pos*  positions;
	FT2_1_3_Long  distortion;

} AH_Configuration;


typedef struct  AH_Optimizer_ {
	FT2_1_3_Memory         memory;
	AH_Outline        outline;

	FT2_1_3_Int            num_hstems;
	AH_Stem*          horz_stems;

	FT2_1_3_Int            num_vstems;
	AH_Stem*          vert_stems;

	FT2_1_3_Int            num_hsprings;
	FT2_1_3_Int            num_vsprings;
	AH_Spring*        horz_springs;
	AH_Spring*        vert_springs;

	FT2_1_3_Int            num_configs;
	AH_Configuration  configs[AH_MAX_CONFIGS];
	FT2_1_3_Pos*           positions;

	/* during each pass, use these instead */
	FT2_1_3_Int            num_stems;
	AH_Stem*          stems;

	FT2_1_3_Int            num_springs;
	AH_Spring*        springs;
	FT2_1_3_Bool           vertical;

	FT2_1_3_Fixed          tension_scale;
	FT2_1_3_Pos            tension_threshold;

} AH_Optimizer;


/* loads the outline into the optimizer */
int
AH_Optimizer_Init( AH_Optimizer*  optimizer,
                   AH_Outline     outline,
                   FT2_1_3_Memory      memory );


/* compute optimal outline */
void
AH_Optimizer_Compute( AH_Optimizer*  optimizer );


/* release the optimization data */
void
AH_Optimizer_Done( AH_Optimizer*  optimizer );


FT2_1_3_END_HEADER

#endif /* __AHOPTIM_H__ */


/* END */
