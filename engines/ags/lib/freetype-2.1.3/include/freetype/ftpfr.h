/***************************************************************************/
/*                                                                         */
/*  ftpfr.h                                                                */
/*                                                                         */
/*    FreeType API for accessing PFR-specific data                         */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTPFR_H__
#define __FTPFR_H__

#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    pfr_fonts                                                          */
/*                                                                       */
/* <Title>                                                               */
/*    PFR Fonts                                                          */
/*                                                                       */
/* <Abstract>                                                            */
/*    PFR/TrueDoc specific APIs                                          */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains the declaration of PFR-specific functions.   */
/*                                                                       */
/*************************************************************************/


/**********************************************************************
 *
 * @function:
 *    FT2_1_3_Get_PFR_Metrics
 *
 * @description:
 *    returns the outline and metrics resolutions of a given PFR
 *    face.
 *
 * @input:
 *    face :: handle to input face. It can be a non-PFR face.
 *
 * @output:
 *    aoutline_resolution ::
 *      outline resolution. This is equivalent to "face->units_per_EM".
 *      optional (parameter can be NULL)
 *
 *    ametrics_resolution ::
 *      metrics_resolution. This is equivalent to "outline_resolution"
 *      for non-PFR fonts. can be NULL
 *      optional (parameter can be NULL)
 *
 *    ametrics_x_scale ::
 *      a 16.16 fixed-point number used to scale distance expressed
 *      in metrics units to device sub-pixels. This is equivalent to
 *      'face->size->x_scale', but for metrics only.
 *      optional (parameter can be NULL)
 *
 *    ametrics_y_scale ::
 *      same as 'ametrics_x_scale', but for the vertical direction.
 *      optional (parameter can be NULL)
 *
 * @note:
 *   if the input face is not a PFR, this function will return an error.
 *   However, in all cases, it will return valid values.
 */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Get_PFR_Metrics( FT2_1_3_Face     face,
                    FT2_1_3_UInt    *aoutline_resolution,
                    FT2_1_3_UInt    *ametrics_resolution,
                    FT2_1_3_Fixed   *ametrics_x_scale,
                    FT2_1_3_Fixed   *ametrics_y_scale );

/**********************************************************************
 *
 * @function:
 *    FT2_1_3_Get_PFR_Kerning
 *
 * @description:
 *    returns the kerning pair corresponding to two glyphs in
 *    a PFR face. The distance is expressed in metrics units, unlike
 *    the result of @FT2_1_3_Get_Kerning.
 *
 * @input:
 *    face :: handle to input face.
 *    left  :: left glyph index
 *    right :: right glyph index
 *
 * @output:
 *    avector :: kerning vector
 *
 * @note:
 *    this function always return distances in original PFR metrics
 *    units. This is unlike @FT2_1_3_Get_Kerning with the @FT2_1_3_KERNING_UNSCALED
 *    mode, which always return distances converted to outline units.
 *
 *    you can use the value of the 'x_scale' and 'y_scale' parameters
 *    returned by @FT2_1_3_Get_PFR_Metrics to scale these to device sub-pixels
 */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Get_PFR_Kerning( FT2_1_3_Face     face,
                    FT2_1_3_UInt     left,
                    FT2_1_3_UInt     right,
                    FT2_1_3_Vector  *avector );

/**********************************************************************
 *
 * @function:
 *    FT2_1_3_Get_PFR_Advance
 *
 * @description:
 *    returns a given glyph advance, expressed in original metrics units,
 *    from a PFR font.
 *
 * @input:
 *    face   :: handle to input face.
 *    gindex :: glyph index
 *
 * @output:
 *    aadvance :: glyph advance in metrics units
 *
 * @return:
 *    error code. 0 means success
 *
 * @note:
 *    you can use the 'x_scale' or 'y_scale' results of @FT2_1_3_Get_PFR_Metrics
 *    to convert the advance to device sub-pixels (i.e. 1/64th of pixels)
 */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Get_PFR_Advance( FT2_1_3_Face    face,
                    FT2_1_3_UInt    gindex,
                    FT2_1_3_Pos    *aadvance );

/* */

FT2_1_3_END_HEADER

#endif /* __FTBDF_H__ */


/* END */
