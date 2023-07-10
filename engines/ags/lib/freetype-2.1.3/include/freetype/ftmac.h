/***************************************************************************/
/*                                                                         */
/*  ftmac.h                                                                */
/*                                                                         */
/*    Additional Mac-specific API.                                         */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  Just van Rossum, David Turner, Robert Wilhelm, and Werner Lemberg.     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* NOTE: Include this file after <freetype/freetype.h> and after the       */
/*       Mac-specific <Types.h> header (or any other Mac header that       */
/*       includes <Types.h>); we use Handle type.                          */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTMAC_H
#define AGS_LIB_FREETYPE_FTMAC_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    mac_specific                                                       */
/*                                                                       */
/* <Title>                                                               */
/*    Mac-Specific Interface                                             */
/*                                                                       */
/* <Abstract>                                                            */
/*    Only available on the Macintosh.                                   */
/*                                                                       */
/* <Description>                                                         */
/*    The following definitions are only available if FreeType is        */
/*    compiled on a Macintosh.                                           */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_New_Face_From_FOND                                              */
/*                                                                       */
/* <Description>                                                         */
/*    Creates a new face object from an FOND resource.                   */
/*                                                                       */
/* <InOut>                                                               */
/*    library    :: A handle to the library resource.                    */
/*                                                                       */
/* <Input>                                                               */
/*    fond       :: An FOND resource.                                    */
/*                                                                       */
/*    face_index :: Only supported for the -1 `sanity check' special     */
/*                  case.                                                */
/*                                                                       */
/* <Output>                                                              */
/*    aface      :: A handle to a new face object.                       */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Notes>                                                               */
/*    This function can be used to create FT_Face abjects from fonts     */
/*    that are installed in the system like so:                          */
/*                                                                       */
/*    {                                                                  */
/*      fond = GetResource( 'FOND', fontName );                          */
/*      error = FT_New_Face_From_FOND( library, fond, 0, &face );        */
/*    }                                                                  */
/*                                                                       */
FT2_1_3_EXPORT( FT_Error )
FT_New_Face_From_FOND( FT_Library  library,
					   Handle      fond,
					   FT_Long     face_index,
					   FT_Face    *aface );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT_GetFile_From_Mac_Name                                           */
/*                                                                       */
/* <Description>                                                         */
/*    Returns an FSSpec for the disk file containing the named font.     */
/*                                                                       */
/* <Input>                                                               */
/*    fontName   :: Mac OS name of the font (eg. Times New Roman Bold).  */
/*                                                                       */
/* <Output>                                                              */
/*    pathSpec   :: FSSpec to the file.  For passing to @FT2_1_3_New_Face.    */
/*                                                                       */
/*    face_index :: Index of the face.  For passing to @FT2_1_3_New_Face.     */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT_DEF( FT_Error )
FT_GetFile_From_Mac_Name( char*     fontName,
						  FSSpec*   pathSpec,
						  FT_Long*  face_index );

/* */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTMAC_H */


/* END */
