#ifndef _mixer_h_included
#define _mixer_h_included

typedef uint32 PlayingSoundHandle;

class SoundMixer {
private:	
	class Channel {
	public:
		virtual void mix(int16 *data, uint len) = 0;
		virtual void destroy() = 0;
	};

	class Channel_RAW : public Channel {
		SoundMixer *_mixer;
		void *_ptr;
		uint32 _pos;
		uint32 _size;
		uint32 _fp_speed;
		uint32 _fp_pos;
		byte _flags;
		

	public:
		void mix(int16 *data, uint len);
		void destroy();

		Channel_RAW(SoundMixer *mixer, void *sound, uint32 size, uint rate, byte flags);
	};

#ifdef COMPRESSED_SOUND_FILE

	class Channel_RAW : public Channel {
		SoundMixer *_mixer;

	public:
		void mix(int16 *data, uint len);
		void destroy();

		Channel_MP3(SoundMixer *mixer, void *sound, uint rate);
	};

#endif

	static void on_generate_samples(void *s, byte *samples, int len);

public:
	typedef void PremixProc(void *param, int16 *data, uint len);

	uint _output_rate;

	int16 *_volume_table;

	enum {
		NUM_CHANNELS = 16,
	};

	void *_premix_param;
	PremixProc *_premix_proc;

	Channel *_channels[NUM_CHANNELS];
	PlayingSoundHandle *_handles[NUM_CHANNELS];
	
	void insert(PlayingSoundHandle *handle, Channel *chan);
	void uninsert(Channel *chan);

	/* start playing a raw sound */
	enum {
		FLAG_AUTOFREE = 1,
		FLAG_UNSIGNED = 2, /* unsigned samples */
		FLAG_FILE = 4,		 /* sound is a FILE * that's read from */
	};
	void play_raw(PlayingSoundHandle *handle, void *sound, uint32 size, uint rate, byte flags);

	/* Premix procedure, useful when using fmopl adlib */
	void setup_premix(void *param, PremixProc *proc);

	/* mix */
	void mix(int16 *buf, uint len);

	/* stop all currently playing sounds */
	void stop_all();

	/* stop playing a specific sound */
	void stop(PlayingSoundHandle psh);

	/* is any channel active? */
	bool has_active_channel();

	/* bind to the OSystem object => mixer will be
	 * invoked automatically when samples need
	 * to be generated */
	void bind_to_system(OSystem *syst);

	/* set the volume, 0-256 */
	void set_volume(int volume);

};


struct MP3OffsetTable {	/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};



#if 0
typedef enum {			/* Mixer types */
  MIXER_STANDARD,
  MIXER_MP3,
  MIXER_MP3_CDMUSIC
} MixerType;

struct MixerChannel {	/* Mixer Channel */
	void *_sfx_sound;
	MixerType type;
	union {
	  struct {
	    uint32 _sfx_pos;
	    uint32 _sfx_size;
	    uint32 _sfx_fp_speed;
	    uint32 _sfx_fp_pos;
	  } standard;
#ifdef COMPRESSED_SOUND_FILE
	  struct {
	    struct mad_stream stream;
	    struct mad_frame frame;
	    struct mad_synth synth;
	    uint32 silence_cut;
	    uint32 pos_in_frame;
	    uint32 position;
	    uint32 size;
	  } mp3;
	  struct {
            struct mad_stream stream;
            struct mad_frame frame;
            struct mad_synth synth;
            uint32 pos_in_frame;
            uint32 position;
            uint32 size;
            uint32 buffer_size;
            mad_timer_t duration;
            bool   playing;
            FILE   *file;
          } mp3_cdmusic;
#endif
	} sound_data;
	void mix(int16 *data, uint32 len);
	void clear();
};
#endif


#endif /* _mixer_h_included */
