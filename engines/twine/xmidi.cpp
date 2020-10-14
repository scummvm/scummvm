/** @file xmidi.cpp
	@brief
	This file contains MIDI-related routines.
	These routines were adapted from ScrummVM/Exult engine source code.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2013 The ScrummVM/ExultEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "xmidi.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define warning(...) if (cfgfile.Debug) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
//#define info(...) if (cfgfile.Debug) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

/**
 * Provides comprehensive information on the next event in the MIDI stream.
 * An EventInfo struct is instantiated by format-specific implementations
 * of MidiParser::parseNextEvent() each time another event is needed.
 *
 * Adapted from the ScummVM project
 */
struct EventInfo {
	uint8 * start; ///< Position in the MIDI stream where the event starts.
	              ///< For delta-based MIDI streams (e.g. SMF and XMIDI), this points to the delta.
	uint32 delta; ///< The number of ticks after the previous event that this event should occur.
	uint8 event; ///< Upper 4 bits are the command code, lower 4 bits are the MIDI channel.
	              ///< For META, event == 0xFF. For SysEx, event == 0xF0.
	union {
		struct {
			uint8 param1; ///< The first parameter in a simple MIDI message.
			uint8 param2; ///< The second parameter in a simple MIDI message.
		} basic;
		struct {
			uint8   type; ///< For META events, this indicates the META type.
			uint8 * data; ///< For META and SysEx events, this points to the start of the data.
		} ext;
	};
	uint32 length; ///< For META and SysEx blocks, this indicates the length of the data.
	               ///< For Note On events, a non-zero value indicates that no Note Off event
	               ///< will occur, and the MidiParser will have to generate one itself.
	               ///< For all other events, this value should always be zero.
};

// Adapted from the ScummVM project
struct XMIDI_info {
	uint8 num_tracks;
	uint8* tracks[120]; // Maximum 120 tracks
};

/* Linked list of saved note offs to be injected in the midi stream at a later
 * time. */
struct CachedEvent {
	struct EventInfo* eventInfo;
	uint32 time;
	struct CachedEvent* next;
};
static struct CachedEvent* cached_events = NULL;

/*
 * Forward declarations
 */
static uint16 read2low(uint8** data);
static uint32 read4high(uint8** data);
static void write4high(uint8** data, uint32 val);
static void write2high(uint8** data, uint16 val);
static uint32 readVLQ2(uint8** data);
static uint32 readVLQ(uint8** data);
static int32 putVLQ(uint8* dest, uint32 value);

/* Returns an EventInfo struct if there is a cached event that should be
 * played between current_time and current_time + delta. The cached event
 * is removed from the internal list of cached events! */
static struct EventInfo* pop_cached_event(uint32 current_time, uint32 delta);
static void save_event(struct EventInfo* info, uint32 current_time);

static int32 read_event_info(uint8* data, struct EventInfo* info, uint32 current_time);
static int32 put_event(uint8* dest, struct EventInfo* info);
static int32 convert_to_mtrk(uint8* data, uint32 size, uint8* dest);
static int32 read_XMIDI_header(uint8* data, uint32 size, struct XMIDI_info* info);

// Adapted from the ScummVM project
static uint16 read2low(uint8** data)
{
	uint8* d = *data;
	uint16 value = (d[1] << 8) | d[0];
	*data = (d + 2);
	return value;
}

// Adapted from the ScummVM project
static uint32 read4high(uint8** data)
{
	uint8* d = *data;
	uint16 value = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3]);
	*data = (d + 4);
	return value;
}

// Adapted from the ScummVM project
static void write4high(uint8** data, uint32 val)
{
	uint8* d = *data;
	*d++ = (val >> 24) & 0xff;
	*d++ = (val >> 16) & 0xff;
	*d++ = (val >> 8) & 0xff;
	*d++ = val & 0xff;
	*data = d;
}

// Adapted from the ScummVM project
static void write2high(uint8** data, uint16 val)
{
	uint8* d = *data;
	*d++ = (val >> 8) & 0xff;
	*d++ = val & 0xff;
	*data = d;
}

// This is a special XMIDI variable length quantity
//
// Adapted from the ScummVM project
static uint32 readVLQ2(uint8** data)
{
	uint8* pos = *data;
	uint32 value = 0;
	while (!(pos[0] & 0x80)) {
		value += *pos++;
	}
	*data = pos;
	return value;
}

// This is the conventional (i.e. SMF) variable length quantity
//
// Adapted from the ScummVM project
static uint32 readVLQ(uint8** data) {
	uint8* d = *data;
	uint8 str;
	uint32 value = 0;
	int32 i;

	for (i = 0; i < 4; ++i) {
		str = *d++;
		value = (value << 7) | (str & 0x7F);
		if (!(str & 0x80))
			break;
	}
	*data = d;
	return value;
}

// PutVLQ
//
// Write a Conventional Variable Length Quantity
//
// Code adapted from the Exult engine
static int32 putVLQ(uint8* dest, uint32 value)
{
	int32 buffer;
	int32 j, i = 1;

	buffer = value & 0x7F;
	while (value >>= 7)
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
		i++;
	}
	if (!dest) return i;
	for (j = 0; j < i; j++)
	{
		*dest++ = buffer & 0xFF;
		buffer >>= 8;
	}
	return i;
}

static void save_event(struct EventInfo* info, uint32 current_time)
{
	uint32 delta = info->length;
	struct CachedEvent *prev, *next, *temp;

	temp = malloc(sizeof(struct CachedEvent));
	temp->eventInfo = info;
	temp->time = current_time + delta;
	temp->next = NULL;

	//info("Saving event to be stopped at %2X", temp->time);

	if (!cached_events) {
		cached_events = temp;
	}
	else {
		prev = NULL;
		next = cached_events;

		/* Find the proper time slot */
		while (next && next->time < current_time + delta) {
			prev = next;
			next = next->next;
		}

		if (!next) {
			prev->next = temp;
		}
		else {
			if (prev) {
				temp->next = prev->next;
				prev->next = temp;
			}
			else {
				temp->next = cached_events;
				cached_events = temp;
			}
		}
	}
}

static struct EventInfo* pop_cached_event(uint32 current_time, uint32 delta)
{
	struct EventInfo* info = NULL;
	struct CachedEvent* old;

	if (cached_events && cached_events->time < current_time + delta) {
		info = cached_events->eventInfo;
		info->delta = cached_events->time - current_time;
		old = cached_events;
		cached_events = cached_events->next;
		free(old);
	}

	return info;
}

// Adapted from the ScummVM project
static int32 read_event_info(uint8* data, struct EventInfo* info, uint32 current_time)
{
	struct EventInfo* injectedEvent;
	info->start = data;
	info->delta = readVLQ2(&data);
	info->event = *data++;

	/* Advance current time here, but not yet in the main conversion loop.
	 * This is so that cached events can still be injected correctly */
	current_time += info->delta;

	//info("%02X: Parsing event %02X", current_time, info->event);
	switch (info->event >> 4) {
	case 0x9: // Note On
		info->basic.param1 = *(data++);
		info->basic.param2 = *(data++);
		info->length = readVLQ(&data);
		if (info->basic.param2 == 0) {
			info->event = (info->event & 0x0F) | 0x80;
			info->length = 0;
		}
		else {
			//info("Found Note On with duration %X. Saving a Note Off for later", info->length);
			injectedEvent = malloc(sizeof(struct EventInfo));
			injectedEvent->event = 0x80 | (info->event & 0x0f);
			injectedEvent->basic.param1 = info->basic.param1;
			injectedEvent->basic.param2 = info->basic.param2;
			injectedEvent->length = info->length;
			save_event(injectedEvent, current_time);
		}
		break;

	case 0xC:
	case 0xD:
		info->basic.param1 = *(data++);
		info->basic.param2 = 0;
		break;

	case 0x8:
	case 0xA:
	case 0xE:
		info->basic.param1 = *(data++);
		info->basic.param2 = *(data++);
		break;

	case 0xB:
		info->basic.param1 = *(data++);
		info->basic.param2 = *(data++);

		// This isn't a full XMIDI implementation, but it should
		// hopefully be "good enough" for most things.

		switch (info->basic.param1) {
		// Simplified XMIDI looping.
		case 0x74: {	// XMIDI_CONTROLLER_FOR_LOOP
#if 0 // TODO
				uint8 *pos = data;
				if (_loopCount < ARRAYSIZE(_loop) - 1)
					_loopCount++;
				/*else
					warning("XMIDI: Exceeding maximum loop count %d", ARRAYSIZE(_loop));*/

				_loop[_loopCount].pos = pos;
				_loop[_loopCount].repeat = info->basic.param2;
#endif
				break;
			}

		case 0x75:	// XMIDI_CONTORLLER_NEXT_BREAK
#if 0 // TODO
			if (_loopCount >= 0) {
				if (info->basic.param2 < 64) {
					// End the current loop.
					_loopCount--;
				} else {
					// Repeat 0 means "loop forever".
					if (_loop[_loopCount].repeat) {
						if (--_loop[_loopCount].repeat == 0)
							_loopCount--;
						else
							data = _loop[_loopCount].pos;
					} else {
						data = _loop[_loopCount].pos;
					}
				}
			}
#endif
			break;

		case 0x77:	// XMIDI_CONTROLLER_CALLBACK_TRIG
#if 0 // TODO
			if (_callbackProc)
				_callbackProc(info->basic.param2, _callbackData);
#endif
			break;

		case 0x6e:	// XMIDI_CONTROLLER_CHAN_LOCK
		case 0x6f:	// XMIDI_CONTROLLER_CHAN_LOCK_PROT
		case 0x70:	// XMIDI_CONTROLLER_VOICE_PROT
		case 0x71:	// XMIDI_CONTROLLER_TIMBRE_PROT
		case 0x72:	// XMIDI_CONTROLLER_BANK_CHANGE
		case 0x73:	// XMIDI_CONTROLLER_IND_CTRL_PREFIX
		case 0x76:	// XMIDI_CONTROLLER_CLEAR_BB_COUNT
		case 0x78:	// XMIDI_CONTROLLER_SEQ_BRANCH_INDEX
		default:
			if (info->basic.param1 >= 0x6e && info->basic.param1 <= 0x78) {
				/*warning("Unsupported XMIDI controller %d (0x%2x)",
					info->basic.param1, info->basic.param1);*/
			}
		}

		// Should we really keep passing the XMIDI controller events to
		// the MIDI driver, or should we turn them into some kind of
		// NOP events? (Dummy meta events, perhaps?) Ah well, it has
		// worked so far, so it shouldn't cause any damage...

		break;

	case 0xF: // Meta or SysEx event
		switch (info->event & 0x0F) {
		case 0x2: // Song Position Pointer
			info->basic.param1 = *(data++);
			info->basic.param2 = *(data++);
			break;

		case 0x3: // Song Select
			info->basic.param1 = *(data++);
			info->basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info->basic.param1 = info->basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info->length = readVLQ(&data);
			info->ext.data = data;
			data += info->length;
			break;

		case 0xF: // META event
			info->ext.type = *(data++);
			info->length = readVLQ(&data);
			info->ext.data = data;
			data += info->length;
			if (info->ext.type == 0x51 && info->length == 3) {
				// Tempo event. We want to make these constant 500,000.
				info->ext.data[0] = 0x07;
				info->ext.data[1] = 0xA1;
				info->ext.data[2] = 0x20;
			}
			break;

		default:
			//warning("MidiParser_XMIDI::parseNextEvent: Unsupported event code %x (delta: %X)", info->event, info->delta);
			return 0;
		}
	}

	return (data - info->start);
}

// Code adapted from the Exult engine
static int32 put_event(uint8* dest, struct EventInfo* info)
{
	int32 i = 0,j;
	int32 rc = 0;
	static uint8 last_event = 0;

	rc = putVLQ (dest, info->delta);
	if (dest) dest += rc;
	i += rc;

	if ((info->event != last_event) || (info->event >= 0xF0))
	{
		if (dest) *dest++ = (info->event);
		i++;
	}

	last_event = info->event;

	switch (info->event >> 4)
	{
		// 2 bytes data
		// Note off, Note on, Aftertouch, Controller and Pitch Wheel
		case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
		if (dest)
		{
			*dest++ = (info->basic.param1);
			*dest++ = (info->basic.param2);
		}
		i += 2;
		break;


		// 1 bytes data
		// Program Change and Channel Pressure
		case 0xC: case 0xD:
		if (dest) *dest++ = (info->basic.param1);
		i++;
		break;


		// Variable length
		// SysEx
		case 0xF:
		if (info->event == 0xFF)
		{
			if (dest) *dest++ = (info->basic.param1);
			i++;
		}

		rc = putVLQ (dest, info->length);
		if (dest) dest += rc;
		i += rc;

		if (info->length)
		{
			for (j = 0; j < (int)info->length; j++)
			{
				if (dest) *dest++ = (info->ext.data[j]);
				i++;
			}
		}

		break;


		// Never occur
		default:
		//warning("Not supposed to see this");
		break;
	}

	return i;
}

// Code adapted from the Exult engine
static int32 convert_to_mtrk(uint8* data, uint32 size, uint8* dest)
{
	int32 time = 0;
	int32 lasttime = 0;
	int32 rc;
	uint32 	i = 8;
	uint8*	size_pos = NULL;
	uint8*	data_end = data + size;
	struct XMIDI_info xmidi_info;
	struct EventInfo info;
	struct EventInfo* cached_info;

	if (dest)
	{
		*dest++ =('M');
		*dest++ =('T');
		*dest++ =('r');
		*dest++ =('k');

		size_pos = dest;
		dest += 4;
	}

	rc = read_XMIDI_header(data, size, &xmidi_info);
	if (!rc) {
		//warning("Failed to read XMIDI header");
		return 0;
	}

	data = xmidi_info.tracks[0];

	while (data < data_end)
	{
		//info("=======================================================================");
		// We don't write the end of stream marker here, we'll do it later
		if (data[0] == 0xFF && data[1] == 0x2f) {
			//info("Got EOX");
//			lasttime = event->time;
			continue;
		}

		rc = read_event_info(data, &info, time);
		if (!rc) {
			//warning("Failed to read event info %ld bytes from the end!", data_end - data);
			return 0;
		}
		data += rc;

		cached_info = pop_cached_event(time, info.delta);
		while (cached_info) {
			//info("Injecting event %2X at time %2X", cached_info->event, time);
			rc = put_event(dest, cached_info);
			if (!rc) {
				//warning("Failed to save injected event!");
				return 0;
			}
			if (dest) dest += rc;
			i += rc;
			time += cached_info->delta;
			info.delta -= cached_info->delta;
			free(cached_info);
			cached_info = pop_cached_event(time, info.delta);
		}

		//info("Saving event %02X", info.event);
		rc = put_event(dest, &info);
		if (!rc) {
			//warning("Failed to save event!");
			return 0;
		}
		if (dest) dest += rc;
		i += rc;
		time += info.delta;
		if (info.event == 0xFF && info.ext.type == 0x2F) {
			//info("GOT EOX");
			data = data_end;
		}
	}

	// Write out end of stream marker
	if (lasttime > time) {
		rc = putVLQ (dest, lasttime-time);
		if (dest) dest += rc;
		i += rc;
	}
	else {
		rc = putVLQ (dest, 0);
		if (dest) dest += rc;
		i += rc;
	}
	if (dest) {
		*dest++ = (0xFF);
		*dest++ = (0x2F);
	}
	rc = putVLQ (dest, 0);
	i += 2+rc;

	if (dest)
	{
		dest += rc;
		write4high(&size_pos, i-8);
	}
	return i;
}

/* Code adapted from the ScummVM project, which originally adapted it from the
 * Exult engine */
static int32 read_XMIDI_header(uint8* data, uint32 size, struct XMIDI_info* info)
{
	uint32 i = 0;
	uint8 *start;
	uint32 len;
	uint32 chunkLen;
	char buf[32];
	uint8 *pos = data;
	int32 tracksRead = 0;

	if (!memcmp(pos, "FORM", 4)) {
		pos += 4;

		// Read length of
		len = read4high(&pos);
		start = pos;

		// XDIRless XMIDI, we can handle them here.
		if (!memcmp(pos, "XMID", 4)) {
			//warning("XMIDI doesn't have XDIR");
			pos += 4;
			info->num_tracks = 1;
		} else if (memcmp(pos, "XDIR", 4)) {
			// Not an XMIDI that we recognize
			//warning("Expected 'XDIR' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
			return 0;
		} else {
			// Seems Valid
			pos += 4;
			info->num_tracks = 0;

			for (i = 4; i < len; i++) {
				// Read 4 bytes of type
				memcpy(buf, pos, 4);
				pos += 4;

				// Read length of chunk
				chunkLen = read4high(&pos);

				// Add eight bytes
				i += 8;

				if (memcmp(buf, "INFO", 4) == 0) {
					// Must be at least 2 bytes long
					if (chunkLen < 2) {
						//warning("Invalid chunk length %d for 'INFO' block", (int)chunkLen);
						return 0;
					}

					info->num_tracks = (uint8)read2low(&pos);
					pos += 2;

					if (chunkLen > 2) {
						//warning("Chunk length %d is greater than 2", (int)chunkLen);
						//pos += chunkLen - 2;
					}
					break;
				}

				// Must align
				pos += (chunkLen + 1) & ~1;
				i += (chunkLen + 1) & ~1;
			}

			// Didn't get to fill the header
			if (info->num_tracks == 0) {
				//warning("Didn't find a valid track count");
				return 0;
			}

			// Ok now to start part 2
			// Goto the right place
			pos = start + ((len + 1) & ~1);

			if (memcmp(pos, "CAT ", 4)) {
				// Not an XMID
				//warning("Expected 'CAT ' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
				return 0;
			}
			pos += 4;

			// Now read length of this track
			len = read4high(&pos);

			if (memcmp(pos, "XMID", 4)) {
				// Not an XMID
				//warning("Expected 'XMID' but found '%c%c%c%c'", pos[0], pos[1], pos[2], pos[3]);
				return 0;
			}
			pos += 4;

		}

		// Ok it's an XMIDI.
		// We're going to identify and store the location for each track.
		if (info->num_tracks > ARRAYSIZE(info->tracks)) {
			//warning("Can only handle %d tracks but was handed %d", (int)ARRAYSIZE(info->tracks), (int)info->num_tracks);
			return 0;
		}

		while (tracksRead < info->num_tracks) {
			if (!memcmp(pos, "FORM", 4)) {
				// Skip this plus the 4 bytes after it.
				pos += 8;
			} else if (!memcmp(pos, "XMID", 4)) {
				// Skip this.
				pos += 4;
			} else if (!memcmp(pos, "TIMB", 4)) {
				// Custom timbres?
				// We don't support them.
				// Read the length, skip it, and hope there was nothing there.
				pos += 4;
				len = read4high(&pos);
				pos += (len + 1) & ~1;
			} else if (!memcmp(pos, "EVNT", 4)) {
				// Ahh! What we're looking for at last.
				info->tracks[tracksRead] = pos + 8; // Skip the EVNT and length bytes
				pos += 4;
				len = read4high(&pos);
				pos += (len + 1) & ~1;
				++tracksRead;
			} else {
				//warning("Hit invalid block '%c%c%c%c' while scanning for track locations", pos[0], pos[1], pos[2], pos[3]);
				return 0;
			}
		}

		return 1;
	}

	return 0;
}

/********************************* Public API *********************************/
// Code adapted from the Exult engine
uint32 convert_to_midi(uint8* data, uint32 size, uint8** dest)
{
	int32 len;
	uint8* d,* start;

	if (!dest)
		return 0;

	/* Do a dry run first so we know how much memory to allocate */
	len = convert_to_mtrk (data, size, NULL);
	if (!len) {
		//warning("Failed dummy conversion!");
		return 0;
	}

	//info("Allocating %d bytes of memory", len);
	d = malloc(len + 14);
	if (!d) {
		perror("Could not allocate memory");
		return 0;
	}
	start = d;

	*d++ = ('M');
	*d++ = ('T');
	*d++ = ('h');
	*d++ = ('d');

	write4high (&d, 6);

	write2high (&d, 0);
	write2high (&d, 1);
	write2high (&d, 60);	// The PPQN

	len = convert_to_mtrk(data, size, d);
	if (!len) {
		//warning("Failed to convert");
		free(d);
		return 0;
	}

	*dest = start;

	return len + 14;
}

