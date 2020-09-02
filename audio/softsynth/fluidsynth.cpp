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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef USE_FLUIDSYNTH

// Fluidsynth v2.1+ uses printf in one of it's headers,
// include/fluidsynth/log.h around line 82 so need to include this
// prior scummsys.h inclusion and thus forbidden.h
#include <fluidsynth.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"
#include "audio/softsynth/emumidi.h"
#include "gui/message.h"
#if defined(IPHONE_IOS7) && defined(IPHONE_SANDBOXED)
#include "backends/platform/ios7/ios7_common.h"
#endif

class MidiDriver_FluidSynth : public MidiDriver_Emulated {
private:
	MidiChannel_MPU401 _midiChannels[16];
	fluid_settings_t *_settings;
	fluid_synth_t *_synth;
	int _soundFont;
	int _outputRate;
	Common::SeekableReadStream *_engineSoundFontData;

protected:
	// Because GCC complains about casting from const to non-const...
	void setInt(const char *name, int val);
	void setNum(const char *name, double num);
	void setStr(const char *name, const char *str);

	void generateSamples(int16 *buf, int len) override;

public:
	MidiDriver_FluidSynth(Audio::Mixer *mixer);

	int open() override;
	void close() override;
	void send(uint32 b) override;

	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

	void setEngineSoundFont(Common::SeekableReadStream *soundFontData) override;
	bool acceptsSoundFontData() override {
#if defined(FLUIDSYNTH_VERSION_MAJOR) && FLUIDSYNTH_VERSION_MAJOR > 1
		return true;
#else
		return false;
#endif
	}

	// AudioStream API
	bool isStereo() const override { return true; }
	int getRate() const override { return _outputRate; }
};

// MidiDriver method implementations

MidiDriver_FluidSynth::MidiDriver_FluidSynth(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer), _engineSoundFontData(nullptr) {

	for (int i = 0; i < ARRAYSIZE(_midiChannels); i++) {
		_midiChannels[i].init(this, i);
	}

	// It ought to be possible to get FluidSynth to generate samples at
	// lower

	_outputRate = _mixer->getOutputRate();
	if (_outputRate < 22050)
		_outputRate = 22050;
	else if (_outputRate > 96000)
		_outputRate = 96000;
}

// The string duplication below is there only because older versions (1.1.6
// and earlier?) of FluidSynth expected the string parameters to be non-const.

void MidiDriver_FluidSynth::setInt(const char *name, int val) {
	char *name2 = scumm_strdup(name);

	fluid_settings_setint(_settings, name2, val);
	free(name2);
}

void MidiDriver_FluidSynth::setNum(const char *name, double val) {
	char *name2 = scumm_strdup(name);

	fluid_settings_setnum(_settings, name2, val);
	free(name2);
}

void MidiDriver_FluidSynth::setStr(const char *name, const char *val) {
	char *name2 = scumm_strdup(name);
	char *val2 = scumm_strdup(val);

	fluid_settings_setstr(_settings, name2, val2);
	free(name2);
	free(val2);
}

// Soundfont memory loader callback functions.

#if defined(FLUIDSYNTH_VERSION_MAJOR) && FLUIDSYNTH_VERSION_MAJOR > 1
static void *SoundFontMemLoader_open(const char *filename) {
	void *p;
	if (filename[0] != '&') {
		return nullptr;
	}
	sscanf(filename, "&%p", &p);
	return p;
}

static int SoundFontMemLoader_read(void *buf, int count, void *handle) {
	return ((Common::SeekableReadStream *) handle)->read(buf, count) == (uint32)count ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontMemLoader_seek(void *handle, long offset, int origin) {
	return ((Common::SeekableReadStream *) handle)->seek(offset, origin) ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontMemLoader_close(void *handle) {
	delete (Common::SeekableReadStream *) handle;
	return FLUID_OK;
}

static long SoundFontMemLoader_tell(void *handle) {
	return ((Common::SeekableReadStream *) handle)->pos();
}
#endif

int MidiDriver_FluidSynth::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

#if defined(FLUIDSYNTH_VERSION_MAJOR) && FLUIDSYNTH_VERSION_MAJOR > 1
	// When provided with in-memory SoundFont data, only use the configured
	// SoundFont instead if it's explicitly configured on the current game.
	bool isUsingInMemorySoundFontData = _engineSoundFontData && !ConfMan.getActiveDomain()->contains("soundfont");
#else
	bool isUsingInMemorySoundFontData = false;
#endif

	if (!isUsingInMemorySoundFontData && !ConfMan.hasKey("soundfont")) {
		GUI::MessageDialog dialog(_("FluidSynth requires a 'soundfont' setting. Please specify it in ScummVM GUI on MIDI tab. Music is off."));
		dialog.runModal();
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	_settings = new_fluid_settings();

	// The default gain setting is ridiculously low - at least for me. This
	// cannot be fixed by ScummVM's volume settings because they can only
	// soften the sound, not amplify it, so instead we add an option to
	// adjust the gain of FluidSynth itself.

	double gain = (double)ConfMan.getInt("midi_gain") / 100.0;

	setNum("synth.gain", gain);
	setNum("synth.sample-rate", _outputRate);

	_synth = new_fluid_synth(_settings);

	if (ConfMan.getBool("fluidsynth_chorus_activate")) {
		fluid_synth_set_chorus_on(_synth, 1);

		int chorusNr = ConfMan.getInt("fluidsynth_chorus_nr");
		double chorusLevel = (double)ConfMan.getInt("fluidsynth_chorus_level") / 100.0;
		double chorusSpeed = (double)ConfMan.getInt("fluidsynth_chorus_speed") / 100.0;
		double chorusDepthMs = (double)ConfMan.getInt("fluidsynth_chorus_depth") / 10.0;

		Common::String chorusWaveForm = ConfMan.get("fluidsynth_chorus_waveform");
		int chorusType = FLUID_CHORUS_MOD_SINE;
		if (chorusWaveForm == "sine") {
			chorusType = FLUID_CHORUS_MOD_SINE;
		} else {
			chorusType = FLUID_CHORUS_MOD_TRIANGLE;
		}

		fluid_synth_set_chorus(_synth, chorusNr, chorusLevel, chorusSpeed, chorusDepthMs, chorusType);
	} else {
		fluid_synth_set_chorus_on(_synth, 0);
	}

	if (ConfMan.getBool("fluidsynth_reverb_activate")) {
		fluid_synth_set_reverb_on(_synth, 1);

		double reverbRoomSize = (double)ConfMan.getInt("fluidsynth_reverb_roomsize") / 100.0;
		double reverbDamping = (double)ConfMan.getInt("fluidsynth_reverb_damping") / 100.0;
		int reverbWidth = ConfMan.getInt("fluidsynth_reverb_width");
		double reverbLevel = (double)ConfMan.getInt("fluidsynth_reverb_level") / 100.0;

		fluid_synth_set_reverb(_synth, reverbRoomSize, reverbDamping, reverbWidth, reverbLevel);
	} else {
		fluid_synth_set_reverb_on(_synth, 0);
	}

	Common::String interpolation = ConfMan.get("fluidsynth_misc_interpolation");
	int interpMethod = FLUID_INTERP_4THORDER;

	if (interpolation == "none") {
		interpMethod = FLUID_INTERP_NONE;
	} else if (interpolation == "linear") {
		interpMethod = FLUID_INTERP_LINEAR;
	} else if (interpolation == "4th") {
		interpMethod = FLUID_INTERP_4THORDER;
	} else if (interpolation == "7th") {
		interpMethod = FLUID_INTERP_7THORDER;
	}

	fluid_synth_set_interp_method(_synth, -1, interpMethod);

	const char *soundfont = !isUsingInMemorySoundFontData ?
			ConfMan.get("soundfont").c_str() : Common::String::format("&%p", (void *)_engineSoundFontData).c_str();

#if defined(FLUIDSYNTH_VERSION_MAJOR) && FLUIDSYNTH_VERSION_MAJOR > 1
	if (isUsingInMemorySoundFontData) {
		fluid_sfloader_t *soundFontMemoryLoader = new_fluid_defsfloader(_settings);
		fluid_sfloader_set_callbacks(soundFontMemoryLoader,
									 SoundFontMemLoader_open,
									 SoundFontMemLoader_read,
									 SoundFontMemLoader_seek,
									 SoundFontMemLoader_tell,
									 SoundFontMemLoader_close);
		fluid_synth_add_sfloader(_synth, soundFontMemoryLoader);
	}
#endif

#if defined(IPHONE_IOS7) && defined(IPHONE_SANDBOXED)
	if (!isUsingInMemorySoundFontData) {
		// HACK: Due to the sandbox on non-jailbroken iOS devices, we need to deal
		// with the chroot filesystem. All the path selected by the user are
		// relative to the Document directory. So, we need to adjust the path to
		// reflect that.
		Common::String soundfont_fullpath = iOS7_getDocumentsDir();
		soundfont_fullpath += soundfont;
		_soundFont = fluid_synth_sfload(_synth, soundfont_fullpath.c_str(), 1);
	} else {
		_soundFont = fluid_synth_sfload(_synth, soundfont, 1);
	}
#else
	_soundFont = fluid_synth_sfload(_synth, soundfont, 1);
#endif

	if (_soundFont == -1) {
		GUI::MessageDialog dialog(_("FluidSynth: Failed loading custom SoundFont '%s'. Music is off."), soundfont);
		dialog.runModal();
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	return 0;
}

void MidiDriver_FluidSynth::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	_mixer->stopHandle(_mixerSoundHandle);

	if (_soundFont != -1)
		fluid_synth_sfunload(_synth, _soundFont, 1);

	delete_fluid_synth(_synth);
	delete_fluid_settings(_settings);
}

void MidiDriver_FluidSynth::send(uint32 b) {
	if (!_isOpen)
		return;

	midiDriverCommonSend(b);

	//byte param3 = (byte) ((b >> 24) & 0xFF);
	uint param2 = (byte) ((b >> 16) & 0xFF);
	uint param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	byte chan   = (byte) (b & 0x0F);

	switch (cmd) {
	case 0x80:	// Note Off
		fluid_synth_noteoff(_synth, chan, param1);
		break;
	case 0x90:	// Note On
		fluid_synth_noteon(_synth, chan, param1, param2);
		break;
	case 0xA0:	// Aftertouch
		break;
	case 0xB0:	// Control Change
		fluid_synth_cc(_synth, chan, param1, param2);
		break;
	case 0xC0:	// Program Change
		fluid_synth_program_change(_synth, chan, param1);
		break;
	case 0xD0:	// Channel Pressure
		break;
	case 0xE0:	// Pitch Bend
		fluid_synth_pitch_bend(_synth, chan, (param2 << 7) | param1);
		break;
	case 0xF0:	// SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		warning("MidiDriver_FluidSynth: Receiving SysEx command on a send() call");
		break;
	default:
		warning("MidiDriver_FluidSynth: Unknown send() command 0x%02X", cmd);
		break;
	}
}

MidiChannel *MidiDriver_FluidSynth::allocateChannel() {
	for (int i = 0; i < ARRAYSIZE(_midiChannels); i++) {
		if (i != 9 && _midiChannels[i].allocate())
			return &_midiChannels[i];
	}
	return NULL;
}

MidiChannel *MidiDriver_FluidSynth::getPercussionChannel() {
	return &_midiChannels[9];
}

void MidiDriver_FluidSynth::generateSamples(int16 *data, int len) {
	fluid_synth_write_s16(_synth, len, data, 0, 2, data, 1, 2);
}

void MidiDriver_FluidSynth::setEngineSoundFont(Common::SeekableReadStream *soundFontData) {
	_engineSoundFontData = soundFontData;
}


// Plugin interface

class FluidSynthMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "FluidSynth";
	}

	const char *getId() const {
		return "fluidsynth";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices FluidSynthMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error FluidSynthMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_FluidSynth(g_system->getMixer());

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(FLUIDSYNTH)
	//REGISTER_PLUGIN_DYNAMIC(FLUIDSYNTH, PLUGIN_TYPE_MUSIC, FluidSynthMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(FLUIDSYNTH, PLUGIN_TYPE_MUSIC, FluidSynthMusicPlugin);
//#endif

#endif
