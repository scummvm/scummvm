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

void Hugo::RunSet(int gotvalue) {
	char inc = 0;                   /* increment/decrement */
	char temparrexpr, propval = 0;
	int a = 0, t = 0, obj = 0;
	int newl = 0;                   /* new length */
	int newp = 0;			/* new property val */
	unsigned int element = 0;		/* of an array */

	unsigned short n, m, v;		/* must be 16 bits */

	inobj = 0;

	if (gotvalue!=-1)
	{
		obj = gotvalue;
		t = SetCompound(t);
		goto StoreVal;
	}

	t = MEM(codeptr);

	switch (t)
	{
		case OBJECTNUM_T:
		{
			codeptr++;
			obj = PeekWord(codeptr);
			codeptr += 2;
			t = SetCompound(t);
			break;
		}

		case VAR_T:
		{
			a = MEM(codeptr + 1);

			/* Check for ++, --, +=, etc. */
			inc = IsIncrement(codeptr+2);

			if (MEM(codeptr + 2)==EQUALS_T || inc)
			{
				if (a < MAXGLOBALS) SaveUndo(VAR_T, a, var[a], 0, 0);

				/* anonymous function */
				if (!inc && MEM(codeptr+3)==EOL_T)
				{
					var[a] = GetAnonymousFunction(codeptr+4);
					return;
				}

				if (inc)
				{
					var[a] = (Increment(var[a], inc)) + incdec;

					/* backward-compatibility tweak */
					if ((game_version<23) && MEM(codeptr)!=CLOSE_BRACE_T) codeptr--;

					codeptr++;      /* eol */
				}
				else
				{
					codeptr += 3;
					inexpr = 1;
					SetupExpr();
					inexpr = 0;
					v = EvalExpr(0);
					var[a] = v;
				}

				/* If a global variable */
				if (a < MAXGLOBALS)
				{
					if (a==wordcount) words = var[wordcount];
				}

				return;
			}

			obj = var[a];
			codeptr += 2;
			t = SetCompound(t);

			break;
		}

		case WORD_T:			/* "word" */
		{
			codeptr += 2;           /* skip "[" */
			n = GetValue();
#if defined (DEBUGGER)
			if ((debug_eval) && debug_eval_error)
				return;
#endif
			codeptr += 2;           /* skip "] =" */
			inexpr = 1;
			SetupExpr();
			inexpr = 0;

GetNextWord:
			if (n >= MAXWORDS) n = MAXWORDS-1;
			SaveUndo(WORD_T, n, wd[n], 0, 0);
			wd[n] = EvalExpr(0);

			if (MEM(codeptr)==COMMA_T)
			{
				codeptr++;
				n++;
				goto GetNextWord;
			}

			/* Have to (rather unfortunately) rebuild the entire
			   input buffer and word array here
			*/
			strcpy(buffer, "");
			t = 0;
			for (a=1; a<=(int)MAXWORDS; a++)
			{
				if ((unsigned short)wd[a]!=UNKNOWN_WORD)
					strcpy(buffer+t, GetWord(wd[a]));
				else
					itoa(parsed_number, buffer+t, 10);
			        word[a] = buffer + t;
				t+=strlen(word[a])+1;
			}

			if (n>(unsigned)var[wordcount])
				var[wordcount] = n;

			return;
		}

		case ARRAYDATA_T:
		case ARRAY_T:
		{
			char af_flag = false;
			/* array[n]... */
			if (t==ARRAYDATA_T)
			{
				m = PeekWord(codeptr + 1);
				codeptr += 4;   /* "[" */
				n = GetValue();
#if defined (DEBUGGER)
				if ((debug_eval) && debug_eval_error)
					return;
#endif
				codeptr++;      /* "]" */
			}

			/* ...or array val[n] */
			else
			{
				codeptr++;
				m = GetValue();
#if defined (DEBUGGER)
				if ((debug_eval) && debug_eval_error)
					return;
#endif
				codeptr++;      /* "[" */
				n = GetValue();
#if defined (DEBUGGER)
				if ((debug_eval) && debug_eval_error)
					return;
#endif
				codeptr++;      /* "]" */
			}

			if (game_version>=22)
			{
				/* Convert to word value */
				m*=2;

				if (game_version>=23)
					/* Space for array length */
					a = 2;
			}

#if defined (DEBUGGER)
			CheckinRange(m+a+n*2, debug_workspace, "array data");
#endif
			/* Check for ++, --, +=, etc. */
			inc = IsIncrement(codeptr);
			if (inc)
			{
				defseg = arraytable;
				v = PeekWord(m+a+n*2);
				defseg = gameseg;
				v = (Increment(v, inc)) + incdec;

				codeptr++;      /* eol */

				element = m+a+n*2;

				goto WriteArrayValue;
			}

			if (MEM(codeptr)==EQUALS_T)
			{
				codeptr++;

				do
				{
					element = m+a+n*2;

					temparrexpr = arrexpr;
					arrexpr = true;

					/* anonymous function */
					if (!inc && MEM(codeptr)==EOL_T)
					{
						v = GetAnonymousFunction(codeptr+1);
						af_flag = true;
					}
					else
					{
						v = GetValue();
					}
#if defined (DEBUGGER)
					if ((debug_eval) && debug_eval_error)
						return;
#endif
					if (arrexpr==false && MEM(codeptr-1)==76)
						codeptr--;
					arrexpr = temparrexpr;

					if (!af_flag && (MEM(codeptr)==COMMA_T || MEM(codeptr)==CLOSE_BRACKET_T))
						codeptr++;
WriteArrayValue:
					defseg = arraytable;

					/* Make sure the value to be written is within range */
					if ((element>0) && (element < (unsigned)(dicttable-arraytable)*16))
					{
						SaveUndo(ARRAYDATA_T, m+a, n, PeekWord(element), 0);

						PokeWord(element, (unsigned int)v);
					}

					defseg = gameseg;

					if (inc || af_flag) return;

					n++;
				}
				while (MEM(codeptr)!=EOL_T);

				codeptr++;
				return;
			}

			defseg = arraytable;
			obj = PeekWord((unsigned int)(m+a + n*2));
			defseg = gameseg;
			t = SetCompound(t);

			break;
		}
	}


StoreVal:

	/* Now store the evaluated expression in the appropriate place... */

	/*
		t = 1:  property
		t = 2:  attribute
		t = 3:  not attribute
		t = 4:  property reference
	*/

	n = 1;

	if (t==4)
	{
		inobj = true;
		n = GetValue();

#if defined (DEBUGGER)
		if ((debug_eval) && debug_eval_error)
			return;
#endif
		inobj = false;

LoopBack:
		if (MEM(codeptr)==IS_T || MEM(codeptr)==DECIMAL_T)
		{
			obj = GetProp(obj, set_value, n, 0);
			t = SetCompound(t);
			goto LoopBack;
		}
		/* Don't set t = 1 if it changed above before going back
		   to LoopBack:
		*/
		else if (t==4)
			t = 1;          /* Just a property */
	}
	else if (t==1)
	{
		while (MEM(codeptr)==IS_T || MEM(codeptr)==DECIMAL_T)
		{
			obj = GetProp(obj, set_value, n, 0);
			t = SetCompound(t);
		}
	}

	switch (t)
	{
		case 1:
		{
			incdec = 0;

			if (MEM(codeptr) != EQUALS_T)
			{
				/* Check for ++, --, +=, etc. */
				if (!(inc = IsIncrement(codeptr)))
				{
#if defined (DEBUGGER)
					if (debug_eval)
					{
						debug_eval_error = true;
						return;
					}
#endif
					FatalError(ILLEGAL_OP_E);
				}
				else if (MEM(codeptr)==EOL_T)
				{
					goto GetNextPropVal;
				}
			}
			else
				codeptr++;

			/* Property routine (anonymous function)... */
			if (MEM(codeptr)==EOL_T)
			{
				/* m = skipptr to the end of the property
				   routine block (i.e., the next statement
				   following it)
				*/
				m = PeekWord(codeptr + 1);

				newl = PROP_ROUTINE;
				newp =(unsigned int)(((codeptr + 4)+(codeptr + 4)%address_scale)/address_scale);
				codeptr = (long)m*address_scale;
				m = PropAddr(obj, set_value, 0);
			}

			/* ...or not */
			else
			{
GetNextPropVal:
				inexpr = false;
				temparrexpr = multiprop;
				multiprop = true;
				propval = true;
				if (!inc) newp = GetValue();
#if defined (DEBUGGER)
				if ((debug_eval) && debug_eval_error)
					return;
#endif

				if (!multiprop)
					codeptr--;
				multiprop = temparrexpr;

				m = PropAddr(obj, set_value, 0);
				if (m)
				{
					defseg = proptable;
					newl = Peek((unsigned int)m + 1);
					if (newl==PROP_ROUTINE) newl = 1;
				}

				/* Deal with setting built-in display object
				   properties
				*/
				else if ((obj==display_object) && n==1)
				{
					if (set_value==title_caption)
					{
						strncpy(game_title, GetWord(newp), MAX_GAME_TITLE);
						hugo_setgametitle(game_title);
					}
					else if (set_value==needs_repaint)
					{
						display_needs_repaint = (char)newp;
					}
				}
#if defined (DEBUGGER)
/*
				else if (runtime_warnings)
				{
					RuntimeWarning("Setting non-existent property");
				}
*/
#endif
			}

			/* Write property obj.z = newl words of newp */

			if (m && (int)n <= 0)
			{
#if defined (DEBUGGER)
				RuntimeWarning("Property element <= 0");
#endif
				if (inc) codeptr++;
			}
			else if (m && (int)n <= newl)
			{
				defseg = proptable;

#if defined (DEBUGGER)
				CheckinRange((unsigned)n, (unsigned)Peek(m+1), "property element");
#endif
				/* Check to make sure this property value is within range */
				if ((unsigned)(m+2+(n-1)*2)<(unsigned)(eventtable-proptable)*16)
				{
					SaveUndo(PROP_T, obj, (unsigned int)set_value, n, PeekWord((unsigned int)(m+2+(n-1)*2)));

					/* Save the (possibly changed) length) */
					Poke((unsigned int)m + 1, (unsigned char)newl);

					/* An assignment such as obj.prop++ or
					   obj.prop += ...
					*/
					if (inc)
					{
						PokeWord((unsigned int)(m+2+(n-1)*2), Increment(PeekWord((unsigned int)(m+2+(n-1)*2)), inc) + incdec);
						codeptr++;      /* eol */
					}

					/* A regular obj.prop = ... assignment */
					else
						PokeWord((unsigned int)(m+2+(n-1)*2), newp);
				}
			}
			else if (inc) codeptr++;	/* eol */

			defseg = gameseg;

			if (inc) return;

			if (propval && MEM(codeptr)==COMMA_T)
				{n++;
				codeptr++;
				goto GetNextPropVal;}

			if (propval) codeptr++;
			if (MEM(codeptr)==EOL_T) codeptr++;
			return;
		}

		case 2:
		case 3:
		{
ModifyAttribute:

#if defined (DEBUGGER)
			CheckinRange((unsigned int)set_value, (unsigned)attributes, "attribute");
#endif
			SaveUndo(ATTR_T, obj, (unsigned int)set_value, TestAttribute(obj, (unsigned int)set_value, 0), 0);

			SetAttribute(obj, set_value, (t==2));
			t = 2;	/* reset after 'not' */

			if (MEM(codeptr++)==EOL_T) return;

			/* Allow multiple attributes, comma-separated */
			if (MEM(codeptr)==COMMA_T)
				codeptr++;

			if (MEM(codeptr)==NOT_T)
			{
				t = 3;
				codeptr++;
			}

			set_value = GetValue();
			goto ModifyAttribute;
		}

		default:
		{
#if defined (DEBUGGER)
			if (debug_eval)
			{
				debug_eval_error = true;
				return;
			}
#endif
			/* Not any sort of variable data type */
			FatalError(ILLEGAL_OP_E);
		}
	}
}

unsigned int Hugo::GetAnonymousFunction(long addr) {
	long skipaddr;
	unsigned int af_addr;
	
	skipaddr = PeekWord(addr);
	/* The address of the anonymous function is the next address boundary,
	   calculated as:
	   (((addr+2)/address_scale+1)*address_scale)/address_scale */
	af_addr =(unsigned int)((addr+2)/address_scale+1);
	codeptr = (long)skipaddr*address_scale;
	return af_addr;
}

int Hugo::SetCompound(int t) {
	if (Peek(codeptr)==DECIMAL_T)		/* obj.property */
	{
		codeptr++;
		inobj = 1;
		set_value = GetValue();		/* the prop. # */
		inobj = 0;

		if (Peek(codeptr)==POUND_T)	/* if obj.prop #... */
		{
			codeptr++;
			return 4;
		}
		return 1;
	}

	if (Peek(codeptr)==IS_T)		/* obj is ... */
	{
		inobj = 1;
		if (Peek(codeptr+1)==NOT_T)
		{
			codeptr += 2;
                        set_value = GetValue();	/* the attr. # */
			inobj = 0;
			return 3;
		}

		codeptr++;
		set_value = GetValue();		/* the attr. # */
		inobj = 0;
		return 2;
	}

#if defined (DEBUGGER)
	if (debug_eval)
		debug_eval_error = true;
	else
#endif

	FatalError(ILLEGAL_OP_E);

	return 0;
}

} // End of namespace Hugo
} // End of namespace Glk
