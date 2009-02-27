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

#ifndef SCI_ENGINE_MESSAGE_H
#define SCI_ENGINE_MESSAGE_H

#include "sci/scicore/resource.h"

namespace Sci {

struct message_tuple_t {
	int noun;
	int verb;
	int cond;
	int seq;
};

struct index_record_cursor_t {
	byte *index_record;
	int index;
	byte *resource_beginning;
};

typedef int index_record_size_t();
typedef void parse_index_record_t(index_record_cursor_t *index_record, message_tuple_t *t);
typedef int get_talker_t(index_record_cursor_t *cursor);
typedef void get_text_t(index_record_cursor_t *cursor, char *buffer, int buffer_size);
typedef int index_record_count_t(byte *header);

struct message_handler_t {
	int version_id;
	parse_index_record_t *parse;
	get_talker_t *get_talker;
	get_text_t *get_text;
	index_record_count_t *index_record_count;

	int header_size;
	int index_record_size;
};

struct message_state_t {
	int initialized;
	message_handler_t *handler;
	ResourceManager *resmgr;
	resource_t *current_res;
	int module;
	int record_count;
	byte *index_records;
	index_record_cursor_t engine_cursor;
};

int message_get_specific(message_state_t *state, message_tuple_t *t);
int message_get_next(message_state_t *state);
int message_get_talker(message_state_t *state);
int message_get_length(message_state_t *state);
int message_get_text(message_state_t *state, char *buffer, int length);
int message_state_load_res(message_state_t *state, int module);
void message_state_initialize(ResourceManager *resmgr, message_state_t *state);

} // End of namespace Sci

#endif // SCI_ENGINE_MESSAGE_H
