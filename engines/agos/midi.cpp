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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/memstream.h"

#include "agos/agos.h"
#include "agos/midi.h"

#include "agos/midiparser_gmf.h"
#include "agos/midiparser_simonwin.h"

#include "agos/drivers/accolade/mididriver.h"
#include "agos/drivers/accolade/adlib.h"
#include "agos/drivers/accolade/mt32.h"
#include "agos/drivers/simon1/adlib.h"
#include "agos/drivers/simon1/adlib_win.h"
// Miles Audio for Simon 2
#include "audio/miles.h"
#include "audio/midiparser.h"

// PKWARE data compression library decompressor required for Simon 2
#include "common/dcl.h"
#include "common/translation.h"

#include "gui/message.h"
#include <engines/agos/intern_detection.h>

namespace AGOS {


// MidiParser_S1D is not considered part of the standard
// MidiParser suite, but we still try to mask its details
// and just provide a factory function.
extern MidiParser *MidiParser_createS1D();

MidiPlayer::MidiPlayer(AGOSEngine *vm) {
	// Since initialize() is called every time the music changes,
	// this is where we'll initialize stuff that must persist
	// between songs.
	_vm = vm;
	_driver = nullptr;
	_driverMsMusic = nullptr;
	_driverMsSfx = nullptr;
	_map_mt32_to_gm = false;

	_current = nullptr;

	_musicVolume = 255;
	_sfxVolume = 255;

	resetVolumeTable();
	_paused = false;

	_currentTrack = 255;
	_loopTrack = 0;
	_queuedTrack = 255;
	_loopQueuedTrack = 0;

	_musicMode = kMusicModeDisabled;
	_deviceType = MT_NULL;
	_dataType = MT_NULL;

	_parserMusic = nullptr;
	_parserSfx = nullptr;
	_musicData = nullptr;
	_sfxData = nullptr;
}

MidiPlayer::~MidiPlayer() {
	stop();
	stopSfx();

	Common::StackLock lock(_mutex);

	if (_driverMsSfx && _driverMsSfx != _driverMsMusic) {
		_driverMsSfx->setTimerCallback(nullptr, nullptr);
		_driverMsSfx->close();
		delete _driverMsSfx;
		_driverMsSfx = nullptr;
	}
	if (_driverMsMusic) {
		_driverMsMusic->setTimerCallback(nullptr, nullptr);
		_driverMsMusic->close();
		delete _driverMsMusic;
		_driverMsMusic = nullptr;
	} else if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
		delete _driver;
		_driver = nullptr;
	}

	if (_parserMusic)
		delete _parserMusic;
	if (_parserSfx)
		delete _parserSfx;

	if (_musicData)
		delete[] _musicData;
	if (_sfxData)
		delete[] _sfxData;

	clearConstructs();
}

int MidiPlayer::open(int gameType, Common::Platform platform, bool isDemo) {
	// Don't call open() twice!
	assert(!_driver);

	Common::String accoladeDriverFilename;
	// All games have MT-32 data, except Simon 2, which has both GM and MT-32
	// data for DOS, or only GM for Windows. Some of the GM tracks have extra
	// instruments compared to the MT-32 tracks, so we prefer GM.
	int devFlags = MDT_MIDI | MDT_ADLIB | (_vm->getGameType() == GType_SIMON2 ? MDT_PREFER_GM : MDT_PREFER_MT32);

	if (_vm->getGameType() == GType_SIMON1) {
		// Check the type of device that the user has configured.
		MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(devFlags);
		_deviceType = MidiDriver::getMusicType(dev);
		if (_deviceType == MT_GM && ConfMan.getBool("native_mt32"))
			_deviceType = MT_MT32;

		// All versions of Simon 1 that use MIDI have data that targets the
		// MT-32 (even the Windows and Acorn versions).
		_dataType = MT_MT32;

		if (_dataType == MT_MT32 && _deviceType == MT_GM) {
			// Not a real MT32 / no MUNT
			::GUI::MessageDialog dialog(_(
				"You appear to be using a General MIDI device,\n"
				"but your game only supports Roland MT32 MIDI.\n"
				"We try to map the Roland MT32 instruments to\n"
				"General MIDI ones. It is still possible that\n"
				"some tracks sound incorrect."));
			dialog.runModal();
		}

		if (_vm->getPlatform() == Common::kPlatformDOS &&
				(_vm->getFeatures() & GF_DEMO) && !(_vm->getFeatures() & GF_TALKIE)) {
			// DOS floppy demo uses the older Accolade music drivers.
			_musicMode = kMusicModeAccolade;
			accoladeDriverFilename = "MUSIC.DRV";

			switch (_deviceType) {
			case MT_ADLIB:
				_driver = MidiDriver_Accolade_AdLib_create(accoladeDriverFilename);
				break;
			case MT_MT32:
				_driver = MidiDriver_Accolade_MT32_create(accoladeDriverFilename);
				break;
			default:
				_driver = new MidiDriver_NULL_Multisource();
				break;
			}

			// Create the MIDI parser for the MUS format used by this version.
			_parserMusic = MidiParser_createS1D();
			_parserMusic->property(MidiParser::mpDisableAutoStartPlayback, true);

			// Open the MIDI driver.
			int returnCode = _driver->open();
			if (returnCode != 0)
				error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);

			// Connect the driver and the parser.
			_parserMusic->setMidiDriver(_driver);
			_parserMusic->setTimerRate(_driver->getBaseTempo());

			_driver->setTimerCallback(_parserMusic, &_parserMusic->timerCallback);

			return 0;
		} else {
			// Create the correct driver(s) for the device type and platform.
			switch (_deviceType) {
			case MT_ADLIB:
				if (_vm->getPlatform() == Common::kPlatformDOS) {
					// DOS versions use a specific AdLib driver implementation
					// that needs an instrument bank file.
					_driverMsMusic = createMidiDriverSimon1AdLib("MT_FM.IBK");
					if (!(_vm->getFeatures() & GF_TALKIE)) {
						// DOS floppy version has MIDI SFX for AdLib.
						_driverMsSfx = _driverMsMusic;
					}
				} else {
					// Windows and Acorn CD
					// TODO Acorn does not use an OPL chip, but we don't have
					// an implementation for the Acorn audio. It has the same
					// music data has the DOS CD version, but it does not have
					// the MT_FM.IBK instrument bank, so we use the standard
					// OPL MIDI driver.
					_driverMsMusic = new MidiDriver_Simon1_AdLib_Windows();
				}
				break;
			case MT_MT32:
			case MT_GM:
				_driverMsMusic = new MidiDriver_MT32GM(_dataType);
				if (_vm->getPlatform() == Common::kPlatformDOS && !(_vm->getFeatures() & GF_TALKIE) &&
						ConfMan.getBool("multi_midi")) {
					// DOS floppy version uses AdLib MIDI SFX if mixed MIDI
					// mode is active.
					_driverMsSfx = createMidiDriverSimon1AdLib("MT_FM.IBK");
				}
				break;
			default:
				_driverMsMusic = new MidiDriver_NULL_Multisource();
				break;
			}
			_driver = _driverMsMusic;

			// WORKAROUND The Simon 1 MIDI data does not always set a value for
			// program, volume or panning before it starts playing notes on a
			// MIDI channel. This can cause settings for these parameters from
			// a previous track to be used unintentionally. To correct this,
			// default values for these parameters are set when a new track is
			// started.
			_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM);
			_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_VOLUME);
			_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PANNING);
			_driverMsMusic->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);

			// Create the MIDI parser(s) for the format used by the platform.
			if (_vm->getPlatform() == Common::kPlatformWindows) {
				// Windows version uses a specific SMF variant.
				_parserMusic = new MidiParser_SimonWin(0, true);
			} else {
				// DOS floppy & CD and Acorn CD use GMF (also an SMF variant).
				_parserMusic = new MidiParser_GMF(0);

				if (_driverMsSfx) {
					// DOS floppy needs a second parser for AdLib SFX.
					_parserSfx = new MidiParser_GMF(1);
					_parserMusic->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
					_parserSfx->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
					_parserSfx->property(MidiParser::mpDisableAutoStartPlayback, true);
				}
			}
			_parserMusic->property(MidiParser::mpDisableAutoStartPlayback, true);

			// Open the MIDI driver(s).
			int returnCode = _driverMsMusic->open();
			if (returnCode != 0)
				error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);
			_driverMsMusic->syncSoundSettings();
			if (_driverMsSfx && _driverMsMusic != _driverMsSfx) {
				_driverMsSfx->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
				returnCode = _driverMsSfx->open();
				if (returnCode != 0)
					error("MidiPlayer::open - Failed to open MIDI SFX driver - error code %d.", returnCode);
				_driverMsSfx->syncSoundSettings();
			}

			// Connect the driver(s) and parser(s).
			_parserMusic->setMidiDriver(_driverMsMusic);
			_parserMusic->setTimerRate(_driverMsMusic->getBaseTempo());
			if (_parserSfx) {
				_parserSfx->setMidiDriver(_driverMsSfx);
				_parserSfx->setTimerRate(_driverMsSfx->getBaseTempo());
			}

			if (_driverMsSfx == _driverMsMusic) {
				// Use MidiPlayer::onTimer to trigger both parsers from the
				// single driver (it can only have one timer callback).
				_driverMsMusic->setTimerCallback(this, &onTimer);
			} else {
				// Trigger each parser callback from the corresponding driver.
				_driverMsMusic->setTimerCallback(_parserMusic, &_parserMusic->timerCallback);
				if (_driverMsSfx) {
					_driverMsSfx->setTimerCallback(_parserSfx, &_parserSfx->timerCallback);
				}
			}

			return 0;
		}
	} else if (_vm->getGameType() == GType_SIMON2) {
		// Check the type of device that the user has configured.
		MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(devFlags);
		_deviceType = MidiDriver::getMusicType(dev);
		if (_deviceType == MT_GM && ConfMan.getBool("native_mt32"))
			_deviceType = MT_MT32;

		// DOS version has both MT-32 and GM tracks; Windows is GM only.
		_dataType = (_vm->getPlatform() == Common::kPlatformDOS && _deviceType == MT_MT32) ? MT_MT32 : MT_GM;

		switch (_deviceType) {
		case MT_ADLIB:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				if (Common::File::exists("MIDPAK.AD")) {
					// if there is a file called MIDPAK.AD, use it directly
					warning("MidiPlayer::open - SIMON 2: using MIDPAK.AD");
					_driverMsMusic = Audio::MidiDriver_Miles_AdLib_create("MIDPAK.AD", "");
				} else {
					// if there is no file called MIDPAK.AD, try to extract it from the file SETUP.SHR
					// if we didn't do this, the user would be forced to "install" the game instead of simply
					// copying all files from CD-ROM.
					Common::SeekableReadStream *midpakAdLibStream = simon2SetupExtractFile("MIDPAK.AD");
					if (!midpakAdLibStream)
						error("MidiPlayer::open - Could not extract MIDPAK.AD from SETUP.SHR");

					// Pass this extracted data to the driver
					warning("MidiPlayer::open - SIMON 2: using MIDPAK.AD extracted from SETUP.SHR");
					_driverMsMusic = Audio::MidiDriver_Miles_AdLib_create("", "", midpakAdLibStream);
					delete midpakAdLibStream;
				}
			} else {
				// Windows
				_driverMsMusic = new MidiDriver_ADLIB_Multisource(OPL::Config::kOpl3);
			}
			break;
		case MT_MT32:
		case MT_GM:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				_driverMsMusic = Audio::MidiDriver_Miles_MIDI_create(_dataType, "");
			} else {
				// Windows
				_driverMsMusic = new MidiDriver_MT32GM(_dataType);
			}
			break;
		default:
			_driverMsMusic = new MidiDriver_NULL_Multisource();
			break;
		}
		_driver = _driverMsMusic;
		_driverMsMusic->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);

		// Create the MIDI parser(s) for the format used by the platform.
		if (_vm->getPlatform() == Common::kPlatformDOS) {
			_parserMusic = MidiParser::createParser_XMIDI(MidiParser::defaultXMidiCallback, 0, 0);
		} else {
			// Windows version uses a specific SMF variant.
			_parserMusic = new MidiParser_SimonWin(0);
		}
		_parserMusic->property(MidiParser::mpDisableAutoStartPlayback, true);

		// Open the MIDI driver.
		int returnCode = _driverMsMusic->open();
		if (returnCode != 0)
			error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);
		_driverMsMusic->syncSoundSettings();

		// Connect the driver and parser.
		_parserMusic->setMidiDriver(_driverMsMusic);
		_parserMusic->setTimerRate(_driverMsMusic->getBaseTempo());
		_driverMsMusic->setTimerCallback(this, &onTimer);

		return 0;
	}

	// TODO Old code below is no longer used for Simon 1 and 2.
	_deviceType = MT_INVALID;

	switch (gameType) {
	case GType_ELVIRA1:
		if (platform == Common::kPlatformPC98) {
			_musicMode = kMusicModePC98;
			devFlags = (devFlags & ~MDT_ADLIB) | MDT_PC98;
		} else {
			_musicMode = kMusicModeAccolade;
			accoladeDriverFilename = "INSTR.DAT";
		}
		break;
	case GType_ELVIRA2:
	case GType_WW:
		// Attention: Elvira 2 shipped with INSTR.DAT and MUSIC.DRV
		// MUSIC.DRV is the correct one. INSTR.DAT seems to be a left-over
		_musicMode = kMusicModeAccolade;
		accoladeDriverFilename = "MUSIC.DRV";
		break;
	case GType_SIMON1:
		if (isDemo) {
			_musicMode = kMusicModeAccolade;
			accoladeDriverFilename = "MUSIC.DRV";
		} else if (Common::File::exists("MT_FM.IBK")) {
			_musicMode = kMusicModeSimon1;
		}
		break;
	case GType_SIMON2:
		//_musicMode = kMusicModeMilesAudio;
		// currently disabled, because there are a few issues
		// MT32 seems to work fine now, AdLib seems to use bad instruments and is also outputting music on
		// the right speaker only. The original driver did initialize the panning to 0 and the Simon2 XMIDI
		// tracks don't set panning at all. We can reset panning to be centered, which would solve this
		// issue, but we still don't know who's setting it in the original interpreter.
		break;
	default:
		break;
	}

	MidiDriver::DeviceHandle dev;
	int ret = 0;

	if (_musicMode == kMusicModePC98) {
		dev = MidiDriver::detectDevice(devFlags);
		_driver = MidiDriverPC98_create(dev);
		if (_driver && !_driver->open()) {
			_driver->setTimerCallback(this, &onTimer);
			return 0;
		}
	} else if (_musicMode != kMusicModeDisabled) {
		dev = MidiDriver::detectDevice(devFlags);
		_deviceType = MidiDriver::getMusicType(dev);

		switch (_deviceType) {
		case MT_ADLIB:
		case MT_MT32:
			break;
		case MT_GM:
			if (!ConfMan.getBool("native_mt32")) {
				// Not a real MT32 / no MUNT
				::GUI::MessageDialog dialog(_(
											"You appear to be using a General MIDI device,\n"
											"but your game only supports Roland MT32 MIDI.\n"
											"We try to map the Roland MT32 instruments to\n"
											"General MIDI ones. It is still possible that\n"
											"some tracks sound incorrect."));
				dialog.runModal();
			}
			// Switch to MT32 driver in any case
			_deviceType = MT_MT32;
			break;
		default:
			_musicMode = kMusicModeDisabled;
			break;
		}
	}

	switch (_musicMode) {
	case kMusicModeAccolade: {
		// Setup midi driver
		switch (_deviceType) {
		case MT_ADLIB:
			_driver = MidiDriver_Accolade_AdLib_create(accoladeDriverFilename);
			break;
		case MT_MT32:
			_driver = MidiDriver_Accolade_MT32_create(accoladeDriverFilename);
			break;
		default:
			assert(0);
			break;
		}
		if (!_driver)
			return 255;

		ret = _driver->open();
		if (ret == 0) {
			// Reset is done inside our MIDI driver
			_driver->setTimerCallback(this, &onTimer);
		}

		//setTimerRate(_driver->getBaseTempo());
		return 0;
	}

	case kMusicModeMilesAudio: {
		switch (_deviceType) {
		case MT_ADLIB: {
			Common::File instrumentDataFile;
			if (instrumentDataFile.exists("MIDPAK.AD")) {
				// if there is a file called MIDPAK.AD, use it directly
				warning("SIMON 2: using MIDPAK.AD");
				_driver = Audio::MidiDriver_Miles_AdLib_create("MIDPAK.AD", "MIDPAK.AD");
			} else {
				// if there is no file called MIDPAK.AD, try to extract it from the file SETUP.SHR
				// if we didn't do this, the user would be forced to "install" the game instead of simply
				// copying all files from CD-ROM.
				Common::SeekableReadStream *midpakAdLibStream;
				midpakAdLibStream = simon2SetupExtractFile("MIDPAK.AD");
				if (!midpakAdLibStream)
					error("MidiPlayer: could not extract MIDPAK.AD from SETUP.SHR");

				// Pass this extracted data to the driver
				warning("SIMON 2: using MIDPAK.AD extracted from SETUP.SHR");
				_driver = Audio::MidiDriver_Miles_AdLib_create("", "", midpakAdLibStream);
				delete midpakAdLibStream;
			}
			// TODO: not sure what's going wrong with AdLib
			// it doesn't seem to matter if we use the regular XMIDI tracks or the 2nd set meant for MT32
			break;
		}
		case MT_MT32:
			_driver = Audio::MidiDriver_Miles_MT32_create("");
			_nativeMT32 = true; // use 2nd set of XMIDI tracks
			break;
		case MT_GM:
			if (ConfMan.getBool("native_mt32")) {
				_driver = Audio::MidiDriver_Miles_MT32_create("");
				_nativeMT32 = true; // use 2nd set of XMIDI tracks
			}
			break;

		default:
			break;
		}
		if (!_driver)
			return 255;

		ret = _driver->open();
		if (ret == 0) {
			// Reset is done inside our MIDI driver
			_driver->setTimerCallback(this, &onTimer);
		}
		return 0;
	}

	case kMusicModeSimon1: {
		// This only handles the original AdLib driver of Simon1.
		if (_deviceType == MT_ADLIB) {
			_map_mt32_to_gm = false;
			_nativeMT32 = false;

			_driver = createMidiDriverSimon1AdLib("MT_FM.IBK");
			if (_driver && _driver->open() == 0) {
				_driver->setTimerCallback(this, &onTimer);
				// Like the original, we enable the rhythm support by default.
				_driver->send(0xB0, 0x67, 0x01);
				return 0;
			}

			delete _driver;
			_driver = nullptr;
		}

		_musicMode = kMusicModeDisabled;
	}

	default:
		break;
	}

	dev = MidiDriver::detectDevice(MDT_ADLIB | MDT_MIDI | (gameType == GType_SIMON1 ? MDT_PREFER_MT32 : MDT_PREFER_GM));
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));

	_driver = MidiDriver::createMidi(dev);
	if (!_driver)
		return 255;

	if (_nativeMT32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	_map_mt32_to_gm = (gameType != GType_SIMON2 && !_nativeMT32);

	ret = _driver->open();
	if (ret)
		return ret;
	_driver->setTimerCallback(this, &onTimer);

	if (_nativeMT32)
		_driver->sendMT32Reset();
	else
		_driver->sendGMReset();

	return 0;
}

void MidiPlayer::send(uint32 b) {
	if (!_current)
		return;

	if (_musicMode != kMusicModeDisabled) {
		// Handle volume control for Simon1 output.
		if (_musicMode == kMusicModeSimon1) {
			// The driver does not support any volume control, thus we simply
			// scale the velocities on note on for now.
			// TODO: We should probably handle this at output level at some
			// point. Then we can allow volume changes to affect already
			// playing notes too. For now this simple change allows us to
			// have some simple volume control though.
			if ((b & 0xF0) == 0x90) {
				byte volume = (b >> 16) & 0x7F;

				if (_current == &_sfx) {
					volume = volume * _sfxVolume / 255;
				} else if (_current == &_music) {
					volume = volume * _musicVolume / 255;
				}

				b = (b & 0xFF00FFFF) | (volume << 16);
			}
		}

		// Send directly to Accolade/Miles/Simon1 Audio driver
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master music and master sfx volume.
		byte volume = (byte)((b >> 16) & 0x7F);
		_current->volume[channel] = volume;
		if (_current == &_sfx)
			volume = volume * _sfxVolume / 255;
		else if (_current == &_music)
			volume = volume * _musicVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && _map_mt32_to_gm) {
		b = (b & 0xFFFF00FF) | (MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// Only respond to an All Notes Off if this channel
		// has already been allocated.
		if (!_current->channel[b & 0x0F])
			return;
	} else if ((b & 0xFFF0) == 0x79B0) {
		// "Reset All Controllers". There seems to be some confusion
		// about what this message should do to the volume controller.
		// See http://www.midi.org/about-midi/rp15.shtml for more
		// information.
		//
		// If I understand it correctly, the current standard indicates
		// that the volume should be reset, but the next revision will
		// exclude it. On my system, both ALSA and FluidSynth seem to
		// reset it, while AdLib does not. Let's follow the majority.

		_current->volume[channel] = 127;
	}

	// Allocate channels if needed
	if (!_current->channel[channel])
		_current->channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_current->channel[channel]) {
		if (channel == 9) {
			if (_current == &_sfx)
				_current->channel[9]->volume(_current->volume[9] * _sfxVolume / 255);
			else if (_current == &_music)
				_current->channel[9]->volume(_current->volume[9] * _musicVolume / 255);
		}
		_current->channel[channel]->send(b);
		if ((b & 0xFFF0) == 0x79B0) {
			// We have received a "Reset All Controllers" message
			// and passed it on to the MIDI driver. This may or may
			// not have affected the volume controller. To ensure
			// consistent behavior, explicitly set the volume to
			// what we think it should be.

			if (_current == &_sfx)
				_current->channel[channel]->volume(_current->volume[channel] * _sfxVolume / 255);
			else if (_current == &_music)
				_current->channel[channel]->volume(_current->volume[channel] * _musicVolume / 255);
		}
	}
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	// Only thing we care about is End of Track.
	if (!_current || type != 0x2F) {
		return;
	} else if (_current == &_sfx) {
		clearConstructs(_sfx);
	} else if (_loopTrack) {
		_current->parser->jumpToTick(0);
	} else if (_queuedTrack != 255) {
		_currentTrack = 255;
		byte destination = _queuedTrack;
		_queuedTrack = 255;
		_loopTrack = _loopQueuedTrack;
		_loopQueuedTrack = false;

		// Remember, we're still inside the locked mutex.
		// Have to unlock it before calling jump()
		// (which locks it itself), and then relock it
		// upon returning.
		_mutex.unlock();
		startTrack(destination);
		_mutex.lock();
	} else {
		stop();
	}
}

void MidiPlayer::onTimer(void *data) {
	MidiPlayer *p = (MidiPlayer *)data;
	Common::StackLock lock(p->_mutex);

	if (p->_parserMusic) {
		p->_parserMusic->onTimer();
		if (!p->_parserMusic->isPlaying() && p->_queuedTrack != 255) {
			// Music is no longer playing and there is a track queued up.
			// Play the queued track.
			p->setLoop(p->_loopQueuedTrack);
			p->play(p->_queuedTrack, false, false, true);
			p->_queuedTrack = 255;
			p->_loopQueuedTrack = false;
		}
	}
	if (p->_parserSfx)
		p->_parserSfx->onTimer();

	if (!p->_paused) {
		if (p->_music.parser && p->_currentTrack != 255) {
			p->_current = &p->_music;
			p->_music.parser->onTimer();
		}
	}
	if (p->_sfx.parser) {
		p->_current = &p->_sfx;
		p->_sfx.parser->onTimer();
	}
	p->_current = nullptr;
}

bool MidiPlayer::usesMT32Data() const {
	return _dataType == MT_MT32;
}

bool MidiPlayer::hasAdLibSfx() const {
	return _parserSfx != nullptr;
}

bool MidiPlayer::isPlaying(bool checkQueued) {
	Common::StackLock lock(_mutex);

	return _parserMusic->isPlaying() && (!checkQueued || _queuedTrack != 255);
}

void MidiPlayer::startTrack(int track) {
	Common::StackLock lock(_mutex);

	if (track == _currentTrack)
		return;

	if (_music.num_songs > 0) {
		if (track >= _music.num_songs)
			return;

		if (_music.parser) {
			_current = &_music;
			delete _music.parser;
			_current = nullptr;
			_music.parser = nullptr;
		}

		MidiParser *parser = MidiParser::createParser_SMF();
		parser->property (MidiParser::mpMalformedPitchBends, 1);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		if (!parser->loadMusic(_music.songs[track], _music.song_sizes[track])) {
			warning("Error reading track %d", track);
			delete parser;
			parser = nullptr;
		}

		_currentTrack = (byte)track;
		_music.parser = parser; // That plugs the power cord into the wall
	} else if (_music.parser) {
		if (!_music.parser->setTrack(track)) {
			return;
		}
		_currentTrack = (byte)track;
		_current = &_music;
		_music.parser->jumpToTick(0);
		_current = nullptr;
	}
}

void MidiPlayer::stop() {
	Common::StackLock lock(_mutex);

	// Clear the queued track to prevent it from starting when the current
	// track is stopped.
	_queuedTrack = 255;

	if (_parserMusic) {
		_parserMusic->stopPlaying();
		if (_driverMsMusic)
			_driverMsMusic->deinitSource(0);
	}

	if (_music.parser) {
		_current = &_music;
		_music.parser->jumpToTick(0);
	}
	_current = nullptr;
	_currentTrack = 255;
}

void MidiPlayer::stopSfx() {
	Common::StackLock lock(_mutex);

	if (_parserSfx) {
		_parserSfx->stopPlaying();
		if (_driverMsSfx)
			_driverMsSfx->deinitSource(1);
	}
}

void MidiPlayer::pause(bool b) {
	if (_paused == b || !_driver)
		return;
	_paused = b;

	Common::StackLock lock(_mutex);

	if (_paused) {
		if (_parserMusic)
			_parserMusic->pausePlaying();
		if (_parserSfx)
			_parserSfx->pausePlaying();
	} else {
		if (_parserMusic)
			_parserMusic->resumePlaying();
		if (_parserSfx)
			_parserSfx->resumePlaying();
	}

	// if using the driver Accolade_AdLib call setVolume() to turn off\on the volume on all channels
	if (_deviceType == MT_ADLIB && _musicMode == kMusicModeAccolade) {
		static_cast <MidiDriver_Accolade_AdLib*> (_driver)->setVolume(_paused ? 0 : ConfMan.getInt("music_volume"));
	} else if (_musicMode == kMusicModePC98) {
		_driver->property(0x30, _paused ? 1 : 0);
	}

	for (int i = 0; i < 16; ++i) {
		if (_music.channel[i])
			_music.channel[i]->volume(_paused ? 0 : (_music.volume[i] * _musicVolume / 255));
		if (_sfx.channel[i])
			_sfx.channel[i]->volume(_paused ? 0 : (_sfx.volume[i] * _sfxVolume / 255));
	}
}

void MidiPlayer::fadeOut() {
	Common::StackLock lock(_mutex);

	if (!_parserMusic->isPlaying())
		return;

	// 1 second fade-out to silence.
	_driverMsMusic->startFade(0, 1000, 0);
}

void MidiPlayer::setVolume(int musicVol, int sfxVol) {
	musicVol = CLIP(musicVol, 0, 255);
	sfxVol   = CLIP(sfxVol,   0, 255);

	if (_musicVolume == musicVol && _sfxVolume == sfxVol)
		return;

	_musicVolume = musicVol;
	_sfxVolume   = sfxVol;

	if (_musicMode == kMusicModePC98) {
		_driver->property(0x10, _musicVolume);
		_driver->property(0x20, _sfxVolume);
	} else if (_musicMode == kMusicModeAccolade && _deviceType == MT_ADLIB) {
		static_cast <MidiDriver_Accolade_AdLib*> (_driver)->setVolume(_musicVolume);
	}

	// Now tell all the channels this.
	Common::StackLock lock(_mutex);
	if (_driver && !_paused) {
		for (int i = 0; i < 16; ++i) {
			if (_music.channel[i])
				_music.channel[i]->volume(_music.volume[i] * _musicVolume / 255);
			if (_sfx.channel[i])
				_sfx.channel[i]->volume(_sfx.volume[i] * _sfxVolume / 255);
		}
	}
}

void MidiPlayer::syncSoundSettings() {
	if (_driverMsMusic)
		_driverMsMusic->syncSoundSettings();
	if (_driverMsSfx)
		_driverMsSfx->syncSoundSettings();

	// Sync code for non-multisource drivers.
	// TODO Remove when no longer necessary
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("sfx_volume");

	setVolume(mute ? 0 : soundVolumeMusic, mute ? 0 : soundVolumeSFX);
}

void MidiPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	_loopTrack = loop;

	if (_parserMusic)
		_parserMusic->property(MidiParser::mpAutoLoop, loop);
}

void MidiPlayer::queueTrack(int track, bool loop) {
	Common::StackLock lock(_mutex);

	if (!_parserMusic->isPlaying()) {
		// There is no music playing right now, so immediately play the track.
		setLoop(loop);
		play(track);
	} else {
		// Queue up the track for playback at the end of the current track.
		_queuedTrack = track;
		_loopQueuedTrack = loop;
	}
}

void MidiPlayer::clearConstructs() {
	clearConstructs(_music);
	clearConstructs(_sfx);
}

void MidiPlayer::clearConstructs(MusicInfo &info) {
	int i;
	if (info.num_songs > 0) {
		for (i = 0; i < info.num_songs; ++i)
			free(info.songs[i]);
		info.num_songs = 0;
	}

	free(info.data);
	info.data = nullptr;

	delete info.parser;
	info.parser = nullptr;

	if (_driver) {
		for (i = 0; i < 16; ++i) {
			if (info.channel[i]) {
				info.channel[i]->allNotesOff();
				info.channel[i]->release();
			}
		}
	}
	info.clear();
}

void MidiPlayer::resetVolumeTable() {
	int i;
	for (i = 0; i < 16; ++i) {
		_music.volume[i] = _sfx.volume[i] = 127;
		if (_driver)
			_driver->send(((_musicVolume >> 1) << 16) | 0x7B0 | i);
	}
}

void MidiPlayer::loadMusic(Common::SeekableReadStream *in, int32 size, bool sfx) {
	Common::StackLock lock(_mutex);

	MidiParser *parser = sfx ? _parserSfx : _parserMusic;

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = parser->determineDataSize(in);
		if (size < 0) {
			warning("MidiPlayer::loadMusic - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos, SEEK_SET);
	}

	parser->unloadMusic();

	// Copy the data into _musicData or _sfxData.
	byte **dataPtr = sfx ? &_sfxData : &_musicData;
	if (*dataPtr) {
		delete[] *dataPtr;
	}

	*dataPtr = new byte[size];
	in->read(*dataPtr, size);

	// Finally, load the data into the parser.
	parser->loadMusic(*dataPtr, size);
}

void MidiPlayer::play(int track, bool sfx, bool sfxUsesRhythm, bool queued) {
	Common::StackLock lock(_mutex);

	MidiParser *parser = sfx ? _parserSfx : _parserMusic;

	if (parser->setTrack(track)) {
		if (sfx && sfxUsesRhythm) {
			// This sound effect uses OPL rhythm instruments. Disable music
			// rhythm notes while this sound effect is playing to prevent
			// conflicts.
			// Note that if AdLib music is used, _driverMsMusic and
			// _driverMsSfx point to the same object. If AdLib music is not
			// used, the disableMusicRhythmNotes call will do nothing.
			MidiDriver_Simon1_AdLib *adLibSfxDriver = dynamic_cast<MidiDriver_Simon1_AdLib *>(_driverMsSfx);
			if (adLibSfxDriver)
				adLibSfxDriver->disableMusicRhythmNotes();
		}

		if (!sfx && !queued && _driverMsMusic)
			// Reset the volume to neutral (in case the previous track was
			// faded out).
			_driverMsMusic->resetSourceVolume(0);
		parser->startPlaying();
	} else {
		// Original interpreter stops playing when an invalid track is
		// requested (f.e. Simon 2 MT-32 intro).
		parser->stopPlaying();
		warning("MidiPlayer::play - Could not play %s track %i", sfx ? "SFX" : "music", track);
	}
}

// TODO Remove dead code

static const int simon1_gmf_size[] = {
	8900, 12166, 2848, 3442, 4034, 4508, 7064, 9730, 6014, 4742, 3138,
	6570, 5384, 8909, 6457, 16321, 2742, 8968, 4804, 8442, 7717,
	9444, 5800, 1381, 5660, 6684, 2456, 4744, 2455, 1177, 1232,
	17256, 5103, 8794, 4884, 16
};

void MidiPlayer::loadSMF(Common::SeekableReadStream *in, int song, bool sfx) {
	Common::StackLock lock(_mutex);

	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	uint32 startpos = in->pos();
	byte header[4];
	in->read(header, 4);
	bool isGMF = !memcmp(header, "GMF\x1", 4);
	in->seek(startpos, SEEK_SET);

	uint32 size = in->size() - in->pos();
	if (isGMF) {
		if (sfx) {
			// Multiple GMF resources are stored in the SFX files,
			// but each one is referenced by a pointer at the
			// beginning of the file. Those pointers can be used
			// to determine file size.
			in->seek(0, SEEK_SET);
			uint16 value = in->readUint16LE() >> 2; // Number of resources
			if (song != value - 1) {
				in->seek(song * 2 + 2, SEEK_SET);
				value = in->readUint16LE();
				size = value - startpos;
			}
			in->seek(startpos, SEEK_SET);
		} else if (size >= 64000) {
			// For GMF resources not in separate
			// files, we're going to have to use
			// hardcoded size tables.
			size = simon1_gmf_size[song];
		}
	}

	// When allocating space, add 4 bytes in case
	// this is a GMF and we have to tack on our own
	// End of Track event.
	p->data = (byte *)calloc(size + 4, 1);
	in->read(p->data, size);

	uint32 timerRate = _driver->getBaseTempo();

	if (isGMF) {
		// The GMF header
		// 3 BYTES: 'GMF'
		// 1 BYTE : Major version
		// 1 BYTE : Minor version
		// 1 BYTE : Ticks (Ranges from 2 - 8, always 2 for SFX)
		// 1 BYTE : Loop control. 0 = no loop, 1 = loop (Music only)
		if (!sfx) {
			// In the original, the ticks value indicated how many
			// times the music timer was called before it actually
			// did something. The larger the value the slower the
			// music.
			//
			// We, on the other hand, have a timer rate which is
			// used to control by how much the music advances on
			// each onTimer() call. The larger the value, the
			// faster the music.
			//
			// It seems that 4 corresponds to our base tempo, so
			// this should be the right way to calculate it.
			timerRate = (4 * _driver->getBaseTempo()) / p->data[5];

			// According to bug #1706 calling setLoop() from
			// within a lock causes a lockup, though I have no
			// idea when this actually happens.
			_loopTrack = (p->data[6] != 0);
		}
	}

	MidiParser *parser = MidiParser::createParser_SMF();
	parser->property(MidiParser::mpMalformedPitchBends, 1);
	parser->setMidiDriver(this);
	parser->setTimerRate(timerRate);
	if (!parser->loadMusic(p->data, size)) {
		warning("Error reading track");
		delete parser;
		parser = nullptr;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

void MidiPlayer::loadMultipleSMF(Common::SeekableReadStream *in, bool sfx) {
	// This is a special case for Simon 2 Windows.
	// Instead of having multiple sequences as
	// separate tracks in a Type 2 file, simon2win
	// has multiple songs, each of which is a Type 1
	// file. Thus, preceding the songs is a single
	// byte specifying how many songs are coming.
	// We need to load ALL the songs and then
	// treat them as separate tracks -- for the
	// purpose of jumps, anyway.
	Common::StackLock lock(_mutex);

	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	p->num_songs = in->readByte();
	if (p->num_songs > 16) {
		warning("playMultipleSMF: %d is too many songs to keep track of", (int)p->num_songs);
		return;
	}

	byte i;
	for (i = 0; i < p->num_songs; ++i) {
		byte buf[5];
		uint32 pos = in->pos();

		// Make sure there's a MThd
		in->read(buf, 4);
		if (memcmp(buf, "MThd", 4) != 0) {
			warning("Expected MThd but found '%c%c%c%c' instead", buf[0], buf[1], buf[2], buf[3]);
			return;
		}
		in->seek(in->readUint32BE(), SEEK_CUR);

		// Now skip all the MTrk blocks
		while (true) {
			in->read(buf, 4);
			if (memcmp(buf, "MTrk", 4) != 0)
				break;
			in->seek(in->readUint32BE(), SEEK_CUR);
		}

		uint32 pos2 = in->pos() - 4;
		uint32 size = pos2 - pos;
		p->songs[i] = (byte *)calloc(size, 1);
		in->seek(pos, SEEK_SET);
		in->read(p->songs[i], size);
		p->song_sizes[i] = size;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
}

void MidiPlayer::loadXMIDI(Common::SeekableReadStream *in, bool sfx) {
	Common::StackLock lock(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	char buf[4];
	uint32 pos = in->pos();
	uint32 size = 4;
	in->read(buf, 4);
	if (!memcmp(buf, "FORM", 4)) {
		int i;
		for (i = 0; i < 16; ++i) {
			if (!memcmp(buf, "CAT ", 4))
				break;
			size += 2;
			memcpy(buf, &buf[2], 2);
			in->read(&buf[2], 2);
		}
		if (memcmp(buf, "CAT ", 4) != 0) {
			error("Could not find 'CAT ' tag to determine resource size");
		}
		size += 4 + in->readUint32BE();
		in->seek(pos, 0);
		p->data = (byte *)calloc(size, 1);
		in->read(p->data, size);
	} else {
		error("Expected 'FORM' tag but found '%c%c%c%c' instead", buf[0], buf[1], buf[2], buf[3]);
	}

	// In the DOS version of Simon the Sorcerer 2, the music contains lots
	// of XMIDI callback controller events. As far as we know, they aren't
	// actually used, so we disable the callback handler explicitly.

	MidiParser *parser = MidiParser::createParser_XMIDI(nullptr);
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size))
		error("Error reading track");

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

void MidiPlayer::loadS1D(Common::SeekableReadStream *in, bool sfx) {
	Common::StackLock lock(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	uint16 size = in->readUint16LE();
	if (size != in->size() - 2) {
		error("Size mismatch in MUS file (%ld versus reported %d)", (long)in->size() - 2, (int)size);
	}

	p->data = (byte *)calloc(size, 1);
	in->read(p->data, size);

	MidiParser *parser = MidiParser_createS1D();
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size))
		error("Error reading track");

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

#define MIDI_SETUP_BUNDLE_HEADER_SIZE 56
#define MIDI_SETUP_BUNDLE_FILEHEADER_SIZE 48
#define MIDI_SETUP_BUNDLE_FILENAME_MAX_SIZE 12

// PKWARE data compression library (called "DCL" in ScummVM) was used for storing files within SETUP.SHR
// we need it to be able to get the file MIDPAK.AD, otherwise we would have to require the user
// to "install" the game before being able to actually play it, when using AdLib.
//
// SETUP.SHR file format:
//  [bundle file header]
//    [compressed file header] [compressed file data]
//     * compressed file count
Common::SeekableReadStream *MidiPlayer::simon2SetupExtractFile(const Common::String &requestedFileName) {
	Common::File *setupBundleStream = new Common::File();
	uint32        bundleSize = 0;
	uint32        bundleBytesLeft = 0;
	byte          bundleHeader[MIDI_SETUP_BUNDLE_HEADER_SIZE];
	byte          bundleFileHeader[MIDI_SETUP_BUNDLE_FILEHEADER_SIZE];
	uint16        bundleFileCount = 0;
	uint16        bundleFileNr = 0;

	Common::String fileName;
	uint32         fileCompressedSize = 0;
	byte          *fileCompressedDataPtr = nullptr;

	Common::SeekableReadStream *extractedStream = nullptr;

	if (!setupBundleStream->open("setup.shr"))
		error("MidiPlayer: could not open setup.shr");

	bundleSize = setupBundleStream->size();
	bundleBytesLeft = bundleSize;

	if (bundleSize < MIDI_SETUP_BUNDLE_HEADER_SIZE)
		error("MidiPlayer: unexpected EOF in setup.shr");

	if (setupBundleStream->read(bundleHeader, MIDI_SETUP_BUNDLE_HEADER_SIZE) != MIDI_SETUP_BUNDLE_HEADER_SIZE)
		error("MidiPlayer: setup.shr read error");
	bundleBytesLeft -= MIDI_SETUP_BUNDLE_HEADER_SIZE;

	// Verify header byte
	if (bundleHeader[13] != 't')
		error("MidiPlayer: setup.shr bundle header data mismatch");

	bundleFileCount = READ_LE_UINT16(&bundleHeader[14]);

	// Search for requested file
	while (bundleFileNr < bundleFileCount) {
		if (bundleBytesLeft < sizeof(bundleFileHeader))
			error("MidiPlayer: unexpected EOF in setup.shr");

		if (setupBundleStream->read(bundleFileHeader, sizeof(bundleFileHeader)) != sizeof(bundleFileHeader))
			error("MidiPlayer: setup.shr read error");
		bundleBytesLeft -= MIDI_SETUP_BUNDLE_FILEHEADER_SIZE;

		// Extract filename from file-header
		fileName.clear();
		for (byte curPos = 0; curPos < MIDI_SETUP_BUNDLE_FILENAME_MAX_SIZE; curPos++) {
			if (!bundleFileHeader[curPos]) // terminating NUL
				break;
			fileName.insertChar(bundleFileHeader[curPos], curPos);
		}

		// Get compressed
		fileCompressedSize = READ_LE_UINT32(&bundleFileHeader[20]);
		if (!fileCompressedSize)
			error("MidiPlayer: compressed file is 0 bytes, data corruption?");
		if (bundleBytesLeft < fileCompressedSize)
			error("MidiPlayer: unexpected EOF in setup.shr");

		if (fileName == requestedFileName) {
			// requested file found
			fileCompressedDataPtr = new byte[fileCompressedSize];

			if (setupBundleStream->read(fileCompressedDataPtr, fileCompressedSize) != fileCompressedSize)
				error("MidiPlayer: setup.shr read error");

			Common::MemoryReadStream *compressedStream = nullptr;

			compressedStream = new Common::MemoryReadStream(fileCompressedDataPtr, fileCompressedSize);
			// we don't know the unpacked size, let decompressor figure it out
			extractedStream = Common::decompressDCL(compressedStream);
			delete compressedStream;
			break;
		}

		// skip compressed size
		setupBundleStream->skip(fileCompressedSize);
		bundleBytesLeft -= fileCompressedSize;

		bundleFileNr++;
	}
	setupBundleStream->close();
	delete setupBundleStream;

	return extractedStream;
}

} // End of namespace AGOS
