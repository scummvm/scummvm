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
 * This file contains the Sound Driver data structures etc.
 */

#ifndef TINSEL_SOUND_H
#define TINSEL_SOUND_H

#include "common/file.h"
#include "common/file.h"

#include "sound/mixer.h"

#include "tinsel/dw.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

#define MAXSAMPVOL 127

/*----------------------------------------------------------------------*\
|*				Function Prototypes			*|
\*----------------------------------------------------------------------*/

class SoundManager {
protected:

	//TinselEngine *_vm;	// TODO: Enable this once global _vm var is gone

	/** Sample handle */
	Audio::SoundHandle _handle;

	/** Sample index buffer and number of entries */
	uint32 *_sampleIndex;

	/** Number of entries in the sample index */
	long _sampleIndexLen;

	/** file stream for sample file */
	Common::File _sampleStream;

public:

	SoundManager(TinselEngine *vm);
	~SoundManager();

	bool playSample(int id, Audio::Mixer::SoundType type, Audio::SoundHandle *handle = 0);
	void stopAllSamples(void);		// Stops any currently playing sample
	
	bool sampleExists(int id);
	bool sampleIsPlaying(void);
	
	// TODO: Internal method, make this protected?
	void openSampleFiles(void);
};

} // end of namespace Tinsel

#endif	// TINSEL_SOUND_H
