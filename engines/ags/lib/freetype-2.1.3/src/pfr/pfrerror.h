/***************************************************************************/
/*                                                                         */
/*  pfrerror.h                                                             */
/*                                                                         */
/*    PFR error codes (specification only).                                */
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


/*************************************************************************/
/*                                                                       */
/* This file is used to define the PFR error enumeration constants.      */
/*                                                                       */
/*************************************************************************/

#ifndef __PFRERROR_H__
#define __PFRERROR_H__

#include FT2_1_3_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT2_1_3_ERR_PREFIX  PFR_Err_
#define FT2_1_3_ERR_BASE    FT2_1_3_Mod_Err_PFR

#include FT2_1_3_ERRORS_H

#endif /* __PFRERROR_H__ */


/* END */
