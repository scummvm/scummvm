/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#ifndef SOUND_ADPCM_H
#define SOUND_ADPCM_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/stream.h"


namespace Audio {

class AudioStream;

enum typesADPCM {
	kADPCMOki,
	kADPCMMSIma,
	kADPCMMS
};

AudioStream *makeADPCMStream(Common::SeekableReadStream *stream, uint32 size, typesADPCM type, int rate = 22050, int channels = 2, uint32 blockAlign = 0);

} // End of namespace Audio

#endif
