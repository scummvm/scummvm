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

#if !defined(SCUMM_HE_ANIMATION_H) && defined(ENABLE_HE)
#define SCUMM_HE_ANIMATION_H

#include "common/file.h"

#include "graphics/video/smk_decoder.h"

#include "sound/mixer.h"

namespace Scumm {

class ScummEngine_v90he;

class MoviePlayer : public Graphics::SmackerDecoder {
	ScummEngine_v90he *_vm;

	Audio::Mixer *_mixer;

	Audio::SoundHandle _bgSound;
	Audio::AudioStream *_bgSoundStream;

	char baseName[40];
	uint32 _flags;
	uint32 _wizResNum;

public:
	MoviePlayer(ScummEngine_v90he *vm, Audio::Mixer *mixer);

	int getImageNum();
	int load(const char *filename, int flags, int image = 0);

	void copyFrameToBuffer(byte *dst, int dstType, uint x, uint y, uint pitch);
	void handleNextFrame();

protected:
	virtual void setPalette(byte *pal);
};

} // End of namespace Scumm

#endif
