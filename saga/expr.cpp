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
 
    Expression parsing module, and string handling functions

 Notes: 

    EXPR_ParseArgs() lifted wholesale from SDL win32 initialization code by
    Sam Lantinga
*/

#include "reinherit.h"

/*
   Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "cvar_mod.h"

/*
   Begin module
\*--------------------------------------------------------------------------*/
#include "expr.h"

namespace Saga {

static const char *EXPR_ErrMsg[] = {

	"Invalid error state.",
	"No Error",
	"Memory allocation failed",
	"Illegal variable name",
	"Expected \'=\' or \'(\' in expression",
	"Expected \'(\' in function call",
	"Illegal \'(\', identifier is not function",
	"Expected a value to assign",
	"Unterminated string literal",
	"Unmatched parenthesis in function call",
	"Error reading value string",
	"Expected a number or boolean",
	"Unknown variable or function"
};

enum EXPR_Errors {

	EXERR_ASSERT,
	EXERR_NONE,
	EXERR_MEM,
	EXERR_ILLEGAL,
	EXERR_EXPR,
	EXERR_FUNC,
	EXERR_NOTFUNC,
	EXERR_RVALUE,
	EXERR_LITERAL,
	EXERR_PAREN,
	EXERR_STRING,
	EXERR_NUMBER,
	EXERR_NOTFOUND
};

static enum EXPR_Errors EXPR_ErrorState;

int EXPR_GetError(const char **err_str)
/*--------------------------------------------------------------------------*\
 Returns the appropriate expression parser error string given an error code.
\*--------------------------------------------------------------------------*/
{

	*err_str = EXPR_ErrMsg[EXPR_ErrorState];

	return EXPR_ErrorState;
}

int
EXPR_Parse(const char **exp_pp, int *len, R_CVAR_P * expr_cvar, char **rvalue)
/*--------------------------------------------------------------------------*\
 Parses an interactive expression.
 Sets 'expr_cvar' to the cvar/cfunction identifier input by the user, and
 'rvalue' to the corresponding rvalue ( in an expression ) or argument string
 ( in a function call ).

 Memory pointed to by rvalue after return must be explicitly freed by the
 caller.
\*--------------------------------------------------------------------------*/
{

	int i;
	int in_char;
	int equ_offset;
	int rvalue_offset;

	char *lvalue_str;
	int lvalue_len;

	char *rvalue_str;
	int rvalue_len;

	const char *scan_p;
	int scan_len;

	const char *expr_p;
	int expr_len;
	int test_char = '\0';
	int have_func = 0;

	R_CVAR_P lvalue_cvar;

	expr_p = *exp_pp;
	expr_len = strlen(*exp_pp);

	scan_p = *exp_pp;
	scan_len = expr_len;

    /**lvalue = NULL;*/
	*rvalue = NULL;

	EXPR_ErrorState = EXERR_ASSERT;

	for (i = 0; i <= scan_len; i++, scan_p++) {

		in_char = *scan_p;

		if ((i == 0) && isdigit(in_char)) {
			/* First character of a valid identifier cannot be a digit */
			EXPR_ErrorState = EXERR_ILLEGAL;
			return R_FAILURE;
		}

		/* If we reach a character that isn't valid in an identifier... */
		if ((!isalnum(in_char)) && ((in_char != '_'))) {

			/* then eat remaining whitespace, if any */
			equ_offset = strspn(scan_p, R_EXPR_WHITESPACE);

			test_char = scan_p[equ_offset];
			/* and test for the only valid characters after an identifier */
			if ((test_char != '=') &&
			    (test_char != '\0') && (test_char != '(')) {

				if ((equ_offset == 0)
				    && ((scan_p - expr_p) != expr_len)) {
					EXPR_ErrorState = EXERR_ILLEGAL;
				} else {
					EXPR_ErrorState = EXERR_EXPR;
				}
				return R_FAILURE;
			}

			break;
		}
	}

	lvalue_len = (scan_p - expr_p);
	lvalue_str = (char *)malloc(lvalue_len + 1);

	if (lvalue_str == NULL) {
		EXPR_ErrorState = EXERR_MEM;
		return R_FAILURE;
	}

	strncpy(lvalue_str, expr_p, lvalue_len);
	lvalue_str[lvalue_len] = 0;

	/* We now have the lvalue, so attempt to find it */
	lvalue_cvar = CVAR_Find(lvalue_str);
	if (lvalue_cvar == NULL) {
		EXPR_ErrorState = EXERR_NOTFOUND;
		return R_FAILURE;
	}
	if (lvalue_str) {
		free(lvalue_str);
		lvalue_str = NULL;
	}

	/* Skip parsed character, if any */
	scan_p += equ_offset + 1;
	scan_len = (scan_p - expr_p);

	/* Check if the 'cvar' is really a function */
	have_func = CVAR_IsFunc(lvalue_cvar);

	if (test_char == '(') {

		if (have_func) {

			rvalue_str =
			    EXPR_ReadString(&scan_p, &rvalue_len, ')');
			if (rvalue_str != NULL) {
				/* Successfully read string */
				/*CON_Print( "Read function parameters \"%s\".", rvalue_str ); */
				*expr_cvar = lvalue_cvar;
				*rvalue = rvalue_str;

				scan_len = (scan_p - expr_p);

				*exp_pp = scan_p;
				*len -= scan_len;

				EXPR_ErrorState = EXERR_NONE;
				return R_SUCCESS;
			} else {
				EXPR_ErrorState = EXERR_PAREN;
				return R_FAILURE;
			}
		} else {
			EXPR_ErrorState = EXERR_NOTFUNC;
			return R_FAILURE;
		}

	}

	/* Eat more whitespace */
	rvalue_offset = strspn(scan_p, R_EXPR_WHITESPACE);

	if (rvalue_offset + i == expr_len) {
		/* Only found single lvalue */
		*expr_cvar = lvalue_cvar;
		*exp_pp = scan_p;
		*len -= scan_len;
		return R_SUCCESS;
	}

	scan_p += rvalue_offset;
	scan_len = (scan_p - expr_p) + 1;

	in_char = *scan_p;

	in_char = toupper(in_char);

	switch (in_char) {

	case '\"':
		scan_p++;
		scan_len--;
		rvalue_str = EXPR_ReadString(&scan_p, &rvalue_len, '\"');

		if (rvalue_str != NULL) {
			/* Successfully read string */
			break;
		} else {
			EXPR_ErrorState = EXERR_LITERAL;
			return R_FAILURE;
		}
		break;

#if 0
	case 'Y':		/* Y[es] */
	case 'T':		/* T[rue] */

		break;

	case 'N':		/* N[o] */
	case 'F':		/* F[alse] */

		break;
#endif

	default:

		if (isdigit(in_char) || (in_char == '-') || (in_char == '+')) {

			rvalue_str = EXPR_ReadString(&scan_p, &rvalue_len, 0);

			if (rvalue_str != NULL) {
				/* Successfully read string */
				break;
			} else {
				EXPR_ErrorState = EXERR_STRING;
				return R_FAILURE;
			}
		} else {
			EXPR_ErrorState = EXERR_NUMBER;
			return R_FAILURE;
		}

		break;

	}

	*expr_cvar = lvalue_cvar;
	*rvalue = rvalue_str;

	scan_len = (scan_p - expr_p);

	*exp_pp = scan_p;
	*len -= scan_len;

	EXPR_ErrorState = EXERR_NONE;
	return R_SUCCESS;

}

char *EXPR_ReadString(const char **string_p, int *len, int term_char)
/****************************************************************************\
 Reads in a string of characters from '*string_p' until 'term_char' is 
 encountered. If 'term_char' == 0, the function reads characters until
 whitespace is encountered. 
 Upon reading a string, the function modifies *string_p and len based on 
 the number of characters read.
\****************************************************************************/
{

	int string_len;
	char *str_p;
	char *term_p;

	const char *scan_p;
	int in_char;

	if (term_char > 0) {

		term_p = strchr(*string_p, term_char);

		if (term_p == NULL) {
			return NULL;
		}

		string_len = (int)(term_p - *string_p);

		str_p = (char *)malloc(string_len + 1);

		if (str_p == NULL) {
			return NULL;
		}

		strncpy(str_p, *string_p, string_len);
		str_p[string_len] = 0;

		*string_p += (string_len + 1);	/* Add 1 for terminating char */
		*len -= (string_len + 1);

	} else {

		scan_p = *string_p;
		string_len = 0;

		while (scan_p) {

			in_char = *scan_p++;

			if (!isspace(in_char)) {
				string_len++;
			} else if (string_len) {

			  str_p = (char *)malloc(string_len + 1);

				if (str_p == NULL) {
					return NULL;
				}

				strncpy(str_p, *string_p, string_len);
				str_p[string_len] = 0;

				*string_p += string_len;
				*len -= string_len;
				break;

			} else {
				return NULL;
			}
		}

	}

	return str_p;
}

int EXPR_GetArgs(char *cmd_str, char ***expr_argv)
/****************************************************************************\
 Parses the string 'cmd_str' into argc/argv format, returning argc.
 The resulting argv pointers point into the 'cmd_str' string, so any argv
 entries should not be used after cmd_str is deallocated.

 Memory pointed to by expr_argv must be explicitly freed by the caller.
\****************************************************************************/
{

	int expr_argc;

	expr_argc = EXPR_ParseArgs(cmd_str, NULL);
	*expr_argv = (char **)malloc((expr_argc + 1) * sizeof(**expr_argv));

	if (expr_argv == NULL) {
		return R_FAILURE;
	}

	EXPR_ParseArgs(cmd_str, *expr_argv);

	return expr_argc;

}

int EXPR_ParseArgs(char *cmd_str, char **argv)
{

	char *bufp;
	int argc;

	argc = 0;
	for (bufp = cmd_str; *bufp;) {
		/* Skip leading whitespace */
		while (isspace(*bufp)) {
			++bufp;
		}
		/* Skip over argument */
		if (*bufp == '"') {
			++bufp;
			if (*bufp) {
				if (argv) {
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while (*bufp && (*bufp != '"')) {
				++bufp;
			}
		} else {
			if (*bufp) {
				if (argv) {
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while (*bufp && !isspace(*bufp)) {
				++bufp;
			}
		}
		if (*bufp) {
			if (argv) {
				*bufp = '\0';
			}
			++bufp;
		}
	}
	if (argv) {
		argv[argc] = NULL;
	}
	return (argc);
}

} // End of namespace Saga
