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

#if defined (DEBUGGER)

int Hugo::CheckObjectRange(int obj) {
	if (runtime_warnings) {
		return CheckinRange((unsigned)obj, (unsigned)objects, "object");
	} else
		return true;
}

#endif

int Hugo::Child(int obj) {
	int c;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	defseg = objtable;

	c = PeekWord(2 + obj*object_size + object_size - 4);

	defseg = gameseg;

	return c;
}

int Hugo::Children(int obj) {
	int count = 0;
	int nextobj;

	if (obj<0 || obj>=objects) return 0;

	nextobj = Child(obj);
	while (nextobj)
		{count++;
		nextobj = Sibling(nextobj);}
	return count;
}

int Hugo::Elder(int obj) {
	int lastobj;
	int p, cp;

	if (obj<0 || obj>=objects) return 0;

	p = Parent(obj);
	cp = Child(p);

        if (p==0 || cp==obj)
		return 0;

	lastobj = cp;
	while (Sibling(lastobj) != obj)
		lastobj = Sibling(lastobj);

	return lastobj;
}

unsigned long Hugo::GetAttributes(int obj, int attribute_set) {
	unsigned long a;

	defseg = objtable;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	a = (unsigned long)PeekWord(2 + obj*object_size + attribute_set*4)
		+ (unsigned long)PeekWord(2 + obj*object_size + attribute_set*4 + 2)*65536L;

	defseg = gameseg;

	return a;
}

int Hugo::GetProp(int obj, int p, int n, char s) {
	char objonly,			/* no verbroutine given in before, etc. */
		isadditive = 0,		/* before, after, etc.			*/
		gotone = 0,             /* when a match has been made           */
		getpropaddress = 0;     /* when getting &object.property        */
	int i;
	int tempself,
		objtype,		/* i.e., what we're matching to		*/
		flag = 0;
	int g = 0;
	int templocals[MAXLOCALS];
	int temp_stack_depth;
	char tempinexpr = inexpr;
	unsigned int pa,		/* property address			*/
		offset = 0;
	long inprop,			/* code position in complex property	*/
		returnaddr;
#if defined (DEBUGGER)
	long orig_inprop;
	int tempdbnest;

	/* Don't check a possible non-existent display object (-1) */
	if (obj!=-1 || display_object!=-1) CheckObjectRange(obj);
#endif
	/* This way either -1 (the non-existent display object) or a too-high
	   object will fail
	*/
	if (obj<0 || obj>=objects) return 0;

	/* The display object, which is automatically created by the compiler,
	   did not exist pre-v2.4
	*/
	if ((obj==display_object) && game_version>=24)
	{
		/* There are no actual default "properties" per se on the
		   display object--but reading them returns certain data about
		   the current state of display affairs
		*/

		/* no display.<prop> #2, etc. */
		if (n==1 && p<=pointer_y)
		{
			if (p==screenwidth)
#if defined (GLK) && defined (ACTUAL_LINELENGTH)
				g = ACTUAL_LINELENGTH();
#else
				g = SCREENWIDTH/FIXEDCHARWIDTH;
#endif
			else if (p==screenheight)
/* ACTUAL_SCREENHEIGHT can be set to a non-portable function if
   SCREENHEIGHT and SCREENHEIGHT have been set to large values in
   order to force the non-portable layer to handle wrapping and
   scrolling (as in the Glk port).
*/
#if defined (ACTUAL_SCREENHEIGHT)
				g = ACTUAL_SCREENHEIGHT();
#else
				g = SCREENHEIGHT/FIXEDLINEHEIGHT;
#endif
			else if (p==linelength)
/* ACTUAL_LINELENGTH functions similarly to ACTUAL_SCREENWIDTH,
   above.
*/
#if defined (ACTUAL_LINELENGTH)
				g = ACTUAL_LINELENGTH();
#else
				g = physical_windowwidth/FIXEDCHARWIDTH;
#endif
			else if (p==windowlines)
				g = physical_windowheight/FIXEDLINEHEIGHT;
			else if (p==cursor_column)
				g = (currentpos+1+hugo_textwidth(pbuffer))/FIXEDCHARWIDTH;
			else if (p==cursor_row)
				g = currentline;
			else if (p==hasgraphics)
				g = hugo_hasgraphics();
			else if (p==title_caption)
				g = FindWord(game_title);
			else if (p==hasvideo)
#if !defined (COMPILE_V25)
				g = hugo_hasvideo();
#else
				g = 0;
#endif
			else if (p==needs_repaint)
				g = display_needs_repaint;
			else if (p==pointer_x)
				g = display_pointer_x;
			else if (p==pointer_y)
				g = display_pointer_y;
			else
				g = 0;

			return g;
		}
	}

	/* To avoid prematurely getting an address in &obj.prop.prop */
	if (getaddress && MEM(codeptr)!=DECIMAL_T)
		getpropaddress = true;

        tempself = var[self];
	if (!s) var[self] = obj;

	temp_stack_depth = stack_depth;


GetNextProp:

	pa = PropAddr(obj, p, offset);

	defseg = proptable;

	/* If the object doesn't have property p, see if there's a
	   default value.
	*/
	if (!pa)
	{
		if (offset) goto NoMorePropMatches;

		if (getpropaddress)             /* if an &value */
			g = 0;
		else
			g = PeekWord(p * 2 + 2);
	}

	else
	{
		/* Property is a value... */
		if (Peek(pa+1) < PROP_ROUTINE)
		{
			if (getaddress || (int)Peek(pa+1) < n || n<=0)
			{
#if defined (DEBUGGER)
				if (n!=1)
					CheckinRange(n, (int)Peek(pa+1), "property element");
#endif
				g = 0;
			}
			else
				g = PeekWord(pa + n * 2);
		}

		/* ...or a property routine */
		else
		{
			/* Check if this is an additive property */
			defseg = proptable;
			if (Peek(2 + Peek(0)*2 + p)&ADDITIVE_FLAG)
				isadditive = true;

			/* If an &value, return the address of the
			   property routine.
			*/
			if (getpropaddress)
			{
				g = PeekWord(pa+2);
				goto NoMorePropMatches;
			}
			else
			{
#if defined (DEBUGGER)
				if (debug_eval)
				{
					debug_eval_error = true;
					DebugMessageBox("Expression Error",
						"Property routine illegal in watch/assignment");
					defseg = gameseg;
					return 0;
				}
#endif
				/* If not a complex property such as
				   before or after:
				*/
				if ((game_version>=22 && (Peek(2 + Peek(0)*2 + p)&COMPLEX_FLAG)==0) || (game_version<22 && p!=before && p!=after))
				{
					ret = 1;
					returnaddr = codeptr;

					/* Check to see if this is a valid tail-recursive return... */
					if (tail_recursion==TAIL_RECURSION_PROPERTY && MEM(codeptr)==EOL_T)
					{
						PassLocals(0);
						tail_recursion_addr = (long)PeekWord(pa+2)*address_scale;
						return 0;
					}
					/* ...but if we're not immediately followed by and end-of-line marker,
					   or another property value, cancel the pending tail-recursion
					*/
					else if (MEM(codeptr)!=DECIMAL_T)
					{
						tail_recursion = 0;
					}

					for (i=0; i<MAXLOCALS; i++)
						templocals[i] = var[MAXGLOBALS+i];

					PassLocals(0);

					SetStackFrame(stack_depth+1, RUNROUTINE_BLOCK, 0, 0);
#if defined (DEBUGGER)
					tempdbnest = dbnest;
					DebugRunRoutine((long)PeekWord(pa+2)*address_scale);
					dbnest = tempdbnest;
#else
					RunRoutine((long)PeekWord(pa+2)*address_scale);
#endif

					retflag = 0;
					codeptr = returnaddr;
					g = ret;
					ret = 0;
				}

		                /* Complex property: */
				else
				{
					for (i=0; i<MAXLOCALS; i++)
						templocals[i] = var[MAXGLOBALS+i];

					inprop = (long)PeekWord(pa + 2)*address_scale;
#ifdef DEBUGGER
					orig_inprop = inprop;
#endif
					defseg = gameseg;
					while (Peek(inprop)!=CLOSE_BRACE_T)
					{
						returnaddr = codeptr;

						codeptr = inprop;
						objonly = false;
						objtype = GetValue();
						inprop = codeptr;
						codeptr = returnaddr;

						flag = 0;

						/* If only an object (or other variable) is
						   given, and no verbroutine
						*/
						if (Peek(inprop)==JUMP_T)
						{
							objonly = true;
							if (!gotone && obj==objtype) flag = 1;
						}

						/* Otherwise, one or more verbroutines are
						   specified
						*/
						else
						{
							while (Peek(inprop)!=JUMP_T)
							{
								if (PeekWord(inprop+1)==(unsigned int)var[verbroutine] ||

									/* This is necessary because of the awkward way the pre-v2.2
									   differentiated non-verbroutine blocks, i.e., with Parse
									*/
									((game_version<22) && PeekWord(inprop+1)==(unsigned int)parseaddr && !gotone))
								{
									if (obj==objtype) flag = 1;
								}
								inprop += 3;
							}
						}

						if (flag==1)
						{
							gotone = true;
							ret = 1;
							returnaddr = codeptr;

							SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

							PassLocals(0);
#if defined (DEBUGGER)
							/* Prevent premature stopping */
							if (debugger_step_over && !debugger_finish)
								debugger_run = true;
							
							if (IsBreakpoint(orig_inprop))
								complex_prop_breakpoint = true;
							
							sprintf(debug_line, "Calling:  %s.%s", objectname[obj], propertyname[p]);
							trace_complex_prop_routine = true;

							tempdbnest = dbnest;
							DebugRunRoutine(inprop+3);
							dbnest = tempdbnest;
#else
							RunRoutine(inprop+3);
#endif
							retflag = 0;
							codeptr = returnaddr;
							g = ret;
							ret = 0;
						}

						/* The following used to read "if (!flag || objonly..."
						   meaning that any non-verbroutine related routines
						   would fall through regardless of whether they returned
						   true or false.  I don't recall the rationale for this,
						   and have therefore removed it.
						*/
						if (!flag || (objonly && !g) || ((game_version<22) && PeekWord(inprop-2)==(unsigned int)parseaddr))
                                                        inprop = (long)PeekWord(inprop+1)*address_scale;
						else break;
					}
				}

				for (i=0; i<MAXLOCALS; i++)
					var[MAXGLOBALS+i] = templocals[i];

				if (isadditive && !g)
				{
					offset = pa + 4;
					gotone = false;
					goto GetNextProp;
				}
			}
		}
	}

NoMorePropMatches:

	defseg = gameseg;

	var[self] = tempself;
	inexpr = tempinexpr;
	stack_depth = temp_stack_depth;

	return g;
}

int Hugo::GrandParent(int obj) {
	int nextobj;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	defseg = objtable;
	while ((nextobj = PeekWord(2 + obj*object_size + object_size-8)) != 0)
		obj = nextobj;
	defseg = gameseg;

	return obj;
}

void Hugo::MoveObj(int obj, int p) {
	int oldparent, prevobj, s;
	unsigned int objaddr, parentaddr, lastobjaddr;

	if (obj==p) return;
	if (obj<0 || obj>=objects) return;

	oldparent = Parent(obj);
	/* if (oldparent==p) return; */

	objaddr = 2 + obj*object_size;

	/* First, detach the object from its old parent and siblings... */

	prevobj = Elder(obj);
	s = Sibling(obj);
	defseg = objtable;
	if (prevobj)                                    /* sibling */
		PokeWord(2 + prevobj*object_size + object_size-6, s);
	else                                            /* child */
		PokeWord(2 + oldparent*object_size + object_size-4, s);


	/* Then move it to the new parent... */

	defseg = objtable;
	PokeWord(objaddr + object_size-8, p);   /* new parent 		*/
	PokeWord(objaddr + object_size-6, 0);   /* erase old sibling 	*/

	/* Only operate on the new parent if it isn't object 0 */
	if (p!=0)
	{

		/* Object is sole child, or... */
		if (Child(p)==0)
		{
			parentaddr = 2 + p*object_size;
			defseg = objtable;
			PokeWord(parentaddr + object_size-4, obj);
		}

		/* ...object is next sibling. */
		else
		{
			lastobjaddr = 2 + Youngest(p)*object_size;
			defseg = objtable;
			PokeWord(lastobjaddr + object_size-6, obj);
		}
	}
}

const char *Hugo::Name(int obj) {
	int p;

	p = GetProp(obj, 0, 1, 0);

	if (p)
		return GetWord((unsigned int)p);
	else
		return nullptr;
}

int Hugo::Parent(int obj) {
	int p;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	defseg = objtable;

	p = PeekWord(2 + obj*object_size + object_size-8);

	defseg = gameseg;

	return p;
}

unsigned int Hugo::PropAddr(int obj, int p, unsigned int offset) {
	unsigned char c;
	int proplen;
	unsigned int ptr;

#if defined (DEBUGGER)
	/* Don't check any non-existent display object (-1) */
	if (p!=-1) CheckinRange(p, properties, "property");
	CheckObjectRange(obj);
#endif
	/* This way either -1 (the non-existent display object) or a too-high
	   object will fail
	*/
	if (obj<0 || obj>=objects) return 0;

	defseg = objtable;

	/* Position in the property table...

	   i.e., ptr = PeekWord(2 + obj*object_size + (object_size-2));
	*/
	ptr = PeekWord(object_size*(obj+1));

	/* ...unless a position has already been given */
	if (offset) ptr = offset;

	defseg = proptable;

	c = Peek(ptr);
	while (c != PROP_END && c != (unsigned char)p)
	{
		proplen = Peek(ptr + 1);

		/* Property routine address is 1 word */
		if (proplen==PROP_ROUTINE) proplen = 1;

		ptr += proplen * 2 + 2;
		c = Peek(ptr);
	}

	defseg = gameseg;

	if (c==PROP_END)
		return 0;
	else
		return ptr;
}

void Hugo::PutAttributes(int obj, unsigned long a, int attribute_set) {
	unsigned int lword, hword;

	hword = (unsigned int)(a/65536L);
	lword = (unsigned int)(a%65536L);

	defseg = objtable;

	PokeWord(2 + obj*object_size + attribute_set*4, lword);
	PokeWord(2 + obj*object_size + attribute_set*4 + 2, hword);

	defseg = gameseg;
}

void Hugo::SetAttribute(int obj, int attr, int c) {
	unsigned long a, mask;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return;
#endif
	if (obj<0 || obj>=objects) return;

	a = GetAttributes(obj, attr/32);

	mask = 1L<<(long)(attr%32);

	if (c==1)
		a = a | mask;
	else
	{
		if (a & mask)
			a = a ^ mask;
	}

	PutAttributes(obj, a, attr/32);
}

int Hugo::Sibling(int obj) {
	int s;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	defseg = objtable;

	s = PeekWord(2+ obj*object_size + object_size-6);

	defseg = gameseg;

	return s;
}

int Hugo::TestAttribute(int obj, int attr, int nattr) {
	unsigned long a, mask, ta;

#if defined (DEBUGGER)
	if (!CheckObjectRange(obj)) return 0;
#endif
	if (obj<0 || obj>=objects) return 0;

	a = GetAttributes(obj, attr/32);

	mask = 1L<<(attr%32);

	ta = a & mask;
	if (ta) ta = 1;

	if (nattr) ta = ta ^ 1;

	return (int)ta;
}

int Hugo::Youngest(int obj) {
	int nextobj;

	if (Child(obj)==0) return 0;

	nextobj = Child(obj);

	while (Sibling(nextobj))
		nextobj = Sibling(nextobj);

	return nextobj;
}

} // End of namespace Hugo
} // End of namespace Glk
