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
 
    Configuration variable module public header file

 Notes: 
*/

#ifndef SAGA_CVAR_MOD_H_
#define SAGA_CVAR_MOD_H_

namespace Saga {

/* Modify these to change base cvar types */
#define CV_INTMAX INT_MAX
#define CV_INTMIN INT_MIN

#define CV_UINTMAX UINT_MAX
typedef int cv_int_t;
typedef unsigned int cv_uint_t;
typedef float cv_float_t;
typedef char cv_char_t;
typedef void (*cv_func_t) (int cv_argc, char *cv_argv[]);
/******************************************/

typedef struct R_CVAR_tag *R_CVAR_P;	/* opaque typedef */

typedef enum R_CVAR_TYPES_tag {

	R_CVAR_INVALID,
	R_CVAR_INT,
	R_CVAR_UINT,
	R_CVAR_FLOAT,
	R_CVAR_STRING,
	R_CVAR_FUNC
} R_CVAR_TYPES;

typedef enum R_CVAR_FLAGS_tag {

	R_CVAR_NONE,
	R_CVAR_READONLY,
	R_CVAR_LBOUND,
	R_CVAR_UBOUND,
	R_CVAR_CFG,
	R_CVAR_SECTION
} R_CVAR_FLAGS;

#define R_CVAR_BOUNDED ( R_CVAR_LBOUND | R_CVAR_UBOUND )

int CVAR_Shutdown(void);
R_CVAR_P CVAR_Find(const char *var_str);
int CVAR_SetValue(R_CVAR_P cvar, char *r_value);
int CVAR_Print(R_CVAR_P con_cvar);
int CVAR_GetError(char **err_str);
int CVAR_IsFunc(R_CVAR_P cvar_func);
int CVAR_Exec(R_CVAR_P cvar_func, char *r_value);

int
CVAR_RegisterFunc(cv_func_t func,
    const char *func_name,
    const char *func_argstr, uint flags, int min_args, int max_args);

int CVAR_Register_I(cv_int_t * var_p,
    const char *var_name,
    const char *section, uint flags, cv_int_t lbound, cv_int_t ubound);

int CVAR_Register_UI(cv_uint_t * var_p,
    const char *var_name,
    const char *section, uint flags, cv_uint_t lbound, cv_uint_t ubound);

int CVAR_Register_F(cv_float_t * var_p,
    const char *var_name,
    const char *section, uint flags, cv_float_t lbound, cv_float_t ubound);

int CVAR_Register_S(cv_char_t * var_str,
    const char *var_name, const char *section, uint flags, int ubound);

int EXPR_Parse(const char **exp_pp, int *len, R_CVAR_P * expr_cvar,
    char **rvalue);

char *EXPR_ReadString(const char **string_p, int *len, int term_char);

int EXPR_GetError(char **err_str);

int EXPR_GetArgs(char *cmd_str, char ***expr_argv);

} // End of namespace Saga

#endif				/* R_CVAR_MOD_H_ */
/* end r_cvar_mod.h_ */
