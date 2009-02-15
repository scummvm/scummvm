/***************************************************************************
 audiobuf.h Copyright (C) 2003 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* Auxiliary audio buffer for PCM devices
** Polled PCM devices must store data written to them until it is explicitly
** requiested. This is facilitated by the structures and functions defined
** here.
**   This is generic for all PCM devices; it implies no specific requirements.
**
** Usage: Introduce an sfx_audio_buf_t into your state and make sure to use
** each of the functions provided here at least once in the appropriate
** places.
*/


#ifndef _AUDIOBUF_H_
#define _AUDIOBUF_H_

#include <resource.h>
#include <sfx_time.h>
#include <sci_memory.h>
#include <sfx_pcm.h>


#define SFX_AUDIO_BUF_SIZE 8192	/* Must be multiple of framesize */
#define SFX_AUDIO_MAX_FRAME 8	/* Max. individual frame size */

typedef struct _sfx_audio_buf_chunk {
	unsigned char data[SFX_AUDIO_BUF_SIZE];
	int used;
	struct _sfx_audio_buf_chunk *prev;
	struct _sfx_audio_buf_chunk *next;
} sfx_audio_buf_chunk_t;

typedef struct {
	int read_offset;
	sfx_audio_buf_chunk_t *first; /* Next to read-- can be = last */
	sfx_audio_buf_chunk_t *last; /* Next to write-- can be = first */
	sfx_audio_buf_chunk_t *unused; /* Unused chunk list, can be NULL */
	unsigned char last_frame[SFX_AUDIO_MAX_FRAME];
	/* Contains the last frame successfully read; used for buffer
	** underruns to avoid crack before silance  */
	sfx_timestamp_t read_timestamp; /* Timestamp for reading */
	int frames_nr; /* Total number of frames currently in between reading and writing */
	int framesize;
} sfx_audio_buf_t;


void
sfx_audbuf_init(sfx_audio_buf_t *buf, sfx_pcm_config_t conf);
/* Initialises an audio buffer
** Parameters: (sfx_audio_buf_t *) buf: The buffer to initialise
**             (sfx_pcm_config_t) conf: The configuration for which the buffer should
**                                      be set up
** Modifies  : (sfx_audio_buf_t) *buf
*/

void
sfx_audbuf_free(sfx_audio_buf_t *buf);
/* Frees all memory associated with an audio buffer
** Parameters: (sfx_audio_buf_t *) buf: The buffer whose associated memory
**					should be freed
** Modifies  : (sfx_audio_buf_t) *buf
*/

void
sfx_audbuf_write(sfx_audio_buf_t *buf, unsigned char *src, int frames);
/* Store data in an audion buffer
** Parameters: (sfx_audio_buf_t *) buf: The buffer to write to
**             (unsigned char *) src: Pointer to the data that should be
**                                    written
**             (int) frames: Number of frames to write
** Modifies  : (sfx_audio_buf_t) *buf
*/


void
sfx_audbuf_write_timestamp(sfx_audio_buf_t *buf, sfx_timestamp_t ts);
/* Sets the most recently written timestamp for the buffer
** Parameters: (sfx_audio_buf_t *) buf: The buffer to operate on
**             (sfx_timestamp_t) ts: The timestamp to set
** If a timestamp is already set, 'ts' is checked for consistency and
** 'silent' frames are introduced as padding for future writes.
*/


int
sfx_audbuf_read_timestamp(sfx_audio_buf_t *buf, sfx_timestamp_t *ts);
/* Reads the timestamp describing the time right before the next frame being read
** Parameters: (sfx_audio_buf_t *) buf: The buffer to read from
** Returns   : (sfx_timestamp_t) *ts: The requested timestamp, or nothing
**             (int) zero on success, nonzero if no timestamp is known
*/


int
sfx_audbuf_read(sfx_audio_buf_t *buf, unsigned char *dest, int frames);
/* Read data from audio buffer
** Parameters: (sfx_audio_buf_t *) buf: The buffer to write to
**             (unsigned char *) dest: Pointer to the place the read data
**                                     should be written to
**             (int) frames: Number of frames to write
** Returns   : (int) Number of frames actually read
** Affects   : (sfx_audio_buf_t) *buf
**             (unsigned char ) *dest
**             global error stream
** If the returned number of frames is smaller than the number of frames
** requested to be written, this function will issue a buffer underrun
** warning and fill up the remaining space with the last frame it en--
** countered, or a block of '0' if no such frame is known.
*/




#endif /* !_AUDIOBUF_H_ */
