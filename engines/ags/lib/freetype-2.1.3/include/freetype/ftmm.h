/***************************************************************************/
/*                                                                         */
/*  ftmm.h                                                                 */
/*                                                                         */
/*    FreeType Multiple Master font interface (specification).             */
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


#ifndef __FTMM_H__
#define __FTMM_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/t1tables.h"


FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    multiple_masters                                                   */
/*                                                                       */
/* <Title>                                                               */
/*    Multiple Masters                                                   */
/*                                                                       */
/* <Abstract>                                                            */
/*    How to manage Multiple Masters fonts.                              */
/*                                                                       */
/* <Description>                                                         */
/*    The following types and functions are used to manage Multiple      */
/*    Master fonts, i.e. the selection of specific design instances by   */
/*    setting design axis coordinates.                                   */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_MM_Axis                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    A simple structure used to model a given axis in design space for  */
/*    Multiple Masters fonts.                                            */
/*                                                                       */
/* <Fields>                                                              */
/*    name    :: The axis's name.                                        */
/*                                                                       */
/*    minimum :: The axis's minimum design coordinate.                   */
/*                                                                       */
/*    maximum :: The axis's maximum design coordinate.                   */
/*                                                                       */
typedef struct  FT2_1_3_MM_Axis_ {
	FT2_1_3_String*  name;
	FT2_1_3_Long     minimum;
	FT2_1_3_Long     maximum;

} FT2_1_3_MM_Axis;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_Multi_Master                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    A structure used to model the axes and space of a Multiple Masters */
/*    font.                                                              */
/*                                                                       */
/* <Fields>                                                              */
/*    num_axis    :: Number of axes.  Cannot exceed 4.                   */
/*                                                                       */
/*    num_designs :: Number of designs; should ne normally 2^num_axis    */
/*                   even though the Type 1 specification strangely      */
/*                   allows for intermediate designs to be present. This */
/*                   number cannot exceed 16.                            */
/*                                                                       */
/*    axis        :: A table of axis descriptors.                        */
/*                                                                       */
typedef struct  FT2_1_3_Multi_Master_ {
	FT2_1_3_UInt     num_axis;
	FT2_1_3_UInt     num_designs;
	FT2_1_3_MM_Axis  axis[T1_MAX_MM_AXIS];

} FT2_1_3_Multi_Master;

/* */

typedef FT2_1_3_Error
(*FT2_1_3_Get_MM_Func)( FT2_1_3_Face           face,
                   FT2_1_3_Multi_Master*  master );

typedef FT2_1_3_Error
(*FT2_1_3_Set_MM_Design_Func)( FT2_1_3_Face   face,
                          FT2_1_3_UInt   num_coords,
                          FT2_1_3_Long*  coords );

typedef FT2_1_3_Error
(*FT2_1_3_Set_MM_Blend_Func)( FT2_1_3_Face   face,
                         FT2_1_3_UInt   num_coords,
                         FT2_1_3_Long*  coords );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Get_Multi_Master                                                */
/*                                                                       */
/* <Description>                                                         */
/*    Retrieves the Multiple Master descriptor of a given font.          */
/*                                                                       */
/* <Input>                                                               */
/*    face    :: A handle to the source face.                            */
/*                                                                       */
/* <Output>                                                              */
/*    amaster :: The Multiple Masters descriptor.                        */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Get_Multi_Master( FT2_1_3_Face           face,
                     FT2_1_3_Multi_Master  *amaster );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Set_MM_Design_Coordinates                                       */
/*                                                                       */
/* <Description>                                                         */
/*    For Multiple Masters fonts, choose an interpolated font design     */
/*    through design coordinates.                                        */
/*                                                                       */
/* <InOut>                                                               */
/*    face       :: A handle to the source face.                         */
/*                                                                       */
/* <Input>                                                               */
/*    num_coords :: The number of design coordinates (must be equal to   */
/*                  the number of axes in the font).                     */
/*                                                                       */
/*    coords     :: An array of design coordinates.                      */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Set_MM_Design_Coordinates( FT2_1_3_Face   face,
                              FT2_1_3_UInt   num_coords,
                              FT2_1_3_Long*  coords );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_Set_MM_Blend_Coordinates                                        */
/*                                                                       */
/* <Description>                                                         */
/*    For Multiple Masters fonts, choose an interpolated font design     */
/*    through normalized blend coordinates.                              */
/*                                                                       */
/* <InOut>                                                               */
/*    face       :: A handle to the source face.                         */
/*                                                                       */
/* <Input>                                                               */
/*    num_coords :: The number of design coordinates (must be equal to   */
/*                  the number of axes in the font).                     */
/*                                                                       */
/*    coords     :: The design coordinates array (each element must be   */
/*                  between 0 and 1.0).                                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_Set_MM_Blend_Coordinates( FT2_1_3_Face    face,
                             FT2_1_3_UInt    num_coords,
                             FT2_1_3_Fixed*  coords );


/* */


FT2_1_3_END_HEADER

#endif /* __FTMM_H__ */


/* END */
