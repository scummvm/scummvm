/***************************************************************************
 pcm-iterator.c  Copyright (C) 2002 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include "sci/include/sfx_iterator.h"
#include "sci/include/resource.h" /* for BREAKPOINT */
#include "sci/include/sci_memory.h"

#define D ((pcm_data_internal_t *)self->internal)

static int
pi_poll(sfx_pcm_feed_t *self, byte *dest, int size);
static void
pi_destroy(sfx_pcm_feed_t *self);

typedef struct {
	byte *base_data;
	byte *data;
	int frames_left;
} pcm_data_internal_t;


static sfx_pcm_feed_t pcm_it_prototype = {
	pi_poll,
	pi_destroy,
	NULL, /* No timestamp getter */
	NULL, /* Internal data goes here */
	{0, 0, 0}, /* Must fill in configuration */
	"song-iterator",
	0, /* Ideally the resource number should go here */
	0  /* The mixer computes this for us */
};


sfx_pcm_feed_t *
sfx_iterator_make_feed(byte *base_data,
                       int offset,
                       int size,
                       sfx_pcm_config_t conf) {
	sfx_pcm_feed_t *feed;
	pcm_data_internal_t *idat;
	byte *data = base_data + offset;

	if (!data) {
		/* Now this is silly; why'd you call this function in the first place? */
		return NULL;
	}
	sci_refcount_incref(base_data);

	idat = (pcm_data_internal_t*)sci_malloc(sizeof(pcm_data_internal_t));
	idat->base_data = base_data;
	idat->data = data;
	idat->frames_left = size;
	feed = (sfx_pcm_feed_t*)sci_malloc(sizeof(sfx_pcm_feed_t));
	*feed = pcm_it_prototype;
	feed->internal = idat;
	feed->conf = conf;

	return feed;
}


static int
pi_poll(sfx_pcm_feed_t *self, byte *dest, int size) {
	int data_len;

	if (size >= D->frames_left)
		size = D->frames_left;

	D->frames_left -= size;

	data_len = size * self->frame_size;

	memcpy(dest, D->data, data_len);
#if 0
	memset(dest, 0xff, data_len);
#endif

	D->data += data_len;

	return size;
}

static void
pi_destroy(sfx_pcm_feed_t *self) {
	sci_refcount_decref(D->base_data);
	free(D);
	free(self);
}
