/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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

class IMuseInternal;

class IMuse {
public:
	enum {
		PROP_TEMPO_BASE = 1,
	};

	  IMuse();
	 ~IMuse();

	void on_timer();							// For the MacOS 9 port only
	void pause(bool paused);
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

	static IMuse *create_adlib(OSystem *syst, SoundMixer *mixer) {
		return create(syst, NULL, mixer);
	}
	static IMuse *create_midi(OSystem *syst, MidiDriver *midi) {
		return create(syst, midi, NULL);
	}

private:
	IMuseInternal *_imuse;				// Pointer to the real imuse object
};

#define MAX_DIGITAL_CHANNELS 10

class IMuseDigital {
private:

	struct channel {
		int8 _volumeLeft;
		int8 _volumeRight;
		bool _isLoop;
		uint32 _offsetStop;
		uint32 _offsetJump;
		uint32 _offsetRegion;
		uint32 _offset;
		byte *_data;
		uint32 _freq;
		uint32 _channels;
		uint32 _bits;
		uint32 _size;
		int32 _idSound;
		uint32 _mixerSize;
		uint8 _mixerFlags;
		bool _used;
		bool _toBeRemoved;
		uint32 _mixerTrack;
	} _channel[MAX_DIGITAL_CHANNELS];

	Scumm * _scumm;

public:
	IMuseDigital(Scumm *scumm);
	~IMuseDigital();
	void handler();
	void startSound(int sound);
	void stopSound(int sound);
	int32 doCommand(int a, int b, int c, int d, int e, int f, int g, int h);
	int getSoundStatus(int sound);
};

