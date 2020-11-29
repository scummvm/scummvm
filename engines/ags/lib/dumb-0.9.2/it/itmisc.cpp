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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/lib/dumb-0.9.2/dumb.h"
#include "common/textconsole.h"

namespace AGS3 {

DUMB_IT_SIGDATA *duh_get_it_sigdata(DUH *duh) {
	warning("TODO: duh_get_it_sigdata");
	return nullptr;
}

DUH_SIGRENDERER *duh_encapsulate_raw_sigrenderer(sigrenderer_t *vsigrenderer, DUH_SIGTYPE_DESC *desc, int n_channels, long pos) {
	warning("TODO: duh_encapsulate_raw_sigrenderer");
	return nullptr;
}

sigrenderer_t *duh_get_raw_sigrenderer(DUH_SIGRENDERER *sigrenderer, long type) {
	warning("TODO: duh_get_raw_sigrenderer");
	return nullptr;
}

void dumb_silence(sample_t *samples, long length) {
	warning("TODO: dumb_silence");
}

long dumb_resample(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size, float volume, float delta) {
	warning("TODO: dumb_resample");
	return 0;
}

void dumb_reset_resampler(DUMB_RESAMPLER *resampler, sample_t *src, long pos, long start, long end) {
	warning("TODO: dumb_reset_resampler");
}

sample_t dumb_resample_get_current_sample(DUMB_RESAMPLER *resampler, float volume) {
	warning("TODO: dumb_resample_get_current_sample");
	return (sample_t)0;
}

sample_t **create_sample_buffer(int n_channels, long length) {
	warning("TODO: create_sample_buffer");
	return nullptr;
}

void destroy_sample_buffer(sample_t **samples) {
	warning("TODO: destroy_sample_buffer");
}

} // namespace AGS3
