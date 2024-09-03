/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef USE_FLUIDSYNTH

// Fluidsynth v2.1+ uses printf in one of it's headers,
// include/fluidsynth/log.h around line 82 so need to include this
// prior scummsys.h inclusion and thus forbidden.h
#ifdef USE_FLUIDLITE
#include <fluidlite.h>

#define FLUID_OK (0)
#define FLUID_FAILED (-1)
#else
#include <fluidsynth.h>
#endif

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"
#include "audio/softsynth/emumidi.h"
#include "gui/message.h"
#include "backends/fs/fs-factory.h"
#ifdef ANDROID_BACKEND
#include "backends/fs/android/android-fs-factory.h"
#endif

// We assume here Fluidsynth minor will never be above 255 and
// that micro versions won't break API compatibility
// Older versions of FluidLite used FLUIDSYNTH_VERSION and now use FLUIDLITE_VERSION
#if defined(FLUIDSYNTH_VERSION_MAJOR) && defined(FLUIDSYNTH_VERSION_MINOR)
#define FS_API_VERSION ((FLUIDSYNTH_VERSION_MAJOR << 8) | FLUIDSYNTH_VERSION_MINOR)
#elif defined(FLUIDLITE_VERSION_MAJOR) && defined(FLUIDLITE_VERSION_MINOR)
#define FS_API_VERSION ((FLUIDLITE_VERSION_MAJOR << 8) | FLUIDLITE_VERSION_MINOR)
#else
#define FS_API_VERSION 0
#endif

#if FS_API_VERSION >= 0x0200
static void logHandler(int level, const char *message, void *data)
#else
static void logHandler(int level, char *message, void *data)
#endif
{
	switch (level) {
	case FLUID_PANIC:
		error("FluidSynth: %s", message);
		break;
	case FLUID_ERR:
		warning("FluidSynth: %s", message);
		break;
	case FLUID_WARN:
		debug(1, "FluidSynth: %s", message);
		break;
	case FLUID_INFO:
		debug(2, "FluidSynth: %s", message);
		break;
	case FLUID_DBG:
		debug(3, "FluidSynth: %s", message);
		break;
	default:
		fluid_default_log_function(level, message, data);
		break;
	}
}

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

	Common::Path getSoundFontPath() const;

public:
	MidiDriver_FluidSynth(Audio::Mixer *mixer);

	int open() override;
	void close() override;
	void send(uint32 b) override;

	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

	void setEngineSoundFont(Common::SeekableReadStream *soundFontData) override;
	bool acceptsSoundFontData() override;

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

#if defined(USE_FLUIDLITE) && FS_API_VERSION >= 0x0102

#define FS_HAS_STREAM_SUPPORT

// FluidLite calls fopen and fclose callback twice which causes a double delete
// So, use a holder which will take care of use count
// Luckily the open() calls are not intermixed and we don't need to maintain state
struct fluidlite_stream_holder {
	Common::SeekableReadStream *stream;
	unsigned int openCounter;
};

static void *SoundFontMemLoader_open(fluid_fileapi_t *fileapi, const char *filename) {
	fluidlite_stream_holder *holder;
	if (filename[0] != '&') {
		return nullptr;
	}
	sscanf(filename, "&%p", (void **)&holder);

	// Reset the file cursor
	holder->stream->seek(0, SEEK_SET);
	holder->openCounter++;
	return holder;
}

static int SoundFontMemLoader_read(void *buf, int count, void *handle) {
	fluidlite_stream_holder *holder = (fluidlite_stream_holder *)handle;
	return holder->stream->read(buf, count) == (uint32)count ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontMemLoader_seek(void *handle, long offset, int origin) {
	fluidlite_stream_holder *holder = (fluidlite_stream_holder *)handle;
	return holder->stream->seek(offset, origin) ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontMemLoader_close(void *handle) {
	fluidlite_stream_holder *holder = (fluidlite_stream_holder *)handle;
	if (!--holder->openCounter) {
		delete holder->stream;
		delete holder;
	}
	return FLUID_OK;
}

static long SoundFontMemLoader_tell(void *handle) {
	fluidlite_stream_holder *holder = (fluidlite_stream_holder *)handle;
	return holder->stream->pos();
}

static const fluid_fileapi_t SoundFontMemLoader_callbacks = {
  NULL,
  NULL,
  SoundFontMemLoader_open,
  SoundFontMemLoader_read,
  SoundFontMemLoader_seek,
  SoundFontMemLoader_close,
  SoundFontMemLoader_tell
};

#elif FS_API_VERSION >= 0x0200

#define FS_HAS_STREAM_SUPPORT

static void *SoundFontMemLoader_open(const char *filename) {
	void *p;
	if (filename[0] != '&') {
		return nullptr;
	}
	sscanf(filename, "&%p", &p);
	return p;
}

#if FS_API_VERSION >= 0x0202
static int SoundFontMemLoader_read(void *buf, fluid_long_long_t count, void *handle) {
#else
static int SoundFontMemLoader_read(void *buf, int count, void *handle) {
#endif
	return ((Common::SeekableReadStream *) handle)->read(buf, count) == (uint32)count ? FLUID_OK : FLUID_FAILED;
}

#if FS_API_VERSION >= 0x0202
static int SoundFontMemLoader_seek(void *handle, fluid_long_long_t offset, int origin) {
#else
static int SoundFontMemLoader_seek(void *handle, long offset, int origin) {
#endif
	return ((Common::SeekableReadStream *) handle)->seek(offset, origin) ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontMemLoader_close(void *handle) {
	delete (Common::SeekableReadStream *) handle;
	return FLUID_OK;
}

#if FS_API_VERSION >= 0x0202
static fluid_long_long_t SoundFontMemLoader_tell(void *handle) {
#else
static long SoundFontMemLoader_tell(void *handle) {
#endif
	return ((Common::SeekableReadStream *) handle)->pos();
}

#endif // USE_FLUIDLITE

Common::Path MidiDriver_FluidSynth::getSoundFontPath() const {
	Common::Path path = ConfMan.getPath("soundfont");
	if (path.empty())
		return path;

	Common::FSNode fileNode(path);
	if (fileNode.exists()) {
		// Return the full system path to the soundfont
		return Common::Path(g_system->getFilesystemFactory()->getSystemFullPath(path.toString(Common::Path::kNativeSeparator)), Common::Path::kNativeSeparator);
	}

	// Then check with soundfontpath
	if (ConfMan.hasKey("soundfontpath")) {
		Common::FSNode dirNode(ConfMan.getPath("soundfontpath"));
		if (dirNode.exists() && dirNode.isDirectory()) {
			fileNode = dirNode.getChild(path.baseName());
			if (fileNode.exists())
				return fileNode.getPath();
		}
	}

	// Finally look for it with SearchMan
	Common::ArchiveMemberDetailsList files;
	SearchMan.listMatchingMembers(files, path);
	for (Common::ArchiveMemberDetails file : files) {
		Common::FSDirectory* dir = dynamic_cast<Common::FSDirectory*>(SearchMan.getArchive(file.arcName));
		if (!dir)
			continue;
		fileNode = dir->getFSNode().getChild(file.arcMember->getPathInArchive().toString(Common::Path::kNativeSeparator));
		if (fileNode.exists())
			return fileNode.getPath();
	}

	return path;
}

int MidiDriver_FluidSynth::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	fluid_set_log_function(FLUID_PANIC, logHandler, nullptr);
	fluid_set_log_function(FLUID_ERR, logHandler, nullptr);
	fluid_set_log_function(FLUID_WARN, logHandler, nullptr);
	fluid_set_log_function(FLUID_INFO, logHandler, nullptr);
	fluid_set_log_function(FLUID_DBG, logHandler, nullptr);

#ifdef FS_HAS_STREAM_SUPPORT
	// When provided with in-memory SoundFont data, only use the configured
	// SoundFont instead if it's explicitly configured on the current game.
	bool isUsingInMemorySoundFontData = _engineSoundFontData && !ConfMan.getActiveDomain()->contains("soundfont");
#else
	const bool isUsingInMemorySoundFontData = false;
#endif

	if (!isUsingInMemorySoundFontData && ConfMan.get("soundfont").empty()) {
		GUI::MessageDialog dialog(_("FluidSynth requires a 'soundfont' setting. Please specify it in ScummVM GUI on MIDI tab. Music is off."));
		dialog.runModal();
		return MERR_DEVICE_NOT_AVAILABLE;
	}

#if defined(ANDROID_BACKEND) && defined(FS_HAS_STREAM_SUPPORT)
	// In Android, when using SAF we need to wrap IO to make it work
	// We can only do this with FluidSynth 2.0
	if (!isUsingInMemorySoundFontData &&
			AndroidFilesystemFactory::instance().hasSAF()) {
		Common::FSNode fsnode(getSoundFontPath());
		_engineSoundFontData = fsnode.createReadStream();
		isUsingInMemorySoundFontData = _engineSoundFontData != nullptr;
	}
#endif

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
#if FS_API_VERSION >= 0x0202
		fluid_synth_chorus_on(_synth, -1, 1);
#else
		fluid_synth_set_chorus_on(_synth, 1);
#endif

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

#if FS_API_VERSION >= 0x0202
		fluid_synth_set_chorus_group_nr(_synth, -1, chorusNr);
		fluid_synth_set_chorus_group_level(_synth, -1, chorusLevel);
		fluid_synth_set_chorus_group_speed(_synth, -1, chorusSpeed);
		fluid_synth_set_chorus_group_depth(_synth, -1, chorusDepthMs);
		fluid_synth_set_chorus_group_type(_synth, -1, chorusType);
#else
		fluid_synth_set_chorus(_synth, chorusNr, chorusLevel, chorusSpeed, chorusDepthMs, chorusType);
#endif
	} else {
#if FS_API_VERSION >= 0x0202
		fluid_synth_chorus_on(_synth, -1, 0);
#else
		fluid_synth_set_chorus_on(_synth, 0);
#endif
	}

	if (ConfMan.getBool("fluidsynth_reverb_activate")) {
#if FS_API_VERSION >= 0x0202
		fluid_synth_reverb_on(_synth, -1, 1);
#else
		fluid_synth_set_reverb_on(_synth, 1);
#endif

		double reverbRoomSize = (double)ConfMan.getInt("fluidsynth_reverb_roomsize") / 100.0;
		double reverbDamping = (double)ConfMan.getInt("fluidsynth_reverb_damping") / 100.0;
		double reverbWidth = ConfMan.getInt("fluidsynth_reverb_width") / 10.0;
		double reverbLevel = (double)ConfMan.getInt("fluidsynth_reverb_level") / 100.0;

#if FS_API_VERSION >= 0x0202
		fluid_synth_set_reverb_group_roomsize(_synth, -1, reverbRoomSize);
		fluid_synth_set_reverb_group_damp(_synth, -1, reverbDamping);
		fluid_synth_set_reverb_group_width(_synth, -1, reverbWidth);
		fluid_synth_set_reverb_group_level(_synth, -1, reverbLevel);
#else
		fluid_synth_set_reverb(_synth, reverbRoomSize, reverbDamping, reverbWidth, reverbLevel);
#endif
	} else {
#if FS_API_VERSION >= 0x0202
		fluid_synth_reverb_on(_synth, -1, 0);
#else
		fluid_synth_set_reverb_on(_synth, 0);
#endif
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

	Common::String soundfont;

#if defined(FS_HAS_STREAM_SUPPORT)
	if (isUsingInMemorySoundFontData) {
#if defined(USE_FLUIDLITE)
		fluidlite_stream_holder *holder = new fluidlite_stream_holder;
		holder->stream = _engineSoundFontData;
		holder->openCounter = 0;

		fluid_sfloader_t *soundFontMemoryLoader = new_fluid_defsfloader();
		soundFontMemoryLoader->fileapi = const_cast<fluid_fileapi_t *>(&SoundFontMemLoader_callbacks);
		fluid_synth_add_sfloader(_synth, soundFontMemoryLoader);

		soundfont = Common::String::format("&%p", (void *)holder);
#else
		// Fluidsynth 2.0+
		fluid_sfloader_t *soundFontMemoryLoader = new_fluid_defsfloader(_settings);
		fluid_sfloader_set_callbacks(soundFontMemoryLoader,
									 SoundFontMemLoader_open,
									 SoundFontMemLoader_read,
									 SoundFontMemLoader_seek,
									 SoundFontMemLoader_tell,
									 SoundFontMemLoader_close);
		fluid_synth_add_sfloader(_synth, soundFontMemoryLoader);

		soundfont = Common::String::format("&%p", (void *)_engineSoundFontData);
#endif
	} else
#endif // FS_HAS_STREAM_SUPPORT
	{
//		soundfont = ConfMan.get("soundfont");
		soundfont = getSoundFontPath().toString(Common::Path::kNativeSeparator);
	}

	_soundFont = fluid_synth_sfload(_synth, soundfont.c_str(), 1);

	if (_soundFont == -1) {
		GUI::MessageDialog dialog(Common::U32String::format(_("FluidSynth: Failed loading custom SoundFont '%s'. Music is off."), soundfont.c_str()));
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
	return nullptr;
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

bool MidiDriver_FluidSynth::acceptsSoundFontData() {
#ifdef FS_HAS_STREAM_SUPPORT
	return true;
#else
	return false;
#endif
}

// Plugin interface

class FluidSynthMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const override {
		return "FluidSynth";
	}

	const char *getId() const override {
		return "fluidsynth";
	}

	MusicDevices getDevices() const override;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const override;
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
