/***************************************************************************/
/*                                                                         */
/*  ftbbox.h                                                               */
/*                                                                         */
/*    FreeType exact bbox computation (specification).                     */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This component has a _single_ role: to compute exact outline bounding */
/* boxes.                                                                */
/*                                                                       */
/* It is separated from the rest of the engine for various technical     */
/* reasons.  It may well be integrated in `ftoutln' later.               */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTBBOX_H
#define AGS_LIB_FREETYPE_FTBBOX_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    outline_processing                                                 */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Outline_Get_BBox                                                */
/*                                                                       */
/* <Description>                                                         */
/*    Computes the exact bounding box of an outline.  This is slower     */
/*    than computing the control box.  However, it uses an advanced      */
/*    algorithm which returns _very_ quickly when the two boxes          */
/*    coincide.  Otherwise, the outline Bezier arcs are walked over to   */
/*    extract their extrema.                                             */
/*                                                                       */
/* <Input>                                                               */
/*    outline :: A pointer to the source outline.                        */
/*                                                                       */
/* <Output>                                                              */
/*    abbox   :: The outline's exact bounding box.                       */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Outline_Get_BBox(FT2_1_3_Outline *outline,
						 FT2_1_3_BBox *abbox);

/* */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTBBOX_H */


/* END */
