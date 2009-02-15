/***************************************************************************
 message.h Copyright (C) 2008 Lars Skovlund


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

    Lars Skovlund (LS) [lskovlun@image.dk]

***************************************************************************/

#include <sciresource.h>

typedef struct
{
	int noun;
	int verb;
	int cond;
	int seq;
} message_tuple_t;

typedef struct
{
	byte *index_record;
	int index;
	byte *resource_beginning;
} index_record_cursor_t;

typedef int index_record_size_t(void);
typedef void parse_index_record_t(index_record_cursor_t *index_record, message_tuple_t *t);
typedef int get_talker_t(index_record_cursor_t *cursor);
typedef void get_text_t(index_record_cursor_t *cursor, char *buffer, int buffer_size);
typedef int index_record_count_t(byte *header);

typedef struct
{
	int version_id;
	parse_index_record_t *parse;
	get_talker_t *get_talker;
	get_text_t *get_text;
	index_record_count_t *index_record_count;
	
	int header_size;
	int index_record_size;
} message_handler_t;

typedef struct
{
	int initialized;
	message_handler_t *handler;
	resource_mgr_t *resmgr;
	resource_t *current_res;
	int module;
	int record_count;
	byte *index_records;
	index_record_cursor_t engine_cursor;
} message_state_t;

int message_get_specific(message_state_t *state, message_tuple_t *t);
int message_get_next(message_state_t *state);
int message_get_talker(message_state_t *state);
int message_get_length(message_state_t *state);
int message_get_text(message_state_t *state, char *buffer, int length);
int message_state_load_res(message_state_t *state, int module);
void message_state_initialize(resource_mgr_t *resmgr, message_state_t *state);


