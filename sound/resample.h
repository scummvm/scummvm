/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_RESAMPLE_H
#define SOUND_RESAMPLE_H

#include "sound/rate.h"

typedef struct {
	byte priv[1024];
} eff_struct;
typedef eff_struct *eff_t;

/** High quality rate conversion algorithm, based on SoX (http://sox.sourceforge.net). */
class ResampleRateConverter : public RateConverter {
protected:
	eff_struct effp;
public:
	ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality);
	~ResampleRateConverter();
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
};


#endif
