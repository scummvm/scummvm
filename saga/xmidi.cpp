/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    XMIDI conversion routines

 Notes: 
 
    Code adapted from XMILoader by Keet ( fox@foxpaws.net ), (C)2000
*/

#include "reinherit.h"

#include "yslib.h"

/* Begin module component
\*--------------------------------------------------------------------------*/
#include "xmidi_mod.h"
#include "xmidi.h"

namespace Saga {

int
ConvertEventListToSMF(XMIDIEVENT_LIST * event_list,
    uchar ** smf_ptr, size_t * smf_len)
/*--------------------------------------------------------------------------*\
 * Given a pointer to an event list structure, this function creates and 
 * returns a pointer to a Standard Midi File (SMF) image and the image's
 * length in bytes.
\*--------------------------------------------------------------------------*/
{
	YS_IGNORE_PARAM(event_list);
	YS_IGNORE_PARAM(smf_ptr);
	YS_IGNORE_PARAM(smf_len);

#if 0
	SMF_HEADER_CHUNK smfh;
	SMF_TRACK_CHUNK smft;

	XMIDIEVENT *event_p;

	uchar *write_p = NULL;
	uchar *smf_buf = NULL;

	size_t alloc_size;

	int vlq_len;

	if ((smf_ptr == NULL) || (smf_len == NULL)) {

		return R_FAILURE;
	}

	/* Allocate memory for SMF image
	 * \*---------------------------------------------------------------------- */
	alloc_size = event_list->smf_size + MIDI_HEADER_LEN +
	    MIDI_TRACK_CHUNK_LEN;

	/* SMF requires an even size */
	if (alloc_size % 2) {
		alloc_size++;
	}

	smf_buf = malloc(alloc_size);
	if (smf_buf == NULL) {
		R_printf(R_STDERR, "Memory allocation error.\n");

		return R_FAILURE;
	}

	memset(smf_ptr, 0, alloc_size);

	/* Write header chunk
	 * \*---------------------------------------------------------------------- */
	write_p = smf_buf;

	smfh.smf_header_len = MIDI_HEADER_CHUNK_LEN;
	smfh.smf_format = 0;
	smfh.smf_ntracks = 1;
	smfh.time_division.ppqn = XMIDI_TIMEDIV;

	memcpy(write_p, MIDI_HEADER_TAG, 4);
	write_p += 4;

	ys_write_u32_be(smfh.smf_header_len, write_p, &write_p);
	ys_write_u16_be(smfh.smf_format, write_p, &write_p);
	ys_write_u16_be(smfh.smf_ntracks, write_p, &write_p);
	ys_write_u16_be(smfh.time_division.ppqn, write_p, &write_p);

	/* Write track chunk
	 * \*---------------------------------------------------------------------- */
	memcpy(write_p, MIDI_TRACK_TAG, 4);
	write_p += 4;

	smft.smf_track_len = event_list->smf_size;

	ys_write_u32_be(smft.smf_track_len, write_p, &write_p);

	/* Write MIDI events
	 * \*---------------------------------------------------------------------- */
	event_p = event_list->head;

	while (event_p != NULL) {

		vlq_len = WriteVLQ_DW(write_p, event_p->delta_time);
		write_p += vlq_len;

		/*
		 * R_printf( R_STDOUT, 
		 * "Wrote %d len VLQ. (%d)\n",
		 * vlq_len,
		 * event_p->delta_time );
		 */

		switch (event_p->event) {

		case MIDI_NOTE_ON:
		case MIDI_NOTE_OFF:
		case MIDI_AFTERTOUCH:
		case MIDI_CONTROLCHANGE:
		case MIDI_PITCHWHEEL:

			*write_p++ =
			    (uchar) (event_p->event | (uchar) event_p->
			    channel);
			*write_p++ = event_p->op1;
			*write_p++ = event_p->op2;
			break;

		case MIDI_PROGRAMCHANGE:
		case MIDI_CHANNELPRESSURE:

			*write_p =
			    (uchar) (event_p->event | (uchar) event_p->
			    channel);
			*write_p = event_p->op1;
			break;

		case MIDI_SYSTEMEXCLUSIVE:

			*write_p = (uchar) MIDI_NONMIDI;

			switch (event_p->sysex_op) {

			case MIDI_SYSEX_TRACKEND:

				*write_p++ = event_p->sysex_op;
				*write_p++ = (uchar) 0;
				break;

			case MIDI_SYSEX_TEMPO:

				*write_p++ = event_p->sysex_op;
				*write_p++ = (uchar) 3;
				/*
				 *write_p++ = event_p->op1;
				 *write_p++ = event_p->op2;
				 *write_p++ = event_p->op3;
				 */

				/* Override tempo change */
				*write_p++ = (uchar) 0x07;
				*write_p++ = (uchar) 0xA1;
				*write_p++ = (uchar) 0x20;
				break;

			case MIDI_SYSEX_TIMESIG:

				*write_p++ = event_p->sysex_op;
				*write_p++ = (uchar) 4;
				*write_p++ = event_p->op1;
				*write_p++ = event_p->op2;
				*write_p++ = event_p->op3;
				*write_p++ = event_p->op4;
				break;

			default:

				R_printf(R_STDERR,
				    "Error, invalid sysex event type (%d): "
				    "Aborting.\n", event_p->sysex_op);

				return R_FAILURE;
				break;

			}
			break;

		default:
			R_printf(R_STDERR,
			    "Invalid event code encountered; " "aborting.\n");

			return R_FAILURE;
			break;
		}

		event_p = event_p->next_event;
	}

	*smf_ptr = smf_buf;
	*smf_len = alloc_size;
#endif
	return R_SUCCESS;
}

int WriteVLQ_DW(char *write_ptr, DWORD value)
{

	int vlq_len = 1;
	DWORD pack = value & 0x7F;
	int x;

	while (value >>= 7) {
		pack <<= 8;
		pack |= ((value & 0x7F) | 0x80);
		vlq_len++;
	}
	for (x = 0; x < sizeof(DWORD); x++) {
		*write_ptr++ = ((char *)(&pack))[x];
	}

	return vlq_len;
}

int XMIDI_Read(const uchar * XMI_img, XMIDIEVENT_LIST * event_list)
{
	/* XMI header data */
	const uchar *XMIDI_data;
	uint n_tracks;

	/* XMIDI data */
	IFF_ID_CHUNK cat_chunk;
	IFF_ID_CHUNK id_chunk;	/* Present after categeory chunk */
	XMI_TIMB_CHUNK timbre_chunk;	/* Present after id chunk */
	XMI_EVENT_CHUNK event_chunk;

	const uchar *read_p;

	const uchar *event_data;
	size_t event_data_len;

	if (XMIDI_ReadXMIHeader(XMI_img, &XMIDI_data, &n_tracks) != R_SUCCESS) {

		return R_FAILURE;
	}

	read_p = XMIDI_data;

	/* Read category chunk
	 * \*------------------------------------------------------------- */
	ReadIFF_IDChunk(&cat_chunk, read_p, &read_p);

	if (memcmp(cat_chunk.id_4cc, IFF_CATEGORY_4CC, 4) != 0) {

		R_printf(R_STDERR, "Error: Category chunk not present.\n");
		Print4CC(cat_chunk.id_4cc);

		return R_FAILURE;
	}

	if (memcmp(cat_chunk.desc_4cc, XMIDI_DESC_4CC, 4) != 0) {

		R_printf(R_STDERR,
		    "Error: Incorrect category description field.\n");
		Print4CC(cat_chunk.desc_4cc);

		return R_FAILURE;
	}

	/* Read XMIDI ID Chunk
	 * \*------------------------------------------------------------- */
	ReadIFF_IDChunk(&id_chunk, read_p, &read_p);

	if (memcmp(id_chunk.id_4cc, IFF_FORMAT_4CC, 4) != 0) {

		R_printf(R_STDERR, "Error: ID chunk not present.\n");
		Print4CC(id_chunk.id_4cc);

		return R_FAILURE;
	}

	if (memcmp(id_chunk.desc_4cc, XMIDI_DESC_4CC, 4) != 0) {

		R_printf(R_STDERR,
		    "Error: XMID tag not present in ID chunk: "
		    "Not XMIDI data.\n");
		Print4CC(id_chunk.desc_4cc);

		return R_FAILURE;
	}

	/* Read XMIDI Timbre Chunk
	 * \*------------------------------------------------------------- */
	ys_read_4cc(timbre_chunk.id_4cc, read_p, &read_p);
	timbre_chunk.chunk_len = ys_read_u32_be(read_p, &read_p);

	if (memcmp(timbre_chunk.id_4cc, XMIDI_TIMBRE_4CC, 4) != 0) {

		R_printf(R_STDERR, "Error: Timbre chunk not present.\n");
		Print4CC(timbre_chunk.id_4cc);

		return R_FAILURE;
	}

	/* Read XMIDI Event Chunk
	 * \*------------------------------------------------------------- */
	read_p += timbre_chunk.chunk_len;

	ys_read_4cc(event_chunk.id_4cc, read_p, &read_p);
	event_chunk.chunk_len = ys_read_u32_be(read_p, &read_p);

	if (memcmp(event_chunk.id_4cc, XMIDI_EVENT_4CC, 4) != 0) {

		R_printf(R_STDERR, "Error: Event chunk not present.\n");
		Print4CC(event_chunk.id_4cc);

		return R_FAILURE;
	}

	/* Read XMIDI Event data
	 * \*------------------------------------------------------------- */
	event_data = read_p;
	event_data_len = event_chunk.chunk_len;

	R_printf(R_STDOUT,
	    "Converting %d bytes of event data:\n", event_data_len);

	XMIDI_ReadEvents(event_list, event_data, event_data_len, n_tracks);

	/* Process XMIDI Event data
	 * \*------------------------------------------------------------- */
	ProcessEventList(event_list);

	return R_SUCCESS;
}

int
ReadIFF_IDChunk(IFF_ID_CHUNK * chunk,
    const uchar * read_p, const uchar ** read_pp)
{
	const uchar *chunk_p = read_p;

	ys_read_4cc(chunk->id_4cc, chunk_p, &chunk_p);

	chunk->chunk_len = ys_read_u32_be(chunk_p, &chunk_p);

	ys_read_4cc(chunk->desc_4cc, chunk_p, &chunk_p);

	if (read_pp != NULL) {
		*read_pp = chunk_p;
	}

	return R_SUCCESS;
}

int Print4CC(char *fourcc)
{
	R_printf(R_STDERR,
	    "FourCC: %c%c%c%c (%X %X %X %X)\n",
	    fourcc[0], fourcc[1], fourcc[2], fourcc[3],
	    fourcc[0], fourcc[1], fourcc[2], fourcc[3]);

	return R_SUCCESS;
}

int
XMIDI_ReadXMIHeader(const uchar * XMI_img,
    const uchar ** XMIDI_data, uint * n_tracks)
{
	const uchar *read_p;

	IFF_ID_CHUNK id_chunk;
	XMI_INFO_CHUNK info_chunk;

	*n_tracks = 0;
	*XMIDI_data = NULL;

	/* Read ID chunk
	 * \*------------------------------------------------------------ */
	read_p = XMI_img;

	ys_read_4cc(id_chunk.id_4cc, read_p, &read_p);
	id_chunk.chunk_len = ys_read_u32_be(read_p, &read_p);
	ys_read_4cc(id_chunk.desc_4cc, read_p, &read_p);

	if (memcmp(id_chunk.id_4cc, IFF_FORMAT_4CC, 4) != 0) {
		R_printf(R_STDERR, "Error: ID chunk not present.\n");

		return R_FAILURE;
	}

	if (memcmp(id_chunk.desc_4cc, XMI_DESC_4CC, 4) != 0) {
		R_printf(R_STDERR,
		    "Error: XDIR tag not present in ID chunk.\n");

		return R_FAILURE;
	}

	/* Read INFO chunk
	 * \*------------------------------------------------------------ */
	ys_read_4cc(info_chunk.id_4cc, read_p, &read_p);
	info_chunk.chunk_len = ys_read_u32_be(read_p, &read_p);
	info_chunk.n_tracks = ys_read_u16_le(read_p, &read_p);

	if (memcmp(info_chunk.id_4cc, XMI_INFO_4CC, 4) != 0) {

		R_printf(R_STDERR, "Error: INFO chunk not present.\n");

		return R_FAILURE;
	}

	*n_tracks = info_chunk.n_tracks;

	*XMIDI_data = XMI_img +
	    (id_chunk.chunk_len + IFF_ID_CHUNK_HEADERLEN - 4);

	return R_SUCCESS;
}

int
XMIDI_ReadEvents(XMIDIEVENT_LIST * event_list,
    const uchar * event_data, size_t event_data_len, uint n_tracks)
{

	const uchar *event_data_ptr = event_data;
	size_t event_bytes_left = event_data_len;

	ulong new_event_time = 0;
	ulong event_time = 0;
	ulong event_len;

	ulong vlq_len;
	uint data_byte;

	int channel;
	int event;

	/*int tempo = MIDI_STD_TEMPO; */

	unsigned int sysex_op;
	unsigned int op1;
	unsigned int op2;
	unsigned int op3;
	unsigned int op4;

	/* Set initial tempo */
	/*
	 * AddEventToList( event_list, MIDI_SYSEX_TEMPO_LEN + GetLengthAsVLQ( 0 ), 0, MIDI_SYSTEMEXCLUSIVE, 0, MIDI_SYSEX_TEMPO, 0, );
	 */

	while (event_bytes_left > 0) {

		vlq_len = ReadVLQ2_DW((char *)event_data_ptr,
							  (DWORD)event_bytes_left, (DWORD *)&new_event_time);

		event_time += new_event_time;
		event_data_ptr += vlq_len;
		event_bytes_left -= vlq_len;

		/*
		 * vlq_len = GetLengthAsVLQ( new_event_time );
		 * R_printf( R_STDOUT, "Count: %d len VLQ (%d)\n", vlq_len, new_event_time );
		 */

		data_byte = *event_data_ptr++;

		channel = data_byte & 0x0FU;
		event = data_byte & 0xF0U;

		switch (event) {

		case MIDI_NOTE_ON:

#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_NOTE_ON event:\n");
#endif

			op1 = *(event_data_ptr++);
			op2 = *(event_data_ptr++);

			AddEventToList(event_list,
			    MIDI_NOTE_ON_LEN,
			    event_time, event, channel, 0, op1, op2, 0, 0);

			vlq_len =
			    ReadVLQ_DW((char *)event_data_ptr, (DWORD)event_bytes_left,
						   (DWORD *)&event_len);
			AddEventToList(event_list, MIDI_NOTE_OFF_LEN,
			    event_time + event_len, MIDI_NOTE_OFF, channel, 0,
			    op1, MIDI_STD_VELOCITY, 0, 0);

			event_data_ptr += (vlq_len);
			event_bytes_left -= (2 + vlq_len);
			break;

		case MIDI_AFTERTOUCH:
#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_AFTERTOUCH event:\n");
#endif
			op1 = *(event_data_ptr++);
			op2 = *(event_data_ptr++);

			AddEventToList(event_list,
			    MIDI_AFTERTOUCH_LEN,
			    event_time, event, channel, 0, op1, op2, 0, 0);

			event_bytes_left -= 2;
			break;

		case MIDI_CONTROLCHANGE:
#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_CONTROLCHANGE event:\n");
#endif
			op1 = *(event_data_ptr++);
			op2 = *(event_data_ptr++);

			AddEventToList(event_list,
			    MIDI_CONTROLCHANGE_LEN,
			    event_time, event, channel, 0, op1, op2, 0, 0);

			event_bytes_left -= 2;
			break;

		case MIDI_PITCHWHEEL:
#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_PITCHWHEEL event:\n");
#endif
			op1 = *(event_data_ptr++);
			op2 = *(event_data_ptr++);

			AddEventToList(event_list,
			    MIDI_PITCHWHEEL_LEN,
			    event_time, event, channel, 0, op1, op2, 0, 0);

			event_bytes_left -= 2;
			break;

		case MIDI_PROGRAMCHANGE:
#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_PROGRAMCHANGE event:\n");
#endif
			op1 = *(event_data_ptr++);
			AddEventToList(event_list, MIDI_PROGRAMCHANGE_LEN,
			    event_time, event, channel, 0, op1, 0, 0, 0);

			event_bytes_left--;
			break;

		case MIDI_CHANNELPRESSURE:
#ifdef XMIPLAY_VERBOSE
			R_printf(R_STDOUT, "MIDI_CHANNELPRESSURE event:\n");
#endif
			op1 = *(event_data_ptr++);
			AddEventToList(event_list, MIDI_CHANNELPRESSURE_LEN,
			    event_time, event, channel, 0, op1, 0, 0, 0);

			event_bytes_left--;
			break;

		case MIDI_SYSTEMEXCLUSIVE:

			sysex_op = (BYTE) * event_data_ptr++;
			event_bytes_left--;

			if (data_byte == MIDI_NONMIDI) {

				switch (sysex_op) {

				case MIDI_SYSEX_TRACKEND:
					R_printf(R_STDOUT,
					    "Track end encountered.\n");
					AddEventToList(event_list,
					    MIDI_SYSEX_TRACKEND_LEN,
					    event_time, event, channel,
					    sysex_op, op1, op2, 0, 0);
					event_bytes_left = 0;
					break;

				case MIDI_SYSEX_TEMPO:
					event_data_ptr++;	/*(skip length VLQ) (always 3) */

					op1 = (BYTE) * event_data_ptr++;
					op2 = (BYTE) * event_data_ptr++;
					op3 = (BYTE) * event_data_ptr++;
					AddEventToList(event_list,
					    MIDI_SYSEX_TEMPO_LEN, event_time,
					    event, channel, sysex_op, op1, op2,
					    op3, 0);
					/*
					 * R_printf( R_STDOUT, "Adding tempo change event. :%X %X %X\n", op1, op2, op3 );
					 */
					event_bytes_left -= 4;
					break;

				case MIDI_SYSEX_TIMESIG:
					event_data_ptr++;	/*(skip length VLQ) (always 4) */

					op1 = (BYTE) * event_data_ptr++;
					op2 = (BYTE) * event_data_ptr++;
					op3 = (BYTE) * event_data_ptr++;
					op4 = (BYTE) * event_data_ptr++;
					AddEventToList(event_list,
					    MIDI_SYSEX_TIMESIG_LEN, event_time,
					    event, channel, sysex_op, op1, op2,
					    op3, op4);

					/*
					 * R_printf( R_STDOUT, "Adding time signature event. :%X %X %X %X\n", op1, op2, op3, op4 );
					 */
					event_bytes_left -= 5;
					break;

				default:
					R_printf(R_STDERR,
					    "Unhandled sysex nonmidi event, aborting.\n");
					R_printf(R_STDERR, "%X %X %X %X",
					    *event_data_ptr,
					    *(event_data_ptr + 1),
					    *(event_data_ptr + 2),
					    *(event_data_ptr + 3));

					event_bytes_left = 0;
					break;

				}
			} else {
				R_printf(R_STDERR,
				    "Unhandled sysex event, aborting.\n");
				event_bytes_left = 0;
			}

			break;

		default:
			R_printf(R_STDERR,
			    "Invalid event code encountered; aborting.\n");
			event_bytes_left = 0;
			break;
		}

	}			/* end while ( event_bytes_left > 0 ) */

	return R_SUCCESS;
}

int GetLengthAsVLQ(DWORD data)
{

	int len = 1;

	while (data >>= 7)
		len++;
	return len;

}

DWORD ReadVLQ_DW(char *data, DWORD bytes_left, DWORD * value)
{
	BYTE byte;
	DWORD vlq_len = 0;
	*value = 0;

	do {
		if (bytes_left <= 0)
			return 0;
		byte = *data++;
		bytes_left--;
		vlq_len++;
		*value = (*value << 7) | (byte & 0x7F);
	} while (byte & 0x80);

	return vlq_len;
}

DWORD ReadVLQ2_DW(char *data, DWORD bytes_left, DWORD * value)
{

	BYTE byte;
	DWORD vlq_len = 0;
	*value = 0;

	while (!((byte = *data++) & 0x80)) {
		if (bytes_left <= 0)
			return 0;
		bytes_left--;
		vlq_len++;
		(*value) += byte;
	}

	return vlq_len;
}

int
AddEventToList(XMIDIEVENT_LIST * event_list, int smf_size, int time, int event,
    int channel, int sysex_op, int op1, int op2, int op3, int op4)
{

	XMIDIEVENT *new_event;
	XMIDIEVENT *search_ptr = event_list->tail;

	new_event = (XMIDIEVENT *)malloc(sizeof(XMIDIEVENT));

	if (new_event == NULL) {
		R_printf(R_STDERR,
		    "Error: Out of memory allocating XMIDI event list entry.");
		return -1;
	}

	new_event->next_event = NULL;
	new_event->prev_event = NULL;

	if (event_list->head == NULL) {
		/* Set up new list */
		event_list->head = new_event;
		event_list->tail = new_event;
	} else {
		/* List isn't empty */
		if ((unsigned int)time >= event_list->tail->delta_time) {

			/* If this is the most recent event, append */
			event_list->tail->next_event = new_event;
			new_event->prev_event = event_list->tail;
			event_list->tail = new_event;

		} else {
			/* Otherwise scan list backwards and insert in proper position */
			while (search_ptr != NULL) {

				if ((unsigned int)time >=
				    search_ptr->delta_time) {
					/* Insert entry */
					new_event->next_event =
					    search_ptr->next_event;
					new_event->prev_event = search_ptr;

					search_ptr->next_event->prev_event =
					    new_event;
					search_ptr->next_event = new_event;
					break;
				}
				search_ptr = search_ptr->prev_event;
			}
		}
	}

	new_event->smf_size = smf_size;
	new_event->delta_time = time;

	new_event->sysex_op = sysex_op;
	new_event->event = (BYTE) event;
	new_event->channel = (BYTE) channel;
	new_event->op1 = (BYTE) op1;
	new_event->op2 = (BYTE) op2;
	new_event->op3 = (BYTE) op3;
	new_event->op4 = (BYTE) op4;

#ifdef XMIPLAY_VERBOSE
	R_printf(R_STDOUT,
	    "Added event: Time: %d Tempo: %d Event: %d Chan: %d Op1: %d Op2: %d\n",
	    new_event->delta_time, new_event->tempo, new_event->event,
	    new_event->channel, new_event->op1, new_event->op2);
#endif
	return 0;
}

int ProcessEventList(XMIDIEVENT_LIST * event_list)
{
	XMIDIEVENT *convert_ptr = event_list->head;
	int last_time = 0;
	int delta = 0;

	while (convert_ptr != NULL) {

		delta = convert_ptr->delta_time - last_time;
		if (delta < 0)
			R_printf(R_STDERR,
			    "Error: Negative delta time found.");
		last_time = convert_ptr->delta_time;
		convert_ptr->delta_time = delta;

		/* Update smf size count */
		event_list->smf_size +=
		    (convert_ptr->smf_size + GetLengthAsVLQ(delta));
		convert_ptr = convert_ptr->next_event;
	}

	R_printf(R_STDOUT,
	    "ProcessEventList(): %d bytes of SMF data processed.\n",
	    event_list->smf_size);

	return 0;
}

int XMIDI_Free(XMIDIEVENT_LIST * event_list)
{

	XMIDIEVENT *free_ptr = event_list->head;
	XMIDIEVENT *temp_ptr;

	while (free_ptr != NULL) {
		temp_ptr = free_ptr->next_event;
		free(free_ptr);
		free_ptr = temp_ptr;
	}

	return 0;

}

} // End of namespace Saga

