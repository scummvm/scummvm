#include "stdafx.h"
#include "scumm.h"
#include "cdmusic.h"

void SoundMixer::uninsert(Channel *chan) {

	for(int i=0; i!=NUM_CHANNELS; i++) {	
		if (_channels[i] == chan) {
			if (_handles[i]) {
				*_handles[i] = 0;
				_handles[i] = NULL;
			}
			_channels[i] = NULL;
			return;
		}
	}
	error("SoundMixer::channel_deleted chan not found");
}

void SoundMixer::insert(PlayingSoundHandle *handle, Channel *chan) {
	for(int i=0; i!=NUM_CHANNELS; i++) {
		if (_channels[i] == NULL) {
			_channels[i] = chan;
			_handles[i] = handle;
			if (handle)
				*handle = i + 1;
			return;
		}
	}
	
	warning("SoundMixer::insert out of mixer slots");
	chan->destroy();
}


void SoundMixer::play_raw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags) {
	insert(handle, new Channel_RAW(this, sound, size, rate, flags));
}

void SoundMixer::mix(int16 *buf, uint len) {
	if (_premix_proc) {
		_premix_proc(_premix_param, buf, len);
	} else {
		/* no premixer available, zero the buf out */
		memset(buf, 0, len * sizeof(int16));
	}

	/* now mix all channels */
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->mix(buf, len);
}

void SoundMixer::on_generate_samples(void *s, byte *samples, int len) {
	((SoundMixer*)s)->mix((int16*)samples, len>>1);
}

bool SoundMixer::bind_to_system(OSystem *syst) {
	_volume_table = (int16*)calloc(256*sizeof(int16),1);
		
	uint rate = (uint)syst->property(OSystem::PROP_GET_SAMPLE_RATE, 0);

	_output_rate = rate;
	
	if (rate == 0)
		error("OSystem returned invalid sample rate");
	
	return syst->set_sound_proc(this, on_generate_samples, OSystem::SOUND_16BIT);
}

void SoundMixer::stop_all() {
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			_channels[i]->destroy();
}

void SoundMixer::stop(PlayingSoundHandle psh) {
	if (psh && _channels[psh-1])
		_channels[psh-1]->destroy();
}


bool SoundMixer::has_active_channel() {
	for(int i=0; i!=NUM_CHANNELS; i++)
		if (_channels[i])
			return true;
	return false;
}

void SoundMixer::setup_premix(void *param, PremixProc *proc) {
	_premix_param = param;
	_premix_proc = proc;
}

void SoundMixer::set_volume(int volume) {
	for(int i=0; i!=256; i++)
		_volume_table[i] =((int8)i) * volume;
}


/* RAW mixer */
SoundMixer::Channel_RAW::Channel_RAW(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags) {
	_mixer = mixer;
	_flags = flags;
	_ptr = sound;
	_pos = 0;
	_fp_pos = 0;
	_fp_speed = (1 << 16) * rate / mixer->_output_rate;

	/* adjust the magnitute to prevent division error */
	while (size & 0xFFFF0000)
		size >>= 1, rate >>= 1;

	_size = size * mixer->_output_rate / rate;
}

void SoundMixer::Channel_RAW::mix(int16 *data, uint len) {
	byte *s, *s_org = NULL;
	uint32 fp_pos;

	if (len > _size)
		len = _size;
	_size -= len;

	/* 
	 * simple support for fread() reading of samples
	 */
	if (_flags & FLAG_FILE) {
		/* determine how many samples to read from the file */
		uint num = len * _fp_speed >> 16;

		s_org = (byte*)malloc(num);
		if (s_org == NULL)
			error("Channel_RAW::mix out of memory");
		
		uint num_read = fread(s_org, 1, num, (FILE*)_ptr);
		if (num - num_read != 0)
			memset(s_org + num_read, 0x80, num - num_read);
				
		s = s_org;
		fp_pos = 0;
	} else {
		s = (byte*)_ptr + _pos;
		fp_pos = _fp_pos;
	}

	const uint32 fp_speed = _fp_speed;
	const int16 *vol_tab = _mixer->_volume_table;

	if (_flags & FLAG_UNSIGNED) {
		do {
			fp_pos += fp_speed;
			*data++ += vol_tab[*s ^ 0x80];
			s += fp_pos >> 16;
			fp_pos &= 0x0000FFFF;
		} while (--len);
	} else {
		do {
			fp_pos += fp_speed;
			*data++ += vol_tab[*s];
			s += fp_pos >> 16;
			fp_pos &= 0x0000FFFF;
		} while (--len);
	}

	_pos = s - (byte*) _ptr;
	_fp_pos = fp_pos;

	if (_flags & FLAG_FILE) {
		free(s_org);
	}

	if (!_size)
		destroy();
}

void SoundMixer::Channel_RAW::destroy() {
	if (_flags & FLAG_AUTOFREE)
		free(_ptr);
	_mixer->uninsert(this);
	delete this;
}


/* MP3 mixer goes here */

#if 0

#ifdef COMPRESSED_SOUND_FILE
void Scumm::playSfxSound_MP3(void *sound, uint32 size)
{
	MixerChannel *mc = allocateMixer();

	if (!mc) {
		warning("No mixer channel available");
		return;
	}

	mc->type = MIXER_MP3;
	mc->_sfx_sound = sound;

	mad_stream_init(&mc->sound_data.mp3.stream);



#ifdef _WIN32_WCE

	// 11 kHz on WinCE

	mad_stream_options((mad_stream *) & mc->sound_data.mp3.stream,
										 MAD_OPTION_HALFSAMPLERATE);

#endif


	mad_frame_init(&mc->sound_data.mp3.frame);
	mad_synth_init(&mc->sound_data.mp3.synth);
	mc->sound_data.mp3.position = 0;
	mc->sound_data.mp3.pos_in_frame = 0xFFFFFFFF;
	mc->sound_data.mp3.size = size;
	/* This variable is the number of samples to cut at the start of the MP3
	   file. This is needed to have lip-sync as the MP3 file have some miliseconds
	   of blank at the start (as, I suppose, the MP3 compression algorithm need to
	   have some silence at the start to really be efficient and to not distort
	   too much the start of the sample).

	   This value was found by experimenting out. If you recompress differently your
	   .SO3 file, you may have to change this value.

	   When using Lame, it seems that the sound starts to have some volume about 50 ms
	   from the start of the sound => we skip about 1024 samples.
	 */
	mc->sound_data.mp3.silence_cut = 1024;
}
#endif

#ifdef COMPRESSED_SOUND_FILE
static inline int scale_sample(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize and scale to not saturate when mixing a lot of channels */
	return sample >> (MAD_F_FRACBITS + 2 - 16);
}
#endif

void MixerChannel::mix(int16 * data, uint32 len)
{
	if (!_sfx_sound)
		return;

#ifdef COMPRESSED_SOUND_FILE
	if (type == MIXER_STANDARD) {
#endif
		int8 *s;
		uint32 fp_pos, fp_speed;

		if (len > sound_data.standard._sfx_size)
			len = sound_data.standard._sfx_size;
		sound_data.standard._sfx_size -= len;

		s = (int8 *) _sfx_sound + sound_data.standard._sfx_pos;
		fp_pos = sound_data.standard._sfx_fp_pos;
		fp_speed = sound_data.standard._sfx_fp_speed;

		do {
			fp_pos += fp_speed;
			*data++ += (*s << 6);
			s += fp_pos >> 16;
			fp_pos &= 0x0000FFFF;
		} while (--len);

		sound_data.standard._sfx_pos = s - (int8 *) _sfx_sound;
		sound_data.standard._sfx_fp_speed = fp_speed;
		sound_data.standard._sfx_fp_pos = fp_pos;

		if (!sound_data.standard._sfx_size)
			clear();
#ifdef COMPRESSED_SOUND_FILE
	} else {
		if (type == MIXER_MP3) {
			mad_fixed_t const *ch;
			while (1) {
				ch =
					sound_data.mp3.synth.pcm.samples[0] + sound_data.mp3.pos_in_frame;
				while ((sound_data.mp3.pos_in_frame < sound_data.mp3.synth.pcm.length)
							 && (len > 0)) {
					if (sound_data.mp3.silence_cut > 0) {
						sound_data.mp3.silence_cut--;
					} else {
						*data++ += scale_sample(*ch++);
						len--;
					}
					sound_data.mp3.pos_in_frame++;
				}
				if (len == 0)
					return;

				if (sound_data.mp3.position >= sound_data.mp3.size) {
					clear();
					return;
				}

				mad_stream_buffer(&sound_data.mp3.stream,
													((unsigned char *)_sfx_sound) +
													sound_data.mp3.position,
													sound_data.mp3.size + MAD_BUFFER_GUARD -
													sound_data.mp3.position);

				if (mad_frame_decode(&sound_data.mp3.frame, &sound_data.mp3.stream) ==
						-1) {
					/* End of audio... */
					if (sound_data.mp3.stream.error == MAD_ERROR_BUFLEN) {
						clear();
						return;
					} else if (!MAD_RECOVERABLE(sound_data.mp3.stream.error)) {
						error("MAD frame decode error !");
					}
				}
				mad_synth_frame(&sound_data.mp3.synth, &sound_data.mp3.frame);
				sound_data.mp3.pos_in_frame = 0;
				sound_data.mp3.position =
					(unsigned char *)sound_data.mp3.stream.next_frame -
					(unsigned char *)_sfx_sound;
			}
		} else if (type == MIXER_MP3_CDMUSIC) {
			mad_fixed_t const *ch;
			mad_timer_t frame_duration;
			static long last_pos = 0;

			if (!sound_data.mp3_cdmusic.playing)
				return;

			while (1) {

				// See if we just skipped
				if (ftell(sound_data.mp3_cdmusic.file) != last_pos) {
					int skip_loop;

					// Read the new data
					memset(_sfx_sound, 0,
								 sound_data.mp3_cdmusic.buffer_size + MAD_BUFFER_GUARD);
					sound_data.mp3_cdmusic.size =
						fread(_sfx_sound, 1, sound_data.mp3_cdmusic.buffer_size,
									sound_data.mp3_cdmusic.file);
					if (!sound_data.mp3_cdmusic.size) {
						sound_data.mp3_cdmusic.playing = false;
						return;
					}
					last_pos = ftell(sound_data.mp3_cdmusic.file);
					// Resync
					mad_stream_buffer(&sound_data.mp3_cdmusic.stream,
														(unsigned char *)_sfx_sound,
														sound_data.mp3_cdmusic.size);
					skip_loop = 2;
					while (skip_loop != 0) {
						if (mad_frame_decode(&sound_data.mp3_cdmusic.frame,
																 &sound_data.mp3_cdmusic.stream) == 0) {
							/* Do not decrease duration - see if it's a problem */
							skip_loop--;
							if (skip_loop == 0) {
								mad_synth_frame(&sound_data.mp3_cdmusic.synth,
																&sound_data.mp3_cdmusic.frame);
							}
						} else {
							if (!MAD_RECOVERABLE(sound_data.mp3_cdmusic.stream.error)) {
								debug(1, "Unrecoverable error while skipping !");
								sound_data.mp3_cdmusic.playing = false;
								return;
							}
						}
					}
					// We are supposed to be in synch
					mad_frame_mute(&sound_data.mp3_cdmusic.frame);
					mad_synth_mute(&sound_data.mp3_cdmusic.synth);
					// Resume decoding
					if (mad_frame_decode(&sound_data.mp3_cdmusic.frame,
															 &sound_data.mp3_cdmusic.stream) == 0) {
						sound_data.mp3_cdmusic.position =
							(unsigned char *)sound_data.mp3_cdmusic.stream.next_frame -
							(unsigned char *)_sfx_sound;
						sound_data.mp3_cdmusic.pos_in_frame = 0;
					} else {
						sound_data.mp3_cdmusic.playing = false;
						return;
					}
				}
				// Get samples, play samples ... 

				ch = sound_data.mp3_cdmusic.synth.pcm.samples[0] +
					sound_data.mp3_cdmusic.pos_in_frame;
				while ((sound_data.mp3_cdmusic.pos_in_frame <
								sound_data.mp3_cdmusic.synth.pcm.length) && (len > 0)) {
					*data++ += scale_sample(*ch++);
					len--;
					sound_data.mp3_cdmusic.pos_in_frame++;
				}
				if (len == 0) {
					return;
				}
				// See if we have finished
				// May be incorrect to check the size at the end of a frame but I suppose
				// they are short enough :)   

				frame_duration = sound_data.mp3_cdmusic.frame.header.duration;

				mad_timer_negate(&frame_duration);
				mad_timer_add(&sound_data.mp3_cdmusic.duration, frame_duration);
				if (mad_timer_compare(sound_data.mp3_cdmusic.duration, mad_timer_zero)
						< 0) {
					sound_data.mp3_cdmusic.playing = false;
				}

				if (mad_frame_decode(&sound_data.mp3_cdmusic.frame,
														 &sound_data.mp3_cdmusic.stream) == -1) {

					if (sound_data.mp3_cdmusic.stream.error == MAD_ERROR_BUFLEN) {
						int not_decoded;

						if (!sound_data.mp3_cdmusic.stream.next_frame) {
							memset(_sfx_sound, 0,
										 sound_data.mp3_cdmusic.buffer_size + MAD_BUFFER_GUARD);
							sound_data.mp3_cdmusic.size =
								fread(_sfx_sound, 1, sound_data.mp3_cdmusic.buffer_size,
											sound_data.mp3_cdmusic.file);
							sound_data.mp3_cdmusic.position = 0;
							not_decoded = 0;
						} else {
							not_decoded = sound_data.mp3_cdmusic.stream.bufend -
								sound_data.mp3_cdmusic.stream.next_frame;
							memcpy(_sfx_sound, sound_data.mp3_cdmusic.stream.next_frame,
										 not_decoded);

							sound_data.mp3_cdmusic.size =
								fread((unsigned char *)_sfx_sound + not_decoded, 1,
											sound_data.mp3_cdmusic.buffer_size - not_decoded,
											sound_data.mp3_cdmusic.file);
						}
						last_pos = ftell(sound_data.mp3_cdmusic.file);
						sound_data.mp3_cdmusic.stream.error = MAD_ERROR_NONE;
						// Restream
						mad_stream_buffer(&sound_data.mp3_cdmusic.stream,
															(unsigned char *)_sfx_sound,
															sound_data.mp3_cdmusic.size + not_decoded);
						if (mad_frame_decode
								(&sound_data.mp3_cdmusic.frame,
								 &sound_data.mp3_cdmusic.stream) == -1) {
							debug(1, "Error decoding after restream %d !",
										sound_data.mp3.stream.error);
						}
					} else if (!MAD_RECOVERABLE(sound_data.mp3.stream.error)) {
						error("MAD frame decode error in MP3 CDMUSIC !");
					}
				}

				mad_synth_frame(&sound_data.mp3_cdmusic.synth,
												&sound_data.mp3_cdmusic.frame);
				sound_data.mp3_cdmusic.pos_in_frame = 0;
				sound_data.mp3_cdmusic.position =
					(unsigned char *)sound_data.mp3_cdmusic.stream.next_frame -
					(unsigned char *)_sfx_sound;
			}
		}
	}
#endif
}

void MixerChannel::clear()
{
	free(_sfx_sound);
	_sfx_sound = NULL;

#ifdef COMPRESSED_SOUND_FILE
	if (type == MIXER_MP3) {
		mad_synth_finish(&sound_data.mp3.synth);
		mad_frame_finish(&sound_data.mp3.frame);
		mad_stream_finish(&sound_data.mp3.stream);
	}
#endif
}

#endif


