/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "m4/adv_r/chunk_ops.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/vars.h"

namespace M4 {

long conv_ops_text_strlen(char *s) {
	long len = 0;
	len = strlen(s) + 1;	// Added +1 for null char.

	if ((len % 4) == 0)
		return len;

	len += 4 - (len % 4);
	return len;
}

void conv_ops_unknown_chunk(long tag, const char *s) {
	char *tag_name = nullptr;

	tag_name = (char *)&tag;

	error_show(FL, 'PARS', "'%s' What type is this chunk: %c%c%c%c ?", s, tag_name[3], tag_name[2], tag_name[1], tag_name[0]);
}

/*
 * Find an entry and auto-advance the pointer past it.
 */
char *conv_ops_get_entry(long i, long *next, long *tag, Conv *c) {
	long num_blocks = 0;
	long j = 0, k = 0;
	lnode_chunk *L;
	node_chunk *N;
	text_chunk *T;
	w_reply_chunk *W;
	conv_chunk *CC;
	char *outChunk = nullptr;

	if ((i + c->myCNode) > c->chunkSize) {
		error_show(FL, 'PARS', "Conv pointer skipped past chunk EOF. Please check script and make sure HAG is up to date");
	}

	outChunk = &(c->conv[c->myCNode]);
	*tag = *(long *)&outChunk[i];

	if (_GC(swap))
		*tag = convert_intel32(*tag);

	switch (*tag) {
	case C_ASGN_CHUNK:
		//fprintf( conv_fp, "C_ASGN_CHUNK\n" );
		k = sizeof(c_assign_chunk);
		break;

	case ASGN_CHUNK:
		//fprintf( conv_fp, "ASGN_CHUNK\n" );
		k = sizeof(assign_chunk);
		break;

	case CONV_CHUNK:
		//fprintf( conv_fp, "CONV_CHUNK\n" );
		CC = (conv_chunk *)&outChunk[i];

		j = CC->size; //oct11 was +=
		break;

	case DECL_CHUNK:
		//fprintf( conv_fp, "DECL_CHUNK\n" );
		k = sizeof(decl_chunk);
		break;

	case FALL_CHUNK:
		//fprintf( conv_fp, "FALL_CHUNK\n" );
		k = sizeof(fall_chunk);
		break;

	case LNODE_CHUNK:
		//fprintf( conv_fp, "LNODE_CHUNK\n" );
		L = (lnode_chunk *)&outChunk[i];

		//oct11
		k = sizeof(lnode_chunk);

		if (_GC(swap)) {
			j = convert_intel32(L->num_entries) * sizeof(long); //was +=
			j = convert_intel32(j); //oct11
		} else {
			j = L->num_entries * sizeof(long); //was +=
		}
		break;

	case NODE_CHUNK:
		//fprintf( conv_fp, "NODE_CHUNK\n" );
		N = (node_chunk *)&outChunk[i];

		//oct11
		k = sizeof(node_chunk);

		if (_GC(swap)) {
			j = convert_intel32(N->num_entries) * sizeof(long); //was +=
			j = convert_intel32(j);
		} else {
			j = N->num_entries * sizeof(long); //was +=
		}
		break;

	case ENTRY_CHUNK:
		//fprintf( conv_fp, "ENTRY_CHUNK\n" );
		k = sizeof(entry_chunk);
		break;

	case TEXT_CHUNK:
	case MESSAGE_CHUNK:
		//if( *tag == TEXT_CHUNK )
		//	  fprintf( conv_fp, "MESSAGE_CHUNK\n" );
		//if( *tag == TEXT_CHUNK )
		//	  fprintf( conv_fp, "TEXT_CHUNK\n" );

		T = (text_chunk *)&outChunk[i];
		num_blocks = T->size;

		k = sizeof(text_chunk);
		j = num_blocks;

		//fprintf( conv_fp, "%s\n", outChunk[i+j] );
		break;

	case REPLY_CHUNK:
		//fprintf( conv_fp, "REPLY_CHUNK\n" );
		k = sizeof(reply_chunk);
		break;

	case WEIGHT_REPLY_CHUNK:
		//fprintf( conv_fp, "WREPLY_CHUNK\n" );
		W = (w_reply_chunk *)&outChunk[i];
		k = sizeof(w_reply_chunk);

		if (_GC(swap)) {
			j = convert_intel32(W->num_replies) * (2 * sizeof(long)); //was +=
			j = convert_intel32(j);
		} else {
			j = W->num_replies * (2 * sizeof(long)); //was +=
		}
		break;

	case WEIGHT_PREPLY_CHUNK:
		//fprintf( conv_fp, "WREPLY_CHUNK\n" );
		W = (w_reply_chunk *)&outChunk[i];
		k = sizeof(w_reply_chunk);

		if (_GC(swap)) {
			j = convert_intel32(W->num_replies) * (2 * sizeof(long)); //was +=
			j = convert_intel32(j);
		} else {
			j = W->num_replies * (2 * sizeof(long)); //was +=
		}
		break;

	case COND_REPLY_CHUNK:
		//fprintf( conv_fp, "COND_REPLY_CHUNK\n" );
		k = sizeof(c_reply_chunk);
		break;

	case COND_EXIT_GOTO_CHUNK:
	case COND_GOTO_CHUNK:
		//if( *tag == COND_EXIT_GOTO_CHUNK )
		//	  fprintf( conv_fp, "COND_EXIT_GOTO_CHUNK\n" );
		//if( *tag == COND_GOTO_CHUNK )
		//	  fprintf( conv_fp, "COND_GOTO_CHUNK\n" );
		k = sizeof(c_goto_chunk);
		break;

	case GOTO_CHUNK:
	case EXIT_GOTO_CHUNK:
		//if( *tag == EXIT_GOTO_CHUNK )
		//	  fprintf( conv_fp, "EXIT_GOTO_CHUNK\n" );
		//if( *tag == GOTO_CHUNK )
		//	  fprintf( conv_fp, "GOTO_CHUNK\n" );

		k = sizeof(goto_chunk);
		break;

	case HIDE_CHUNK:
	case UHID_CHUNK:
	case DSTR_CHUNK:
		//fprintf( conv_fp, "MISC_CHUNK\n" );
		k = sizeof(misc_chunk);
		break;

	case CHDE_CHUNK:
	case CUHD_CHUNK:
	case CDST_CHUNK:
		//fprintf( conv_fp, "CMISC_CHUNK\n" );
		k = sizeof(c_misc_chunk);
		break;

	default:
#ifdef TEST_CONV
		printf("tag %d node %d %x\n", *tag, c->myCNode, c->myCNode);
#else
		//oct11
		error_show(FL, 'PARS', "Tag: %d (%x) Node: %d (%x hex)", *tag, *tag, c->myCNode, c->myCNode);
#endif

		conv_ops_unknown_chunk(*tag, "conv_ops_get_entry");
		//exit( 1 ); //fix me!!!! //oct10
		break;
	}

	//oct11
	if (_GC(swap))
		j = convert_intel32(j);
	j += k;

	j += i;
	*next = j;
	//fprintf( conv_fp, "\n\n" );
	return &outChunk[i];
}

static void swap_assign(assign_chunk *a) {
	a->tag = convert_intel32(a->tag);
	a->index = convert_intel32(a->index);
	a->op = convert_intel32(a->op);
	a->opnd1 = convert_intel32(a->opnd1);
}

assign_chunk *get_asgn(Conv *c, long cSize) {
	char *s = nullptr;
	assign_chunk *a = nullptr;

	s = &(c->conv[c->myCNode]);
	a = (assign_chunk *)&s[cSize];

	//swap_assign( a );
	return a;
}

static void swap_c_asgn(c_assign_chunk *c) {
	c->tag = convert_intel32(c->tag);

	c->c_op_l = convert_intel32(c->c_op_l);
	c->c_op = convert_intel32(c->c_op);
	c->c_op_r = convert_intel32(c->c_op_r);

	c->index = convert_intel32(c->index);
	c->op = convert_intel32(c->op);
	c->opnd1 = convert_intel32(c->opnd1);
}

c_assign_chunk *get_c_asgn(Conv *c, long cSize) {
	char *s = nullptr;
	c_assign_chunk *c_a = nullptr;

	s = &(c->conv[c->myCNode]);
	c_a = (c_assign_chunk *)&s[cSize];

	//swap_c_asgn( c_a );
	return c_a;
}

static void swap_conv(conv_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->size = convert_intel32(c->size);
}

conv_chunk *get_conv(Conv *c, long cSize) {
	char *s = nullptr;
	conv_chunk *c_v = nullptr;

	s = &(c->conv[c->myCNode]);
	c_v = (conv_chunk *)&s[cSize];

	//swap_conv( c_v );
	return c_v;
}

static void swap_decl(decl_chunk *d) {
	d->tag = convert_intel32(d->tag);
	d->val = convert_intel32(d->val);
	d->flags = convert_intel32(d->flags);
	//long = convert_intel32( *addr );
}

decl_chunk *get_decl(Conv *c, long cSize) {
	char *s = nullptr;
	decl_chunk *d = nullptr;

	s = c->conv;
	d = (decl_chunk *)&s[cSize];

	//swap_decl( d );
	return d;
}

static void swap_fall(fall_chunk *l) {
	l->tag = convert_intel32(l->tag);
	l->val = convert_intel32(l->val);
	l->index = convert_intel32(l->index);
}

fall_chunk *get_fall(Conv *c, long cSize) {
	char *s = nullptr;
	fall_chunk *f = nullptr;

	s = &(c->conv[c->myCNode]);
	f = (fall_chunk *)&s[cSize];

	//swap_fall( f );
	return f;
}

static void swap_lnode(lnode_chunk *l) {
	int i = 0;
	long *L = nullptr;

	l->tag = convert_intel32(l->tag);
	l->hash = convert_intel32(l->hash);
	l->size = convert_intel32(l->size);
	l->entry_num = convert_intel32(l->entry_num);
	l->num_entries = convert_intel32(l->num_entries);

	//oct11
	//
	L = (long *)l; //was &l
	L += 5; // *sizeof( long );
	for (i = 0; i < l->num_entries; i++) {
		L[i] = convert_intel32(L[i]);
	}
	//
}

lnode_chunk *get_lnode(Conv *c, long cSize) {
	char *s = nullptr;
	lnode_chunk *l = nullptr;

	s = &(c->conv[c->myCNode]);
	l = (lnode_chunk *)&s[cSize];

	//swap_lnode( l );
	return l;
}

static void swap_node(node_chunk *n) {
	int i = 0, j = 0;
	long *L = nullptr;

	n->tag = convert_intel32(n->tag);
	n->hash = convert_intel32(n->hash);
	n->size = convert_intel32(n->size);
	n->num_entries = convert_intel32(n->num_entries);

	//oct11
	//
	j = sizeof(long);
	L = (long *)n; //was &n
	L += 4; // *sizeof( long ); //was sizeof node_chunk
	for (i = 0; i < n->num_entries; i++) {
		L[i] = convert_intel32(L[i]);
	}
	//
}

node_chunk *get_node(Conv *c, long cSize) {
	char *s = nullptr;
	node_chunk *n = nullptr;

	s = &(c->conv[c->myCNode]);
	n = (node_chunk *)&s[cSize];

	//swap_node( n );
	return n;
}

static void swap_entry(entry_chunk *e) {
	e->tag = convert_intel32(e->tag);
	e->size = convert_intel32(e->size);
	e->status = convert_intel32(e->status);
}

entry_chunk *get_entry(Conv *c, long cSize) {
	char *s = nullptr;
	entry_chunk *e = nullptr;

	s = &(c->conv[c->myCNode]);
	e = (entry_chunk *)&s[cSize];

	//swap_entry( e );
	return e;
}

#if 0
static void swap_hash_entry(entry_chunk *e) {
	e->tag = convert_intel32(e->tag);
	e->size = convert_intel32(e->size);
	e->status = convert_intel32(e->status);
}
#endif

entry_chunk *get_hash_entry(Conv *c, long cSize) {
	char *s = nullptr;
	entry_chunk *e = nullptr;

	s = &(c->conv[0]);
	e = (entry_chunk *)&s[cSize];

	//swap_hash_entry( e );
	return e;
}

static void swap_text(text_chunk *t) {
	t->tag = convert_intel32(t->tag);
	t->size = convert_intel32(t->size);
}

text_chunk *get_text(Conv *c, long cSize) {
	char *s = nullptr;
	text_chunk *t = nullptr;

	s = &(c->conv[c->myCNode]);
	t = (text_chunk *)&s[cSize];

	//swap_text( t );
	return t;
}

static void swap_mesg(mesg_chunk *m) {
	m->tag = convert_intel32(m->tag);
	m->size = convert_intel32(m->size);
}

mesg_chunk *get_mesg(Conv *c, long cSize) {
	char *s = nullptr;
	mesg_chunk *m = nullptr;

	s = &(c->conv[c->myCNode]);
	m = (mesg_chunk *)&s[cSize];

	//swap_mesg( m );
	return m;
}

static void swap_reply(reply_chunk *r) {
	r->tag = convert_intel32(r->tag);
	r->index = convert_intel32(r->index);
}

reply_chunk *get_reply(Conv *c, long cSize) {
	char *s = nullptr;
	reply_chunk *r = nullptr;

	s = &(c->conv[c->myCNode]);
	r = (reply_chunk *)&s[cSize];

	//swap_reply( r );
	return r;
}

static void swap_c_reply(c_reply_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->op_l = convert_intel32(c->op_l);
	c->op = convert_intel32(c->op);
	c->op_r = convert_intel32(c->op_r);
	c->index = convert_intel32(c->index);
}

c_reply_chunk *get_c_reply(Conv *c, long cSize) {
	char *s = nullptr;
	c_reply_chunk *c_r = nullptr;

	s = &(c->conv[c->myCNode]);
	c_r = (c_reply_chunk *)&s[cSize];

	//swap_c_reply( c_r );
	return c_r;
}

static void swap_w_reply(w_reply_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->num_replies = convert_intel32(c->num_replies);
}

w_reply_chunk *get_w_reply(Conv *c, long cSize) {
	char *s = nullptr;
	w_reply_chunk *w = nullptr;

	s = &(c->conv[c->myCNode]);
	w = (w_reply_chunk *)&s[cSize];

	//swap_w_reply( w );
	return w;
}

static void swap_w_entry(w_entry_chunk *w) {
	w->weight = convert_intel32(w->weight);
	w->index = convert_intel32(w->index);
}

w_entry_chunk *get_w_entry(Conv *c, long cSize) {
	char *s = nullptr;
	w_entry_chunk *w = nullptr;

	s = &(c->conv[c->myCNode]);
	w = (w_entry_chunk *)&s[cSize];

	//swap_w_entry( w );
	return w;
}

static void swap_goto(goto_chunk *g) {
	g->tag = convert_intel32(g->tag);
	g->index = convert_intel32(g->index);
}

goto_chunk *get_goto(Conv *c, long cSize) {
	char *s = nullptr;
	goto_chunk *g = nullptr;

	s = &(c->conv[c->myCNode]);
	g = (goto_chunk *)&s[cSize];

	//swap_goto( g );
	return g;
}

static void swap_c_goto(c_goto_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->opnd1 = convert_intel32(c->opnd1);
	c->op = convert_intel32(c->op);
	c->opnd2 = convert_intel32(c->opnd2);
	c->index = convert_intel32(c->index);
}

c_goto_chunk *get_c_goto(Conv *c, long cSize) {
	char *s = nullptr;
	c_goto_chunk *cg = nullptr;

	s = &(c->conv[c->myCNode]);
	cg = (c_goto_chunk *)&s[cSize];

	//swap_c_goto( cg );
	return cg;
}

static void swap_misc(misc_chunk *m) {
	m->tag = convert_intel32(m->tag);
	m->index = convert_intel32(m->index);
}

misc_chunk *get_misc(Conv *c, long cSize) {
	char *s = nullptr;
	misc_chunk *m = nullptr;

	s = &(c->conv[c->myCNode]);
	m = (misc_chunk *)&s[cSize];

	//swap_misc( m );
	return m;
}

static void swap_c_misc(c_misc_chunk *c) {
	c->tag = convert_intel32(c->tag);

	c->c_op_l = convert_intel32(c->c_op_l);
	c->c_op = convert_intel32(c->c_op);
	c->c_op_r = convert_intel32(c->c_op_r);

	c->index = convert_intel32(c->index);
}

c_misc_chunk *get_c_misc(Conv *c, long cSize) {
	char *s = nullptr;
	c_misc_chunk *cm = nullptr;

	s = &(c->conv[c->myCNode]);
	cm = (c_misc_chunk *)&s[cSize];

	//swap_c_misc( cm );
	return cm;
}

#if 0
static void swap_long(long *l) {
	*l = convert_intel32(*l);
}
#endif

long get_long(Conv *c, long cSize) {
	char *s = nullptr;
	long *l = nullptr;

	s = &(c->conv[c->myCNode]);
	l = (long *)&s[cSize];

	//swap_long( l );
	return *l;
}

char *get_string(Conv *c, long cSize) {
	char *s = nullptr;
	char *c_s = nullptr;

	s = c->conv;
	c_s = (char *)&s[cSize];
	return c_s;
}

int conv_ops_cond_successful(long l_op, long op, long r_op) {
	switch (op) {
	case PERCENT:
		return l_op % r_op;

	case GE:
		return l_op >= r_op;

	case LE:
		return l_op <= r_op;

	case GT:
		return l_op > r_op;

	case LT:
		return l_op < r_op;

	case NE:
	case CNE:
		return !(l_op == r_op);

	case IS_ASSIGNED:
		return l_op == r_op;

	case ANDAND:
		return l_op && r_op;

	case OROR:
		return l_op || r_op;
	}
	return 0;
}

long conv_ops_process_asgn(long val, long oprtr, long opnd) {
	switch (oprtr) {
	case PPLUS:
		val += opnd;
		break;

	case MINUS:
		val -= opnd;
		break;

	case TIMES:
		val *= opnd;
		break;

	case DIVIDE:
		val /= opnd;
		break;

	case IS_ASSIGNED:
		val = opnd;
		break;

	default:
#ifdef TEST_CONV
		printf("uh-oh\n");
#else
		error_show(FL, 'PARS', "Operator must be a +,-,*,/,=, Please check tag type: %d in token header file", oprtr);
#endif
		break;
	}
	return val;
}

void conv_swap_words(Conv *c) {
	long ent = 0, tag = 0, next;
	long ent_old = 0;
	long tempEnt = 0;
	int x = 0;

	conv_chunk *conv = nullptr;
	decl_chunk *decl = nullptr;
	node_chunk *node = nullptr;
	fall_chunk *fall = nullptr;
	lnode_chunk *lnode = nullptr;
	entry_chunk *entry = nullptr;
	//entry_chunk *hash_entry = nullptr;

	text_chunk *text = nullptr;
	mesg_chunk *mesg = nullptr;
	reply_chunk *reply = nullptr;
	c_reply_chunk *c_reply = nullptr;
	goto_chunk *go = nullptr;
	c_goto_chunk *c_goto = nullptr;
	c_assign_chunk *c_asgn = nullptr;
	w_reply_chunk *w_reply = nullptr;
	w_entry_chunk *w_entry = nullptr;
	misc_chunk *misc = nullptr;
	c_misc_chunk *c_misc = nullptr;
	assign_chunk *asgn = nullptr;

	if (!c)
		return;

	c->myCNode = 0;
	ent_old = c->myCNode;
	c->myCNode = 0;

	_GC(swap) = true;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case C_ASGN_CHUNK:
			c_asgn = get_c_asgn(c, ent);
			swap_c_asgn(c_asgn);
			break;

		case ASGN_CHUNK:
			asgn = get_asgn(c, ent);
			swap_assign(asgn);
			break;

		case HIDE_CHUNK:
		case DSTR_CHUNK:
		case UHID_CHUNK:
			misc = get_misc(c, ent);
			swap_misc(misc);
			break;

		case CHDE_CHUNK:
		case CUHD_CHUNK:
		case CDST_CHUNK:
			c_misc = get_c_misc(c, ent);
			swap_c_misc(c_misc);
			break;

		case CONV_CHUNK:
			conv = get_conv(c, ent);
			swap_conv(conv);
			break;

		case DECL_CHUNK:
			decl = get_decl(c, ent);
			swap_decl(decl);
			break;

		case FALL_CHUNK:
			fall = get_fall(c, ent);
			swap_fall(fall);
			break;

		case LNODE_CHUNK:
			lnode = get_lnode(c, ent);
			swap_lnode(lnode);
			break;

		case NODE_CHUNK:
			node = get_node(c, ent);
			swap_node(node);
			break;

		case ENTRY_CHUNK:
			entry = get_entry(c, ent);
			swap_entry(entry);
			break;

		case TEXT_CHUNK:
			text = get_text(c, ent);
			swap_text(text);
			break;

		case REPLY_CHUNK:
			reply = get_reply(c, ent);
			swap_reply(reply);
			break;

		case WEIGHT_REPLY_CHUNK:
		case WEIGHT_PREPLY_CHUNK:
			w_reply = get_w_reply(c, ent);
			swap_w_reply(w_reply);

			//dec1
			tempEnt = ent + sizeof(w_reply_chunk);

			for (x = 0; x < w_reply->num_replies; x++) {
				w_entry = get_w_entry(c, tempEnt);
				swap_w_entry(w_entry);
				tempEnt += sizeof(w_entry_chunk);
			}
			break;

		case COND_REPLY_CHUNK:
			c_reply = get_c_reply(c, ent);
			swap_c_reply(c_reply);
			break;

		case MESSAGE_CHUNK:
			mesg = get_mesg(c, ent);
			swap_mesg(mesg);
			break;

		case GOTO_CHUNK:
		case EXIT_GOTO_CHUNK:
			go = get_goto(c, ent);
			swap_goto(go);
			break;

		case COND_GOTO_CHUNK:
		case COND_EXIT_GOTO_CHUNK:
			c_goto = get_c_goto(c, ent);
			swap_c_goto(c_goto);
			break;

		default:
			//term_message( "uh-oh" );
			ent = 0;
			break;
		}

		ent = next;
	}

	_GC(swap) = false;
	c->myCNode = ent_old;
}

} // End of namespace M4
