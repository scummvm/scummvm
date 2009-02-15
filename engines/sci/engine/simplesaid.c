/***************************************************************************
 simplesaid.c Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/


#ifdef SCI_SIMPLE_SAID_CODE

#include <engine.h>
#include <kdebug.h>

static int current_pword;
static int said_pos;
static int refstart_pos, parse_pos;
static parse_tree_node_t *words;
static int dontclaim; /* Whether the Parse() event should be claimed */
static int reached_end;

static state_t *state;
static heap_ptr addr;

#define WORDP_MODE_STAY 0
#define WORDP_MODE_NEXT 1
#define WORDP_MODE_SEEK 2

/* hack */
#define s state

static inline int /* Returns whether the specified word matches */
word_p(int mode, int word)
{
  int oldpp = parse_pos;
  do {
    if (mode)
      ++parse_pos;

    current_pword = words[parse_pos << 1].content.value;
    /*    SDEBUG("Word at %d = %03x\n", parse_pos, current_pword); */
    if (current_pword == word) {
      /*      ++matches; */
      SCIkdebug(SCIkSAID, "wordp(%d, %03x) from %d -> 1\n", mode, word, oldpp);

      reached_end |= (words[(parse_pos + 1) << 1].type == -1);

      return 1;
    }

  } while ((mode == WORDP_MODE_SEEK) && (words[parse_pos <<1].type != -1));

  if (words[parse_pos << 1].type == -1)
    --parse_pos;

  SCIkdebug(SCIkSAID, "wordp(%d, %03x) from %d -> 0\n", mode, word, oldpp);

  return 0;
}



static inline int /* End of input? */
end_p()
{
  int val = (words[(parse_pos + 1) << 1 ].type == -1) || reached_end;

  SCIkdebug(SCIkSAID, "endp(pp=%d) = %d\n", parse_pos, val);

  return val;
}



static inline int /* Returns whether the current_word references that specified word */
reference_p(int referenced_word_index, int word)
{
  int val = 0;
  int seeker = (refstart_pos << 1) + 2;

  while (words[seeker].type != -1) {

    if (words[seeker].content.value == word)
      if (((words[seeker + 1].content.branches[0] > -1)
	   && (words[seeker + 1].content.branches[0] == referenced_word_index))
	  || ((words[seeker + 1].content.branches[1] > -1)
	      && (words[seeker + 1].content.branches[1] == referenced_word_index))) {
	val = 1;
	reached_end |= (words[seeker + 2].type == -1);
	break;
      }

    seeker += 2;
  }

  SCIkdebug(SCIkSAID, "%03x > pos[%d]  = %d  (start at %d)\n", word, referenced_word_index, val, refstart_pos);

  return val;
}


static inline int /* Checks whether the current word has trailing references */
follows_p(void)
{
  /*  int val = (words[(parse_pos << 1) + 1].content.branches[1] > (parse_pos << 1));

      SDEBUG("follows-p(pp=%d) = %d\n", parse_pos, val);*/

  dontclaim = 1;
  return 1; /* appears to work best */
}


static inline int
next_parse_token(int *token_is_op)
{
  int item = state->heap[addr++];

  if (item < 0xf0) {
    item = item << 8 | state->heap[addr++];
    *token_is_op = 0;
  } else
    *token_is_op = 1;

  return item;
}

#define STATE_INITIAL 0
#define STATE_OR 1
#define STATE_SEEK 2
#define STATE_REF 3

static int
simplesaid(int minor_state, int terminator)
{
  int major_state = STATE_INITIAL;
  int refword = parse_pos; /* The word references apply to */
  int aspiring_refword = -1; /* in "a < b < c", c refers to b, and aspiring_refword will be b. */
  int truth = 1;
  int token_is_op;
  SCIkdebug(SCIkSAID, "simplesaid(%02x)\n", terminator);

  while (42) {

    int token = next_parse_token(&token_is_op);
    SCIkdebug(SCIkSAID, "Got token %03x on truth %d\n", token, truth);

    if (token_is_op && (token == terminator)) {
      if (terminator == SAID_TERM)
	truth = truth && end_p();
      SCIkdebug(SCIkSAID, "Terminator; returning %d\n", truth);
      return truth;
    }

    if (token_is_op) /* operator? */
      switch (token) {

      case SAID_COMMA:
	minor_state = STATE_OR;
	break;

      case SAID_SLASH:
	if (!truth) {
	  while (((token = next_parse_token(&token_is_op)) != terminator)
		 && (token != SAID_TERM)); /* Proceed to end of block */
	  if (token != terminator)
	    SCIkwarn(SCIkERROR, "Syntax error: Unexpected end of spec");
	  return 0;
	}

	major_state = STATE_SEEK;
	minor_state = STATE_INITIAL;
	break;

      case SAID_PARENC:
	SCIkwarn(SCIkERROR, "')' without matching '('\n");
        return 0;

      case SAID_PARENO:
	switch (minor_state) {

	case STATE_OR:
	  truth |= simplesaid(minor_state, SAID_PARENC);
	  break;

	case STATE_INITIAL:
	  truth = truth && simplesaid(minor_state, SAID_PARENC);
	  break;

	default:
	  SCIkwarn(SCIkERROR, "'(' in minor state %d\n", minor_state);
	}
	break;

      case SAID_BRACKC:
	SCIkwarn(SCIkERROR, "']' without matching '['\n");
        return 0;

      case SAID_BRACKO: {
	int parse_pos_old = parse_pos;
	int recurse = simplesaid(minor_state, SAID_BRACKC);
	if (!recurse)
	  parse_pos = parse_pos_old;
	break;
      }

      case SAID_LT:
	if (aspiring_refword > -1) /* "a < b < c" */
	  refword = aspiring_refword; /* refword = 'b' (in the case above) */
	major_state = STATE_REF;
	break;

      case SAID_GT:
	return truth && follows_p();

      case SAID_TERM:
	SCIkwarn(SCIkERROR, "Unexpected end of spec\n");
        return 0;

      default:
	SCIkwarn(SCIkERROR, "Syntax error: Unexpected token 0x%02x\n", token);
        return 0;
      } else {
	int tempresult;

	/*      ++word_tokens_nr; /* Normal word token */

	switch(major_state) {

	case STATE_INITIAL:
	  tempresult = word_p(((minor_state == STATE_OR)? WORDP_MODE_STAY : WORDP_MODE_NEXT), token);
	  refword = parse_pos;
	  break;

	case STATE_SEEK:
	  tempresult = word_p(WORDP_MODE_SEEK, token);
	  major_state = STATE_INITIAL;
	  refword = parse_pos;
	  break;

	case STATE_REF:
	  tempresult = reference_p(refword, token);
	  aspiring_refword = parse_pos;
	  break;

	default:
	  SCIkwarn(SCIkERROR, "Invalid major state!\n");
          return 0;
	}

	SCIkdebug(SCIkSAID, "state=(%d,%d), truth = %d * %d\n", major_state, minor_state, truth, tempresult);

	if (minor_state == STATE_OR)
	  truth |= tempresult;
	else
	  truth = truth && tempresult;

	minor_state = STATE_INITIAL;
      }
  }
}

#undef s

int
vocab_simple_said_test(state_t *s, heap_ptr address)
{
  int matched;
  current_pword = reached_end = 0;
  dontclaim = 0;
  said_pos = 0;
  if (s->parser_lastmatch_word == SAID_NO_MATCH)
    SCIkdebug(SCIkSAID, "lastmatch_word: none\n");
  else
    SCIkdebug(SCIkSAID, "lastmatch_word=%d\n", s->parser_lastmatch_word);
  parse_pos = (s->parser_lastmatch_word == SAID_NO_MATCH)? -1 : s->parser_lastmatch_word;
  refstart_pos = parse_pos;
  state = s;
  addr = address;
  words = s->parser_nodes;
  matched = simplesaid(STATE_INITIAL, SAID_TERM);
  SCIkdebug(SCIkSAID, "Result: (matched,dontclaim)=(%d,%d)\n", matched, dontclaim);

  if (!matched)
    return SAID_NO_MATCH;

  if (dontclaim) /* partial match */
    return parse_pos; /* Return lastmatch word */

  return SAID_FULL_MATCH;
}

#endif /* SCI_SIMPLE_SAID_CODE */
