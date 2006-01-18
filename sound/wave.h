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
 * $Header$
 *
 */

#ifndef SOUND_WAVE_H
#define SOUND_WAVE_H

#include "common/stdafx.h"
#include "common/scummsys.h"

class AudioStream;
namespace Common { class SeekableReadStream; }

/**
 * Try to load a WAVE from the given seekable stream. Returns true if successful; in that case,
 * the stream will point at the start of the audio data, and size, rate and flags contain
 * all information about the data necessary for playback.
 * Currently this only support uncompressed raw PCM data.
 */
extern bool loadWAVFromStream(Common::SeekableReadStream &stream, int &size, int &rate, byte &flags, uint16 *wavType = 0, int *blockAlign = 0);

AudioStream *makeWAVStream(Common::SeekableReadStream &stream);

#endif
