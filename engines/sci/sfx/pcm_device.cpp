#include "sci/include/sfx_time.h"
#include "sci/include/sfx_pcm.h"
#include "engines/engine.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"


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
