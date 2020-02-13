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

#define STARTS_AS_NUMBER(a) (((a[0]>='0' && a[0]<='9') || a[0]=='-')?1:0)

static char EMPTY[1] = { 0 };

void Hugo::AddAllObjects(int loc) {
	int i;

	if (loc==var[player] && domain!=loc)
		return;

	/* Try to add everything in the specified domain
	   to objlist[]
	*/
	for (i=Child(loc); i!=0; i=Sibling(i))
	{
		if (i==var[xobject]) continue;

		TryObj(i);
		if (domain==0)
		{
			if (Child(i)) AddAllObjects(i);
		}
	}
}

void Hugo::AddObj(int obj) {
	int i;

	for (i=0; i<objcount; i++)
	{
		if (objlist[i]==obj)
			return;
	}

	objlist[(int)objcount] = obj;
	if (++objcount> MAXOBJLIST) objcount = MAXOBJLIST;
}

void Hugo::AddPossibleObject(int obj, char type, unsigned int w) {
	int i;

	if (pobjcount==MAXPOBJECTS)
		return;

	for (i=0; i<pobjcount; i++)
	{
		/* If it is already in the list */
		if (pobjlist[i].obj==obj)
		{
			/* Being referred to with a noun outweighs being
			   referred to previously with an adjective
			*/
			if (type==(char)noun || ObjWordType(obj, w, noun))
				pobjlist[i].type = (char)noun;

			return;
		}
	}

	/* Getting this to point is presuming that we're adding an object
	   referred to with an adjective, but check just to be sure it isn't
	   also a noun for that same object
	*/
	if (ObjWordType(obj, w, noun)) type = (char)noun;
	
	pobjlist[pobjcount].obj = obj;
	pobjlist[pobjcount].type = type;

	pobjcount++;
#ifdef DEBUG_PARSER
{
	char buf[100];
	sprintf(buf, "AddPossibleObject(%d:\"%s\")", obj, Name(obj));
	Printout(buf);
}
#endif
}

void Hugo::AdvanceGrammar() {
	int a;

	defseg = gameseg;

	switch (a = Peek(grammaraddr))
	{
		case FORWARD_SLASH_T:
		case HELD_T:
		case MULTI_T:
		case MULTIHELD_T:
		case ANYTHING_T:
		case NUMBER_T:
		case PARENT_T:
		case NOTHELD_T:
		case MULTINOTHELD_T:
		case WORD_T:
		case OBJECT_T:
		case XOBJECT_T:
		case STRING_T:
			grammaraddr++;
			break;

		case ASTERISK_T:
		case ATTR_T:
			grammaraddr += 2;
			break;

		case DICTENTRY_T:
		case ROUTINE_T:
		case OBJECTNUM_T:
			grammaraddr += 3;
			break;

		case OPEN_BRACKET_T:
			grammaraddr +=5;
			break;
	}
}

int Hugo::AnyObjWord(int wn) {
	int i;
	
	if (objword_cache[wn])
		return objword_cache[wn];

	for (i=0; i<objects; i++)
	{
		if (ObjWord(i, wd[wn]))
		{
			return (objword_cache[wn] = 1);
		}
	}

	return (objword_cache[wn] = -1);
}

int Hugo::Available(int obj, char non_grammar) {
	int temp_stack_depth;

	if (findobjectaddr)
	{
		passlocal[0] = obj;

		/* if anything or (Routine) grammar */
		if ((Peek(grammaraddr)==ANYTHING_T
			|| (Peek(grammaraddr)==OPEN_BRACKET_T && Peek(grammaraddr+1)==ROUTINE_T))
			&& non_grammar==0)
		{
			passlocal[1] = 0;
		}
		else
		{
			if (domain > 0)
				passlocal[1] = domain;
			else if (speaking && non_grammar==0)
				passlocal[1] = GrandParent(speaking);
			/* domain of -1 is an explicit 'parent' */
/*
			else if (domain==-1)
				passlocal[1] = parse_location;
*/
			else
				passlocal[1] = parse_location;
		}

		ret = 0;

		PassLocals(2);
		temp_stack_depth = stack_depth;

		SetStackFrame(stack_depth, RUNROUTINE_BLOCK, 0, 0);

#if defined (DEBUGGER)
		DebugRunRoutine((long)findobjectaddr*address_scale);
#else
		RunRoutine((long)findobjectaddr*address_scale);
#endif
		retflag = 0;
		stack_depth = temp_stack_depth;
		return ret;
	}
	else
		return 1;
}

void Hugo::CallLibraryParse() {
	if (parseaddr)
	{
#ifdef DEBUG_PARSER
		Printout("CallLibraryParse()");
#endif
		parse_called_twice = false;

		SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

		ret = 0;
		PassLocals(0);
#if defined (DEBUGGER)
		DebugRunRoutine((long)parseaddr*address_scale);
#else
		RunRoutine((long)parseaddr*address_scale);
#endif
		retflag = 0;

		/* Returning non-zero return calls the
		   engine's Parse routine again.
		*/
		if (ret)
		{
			parse_called_twice = true;
			Parse();
		}
#ifdef DEBUG_PARSER
		if (ret)
			Printout("CallLibraryParse() returned true");
		else
			Printout("CallLibraryParse() returned false");
#endif
	}
}

int Hugo::DomainObj(int obj) {
	int yes = false;

	if (obj != var[actor])
	{
		switch (domain)
		{
			case 0:
			case -1:
			{
				if (Parent(obj)==parse_location)
					yes = true;
				else if ((parse_allflag) && GrandParent(obj)==parse_location)
					yes = true;
				else
				{
					if (Parent(obj)==parse_location && !InList(Parent(obj)))
						yes = true;
				}

				if (Peek(grammaraddr)==MULTINOTHELD_T)
				{
					if (Parent(obj)==var[actor])
						yes = false;
				}
				break;
			}

			default:
			{
				if (Parent(obj)==domain)
					yes = true;
			}
		}
	}

	return yes;
}

unsigned int Hugo::FindWord(const char *a) {
	unsigned int ptr = 0;
	int i, p, alen;

	if (a[0]=='\0')
		return 0;

	alen = strlen(a);

	defseg = dicttable;

	for (i=1; i<=dictcount; i++)
	{
		if (alen==(p = Peek(ptr+2)) && (unsigned char)(MEM(dicttable*16L+ptr+3)-CHAR_TRANSLATION)==(unsigned char)a[0])
		{
			if (!strcmp(GetString(ptr + 2), a))
			{
				defseg = gameseg;
				return ptr;
			}
		}

		ptr += (p + 1);
	}
	
	/* As a last resort, see if the first 6 characters of the word (if it
	   has at least six characters) match a dictionary word:
	*/
	if (alen >= 6)
	{
		unsigned int possible = 0;
		int posscount = 0;

		ptr = 0;
	
		for (i=1; i<=dictcount; i++)
		{
			if (alen<=(p = Peek(ptr+2)) && (unsigned char)MEM(dicttable*16L+ptr+3)-CHAR_TRANSLATION==a[0])
			{
				if (!strncmp(GetString(ptr + 2), a, alen))
				{
					/* As long as the dictionary word 
					   doesn't contain a space */
					if (!strrchr(GetString(ptr+2), ' '))
					{
						possible = ptr;
						posscount++;
					}
				}
			}
			ptr += (p + 1);
		}
		
		if (posscount==1)
			return possible;
	}

	defseg = gameseg;

	return UNKNOWN_WORD;                    /* not found */
}

int Hugo::InList(int obj) {
	int i;

	for (i=0; i<objcount; i++)
	{
		if (objlist[i]==obj)
			return true;
	}
	return false;
}

void Hugo::KillWord(int a) {
	int i;

	if (a>words)
		return;

	for (i=a; i<words; i++)
		word[i] = word[i+1];
	word[words] = EMPTY;

	RemoveWord(a);
	words--;
}

int Hugo::MatchCommand() {
	int i, j, flag, a, mw = 0, gotspeaker = 0;
	int wordnum = 0;
	int numverbs = 0;
	bool nextverb = false;
	unsigned int ptr, verbptr, nextgrammar;
	unsigned int obj, propaddr;

#ifdef DEBUG_PARSER
	Printout("Entering MatchCommand()");
#endif

	odomain = 0;
	
	/* Reset these for command-matching */
	if (!speaking)
	{
		var[actor] = var[player];
		parse_location = var[location];
	}
	else
	{
		var[actor] = speaking;
		parse_location = GrandParent(speaking);
	}

	if (!strcmp(word[1], "~oops"))
	{
		strcpy(parseerr, "");

		/* "oops" on its own */
		if (words==1 || !strcmp(oops, ""))
		{
			ParseError(16, 0);       /* "You'll have to make a mistake..." */
			return 0;
		}

		/* trying to correct more than one word */
		if (words > 2 || oopscount)
		{
			ParseError(17, 0);       /* "...one word at a time..." */
			return 0;
		}

		/* trying to correct a correction */
		if (!strcmp(Left(errbuf, 5), "~oops"))
		{
			ParseError(13, 0);
			return 0;
		}

		/* Rebuild the corrected buffer */
		oopscount = 1;
		strcpy(line, word[2]);
		for (i=1; i<=(int)strlen(errbuf); i++)
		{
			if (!strcmp(Mid(errbuf, i, strlen(oops)), oops))
				break;
		}

		strcpy(buffer, errbuf);
		buffer[i-1] = '\0';
		strcat(buffer, line);

		strcat(buffer, Right(errbuf, strlen(errbuf) - i - strlen(oops) + 1));

		SeparateWords();
		if (!Parse())
			return 0;

		CallLibraryParse();
	}

	if (word[1][0]=='.') KillWord(1);


	/*
	 * STEP 1:  Match verb
	 *
	 */

	ptr = 64;

MatchVerb:

#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Step 1");
#endif
	if (words==0)
		return 0;

	defseg = gameseg;

	grammaraddr = 0;
	domain = 0;
	obj_match_state = 0;
	xverb = 0;
	starts_with_verb = 0;
	objcount = 0;
	parse_allflag = false;
	objstart = 0;
	object_is_number = false;

	for (i=1; i<MAXWORDS; i++)
		objword_cache[i] = 0;

	var[object] = 0;
	var[xobject] = 0;
	var[self] = 0;
	var[verbroutine] = 0;

	while ((a = Peek(ptr)) != 255)
	{
		defseg = gameseg;

		/* verb or xverb header */
		if (a==VERB_T || a==XVERB_T)
		{
			/* Skim through 1 or more verb words */
			numverbs = Peek(ptr + 1);
			verbptr = ptr + 2;
			for (i=1; i<=numverbs; i++)
			{
				/* 0xffff signals something other than a 
				   dictionary word--see BuildVerb() in hcbuild.c.
				   This will be the case, like, 1% of the time.
				*/

				/* If it is a dictionary word... */
				if (PeekWord(verbptr)!=0xffff)
				{
					/* If one of the verb words matches the first
					   word in the input line
					*/
					if (wd[1]==PeekWord(verbptr))
					{
						grammaraddr = ptr;
						goto GotVerb;
					}

					verbptr += 2;
				}

				/* ...otherwise assume it's an object (value) */
				else
				{
					codeptr = verbptr + 1; /* skip 0xffff */

					/* GetVal(), not GetValue(), since it's
					   always a simple value
					*/
					obj = GetVal();

					/* codeptr can't be >65535 on a 16-bit
					   compiler
					*/
					verbptr = (unsigned int)codeptr;
					propaddr = PropAddr(obj, noun, 0);
					if (propaddr)
					{
						defseg = proptable;
						a = Peek(propaddr+1);	/* obj.#prop */
						defseg = gameseg;

						for (j=1; j<=a; j++)
						{
							if (wd[1]==(unsigned)GetProp(obj, noun, j, 0))
							{
								grammaraddr = ptr;
								goto GotVerb;
							}
						}
					}
				}
			}

			/* Otherwise skip over this verb header */
			ptr += 2 + numverbs * 2;
		}

		/* anything else */
		else
			ptr += Peek(ptr + 1) + 1;
	}


	/*
	 * STEP 2:  Match object/character (if no verb match)
	 *
	 */
#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Step 2");
#endif
	/* If we hit the end of the grammar without finding a verb match: */
	if (Peek(ptr)==255)
	{
		/* If we already tried this once */
		if (gotspeaker)
		{
			if (!starts_with_verb)
				/* "Better start with a verb..." */
				ParseError(2, 0);
			else
				/* "That doesn't make any sense..." */
				ParseError(6, 0);
			return 0;
		}

		/* See if the command begins with an object
		   (character) name:
		*/
		flag = 0;
		if (AnyObjWord(1)==1)
			flag = 1;

		/* No match, ergo an invalid command */
		if (flag==0 && nextverb==true)
		{
			strcpy(parseerr, "");
			ParseError(6, 0);        /* "...doesn't make any sense..." */
			return 0;
		}

		/* No provision made for addressing objects (characters) */
		if (flag==0 || speaktoaddr==0)
		{
			strcpy(parseerr, "");
			ParseError(2, 0);        /* "Better start with a verb..." */
			return 0;
		}

		/* Count how many object words there are */
		for (i=2; i<=words; i++)
		{
			if (AnyObjWord(i)!=1)
				break;
		}

		/* Try to match the first word to a valid object */
		objfinish = i - 1;
		obj_match_state = 5;
		i = 1;
		recursive_call = 0;
		
		if (MatchObject(&i) != true)
			return 0;  /* unsuccessful */
		
		speaking = pobj;                        /* successful */
		gotspeaker = true;

		/* So erase the object name from the start of the line */
		for (i=1; i<=objfinish; i++)
			KillWord(1);
		if (word[1][0]=='~') KillWord(1);
		
		/* If it's a name and that's all...*/
		if (words==0)
			return true;

		/* ...or else proceed as usual */
		ptr = 64;
		goto MatchVerb;
	}
	else if (!gotspeaker)
		speaking = 0;

GotVerb:

	if (!speaking)
	{
		var[actor] = var[player];
		parse_location = var[location];
	}
	else
	{
		var[actor] = speaking;
		parse_location = GrandParent(speaking);
	}

	obj_match_state = 0;
	starts_with_verb = 1;
	strcpy(parseerr, word[1]);

	if (Peek(grammaraddr)==XVERB_T) xverb = true;
	grammaraddr += 2 + numverbs * 2;

	/*
	 * STEP 3:  Match proper grammar structure (syntax)
	 *
	 */
#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Step 3");
#endif

	/*
	 * (STEP 4:  We'll be matching xobject, if any, before object)
	 *
	 */
#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Step 4");
#endif

	/* Loop until end of grammar table, or next verb:
	 */
	while (Peek(grammaraddr)!=255 &&
		Peek(grammaraddr)!=VERB_T && Peek(grammaraddr)!=XVERB_T)
	{
		wordnum = 1;

		nextgrammar = grammaraddr + Peek(grammaraddr + 1) + 1;

		/* Loop until end of table or next verb: */
		while (Peek(grammaraddr) != 255 && Peek(grammaraddr) != VERB_T && Peek(grammaraddr) != XVERB_T)
		{
			mw = MatchWord(&wordnum);

			if (mw==1)
			{
				/* end of both input and grammar */
				if (wd[wordnum]==0 && Peek(grammaraddr)==ROUTINE_T)
				{
					full_buffer = (byte)wordnum;
					break;
				}

				/* end of grammar, not input */
				if (Peek(grammaraddr)==ROUTINE_T)
				{
					mw = false;
					goto NextStructure;
				}
			}
			else
			{
				/* If error already signalled */
				if (mw==2)
					return 0;

				/* No match, so try next structure */
				else
				{
NextStructure:
					grammaraddr = nextgrammar;
					var[object] = 0;
					var[xobject] = 0;
					var[verbroutine] = 0;
					domain = 0;
					odomain = 0;
					obj_match_state = 0;
					xverb = 0;
					objcount = 0;
					objstart = 0;
					break;
				}
			}
		}

		/* Matched the complete syntax of a verb */
		if (mw==1)
		{
			var[verbroutine] = PeekWord(grammaraddr + 1);
			break;
		}
	}

	if (mw != 1)
	{
		if (mw==0)	/* mw = 2 if error already printed */
		{
			/* If there's more grammar to check... */
			if (Peek(grammaraddr) != 255)
			{
				ptr = grammaraddr;
				nextverb = true;
				goto MatchVerb;
			}

			/* ...or if we reached the end without a sensible
			   syntax matched:
			*/
			strcpy(parseerr, "");

			/* "...doesn't make any sense..." */
			ParseError(6, 0);
		}
		return 0;
	}


	/*
	 * STEP 5:  Match remaining object(s), if any
	 *
	 */
#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Step 5");
#endif

	/* If there are objects waiting to be loaded into objlist[] */
	if (objstart)
	{
		ResetFindObject();

		obj_match_state = 2;
		recursive_call = false;
		if (odomain) domain = odomain;

		grammaraddr = objgrammar;
		i = objstart;

		/* If there was a problem matching them */
		if (MatchObject(&i)==false)
			return 0;
	}

	/* Got a successfully matched verb with all the requisite
	   parameters (if any).
	*/

	if (words < wordnum)
		remaining = 0;
	else
		remaining = (char)(words - wordnum);

#ifdef DEBUG_PARSER
	Printout("MatchCommand(): Leaving");
#endif
	return true;
}

bool Hugo::MatchObject(int *wordnum) {
	char found_noun = false;
	int i, j, k, m, flag;
	int mobjs; unsigned int mobj[MAX_MOBJ];
	bool allmatch;
	int roomloc;
	int bestobj = 0;
	char bestavail = 0;

	/* If this is a recursive call, we're not adding new objects */
	if (!recursive_call) addflag = true;

	stack_depth = 0;

	pobj = 0;                          /* possible object */
	objcount = 0;                      /* of objlist[] */
	pobjcount = 0;                     /* of pobjlist[] */
	mobjs = 0;                         /* # of previous words in phrase */
	bestavail = 0;                     /* adjective or noun */

#ifdef DEBUG_PARSER
	Printout("MatchObject(): Entering");
#endif
	strcpy(parseerr, "");

	do                                 /* starting at word #a */
	{
		/* Check first to make sure it's not a housekeeping word
		   such as "~and" or "~all".
		*/
		if (word[*wordnum][0]!='~' && word[*wordnum][0]!='\0')
		{
			if (parseerr[0]!='\0') strcat(parseerr, " ");
			strcat(parseerr, word[*wordnum]);

			flag = 0;
			for (i=0; i<objects; i++)
			{
				if (wd[*wordnum]==0)
					break;

				/* Might be this object if wd[*wordnum] is an
				   adjective or noun of object i
				*/
				m = ObjWord(i, wd[*wordnum]);

				if (m)
				{
					flag = true;
					allmatch = true;

					/* check previously matched words */
					for (j=1; j<=mobjs; j++)
					{
						if (!ObjWord(i, mobj[j])) /* || wd[*wordnum]==mobj[j]) */
							allmatch = false;
					}

					/* matches all previous words */
					if (allmatch==true)
					{
						AddPossibleObject(i, (char)m, wd[*wordnum]);
						pobj = i;
						if (bestavail==0 || bestavail>=(char)m)
						{
							if (!bestobj)
								bestobj = i;
							bestavail = (char)m;
						}
					}

					/* doesn't match previous words */
					else
						SubtractPossibleObject(i);
				}

				/* definitely not this object */
				else
					SubtractPossibleObject(i);
			}


			/* If checking the start of an input line, i.e. for
			   a command addressed to an object (character):
			*/
			if (obj_match_state==5 && !flag) goto Clarify;
		}

		else if (!strcmp(word[*wordnum], "~any"))
			goto NextLoop;

		/* "~and", "~all",... */
		else
			goto Clarify;

		/* Didn't get any suspects */
		if (pobjcount==0)
		{
			/* If "~and", "~all",... */
			if (word[*wordnum][0]=='~')
			{
				/* If checking the xobject */
				if (obj_match_state==1)
				{
					strcpy(parseerr, word[1]);
					/* "...can't use multiple objects..."
					   (as indirect objects) */
					ParseError(7, 0);
					return false;
				}
				goto Clarify;
			}

			/* Got an unmatchable sequence of words */
			else
			{
				if (obj_match_state==5)
				{
					if (!starts_with_verb)
						/* "Better start with a verb..." */
						ParseError(2, 0);
					else
						/* "That doesn't make any sense..." */
						ParseError(6, 0);
				}
				else
					/* "(no such thing)..." */
					ParseError(5, 0);
				return false;
			}
		}

		if (word[*wordnum][0]!='~')
		{
			/* Go back for next word in this object phrase */

			mobjs++;
			if (mobjs==MAX_MOBJ)
			{
				/* "(no such thing)..." */
				ParseError(5, 0);
				return false;
			}
			mobj[mobjs] = wd[*wordnum];
		}
		else
		{
			/* Since hitting "~and" or "~all", we've obviously
			   finished an object phrase
			*/
			(*wordnum)++;
			goto Clarify;
		}

NextLoop:
		(*wordnum)++;                   /* next word */
		if ((*wordnum > words || word[*wordnum][0]=='\0')
			|| *wordnum > objfinish)
			goto Clarify;
	}
	while (true);                           /* endless loop */


Clarify:

#ifdef DEBUG_PARSER
	Printout("MatchObject(): Clarify");
#endif
	/* If "~and", "~all",... */
	if (word[*wordnum][0]=='~')
	{
		/* If checking the xobject or addressing a command */
		if (obj_match_state==1 || speaking)
		{
			strcpy(parseerr, word[1]);
			/* "...can't use multiple objects..."
			   (as indirect objects) */
			ParseError(7, 0);
			return false;
		}
	}

	if (!strcmp(word[*wordnum], "~all"))    /* if "~all" is specified */
	{
		parse_allflag = true;
		
		/* If one or more words were already matched, however... */

		if (mobjs > 0)
		{
			ParseError(6, 0);       /* "...doesn't make any sense..." */
			return false;
		}

		if (!domain)    /* no particular domain object specified */
			roomloc = parse_location;
		else
			roomloc = domain;

		AddAllObjects(roomloc);

		(*wordnum)++;


		/* Done processing the object phrase yet? */

		/* only >GET ALL EXCEPT... if we're not done the object phrase */
		if (*wordnum<=objfinish && strcmp(word[*wordnum], "~except"))
		{
			ParseError(6, 0);   /* "Doesn't make any sense..." */
			return false;
		}

		if ((*wordnum > words || word[*wordnum][0]=='\0')
			|| (obj_match_state != 1 && *wordnum >= objfinish))
		{
			if (!objcount && !speaking)
			{
				strcpy(parseerr, word[1]);
				ParseError(9, 0);   /* "Nothing to (verb)..." */
				return false;
			}
			return true;
		}

		/* Go back for the next piece of the phrase */
		pobjcount = 0;
		(*wordnum)--;
		goto NextLoop;
	}


	/* If we have a possible object or set of objects, go through the
	   disqualification process, either to sort out any confusion, or
	   even if there's only one possible object, to make sure it's
	   available
	*/
	if (pobjcount >= 1)
	{
		bestavail = 0;

		for (k=0; k<pobjcount; k++)	/* disqualify if unavailable */
		{
			i = pobjlist[k].obj;

			/* held or multiheld */
			if ((domain) && domain==var[actor])
			{
				if (Parent(i) != var[actor])
				{
					SubtractPossibleObject(i);
					k--;
				}
				else
					pobj = i;
			}

			else if ((Peek(grammaraddr)==NOTHELD_T || Peek(grammaraddr)==MULTINOTHELD_T) 
				&& Parent(i)==var[actor])
			{
				SubtractPossibleObject(i);
				k--;

				/* if this was the last suspect */
				if (pobjcount<1)  /* i.e., 0 */
				{
					ParseError(11, i);  /* "You don't see that..." */
					return false;
				}
			}

			/* otherwise */
			else if (Available(i, 0)==false) /* and obj_match_state!=5) */
			{
				SubtractPossibleObject(i);
				k--;
			}
			else
				pobj = i;

			/* Try to determine the best available object */
			if ((!bestavail) || (bestavail==(char)adjective && pobjlist[k].type==(char)noun))
			{
				m = domain;
				/* Temporary parent domain */
				domain = -1;
				if (Available(i, 0))
				{
					bestavail = pobjlist[k].type;
					bestobj = i;
				}
				domain = m;
			}

			/* Pick a default (poor) best match */
			if (!bestobj)
				bestobj = i;
		}


		/* Disqualify if an object is less exact--i.e., if the
		   word is a noun for one object but only an adjective for
		   another, disqualify the one for which it is an
		   adjective.
		*/
		if (pobjcount > 1)
		{
			for (i=0; i<pobjcount; i++)
			{
				if (pobjlist[i].type==(char)noun)
					found_noun = true;
			}

			if (found_noun)
			{
				for (i=0; i<pobjcount; i++)
				{
					/* Use ObjWord()'s test to see if this word
					   should get upgraded from an adjective to a noun
					*/
					if (pobjlist[i].type==(char)adjective)
					{
						if (ObjWordType(pobjlist[i].obj, wd[*wordnum-1], noun))
							pobjlist[i].type = (char)noun;
					}

					if (pobjlist[i].type==(char)adjective)
					{
						SubtractPossibleObject(pobjlist[i].obj);
						i--;
					}

					if (pobjcount==1) break;
				}
			}
		}


		/* If we're dealing with an 'anything' object, prefer an
		   object that's currently available to one that isn't
		*/
		if (pobjcount > 1 && Peek(grammaraddr)==ANYTHING_T)
		{
			for (i=0; i<pobjcount; i++)
			{
				if (Available(pobjlist[i].obj, true))
				{
					for (j=0; j<pobjcount; j++)
					{
						if (j!=i)
						{
							if (!Available(pobjlist[j].obj, true))
							{
								SubtractPossibleObject(pobjlist[j].obj);
								j--;	/* don't skip one */
								if (pobjcount<=1) break;
							}
						}
					}
				}
				if (pobjcount<=1) break;
			}
		}


		/* Use whatever disambiguation the FindObject routine might provide */
		if (pobjcount > 1 && findobjectaddr)
		{
			for (k=0; k<pobjcount; k++)
			{
				i = pobjlist[k].obj;
				ret = 0;
				passlocal[0] = i;
				passlocal[1] = 0;
				PassLocals(2);

				SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);
#if defined (DEBUGGER)
				DebugRunRoutine((long)findobjectaddr*address_scale);
#else
				RunRoutine((long)findobjectaddr*address_scale);
#endif
				retflag = 0;

				if (ret==0)     /* returned false */
				{
					SubtractPossibleObject(i);
					k--;	/* so we don't skip one */
					if (pobjcount<=1) break;
				}
				else
				{
					bestobj = i;
					pobj = i;
				}

				if (pobjcount==1) break;
			}
		}


		/* If this is a clarification-required call to MatchObject(),
		   return true if more than one possible object is found.
		*/
		if (pobjcount > 1 && recursive_call)
			return true;


		/* On the other hand, if we've managed to disqualify
		   everything:
		*/
		if (pobjcount==0 && !speaking)
		{
			if (obj_match_state==5)
			{
				if (!starts_with_verb)
					/* "Better start with a verb..." */
					ParseError(2, 0);
				else
					/* "That doesn't make any sense..." */
					ParseError(6, 0);
			}
			else
			{
				if ((!domain) || domain != var[actor])
				{
					if (Peek(grammaraddr)==ANYTHING_T)
						/* "...haven't seen..." */
						ParseError(10, bestobj);
					else if (domain)
						/* "...don't see that there..." */
						ParseError(14, bestobj);
					else
						/* "...don't see any..." */
						ParseError(11, bestobj);
				}
				else
					ParseError(15, bestobj);  /* "...don't have any..." */
			}
			return false;
		}
		else if (pobjcount==0)
			pobj = bestobj;


		/* If, after all this, there still exists some confusion
		   about what object is meant:
		*/
		if (pobjcount > 1)
		{
			int wtemp;
			unsigned int wdtemp[MAXWORDS+1];
			int tempobjlist[MAXWORDS+1], tempobjcount;
			struct pobject_structure temppobjlist[MAXPOBJECTS];
			int tempobjfinish;
			int temppobjcount;
			bool tempaddflag;

			char tempxverb = xverb;
			ParseError(8, 0);      /* "Which...do you mean..." */
			xverb = tempxverb;

			for (i=1; i<=words; i++)      /* save word arrays */
				wdtemp[i] = wd[i];
			wtemp = words;
			tempobjfinish = objfinish;

			for (i=0; i<=objcount; i++)   /* save object arrays */
				tempobjlist[i] = objlist[i];
			for (i=0; i<=pobjcount; i++)
				temppobjlist[i] = pobjlist[i];

			tempobjcount = objcount;
			temppobjcount = pobjcount;
			tempaddflag = addflag;


			/* Get a new input and properly parse it; after
			   all, it may be returned to MatchCommand() as
			   a potentially valid command.
			*/
			GetCommand();
			SeparateWords();
			if (words==0)
			{
				ParseError(0, 1);
				return false;
			}
			if (!Parse())
				return false;

			objfinish = words;

			/* Do we not care?  i.e. is "any", "either", etc. given? */
			if (!strcmp(word[1], "~any") && words==1)
			{
				for (k=0; k<pobjcount; k++)
				{
					i = pobjlist[k].obj;

					if (strcmp(Name(i), ""))
					{
						pobj = i;
						sprintf(line, "(%s)", Name(i));
						AP(line);
						goto RestoreTempArrays;
					}
				}
			}

			/* Check to see if any object is disqualified by
			   any one of the words.
			*/
			for (i=1; i<=words; i++)
			{
				if (word[i][0]!='~')
				{
					for (j=0; j<pobjcount; j++)
					{
						if (!ObjWord(pobjlist[j].obj, wd[i]))
						{
							SubtractPossibleObject(pobjlist[j].obj);
							j--;
						}
					}
				}
			}
			if (pobjcount==1)
			{
				pobj = pobjlist[0].obj;
				full_buffer = 0;
				goto RestoreTempArrays;
			}

			/* Check to see if the first word is a noun
			   or adjective for any of the possible suspect objects.
			*/
			flag = 0;
			for (i=0; i<pobjcount; i++)
			{
				if (ObjWord(pobjlist[i].obj, wd[1]))
				{
					flag = 1;
					break;
				}
			}
			if (!strcmp(word[1], "~all"))
				flag = 1;

			/* If not, tell MatchCommand() that there's a 
			   new command coming down the hopper.
			*/
			if (flag==0)
			{
				full_buffer = 1;
				return false;
			}

			/* Here, tell MatchObject()--this function--
			   that this isn't a virgin call.
			*/
			recursive_call = true;
			addflag = true;
			i = 1;
			j = MatchObject(&i);
			if (j==false)	/* parsing error */
			{
				full_buffer = 0;
				return false;
			}
			else if (j==-1)	/* multiple matches found */
			{
				/* See if a single entered word matches
				   exactly the name of only one object
				*/
				if (words==1)
				{
					flag = 0;
					for (j=0; j<temppobjcount; j++)
					{
						if (wd[1]==(unsigned int)GetProp(temppobjlist[j].obj, 0, 1, 0))
						{
							pobj = temppobjlist[j].obj;
							flag++;
						}
					}
					if (flag==1)
					{
						full_buffer = 0;
						goto RestoreTempArrays;
					}
				}

				/* Now, even if we weren't able to find a
				   match, check to see if the last word
				   belongs to only one of the possible
				   suspects, especially if it's a noun.
				*/
				flag = 0;
				bestobj = 0;
				i = words;
				while (wd[i]==0) i--;
				for (j=0; j<temppobjcount; j++)
				{
					if ((m = ObjWord(temppobjlist[j].obj, wd[i])) != 0)
					{
						if (!bestobj || m<=bestobj)
						{
							flag++;
							bestobj = m;
							pobj = temppobjlist[j].obj;
						}
					}
				}
				if (flag != 1)
				{
					/* "You'll have to be more specific..." */
					ParseError(13, 0);
					full_buffer = 0;
					return false;
				}
				full_buffer = 0;
			}

RestoreTempArrays:
		/* Rebuild <buffer> and word[] array */
			strcpy(buffer, "");
			for (i=1; i<=wtemp; i++)
			{
				strcat(buffer, GetWord(wdtemp[i]));
				strcat(buffer, " ");
			}
			SeparateWords();

		/* Restore wd[] array after SeparateWords() */
			for (i=1; i<=wtemp; i++)
				wd[i] = wdtemp[i];
			words = wtemp;
			objfinish = tempobjfinish;

		/* Restore object lists */
			for (i=0; i<temppobjcount; i++)
				pobjlist[i] = temppobjlist[i];
			pobjcount = temppobjcount;

			/* Multiple disambig. results for a non-multi verb? */
			i = Peek(grammaraddr);
			if (objcount>1 && i!=MULTI_T && i!=MULTIHELD_T && i!=MULTINOTHELD_T)
			{
				strcpy(parseerr, word[1]);
				/* "You can't...multiple objects." */
				ParseError(3, 0);
				return false;
			}

			/* Check objlist against original pobjlist */
			for (i=0; i<objcount; i++)
			{
				flag = 0;
				for (j=0; j<pobjcount; j++)
				{
					if (pobjlist[j].obj==objlist[i])
						flag = true;
				}
				if (!flag)
				{
					/* "You'll have to be more specific..." */
					ParseError(13, 0);
					full_buffer = 0;
					return false;
				}
			}

			/* Check pobjlist against disambiguation objlist */
			for (i=0; i<pobjcount; i++)
			{
				flag = 0;
				for (j=0; j<objcount; j++)
				{
					if (pobjlist[i].obj==objlist[j])
						flag = true;
				}
				if (!flag)
				{
					SubtractPossibleObject(pobjlist[i].obj);
					i--;
				}
			}

			for (i=0; i<=tempobjcount; i++)
				objlist[i] = tempobjlist[i];
			objcount = (char)tempobjcount;
			addflag = (char)tempaddflag;

			if (word[*wordnum][0]=='~') (*wordnum)--;
		}
	}

	/* Rule out "noun noun" and "noun adjective" combinations */

	i = 0;                        /* count nouns */
	k = 10;                       /* best (i.e., lowest) match */
	for (j=1; j<=mobjs; j++)
	{
		if ((m = ObjWord(pobj, mobj[j]))==noun)
		{
			k = noun;
			i++;
		}
		else if (k==noun) i = 2;
	}

	/* Can't use more than one noun for a given object */
	if (i > 1)
	{
		/* "...no such thing" */
		ParseError(5, 0);
		return false;
	}

	/* Check finally to make sure it's valid */
	ResetFindObject();
	if (!ValidObj(pobj))
		return false;

	/* Finally--now add it or subtract it from the list, as
	   appropriate:
	*/
	if (addflag==true)
	{
		if (pobjcount)
		{
			for (i=0; i<pobjcount; i++)
				AddObj(pobjlist[i].obj);
		}

		/* backup */
		else if (pobj != 0)
			AddObj(pobj);
	}
	else
		SubtractObj(pobj);

	/* If the object wasn't where it was specifically claimed to be,
	   applies to second pass through object phrase(s) after xobject
	   is found (since <obj_match_state> is 2):
	*/
	if (obj_match_state==2 && domain != 0 &&
		Parent(pobj) != domain && pobj != 0 && !speaking)
	{
		if (domain==var[player])
			ParseError(15, pobj);    /* "You don't have any..." */
		else
			ParseError(14, pobj);    /* "You don't see any...there..." */
		return false;
	}

	if (!strcmp(word[*wordnum], "~except"))
	{
		if (obj_match_state==1)
		{
			ParseError(7, 0);	/* can't have multiple xobjects */
			return false;
		}
		addflag = false;
	}

	if ((strcmp(word[*wordnum], "~and") && strcmp(word[*wordnum], "~except"))
		|| obj_match_state==5)
	{
		/* At the end yet? */
		if ((*wordnum > words || word[*wordnum][0]=='\0')
			|| *wordnum > objfinish)
		{
			if ((objcount > 0 && pobj != 0) || recursive_call || speaking)
				return true;
			else
			{
				/* No objects found */
				strcpy(parseerr, word[1]);
				ParseError(9, 0);   /* "Nothing to (verb)..." */
				return false;
			}
		}
	}

	/* Go back for the next object phrase */
	pobjcount = 0;
	mobjs = 0;
	strcpy(parseerr, "");

	goto NextLoop;
}

int Hugo::MatchWord(int *wordnum) {
	char num[18];
	int i, p, t, flag, finish;
	unsigned int thissyntax, nextsyntax;

	if (wd[*wordnum]==0)
		return 0;

	switch ((t = Peek(grammaraddr)))
	{
		/* the verb ("*") */
		case ASTERISK_T:
			(*wordnum)++;
			AdvanceGrammar();
			return 1;

		/* a non-specific dictionary word */
		case WORD_T:
			if (obj_match_state==1)
				var[xobject] = wd[*wordnum];
			else
			{
				var[object] = wd[*wordnum];
				obj_match_state = 1;
			}
			object_is_number = true;
			(*wordnum)++;
			AdvanceGrammar();
			return 1;

		/* a specific dictionary entry */
		case DICTENTRY_T:
CheckWord:
			/* matches word */
			if (wd[*wordnum]==PeekWord(grammaraddr + 1))
			{
				(*wordnum)++;
				AdvanceGrammar();
				while (Peek(grammaraddr)==9)
					grammaraddr += 4;
				return 1;
			}
			else
			{
				/* if next word is a "/" */
				if (Peek(grammaraddr + 3)==FORWARD_SLASH_T)
				{
					AdvanceGrammar();       /* this word */
					AdvanceGrammar();       /* "/" */
					goto CheckWord;
				}

				return 0;
			}

		/* alternative dictionary words */
		case FORWARD_SLASH_T:
			grammaraddr++;
			return 1;

		/* a number */
		case NUMBER_T:
			if ((STARTS_AS_NUMBER(word[*wordnum])) &&
				!strcmp(itoa(atoi(word[*wordnum]), num, 10), word[*wordnum]))
			{
				if (obj_match_state==1)
					var[xobject] = atoi(word[*wordnum]);
				else
				{
					var[object] = atoi(word[*wordnum]);
					obj_match_state = 1;
				}
				object_is_number = true;
				AdvanceGrammar();
				(*wordnum)++;
				return 1;
			}
			break;

		/* a string enclosed in quotes */
		case STRING_T:
			if (parsestr[0]=='\"')
			{
				AdvanceGrammar();
				(*wordnum)++;
				return 1;
			}
			else
				return 0;

		default:
		{
		
		/* Some manifestation of an object (or objects) before domain is
		   found, since <obj_match_state> is initially set to 0:
		*/
			if (obj_match_state==0)
			{
				if (Peek(grammaraddr)==HELD_T || Peek(grammaraddr)==MULTIHELD_T)
					odomain = var[actor];

				obj_match_state = 1;    /* since next set of object words
							   must be the xobject */
				objstart = *wordnum;
				objgrammar = grammaraddr;

				while (wd[*wordnum] != 0)
				{
					finish = *wordnum;

					/* Check what's coming up in case it's a dictionary
					   word--which would override an object phrase.
					*/
					thissyntax = grammaraddr;
					AdvanceGrammar();
					nextsyntax = grammaraddr;
					grammaraddr = thissyntax;

					/* dictionary word or string */
CheckWordorString:
					p = Peek(nextsyntax);
					if (p==DICTENTRY_T || p==STRING_T)
					{
						if ((PeekWord(nextsyntax + 1)==wd[*wordnum]) ||
							(p==STRING_T && wd[*wordnum]==UNKNOWN_WORD))
						{
							grammaraddr = nextsyntax;
							if (*wordnum != objstart)
								return 1;
							else
								return 0;
						}
						else if (Peek(nextsyntax + 3)==FORWARD_SLASH_T)
						{
							thissyntax = grammaraddr;
							grammaraddr = nextsyntax + 3;
							AdvanceGrammar();
							nextsyntax = grammaraddr;
							grammaraddr = thissyntax;
							
							goto CheckWordorString;
						}
					}

					/* or a number */
					else if ((p==NUMBER_T && STARTS_AS_NUMBER(word[*wordnum])) &&
						!strcmp(word[*wordnum], itoa(atoi(word[*wordnum]), num, 10)))
					{
						grammaraddr = nextsyntax;
						if (*wordnum != objstart)
							return 1;
						else
							return 0;
					}

					/* Pass over any object words--they'll be matched
					   specifically later in MatchCommand().
					*/
					flag = 0;
					if (AnyObjWord(*wordnum)==1)
					{
						(*wordnum)++;
						flag = 1;
					}

					/* if "~and", "~all",... */
					if (word[*wordnum][0]=='~')
					{
						int multicheck = Peek(grammaraddr);
						
						(*wordnum)++;
						flag = 1;

						/* multi or multi(something) */
						if (multicheck != MULTI_T &&
							multicheck != MULTIHELD_T &&
							multicheck != MULTINOTHELD_T)
						{
							strcpy(parseerr, word[1]);
							/* "You can't...multiple objects." */
							ParseError(3, 0);
							return 2;
						}
					}

					objfinish = finish;

					if (flag==0)
						return 0;
				}

				AdvanceGrammar();
				return 1;
			}

		/* hitting xobject */

			else if (obj_match_state==1)
			{
				int temp_objfinish = objfinish;

				/* If we don't know the verbroutine, try to figure it out */
				if (var[verbroutine]==0)
				{
					thissyntax = grammaraddr;
					AdvanceGrammar();
					nextsyntax = grammaraddr;
					grammaraddr = thissyntax;
					if (Peek(nextsyntax)==ROUTINE_T)
					{
						var[verbroutine] = PeekWord(nextsyntax+1);
					}
				}

				p = Peek(grammaraddr);
				
				/* If the xobject is specifically a parent of the
				   object(s) to be matched later:
				*/
				if (p==PARENT_T) domain = -1;

				/* Also deal with held xobjects */
				t = domain;
				if (p==HELD_T || p==MULTIHELD_T)
					domain = var[actor];

				/* Figure out where this xobject must end, as per grammar */
				objfinish = -1;
				if (*wordnum < words)
				{
					thissyntax = grammaraddr;
					AdvanceGrammar();
					nextsyntax = grammaraddr;
					grammaraddr = thissyntax;

CheckXobjectFinish:
					p = Peek(nextsyntax);
					for (i=*wordnum+1; i<=words; i++)
					{
						if (p==DICTENTRY_T || p==STRING_T)
						{
							if ((PeekWord(nextsyntax + 1)==wd[i]) ||
								(p==STRING_T && wd[i]==UNKNOWN_WORD))
							{
								objfinish = i-1;
								break;
							}
						}
						else if (Peek(nextsyntax + 3)==FORWARD_SLASH_T)
						{
							thissyntax = grammaraddr;
							grammaraddr = nextsyntax + 3;
							AdvanceGrammar();
							nextsyntax = grammaraddr;
							grammaraddr = thissyntax;
						
							goto CheckXobjectFinish;
						}
						else if ((p==NUMBER_T && STARTS_AS_NUMBER(word[*wordnum])) &&
							!strcmp(word[*wordnum], itoa(atoi(word[*wordnum]), num, 10)))
						{
							objfinish = i;
							break;
						}
						else
							break;
					}

				}

				if (objfinish==-1) objfinish = words;

				/* Regardless, try to match the xobject */
				recursive_call = false;
				if (MatchObject(&(*wordnum))==0)
				{
					objfinish = temp_objfinish;

					return 2;
				}
				else
				{
					objfinish = temp_objfinish;

					domain = 0;
					if (ValidObj(pobj)==false)
						return 2;
					domain = t;

					if (objcount==1)
						var[xobject] = objlist[0];
					else
						var[xobject] = pobj;
					if (domain==-1) domain = var[xobject]; /* parent */
					obj_match_state = 2;

					AdvanceGrammar();

					/* Can't have multiple xobjects */
					if (objcount > 1)
					{
						ParseError(7, 0);
						return 2;
					}
					objcount = 0;
					return 1;
				}
			}
		}
	}
	
	return 0;
}

int Hugo::ObjWordType(int obj, unsigned int w, int type) {
	int j, num;
	unsigned int pa;
	
	pa = PropAddr(obj, type, 0);
	if (pa)
	{
		defseg = proptable;
		num = Peek(pa + 1);
		
		if (num==PROP_ROUTINE)
		{
			if ((unsigned int)GetProp(obj, type, 1, false)==w)
			{
				defseg = gameseg;
				return true;
			}
		}
		else
		{
			for (j=1; j<=num; j++)
			{
				if (PeekWord(pa + j * 2)==w)
				{
					defseg = gameseg;
					return true;
				}
			}
		}
	}

	defseg = gameseg;

	return false;
}

int Hugo::ObjWord(int obj, unsigned int w) {
	if ((obj_parselist) && !(obj_parselist[obj/8]&1<<(obj%8)))
		return 0;

	if (ObjWordType(obj, w, adjective))
		return adjective;
	
	if (ObjWordType(obj, w, noun))
		return noun;
		
	return 0;
}

int Hugo::Parse() {
	char foundstring = 0;           /* allow one unknown word/phrase */
	int notfound_word = 0;
	int i, j, k, m;
	char num[33];
	char tempword[81];
	unsigned int period, comma;
	unsigned int synptr;

	period = FindWord(".");
	comma = FindWord(",");

	strcpy(parsestr, "");           /* for storing any unknown string */
	parsed_number = 0;              /*  "     "     "  parsed number  */

	for (i=1; i<=words; i++)        /* find dictionary addresses */
	{
		if (word[i][0]=='\"' && foundstring==0)
		{
			strcpy(parsestr, word[i]);
			foundstring = 1;
			wd[i] = UNKNOWN_WORD;
		}
		else
		{
			wd[i] = FindWord(word[i]);

			/* Numbers -32768 to 32767 are valid...*/
			if (!strcmp(word[i], itoa(atoi(word[i]), num, 10)))
			{
#if !defined (MATH_16BIT)
				if (atoi(word[i]) > 32767 || atoi(word[i]) < -32768)
					goto NotinDictionary;
#endif
				parsed_number = atoi(word[i]);
				if (parseerr[0]=='\0')
					strcpy(parseerr, word[i]);
			}

			/* Otherwise it must be a dictionary entry */
			else
			{
				/* If it's not in the dictionary */
				if (wd[i]==UNKNOWN_WORD)
				{
NotinDictionary:
					if (!notfound_word)
					{
						strcpy(parseerr, word[i]);
						strcpy(oops, word[i]);

						notfound_word = i;
					}
				}
			}
		}
	}

	/* Return here instead of immediately, so that we have a chance
	   to load the rest of the recognized words into the word array
	*/
	if (notfound_word)
	{
		i = notfound_word;

		/* "...can't use the word..." */
		ParseError(1, 0);
		strcpy(errbuf, "");
		for (i=1; i<=words; i++)
		{
			strcat(errbuf, word[i]);
			if (i != words) strcat(errbuf, " ");
		}

		return 0;
	}

	wd[words+1] = 0;
	oopscount = 0;


	/* Do synonyms, removals, compounds, punctuation */

	for (i=1; i<=words; i++)                /* Look through words... */
	{
		synptr = 2;

		for (j=1; j<=syncount; j++)  /* ...and alterations */
		{
			defseg = syntable;
			if (wd[i]==PeekWord(synptr + 1))
			{
				switch (Peek(synptr))
				{
					case 0:        /* synonym */
					{
						defseg = syntable;
						wd[i] = PeekWord(synptr + 3);
						m = strlen(GetWord(wd[i])) - strlen(word[i]);
						if (m)
						{
							if (m + (int)strlen(buffer) > 81)
								{strcpy(buffer, "");
								words = 0;
								ParseError(0, 0);
								return 0;}

							for (k=words; k>i; k--)
							{
								strcpy(tempword, word[k]);
								word[k] += m;
								strcpy(word[k], tempword);
							}
						}
						strcpy(word[i], GetWord(wd[i]));
						i--;
						break;
					}

					case 1:        /* removal */
					{
						KillWord(i);
						i--;
						break;
					}

					case 2:        /* compound */
					{
						if (wd[i+1]==PeekWord(synptr+3))
						{
							strcat(word[i], word[i+1]);
							wd[i] = FindWord(word[i]);
							KillWord(i+1);
						}
						break;
					}
				}
				goto NextSyn;
			}
NextSyn:
			synptr += 5;
		}

		if (wd[i]==comma)
		{
			if (strcmp(word[i+1], "~and"))
			{
				static char AND[5] = "~and";
				word[i] = AND;
				wd[i] = FindWord("~and");
			}
			else
				KillWord(i);
		}

		if (wd[i]==period)
		{
			wd[i] = 0;
			word[i] = EMPTY;
		}
	}

	defseg = gameseg;

	if (strcmp(word[1], "~oops")) strcpy(oops, "");

	if (words==0)
	{
		ParseError(0,0);        /* What? */
		return false;
	}
		
	return true;
}

void Hugo::ParseError(int e, int a) {
	int i, k, count;

	remaining = 0;
	xverb = true;

	if (e==5 && !strcmp(parseerr, "")) e = 6;

	if (parseerroraddr)
	{
		ret = 0;
		passlocal[0] = e;
		passlocal[1] = a;
		PassLocals(2);

		SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

#if defined (DEBUGGER)
		DebugRunRoutine((long)parseerroraddr*address_scale);
#else
		RunRoutine((long)parseerroraddr*address_scale);
#endif
		stack_depth = 0;
		retflag = 0;
		if (ret)
		{
			if (ret==2) reparse_everything = true;
			return;
		}
	}

	switch (e)
	{
		case 0:
			AP("What?");
			break;

		case 1:
			sprintf(line, "You can't use the word \"%s\".", parseerr);
			AP(line);
			break;

		case 2:
			AP("Better start with a verb.");
			break;

		case 3:
			sprintf(line, "You can't %s multiple objects.", parseerr);
			AP(line);
			break;

		case 4:
			AP("Can't do that.");
			break;

		case 5:
			sprintf(line, "You haven't seen any \"%s\", nor are you likely to in the near future even if such a thing exists.", parseerr);
			AP(line);
			break;

		case 6:
			AP("That doesn't make any sense.");
			break;

		case 7:
			AP("You can't use multiple objects like that.");
			break;

		case 8:
		{
			sprintf(line, "Which %s do you mean, ", !parse_called_twice?parseerr:"exactly");
			count = 1;
			for (k=0; k<pobjcount; k++)
			{
				i = pobjlist[k].obj;

				if (strcmp(Name(i), ""))
				{
					if (count==pobjcount)
					{
						if (count > 2) strcat(line, ",");
						strcat(line, " or ");
					}
					else
					{
						if (count != 1)
							strcat(line, ", ");
					}
					if (GetProp(i, article, 1, 0))
					{
						const char *w = GetWord(GetProp(i, article, 1, 0));
						/* Don't use "a" or "an" in listing */
						/*
						if (!strcmp(w, "a") || !strcmp(w, "an"))
							strcat(line, "the ");
						else
							sprintf(line+strlen(line), "%s ", w);
						*/
						/* We'll just use "the" */
						if (w) strcat(line, "the ");
					}
					strcat(line, Name(i));
					count++;
				}
			}
			strcat(line, "?");
			AP(line);
			break;
		}

		case 9:
			sprintf(line, "Nothing to %s.", parseerr);
			AP(line);
			break;

		case 10:
			AP("You haven't seen anything like that.");
			break;

		case 11:
			AP("You don't see that.");
			break;

		case 12:
			sprintf(line, "You can't do that with the %s.", Name(a));
			AP(line);
			break;

		case 13:
			AP("You'll have to be a little more specific.");
			break;

		case 14:
			AP("You don't see that there.");
			break;

		case 15:
			AP("You don't have that.");
			break;

		case 16:
			AP("You'll have to make a mistake first.");
			break;

		case 17:
			AP("You can only correct one word at a time.");
			break;
	}
}

void Hugo::RemoveWord(int a) {
	if (a > words)
		return;

	for (; a<words; a++)
	{
		wd[a] = wd[a + 1];
		objword_cache[a] = objword_cache[a + 1];
	}
	wd[words] = 0;
	objword_cache[words] = 0;
}

void Hugo::ResetFindObject() {
	if (findobjectaddr)
	{
		SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);
		PassLocals(0);
#if defined (DEBUGGER)
		DebugRunRoutine((long)findobjectaddr*address_scale);
#else
		RunRoutine((long)findobjectaddr*address_scale);
#endif
		retflag = 0;
	}
}

void Hugo::SeparateWords() {
	char inquote = 0;
	char a[1025];
	char b[2];
	char w1[17], w2[17];            /* for time conversions */
	char temp[17];
	short n1, n2;                   /* must be 16 bits */
	int bloc = 0;                   /* buffer location */
	int i;


	/* First filter the line of any user-specified punctuation */
	do
	{
		i = strcspn(buffer, punc_string);
		if (buffer[i]) buffer[i] = ' ';
	} while (buffer[i]);


	/* Begin the word-splitting proper: */
	
	words = 1;                      /* Setup a blank string */

	for (i=0; i<MAXWORDS+1; i++)
	{
		word[i] = EMPTY;
		wd[i] = 0;
	}
	word[1] = buffer;

	strcpy(a, buffer);
	strcpy(buffer, "");
	
	for (i=1; i<=(int)strlen(a); i++)
	{
		if (inquote!=1 && isascii(a[i-1]))
			b[0] = (char)tolower(a[i-1]);
		else b[0] = a[i-1];
		b[1] = '\0';

		if (b[0]=='\"' && inquote==1)
		{
			strcpy(buffer+bloc, b);
			bloc++;
			inquote++;
		}

		if (b[0]=='\"' || ((b[0]==' ' || b[0]=='!' || b[0]=='?') && inquote!=1))
		{
			if (word[words][0]!='\0')
			{
				bloc++;
				if (++words > MAXWORDS) words = MAXWORDS;
				word[words] = buffer + bloc;
				strcpy(word[words], "");
			}

			if (b[0]=='\"' && inquote==0)
			{
				strcpy(buffer+bloc, b);
				bloc++;
				inquote = 1;
			}
		}
		else
		{
			if ((b[0]=='.' || b[0]==',') && inquote!=1)
			{
				if (word[words][0]!='\0')
				{
					bloc++;
					if (++words > MAXWORDS) words = MAXWORDS;
				}
				word[words] = buffer + bloc;
				strcpy(word[words], b);
				bloc += strlen(b) + 1;
				if (++words > MAXWORDS) words = MAXWORDS;
				word[words] = buffer + bloc;
				strcpy(word[words], "");
			}
			else
			{
				strcpy(buffer+bloc, b);
				bloc++;
			}
		}
	}

	if (!strcmp(word[words], "")) words--;

	for (i=1; i<=words; i++)
	{
		/* Convert hours:minutes time to minutes only */
		if (strcspn(word[i], ":")!=strlen(word[i]) && strlen(word[i])<=5)
		{
			strcpy(w1, Left(word[i], strcspn(word[i], ":")));
			strcpy(w2, Right(word[i], strlen(word[i]) - strcspn(word[i], ":") - 1));
			n1 = (short)atoi(w1);
			n2 = (short)atoi(w2);

			if (!strcmp(Left(w2, 1), "0"))
				strcpy(w2, Right(w2, strlen(w2) - 1));

			/* If this is indeed a hh:mm time, write it back
			   as the modified word, storing the original hh:mm
			   in parse$:
			*/
			if (!strcmp(w1, itoa((int)n1, temp, 10)) && !strcmp(w2, itoa((int)n2, temp, 10)) && (n1 > 0 && n1 < 25) && (n2 >= 0 && n2 < 60))
			{
				strcpy(parseerr, word[i]);
				itoa(n1 * 60 + n2, word[i], 10);
			}
		}
	}
}

void Hugo::SubtractObj(int obj) {
	int i, j;

	for (i=0; i<objcount; i++)
	{
		if (objlist[i]==obj)
		{
			for (j=i; j<objcount; j++)
				objlist[j] = objlist[j+1];
			objcount--;
			return;
		}
	}
}

void Hugo::SubtractPossibleObject(int obj) {
	int i, j, last = 0;

	for (i=0; i<pobjcount; i++)
	{
		if (pobjlist[i].obj==obj)
		{
			if (pobjlist[i].obj==pobj && last!=0) pobj = last;

			for (j=i; j+1<pobjcount; j++)
			{
				pobjlist[j] = pobjlist[j+1];
			}
			pobjcount--;

#ifdef DEBUG_PARSER
{
	char buf[100];
	sprintf(buf, "SubtractPossibleObject(%d:\"%s\")", obj, Name(obj));
	Printout(buf);
}
#endif
			return;
		}
		else last = pobjlist[i].obj;
	}
}

void Hugo::TryObj(int obj) {
	unsigned int tempdomain;

	if ((obj_parselist) && !(obj_parselist[obj/8]&1<<(obj%8)))
		return;

	if (DomainObj(obj))
	{
		tempdomain = domain;
		domain = 0;

		if (Available(obj, 0) && !InList(Parent(obj)))
		{
			AddObj(obj);
		}
		else
		{
			SubtractObj(obj);
		}

		domain = tempdomain;
	}
}

int Hugo::ValidObj(int obj) {
	int attr, nattr = 0;
	unsigned int addr;

	defseg = gameseg;

	if (!Available(obj, 0) && !speaking &&
		(Peek(grammaraddr)!=OPEN_BRACKET_T ||
		Peek(grammaraddr+1)!=ROUTINE_T))
	{
		if (Peek(grammaraddr)==ANYTHING_T)
			ParseError(10, obj);    /* "...haven't seen..." */
		else
			ParseError(11, obj);    /* "...don't see any..." */
		return 0;
	}

	switch (Peek(grammaraddr))
	{
		case OPEN_BRACKET_T:
		{
			if (Peek(grammaraddr+1)==ROUTINE_T)
			{
				addr = PeekWord(grammaraddr+2);
				ret = 0;
				passlocal[0] = obj;
				PassLocals(1);

				SetStackFrame(RESET_STACK_DEPTH, RUNROUTINE_BLOCK, 0, 0);

#if defined (DEBUGGER)
				DebugRunRoutine((long)addr*address_scale);
#else
				RunRoutine((long)addr*address_scale);
#endif
				retflag = 0;

				/* If the routine doesn't return true,
				   the object doesn't qualify.
				*/
				if (!ret)
					return (0);
			}
			else if (Peek(grammaraddr+1)==OBJECTNUM_T)
			{
				if (obj != (int)PeekWord(grammaraddr+2))
				{
					strcpy(parseerr, "");
					if (GetProp(obj, article, 1, 0))
						strcpy(parseerr, "the ");
					strcat(parseerr, Name(obj));

					/* "...can't do that with..." */
					ParseError(12, obj);
					return 0;
				}
			}
			break;
		}

		case ATTR_T:
		case NOT_T:
		{
			if (Peek(grammaraddr)==NOT_T) nattr = 1;
			attr = Peek(grammaraddr + 1 + nattr);

			/* If the attribute match is not made,
			   the object doesn't qualify.
			*/
			if (!TestAttribute(obj, attr, nattr))
			{
				strcpy(parseerr, "");
				if (GetProp(obj, article, 1, 0))
					strcpy(parseerr, "the ");
				strcat(parseerr, Name(obj));

				/* "...can't do that with..." */
				ParseError(12, obj);
				return 0;
			}
			break;
		}
	}
	return true;
}

} // End of namespace Hugo
} // End of namespace Glk
