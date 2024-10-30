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

#include "m4/adv_r/conv.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/chunk_ops.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_univ.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

// ---------------
// Set entry stats,
// Get next node pointer
// Process declarations
// Get message text
static void conv_exec_entry(int32 offset, Conv *c) {
	int32 i = offset, entry_count;
	int32 l_op, r_op;
	int32 tag, next;

	goto_chunk *go;
	c_goto_chunk *c_goto;
	entry_chunk *entry;
	decl_chunk *decl;
	assign_chunk *asgn;
	c_assign_chunk *c_asgn;
	misc_chunk *misc;
	c_misc_chunk *c_misc;

	l_op = r_op = entry_count = 0;

	entry = get_entry(c, i);
	entry_count = entry->size;
	entry->status = conv_toggle_flags(entry);

	i += sizeof(entry_chunk);

	while (i < offset + entry_count) {
		conv_ops_get_entry(i, &next, &tag, c);

		switch (tag) {
		case TEXT_CHUNK:
			break;

		case MESSAGE_CHUNK:
			break;

		case ENTRY_CHUNK:
			break;

		case FALL_CHUNK:
			break;

		case C_ASGN_CHUNK:
			c_asgn = get_c_asgn(c, i);

			decl = get_decl(c, c_asgn->c_op_l);
			l_op = conv_get_decl_val(c, decl);

			if (conv_ops_cond_successful(l_op, c_asgn->c_op, c_asgn->c_op_r)) {
				decl = get_decl(c, c_asgn->index);
				conv_set_decl_val(c, decl, conv_ops_process_asgn(conv_get_decl_val(c, decl), c_asgn->op, c_asgn->opnd1));
			}
			break;

		case ASGN_CHUNK:
			asgn = get_asgn(c, i);
			decl = get_decl(c, asgn->index);
			conv_set_decl_val(c, decl, conv_ops_process_asgn(conv_get_decl_val(c, decl), asgn->op, asgn->opnd1));
			break;

		case HIDE_CHUNK:
			misc = get_misc(c, i);
			entry = get_hash_entry(c, misc->index);

			if (!(entry->status & DESTROYED))
				entry->status |= HIDDEN;
			break;

		case CHDE_CHUNK:
			c_misc = get_c_misc(c, i);
			entry = get_hash_entry(c, c_misc->index);

			decl = get_decl(c, c_misc->c_op_l);
			l_op = conv_get_decl_val(c, decl);

			if (conv_ops_cond_successful(l_op, c_misc->c_op, c_misc->c_op_r)) {
				if (!(entry->status & DESTROYED))
					entry->status |= HIDDEN;
			}
			break;

		case UHID_CHUNK:
			misc = get_misc(c, i);
			entry = get_hash_entry(c, misc->index);
			if (!(entry->status & DESTROYED)) {
				entry->status &= 0xfffffffb; // Mask HIDDEN bit
				entry->status |= 0x00000001;
			}
			break;

		case CUHD_CHUNK:
			c_misc = get_c_misc(c, i);
			entry = get_hash_entry(c, c_misc->index);

			decl = get_decl(c, c_misc->c_op_l);
			l_op = conv_get_decl_val(c, decl);

			if (conv_ops_cond_successful(l_op, c_misc->c_op, c_misc->c_op_r)) {
				if (!(entry->status & DESTROYED)) {
					entry->status &= 0xfffffffb;
					entry->status |= 0x00000001;
				}
			}
			break;

		case DSTR_CHUNK:
			misc = get_misc(c, i);
			entry = get_hash_entry(c, misc->index);
			entry->status |= DESTROYED;
			break;

		case CDST_CHUNK:
			c_misc = get_c_misc(c, i);
			entry = get_hash_entry(c, c_misc->index);

			decl = get_decl(c, c_misc->c_op_l);
			l_op = conv_get_decl_val(c, decl);

			if (conv_ops_cond_successful(l_op, c_misc->c_op, c_misc->c_op_r))
				entry->status |= DESTROYED;
			break;

		case COND_GOTO_CHUNK:
			c_goto = get_c_goto(c, i);
			decl = get_decl(c, c_goto->opnd1);

			l_op = conv_get_decl_val(c, decl);
			r_op = c_goto->opnd2; //val

			if (conv_ops_cond_successful(l_op, c_goto->op, r_op)) {
				c->myCNode = c_goto->index;
				return;
			}
			break;

		case COND_EXIT_GOTO_CHUNK:
			c_goto = get_c_goto(c, i);
			decl = get_decl(c, c_goto->opnd1);

			l_op = conv_get_decl_val(c, decl);
			r_op = c_goto->opnd2; //val

			if (conv_ops_cond_successful(l_op, c_goto->op, r_op)) {
				if (c_goto->index != CONV_QUIT) { //was go->index
					c->myCNode = c_goto->index; //was go->index
					c->exit_now = CONV_BAIL;
				} else {
					c->exit_now = CONV_QUIT;
					c->myCNode = CONV_QUIT;
				}
				return;
			}
			break;

		case EXIT_GOTO_CHUNK:
			go = get_goto(c, i);

			if (go->index != CONV_QUIT) {
				c->myCNode = go->index;
				c->exit_now = CONV_BAIL;
			} else {
				c->exit_now = CONV_QUIT;
				c->myCNode = CONV_QUIT;
			}
			return;

		case GOTO_CHUNK:
			go = get_goto(c, i);
			c->myCNode = go->index;
			return;

		// Replies are non-player responses
		case REPLY_CHUNK:
			break;

		case COND_REPLY_CHUNK:
			break;

		case WEIGHT_REPLY_CHUNK:
			break;

		case WEIGHT_PREPLY_CHUNK:
			break;

		default:
			conv_ops_unknown_chunk(tag, "conv_exec_entry");
			break;
		}

		i = next;
	}
}

static int conv_get_mesg(int32 offset, int32 is_valid, Conv *c) {
	int32 i = offset, entry_count;
	int32 l_op, r_op;
	int32 x, y, s_offset = 0, cSize;
	int32 tag, next;
	int32 text_len;
	int sum, result = 0;

	entry_chunk *entry;
	decl_chunk *decl;
	reply_chunk *reply;
	c_reply_chunk *c_reply;
	w_reply_chunk *w_reply;
	w_entry_chunk *w_entry;

	l_op = r_op = entry_count = 0;

	entry = get_entry(c, i);
	entry_count = entry->size;

	i += sizeof(entry_chunk);

	while (i < offset + entry_count) {
		conv_ops_get_entry(i, &next, &tag, c);

		switch (tag) {
		case TEXT_CHUNK:
			break;

		case MESSAGE_CHUNK:
			break;

		case ENTRY_CHUNK:
			break;

		case FALL_CHUNK:
			break;

		case C_ASGN_CHUNK:
			break;

		case ASGN_CHUNK:
			break;

		case HIDE_CHUNK:
			break;

		case CHDE_CHUNK:
			break;

		case UHID_CHUNK:
			break;

		case CUHD_CHUNK:
			break;

		case DSTR_CHUNK:
			break;

		case CDST_CHUNK:
			break;

		case COND_GOTO_CHUNK:
			break;

		case COND_EXIT_GOTO_CHUNK:
			break;

		case EXIT_GOTO_CHUNK:
			break;

		case GOTO_CHUNK:
			break;

		case REPLY_CHUNK:
			reply = get_reply(c, i);

			if (is_valid) {
				result = 1;
				_G(cdd).player_non_player = 0;

				if (!strcmp(_G(cdd).mesg, "")) {
					text_len = conv_ops_text_strlen(get_string(c, reply->index + sizeof(mesg_chunk)));

					Common::strcpy_s(_G(cdd).mesg, get_string(c, reply->index + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, reply->index + sizeof(mesg_chunk));

				} else {
					Common::strcat_s(_G(cdd).mesg, " ");
					text_len = conv_ops_text_strlen(get_string(c, reply->index + sizeof(mesg_chunk)));
					Common::strcat_s(_G(cdd).mesg, get_string(c, reply->index + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, reply->index + sizeof(mesg_chunk));
				}
			}
			break;

		case COND_REPLY_CHUNK:
			c_reply = get_c_reply(c, i);
			decl = get_decl(c, c_reply->op_l);

			l_op = conv_get_decl_val(c, decl);
			r_op = c_reply->op_r; //val

			if (is_valid && conv_ops_cond_successful(l_op, c_reply->op, r_op)) {
				result = 1;
				_G(cdd).player_non_player = 0;

				if (!strcmp(_G(cdd).mesg, "")) {
					text_len = conv_ops_text_strlen(get_string(c, c_reply->index + sizeof(mesg_chunk)));
					Common::strcpy_s(_G(cdd).mesg, get_string(c, c_reply->index + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, c_reply->index + sizeof(mesg_chunk));

				} else {
					Common::strcat_s(_G(cdd).mesg, " ");
					text_len = conv_ops_text_strlen(get_string(c, c_reply->index + sizeof(mesg_chunk)));
					Common::strcat_s(_G(cdd).mesg, get_string(c, c_reply->index + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, c_reply->index + sizeof(mesg_chunk));
				}
			}
			break;

		case WEIGHT_REPLY_CHUNK:
			cSize = i;
			w_reply = get_w_reply(c, i);
			cSize += sizeof(w_reply_chunk);

			sum = 0;
			for (x = 0; x < w_reply->num_replies; x++) {
				w_entry = get_w_entry(c, cSize);
				sum += w_entry->weight;
				cSize += sizeof(w_entry_chunk);
			}
			y = g_engine->getRandomNumber(sum - 1) + 1;

			cSize = i;
			w_reply = get_w_reply(c, i);
			cSize += sizeof(w_reply_chunk);

			sum = 0;
			for (x = 0; (x < w_reply->num_replies) && (sum < y); x++) {
				w_entry = get_w_entry(c, cSize);
				s_offset = w_entry->index;
				sum += w_entry->weight;
				cSize += sizeof(w_entry_chunk);
			}

			if (is_valid) {
				result = 1;
				_G(cdd).player_non_player = 0;

				if (!strcmp(_G(cdd).mesg, "")) {
					text_len = conv_ops_text_strlen(get_string(c, s_offset + sizeof(mesg_chunk)));
					Common::strcpy_s(_G(cdd).mesg, get_string(c, s_offset + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, s_offset + sizeof(mesg_chunk));
				} else {
					Common::strcat_s(_G(cdd).mesg, " ");
					text_len = conv_ops_text_strlen(get_string(c, s_offset + sizeof(mesg_chunk)));
					Common::strcat_s(_G(cdd).mesg, get_string(c, s_offset + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, s_offset + sizeof(mesg_chunk));
				}
			}
			break;

		case WEIGHT_PREPLY_CHUNK:
			cSize = i;
			w_reply = get_w_reply(c, i);
			cSize += sizeof(w_reply_chunk);

			sum = 0;
			for (x = 0; x < w_reply->num_replies; x++) {
				w_entry = get_w_entry(c, cSize);
				sum += w_entry->weight;
				cSize += sizeof(w_entry_chunk);
			}
			y = g_engine->getRandomNumber(sum - 1) + 1;

			cSize = i;
			w_reply = get_w_reply(c, i);
			cSize += sizeof(w_reply_chunk);

			sum = 0;
			for (x = 0; (x < w_reply->num_replies) && (sum < y); x++) {
				w_entry = get_w_entry(c, cSize);
				s_offset = w_entry->index;
				sum += w_entry->weight;
				cSize += sizeof(w_entry_chunk);
			}

			if (is_valid) {
				result = 1;
				_G(cdd).player_non_player = 1;

				if (!strcmp(_G(cdd).mesg, "")) {
					text_len = conv_ops_text_strlen(get_string(c, s_offset + sizeof(mesg_chunk)));
					Common::strcpy_s(_G(cdd).mesg, get_string(c, s_offset + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, s_offset + sizeof(mesg_chunk));
				} else {
					Common::strcat_s(_G(cdd).mesg, " ");
					text_len = conv_ops_text_strlen(get_string(c, s_offset + sizeof(mesg_chunk)));
					Common::strcat_s(_G(cdd).mesg, get_string(c, s_offset + sizeof(mesg_chunk) + text_len));
					_G(cdd).mesg_snd_file = get_string(c, s_offset + sizeof(mesg_chunk));
				}
			}
			break;

		default:
			conv_ops_unknown_chunk(tag, "conv_get_mesg");
			break;
		}
		i = next;
	}

	return result;
}

static void find_true_ent(int entry_num, Conv *c) {
	node_chunk *node = nullptr;
	lnode_chunk *lnode = nullptr;
	entry_chunk *entry = nullptr;
	fall_chunk *fall = nullptr;
	int32 offset = 0, ent = 0, n = 0;
	int32 next = 0, tag = 0, num_ents = 0;
	int i = 0;
	int32 sub_ent = 0;
	int result = 1;

	for (;;) {
		// Start by getting the current NODE or LNODE
		conv_ops_get_entry(ent, &next, &tag, c);
		switch (tag) {
		case LNODE_CHUNK:
			lnode = get_lnode(c, ent);
			ent += sizeof(lnode_chunk);
			num_ents = lnode->num_entries;
			entry_num = lnode->entry_num;
			c->node_hash = lnode->hash;
			break;

		case NODE_CHUNK:
			node = get_node(c, ent);
			ent += sizeof(node_chunk);
			num_ents = node->num_entries;
			c->node_hash = node->hash;
			break;

		default:
			break;
		}

		// ent will now be pointing at an ENTRY or FALLTHROUGH
		sub_ent = next;
		conv_ops_get_entry(sub_ent, &next, &tag, c);
		switch (tag) {
		case FALL_CHUNK:
			// We either want to jump to a new node
			// or skip to the first offset.

			fall = get_fall(c, sub_ent);
			assert(fall);

			//do this to skip the fall chunk and all will be fine.
			ent += sizeof(int32); //was get_long, sizeof( fall_chunk )
			n++; //don't increment i.
			break;

		case ENTRY_CHUNK:
			//don't bother.... result is 1 if FALL_CHUNK not encountered.
			break;

		default:
			break;
		}

		if (result)
			break;
	}

	_GC(ent) = 0;

	// Not only i<entry_num, check to see entry->num_entries
	for (i = 0, n = 0; n < num_ents; n++) {
		offset = get_long(c, ent);
		entry = get_entry(c, ent + offset);

		if (i == entry_num)
			break;

		if (((entry->status) & 0x00000003) && ok_status(entry)) //was 1
			i++;

		_GC(ent)++;

		ent += sizeof(int32);
	}
}


// Simplify me now that all changes have been made.
static int conv_get_node_text(Conv *c) {
	lnode_chunk *lnode = nullptr;
	node_chunk *node = nullptr;
	entry_chunk *entry = nullptr;
	fall_chunk *fall = nullptr;

	int32 ent = 0, offset = 0, tag, next, num_ents = 0;
	int i = 0, num_vis = 0, result = 0;

	_G(cdd).num_txt_ents = 0;
	_GC(width) = 0; _GC(height) = 0;

	// conv _get_node_text will either get a NODE or LNODE
	conv_ops_get_entry(offset, &next, &tag, c);
	offset = 0; //not needed.?
	_GC(n_t_e) = 0;

	switch (tag) {
	case LNODE_CHUNK:
		lnode = get_lnode(c, offset);
		offset += sizeof(lnode_chunk);
		num_ents = lnode->num_entries;
		c->node_hash = lnode->hash;
		break;

	case NODE_CHUNK:
		node = get_node(c, offset);
		offset += sizeof(node_chunk);
		num_ents = node->num_entries;
		c->node_hash = node->hash;
		break;

	default:
		// handle error.
		break;
	}

	switch (tag) {
	case LNODE_CHUNK:
		// was in bounds.
		// lnode->entry_num starts at 0. in the chunk.
		if (lnode->entry_num >= lnode->num_entries)
			lnode->entry_num = 0;

		for (i = 0; (i <= lnode->entry_num) && (i < num_ents); i++) {
			ent = get_long(c, offset);
			entry = get_entry(c, offset + ent);

			offset += sizeof(int32);
		}

		offset -= sizeof(int32);

		// Set sound file name instead.?
		if ((entry->status != 0) && (num_ents != 0) && ok_status(entry)) {
			//here here here.
			if (conv_get_text(offset + ent, entry->size, c)) {
				result = 1;

				_G(cdd).num_txt_ents = 0;
				_G(cdd).mesg_snd_file = _G(cdd).snd_files[0];

				_G(cdd).text[0] = nullptr;
				_G(cdd).snd_files[0] = nullptr;
				_G(cdd).player_non_player = 1;
				c->c_entry_num = lnode->entry_num;
			}
			num_vis++;
		}

		lnode->entry_num++;

		_GC(ent) = lnode->entry_num;
		break;

	case NODE_CHUNK:
		for (i = 0; i < num_ents; i++) {

			ent = get_long(c, offset);
			entry = get_entry(c, offset + ent);

			if (entry->tag != FALL_CHUNK) {
				if ((entry->status != 0) && (num_ents != 0) && ok_status(entry)) {
					if (conv_get_text(offset + ent, entry->size, c)) {
						result = 1;

						_G(cdd).player_non_player = 1;
					}
					num_vis++;
					_GC(n_t_e)++;
				}
			} else {
				fall = get_fall(c, offset + ent);
			}

			offset += sizeof(int32);
		}

		if (fall) {
			if (num_vis <= fall->val) {
				_GC(n_t_e) = 0;

				c->myCNode = fall->index;
				c->mode = CONV_GET_TEXT_MODE;
				result = 0;
			}
		}
		break;
	}

	return result;
}

void conv_shutdown(void) {
	if (conv_get_handle())
		conv_unload(conv_get_handle());

	if (_GC(myTextScrn))
		TextScrn_Destroy(_GC(myTextScrn));
	_GC(myTextScrn) = nullptr;
}

static void conv_start(Conv *c) {
	int32 ok = 1, ent = 0, tag = 0, next;
	decl_chunk *decl;

	switch (c->exit_now) {
	case CONV_OK:
		// Potential prob. when entering while loop.
		break;

	// Goto_exit encountered.
	// a conversation state.
	case CONV_BAIL:
		return;

	// Goodbye forever.
	case CONV_QUIT:
		return;

	//new conv. no restore file on hard disk.
	case CONV_NEW:
		c->exit_now = CONV_OK;
		c->myCNode = 0;
		break;
	}

	while ((ent < c->chunkSize) && ok) {
		conv_ops_get_entry(ent, &next, &tag, c); //ok if c->myCNode = 0

		switch (tag) {
		case LNODE_CHUNK:
		case NODE_CHUNK:
			ok = 0;
			break;

		case DECL_CHUNK:
			decl = get_decl(c, ent);
			assert(decl);
			break;

		default:
			break;
		}

		if (ok)
			ent = next;
	}
	c->myCNode = ent;

	// if we exit, the current node is set, the next node is null
}

static int conv_next_node(Conv *c) {
	if (c->myCNode == -1)
		return 0;

	switch (c->exit_now) {
	case CONV_OK:
		return 1;

	case CONV_QUIT:
	case CONV_BAIL:
		return 0;

	case CONV_NEW:
		conv_start(c); // Should go in conv_load.
		return 1;
	}
	return 1;
}

static int conv_process_entry(int entry_num, Conv *c, int mode) {
	node_chunk *node = nullptr;
	lnode_chunk *lnode = nullptr;
	entry_chunk *entry = nullptr;
	fall_chunk *fall = nullptr;
	int32 offset = 0, ent = 0, is_valid = 0, n = 0;
	int32 next = 0, tag = 0, num_ents = 0;
	int i = 0;
	int32 sub_ent = 0;
	int	result = 1;

	// Repeat fallthrough till done
	for (;; ) {
		// Start by getting the current NODE or LNODE
		conv_ops_get_entry(ent, &next, &tag, c);
		switch (tag) {
		case LNODE_CHUNK:
			lnode = get_lnode(c, ent);
			ent += sizeof(lnode_chunk);
			num_ents = lnode->num_entries;
			entry_num = lnode->entry_num;
			c->node_hash = lnode->hash;
			break;

		case NODE_CHUNK:
			node = get_node(c, ent);
			ent += sizeof(node_chunk);
			num_ents = node->num_entries;
			c->node_hash = node->hash;
			break;

		default:
			break;
		}

		// ent will now be pointing at an ENTRY or FALLTHROUGH
		sub_ent = next;
		conv_ops_get_entry(sub_ent, &next, &tag, c);
		switch (tag) {
		case FALL_CHUNK:
			// We either want to jump to a new node
			// or skip to the first offset.

			fall = get_fall(c, sub_ent);
			assert(fall);

			// Do this to skip the fall chunk and all will be fine.
			ent += sizeof(int32); //was get_long, sizeof( fall_chunk )
			n++; //don't increment i.
			break;

		case ENTRY_CHUNK:
			break;

		default:
			break;
		}

		if (result)
			break;
	}

	// Not only i<entry_num, check to see entry->num_entries
	while ((i < entry_num) && (n < num_ents)) {
		offset = get_long(c, ent);
		entry = get_entry(c, ent + offset);

		if (((entry->status) != 0) && ok_status(entry)) {
			i++;
			is_valid = 1;
		}

		ent += sizeof(int32);
		n++;
	}

	ent -= sizeof(int32);

	if (is_valid) {
		switch (mode) {
		case CONV_GET_MESG_MODE:
			result = conv_get_mesg(ent + offset, is_valid, c);
			break;

		case CONV_UPDATE_MODE:
			conv_exec_entry(ent + offset, c);
			break;
		}
	}

	return result;
}

static void textBoxInit();

static int conv_run(Conv *c) {
	if (!c)
		return 0;

	int result = 0;
	int ok = 1;

	if (conv_next_node(c)) {
		switch (c->exit_now) {
		case CONV_NEW:
			break;

		case CONV_QUIT:
			break;

		case CONV_BAIL:
			break;

		case CONV_OK:
			while (ok && conv_next_node(c)) {
				switch (c->mode) {
				case CONV_GET_TEXT_MODE:
					cdd_init();

					c->mode = CONV_GET_MESG_MODE;
					if (conv_get_node_text(c)) {
						ok = 0;
						if (_G(cdd).num_txt_ents) { //node
							mouse_unlock_sprite();
							mouse_lock_sprite(0);

							textBoxInit();
						} else { //linear node.
							conv_set_event(-1);

							Common::strcpy_s(_G(player).verb, _GC(conv_name)); // was verb.
							c->c_entry_num = 1;
						}
					}
					break;

				case CONV_GET_MESG_MODE:
					cdd_init();

					if (conv_process_entry(c->c_entry_num, c, CONV_GET_MESG_MODE)) {
						mouse_unlock_sprite();
						mouse_lock_sprite(5);

						conv_set_event(-1);
						Common::strcpy_s(_G(player).verb, _GC(conv_name));

						ok = 0;
					}
					c->mode = CONV_UPDATE_MODE;
					break;

				case CONV_UPDATE_MODE:
					conv_process_entry(c->c_entry_num, c, CONV_UPDATE_MODE);

					c->mode = CONV_GET_TEXT_MODE;
					break;
				}
			}
			break;
		}
	}

	if (!conv_next_node(c))
		conv_unload(c);

	return result;
}

static void convtestCallback(void *a, void *) {
	Conv *c;

	mouse_unlock_sprite();
	mouse_lock_sprite(5);

	player_set_commands_allowed(false);
	TextItem *i = (TextItem *)a;

	c = conv_get_handle();
	if (!c)
		return;

	c->c_entry_num = i->tag;
	c->mode = CONV_GET_MESG_MODE;

	TextScrn_Destroy(_GC(myTextScrn));
	_GC(myTextScrn) = nullptr;

	find_true_ent(c->c_entry_num, c);

	_G(cdd).mesg_snd_file = _G(cdd).snd_files[c->c_entry_num - 1];
	_G(cdd).player_non_player = 1;

	Common::strcpy_s(_G(player).verb, _GC(conv_name));
	_G(player).command_ready = true;
	conv_set_event(-1); // Must have or conv freezes.
}

void set_dlg_rect() {
	int32 sizex = 0, sizey = 0;
	int32 screen_x_center = 0, screen_y_center = 0;
	int32 screen_x_size = 0, screen_y_size = 0;
	int32 status;

	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	if (!game_buff_ptr) error_show(FL, 'BUF!');

	screen_x_center = VIDEO_W / 2;
	screen_y_center = (game_buff_ptr->y2 - game_buff_ptr->y1) / 2;
	screen_x_size = VIDEO_W;
	screen_y_size = (game_buff_ptr->y2 - game_buff_ptr->y1);

	_GC(height) = gr_font_get_height() + _GC(conv_font_spacing_v); // Must have....
	_GC(width) += 2 * _GC(conv_font_spacing_h);

	sizex = _GC(width);
	sizey = _G(cdd).num_txt_ents * (_GC(height))+_GC(conv_font_spacing_v);

	switch (_GC(glob_x)) {
	case DLG_CENTER_H:
		_GC(r_x1) = screen_x_center - (sizex / 2);
		break;

	case DLG_FLUSH_LEFT:
		_GC(r_x1) = 0;
		break;

	case DLG_FLUSH_RIGHT:
		_GC(r_x1) = screen_x_size - sizex;
		break;

	default:
		_GC(r_x1) = _GC(glob_x);
		_GC(r_x1) += game_buff_ptr->x1;
		break;
	}

	switch (_GC(glob_y)) {
	case DLG_CENTER_V:
		_GC(r_y1) = screen_y_center - (sizey / 2);
		break;

	case DLG_FLUSH_TOP:
		_GC(r_y1) = 0;
		break;

	case DLG_FLUSH_BOTTOM:
		_GC(r_y1) = screen_y_size - sizey + game_buff_ptr->y1 - 10;
		break;

	default:
		_GC(r_y1) = _GC(glob_y);
		_GC(r_y1) += game_buff_ptr->y1;
		break;
	}

	if (_GC(r_x1) < 0)
		_GC(r_x1) = 0;

	if (_GC(r_y1) < 0)
		_GC(r_y1) = 0;

	_GC(r_y2) = _GC(r_y1) + sizey - 1;
	_GC(r_x2) = _GC(r_x1) + sizex - 1;

	_GC(r_x2) = imath_min(VIDEO_W, _GC(r_x2));
	_GC(r_y2) = imath_min(VIDEO_H, _GC(r_y2));
}

static void textBoxInit() {
	int32 i;

	player_set_commands_allowed(true);
	mouse_set_sprite(0);

	gr_font_set(_G(font_conv));

	conv_get_node_text(conv_get_handle());
	set_dlg_rect();

	_GC(myTextScrn) = TextScrn_Create(_GC(r_x1), _GC(r_y1), _GC(r_x2), _GC(r_y2), _GC(conv_shading),
		6 | SF_GET_MOUSE | SF_IMMOVABLE | SF_BLOCK_MOUSE,
		_GC(conv_normal_colour), _GC(conv_hilite_colour), _GC(conv_normal_colour_alt1),
		_GC(conv_hilite_colour_alt1), _GC(conv_normal_colour_alt2),
		_GC(conv_hilite_colour_alt2));

	for (i = 0; i < _G(cdd).num_txt_ents; i++) {
		TextScrn_Add_TextItem(_GC(myTextScrn), _GC(conv_font_spacing_h),
			(i * _GC(height)) + _GC(conv_font_spacing_v), i + 1, TS_GIVEN,
			_G(cdd).text[i], convtestCallback);
	}

	TextScrn_Activate(_GC(myTextScrn));
}

void conv_get_dlg_coords(int32 *x1, int32 *y1, int32 *x2, int32 *y2) {
	*x1 = _GC(r_x1);
	*y1 = _GC(r_y1);
	*x2 = _GC(r_x2);
	*y2 = _GC(r_y2);
}

void conv_set_dlg_coords(int32 x1, int32 y1, int32 x2, int32 y2) {
	_GC(r_x1) = x1;
	_GC(r_y1) = y1;
	_GC(r_x2) = x2;
	_GC(r_y2) = y2;
}

void conv_go(Conv *c) {
	conv_run(c);
}

} // End of namespace M4
