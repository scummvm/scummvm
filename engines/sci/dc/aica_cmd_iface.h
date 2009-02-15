/*
 * Copyright 2000, 2001, 2002
 *         Dan Potter. All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Cryptic Allusion nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __ARM_AICA_CMD_IFACE_H
#define __ARM_AICA_CMD_IFACE_H

/* $Id: aica_cmd_iface.h 1486 2005-11-10 13:03:57Z waltervn $ */

#ifndef __ARCH_TYPES_H
typedef unsigned long uint8;
typedef unsigned long uint32;
#endif

/* Command queue; one of these for passing data from the SH-4 to the
   AICA, and another for the other direction. If a command is written
   to the queue and it is longer than the amount of space between the
   head point and the queue size, the command will wrap around to
   the beginning (i.e., queue commands _can_ be split up). */
typedef struct aica_queue {
	uint32		head;		/* Insertion point offset (in bytes)	*/
	uint32		tail;		/* Removal point offset (in bytes)	*/
	uint32		size;		/* Queue size (in bytes)		*/
	uint32		valid;		/* 1 if the queue structs are valid	*/
	uint32		process_ok;	/* 1 if it's ok to process the data	*/
	uint32		data;		/* Pointer to queue data buffer		*/
} aica_queue_t;

/* Command queue struct for commanding the AICA from the SH-4 */
typedef struct aica_cmd {
	uint32		size;		/* Command data size in dwords				*/
	uint32		cmd;		/* Command ID						*/
	uint32		timestamp;	/* When to execute the command (0 == now)		*/
	uint32		cmd_id;		/* Command ID, for cmd/response pairs, or channel id	*/
	uint32		misc[4];	/* Misc Parameters / Padding				*/
	uint8		cmd_data[0];	/* Command data 					*/
} aica_cmd_t;

/* Maximum command size -- 256 dwords */
#define AICA_CMD_MAX_SIZE	256

/* This is the cmd_data for AICA_CMD_CHAN. Make this 16 dwords long
   for two aica bus queues. */
typedef struct aica_channel {
	uint32		cmd;		/* Command ID		*/
	uint32		base;		/* Sample base in RAM	*/
	uint32		type;		/* (8/16bit/ADPCM)	*/
	uint32		length;		/* Sample length	*/
	uint32		loop;		/* Sample looping	*/
	uint32		loopstart;	/* Sample loop start	*/
	uint32		loopend;	/* Sample loop end	*/
	uint32		freq;		/* Frequency		*/
	uint32		vol;		/* Volume 0-255		*/
	uint32		pan;		/* Pan 0-255		*/
	uint32		pos;		/* Sample playback pos	*/
	uint32		pad[5];		/* Padding		*/
} aica_channel_t;

/* Declare an aica_cmd_t big enough to hold an aica_channel_t
   using temp name T, aica_cmd_t name CMDR, and aica_channel_t name CHANR */
#define AICA_CMDSTR_CHANNEL(T, CMDR, CHANR) \
	uint8	T[sizeof(aica_cmd_t) + sizeof(aica_channel_t)]; \
	aica_cmd_t	* CMDR = (aica_cmd_t *)T; \
	aica_channel_t	* CHANR = (aica_channel_t *)(CMDR->cmd_data);
#define AICA_CMDSTR_CHANNEL_SIZE	((sizeof(aica_cmd_t) + sizeof(aica_channel_t))/4)

/* Command values (for aica_cmd_t) */
#define AICA_CMD_NONE		0x00000000	/* No command (dummy packet)	*/
#define AICA_CMD_PING		0x00000001	/* Check for signs of life	*/
#define AICA_CMD_CHAN		0x00000002	/* Perform a wavetable action	*/
#define AICA_CMD_SYNC_CLOCK	0x00000003	/* Reset the millisecond clock	*/

/* Response values (for aica_cmd_t) */
#define AICA_RESP_NONE		0x00000000
#define AICA_RESP_PONG		0x00000001	/* Response to CMD_PING 			*/
#define AICA_RESP_DBGPRINT	0x00000002	/* Entire payload is a null-terminated string	*/

/* Command values (for aica_channel_t commands) */
#define AICA_CH_CMD_MASK	0x0000000f

#define AICA_CH_CMD_NONE	0x00000000
#define AICA_CH_CMD_START	0x00000001
#define AICA_CH_CMD_STOP	0x00000002
#define AICA_CH_CMD_UPDATE	0x00000003

/* Start values */
#define AICA_CH_START_MASK	0x00300000

#define AICA_CH_START_DELAY	0x00100000 /* Set params, but delay key-on */
#define AICA_CH_START_SYNC	0x00200000 /* Set key-on for all selected channels */

/* Update values */
#define AICA_CH_UPDATE_MASK	0x000ff000

#define AICA_CH_UPDATE_SET_FREQ	0x00001000 /* frequency		*/
#define AICA_CH_UPDATE_SET_VOL	0x00002000 /* volume		*/
#define AICA_CH_UPDATE_SET_PAN	0x00004000 /* panning		*/

/* Sample types */
#define AICA_SM_8BIT	1
#define AICA_SM_16BIT	0
#define AICA_SM_ADPCM	2


/* This is where our SH-4/AICA comm variables go... */

/* 0x000000 - 0x010000 are reserved for the program */

/* Location of the SH-4 to AICA queue; commands from here will be
   periodically processed by the AICA and then removed from the queue. */
#define AICA_MEM_CMD_QUEUE	0x010000	/* 32K */

/* Location of the AICA to SH-4 queue; commands from here will be
   periodically processed by the SH-4 and then removed from the queue. */
#define AICA_MEM_RESP_QUEUE	0x018000	/* 32K */

/* This is the channel base, which holds status structs for all the
   channels. This is READ-ONLY from the SH-4 side. */
#define AICA_MEM_CHANNELS	0x020000	/* 64 * 16*4 = 4K */

/* The clock value (in milliseconds) */
#define AICA_MEM_CLOCK		0x021000	/* 4 bytes */

/* 0x021004 - 0x030000 are reserved for future expansion */

/* Open ram for sample data */
#define AICA_RAM_START		0x030000
#define AICA_RAM_END		0x200000

/* Quick access to the AICA channels */
#define AICA_CHANNEL(x)		(AICA_MEM_CHANNELS + (x) * sizeof(aica_channel_t))

#endif	/* __ARM_AICA_CMD_IFACE_H */
