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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/message.h"
#include "sci/tools.h"

namespace Sci {

#if 0
// Unreferenced - removed
static int get_talker_trivial(IndexRecordCursor *cursor) {
	return -1;
}
#endif

/* Version 2.101 and later code ahead */

#if 0
// Unreferenced - removed
static void index_record_parse_2101(IndexRecordCursor *cursor, MessageTuple *t) {
	int noun = *(cursor->index_record + 0);
	int verb = *(cursor->index_record + 1);

	t->noun = noun;
	t->verb = verb;
	t->cond = t->seq = 0;
}
#endif

#if 0
// Unreferenced - removed
static void index_record_get_text_2101(IndexRecordCursor *cursor, char *buffer, int buffer_size) {
	int offset = getUInt16(cursor->index_record + 2);
	char *stringptr = (char *)cursor->resource_beginning + offset;

	strncpy(buffer, stringptr, buffer_size);
}
#endif

#if 0
// Unreferenced - removed
static int header_get_index_record_count_2101(byte *header) {
	return getUInt16(header + 4);
}
#endif

// Version 3.411 and later code ahead

static void index_record_parse_3411(IndexRecordCursor *cursor, MessageTuple *t) {
	int noun = *(cursor->index_record + 0);
	int verb = *(cursor->index_record + 1);
	int cond = *(cursor->index_record + 2);
	int seq = *(cursor->index_record + 3);

	t->noun = noun;
	t->verb = verb;
	t->cond = cond;
	t->seq = seq;
}

static int index_record_get_talker_3411(IndexRecordCursor *cursor) {
	return *(cursor->index_record + 4);
}

static void index_record_get_text_3411(IndexRecordCursor *cursor, char *buffer, int buffer_size) {
	int offset = getUInt16(cursor->index_record + 5);
	char *stringptr = (char *)cursor->resource_beginning + offset;

	strncpy(buffer, stringptr, buffer_size);
}

static int header_get_index_record_count_3411(byte *header) {
	return getUInt16(header + 8);
}

// Generic code from here on

static int four_tuple_match(MessageTuple *t1, MessageTuple *t2) {
	return t1->noun == t2->noun && t1->verb == t2->verb && t1->cond == t2->cond && t1->seq == t2->seq;
}

static void index_record_cursor_initialize(MessageState *state, IndexRecordCursor *cursor) {
	cursor->resource_beginning = state->current_res->data;
	cursor->index_record = state->index_records;
	cursor->index = 1;
}

static int index_record_next(MessageState *state, IndexRecordCursor *cursor) {
	if (cursor->index == state->record_count)
		return 0;
	cursor->index_record += state->handler->index_record_size;
	cursor->index ++;
	return 1;
}

static int index_record_find(MessageState *state, MessageTuple *t, IndexRecordCursor *cursor) {
	MessageTuple looking_at;
	int found = 0;

	index_record_cursor_initialize(state, cursor);

	do {
		state->handler->parse(cursor, &looking_at);
		if (four_tuple_match(t, &looking_at))
			found = 1;
	} while (!found && index_record_next(state, cursor));

	// FIXME: Recursion not handled yet

	return found;
}

int message_get_specific(MessageState *state, MessageTuple *t) {
	return index_record_find(state, t, &state->engine_cursor);
}

int message_get_next(MessageState *state) {
	return index_record_next(state, &state->engine_cursor);
}

int message_get_talker(MessageState *state) {
	return state->handler->get_talker(&state->engine_cursor);
}

int message_get_text(MessageState *state, char *buffer, int length) {
	state->handler->get_text(&state->engine_cursor, buffer, length);
	return strlen(buffer);
}

int message_get_length(MessageState *state) {
	char buffer[500];

	state->handler->get_text(&state->engine_cursor, buffer, sizeof(buffer));
	return strlen(buffer);
}

int message_state_load_res(MessageState *state, int module) {
	if (state->module == module)
		return 1;

	state->module = module;
	state->current_res = state->resmgr->findResource(sci_message, module, 0);

	if (state->current_res == NULL || state->current_res->data == NULL) {
		sciprintf("Message subsystem: Failed to load %d.MSG\n", module);
		return 0;
	}

	state->record_count = state->handler->index_record_count(state->current_res->data);
	state->index_records = state->current_res->data + state->handler->header_size;

	index_record_cursor_initialize(state, &state->engine_cursor);
	return 1;
}

static MessageHandler fixed_handler = {
	3411,
	index_record_parse_3411,
	index_record_get_talker_3411,
	index_record_get_text_3411,
	header_get_index_record_count_3411,
	10,
	11
};

void message_state_initialize(ResourceManager *resmgr, MessageState *state) {
	//Resource *tester = resmgr->findResource(sci_message, 0, 0);
	//int version;

	//if (tester == NULL)
	//	return;

	//version = getUInt16(tester->data);

	state->initialized = 1;
	state->module = -1;
	state->resmgr = resmgr;
	state->current_res = NULL;
	state->record_count = 0;
	state->handler = &fixed_handler;
}

} // End of namespace Sci
