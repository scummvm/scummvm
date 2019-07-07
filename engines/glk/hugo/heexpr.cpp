/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/hugo/hugo.h"

namespace Glk {
namespace Hugo {

int Hugo::EvalExpr(int p) {
	int n1, n2;
	int oper;
	short result = 0;		/* must be 16 bits */

	/* for precedence stacking/unstacking */
	int next_prec, this_prec, temp_lp;

	if (!evalcount) return 0;       /* no expression */

	do
	{
		if (eval[p]==1)
		{
			if (eval[p+1]==OPEN_BRACKET_T ||
				eval[p+1]==OPEN_SQUARE_T)
			{
				eval[p] = 0;
				eval[p+1] = EvalExpr(p+2);
				TrimExpr(p+2);
			}
			else if (eval[p+1]==MINUS_T)
			{
				TrimExpr(p);
				eval[p+1] = -eval[p+1];
			}
		}

		if (evalcount<=p+2)
		{
			result = eval[p+1];
			TrimExpr(p);
			eval[p] = 0;
			eval[p+1] = result;
			goto ReturnResult;
		}

		n1 = eval[p+1];
		oper = eval[p+3];

		/* At this point, <n1> holds the first value, and <oper> holds
		   the token number of the operator.
		*/
		if (eval[p+4]==1 && (eval[p+5]==OPEN_BRACKET_T
			|| eval[p+5]==OPEN_SQUARE_T))
		{
			eval[p+4] = 0;
			eval[p+5] = EvalExpr(p+6);
			TrimExpr(p+6);
		}

		n2 = eval[p+5];

		if (evalcount > p+7)
		{
			if (eval[p+3]==CLOSE_BRACKET_T && eval[p+2]==1)
			{
				TrimExpr(p+2);
				return eval[p+1];
			}

			/* eval[p+7] holds the next operator, i.e., the "*"
			   in:  "x + y * z"

			   This way, we can check if the upcoming operator
			   takes precedence over the current one.
			*/
			if ((next_prec = Precedence(eval[p+7]))
				< (this_prec = Precedence(oper)))
			{
				if (next_prec >= last_precedence)
				{
#if defined (DEBUG_PRECEDENCE)
sprintf(line, "Not preferring %s to %s because of previous level %d", token[eval[p+7]], token[oper], last_precedence);
Printout(line);
#endif
					goto ReturnResult;
				}

#if defined (DEBUG_PRECEDENCE)
sprintf(line, "Preferring %s to %s", token[eval[p+7]], token[oper]);
Printout(line);
#endif

				temp_lp = last_precedence;
				last_precedence = this_prec;
				n2 = EvalExpr(p+4);
				last_precedence = temp_lp;
			}
		}
		else if (Precedence(oper)>=last_precedence)
		{
			goto ReturnResult;
		}

#if defined (DEBUG_PRECEDENCE)
sprintf(line, "Solving %d %s %d", n1, token[oper], n2);
Printout(line);
#endif

		switch (oper)
		{
			case DECIMAL_T:
			{
				result = GetProp(n1, n2, 1, 0);
				break;
			}

			case EQUALS_T:
			{
				result = (n1==n2);
				break;
			}
			case MINUS_T:
			{
				result = n1 - n2;
				break;
			}
			case PLUS_T:
			{
				result = n1 + n2;
				break;
			}
			case ASTERISK_T:
			{
				result = n1 * n2;
				break;
			}
			case FORWARD_SLASH_T:
			{
				if (n2==0)
#if defined (DEBUGGER)
				{
					RuntimeWarning("Division by zero:  invalid result");
					result = 0;
				}
#else
					FatalError(DIVIDE_E);
#endif
				result = n1 / n2;
				break;
			}
			case PIPE_T:
			{
				result = n1 | n2;
				break;
			}
			case GREATER_EQUAL_T:
			{
				result = (n1>=n2);
				break;
			}
			case LESS_EQUAL_T:
			{
				result = (n1<=n2);
				break;
			}
			case NOT_EQUAL_T:
			{
				result = (n1!=n2);
				break;
			}
			case AMPERSAND_T:
			{
				result = n1 & n2;
				break;
			}
			case GREATER_T:
			{
				result = (n1 > n2);
				break;
			}
			case LESS_T:
			{
				result = (n1 < n2);
				break;
			}
			case AND_T:
			{
				result = (n1 && n2);
				break;
			}
			case OR_T:
			{
				result = (n1 || n2);
				break;
			}

			default:
			{
				result = n1;
			}
		}

#if defined (DEBUGGER)
		if ((debug_eval) && debug_eval_error) return 0;
#endif

		TrimExpr(p+4);          /* second value */
		TrimExpr(p+2);          /* operator */

		eval[p] = 0;
		eval[p+1] = result;


	/* Keep looping while there are expression elements, or until there
	   is a ")", "]", or end of line
	*/
	} while ((evalcount>p+2) && !(eval[p+2]==1 &&
		(eval[p+3]==CLOSE_BRACKET_T || eval[p+3]==CLOSE_SQUARE_T ||
		eval[p+3]==255)));

	result = eval[p+1];

	TrimExpr(p);                    /* first value */

ReturnResult:

#if defined (DEBUG_EXPR_EVAL)
	if (p==0 && exprt)
	{
		sprintf(line, " = %d", result);
		AP(line);
	}
#endif
	return result;
}

int Hugo::GetVal() {
	char a = 0;
	char tempinexpr, tempgetaddress, tempinobj;
	int i, j;
	int tempret;

	unsigned short routineaddr, arrayaddr;	/* must be 16 bits */
	short val = 0;

	char inctype = 0;
	int preincdec;                  /* pre-increment/decrement */

	defseg = gameseg;

	tempret = ret;
	tempinexpr = inexpr;
	inexpr = 0;

	preincdec = incdec;
	incdec = 0;

	switch (MEM(codeptr))
	{
		case AMPERSAND_T:       /* an address */
			{codeptr++;
			getaddress = true;
			val = GetValue();
			getaddress = false;
			break;}

		case ROUTINE_T:
		case CALL_T:
		{
			if (MEM(codeptr)==ROUTINE_T)
			{
				if (tail_recursion==0 && MEM(codeptr-1)==RETURN_T)
				{
					/* We may be able to tail-recurse this return
					   statement if it's simply 'return Routine(...)'
					*/
					tail_recursion = TAIL_RECURSION_ROUTINE;
				}

				routineaddr = PeekWord(++codeptr);
				codeptr += 2;

				if (getaddress)
					{val = routineaddr;
					getaddress = false;
					break;}
			}
			else
			{
				codeptr++;
				routineaddr = GetValue();
			}

#if defined (DEBUGGER)
			if (debug_eval)
			{
				debug_eval_error = true;
				val = 0;
				break;
			}
#endif
                        val = CallRoutine(routineaddr);

			break;
		}

		case OPEN_BRACKET_T:
		{
			codeptr++;
			inexpr = 1;
			tempgetaddress = getaddress;
			getaddress = false;
			SetupExpr();
			inexpr = 0;
			val = EvalExpr(0);
			getaddress = tempgetaddress;
			break;
		}

		case MINUS_T:
		{
			codeptr++;
			j = inexpr;	/* don't reuse tempinexpr */
			inexpr = 1;
			val = -GetValue();
			inexpr = (char)j;
			break;
		}

		case VALUE_T:                   /* integer 0 - 65535 */
		case OBJECTNUM_T:
		case DICTENTRY_T:
		{
			val = PeekWord(++codeptr);
			codeptr += 2;
			break;
		}

		case ATTR_T:
		case PROP_T:
		{
			val = MEM(++codeptr);
			codeptr++;
			break;
		}

		case VAR_T:                     /* variable */
		{
			val = var[(i=MEM(++codeptr))];

			if (game_version >= 22)
			{
				/* Pre-v2.4 included linelength and pagelength as
				   global variables after objectcount
				*/
				if (i <= ((game_version>=24)?objectcount:objectcount+2))
				{
					if (i==wordcount) val = words;
					else if (i==objectcount) val = objects;

					/* i.e., pre-v2.4 only */
					else if (i==objectcount+1)
					{
#if defined (ACTUAL_LINELENGTH)
						val = ACTUAL_LINELENGTH();
#else
						val = SCREENWIDTH/charwidth;
#endif
					}
					else if (i==objectcount+2)
						val = SCREENHEIGHT/lineheight;
				}
			}
			codeptr++;

			if (!inobj) inctype = IsIncrement(codeptr);

			/* don't operate on, e.g., ++variable.property as
			   (++variable).property
			*/
			if ((incdec || preincdec) && MEM(codeptr)!=DECIMAL_T)
			{
				if (i < MAXGLOBALS) SaveUndo(VAR_T, i, val, 0, 0);

				if (inctype) val = Increment(val, inctype);

				/* still a post-increment hanging around */
				var[i] = (val+=preincdec) + incdec;

				incdec = preincdec = 0;
			}

			break;
		}

		case TRUE_T:
			val = 1;
			codeptr++;
			break;

		case FALSE_T:
			val = 0;
			codeptr++;
			break;

		case TILDE_T:
			codeptr++;
			val = ~GetValue();
			break;

		case NOT_T:
			codeptr++;
			val = !GetValue();
			break;

		case ARRAYDATA_T:
		case ARRAY_T:
		{
			unsigned int element;

			if (MEM(codeptr)==ARRAY_T)
			{
				codeptr++;
				arrayaddr = GetValue();
			}
			else
			{
				arrayaddr = PeekWord(++codeptr);
				codeptr += 2;
			}

			if (MEM(codeptr)!=OPEN_SQUARE_T)
				{val = arrayaddr;
				break;}

			if (game_version>=22)
			{
				/* convert to word value */
				arrayaddr*=2;

				if (game_version>=23)
					/* space for array length */
					a = 2;
			}

			/* check if this is array[] (i.e., array length) */
			if (MEM(++codeptr)==CLOSE_SQUARE_T)
			{
				defseg = arraytable;
				val = PeekWord(arrayaddr);
				codeptr++;
				break;
			}

			tempinobj = inobj;
			inobj = 0;
			j = GetValue();
			inobj = tempinobj;

			/* The array element we're after: */
			element = arrayaddr+a + j*2;
			
			defseg = arraytable;
#if defined (DEBUGGER)
			CheckinRange(element, debug_workspace, "array data");
#endif
			/* Check to make sure we've got a sane element number */
			if ((element>0) && (element < (unsigned int)(dicttable-arraytable)*16))
				val = PeekWord(element);
			else
				val = 0;
			codeptr++;

			if (!inobj) inctype = IsIncrement(codeptr);

			/* Don't operate on the array on:

				 ++a[n].property
			*/
			if ((incdec || preincdec) && MEM(codeptr)!=DECIMAL_T)
			{
				/* Same sanity check for element number */
				if ((element>0) && (element < (unsigned)(dicttable-arraytable)*16))
				{
					if (inctype) val = Increment(val, inctype);

					/* still a post-increment hanging around */
					SaveUndo(ARRAYDATA_T, arrayaddr+a, j, val, 0);

					PokeWord(element, (val+=preincdec) + incdec);

					incdec = preincdec = 0;
				}
			}

			break;
		}

		case RANDOM_T:
		{
			codeptr += 2;           /* skip the "(" */
			val = GetValue();
			if (val!=0) 
#if !defined (RANDOM)
				val = (hugo_rand() % val)+1;
#else
				val = (RANDOM() % val)+1;
#endif
			if (MEM(codeptr)==2) codeptr++;
			break;
		}

		case WORD_T:
		{
			codeptr += 2;           /* skip the "[" */

			if (MEM(codeptr)==CLOSE_SQUARE_T)	/* words[] */
			{
				val = words;
				break;
			}
			
			val = wd[GetValue()];
			if (MEM(codeptr)==CLOSE_SQUARE_T) codeptr++;
			break;
		}

		case CHILDREN_T:
		{
			codeptr += 2;        /* skip the "(" */
			val = GetValue();
			if (MEM(codeptr)==CLOSE_BRACKET_T) codeptr++;
			val = Children(val);
			break;
		}

		case PARENT_T:
		case SIBLING_T:
		case CHILD_T:
		case YOUNGEST_T:
		case ELDEST_T:
		case YOUNGER_T:
		case ELDER_T:
		{
			i = MEM(codeptr);
			codeptr += 2;         /* skip the "(" */
			val = GetValue();
			if (MEM(codeptr)==CLOSE_BRACKET_T) codeptr++;

			switch (i)
			{
				case PARENT_T:
					val = Parent(val);
					break;

				case SIBLING_T:
				case YOUNGER_T:
					val = Sibling(val);
					break;

				case CHILD_T:
				case ELDEST_T:
					val = Child(val);
					break;

				case YOUNGEST_T:
					val = Youngest(val);
					break;

				case ELDER_T:
					val = Elder(val);
					break;
			}
			break;
		}

		case SAVE_T:
			val = RunSave();
			codeptr++;
			break;

		case RESTORE_T:
			val = RunRestore();
			codeptr++;
			break;

		case SCRIPTON_T:
		case SCRIPTOFF_T:
			val = RunScriptSet();
			codeptr++;
			break;

		case RESTART_T:
			val = RunRestart();
			codeptr++;
			break;

		case STRING_T:
			val = RunString();
			break;

		case UNDO_T:
			val = Undo();
			codeptr++;
			break;

		case DICT_T:
			val = Dict();
			if (MEM(codeptr)==CLOSE_BRACKET_T) codeptr++;
			break;

		case RECORDON_T:
		case RECORDOFF_T:
		case PLAYBACK_T:
			val = RecordCommands();
			codeptr++;
			break;

		case READVAL_T:
		{
			val = 0;
			if (ioblock)
			{
#ifdef TODO
				int low, high;

				if ((ioblock==1)
					|| (low = hugo_fgetc(io))==EOF
					|| (high = hugo_fgetc(io))==EOF)
				{
					ioerror = true;
					retflag = true;
				}
				else val = low + high*256;
#else
				error("TODO: file io");
#endif
			}
			codeptr++;
			break;
		}

		case PARSE_T:
		{
			val = (short)PARSE_STRING_VAL;
			codeptr++;
			break;
		}

		case SERIAL_T:
		{
			val = (short)SERIAL_STRING_VAL;
			codeptr++;
			break;
		}
		
		case SYSTEM_T:
		{
			val = RunSystem();
			codeptr++;
			break;
		}
		
		default:
		{
#if defined (DEBUGGER)
			if (debug_eval)
				debug_eval_error = true;
			else
#endif

			FatalError(EXPECT_VAL_E);

#if defined (DEBUGGER)
			runtime_error = true;
			codeptr++;
			return 0;
#endif
		}
	}
	defseg = gameseg;
	ret = tempret;
	inexpr = tempinexpr;

	incdec = preincdec;

	return val;
}

int Hugo::GetValue() {
	char noself = 0;
	int p, n;
	char inctype; int preincdec;
	int nattr = 0, attr;
	unsigned int pa, val;
	long tempptr;
	short g;			/* must be 16 bits */
	int potential_tail_recursion = 0;

	/* Check to see if this may be a valid tail-recursion */
	if (tail_recursion==0 && MEM(codeptr-1)==RETURN_T)
	{
		/* We may be able to tail-recurse this return statement if
		   it's simply 'return object.property[.property...]'
		*/
		potential_tail_recursion = TAIL_RECURSION_PROPERTY;
	}

	IsIncrement(codeptr);           /* check for ++, -- */

	tempptr = codeptr;
	g = GetVal();

	preincdec = incdec;
	incdec = 0;

	if (inobj==0)
	{
	  switch (MEM(codeptr))
	  {
		case DECIMAL_T:                         /* object.property */
		{
DetermineProperty:
			if (MEM(++codeptr)==DECIMAL_T)  /* object..property */
			{
				noself = true;
				codeptr++;
			}

			if (MEM(codeptr)==POUND_T)      /* object.#property */
			{
				codeptr++;
				inobj = true;
				p = GetValue();
				inobj = false;
				pa = PropAddr(g, p, 0);
				if (pa)
				{
					defseg = proptable;
					g = Peek(pa + 1);
					if (g==PROP_ROUTINE) g = 1;
					defseg = gameseg;
				}
				else
					g = 0;
			}
			else
			{
				inobj = true;
				p = GetValue();
				inobj = false;

				if (MEM(codeptr) != POUND_T)
					n = 1;

				else		/* object.property #x */
				{
					codeptr++;

					/* Not GetValue(), since that might
					   botch "obj.property #n is attr"
					*/
					n = GetVal();
				}

				/* We checked this at the start of the function, but
				   GetValue() for the property would've cleared it
				*/
				tail_recursion = potential_tail_recursion;

				val = GetProp(g, p, n, noself);

				inctype = IsIncrement(codeptr);

				/* Increment/decrement an object.property, although
				   only if this is the last property in, e.g.,
				   object.property.property...
				*/
				if ((incdec || preincdec) && MEM(codeptr)!=DECIMAL_T)
				{
					SaveUndo(PROP_T, g, p, n, val);

					if (inctype) val = Increment(val, inctype);

					/* Still a post-increment hanging around */
					pa = PropAddr(g, p, 0);
					defseg = proptable;

					/* Only change it if not a routine */
					if (Peek(pa+1)!=PROP_ROUTINE)
						PokeWord(pa+n*2, (val+=preincdec)+incdec);

					defseg = gameseg;

					incdec = preincdec = 0;
				}
				g = val;
			}
			if (MEM(codeptr)==IS_T) goto CheckAttribute;

			break;
		}

		case IS_T:
		{
CheckAttribute:
			if (!inobj)
			{
				codeptr++;
				if (MEM(codeptr)==NOT_T)
				{
					nattr = 1;
					codeptr++;
				}
				attr = GetValue();
#if defined (DEBUGGER)
				CheckinRange((unsigned)attr, (unsigned)attributes, "attribute");
#endif
				g = TestAttribute(g, attr, nattr);

				break;
			}
		}
	  }

	  switch (MEM(codeptr))
	  {
		/* This comes here (again) in order to process
		   object.property1.property2...
		*/
		case DECIMAL_T:  goto DetermineProperty;

		case NOT_T:
			if (!inobj)
				{nattr = 1;
				codeptr++;}
			// fall through

		case IN_T:
		{
			if (!inobj)
			{
				codeptr++;
				p = GetValue();          /* testing parent */
				g = (p==Parent(g));
				if (nattr)
					g = !g;
			}
		}
	  }
	}                                       /* end of "if (inobj==0)" */

	n = MEM(codeptr);

	/* See if we have an implicit expression that needs to be
	   taken as a single value, i.e., "n + 1" where we've just
	   read n
	*/
	if (((n>=MINUS_T && n<=PIPE_T) || n==AMPERSAND_T) &&
		((!inexpr)) && !inobj)
/*
#if !defined (DEBUGGER)
		((!inexpr)) && !inobj)
#else
		((!inexpr)) && !inobj && !debug_eval)
#endif
*/
	{
		inexpr = 2;
		codeptr = tempptr;
		SetupExpr();
		g = EvalExpr(0);
		inexpr = 0;
	}

	/* Not a tail-recursive 'return object.property' */
	if (tail_recursion_addr==0)
		tail_recursion = 0;

	return g;
}

int Hugo::Increment(int a, char inctype) {
	short v;			/* must be 16 bits */

	v = a;

	switch (inctype)
	{
		case MINUS_T:           {v -= incdec; break;}
		case PLUS_T:            {v += incdec; break;}
		case ASTERISK_T:        {v *= incdec; break;}
		case AMPERSAND_T:       {v &= incdec; break;}
		case PIPE_T:            {v |= incdec; break;}
		case FORWARD_SLASH_T:
		{
#if defined (DEBUGGER)
			if (incdec==0)
			{
				RuntimeWarning("Division by zero:  invalid result");
				v = 0;
			}
			else
#endif
				v /= incdec;
			break;
		}
	}

	if (inctype!=1) incdec = 0;

	return v;
}

char Hugo::IsIncrement(long addr) {
	unsigned char a, t = 0;

	incdec = 0;

	switch (a = MEM(addr))
	{
		case MINUS_T:
		case PLUS_T:
		case ASTERISK_T:
		case FORWARD_SLASH_T:
		case AMPERSAND_T:
		case PIPE_T:
		{
			/* ++, -- */
			if ((a==MINUS_T || a==PLUS_T) && MEM(addr+1)==a)
			{
				codeptr = addr + 2;
				if (a==PLUS_T) incdec = 1;
				else incdec = -1;
				t = 1;
				break;
			}

			/* +=, -=, etc. */
			else if (MEM(addr+1)==EQUALS_T)
			{
				codeptr = addr + 2;
				incdec = GetValue();
				t = a;
			}
		}
	}

#if defined (DEBUGGER)
	if (t && debug_eval)
	{
		debug_eval_error = true;
		sprintf(debug_line, "'%s%s' illegal in watch/assignment", token[a], token[MEM(addr+1)]);
		DebugMessageBox("Expression Error", debug_line);
		t = 0;
	}
#endif
	return t;
}

int Hugo::Precedence(int t) {
	switch (t)
	{
		case DECIMAL_T:
			return 1;

		case ASTERISK_T:
		case FORWARD_SLASH_T:
			return 2;

		case MINUS_T:
		case PLUS_T:
			return 3;

		case PIPE_T:
		case TILDE_T:
		case AMPERSAND_T:
			return 4;

		case EQUALS_T:
		case GREATER_EQUAL_T:
		case LESS_EQUAL_T:
		case NOT_EQUAL_T:
		case GREATER_T:
		case LESS_T:
			return 5;

		default:
			return 6;
	}
}

#if defined (DEBUG_EXPR_EVAL)
/* PRINTEXPR

Prints the current expression during expression tracing.
*/
void PrintExpr(void)
{
	char e[261];
	int i, bracket = 0;

	if (!evalcount) return;

	strcpy(e, "( ");
	for (i=0; i<=evalcount; i+=2)
	{
		switch (eval[i])
		{
			case 0:
			{
				sprintf(line, "%d ", eval[i + 1]);
				strcat(e, line);
				break;
			}
			case 1:
			{
				if (eval[i+1]==OPEN_BRACKET_T) bracket++;
				if (eval[i+1]==CLOSE_BRACKET_T)
					{bracket--;
					if (bracket<0) goto ExitPrintExpr;}

				if (token[eval[i+1]][0]=='~')
					strcat(e, "\\");
				if (eval[i+1] != 255)
					{sprintf(line, "%s ", token[eval[i+1]]);
					strcat(e, line);}
				break;
			}
		}
	}

ExitPrintExpr:
	strcat(e, ")\\;");

	AP(e);
}
#endif

void Hugo::SetupExpr() {
	char justgotvalue = 1;
	int j, t, bracket = 0;
	int tempret;
	int tempeval[MAX_EVAL_ELEMENTS];
	int tempevalcount;

	last_precedence = 10;

	tempret = ret;
	tempevalcount = 0;

	inobj = false;
	if (!inexpr) inexpr = 1;

	do
	{
		justgotvalue++;

		switch (t = MEM(codeptr))
		{
			/* Various indications that we've hit the
			   end of the expression:
			*/
			case EOL_T:
				arrexpr = false;
				// fall through
			case COMMA_T:
				multiprop = false;
				// fall through
			case SEMICOLON_T:
			case CLOSE_SQUARE_T:
			case JUMP_T:
			{
				if (t==EOL_T || t==COMMA_T || t==JUMP_T)
					codeptr++;
LeaveSetupExpr:
				for (j=0; j<tempevalcount; j++)
					eval[j] = tempeval[j];
				evalcount = tempevalcount;

				eval[evalcount] = 1;
				eval[evalcount + 1] = 255;

#if defined (DEBUG_EXPR_EVAL)
				if (exprt) PrintExpr();
#endif

				ret = tempret;
				return;
			}


			/* Otherwise we have a value: */

			case OPEN_BRACKET_T:

			case MINUS_T:
			case PLUS_T:
			case TILDE_T:
			case AMPERSAND_T:
			case NOT_T:

			case PARENT_T:
			case SIBLING_T:
			case CHILD_T:
			case YOUNGEST_T:
			case ELDEST_T:
			case YOUNGER_T:
			case ELDER_T:
			case CHILDREN_T:
			case RANDOM_T:
			case SYSTEM_T:

			case PROP_T:
			case ATTR_T:
			case VAR_T:
			case DICTENTRY_T:
			case ROUTINE_T:
			case OBJECTNUM_T:
			case VALUE_T:

			case ARRAYDATA_T:
			case ARRAY_T:
			case WORD_T:

			case CALL_T:

			case SAVE_T:
			case RESTORE_T:
			case SCRIPTON_T:
			case SCRIPTOFF_T:
			case RESTART_T:
			case UNDO_T:
			case READVAL_T:

			case STRING_T:
			case DICT_T:

			case PARSE_T:
			case SERIAL_T:
			{
				if ((t==AMPERSAND_T || t==MINUS_T ||
					t==PLUS_T) && justgotvalue==1)
					goto SomeSymbolorToken;

				tempeval[tempevalcount] = 0;
				tempeval[tempevalcount + 1] = GetValue();

#if defined (DEBUGGER)
				if ((debug_eval) && debug_eval_error)
					return;
#endif

				tempevalcount += 2;
				if (tempevalcount > MAX_EVAL_ELEMENTS-2)
					FatalError(OVERFLOW_E);

				justgotvalue = 0;

				break;
			}

			/* Logical constants */
			case TRUE_T:
			case FALSE_T:
			{
				tempeval[tempevalcount] = 0;
				if (Peek(codeptr)==TRUE_T)
					tempeval[tempevalcount + 1] = 1;
				else
					tempeval[tempevalcount + 1] = 0;

				codeptr++;

				tempevalcount += 2;
				if (tempevalcount > MAX_EVAL_ELEMENTS-2)
					FatalError(OVERFLOW_E);

				break;
			}

			/* Some symbol or token */
			default:
			{
SomeSymbolorToken:
				tempeval[tempevalcount] = 1;
				tempeval[tempevalcount + 1] = MEM(codeptr++);

				tempevalcount += 2;
				if (tempevalcount > MAX_EVAL_ELEMENTS-2)
					FatalError(OVERFLOW_E);

				switch (MEM(codeptr-1))
				{
					case OPEN_BRACKET_T:
						{bracket++;
						break;}
					case CLOSE_BRACKET_T:
						{bracket--;
						justgotvalue = 0;
						if (inexpr==2)
							codeptr--;}
				}
				if (bracket < 0) goto LeaveSetupExpr;

				break;
			}
		}
	}
	while (true);                      /* endless loop */
}

void Hugo::TrimExpr(int ptr) {
	int i;

	for (i=ptr; i<=evalcount; i+=2)
	{
		eval[i] = eval[i+2];
		eval[i+1] = eval[i+3];
	}
	evalcount -= 2;
}

} // End of namespace Hugo
} // End of namespace Glk
