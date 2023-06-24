/***************************************************************************/
/*                                                                         */
/*  fterrdef.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
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


/*******************************************************************/
/*******************************************************************/
/*****                                                         *****/
/*****                LIST OF ERROR CODES/MESSAGES             *****/
/*****                                                         *****/
/*******************************************************************/
/*******************************************************************/


/* You need to define both FT2_1_3_ERRORDEF_ and FT2_1_3_NOERRORDEF_ before */
/* including this file.                                           */


/* generic errors */

FT2_1_3_NOERRORDEF_( Ok,                                        0x00, \
                "no error" )

FT2_1_3_ERRORDEF_( Cannot_Open_Resource,                        0x01, \
              "cannot open resource" )
FT2_1_3_ERRORDEF_( Unknown_File_Format,                         0x02, \
              "unknown file format" )
FT2_1_3_ERRORDEF_( Invalid_File_Format,                         0x03, \
              "broken file" )
FT2_1_3_ERRORDEF_( Invalid_Version,                             0x04, \
              "invalid FreeType version" )
FT2_1_3_ERRORDEF_( Lower_Module_Version,                        0x05, \
              "module version is too low" )
FT2_1_3_ERRORDEF_( Invalid_Argument,                            0x06, \
              "invalid argument" )
FT2_1_3_ERRORDEF_( Unimplemented_Feature,                       0x07, \
              "unimplemented feature" )
FT2_1_3_ERRORDEF_( Invalid_Table,                               0x08, \
              "broken table" )
FT2_1_3_ERRORDEF_( Invalid_Offset,                              0x09, \
              "broken offset within table" )

/* glyph/character errors */

FT2_1_3_ERRORDEF_( Invalid_Glyph_Index,                         0x10, \
              "invalid glyph index" )
FT2_1_3_ERRORDEF_( Invalid_Character_Code,                      0x11, \
              "invalid character code" )
FT2_1_3_ERRORDEF_( Invalid_Glyph_Format,                        0x12, \
              "unsupported glyph image format" )
FT2_1_3_ERRORDEF_( Cannot_Render_Glyph,                         0x13, \
              "cannot render this glyph format" )
FT2_1_3_ERRORDEF_( Invalid_Outline,                             0x14, \
              "invalid outline" )
FT2_1_3_ERRORDEF_( Invalid_Composite,                           0x15, \
              "invalid composite glyph" )
FT2_1_3_ERRORDEF_( Too_Many_Hints,                              0x16, \
              "too many hints" )
FT2_1_3_ERRORDEF_( Invalid_Pixel_Size,                          0x17, \
              "invalid pixel size" )

/* handle errors */

FT2_1_3_ERRORDEF_( Invalid_Handle,                              0x20, \
              "invalid object handle" )
FT2_1_3_ERRORDEF_( Invalid_Library_Handle,                      0x21, \
              "invalid library handle" )
FT2_1_3_ERRORDEF_( Invalid_Driver_Handle,                       0x22, \
              "invalid module handle" )
FT2_1_3_ERRORDEF_( Invalid_Face_Handle,                         0x23, \
              "invalid face handle" )
FT2_1_3_ERRORDEF_( Invalid_Size_Handle,                         0x24, \
              "invalid size handle" )
FT2_1_3_ERRORDEF_( Invalid_Slot_Handle,                         0x25, \
              "invalid glyph slot handle" )
FT2_1_3_ERRORDEF_( Invalid_CharMap_Handle,                      0x26, \
              "invalid charmap handle" )
FT2_1_3_ERRORDEF_( Invalid_Cache_Handle,                        0x27, \
              "invalid cache manager handle" )
FT2_1_3_ERRORDEF_( Invalid_Stream_Handle,                       0x28, \
              "invalid stream handle" )

/* driver errors */

FT2_1_3_ERRORDEF_( Too_Many_Drivers,                            0x30, \
              "too many modules" )
FT2_1_3_ERRORDEF_( Too_Many_Extensions,                         0x31, \
              "too many extensions" )

/* memory errors */

FT2_1_3_ERRORDEF_( Out_Of_Memory,                               0x40, \
              "out of memory" )
FT2_1_3_ERRORDEF_( Unlisted_Object,                             0x41, \
              "unlisted object" )

/* stream errors */

FT2_1_3_ERRORDEF_( Cannot_Open_Stream,                          0x51, \
              "cannot open stream" )
FT2_1_3_ERRORDEF_( Invalid_Stream_Seek,                         0x52, \
              "invalid stream seek" )
FT2_1_3_ERRORDEF_( Invalid_Stream_Skip,                         0x53, \
              "invalid stream skip" )
FT2_1_3_ERRORDEF_( Invalid_Stream_Read,                         0x54, \
              "invalid stream read" )
FT2_1_3_ERRORDEF_( Invalid_Stream_Operation,                    0x55, \
              "invalid stream operation" )
FT2_1_3_ERRORDEF_( Invalid_Frame_Operation,                     0x56, \
              "invalid frame operation" )
FT2_1_3_ERRORDEF_( Nested_Frame_Access,                         0x57, \
              "nested frame access" )
FT2_1_3_ERRORDEF_( Invalid_Frame_Read,                          0x58, \
              "invalid frame read" )

/* raster errors */

FT2_1_3_ERRORDEF_( Raster_Uninitialized,                        0x60, \
              "raster uninitialized" )
FT2_1_3_ERRORDEF_( Raster_Corrupted,                            0x61, \
              "raster corrupted" )
FT2_1_3_ERRORDEF_( Raster_Overflow,                             0x62, \
              "raster overflow" )
FT2_1_3_ERRORDEF_( Raster_Negative_Height,                      0x63, \
              "negative height while rastering" )

/* cache errors */

FT2_1_3_ERRORDEF_( Too_Many_Caches,                             0x70, \
              "too many registered caches" )

/* TrueType and SFNT errors */

FT2_1_3_ERRORDEF_( Invalid_Opcode,                              0x80, \
              "invalid opcode" )
FT2_1_3_ERRORDEF_( Too_Few_Arguments,                           0x81, \
              "too few arguments" )
FT2_1_3_ERRORDEF_( Stack_Overflow,                              0x82, \
              "stack overflow" )
FT2_1_3_ERRORDEF_( Code_Overflow,                               0x83, \
              "code overflow" )
FT2_1_3_ERRORDEF_( Bad_Argument,                                0x84, \
              "bad argument" )
FT2_1_3_ERRORDEF_( Divide_By_Zero,                              0x85, \
              "division by zero" )
FT2_1_3_ERRORDEF_( Invalid_Reference,                           0x86, \
              "invalid reference" )
FT2_1_3_ERRORDEF_( Debug_OpCode,                                0x87, \
              "found debug opcode" )
FT2_1_3_ERRORDEF_( ENDF_In_Exec_Stream,                         0x88, \
              "found ENDF opcode in execution stream" )
FT2_1_3_ERRORDEF_( Nested_DEFS,                                 0x89, \
              "nested DEFS" )
FT2_1_3_ERRORDEF_( Invalid_CodeRange,                           0x8A, \
              "invalid code range" )
FT2_1_3_ERRORDEF_( Execution_Too_Long,                          0x8B, \
              "execution context too long" )
FT2_1_3_ERRORDEF_( Too_Many_Function_Defs,                      0x8C, \
              "too many function definitions" )
FT2_1_3_ERRORDEF_( Too_Many_Instruction_Defs,                   0x8D, \
              "too many instruction definitions" )
FT2_1_3_ERRORDEF_( Table_Missing,                               0x8E, \
              "SFNT font table missing" )
FT2_1_3_ERRORDEF_( Horiz_Header_Missing,                        0x8F, \
              "horizontal header (hhea) table missing" )
FT2_1_3_ERRORDEF_( Locations_Missing,                           0x90, \
              "locations (loca) table missing" )
FT2_1_3_ERRORDEF_( Name_Table_Missing,                          0x91, \
              "name table missing" )
FT2_1_3_ERRORDEF_( CMap_Table_Missing,                          0x92, \
              "character map (cmap) table missing" )
FT2_1_3_ERRORDEF_( Hmtx_Table_Missing,                          0x93, \
              "horizontal metrics (hmtx) table missing" )
FT2_1_3_ERRORDEF_( Post_Table_Missing,                          0x94, \
              "PostScript (post) table missing" )
FT2_1_3_ERRORDEF_( Invalid_Horiz_Metrics,                       0x95, \
              "invalid horizontal metrics" )
FT2_1_3_ERRORDEF_( Invalid_CharMap_Format,                      0x96, \
              "invalid character map (cmap) format" )
FT2_1_3_ERRORDEF_( Invalid_PPem,                                0x97, \
              "invalid ppem value" )
FT2_1_3_ERRORDEF_( Invalid_Vert_Metrics,                        0x98, \
              "invalid vertical metrics" )
FT2_1_3_ERRORDEF_( Could_Not_Find_Context,                      0x99, \
              "could not find context" )
FT2_1_3_ERRORDEF_( Invalid_Post_Table_Format,                   0x9A, \
              "invalid PostScript (post) table format" )
FT2_1_3_ERRORDEF_( Invalid_Post_Table,                          0x9B, \
              "invalid PostScript (post) table" )

/* CFF, CID, and Type 1 errors */

FT2_1_3_ERRORDEF_( Syntax_Error,                                0xA0, \
              "opcode syntax error" )
FT2_1_3_ERRORDEF_( Stack_Underflow,                             0xA1, \
              "argument stack underflow" )

/* BDF errors */

FT2_1_3_ERRORDEF_( Missing_Startfont_Field,                     0xB0, \
              "`STARTFONT' field missing" )
FT2_1_3_ERRORDEF_( Missing_Font_Field,                          0xB1, \
              "`FONT' field missing" )
FT2_1_3_ERRORDEF_( Missing_Size_Field,                          0xB2, \
              "`SIZE' field missing" )
FT2_1_3_ERRORDEF_( Missing_Chars_Field,                         0xB3, \
              "`CHARS' field missing" )
FT2_1_3_ERRORDEF_( Missing_Startchar_Field,                     0xB4, \
              "`STARTCHAR' field missing" )
FT2_1_3_ERRORDEF_( Missing_Encoding_Field,                      0xB5, \
              "`ENCODING' field missing" )
FT2_1_3_ERRORDEF_( Missing_Bbx_Field,                           0xB6, \
              "`BBX' field missing" )


/* END */
