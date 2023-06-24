/***************************************************************************/
/*                                                                         */
/*  ttinterp.h                                                             */
/*                                                                         */
/*    TrueType bytecode interpreter (specification).                       */
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


#ifndef __TTINTERP_H__
#define __TTINTERP_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "ttobjs.h"


FT2_1_3_BEGIN_HEADER


#ifndef TT_CONFIG_OPTION_STATIC_INTEPRETER  /* indirect implementation */

#define EXEC_OP_   TT_ExecContext  exc,
#define EXEC_OP    TT_ExecContext  exc
#define EXEC_ARG_  exc,
#define EXEC_ARG   exc

#else                                       /* static implementation */

#define EXEC_OP_   /* void */
#define EXEC_OP    /* void */
#define EXEC_ARG_  /* void */
#define EXEC_ARG   /* void */

#endif /* TT_CONFIG_OPTION_STATIC_INTERPRETER */


/*************************************************************************/
/*                                                                       */
/* Rounding mode constants.                                              */
/*                                                                       */
#define TT_Round_Off             5
#define TT_Round_To_Half_Grid    0
#define TT_Round_To_Grid         1
#define TT_Round_To_Double_Grid  2
#define TT_Round_Up_To_Grid      4
#define TT_Round_Down_To_Grid    3
#define TT_Round_Super           6
#define TT_Round_Super_45        7


/*************************************************************************/
/*                                                                       */
/* Function types used by the interpreter, depending on various modes    */
/* (e.g. the rounding mode, whether to render a vertical or horizontal   */
/* line etc).                                                            */
/*                                                                       */
/*************************************************************************/

/* Rounding function */
typedef FT2_1_3_F26Dot6
(*TT_Round_Func)( EXEC_OP_ FT2_1_3_F26Dot6  distance,
                  FT2_1_3_F26Dot6  compensation );

/* Point displacement along the freedom vector routine */
typedef void
(*TT_Move_Func)( EXEC_OP_ TT_GlyphZone  zone,
                 FT2_1_3_UShort     point,
                 FT2_1_3_F26Dot6    distance );

/* Distance projection along one of the projection vectors */
typedef FT2_1_3_F26Dot6
(*TT_Project_Func)( EXEC_OP_ FT2_1_3_Vector*  v1,
                    FT2_1_3_Vector*  v2 );

/* reading a cvt value.  Take care of non-square pixels if necessary */
typedef FT2_1_3_F26Dot6
(*TT_Get_CVT_Func)( EXEC_OP_ FT2_1_3_ULong  idx );

/* setting or moving a cvt value.  Take care of non-square pixels  */
/* if necessary                                                    */
typedef void
(*TT_Set_CVT_Func)( EXEC_OP_ FT2_1_3_ULong    idx,
                    FT2_1_3_F26Dot6  value );


/*************************************************************************/
/*                                                                       */
/* This structure defines a call record, used to manage function calls.  */
/*                                                                       */
typedef struct  TT_CallRec_ {
	FT2_1_3_Int   Caller_Range;
	FT2_1_3_Long  Caller_IP;
	FT2_1_3_Long  Cur_Count;
	FT2_1_3_Long  Cur_Restart;

} TT_CallRec, *TT_CallStack;


/*************************************************************************/
/*                                                                       */
/* The main structure for the interpreter which collects all necessary   */
/* variables and states.                                                 */
/*                                                                       */
typedef struct  TT_ExecContextRec_ {
	TT_Face            face;
	TT_Size            size;
	FT2_1_3_Memory          memory;

	/* instructions state */

	FT2_1_3_Error           error;      /* last execution error */

	FT2_1_3_Long            top;        /* top of exec. stack   */

	FT2_1_3_UInt            stackSize;  /* size of exec. stack  */
	FT2_1_3_Long*           stack;      /* current exec. stack  */

	FT2_1_3_Long            args;
	FT2_1_3_UInt            new_top;    /* new top after exec.  */

	TT_GlyphZoneRec    zp0,        /* zone records */
	                   zp1,
	                   zp2,
	                   pts,
	                   twilight;

	FT2_1_3_Size_Metrics    metrics;
	TT_Size_Metrics    tt_metrics; /* size metrics */

	TT_GraphicsState   GS;         /* current graphics state */

	FT2_1_3_Int             curRange;  /* current code range number   */
	FT2_1_3_Byte*           code;      /* current code range          */
	FT2_1_3_Long            IP;        /* current instruction pointer */
	FT2_1_3_Long            codeSize;  /* size of current range       */

	FT2_1_3_Byte            opcode;    /* current opcode              */
	FT2_1_3_Int             length;    /* length of current opcode    */

	FT2_1_3_Bool            step_ins;  /* true if the interpreter must */
	/* increment IP after ins. exec */
	FT2_1_3_Long            cvtSize;
	FT2_1_3_Long*           cvt;

	FT2_1_3_UInt            glyphSize; /* glyph instructions buffer size */
	FT2_1_3_Byte*           glyphIns;  /* glyph instructions buffer */

	FT2_1_3_UInt            numFDefs;  /* number of function defs         */
	FT2_1_3_UInt            maxFDefs;  /* maximum number of function defs */
	TT_DefArray        FDefs;     /* table of FDefs entries          */

	FT2_1_3_UInt            numIDefs;  /* number of instruction defs */
	FT2_1_3_UInt            maxIDefs;  /* maximum number of ins defs */
	TT_DefArray        IDefs;     /* table of IDefs entries     */

	FT2_1_3_UInt            maxFunc;   /* maximum function index     */
	FT2_1_3_UInt            maxIns;    /* maximum instruction index  */

	FT2_1_3_Int             callTop,    /* top of call stack during execution */
	                   callSize;   /* size of call stack */
	TT_CallStack       callStack;  /* call stack */

	FT2_1_3_UShort          maxPoints;    /* capacity of this context's `pts' */
	FT2_1_3_Short           maxContours;  /* record, expressed in points and  */
	/* contours.                        */

	TT_CodeRangeTable  codeRangeTable;  /* table of valid code ranges */
	/* useful for the debugger   */

	FT2_1_3_UShort          storeSize;  /* size of current storage */
	FT2_1_3_Long*           storage;    /* storage area            */

	FT2_1_3_F26Dot6         period;     /* values used for the */
	FT2_1_3_F26Dot6         phase;      /* `SuperRounding'     */
	FT2_1_3_F26Dot6         threshold;

#if 0
	/* this seems to be unused */
	FT2_1_3_Int             cur_ppem;   /* ppem along the current proj vector */
#endif

	FT2_1_3_Bool            instruction_trap; /* If `True', the interpreter will */
	/* exit after each instruction     */

	TT_GraphicsState   default_GS;       /* graphics state resulting from   */
	/* the prep program                */
	FT2_1_3_Bool            is_composite;     /* true if the glyph is composite  */
	FT2_1_3_Bool            pedantic_hinting; /* true if pedantic interpretation */

	/* latest interpreter additions */

	FT2_1_3_Long            F_dot_P;    /* dot product of freedom and projection */
	/* vectors                               */
	TT_Round_Func      func_round; /* current rounding function             */

	TT_Project_Func    func_project,   /* current projection function */
	                   func_dualproj,  /* current dual proj. function */
	                   func_freeProj;  /* current freedom proj. func  */

	TT_Move_Func       func_move;      /* current point move function */

	TT_Get_CVT_Func    func_read_cvt;  /* read a cvt entry              */
	TT_Set_CVT_Func    func_write_cvt; /* write a cvt entry (in pixels) */
	TT_Set_CVT_Func    func_move_cvt;  /* incr a cvt entry (in pixels)  */

	FT2_1_3_ULong           loadSize;
	TT_SubGlyph_Stack  loadStack;      /* loading subglyph stack */

} TT_ExecContextRec;


extern const TT_GraphicsState  tt_default_graphics_state;


FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Goto_CodeRange( TT_ExecContext  exec,
                   FT2_1_3_Int          range,
                   FT2_1_3_Long         IP );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Set_CodeRange( TT_ExecContext  exec,
                  FT2_1_3_Int          range,
                  void*           base,
                  FT2_1_3_Long         length );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Clear_CodeRange( TT_ExecContext  exec,
                    FT2_1_3_Int          range );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    TT_New_Context                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    Queries the face context for a given font.  Note that there is     */
/*    now a _single_ execution context in the TrueType driver which is   */
/*    shared among faces.                                                */
/*                                                                       */
/* <Input>                                                               */
/*    face :: A handle to the source face object.                        */
/*                                                                       */
/* <Return>                                                              */
/*    A handle to the execution context.  Initialized for `face'.        */
/*                                                                       */
/* <Note>                                                                */
/*    Only the glyph loader and debugger should call this function.      */
/*                                                                       */
FT2_1_3_EXPORT( TT_ExecContext )
TT_New_Context( TT_Face  face );


FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Done_Context( TT_ExecContext  exec );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Destroy_Context( TT_ExecContext  exec,
                    FT2_1_3_Memory       memory );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Load_Context( TT_ExecContext  exec,
                 TT_Face         face,
                 TT_Size         size );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Save_Context( TT_ExecContext  exec,
                 TT_Size         ins );

FT2_1_3_LOCAL( FT2_1_3_Error )
TT_Run_Context( TT_ExecContext  exec,
                FT2_1_3_Bool         debug );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    TT_RunIns                                                          */
/*                                                                       */
/* <Description>                                                         */
/*    Executes one or more instruction in the execution context.  This   */
/*    is the main function of the TrueType opcode interpreter.           */
/*                                                                       */
/* <Input>                                                               */
/*    exec :: A handle to the target execution context.                  */
/*                                                                       */
/* <Return>                                                              */
/*    FreeType error code.  0 means success.                             */
/*                                                                       */
/* <Note>                                                                */
/*    Only the object manager and debugger should call this function.    */
/*                                                                       */
/*    This function is publicly exported because it is directly          */
/*    invoked by the TrueType debugger.                                  */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
TT_RunIns( TT_ExecContext  exec );


FT2_1_3_END_HEADER

#endif /* __TTINTERP_H__ */


/* END */
