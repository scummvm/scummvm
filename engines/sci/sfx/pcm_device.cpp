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

#include "sci/sfx/sfx_time.h"
#include "sci/sfx/sfx_pcm.h"
#include "engines/engine.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

static int pcmout_scummvm_framesize;
static Audio::AppendableAudioStream * pcmout_scummvm_audiostream;
static Audio::SoundHandle pcmout_scummvm_sound_handle;


static int pcmout_scummvm_init(sfx_pcm_device_t *self) {
	int pcmout_scummvm_audiostream_flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
	pcmout_scummvm_audiostream_flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif

	self->buf_size = 2048 << 1;
	self->conf.rate = g_engine->_mixer->getOutputRate();
	self->conf.stereo = SFX_PCM_STEREO_LR;
	self->conf.format = SFX_PCM_FORMAT_S16_NATIVE;
	pcmout_scummvm_framesize = SFX_PCM_FRAME_SIZE(self->conf);

	pcmout_scummvm_audiostream = Audio::makeAppendableAudioStream(self->conf.rate, pcmout_scummvm_audiostream_flags);
	::g_engine->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &pcmout_scummvm_sound_handle, pcmout_scummvm_audiostream);

	return SFX_OK;
}

static int pcmout_scummvm_output(sfx_pcm_device_t *self, byte *buf, int count,
                                 sfx_timestamp_t *timestamp) {

	byte *__buf = new byte[count * pcmout_scummvm_framesize];

	memcpy(__buf, buf, count * pcmout_scummvm_framesize);

	pcmout_scummvm_audiostream->queueBuffer(__buf, count * pcmout_scummvm_framesize);

	return SFX_OK;
}


sfx_pcm_device_t sfx_pcm_driver_scummvm = {
	&pcmout_scummvm_init,
	&pcmout_scummvm_output,
	NULL,
	{0, 0, 0},
	0
};

} // End of namespace Sci
