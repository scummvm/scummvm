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

#include FT2_1_3_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT2_1_3_ERR_PREFIX  CFF_Err_
#define FT2_1_3_ERR_BASE    FT2_1_3_Mod_Err_CFF


#include FT2_1_3_ERRORS_H

#endif /* __CFFERRS_H__ */


/* END */
