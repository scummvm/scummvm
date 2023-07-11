/***************************************************************************/
/*                                                                         */
/*  ftbdf.h                                                                */
/*                                                                         */
/*    FreeType API for accessing BDF-specific strings (specification).     */
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


#ifndef AGS_LIB_FREETYPE_FTBDF_H
#define AGS_LIB_FREETYPE_FTBDF_H

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    bdf_fonts                                                          */
/*                                                                       */
/* <Title>                                                               */
/*    BDF Fonts                                                          */
/*                                                                       */
/* <Abstract>                                                            */
/*    BDF-specific APIs                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains the declaration of BDF-specific functions.   */
/*                                                                       */
/*************************************************************************/


/**********************************************************************
 *
 * @function:
 *    FT_Get_BDF_Charset_ID
 *
 * @description:
 *    Retrieves a BDF font character set identity, according to
 *    the BDF specification.
 *
 * @input:
 *    face ::
 *       handle to input face
 *
 * @output:
 *    acharset_encoding ::
 *       Charset encoding, as a C string, owned by the face.
 *
 *    acharset_registry ::
 *       Charset registry, as a C string, owned by the face.
 *
 * @return:
 *   FreeType rror code.  0 means success.
 *
 * @note:
 *   This function only works with BDF faces, returning an error otherwise.
 */
FT2_1_3_EXPORT( FT_Error )
FT_Get_BDF_Charset_ID( FT_Face       face,
					   const char*  *acharset_encoding,
					   const char*  *acharset_registry );

/* */

FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTBDF_H */


/* END */
