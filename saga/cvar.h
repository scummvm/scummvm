/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Configuration Variable Module

 Notes: 
*/

#ifndef SAGA_CVAR_H_
#define SAGA_CVAR_H_

namespace Saga {

#define R_CVAR_HASHLEN 32

typedef struct R_SUBCVAR_INT_tag {

	cv_int_t *var_p;
	cv_int_t ubound;
	cv_int_t lbound;

} R_SUBCVAR_INT;

typedef struct R_SUBCVAR_UINT_tag {

	cv_uint_t *var_p;
	cv_uint_t ubound;
	cv_uint_t lbound;

} R_SUBCVAR_UINT;

typedef struct R_SUBCVAR_FLOAT_tag {

	cv_float_t *var_p;
	cv_float_t ubound;
	cv_float_t lbound;

} R_SUBCVAR_FLOAT;

typedef struct R_SUBCVAR_STRING_tag {

	cv_char_t *var_str;
	int ubound;

} R_SUBCVAR_STRING;

typedef struct R_SUBCVAR_FUNC_tag {

	cv_func_t func_p;
	const char *func_argstr;
	int min_args;
	int max_args;

} R_SUBCVAR_FUNC;

typedef struct R_CVAR_tag {

	int type;
	const char *name;
	const char *section;
	uint flags;

	union {
		R_SUBCVAR_INT i;
		R_SUBCVAR_UINT ui;
		R_SUBCVAR_FLOAT f;
		R_SUBCVAR_STRING s;
		R_SUBCVAR_FUNC func;
	} t;

	struct R_CVAR_tag *next;

} R_CVAR;

} // End of namespace Saga

#endif				/* R_CVAR_H_ */
/* end "r_cvar.h" */
