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

// Configuration Variable Module
#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/console.h"

#include "saga/cvar_mod.h"
#include "saga/cvar.h"

namespace Saga {

CVAR *CVHashTbl[CVAR_HASHLEN];

static const char *CVAR_ErrMsg[] = {
	"No Error",
	"Not implememented.",
	"Memory allocation failed",
	"Value overflowed while parsing",
	"Invalid numeric constant",
	"Value overflows destination type",
	"Assignment of negative value to unsigned variable",
	"Value outside of specified bounds",
	"Invalid string literal",
	"Invalid type for assignment",
	"Variable is read-only",
	"Not a valid function"
};

enum CVAR_Errors {
	CVERR_NONE,
	CVERR_NOTIMPL,
	CVERR_MEM,
	CVERR_PARSEOVERFLOW,
	CVERR_INVALID,
	CVERR_DESTOVERFLOW,
	CVERR_SIGN,
	CVERR_BOUND,
	CVERR_STRING,
	CVERR_TYPE,
	CVERR_READONLY,
	CVERR_NOTFUNC
};

static enum CVAR_Errors CVAR_ErrorState;

//Returns the appropriate cvar error string
int CVAR_GetError(const char **err_str) {
	*err_str = CVAR_ErrMsg[CVAR_ErrorState];
	return CVAR_ErrorState;
}
// Frees the cvar hash table
int CVAR_Shutdown() {
	CVAR *walk_ptr;
	CVAR *temp_ptr;
	int i;

	debug(0, "CVAR_Shutdown(): Deleting cvar hash table.");

	for (i = 0; i < CVAR_HASHLEN; i++) {
		for (walk_ptr = CVHashTbl[i]; walk_ptr; walk_ptr = temp_ptr) {
			temp_ptr = walk_ptr->next;
			free(walk_ptr);
		}
	}

	return SUCCESS;
}

// Returns hash index for string 'str'.
// Cannot fail.
unsigned int CVAR_HashString(const char *str) {
	unsigned int index;

	for (index = 0; *str != '\0'; str++) {
		index = *str + 31 * index;
	}

	return index % CVAR_HASHLEN;
}

// Adds a copy of the given cvar into the hash table.
// Returns SUCCESS if cvar was added, MEM if allocation failed.
int CVAR_Add(int index, CVAR *cvar) {
	CVAR *new_cvar;
	CVAR *temp_ptr;

	new_cvar = (CVAR *)malloc(sizeof(CVAR));

	if (new_cvar == NULL) {
		CVAR_ErrorState = CVERR_MEM;
		return MEM;
	}

	memcpy(new_cvar, cvar, sizeof(CVAR));

	if (CVHashTbl[index] == NULL) {
		CVHashTbl[index] = new_cvar;
		new_cvar->next = NULL;
	} else {
		temp_ptr = CVHashTbl[index];
		CVHashTbl[index] = new_cvar;
		new_cvar->next = temp_ptr;
	}

	CVAR_ErrorState = CVERR_NONE;
	return SUCCESS;
}

// Attempts to execute the specified console function with the given argument
// string.
// Returns FAILURE if cvar_func is not a valid console function
int CVAR_Exec(CVAR_P cvar_func, char *value) {
	int cf_argc = 0;
	char **cf_argv = NULL;
	int max_args;

	if (cvar_func->type != CVAR_FUNC) {
		CVAR_ErrorState = CVERR_NOTFUNC;
		return FAILURE;
	}

	cf_argc = EXPR_GetArgs(value, &cf_argv);

	if (cf_argc < cvar_func->t.func.min_args) {
		_vm->_console->print("Too few arguments to function.");
		if (cf_argv)
			free(cf_argv);
		return FAILURE;
	}

	max_args = cvar_func->t.func.max_args;
	if ((max_args > -1) && (cf_argc > max_args)) {
		_vm->_console->print("Too many arguments to function.");
		if (cf_argv)
			free(cf_argv);
		return FAILURE;
	}

	// Call function
	(cvar_func->t.func.func_p) (cf_argc, cf_argv, cvar_func->refCon);

	if (cf_argv)
		free(cf_argv);

	return SUCCESS;
}

// Attempts to assign the value contained in the string 'value' to cvar.
// Returns FAILURE if there was an error parsing 'value'
int CVAR_SetValue(CVAR_P cvar, char *value) {
	long int int_param;
	unsigned long uint16_param;

	char *end_p;
	ptrdiff_t scan_len;
	int value_len;

	value_len = strlen(value);

	if (cvar->flags & CVAR_READONLY) {
		CVAR_ErrorState = CVERR_READONLY;
		return FAILURE;
	}

	switch (cvar->type) {
	case CVAR_INT:
		int_param = strtol(value, &end_p, 10);
		if ((int_param == LONG_MIN) || (int_param == LONG_MAX)) {
			CVAR_ErrorState = CVERR_PARSEOVERFLOW;
			return FAILURE;
		}
		scan_len = end_p - value;

		if (int_param == 0) {
			if (!scan_len || value[scan_len - 1] != '0') {
				// strtol() returned 0, but string isn't "0". Invalid.
				CVAR_ErrorState = CVERR_INVALID;
				return FAILURE;
			}
		}

		if (scan_len != value_len) {
			// Entire string wasn't converted...Invalid
			CVAR_ErrorState = CVERR_INVALID;
			return FAILURE;
		}

		if ((int_param < CV_INTMIN) || (int_param > CV_INTMAX)) {
			// Overflows destination type
			CVAR_ErrorState = CVERR_DESTOVERFLOW;
			return FAILURE;
		}

		// Ignore bounds if equal
		if (cvar->t.i.lbound != cvar->t.i.ubound) {
			if ((int_param < cvar->t.i.lbound) || (int_param > cvar->t.i.ubound)) {
				// Value is outside of cvar bounds 
				CVAR_ErrorState = CVERR_BOUND;
				return FAILURE;
			}
		}

		*(cvar->t.i.var_p) = (cv_int_t) int_param;

#ifdef CVAR_TRACE
		debug(2, "Set cvar to value %ld.\n", int_param);
#endif

		break;
	case CVAR_UINT:
		if (*value == '-') {
			CVAR_ErrorState = CVERR_SIGN;
			return FAILURE;
		}

		uint16_param = strtoul(value, &end_p, 10);
		if (uint16_param == ULONG_MAX) {
			CVAR_ErrorState = CVERR_PARSEOVERFLOW;
			return FAILURE;
		}

		scan_len = end_p - value;
		if (uint16_param == 0) {
			if (!scan_len || value[scan_len - 1] != '0') {
				// strtol() returned 0, but string isn't "0". Invalid.
				CVAR_ErrorState = CVERR_INVALID;
				return FAILURE;
			}
		}

		if (scan_len != value_len) {
			// Entire string wasn't converted...Invalid 
			CVAR_ErrorState = CVERR_INVALID;
			return FAILURE;
		}

		if (uint16_param > CV_UINTMAX) {
			// Overflows destination type 
			CVAR_ErrorState = CVERR_DESTOVERFLOW;
			return FAILURE;
		}

		// Ignore bounds if equal
		if (cvar->t.ui.lbound != cvar->t.ui.ubound) {
			if ((uint16_param < cvar->t.ui.lbound) || (uint16_param > cvar->t.ui.ubound)) {
				// Value is outside cvar bounds 
				CVAR_ErrorState = CVERR_BOUND;
				return FAILURE;
			}
		}

		*(cvar->t.ui.var_p) = (cv_uint16_t) uint16_param;
#ifdef CVAR_TRACE
		debug(2, "Set cvar to value %lu.\n", uint16_param);
#endif
		break;
	case CVAR_FLOAT:
		CVAR_ErrorState = CVERR_NOTIMPL;
		return FAILURE;
		break;
	case CVAR_STRING:
		if (strrchr(value, '\"') != NULL) {
			CVAR_ErrorState = CVERR_STRING;
			return FAILURE;
		}
		strncpy(cvar->t.s.var_str, value, cvar->t.s.ubound);
		if (cvar->t.s.ubound < value_len) {
			cvar->t.s.var_str[cvar->t.s.ubound] = 0;
		}
#ifdef CVAR_TRACE
		debug(2, "Set cvar to value \"%s\".\n", cvar->t.s.var_str);
#endif
		break;
	default:
		CVAR_ErrorState = CVERR_TYPE;
		return FAILURE;
		break;
	}
	CVAR_ErrorState = CVERR_NONE;
	return SUCCESS;
}

// Given a cvar name this function returns a pointer to the appropriate 
// cvar structure or NULL if no match was found.
CVAR_P CVAR_Find(const char *var_str) {
	CVAR *walk_ptr;
	int hash;

	hash = CVAR_HashString(var_str);
#ifdef CVAR_TRACE
	debug(2, "Performing lookup on hash bucket %d.\n", hash);
#endif
	walk_ptr = CVHashTbl[hash];
	while (walk_ptr != NULL) {
		if (strcmp(var_str, walk_ptr->name) == 0) {
			return walk_ptr;
		}
		walk_ptr = walk_ptr->next;
	}

	return NULL;
}

int CVAR_IsFunc(CVAR_P cvar_func) {
	if (cvar_func->type == CVAR_FUNC)
		return 1;
	else
		return 0;
}

// Registers a console function 'cvar' 
// (could think of a better place to put these...?)
int CVAR_RegisterFunc(cv_func_t func, const char *func_name,
		  const char *func_argstr, uint16 flags, int min_args, int max_args, void *refCon) {
	CVAR new_cvar;
	int hash;

	new_cvar.name = func_name;
	new_cvar.type = CVAR_FUNC;
	new_cvar.section = NULL;
	new_cvar.refCon = refCon;
	new_cvar.flags = flags;
	new_cvar.t.func.func_p = func;
	new_cvar.t.func.func_argstr = func_argstr;
	new_cvar.t.func.min_args = min_args;
	new_cvar.t.func.max_args = max_args;
	hash = CVAR_HashString(func_name);

#ifdef CVAR_TRACE
	debug(2, "Added FUNC cvar to hash bucket %d.\n", hash);
#endif

	return CVAR_Add(hash, &new_cvar);
}

// Registers an integer type cvar.
int CVAR_Register_I(cv_int_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_int_t lbound, cv_int_t ubound) {

	CVAR new_cvar;
	int hash;

	new_cvar.name = var_name;
	new_cvar.type = CVAR_INT;
	new_cvar.section = section;
	new_cvar.flags = flags;
	new_cvar.t.i.var_p = var_p;
	new_cvar.t.i.lbound = lbound;
	new_cvar.t.i.ubound = ubound;
	hash = CVAR_HashString(var_name);

#ifdef CVAR_TRACE
	debug(2, "Added INT cvar to hash bucket %d.\n", hash);
#endif

	return CVAR_Add(hash, &new_cvar);
}

// Registers an unsigned integer type cvar.
int CVAR_Register_UI(cv_uint16_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_uint16_t lbound, cv_uint16_t ubound) {
	CVAR new_cvar;
	int hash;

	new_cvar.name = var_name;
	new_cvar.type = CVAR_UINT;
	new_cvar.section = section;
	new_cvar.flags = flags;
	new_cvar.t.ui.var_p = var_p;
	new_cvar.t.ui.lbound = lbound;
	new_cvar.t.ui.ubound = ubound;
	hash = CVAR_HashString(var_name);

#ifdef CVAR_TRACE
	debug(2, "Added UNSIGNED INT ccvar to hash bucket %d.\n", hash);
#endif

	return CVAR_Add(hash, &new_cvar);
}

// Registers a floating point type cvar.
int CVAR_Register_F(cv_float_t * var_p, const char *var_name,
					const char *section, uint16 flags, cv_float_t lbound, cv_float_t ubound) {
	CVAR new_cvar;
	int hash;

	new_cvar.name = var_name;
	new_cvar.type = CVAR_FLOAT;
	new_cvar.section = section;
	new_cvar.flags = flags;
	new_cvar.t.f.var_p = var_p;
	new_cvar.t.f.lbound = lbound;
	new_cvar.t.f.ubound = ubound;
	hash = CVAR_HashString(var_name);

#ifdef CVAR_TRACE
	debug(2, "Added FLOAT cvar to hash bucket %d.\n", hash);
#endif

	return CVAR_Add(hash, &new_cvar);
}

// Registers a string type cvar. Storage must be provided in var_p for 'ubound'
// characters plus 1 for NUL char. 
int CVAR_Register_S(cv_char_t * var_str, const char *var_name, const char *section, uint16 flags, int ubound) {
	CVAR new_cvar;
	int hash;

	new_cvar.name = var_name;
	new_cvar.type = CVAR_STRING;
	new_cvar.section = section;
	new_cvar.flags = flags;
	new_cvar.t.s.var_str = var_str;
	new_cvar.t.s.ubound = ubound;
	hash = CVAR_HashString(var_name);

#ifdef CVAR_TRACE
	debug(2, "Added UNSIGNED INT var to hash bucket %d.\n", hash);
#endif

	return CVAR_Add(hash, &new_cvar);
}

// Displays the value and type of the given cvar to the console.
int CVAR_Print(CVAR_P con_cvar) {
	switch (con_cvar->type) {

	case CVAR_INT:
		_vm->_console->print("\"%s\"(i) = %d", con_cvar->name, *(con_cvar->t.i.var_p));
		break;

	case CVAR_UINT:
		_vm->_console->print("\"%s\"(ui) = %u", con_cvar->name, *(con_cvar->t.ui.var_p));
		break;

	case CVAR_FLOAT:
		_vm->_console->print("\"%s\"(ui) = %f", con_cvar->name, *(con_cvar->t.f.var_p));
		break;

	case CVAR_STRING:
		_vm->_console->print("\"%s\"(s) = \"%s\"", con_cvar->name, con_cvar->t.s.var_str);
		break;

	case CVAR_FUNC:
		if (con_cvar->t.func.func_argstr) {
			_vm->_console->print("\"%s\"(func) Args: %s", con_cvar->name, con_cvar->t.func.func_argstr);
		} else {
			_vm->_console->print("\"%s\"(func) No arguments.", con_cvar->name);
		}
		break;

	default:
		_vm->_console->print("Invalid variable type.\n");
		break;
	}

	return SUCCESS;
}

} // End of namespace Saga
