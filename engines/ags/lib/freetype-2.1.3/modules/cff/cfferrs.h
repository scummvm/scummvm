/***************************************************************************/
/*                                                                         */
/*  cfferrs.h                                                              */
/*                                                                         */
/*    CFF error codes (specification only).                                */
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


/*************************************************************************/
/*                                                                       */
/* This file is used to define the CFF error enumeration constants.      */
/*                                                                       */
/*************************************************************************/

#ifndef __CFFERRS_H__
#define __CFFERRS_H__

#include "engines/ags/lib/freetype-2.1.3/ftmoderr.h"

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  CFF_Err_
#define FT_ERR_BASE    FT_Mod_Err_CFF


#include "engines/ags/lib/freetype-2.1.3/fterrors.h"

#endif /* __CFFERRS_H__ */


/* END */
