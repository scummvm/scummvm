/***************************************************************************
 sfx_device.h Copyright (C) 2003,04 Christoph Reichenbach


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
/* song player structure */

#ifndef _SFX_DEVICE_H
#define _SFX_DEVICE_H

/* Various types of resources */
#define SFX_DEVICE_NONE 0
#define SFX_DEVICE_MIDI 1 /* midi writer */
#define SFX_DEVICE_OPL2 2 /* OPL/2 sequencer */ 

struct _midi_device {
	const char *name;

	int (*init)(struct _midi_device *self);
	/* Initializes the device
	** Parameters: (midi_device_t *) self: Self reference
	** Returns   : (int) SFX_OK on success, SFX_ERROR if the device could not be
	**                   opened
	*/

	int (*set_option)(struct _midi_device *self, char *name, char *value);
	/* Sets an option for the device
	** Parameters: (char *) name: Name of the option to set
	**             (char *) value: Value of the option to set
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise (unsupported option)
	*/
};

// FIXME: This evil hack below is needed to create the
// decorated_midi_writer struct in instrumen-map.c -- in C++, this can
// be replaced by simple subclassing.
#define MIDI_WRITER_BODY	\
	char *name; /* Name description of the device */		\
									\
	int (*init)(struct _midi_writer *self);				\
	/* Initializes the writer					\
	** Parameters: (midi_writer_t *) self: Self reference		\
	** Returns   : (int) SFX_OK on success, SFX_ERROR if the device could not be \
	**                   opened					\
	*/								\
									\
	int (*set_option)(struct _midi_writer *self, char *name, char *value); \
	/* Sets an option for the writer				\
	** Parameters: (char *) name: Name of the option to set		\
	**             (char *) value: Value of the option to set	\
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise (unsupported option) \
	*/								\
									\
	int (*write)(struct _midi_writer *self, unsigned char *buf, int len); \
	/* Writes some bytes to the MIDI stream				\
	** Parameters: (char *) buf: The buffer to write		\
	**             (int) len: Number of bytes to write		\
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure	\
	** No delta time is expected here.				\
	*/								\
									\
	void (*delay)(struct _midi_writer *self, int ticks);		\
	/* Introduces an explicit delay					\
	** Parameters: (int) ticks: Number of 60 Hz ticks to sleep	\
	*/								\
									\
	void (*flush)(struct _midi_writer *self); /* May be NULL */	\
	/* Flushes the MIDI file descriptor				\
	** Parameters: (midi_writer_t *) self: Self reference		\
	*/								\
									\
	void (*reset_timer)(struct _midi_writer *self);			\
	/* Resets the timer associated with this device			\
	** Parameters: (midi_writer_t *) self: Self reference		\
	** This function makes sure that a subsequent write would have effect \
	** immediately, and any delay() would be relative to the point in time \
	** this function was invoked at.				\
	*/								\
									\
	void (*close)(struct _midi_writer *self);			\
	/* Closes the associated MIDI device				\
	** Parameters: (midi_writer_t *) self: Self reference		\
	*/

typedef struct _midi_writer {
	MIDI_WRITER_BODY
} midi_writer_t;


void *
sfx_find_device(int type, char *name);
/* Looks up a device by name
** Parameters: (int) type: Device type to look up
**             (char *) name: Comma-separated list of devices to choose from
**                            (in the order specified), or NULL for default
** Returns   : (void *) The device requested, or NULL if no match was found
*/

#endif /* !_SFX_PLAYER_H */
