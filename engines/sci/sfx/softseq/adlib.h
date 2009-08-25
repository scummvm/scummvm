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

#include "sci/sfx/sci_midi.h"
#include "sound/fmopl.h"

namespace Sci {

class MidiDriver_Adlib : public MidiDriver_Emulated {
public:
	enum {
		kVoices = 9,
		kRhythmKeys = 62
	};

	MidiDriver_Adlib(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer), _playSwitch(true), _masterVolume(15), _rhythmKeyMap(0), _opl(0) { }
	~MidiDriver_Adlib() { }

	// MidiDriver
	int open(bool isSCI0);
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return _stereo; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	void setVolume(byte volume);
	void playSwitch(bool play);
	void loadResource(Resource *res);

private:
	enum ChannelID {
		kLeftChannel = 1,
		kRightChannel = 2
	};

	struct AdlibOperator {
		bool amplitudeMod;
		bool vibrato;
		bool envelopeType;
		bool kbScaleRate;
		byte frequencyMult;		// (0-15)
		byte kbScaleLevel;		// (0-3)
		byte totalLevel;		// (0-63, 0=max, 63=min)
		byte attackRate;		// (0-15)
		byte decayRate;			// (0-15)
		byte sustainLevel;		// (0-15)
		byte releaseRate;		// (0-15)
		byte waveForm;			// (0-3)
	};

	struct AdlibModulator {
		byte feedback;			// (0-7)
		bool algorithm;
	};

	struct AdlibPatch {
		AdlibOperator op[2];
		AdlibModulator mod;
	};

	struct Channel {
		uint8 patch;			// Patch setting
		uint8 volume;			// Channel volume (0-63)
		uint8 pan;				// Pan setting (0-127, 64 is center)
		uint8 holdPedal;		// Hold pedal setting (0 to 63 is off, 127 to 64 is on)
		uint8 extraVoices;		// The number of additional voices this channel optimally needs
		uint16 pitchWheel;		// Pitch wheel setting (0-16383, 8192 is center)
		uint8 lastVoice;		// Last voice used for this MIDI channel
		bool enableVelocity;	// Enable velocity control (SCI0)

		Channel() : patch(0), volume(63), pan(64), holdPedal(0), extraVoices(0),
					pitchWheel(8192), lastVoice(0), enableVelocity(false) { }
	};

	struct AdlibVoice {
		int8 channel;			// MIDI channel that this voice is assigned to or -1
		int8 note;				// Currently playing MIDI note or -1
		int patch;				// Currently playing patch or -1
		uint8 velocity;			// Note velocity
		bool isSustained;		// Flag indicating a note that is being sustained by the hold pedal
		uint16 age;				// Age of the current note

		AdlibVoice() : channel(-1), note(-1), patch(-1), velocity(0), isSustained(false), age(0) { }
	};

	bool _stereo;
	bool _isSCI0;
	OPL::OPL *_opl;
	bool _playSwitch;
	int _masterVolume;
	Channel _channels[MIDI_CHANNELS];
	AdlibVoice _voices[kVoices];
	byte *_rhythmKeyMap;
	Common::Array<AdlibPatch> _patches;

	void loadInstrument(const byte *ins);
	void voiceOn(int voice, int note, int velocity);
	void voiceOff(int voice);
	void setPatch(int voice, int patch);
	void setNote(int voice, int note, bool key);
	void setVelocity(int voice);
	void setOperator(int oper, AdlibOperator &op);
	void setRegister(int reg, int value, int channels = kLeftChannel | kRightChannel);
	void renewNotes(int channel, bool key);
	void noteOn(int channel, int note, int velocity);
	void noteOff(int channel, int note);
	int findVoice(int channel);
	void voiceMapping(int channel, int voices);
	void assignVoices(int channel, int voices);
	void releaseVoices(int channel, int voices);
	void donateVoices();
	int findVoiceBasic(int channel);
	void setVelocityReg(int regOffset, int velocity, int kbScaleLevel, int pan);
	int calcVelocity(int voice, int op);
};

class MidiPlayer_Adlib : public MidiPlayer {
public:
	MidiPlayer_Adlib() { _driver = new MidiDriver_Adlib(g_system->getMixer()); }
	int open(ResourceManager *resourceManager);
	int getPlayMask() const { return 0x04; }
	int getPolyphony() const { return MidiDriver_Adlib::kVoices; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_Adlib *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_Adlib *>(_driver)->playSwitch(play); }
	void loadInstrument(int idx, byte *data);
};

} // End of namespace Sci

