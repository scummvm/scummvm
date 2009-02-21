/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sfx/sfx_iterator.h"
#include "sci/tools.h" /* for BREAKPOINT */
#include "sci/include/sci_memory.h"

namespace Sci {

#define D ((pcm_data_internal_t *)self->internal)

static int pi_poll(sfx_pcm_feed_t *self, byte *dest, int size);
static void pi_destroy(sfx_pcm_feed_t *self);

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


sfx_pcm_feed_t *sfx_iterator_make_feed(byte *base_data, int offset, int size, sfx_pcm_config_t conf) {
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


static int pi_poll(sfx_pcm_feed_t *self, byte *dest, int size) {
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

static void pi_destroy(sfx_pcm_feed_t *self) {
	sci_refcount_decref(D->base_data);
	free(D);
	free(self);
}

} // End of namespace Sci
