/***************************************************************************
 unixraw-midi.c Copyright (C) 2005 Walter van Niftrik


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

    Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include <sfx_engine.h>
#include "../device.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SCI_UNIXRAW_MIDI_VERSION "0.1"

#ifndef O_SYNC
#  define O_SYNC 0
#endif

static int fd;
static const char *devicename = "/dev/midi";

static int
unixraw_init(midi_writer_t *self) {
	sciprintf("[SFX] Initialising UNIX raw MIDI backend, v%s\n", SCI_UNIXRAW_MIDI_VERSION);

	fd = open(devicename, O_WRONLY | O_SYNC);

	if (!IS_VALID_FD(fd)) {
		sciprintf("[SFX] Failed to open %s\n", devicename);
		return SFX_ERROR;
	}

	return SFX_OK;
}

static int
unixraw_set_option(midi_writer_t *self, char *name, char *value) {
	return SFX_ERROR;
}

static int
unixraw_write(midi_writer_t *self, unsigned char *buffer, int len) {
	if (write(fd, buffer, len) != len) {
		sciprintf("[SFX] MIDI write error\n");
		return SFX_ERROR;
	}
	return SFX_OK;
}

static void
unixraw_delay(midi_writer_t *self, int ticks) {
}

static void
unixraw_reset_timer(midi_writer_t *self) {
}

static void
unixraw_close(midi_writer_t *self) {
	close(fd);
}

midi_writer_t sfx_device_midi_unixraw = {
	"unixraw-midi",
	&unixraw_init,
	&unixraw_set_option,
	&unixraw_write,
	&unixraw_delay,
	NULL,
	&unixraw_reset_timer,
	&unixraw_close
};
