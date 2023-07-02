/***************************************************************************/
/*                                                                         */
/*  fttrigon.h                                                             */
/*                                                                         */
/*    FreeType trigonometric functions (specification).                    */
/*                                                                         */
/*  Copyright 2001 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTTRIGON_H
#define AGS_LIB_FREETYPE_FTTRIGON_H

#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* @section:                                                             */
/*   computations                                                        */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* @type:                                                                */
/*    FT2_1_3_Angle                                                           */
/*                                                                       */
/* @description:                                                         */
/*    This type is used to model angle values in FreeType.  Note that    */
/*    the angle is a 16.16 fixed float value expressed in degrees.       */
/*                                                                       */
typedef FT2_1_3_Fixed  FT2_1_3_Angle;


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_ANGLE_PI                                                        */
/*                                                                       */
/* @description:                                                         */
/*   The angle pi expressed in @FT2_1_3_Angle units.                          */
/*                                                                       */
#define FT2_1_3_ANGLE_PI  ( 180L << 16 )


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_ANGLE_2PI                                                       */
/*                                                                       */
/* @description:                                                         */
/*    The angle 2*pi expressed in @FT2_1_3_Angle units.                       */
/*                                                                       */
#define FT2_1_3_ANGLE_2PI  ( FT2_1_3_ANGLE_PI * 2 )


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_ANGLE_PI2                                                       */
/*                                                                       */
/* @description:                                                         */
/*    The angle pi/2 expressed in @FT2_1_3_Angle units.                       */
/*                                                                       */
#define FT2_1_3_ANGLE_PI2  ( FT2_1_3_ANGLE_PI / 2 )


/*************************************************************************/
/*                                                                       */
/* @macro:                                                               */
/*    FT2_1_3_ANGLE_PI4                                                       */
/*                                                                       */
/* @description:                                                         */
/*    The angle pi/4 expressed in @FT2_1_3_Angle units.                       */
/*                                                                       */
#define FT2_1_3_ANGLE_PI4  ( FT2_1_3_ANGLE_PI / 4 )


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Sin                                                             */
/*                                                                       */
/* @description:                                                         */
/*    Return the sinus of a given angle in fixed point format.           */
/*                                                                       */
/* @input:                                                               */
/*    angle :: The input angle.                                          */
/*                                                                       */
/* @return:                                                              */
/*    The sinus value.                                                   */
/*                                                                       */
/* @note:                                                                */
/*    If you need both the sinus and cosinus for a given angle, use the  */
/*    function @FT2_1_3_Vector_Unit.                                          */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Fixed )
FT2_1_3_Sin( FT2_1_3_Angle  angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Cos                                                             */
/*                                                                       */
/* @description:                                                         */
/*    Return the cosinus of a given angle in fixed point format.         */
/*                                                                       */
/* @input:                                                               */
/*    angle :: The input angle.                                          */
/*                                                                       */
/* @return:                                                              */
/*    The cosinus value.                                                 */
/*                                                                       */
/* @note:                                                                */
/*    If you need both the sinus and cosinus for a given angle, use the  */
/*    function @FT2_1_3_Vector_Unit.                                          */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Fixed )
FT2_1_3_Cos( FT2_1_3_Angle  angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Tan                                                             */
/*                                                                       */
/* @description:                                                         */
/*    Return the tangent of a given angle in fixed point format.         */
/*                                                                       */
/* @input:                                                               */
/*    angle :: The input angle.                                          */
/*                                                                       */
/* @return:                                                              */
/*    The tangent value.                                                 */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Fixed )
FT2_1_3_Tan( FT2_1_3_Angle  angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Atan2                                                           */
/*                                                                       */
/* @description:                                                         */
/*    Return the arc-tangent corresponding to a given vector (x,y) in    */
/*    the 2d plane.                                                      */
/*                                                                       */
/* @input:                                                               */
/*    x :: The horizontal vector coordinate.                             */
/*                                                                       */
/*    y :: The vertical vector coordinate.                               */
/*                                                                       */
/* @return:                                                              */
/*    The arc-tangent value (i.e. angle).                                */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Angle )
FT2_1_3_Atan2( FT2_1_3_Fixed  x,
		  FT2_1_3_Fixed  y );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Angle_Diff                                                      */
/*                                                                       */
/* @description:                                                         */
/*    Return the difference between two angles.  The result is always    */
/*    constrained to the ]-PI..PI] interval.                             */
/*                                                                       */
/* @input:                                                               */
/*    angle1 :: First angle.                                             */
/*                                                                       */
/*    angle2 :: Second angle.                                            */
/*                                                                       */
/* @return:                                                              */
/*    Contrainted value of `value2-value1'.                              */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Angle )
FT2_1_3_Angle_Diff( FT2_1_3_Angle  angle1,
			   FT2_1_3_Angle  angle2 );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Vector_Unit                                                     */
/*                                                                       */
/* @description:                                                         */
/*    Return the unit vector corresponding to a given angle.  After the  */
/*    call, the value of `vec.x' will be `sin(angle)', and the value of  */
/*    `vec.y' will be `cos(angle)'.                                      */
/*                                                                       */
/*    This function is useful to retrieve both the sinus and cosinus of  */
/*    a given angle quickly.                                             */
/*                                                                       */
/* @output:                                                              */
/*    vec   :: The address of target vector.                             */
/*                                                                       */
/* @input:                                                               */
/*    angle :: The address of angle.                                     */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_Vector_Unit( FT2_1_3_Vector*  vec,
				FT2_1_3_Angle    angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Vector_Rotate                                                   */
/*                                                                       */
/* @description:                                                         */
/*    Rotate a vector by a given angle.                                  */
/*                                                                       */
/* @inout:                                                               */
/*    vec   :: The address of target vector.                             */
/*                                                                       */
/* @input:                                                               */
/*    angle :: The address of angle.                                     */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_Vector_Rotate( FT2_1_3_Vector*  vec,
				  FT2_1_3_Angle    angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*   FT2_1_3_Vector_Length                                                    */
/*                                                                       */
/* @description:                                                         */
/*   Return the length of a given vector.                                */
/*                                                                       */
/* @input:                                                               */
/*   vec :: The address of target vector.                                */
/*                                                                       */
/* @return:                                                              */
/*   The vector length, expressed in the same units that the original    */
/*   vector coordinates.                                                 */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Fixed )
FT2_1_3_Vector_Length( FT2_1_3_Vector*  vec );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Vector_Normalize                                                */
/*                                                                       */
/* @description:                                                         */
/*    Normalize a given vector (i.e. compute the equivalent unit         */
/*    vector).                                                           */
/*                                                                       */
/* @inout:                                                               */
/*    vec :: The address of target vector.                               */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_Vector_Normalize( FT2_1_3_Vector*  vec );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Vector_Polarize                                                 */
/*                                                                       */
/* @description:                                                         */
/*    Compute both the length and angle of a given vector.               */
/*                                                                       */
/* @input:                                                               */
/*    vec    :: The address of source vector.                            */
/*                                                                       */
/* @output:                                                              */
/*    length :: The vector length.                                       */
/*    angle  :: The vector angle.                                        */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_Vector_Polarize( FT2_1_3_Vector*  vec,
					FT2_1_3_Fixed   *length,
					FT2_1_3_Angle   *angle );


/*************************************************************************/
/*                                                                       */
/* @function:                                                            */
/*    FT2_1_3_Vector_From_Polar                                               */
/*                                                                       */
/* @description:                                                         */
/*    Compute vector coordinates from a length and angle.                */
/*                                                                       */
/* @output:                                                              */
/*    vec    :: The address of source vector.                            */
/*                                                                       */
/* @input:                                                               */
/*    length :: The vector length.                                       */
/*    angle  :: The vector angle.                                        */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_Vector_From_Polar( FT2_1_3_Vector*  vec,
					  FT2_1_3_Fixed    length,
					  FT2_1_3_Angle    angle );

/* */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTTRIGON_H */


/* END */
