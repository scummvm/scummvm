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

// Configuration variable module public header file

#ifndef SAGA_CVAR_MOD_H_
#define SAGA_CVAR_MOD_H_

namespace Saga {

// Modify these to change base cvar types
#define CV_INTMAX INT_MAX
#define CV_INTMIN INT_MIN

#define CV_UINTMAX UINT_MAX
typedef int cv_int_t;
typedef unsigned int cv_uint16_t;
typedef float cv_float_t;
typedef char cv_char_t;
typedef void (*cv_func_t) (int cv_argc, char *cv_argv[], void *refCon);
/******************************************/

typedef struct CVAR *CVAR_P;	// opaque typedef

enum CVAR_TYPES {
	CVAR_INVALID,
	CVAR_INT,
	CVAR_UINT,
	CVAR_FLOAT,
	CVAR_STRING,
	CVAR_FUNC
};

enum CVAR_FLAGS {
	CVAR_NONE,
	CVAR_READONLY,
	CVAR_LBOUND,
	CVAR_UBOUND,
	CVAR_CFG,
	CVAR_SECTION
};

#define CVAR_BOUNDED ( CVAR_LBOUND | CVAR_UBOUND )

int CVAR_Shutdown();
CVAR_P CVAR_Find(const char *var_str);
int CVAR_SetValue(CVAR_P cvar, char *r_value);
int CVAR_Print(CVAR_P con_cvar);
int CVAR_GetError(const char **err_str);
int CVAR_IsFunc(CVAR_P cvar_func);
int CVAR_Exec(CVAR_P cvar_func, char *r_value);
int CVAR_RegisterFunc(cv_func_t func, const char *func_name,
		  const char *func_argstr, uint16 flags, int min_args, int max_args, void *refCon);
int CVAR_Register_I(cv_int_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_int_t lbound, cv_int_t ubound);
int CVAR_Register_UI(cv_uint16_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_uint16_t lbound, cv_uint16_t ubound);
int CVAR_Register_F(cv_float_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_float_t lbound, cv_float_t ubound);
int CVAR_Register_S(cv_char_t * var_str, const char *var_name, const char *section, uint16 flags, int ubound);
int EXPR_Parse(const char **exp_pp, int *len, CVAR_P * expr_cvar, char **rvalue);
char *EXPR_ReadString(const char **string_p, int *len, int term_char);
int EXPR_GetError(const char **err_str);
int EXPR_GetArgs(char *cmd_str, char ***expr_argv);

} // End of namespace Saga

#endif
