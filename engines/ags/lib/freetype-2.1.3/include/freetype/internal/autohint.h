/***************************************************************************/
/*                                                                         */
/*  autohint.h                                                             */
/*                                                                         */
/*    High-level `autohint' module-specific interface (specification).     */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
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
/* The auto-hinter is used to load and automatically hint glyphs if a    */
/* format-specific hinter isn't available.                               */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_AUTOHINT_H
#define AGS_LIB_FREETYPE_AUTOHINT_H


/*************************************************************************/
/*                                                                       */
/* A small technical note regarding automatic hinting in order to        */
/* clarify this module interface.                                        */
/*                                                                       */
/* An automatic hinter might compute two kinds of data for a given face: */
/*                                                                       */
/* - global hints: Usually some metrics that describe global properties  */
/*                 of the face.  It is computed by scanning more or less */
/*                 agressively the glyphs in the face, and thus can be   */
/*                 very slow to compute (even if the size of global      */
/*                 hints is really small).                               */
/*                                                                       */
/* - glyph hints:  These describe some important features of the glyph   */
/*                 outline, as well as how to align them.  They are      */
/*                 generally much faster to compute than global hints.   */
/*                                                                       */
/* The current FreeType auto-hinter does a pretty good job while         */
/* performing fast computations for both global and glyph hints.         */
/* However, we might be interested in introducing more complex and       */
/* powerful algorithms in the future, like the one described in the John */
/* D. Hobby paper, which unfortunately requires a lot more horsepower.   */
/*                                                                       */
/* Because a sufficiently sophisticated font management system would     */
/* typically implement an LRU cache of opened face objects to reduce     */
/* memory usage, it is a good idea to be able to avoid recomputing       */
/* global hints every time the same face is re-opened.                   */
/*                                                                       */
/* We thus provide the ability to cache global hints outside of the face */
/* object, in order to speed up font re-opening time.  Of course, this   */
/* feature is purely optional, so most client programs won't even notice */
/* it.                                                                   */
/*                                                                       */
/* I initially thought that it would be a good idea to cache the glyph   */
/* hints too.  However, my general idea now is that if you really need   */
/* to cache these too, you are simply in need of a new font format,      */
/* where all this information could be stored within the font file and   */
/* decoded on the fly.                                                   */
/*                                                                       */
/*************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


typedef struct FT2_1_3_AutoHinterRec_  *FT2_1_3_AutoHinter;


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_AutoHinter_GlobalGetFunc                                        */
/*                                                                       */
/* <Description>                                                         */
/*    Retrieves the global hints computed for a given face object the    */
/*    resulting data is dissociated from the face and will survive a     */
/*    call to FT2_1_3_Done_Face().  It must be discarded through the API      */
/*    FT2_1_3_AutoHinter_GlobalDoneFunc().                                    */
/*                                                                       */
/* <Input>                                                               */
/*    hinter        :: A handle to the source auto-hinter.               */
/*                                                                       */
/*    face          :: A handle to the source face object.               */
/*                                                                       */
/* <Output>                                                              */
/*    global_hints  :: A typeless pointer to the global hints.           */
/*                                                                       */
/*    global_len    :: The size in bytes of the global hints.            */
/*                                                                       */
typedef void
(*FT2_1_3_AutoHinter_GlobalGetFunc)( FT2_1_3_AutoHinter  hinter,
								FT_Face        face,
								void**         global_hints,
								long*          global_len );


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_AutoHinter_GlobalDoneFunc                                       */
/*                                                                       */
/* <Description>                                                         */
/*    Discards the global hints retrieved through                        */
/*    FT2_1_3_AutoHinter_GlobalGetFunc().  This is the only way these hints   */
/*    are freed from memory.                                             */
/*                                                                       */
/* <Input>                                                               */
/*    hinter :: A handle to the auto-hinter module.                      */
/*                                                                       */
/*    global :: A pointer to retrieved global hints to discard.          */
/*                                                                       */
typedef void
(*FT2_1_3_AutoHinter_GlobalDoneFunc)( FT2_1_3_AutoHinter  hinter,
								 void*          global );


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_AutoHinter_GlobalResetFunc                                      */
/*                                                                       */
/* <Description>                                                         */
/*    This function is used to recompute the global metrics in a given   */
/*    font.  This is useful when global font data changes (e.g. Multiple */
/*    Masters fonts where blend coordinates change).                     */
/*                                                                       */
/* <Input>                                                               */
/*    hinter :: A handle to the source auto-hinter.                      */
/*                                                                       */
/*    face   :: A handle to the face.                                    */
/*                                                                       */
typedef void
(*FT2_1_3_AutoHinter_GlobalResetFunc)( FT2_1_3_AutoHinter  hinter,
								  FT_Face        face );


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_AutoHinter_GlyphLoadFunc                                        */
/*                                                                       */
/* <Description>                                                         */
/*    This function is used to load, scale, and automatically hint a     */
/*    glyph from a given face.                                           */
/*                                                                       */
/* <Input>                                                               */
/*    face        :: A handle to the face.                               */
/*                                                                       */
/*    glyph_index :: The glyph index.                                    */
/*                                                                       */
/*    load_flags  :: The load flags.                                     */
/*                                                                       */
/* <Note>                                                                */
/*    This function is capable of loading composite glyphs by hinting    */
/*    each sub-glyph independently (which improves quality).             */
/*                                                                       */
/*    It will call the font driver with FT2_1_3_Load_Glyph(), with            */
/*    FT2_1_3_LOAD_NO_SCALE set.                                              */
/*                                                                       */
typedef FT_Error
(*FT2_1_3_AutoHinter_GlyphLoadFunc)( FT2_1_3_AutoHinter  hinter,
								FT_GlyphSlot   slot,
								FT_Size        size,
								FT_UInt        glyph_index,
								FT_Int32       load_flags );


/*************************************************************************/
/*                                                                       */
/* <Struct>                                                              */
/*    FT2_1_3_AutoHinter_ServiceRec                                           */
/*                                                                       */
/* <Description>                                                         */
/*    The auto-hinter module's interface.                                */
/*                                                                       */
typedef struct  FT2_1_3_AutoHinter_ServiceRec_ {
	FT2_1_3_AutoHinter_GlobalResetFunc  reset_face;
	FT2_1_3_AutoHinter_GlobalGetFunc    get_global_hints;
	FT2_1_3_AutoHinter_GlobalDoneFunc   done_global_hints;
	FT2_1_3_AutoHinter_GlyphLoadFunc    load_glyph;

} FT2_1_3_AutoHinter_ServiceRec, *FT2_1_3_AutoHinter_Service;


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_AUTOHINT_H */


/* END */
