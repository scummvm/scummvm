/***************************************************************************/
/*                                                                         */
/*  ftcsbits.h                                                             */
/*                                                                         */
/*    A small-bitmap cache (specification).                                */
/*                                                                         */
/*  Copyright 2000-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTCSBITS_H
#define AGS_LIB_FREETYPE_FTCSBITS_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftcache.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/cache/ftcimage.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    cache_subsystem                                                    */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FTC_SBit                                                           */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a small bitmap descriptor.  See the @FTC_SBitRec       */
/*    structure for details.                                             */
/*                                                                       */
typedef struct FTC_SBitRec_*  FTC_SBit;


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FTC_SBitRec                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    A very compact structure used to describe a small glyph bitmap.    */
/*                                                                       */
/* <Fields>                                                              */
/*    width     :: The bitmap width in pixels.                           */
/*                                                                       */
/*    height    :: The bitmap height in pixels.                          */
/*                                                                       */
/*    left      :: The horizontal distance from the pen position to the  */
/*                 left bitmap border (a.k.a. `left side bearing', or    */
/*                 `lsb').                                               */
/*                                                                       */
/*    top       :: The vertical distance from the pen position (on the   */
/*                 baseline) to the upper bitmap border (a.k.a. `top     */
/*                 side bearing').  The distance is positive for upwards */
/*                 Y coordinates.                                        */
/*                                                                       */
/*    format    :: The format of the glyph bitmap (monochrome or gray).  */
/*                                                                       */
/*    max_grays :: Maximum gray level value (in the range 1 to 255).     */
/*                                                                       */
/*    pitch     :: The number of bytes per bitmap line.  May be positive */
/*                 or negative.                                          */
/*                                                                       */
/*    xadvance  :: The horizontal advance width in pixels.               */
/*                                                                       */
/*    yadvance  :: The vertical advance height in pixels.                */
/*                                                                       */
/*    buffer   :: A pointer to the bitmap pixels.                        */
/*                                                                       */
typedef struct  FTC_SBitRec_ {
	FT2_1_3_Byte   width;
	FT2_1_3_Byte   height;
	FT2_1_3_Char   left;
	FT2_1_3_Char   top;

	FT2_1_3_Byte   format;
	FT2_1_3_Byte   max_grays;
	FT2_1_3_Short  pitch;
	FT2_1_3_Char   xadvance;
	FT2_1_3_Char   yadvance;

	FT2_1_3_Byte*  buffer;

} FTC_SBitRec;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FTC_SBitCache                                                      */
/*                                                                       */
/* <Description>                                                         */
/*    A handle to a small bitmap cache.  These are special cache objects */
/*    used to store small glyph bitmaps (and anti-aliased pixmaps) in a  */
/*    much more efficient way than the traditional glyph image cache     */
/*    implemented by @FTC_ImageCache.                                    */
/*                                                                       */
typedef struct FTC_SBitCacheRec_*  FTC_SBitCache;


/*************************************************************************/
/*                                                                       */
/* <Type>                                                                */
/*    FTC_SBit_Cache                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    DEPRECATED.  Use @FTC_SBitCache instead.                           */
/*                                                                       */
typedef FTC_SBitCache  FTC_SBit_Cache;


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FTC_SBitCache_New                                                  */
/*                                                                       */
/* <Description>                                                         */
/*    Creates a new cache to store small glyph bitmaps.                  */
/*                                                                       */
/* <Input>                                                               */
/*    manager :: A handle to the source cache manager.                   */
/*                                                                       */
/* <Output>                                                              */
/*    acache  :: A handle to the new sbit cache.  NULL in case of error. */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FTC_SBitCache_New( FTC_Manager     manager,
				   FTC_SBitCache  *acache );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FTC_SBitCache_Lookup                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Looks up a given small glyph bitmap in a given sbit cache and      */
/*    "lock" it to prevent its flushing from the cache until needed      */
/*                                                                       */
/* <Input>                                                               */
/*    cache  :: A handle to the source sbit cache.                       */
/*                                                                       */
/*    type   :: A pointer to the glyph image type descriptor.            */
/*                                                                       */
/*    gindex :: The glyph index.                                         */
/*                                                                       */
/* <Output>                                                              */
/*    sbit   :: A handle to a small bitmap descriptor.                   */
/*                                                                       */
/*    anode  :: Used to return the address of of the corresponding cache */
/*              node after incrementing its reference count (see note    */
/*              below).                                                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    The small bitmap descriptor and its bit buffer are owned by the    */
/*    cache and should never be freed by the application.  They might    */
/*    as well disappear from memory on the next cache lookup, so don't   */
/*    treat them as persistent data.                                     */
/*                                                                       */
/*    The descriptor's `buffer' field is set to 0 to indicate a missing  */
/*    glyph bitmap.                                                      */
/*                                                                       */
/*    If "anode" is _not_ NULL, it receives the address of the cache     */
/*    node containing the bitmap, after increasing its reference count.  */
/*    This ensures that the node (as well as the image) will always be   */
/*    kept in the cache until you call @FTC_Node_Unref to "release" it.  */
/*                                                                       */
/*    If "anode" is NULL, the cache node is left unchanged, which means  */
/*    that the bitmap could be flushed out of the cache on the next      */
/*    call to one of the caching sub-system APIs.  Don't assume that it  */
/*    is persistent!                                                     */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FTC_SBitCache_Lookup( FTC_SBitCache    cache,
					  FTC_ImageType    type,
					  FT2_1_3_UInt          gindex,
					  FTC_SBit        *sbit,
					  FTC_Node        *anode );


/* */


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FTC_SBit_Cache_New                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    DEPRECATED.  Use @FTC_SBitCache_New instead.                       */
/*                                                                       */
/*    Creates a new cache to store small glyph bitmaps.                  */
/*                                                                       */
/* <Input>                                                               */
/*    manager :: A handle to the source cache manager.                   */
/*                                                                       */
/* <Output>                                                              */
/*    acache  :: A handle to the new sbit cache.  NULL in case of error. */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FTC_SBit_Cache_New( FTC_Manager      manager,
					FTC_SBit_Cache  *acache );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FTC_SBit_Cache_Lookup                                              */
/*                                                                       */
/* <Description>                                                         */
/*    DEPRECATED.  Use @FTC_SBitCache_Lookup instead.                    */
/*                                                                       */
/*    Looks up a given small glyph bitmap in a given sbit cache.         */
/*                                                                       */
/* <Input>                                                               */
/*    cache  :: A handle to the source sbit cache.                       */
/*                                                                       */
/*    desc   :: A pointer to the glyph image descriptor.                 */
/*                                                                       */
/*    gindex :: The glyph index.                                         */
/*                                                                       */
/* <Output>                                                              */
/*    sbit   :: A handle to a small bitmap descriptor.                   */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    The small bitmap descriptor and its bit buffer are owned by the    */
/*    cache and should never be freed by the application.  They might    */
/*    as well disappear from memory on the next cache lookup, so don't   */
/*    treat them as persistent data.                                     */
/*                                                                       */
/*    The descriptor's `buffer' field is set to 0 to indicate a missing  */
/*    glyph bitmap.                                                      */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
					   FTC_Image_Desc*  desc,
					   FT2_1_3_UInt          gindex,
					   FTC_SBit        *sbit );


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTCSBITS_H */


/* END */
