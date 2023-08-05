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

#include "m4/adv_r/conv_io.h"
#include "m4/adv_r/conv.h"
#include "m4/adv_r/chunk_ops.h"
#include "m4/adv_r/db_env.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/vars.h"

namespace M4 {

#define HIDDEN		    0x00000004
#define DESTROYED       0x00000008

#define INITIAL         1
#define PERSISTENT      2

#define CONV_OK          0
#define CONV_QUIT       -1
#define CONV_NEW        -2
#define CONV_BAIL       -3

#define CONV_UNKNOWN_MODE   0
#define CONV_GET_TEXT_MODE  1
#define CONV_SET_TEXT_MODE  2
#define CONV_GET_MESG_MODE  3
#define CONV_UPDATE_MODE    4

#define DECL_POINTER	1

void Converstation_Globals::syncGame(Common::Serializer &s) {
	uint32 val;

	conv_reset_all();

	// Handle size
	val = conv_save_buff.size();
	s.syncAsUint32LE(val);
	if (s.isLoading())
		conv_save_buff.resize(val);

	// Read in the buffer
	if (val)
		s.syncBytes(&conv_save_buff[0], val);
}

void Converstation_Globals::conv_reset_all() {
	conv_save_buff.clear();
}

/*------------------------------------------------------------------------*/

void cdd_init(void) {
	int i;

	for (i = 0; i < 16; i++) {
		_G(cdd).text[i] = nullptr;
		_G(cdd).snd_files[i] = nullptr;
	}

	_G(cdd).num_txt_ents = 0;
	Common::strcpy_s(_G(cdd).mesg, "");
	_G(cdd).mesg_snd_file = nullptr;
}

void set_conv_name(char *s) {
	Common::strcpy_s(_GC(conv_name), s);
}

char *get_conv_name(void) {
	return _GC(conv_name);
}

Conv *conv_get_handle(void) {
	return _GC(globConv);
}

void conv_set_handle(Conv *c) {
	_GC(globConv) = c;
}

void conv_resume(Conv *c) {
	conv_go(c);
}

int conv_is_event_ready(void) {
	return _GC(event_ready);
}

void conv_set_event(int e) {
	_GC(event) = e;
	_GC(event_ready) = 1;
}

int conv_get_event(void) {
	_GC(event_ready) = 0;
	return _GC(event);
}

void conv_play(Conv *c) {
	conv_go(c);
}

long conv_current_node(void) {
	if (conv_get_handle())
		return conv_get_handle()->node_hash;
	return 0;
}

void conv_reset(char *filename) {
	Conv *c = nullptr;
	_GC(restore_conv) = 0;

	c = conv_load(filename, 1, 1, -1, false);
	conv_unload(c);
}


void conv_reset_all(void) {
	_G(conversations).conv_reset_all();
}

char *conv_sound_to_play(void) {
	return _G(cdd).mesg_snd_file;
}

int32 conv_whos_talking(void) {
	return _G(cdd).player_non_player;
}

int ok_status(entry_chunk *entry) {
	if (entry->status & DESTROYED)
		return 0;

	if (entry->status & HIDDEN)
		return 0;

	return 1;
}

int conv_toggle_flags(entry_chunk *entry) {
	if (ok_status(entry))
		return (entry->status & 0x0000000e); //mask off INITIAL bit.
	return entry->status;
}

long conv_get_decl_val(decl_chunk *decl) {
	switch (decl->flags) {
	case DECL_POINTER:
		return *decl->addr;

	default:
		return decl->val;
	}
}

void conv_set_decl_val(decl_chunk *decl, long val) {
	switch (decl->flags) {
	case DECL_POINTER:
		decl->val = val;
		*decl->addr = val;
		break;

	default:
		decl->val = val;
		break;
	}
}

void conv_export_value(Conv *c, long val, int index) {
	long			ent = 0, tag = 0, next;
	decl_chunk *decl;
	long			ent_old = 0;
	int			i = 0;

	if (!c)
		return;

	ent_old = c->myCNode;
	ent = 0;
	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case DECL_CHUNK:
			if (i == index) {
				decl = get_decl(c, ent);
				conv_set_decl_val(decl, val);
			}
			i++;
			break;

		default:
			break;
		}
		ent = next;
	}
	c->myCNode = ent_old;
}

void conv_export_pointer(Conv *c, long *val, int index) {
	long			ent = 0, tag = 0, next;
	decl_chunk *decl;
	long			ent_old = 0;
	int			i = 0;

	if (!c)
		return;

	ent_old = c->myCNode;
	ent = 0;
	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case DECL_CHUNK:
			if (i == index) {
				decl = get_decl(c, ent);
				decl->addr = val;
				decl->flags = DECL_POINTER;
			}
			i++;
			break;

		default:
			break;
		}
		ent = next;
	}
	c->myCNode = ent_old;
}

void conv_init(Conv *c) {
	switch (c->exit_now) {
	case CONV_OK:
		break;

	case CONV_QUIT:
		break;

	case CONV_BAIL:
	case CONV_NEW:
		if (c->myCNode != CONV_QUIT) {
			c->exit_now = CONV_NEW; //conv hasn't been run before. only done here once.
			c->myCNode = 0;
		}
		break;
	}
}

#ifdef TODO
static long find_state(char *s, char *c, int file_size) {
	char	name[9];
	long	size, offset = 0;

	////fprintf( conv_fp, "find_state %s\n", s );
	while (offset < file_size) {
		cstrncpy(name, &c[offset], 8);
		////fprintf( conv_fp, "name '%s' offset %d\n", name, offset );
		name[8] = '\0';

		if (!scumm_strnicmp(name, s, 8)) {
			offset += 8 * sizeof(char);
			////fprintf( conv_fp, "state found\n" );
			goto handled;
		}

		offset += 8 * sizeof(char);
		if (offset < file_size) {
			memcpy(&size, &c[offset], sizeof(long));
			////fprintf( conv_fp, "size %x\n", size );
		}

		offset += size + sizeof(long);
	}

	offset = -1;

	////fprintf( conv_fp, "state not found\n" );
handled:
	return offset;
}
#endif

void find_and_set_conv_name(Conv *c) {
	long			ent = 0, tag = 0, next = 0;
	conv_chunk *conv;

	c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case CONV_CHUNK:
			conv = get_conv(c, ent);
			set_conv_name(get_string(c, c->myCNode + ent + sizeof(conv_chunk)));
			break;

		default:
			break;
		}
		ent = next;
	}
}

/*
void save_state( Conv *c )
{
	long			ent=0;
	long			tag, next;
	long			offset=-1, size=0, amt_to_write=0;

	entry_chunk		*entry;
	decl_chunk		*decl;
	conv_chunk		*conv;
	short				num_decls=0, num_entries=0;
	short				flag_num=0, flag_index=0;
	long				e_flags=0, myCNode, val;

	FILE				*fp=nullptr;
	char				fname[9];
	char				*conv_save_buff=nullptr;
	long				file_size;
	bool				overwrite_file=false;
	long				prev_size=0;

	myCNode = c->myCNode; c->myCNode=0; ent=0;

	while( ent < c->chunkSize ) {
		conv_ops_get_entry( ent, &next, &tag, c );

		switch( tag ) {
			case CONV_CHUNK:
				conv = get_conv( c, ent );
				cstrncpy( fname, get_string( c, c->myCNode+ent+sizeof( conv_chunk ) ), 8 );
				fname[8] = '\0';
			break;

			case DECL_CHUNK:
				num_decls++;
				amt_to_write += sizeof( long );
			break;

			case ENTRY_CHUNK:
				num_entries++;
			break;

			default:
			break;
		}
		ent = next;
	}

	sprintf( _GC(conv_file_name), "%sconvsave.dat", argv );
	//file_size = f_info_get_file_size( _GC(conv_file_name) ); //oct11

	//oct11
	if( !f_info_exists(_GC(conv_file_name)) )
		file_size = 0; //was -1
	else
		file_size = f_info_get_file_size( _GC(conv_file_name) );

	amt_to_write += 3*sizeof( long );
	amt_to_write += (num_entries / 8) * sizeof( long );
	if( (num_entries%8) != 0 )
		amt_to_write += sizeof( long );

	fp = f_io_open( _GC(conv_file_name), "rb" ); //was r+b

	if( fp ) {
		conv_save_buff = (char *)mem_alloc( file_size, "conv save buff" );
		fread( conv_save_buff, file_size, 1, fp );

		f_io_close( fp );
		offset = find_state( fname, conv_save_buff, file_size );

		if( offset != -1 ) {
			overwrite_file=true;
			memcpy( &prev_size, &conv_save_buff[offset], sizeof( long ) );
			prev_size += 3*sizeof(long);
			offset += sizeof( long ); //skip header. (name + size)
		} else {
			//append!!!
			offset=0;
			if( conv_save_buff )
				mem_free( conv_save_buff );

			conv_save_buff = (char *)mem_alloc( amt_to_write+3*sizeof(long), "conv save buff" );
			memcpy( &conv_save_buff[offset], fname, 8*sizeof(char) );
			offset += 8*sizeof( char );
			memcpy( &conv_save_buff[offset], &amt_to_write, sizeof( long ));
			offset += sizeof( long );
		}
	} else {
		offset=0;
		conv_save_buff = (char *)mem_alloc( amt_to_write+3*sizeof(long), "conv save buff" );
		memcpy( &conv_save_buff[offset], fname, 8*sizeof(char) );
		offset += 8*sizeof( char );
		memcpy( &conv_save_buff[offset], &amt_to_write, sizeof( long ));
		offset += sizeof( long );
	}

	memcpy( &conv_save_buff[offset], &myCNode, sizeof( long ) );
	offset += sizeof( long );

	memcpy( &conv_save_buff[offset], &num_decls, sizeof( long ) );
	offset += sizeof( long );

	memcpy( &conv_save_buff[offset], &num_entries, sizeof( long ) );
	offset += sizeof( long );
	size += 3*sizeof( long );

	ent=0; c->myCNode = 0;
	while( ent < c->chunkSize ) {
		conv_ops_get_entry( ent, &next, &tag, c );

		switch( tag ) {
			case DECL_CHUNK:
				decl = get_decl( c, ent );
				val = conv_get_decl_val( decl );

				memcpy( &conv_save_buff[offset], &val, sizeof( long ) );
				offset+=sizeof( long );

				size+=sizeof( long );
			break;

			case LNODE_CHUNK:
			case NODE_CHUNK:
			break;

			case ENTRY_CHUNK:
				entry = get_entry( c, ent );

				if( flag_index == 32 ) {
					flag_index = 0;
					flag_num++;

					memcpy( &conv_save_buff[offset], &e_flags, sizeof( long ) );
					offset += sizeof( long );
					size += sizeof( long );

					e_flags = 0;
				}

				//fprintf( conv_fp, "entry->status %d\n", entry->status );
				e_flags |= ( (entry->status & 0x0000000f) << flag_index );

				flag_index += 4;
			break;

			default:
			break;
		}
		ent = next;
	}

	if( flag_index != 0 ) {
		memcpy( &conv_save_buff[offset], &e_flags, sizeof(long) );
		offset += sizeof( long );
		size += sizeof( long );
	}

	if( (amt_to_write != size) )
		error_show( FL, 'CNVS', "save_state: error! size written != size (%d %d)", amt_to_write, size );

	if( overwrite_file == true ) {
		////fprintf( conv_fp, "overwrite conversation\n" );
		fp = f_io_open( _GC(conv_file_name), "wb" );
		fwrite( conv_save_buff, file_size, 1, fp );

		//if( prev_size != file_size )
		//	error_show( FL, 'CNVS', "save_state() Prev save size != Curr save size (%d, %d)", prev_size, file_size );

	} else {
		////fprintf( conv_fp, "append conversation\n" );
		fp = f_io_open( _GC(conv_file_name), "ab+" );
		fwrite( conv_save_buff, amt_to_write+3*sizeof(long), 1, fp );
	}

	if( conv_save_buff )
		mem_free( conv_save_buff );

	f_io_close( fp );
}
*/

//-------------------------------------------------------------------------------
// nick documented and error checking added 960501
// variables moved into their scopes because I'm a picky S.O.B.

void save_state(Conv *c) {
#ifndef TODO
	error("TODO: save_state");
#else
	//-------------------------------------------------------------------------------
	// calculate amt_to_write by counting up the size of DECL_CHUNKs.
	// the number of ENTRY_CHUNKs affects the amt_to_write
	// also extract fname from the CONV_CHUNK

	long amt_to_write = 3 * sizeof(long);	// mystery padding
	long ent = 0;
	long next, tag;	// receive conv_ops_get_entry results
	long myCNode = c->myCNode;
	char fname[9];

	short num_decls = 0;
	short num_entries = 0;

	c->myCNode = 0;

	while (ent < c->chunkSize)
	{
		conv_chunk *conv;	// declared here for the benefit of Watcom 10.0's initializers.
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case CONV_CHUNK:
			conv = get_conv(c, ent);
			cstrncpy(fname, get_string(c, c->myCNode + ent + sizeof(conv_chunk)), 8);
			fname[8] = '\0';
			break;

		case DECL_CHUNK:
			num_decls++;
			amt_to_write += sizeof(long);
			break;

		case ENTRY_CHUNK:
			num_entries++;
			break;

		default:
			break;
		}
		ent = next;
	}

	amt_to_write += (num_entries / 8) * sizeof(long);
	if ((num_entries % 8) != 0)
		amt_to_write += sizeof(long);	// pad the sucker

	//-------------------------------------------------------------------------------
	// get the name of consave.dat

	Common::strcpy_s(_GC(conv_file_name), "convsave.dat");

	//-------------------------------------------------------------------------------
	// if consave.dat exists, read it in

	long file_size = 0;
	long offset = -1;
	long prev_size = 0;
	char *conv_save_buff = nullptr;
	bool overwrite_file = false;

	if (f_info_exists(_GC(conv_file_name))) {
		file_size = f_info_get_file_size(_GC(conv_file_name));
		FILE *fp = f_io_open(_GC(conv_file_name), "rb"); //was r+b

		conv_save_buff = (char *)mem_alloc(file_size, "conv save buff");
		if (!conv_save_buff)
			error_show(FL, 'OOM!');

		fread(conv_save_buff, file_size, 1, fp);

		f_io_close(fp);

		//----------------------------------------------------------------------------
		// if this conversation already in save file, overwrite it,
		// otherwise chuck out the buffer, and create a new buffer which is just
		// big enough to hold the new save data.

		offset = find_state(fname, conv_save_buff, file_size);

		if (offset != -1) {
			overwrite_file = true;
			memcpy(&prev_size, &conv_save_buff[offset], sizeof(long));
			prev_size += 3 * sizeof(long);
			offset += sizeof(long); //skip header. (name + size)
		} else {
			//append!!!
			offset = 0;

			if (conv_save_buff)
				mem_free(conv_save_buff);

			conv_save_buff = (char *)mem_alloc(amt_to_write + 3 * sizeof(long), "conv save buff");
			if (!conv_save_buff)
				error_show(FL, 'OOM!');

			memcpy(&conv_save_buff[offset], fname, 8 * sizeof(char));
			offset += 8 * sizeof(char);
			memcpy(&conv_save_buff[offset], &amt_to_write, sizeof(long));
			offset += sizeof(long);
		}
	} else
	{
		//----------------------------------------------------------------------------
		// convsav.dat didn't exist, so we set things up for a create here.

		offset = 0;

		conv_save_buff = (char *)mem_alloc(amt_to_write + 3 * sizeof(long), "conv save buff");
		if (!conv_save_buff)
			error_show(FL, 'OOM!');

		memcpy(&conv_save_buff[offset], fname, 8 * sizeof(char));
		offset += 8 * sizeof(char);
		memcpy(&conv_save_buff[offset], &amt_to_write, sizeof(long));
		offset += sizeof(long);
	}

	//----------------------------------------------------------------------------
	// finish filling in conv_save_buff data with num of entries etc.

	memcpy(&conv_save_buff[offset], &myCNode, sizeof(long));
	offset += sizeof(long);

	memcpy(&conv_save_buff[offset], &num_decls, sizeof(long));
	offset += sizeof(long);

	memcpy(&conv_save_buff[offset], &num_entries, sizeof(long));
	offset += sizeof(long);

	long size = 3 * sizeof(long);

	// fill in all the entries themselves

	long e_flags = 0;
	short flag_index = 0;
	//	short flag_num = 0;

	ent = 0;
	c->myCNode = 0;

	long val = 0;
	entry_chunk *entry = nullptr;

	while (ent < c->chunkSize)
	{
		conv_ops_get_entry(ent, &next, &tag, c);
		decl_chunk *decl; 	// declared here for the benefit of Watcom 10.0 not liking to scope things into switches

		switch (tag) {
		case DECL_CHUNK:
			decl = get_decl(c, ent);
			val = conv_get_decl_val(decl);

			memcpy(&conv_save_buff[offset], &val, sizeof(long));
			offset += sizeof(long);

			size += sizeof(long);
			break;

		case LNODE_CHUNK:
		case NODE_CHUNK:
			break;

		case ENTRY_CHUNK:
			entry = get_entry(c, ent);

			if (flag_index == 32) {
				flag_index = 0;
				//flag_num++;

				memcpy(&conv_save_buff[offset], &e_flags, sizeof(long));
				offset += sizeof(long);
				size += sizeof(long);

				e_flags = 0;
			}

			//fprintf( conv_fp, "entry->status %d\n", entry->status );
			e_flags |= ((entry->status & 0x0000000f) << flag_index);

			flag_index += 4;
			break;

		default:
			break;
		}
		ent = next;
	}

	// copy the flags

	if (flag_index != 0) {
		memcpy(&conv_save_buff[offset], &e_flags, sizeof(long));
		offset += sizeof(long);
		size += sizeof(long);
	}

	if ((amt_to_write != size))
		error_show(FL, 'CNVS', "save_state: error! size written != size (%d %d)", amt_to_write, size);

	// finally, write out the conversation data. Scoped to contain fp.
	{
		FILE *fp = nullptr;
		if (overwrite_file == true)
		{
			////fprintf( conv_fp, "overwrite conversation\n" );
			fp = f_io_open(_GC(conv_file_name), "wb");
			if (!fp)
				error_show(FL, 'FNF!', "consave.dat");

			fwrite(conv_save_buff, file_size, 1, fp);
		} else
		{
			////fprintf( conv_fp, "append conversation\n" );
			fp = f_io_open(_GC(conv_file_name), "ab+");
			if (!fp)
				error_show(FL, 'FNF!', "consave.dat");

			fwrite(conv_save_buff, amt_to_write + 3 * sizeof(long), 1, fp);
		}

		if (conv_save_buff)
			mem_free(conv_save_buff);

		f_io_close(fp);
	}
#endif
}

Conv *restore_state(Conv *c) {
#ifndef TODO
	error("TODO: restore_state");
#else
	long ent = 0;
	long tag, next, offset;

	entry_chunk *entry;
	decl_chunk *decl;

	long num_decls = 0, num_entries = 0;
	short flag_num = 0, flag_index = 0;
	long val;
	long e_flags = 0;
	long myCNode;

	FILE *fp = nullptr;
	char fname[9];
	int dont_update_ents = 0;
	int file_size = 0;
	char *conv_save_buff = nullptr;

	ent = 0; c->myCNode = 0;

	find_and_set_conv_name(c);
	cstrncpy(fname, get_conv_name(), 8);
	fname[8] = '\0';

	sprintf(_GC(conv_file_name), "%sconvsave.dat", argv);
	//oct11
	//file_size = f_info_get_file_size( _GC(conv_file_name) );
	if (!f_info_exists(_GC(conv_file_name)))
		file_size = -1;
	else
		file_size = f_info_get_file_size(_GC(conv_file_name));

	fp = f_io_open(_GC(conv_file_name), "rb");
	if (!fp)
	{
		conv_init(c);
		return c;
	}

	conv_save_buff = (char *)mem_alloc(file_size, "conv save buff");
	if (!conv_save_buff)
		error_show(FL, 'OOM!');

	// ------------------

	fread(conv_save_buff, file_size, 1, fp);
	f_io_close(fp);
	offset = find_state(fname, conv_save_buff, file_size);

	// nick 960501 changed from a return c to a goto in order to corret an insidious memory leak!
	if (offset == -1)
		goto i_am_so_done;

	//skip header.
	offset += sizeof(long);

	memcpy(&myCNode, &conv_save_buff[offset], sizeof(long));
	offset += sizeof(long);

	memcpy(&num_decls, &conv_save_buff[offset], sizeof(long));
	offset += sizeof(long);

	memcpy(&num_entries, &conv_save_buff[offset], sizeof(long));
	offset += sizeof(long);

	ent = 0; c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case DECL_CHUNK:
			memcpy(&val, &conv_save_buff[offset], sizeof(long));
			offset += sizeof(long);
			decl = get_decl(c, ent);

			conv_set_decl_val(decl, val);
			break;

		default:
			break;
		}
		ent = next;
	}

	ent = 0; c->myCNode = 0;

	while (ent < c->chunkSize) {
		conv_ops_get_entry(ent, &next, &tag, c);

		switch (tag) {
		case LNODE_CHUNK:
			if (myCNode == ent) {
				dont_update_ents = 1;
			} else {
				dont_update_ents = 0;
			}
			break;

		case NODE_CHUNK:
			dont_update_ents = 0;
			break;

		case ENTRY_CHUNK:
			entry = get_entry(c, ent);

			if (flag_index == 32) {
				flag_index = 0;
				flag_num++;
			}

			if (flag_index == 0) {
				memcpy(&e_flags, &conv_save_buff[offset], sizeof(long));
				offset += sizeof(long);
			}

			val = (e_flags >> flag_index) & 0x0000000f;
			entry->status = val;
			//fprintf( conv_fp, "entry->status %d\n", entry->status );

			flag_index += 4;
			break;

		default:
			break;
		}
		ent = next;
	}

	c->myCNode = myCNode;
	if (c->myCNode == CONV_QUIT) {
		c->exit_now = CONV_QUIT;
		////fprintf( conv_fp, "c->myCNode == CONV_QUIT conv_unload()\n" );
		conv_unload(c);
		c = nullptr;
	} else c->exit_now = CONV_OK;

i_am_so_done:
	if (conv_save_buff)
		mem_free(conv_save_buff);
	return c;
#endif
}

void conv_set_font_spacing(int32 h, int32 v) {
	_GC(conv_font_spacing_h) = h;
	_GC(conv_font_spacing_v) = v;
}


void conv_set_text_colours(int32 norm_colour, int32 norm_colour_alt1, int32 norm_colour_alt2,
	int32 hi_colour, int32 hi_colour_alt1, int32 hi_colour_alt2) {
	_GC(conv_normal_colour) = norm_colour;
	_GC(conv_normal_colour_alt1) = norm_colour_alt1;
	_GC(conv_normal_colour_alt2) = norm_colour_alt2;
	_GC(conv_hilite_colour) = hi_colour;
	_GC(conv_hilite_colour_alt1) = hi_colour_alt1;
	_GC(conv_hilite_colour_alt2) = hi_colour_alt2;
}

void conv_set_text_colour(int32 norm_colour, int32 hi_colour) {
	conv_set_text_colours(norm_colour, norm_colour, norm_colour, hi_colour, hi_colour, hi_colour);
}

void conv_set_default_hv(int32 h, int32 v) {
	_GC(conv_default_h) = h;
	_GC(conv_default_v) = v;
}

void conv_set_default_text_colour(int32 norm_colour, int32 hi_colour) {
	conv_set_text_colours(norm_colour, norm_colour, norm_colour, hi_colour, hi_colour, hi_colour);
}

void conv_set_shading(int32 shade) {
	_GC(conv_shading) = shade;
}

void conv_set_box_xy(int32 x, int32 y) {
	_GC(glob_x) = x;
	_GC(glob_y) = y;
}

static void conv_set_disp_default(void) {
	_GC(conv_font_spacing_h) = _GC(conv_default_h);
	_GC(conv_font_spacing_v) = _GC(conv_default_v);
	_GC(conv_normal_colour) = _GC(conv_default_normal_colour);
	_GC(conv_hilite_colour) = _GC(conv_default_hilite_colour);
	_GC(conv_shading) = 75;
}

Conv *conv_load(char *filename, int x1, int y1, int32 myTrigger, bool want_box) {
	Conv *convers = nullptr;
	long cSize = 0;
	char fullpathname[MAX_FILENAME_SIZE];
	void *bufferHandle;

	term_message("conv_load");

	_GC(playerCommAllowed) = _G(player).comm_allowed;	// Remember if player commands are on before we start the conversation
	_GC(interface_was_visible) = INTERFACE_VISIBLE;

	term_message("conv load:   %s", filename);

	if (want_box) {
		// If we want an interface box
		conv_set_disp_default();
		mouse_set_sprite(0);					// Also if we want a text box, lock the mouse into arrow mode
		mouse_lock_sprite(0);
		player_set_commands_allowed(false);		// with commands off

		// Hide the interface if it's visible
		if (INTERFACE_VISIBLE)
			interface_hide();
	}

	// if not in rooms.db, use actual filename
	char *str = env_find(filename);
	if (str)
		Common::strcpy_s(fullpathname, str);
	else
		Common::sprintf_s(fullpathname, "%s.chk", filename);

	SysFile fp(fullpathname, BINARY);
	if (!fp.exists()) {
		// Force the file open
		error_show(FL, 'CNVL', "couldn't conv_load %s", fullpathname);
		conv_set_handle(nullptr);
		convers = nullptr;
		goto done;
	}

	cSize = fp.size();

	if (conv_get_handle() != nullptr) {
		////fprintf( conv_fp, "conv_get_handle != nullptr conv_unload()\n" );
		conv_unload(conv_get_handle());
	}

	convers = (Conv *)mem_alloc(sizeof(Conv), "Conv struct");

	if (!convers) {
		conv_set_handle(nullptr);
		convers = nullptr;
		goto done;
	}

	convers->chunkSize = cSize;
	convers->conv = nullptr;
	convers->myCNode = 0;
	convers->exit_now = CONV_NEW;
	convers->node_hash = 0;
	convers->mode = CONV_GET_TEXT_MODE;
	convers->c_entry_num = 1;
	_GC(myFinalTrigger) = kernel_trigger_create(myTrigger);

	convers->conv = (char *)mem_alloc(cSize * sizeof(char), "conv char data");

	bufferHandle = convers->conv;
	if (!fp.read((MemHandle)&bufferHandle, cSize)) {
		conv_set_handle(nullptr);
		if (convers)
			mem_free(convers);

		convers = nullptr;
		goto done;
	}

	conv_swap_words(convers); //oct10
	find_and_set_conv_name(convers);

	_GC(glob_x) = x1;
	_GC(glob_y) = y1;

	if (want_box)
		set_dlg_rect();

	if (_GC(restore_conv))
		convers = restore_state(convers);
	_GC(restore_conv) = 1;

	conv_set_handle(convers);

done:
	fp.close();

	return convers;
}

void conv_unload(Conv *c) {
	mouse_unlock_sprite();

	if (_GC(interface_was_visible)) {	// Turn interface back on if it was on
		interface_show();
	}

	_GC(globConv) = nullptr;

	if (c)
		save_state(c);

	player_set_commands_allowed(_GC(playerCommAllowed));

	_G(player).command_ready = false;
	_G(player).ready_to_walk = false;
	_G(player).need_to_walk = false;

	Common::strcpy_s(_G(player).verb, "");
	Common::strcpy_s(_G(player).noun, "");
	kernel_trigger_dispatch(_GC(myFinalTrigger));

	if (c) {
		if (c->conv)
			mem_free(c->conv);
		mem_free(c);
	}

	_GC(globConv) = c = nullptr;
}

// only called if node is visible.
// gets the TEXT chunks inside a node.
int conv_get_text(long offset, long size, Conv *c) {
	long	i = offset, tag, next, text_len, text_width;
	text_chunk *text;
	int	result = 0;

	size -= sizeof(entry_chunk);

	while (i < offset + size) {
		conv_ops_get_entry(i, &next, &tag, c);

		switch (tag) {
		case TEXT_CHUNK:
			result = 1;
			text = get_text(c, i);
			text_len = conv_ops_text_strlen(get_string(c, c->myCNode + i + sizeof(text_chunk)));
			_G(cdd).snd_files[_G(cdd).num_txt_ents] = get_string(c, c->myCNode + i + sizeof(text_chunk));
			_G(cdd).text[_G(cdd).num_txt_ents] = get_string(c, c->myCNode + i + sizeof(text_chunk) + text_len);

			text_width = gr_font_string_width(_G(cdd).text[_G(cdd).num_txt_ents], 1);
			if (text_width > _GC(width))
				_GC(width) = text_width;

			_G(cdd).num_txt_ents++;
			break;
		}
		i = next;
	}
	return result;
}

} // End of namespace M4
