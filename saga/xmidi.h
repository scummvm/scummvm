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

#ifndef SAGA_XMIDI_H_
#define SAGA_XMIDI_H_

namespace Saga {

#define XMIDI_TIMEDIV 0x3C

/* XMIDI/IFF 4CC codes
\*--------------------------------------------------------------------------*/
#define IFF_FORMAT_4CC   "FORM"
#define IFF_CATEGORY_4CC "CAT "

#define XMI_DESC_4CC     "XDIR"
#define XMI_INFO_4CC     "INFO"

#define XMIDI_DESC_4CC   "XMID"
#define XMIDI_TIMBRE_4CC "TIMB"
#define XMIDI_EVENT_4CC  "EVNT"

/* IFF/XMI Data structures
\*--------------------------------------------------------------------------*/

struct IFF_ID_CHUNK {
	char id_4cc[4];	/* 4cc */
	ulong chunk_len;	/* u32_be */
	char desc_4cc[4];	/* 4cc */
};

#define IFF_ID_CHUNK_HEADERLEN 12

struct XMI_INFO_CHUNK {
	char id_4cc[4];	/* 4cc */
	ulong chunk_len;	/* u32_be */
	uint n_tracks;	/* u16_le */
};

struct XMI_TIMB_CHUNK {
	char id_4cc[4];	/* 4cc */
	ulong chunk_len;	/* u32_be */
};

struct XMI_EVENT_CHUNK {
	char id_4cc[4];	/* 4cc */
	ulong chunk_len;	/* u32_be */
};

typedef struct SMF_HEADER_CHUNK_tag {
	char smf_id[4];	/* u8 */
	unsigned int smf_header_len;	/* u32_be */
	unsigned short smf_format;	/* u16_be */
	unsigned short smf_ntracks;	/* u16_be */
	union {
		unsigned short ppqn;	/* u16_be */
		signed char smpte[2];
	} time_division;
} SMF_HEADER_CHUNK;

struct SMF_TRACK_CHUNK {
	char smf_track_id[4];
	unsigned int smf_track_len;
};

#define MIDI_HEADER_LEN 14
#define MIDI_HEADER_CHUNK_LEN 6
#define MIDI_TRACK_CHUNK_LEN 8

#define MIDI_HEADER_TAG "MThd"
#define MIDI_TRACK_TAG  "MTrk"

int XMIDI_ReadXMIHeader(const uchar *XMI_img, const uchar **XMIDI_data, uint *n_tracks);
int ReadIFF_IDChunk(IFF_ID_CHUNK *chunk, const uchar *read_p, const uchar **read_pp);
int Print4CC(char *fourcc);
int XMIDI_ReadEvents(XMIDIEVENT_LIST *event_list, const uchar *event_data, 
					 size_t event_data_len, uint n_tracks);
int WriteVLQ_DW(char *write_ptr, DWORD value);
DWORD ReadVLQ_DW(const uchar *data, DWORD bytes_left, DWORD *value);
DWORD ReadVLQ2_DW(const uchar *data, DWORD bytes_left, DWORD *value);
int GetLengthAsVLQ(DWORD data);
int AddEventToList(XMIDIEVENT_LIST *event_list, int smf_size, int time,
	    int event, int channel, int sysex_op, int op1, int op2, int op3, int op4);
int ProcessEventList(XMIDIEVENT_LIST *event_list);
void PrintMidiOutError(unsigned long err);
}				// End of namespace Saga

#endif				/* SAGA_XMIDI_H_ */
