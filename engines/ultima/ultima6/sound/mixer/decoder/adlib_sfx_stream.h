#ifndef __AdLibSfxStream_h__
#define __AdLibSfxStream_h__
/* Created by Eric Fry
 * Copyright (C) 2013 The Nuvie Team
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
//#include <cstdio>
#include "ultima/shared/std/string.h"
//#include <list>


#include "audiostream.h"
#include "OplClass.h"

class OriginFXAdLibDriver;
class Configuration;


using std::string;

class AdLibSfxStream : public Audio::RewindableAudioStream {
public:
	AdLibSfxStream() {
		opl = NULL;
		duration = 0;
	}

	AdLibSfxStream(Configuration *cfg, int rate, uint8 channel, sint8 note, uint8 velocity, uint8 program_number, uint32 d);
	~AdLibSfxStream();

	int readBuffer(sint16 *buffer, const int numSamples);

	/** Is this a stereo stream? */
	bool isStereo() const {
		return true;
	}

	/** Sample rate of the stream. */
	int getRate() const {
		return opl->getRate();
	}

	bool rewind() {
		return false;
	}

	/**
	 * End of data reached? If this returns true, it means that at this
	 * time there is no data available in the stream. However there may be
	 * more data in the future.
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or stop.
	 */
	bool endOfData() const {
		if (total_samples_played >= duration) {
			return true;
		}
		return false;
	}

	uint32 getLengthInMsec() {
		return (uint32)(duration / (getRate() / 1000.0f));
	}
protected:

	OplClass *opl;
	OriginFXAdLibDriver *driver;
	uint32 duration;
	int interrupt_samples_left;
	int interrupt_rate;
	uint32 total_samples_played;
};

#endif /* __AdLibSfxStream_h__ */
