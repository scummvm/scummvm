
//WARNING: These is only the public interface to the IMUSE class
//This is safe as long as none of the methods are virtual,
//and as long as no variables are put here.
//Removing the private parts from the public class definition,
//means that the internals of IMuse can change without having to
//recompile all files that depend on this interface.
//Also, it probably decreases compile times, since the IMuse specific
//classes only will be parsed once (which is when imuse.cpp is compiled)


//If you change stuff here, you *MUST* change stuff in imuse.cpp as well

class IMuse {
public:
	/* making a dummy constructor means that this object will never be
	 * instanciated on its own */

public:
	enum {
		PROP_TEMPO_BASE = 1,
		PROP_MT32_EMULATE = 2,
	};

	void on_timer();
	void pause(bool paused);
	int terminate();
	int save_or_load(Serializer *ser, Scumm *scumm);
	int set_music_volume(uint vol);
	int get_music_volume();
	int set_master_volume(uint vol);
	int get_master_volume();
	bool start_sound(int sound);
	int stop_sound(int sound);
	int stop_all_sounds();
	int get_sound_status(int sound);
	int32 do_command(int a, int b, int c, int d, int e, int f, int g, int h);
	int clear_queue();
	void setBase(byte **base);
	uint32 property(int prop, uint32 value);

	static IMuse *create(OSystem *syst, MidiDriver *midi, SoundMixer *mixer);

	static IMuse *create_adlib(OSystem *syst, SoundMixer *mixer) { return create(syst, NULL, mixer); }
	static IMuse *create_midi(OSystem *syst, MidiDriver *midi) { return create(syst, midi, NULL); }
};
