/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef SOUND_VORBIS_H
#define SOUND_VORBIS_H

#include "stdafx.h"
#include "common/scummsys.h"

#ifdef USE_VORBIS

#include "sound/audiocd.h"
#include <vorbis/vorbisfile.h>

class AudioInputStream;
class File;

class VorbisTrackInfo : public DigitalTrackInfo {
private:
	File *_file;
	OggVorbis_File _ov_file;
	bool _error_flag;

public:
	VorbisTrackInfo(File *file);
	~VorbisTrackInfo();
	bool error() { return _error_flag; }
	void play(SoundMixer *mixer, PlayingSoundHandle *handle, int startFrame, int duration);
};


void playSfxSound_Vorbis(SoundMixer *mixer, File *file, uint32 size, PlayingSoundHandle *handle);

AudioInputStream *makeVorbisStream(OggVorbis_File *file, int duration);

#endif

#endif
