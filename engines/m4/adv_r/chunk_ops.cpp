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

int32 conv_ops_text_strlen(char *s) {
	int32 len = 0;
	len = strlen(s) + 1;	// Added +1 for null char.

	if ((len % 4) == 0)
		return len;

	len += 4 - (len % 4);
	return len;
}

void conv_ops_unknown_chunk(int32 tag, const char *s) {
	char *tag_name = nullptr;

	tag_name = (char *)&tag;

	error_show(FL, 'PARS', "'%s' What type is this chunk: %c%c%c%c ?", s, tag_name[3], tag_name[2], tag_name[1], tag_name[0]);
}

/*
 * Find an entry and auto-advance the pointer past it.
 */
char *conv_ops_get_entry(int32 i, int32 *next, int32 *tag, Conv *c) {
	int32 num_blocks = 0;
	int32 j = 0, k = 0;
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
	*tag = *(int32 *)&outChunk[i];

	if (_GC(swap))
		*tag = convert_intel32(*tag);

	switch (*tag) {
	case C_ASGN_CHUNK:
		k = sizeof(c_assign_chunk);
		break;

	case ASGN_CHUNK:
		k = sizeof(assign_chunk);
		break;

	case CONV_CHUNK:
		CC = (conv_chunk *)&outChunk[i];

		j = CC->size;
		break;

	case DECL_CHUNK:
		k = sizeof(decl_chunk);
		break;

	case FALL_CHUNK:
		k = sizeof(fall_chunk);
		break;

	case LNODE_CHUNK:
		L = (lnode_chunk *)&outChunk[i];

		k = sizeof(lnode_chunk);

		if (_GC(swap)) {
			j = convert_intel32(L->num_entries) * sizeof(int32);
			j = convert_intel32(j);
		} else {
			j = L->num_entries * sizeof(int32);
		}
		break;

	case NODE_CHUNK:
		N = (node_chunk *)&outChunk[i];

		k = sizeof(node_chunk);

		if (_GC(swap)) {
			j = convert_intel32(N->num_entries) * sizeof(int32);
			j = convert_intel32(j);
		} else {
			j = N->num_entries * sizeof(int32); //was +=
		}
		break;

	case ENTRY_CHUNK:
		k = sizeof(entry_chunk);
		break;

	case TEXT_CHUNK:
	case MESSAGE_CHUNK:
		T = (text_chunk *)&outChunk[i];
		num_blocks = T->size;

		k = sizeof(text_chunk);
		j = num_blocks;

		break;

	case REPLY_CHUNK:
		k = sizeof(reply_chunk);
		break;

	case WEIGHT_REPLY_CHUNK:
		W = (w_reply_chunk *)&outChunk[i];
		k = sizeof(w_reply_chunk);

		if (_GC(swap)) {
			j = convert_intel32(W->num_replies) * (2 * sizeof(int32)); //was +=
			j = convert_intel32(j);
		} else {
			j = W->num_replies * (2 * sizeof(int32)); //was +=
		}
		break;

	case WEIGHT_PREPLY_CHUNK:
		W = (w_reply_chunk *)&outChunk[i];
		k = sizeof(w_reply_chunk);

		if (_GC(swap)) {
			j = convert_intel32(W->num_replies) * (2 * sizeof(int32)); //was +=
			j = convert_intel32(j);
		} else {
			j = W->num_replies * (2 * sizeof(int32)); //was +=
		}
		break;

	case COND_REPLY_CHUNK:
		k = sizeof(c_reply_chunk);
		break;

	case COND_EXIT_GOTO_CHUNK:
	case COND_GOTO_CHUNK:
		k = sizeof(c_goto_chunk);
		break;

	case GOTO_CHUNK:
	case EXIT_GOTO_CHUNK:
		k = sizeof(goto_chunk);
		break;

	case HIDE_CHUNK:
	case UHID_CHUNK:
	case DSTR_CHUNK:
		k = sizeof(misc_chunk);
		break;

	case CHDE_CHUNK:
	case CUHD_CHUNK:
	case CDST_CHUNK:
		k = sizeof(c_misc_chunk);
		break;

	default:
		error_show(FL, 'PARS', "Tag: %d (%x) Node: %d (%x hex)", *tag, *tag, c->myCNode, c->myCNode);
		break;
	}

	if (_GC(swap))
		j = convert_intel32(j);
	j += k;

	j += i;
	*next = j;

	return &outChunk[i];
}

static void swap_assign(assign_chunk *a) {
	a->tag = convert_intel32(a->tag);
	a->index = convert_intel32(a->index);
	a->op = convert_intel32(a->op);
	a->opnd1 = convert_intel32(a->opnd1);
}

assign_chunk *get_asgn(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (assign_chunk *)&s[cSize];
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

c_assign_chunk *get_c_asgn(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (c_assign_chunk *)&s[cSize];
}

static void swap_conv(conv_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->size = convert_intel32(c->size);
}

conv_chunk *get_conv(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (conv_chunk *)&s[cSize];
}

static void swap_decl(decl_chunk *d) {
	d->tag = convert_intel32(d->tag);
	d->val = convert_intel32(d->val);
	d->flags = convert_intel32(d->flags);
}

decl_chunk *get_decl(Conv *c, int32 cSize) {
	return (decl_chunk *)&c->conv[cSize];
}

static void swap_fall(fall_chunk *l) {
	l->tag = convert_intel32(l->tag);
	l->val = convert_intel32(l->val);
	l->index = convert_intel32(l->index);
}

fall_chunk *get_fall(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (fall_chunk *)&s[cSize];
}

static void swap_lnode(lnode_chunk *l) {
	int i = 0;
	int32 *L = nullptr;

	l->tag = convert_intel32(l->tag);
	l->hash = convert_intel32(l->hash);
	l->size = convert_intel32(l->size);
	l->entry_num = convert_intel32(l->entry_num);
	l->num_entries = convert_intel32(l->num_entries);

	L = (int32 *)l;
	L += 5;
	for (i = 0; i < l->num_entries; i++) {
		L[i] = convert_intel32(L[i]);
	}
}

lnode_chunk *get_lnode(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (lnode_chunk *)&s[cSize];
}

static void swap_node(node_chunk *n) {
	int i = 0;
	int32 *L = nullptr;

	n->tag = convert_intel32(n->tag);
	n->hash = convert_intel32(n->hash);
	n->size = convert_intel32(n->size);
	n->num_entries = convert_intel32(n->num_entries);

	L = (int32 *)(n + 1);

	for (i = 0; i < n->num_entries; i++) {
		L[i] = convert_intel32(L[i]);
	}
}

node_chunk *get_node(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (node_chunk *)&s[cSize];
}

static void swap_entry(entry_chunk *e) {
	e->tag = convert_intel32(e->tag);
	e->size = convert_intel32(e->size);
	e->status = convert_intel32(e->status);
}

entry_chunk *get_entry(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (entry_chunk *)&s[cSize];
}

entry_chunk *get_hash_entry(Conv *c, int32 cSize) {
	char *s = &(c->conv[0]);
	return (entry_chunk *)&s[cSize];
}

static void swap_text(text_chunk *t) {
	t->tag = convert_intel32(t->tag);
	t->size = convert_intel32(t->size);
}

text_chunk *get_text(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (text_chunk *)&s[cSize];
}

static void swap_mesg(mesg_chunk *m) {
	m->tag = convert_intel32(m->tag);
	m->size = convert_intel32(m->size);
}

mesg_chunk *get_mesg(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (mesg_chunk *)&s[cSize];
}

static void swap_reply(reply_chunk *r) {
	r->tag = convert_intel32(r->tag);
	r->index = convert_intel32(r->index);
}

reply_chunk *get_reply(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (reply_chunk *)&s[cSize];
}

static void swap_c_reply(c_reply_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->op_l = convert_intel32(c->op_l);
	c->op = convert_intel32(c->op);
	c->op_r = convert_intel32(c->op_r);
	c->index = convert_intel32(c->index);
}

c_reply_chunk *get_c_reply(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (c_reply_chunk *)&s[cSize];
}

static void swap_w_reply(w_reply_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->num_replies = convert_intel32(c->num_replies);
}

w_reply_chunk *get_w_reply(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (w_reply_chunk *)&s[cSize];
}

static void swap_w_entry(w_entry_chunk *w) {
	w->weight = convert_intel32(w->weight);
	w->index = convert_intel32(w->index);
}

w_entry_chunk *get_w_entry(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (w_entry_chunk *)&s[cSize];
}

static void swap_goto(goto_chunk *g) {
	g->tag = convert_intel32(g->tag);
	g->index = convert_intel32(g->index);
}

goto_chunk *get_goto(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (goto_chunk *)&s[cSize];
}

static void swap_c_goto(c_goto_chunk *c) {
	c->tag = convert_intel32(c->tag);
	c->opnd1 = convert_intel32(c->opnd1);
	c->op = convert_intel32(c->op);
	c->opnd2 = convert_intel32(c->opnd2);
	c->index = convert_intel32(c->index);
}

c_goto_chunk *get_c_goto(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (c_goto_chunk *)&s[cSize];
}

static void swap_misc(misc_chunk *m) {
	m->tag = convert_intel32(m->tag);
	m->index = convert_intel32(m->index);
}

misc_chunk *get_misc(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (misc_chunk *)&s[cSize];
}

static void swap_c_misc(c_misc_chunk *c) {
	c->tag = convert_intel32(c->tag);

	c->c_op_l = convert_intel32(c->c_op_l);
	c->c_op = convert_intel32(c->c_op);
	c->c_op_r = convert_intel32(c->c_op_r);

	c->index = convert_intel32(c->index);
}

c_misc_chunk *get_c_misc(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	return (c_misc_chunk *)&s[cSize];
}

int32 get_long(Conv *c, int32 cSize) {
	char *s = &(c->conv[c->myCNode]);
	int32 *l = (int32 *)&s[cSize];
	return *l;
}

char *get_string(Conv *c, int32 cSize) {
	return &c->conv[cSize];
}

int conv_ops_cond_successful(int32 l_op, int32 op, int32 r_op) {
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

int32 conv_ops_process_asgn(int32 val, int32 oprtr, int32 opnd) {
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
		error_show(FL, 'PARS', "Operator must be a +,-,*,/,=, Please check tag type: %d in token header file", oprtr);
		break;
	}
	return val;
}

void conv_swap_words(Conv *c) {
	int32 ent = 0, tag = 0, next;
	int32 ent_old = 0;
	int32 tempEnt = 0;
	int x = 0;

	conv_chunk *conv = nullptr;
	decl_chunk *decl = nullptr;
	node_chunk *node = nullptr;
	fall_chunk *fall = nullptr;
	lnode_chunk *lnode = nullptr;
	entry_chunk *entry = nullptr;

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
			ent = 0;
			break;
		}

		ent = next;
	}

	_GC(swap) = false;
	c->myCNode = ent_old;
}

} // End of namespace M4
