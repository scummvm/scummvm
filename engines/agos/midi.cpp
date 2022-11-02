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
#include "agos/drivers/accolade/cms.h"
#include "agos/drivers/accolade/mt32.h"
#include "agos/drivers/simon1/adlib.h"
// Miles Audio for Simon 2
#include "audio/miles.h"
#include "audio/midiparser.h"

// PKWARE data compression library decompressor required for Simon 2
#include "common/dcl.h"
#include "common/translation.h"

#include "gui/message.h"
#include "agos/intern_detection.h"

namespace AGOS {

// MidiParser_S1D is not considered part of the standard
// MidiParser suite, but we still try to mask its details
// and just provide a factory function.
extern MidiParser *MidiParser_createS1D(uint8 source = 0, bool monophonicChords = false);

// This instrument remapping has been constructed by checking how the GM
// instruments correspond to MT-32 instruments in other tracks (f.e. track 10-3
// is similar to track 11).
const byte MidiPlayer::SIMON2_TRACK10_GM_MT32_INSTRUMENT_REMAPPING[] {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x00, 0x1D, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x56, 0x53, 0x4B, 0x00, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

MidiPlayer::MidiPlayer(AGOSEngine *vm) {
	_vm = vm;
	_driver = nullptr;
	_driverMsMusic = nullptr;
	_driverMsSfx = nullptr;

	_paused = false;

	_queuedTrack = 255;
	_loopQueuedTrack = 0;

	_pc98 = false;
	_deviceType = MT_NULL;
	_dataType = MT_NULL;

	_parserMusic = nullptr;
	_parserSfx = nullptr;
	_musicData = nullptr;
	_sfxData = nullptr;
	_parserSfxAccolade = nullptr;
}

MidiPlayer::~MidiPlayer() {
	stop();
	stop(true);

	// Close the drivers first before locking the mutex. Otherwise a deadlock
	// can occur where the timer thread has locked the timer mutex while
	// waiting for the MidiPlayer mutex in the callback, while the main thread
	// has locked the MidiPlayer mutex and waits for the timer mutex to remove
	// a driver callback.
	if (_driverMsSfx && _driverMsSfx != _driverMsMusic) {
		_driverMsSfx->setTimerCallback(nullptr, nullptr);
		_driverMsSfx->close();
	}
	if (_driverMsMusic) {
		_driverMsMusic->setTimerCallback(nullptr, nullptr);
		_driverMsMusic->close();
	} else if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}

	Common::StackLock lock(_mutex);

	if (_parserMusic)
		delete _parserMusic;
	if (_parserSfx)
		delete _parserSfx;
	if (_parserSfxAccolade)
		delete _parserSfxAccolade;

	if (_musicData)
		delete[] _musicData;
	if (_sfxData)
		delete[] _sfxData;

	if (_driverMsSfx && _driverMsSfx != _driverMsMusic) {
		delete _driverMsSfx;
		_driverMsSfx = nullptr;
	}
	if (_driverMsMusic) {
		delete _driverMsMusic;
		_driverMsMusic = nullptr;
	} else if (_driver) {
		delete _driver;
		_driver = nullptr;
	}
}

int MidiPlayer::open() {
	// Don't call open() twice!
	assert(!_driver);

	_pc98 = _vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformPC98;
	// All games have MT-32 data, except Simon 2, which has both GM and MT-32
	// data for DOS, or only GM for Windows. Some of the GM tracks have extra
	// instruments compared to the MT-32 tracks, so GM is preferred.
	int devFlags = MDT_MIDI | (_pc98 ? MDT_PC98 : MDT_ADLIB) |
		(_vm->getGameType() == GType_SIMON2 ? MDT_PREFER_GM : MDT_PREFER_MT32);
	if (_vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformDOS)
		// CMS is only supported by Elvira 1 DOS.
		devFlags |= MDT_CMS;

	// Check the type of device that the user has configured.
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(devFlags);
	_deviceType = MidiDriver::getMusicType(dev);

	// Check if a Casio device has been configured for Elvira 1 DOS.
	if (_vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformDOS &&
			_deviceType == MT_GM && ConfMan.hasKey("midi_mode")) {
		int midiMode = ConfMan.getInt("midi_mode");
		if (midiMode == 3) {
			_deviceType = MT_MT540;
		} else if (midiMode == 4) {
			_deviceType = MT_CT460;
		}
	}

	if (_deviceType == MT_GM && ConfMan.getBool("native_mt32"))
		_deviceType = MT_MT32;

	if (_vm->getGameType() == GType_SIMON2) {
		// Simon 2 DOS version has both MT-32 and GM tracks; Windows is GM only.
		_dataType = (_vm->getPlatform() == Common::kPlatformDOS && _deviceType == MT_MT32) ? MT_MT32 : MT_GM;
	} else {
		// All the other games' MIDI data targets MT-32 (even Simon 1 Windows
		// and Acorn).
		_dataType = MT_MT32;
	}

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

	// Elvira 2, Waxworks and Simon 1 DOS floppy have MIDI SFX.
	bool usesMidiSfx = _vm->getGameType() == GType_WW || _vm->getGameType() == GType_ELVIRA2 ||
		(_vm->getGameType() == GType_SIMON1 && _vm->getPlatform() == Common::kPlatformDOS &&
			!(_vm->getFeatures() & GF_DEMO) && !(_vm->getFeatures() & GF_TALKIE));

	// OPL3 is used for Windows and Acorn versions, all Simon 2 versions and
	// if the user has set the OPL3 mode option. Otherwise OPL2 is used.
	OPL::Config::OplType oplType = (_vm->getPlatform() != Common::kPlatformDOS ||
		_vm->getGameType() == GType_SIMON2 || ConfMan.getBool("opl3_mode")) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;

	// Create drivers and parsers for the different versions of the games.
	if ((_vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformDOS) ||
			_vm->getGameType() == GType_ELVIRA2 || _vm->getGameType() == GType_WW ||
			(_vm->getGameType() == GType_SIMON1 && _vm->getPlatform() == Common::kPlatformDOS &&
				 (_vm->getFeatures() & GF_DEMO) && !(_vm->getFeatures() & GF_TALKIE))) {
		// Elvira 1 DOS, Elvira 2, Waxworks and Simon 1 DOS floppy demo

		// These games use drivers used by several Accolade games.
		// Elvira 1 DOS uses an older version of the Accolade drivers which
		// uses different files.
		Common::String accoladeDriverFilename = _vm->getGameType() == GType_ELVIRA1 ? "INSTR.DAT" : "MUSIC.DRV";

		switch (_deviceType) {
		case MT_ADLIB:
			if (usesMidiSfx) {
				// Elvira 2 and Waxworks have AdLib SFX.
				_parserSfxAccolade = new SfxParser_Accolade_AdLib();
				// Sync the driver to the AdLib SFX script timer.
				_driverMsMusic = _driverMsSfx = MidiDriver_Accolade_AdLib_create(accoladeDriverFilename, oplType, SfxParser_Accolade::SCRIPT_TIMER_FREQUENCY);
			} else {
				_driverMsMusic = _driverMsSfx = MidiDriver_Accolade_AdLib_create(accoladeDriverFilename, oplType);
			}
			if (_vm->getGameType() == GType_ELVIRA1 && oplType == OPL::Config::kOpl3)
				// WORKAROUND Some Elvira 1 OPL instruments do not work
				// well in OPL3 mode and need some adjustments.
				static_cast<MidiDriver_Accolade_AdLib *>(_driverMsMusic)->patchE1Instruments();
			if (_vm->getGameType() == GType_WW) {
				// WORKAROUND Some Waxworks tracks do not set an instrument on
				// a MIDI channel. This will cause that channel to play with
				// whatever instrument was set there by a previous track.
				// This is fixed by setting default instruments on all channels
				// before starting a track.
				_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM);
				if (oplType == OPL::Config::kOpl3)
					// WORKAROUND Some Waxworks OPL instruments do not work
					// well in OPL3 mode and need some adjustments.
					static_cast<MidiDriver_Accolade_AdLib *>(_driverMsMusic)->patchWwInstruments();
			}
			break;
		case MT_MT32:
		case MT_GM:
			_driverMsMusic = MidiDriver_Accolade_MT32_create(accoladeDriverFilename);
			if (_vm->getGameType() == GType_WW) {
				// WORKAROUND See above.
				int16 defaultInstruments[16];
				Common::fill(defaultInstruments, defaultInstruments + ARRAYSIZE(defaultInstruments), -1);
				Common::copy(MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS, MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS + ARRAYSIZE(MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS), defaultInstruments + 1);
				_driverMsMusic->setControllerDefaults(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM, defaultInstruments);
			}
			if (usesMidiSfx) {
				if (ConfMan.getBool("multi_midi")) {
					// Use AdLib SFX with MT-32 music.
					_driverMsSfx = MidiDriver_Accolade_AdLib_create(accoladeDriverFilename, oplType, SfxParser_Accolade::SCRIPT_TIMER_FREQUENCY);
					_parserSfxAccolade = new SfxParser_Accolade_AdLib();
				} else {
					// Use MT-32 SFX.
					_driverMsSfx = _driverMsMusic;
					_parserSfxAccolade = new SfxParser_Accolade_MT32();
				}
			}
			break;
		case MT_MT540:
		case MT_CT460:
			// Casio devices are supported by Elvira 1 DOS only.
			_driverMsMusic = MidiDriver_Accolade_Casio_create(accoladeDriverFilename);
			break;
		case MT_CMS:
			// CMS is supported by Elvira 1 DOS only.
			if (_vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformDOS) {
				_driver = new MidiDriver_Accolade_Cms();
				break;
			}
			// fall through
		default:
			_driverMsMusic = new MidiDriver_NULL_Multisource();
			break;
		}
		if (!_driver)
			_driver = _driverMsMusic;

		// These games use the MUS MIDI format used by several Accolade games.
		// The Elvira 2 / Waxworks version of the AdLib driver has a mechanism
		// that will only play the highest note of a chord; the Elvira 1
		// version does not have this.
		_parserMusic = MidiParser_createS1D(0, _vm->getGameType() != GType_ELVIRA1 && _deviceType == MT_ADLIB);
	} else if (_vm->getGameType() == GType_ELVIRA1 && _vm->getPlatform() == Common::kPlatformPC98) {
		// Elvira 1 PC-98

		// This version has its own drivers. It uses the same MUS format as the
		// DOS version.
		_driver = MidiDriverPC98_create(dev);
		_parserMusic = MidiParser_createS1D(0);
	} else if (_vm->getGameType() == GType_SIMON1) {
		// Simon 1 (except the DOS floppy demo)

		// The DOS versions have their own drivers. The Windows version uses
		// the standard Windows drivers.
		switch (_deviceType) {
		case MT_ADLIB:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				// The DOS version AdLib driver uses an instrument bank file.
				_driverMsMusic = createMidiDriverSimon1AdLib("MT_FM.IBK", oplType);
				if (!(_vm->getFeatures() & GF_TALKIE)) {
					// The DOS floppy version has AdLib MIDI SFX.
					_driverMsSfx = _driverMsMusic;
				}
			} else {
				// Windows and Acorn CD
				// TODO Acorn does not use an OPL chip, but ScummVM doesn't
				// have an implementation of the Acorn audio. It has the same
				// music data as the DOS CD version, but it does not have
				// the MT_FM.IBK instrument bank, so the standard AdLib MIDI
				// driver is used.
				_driverMsMusic = new MidiDriver_ADLIB_Multisource(oplType);
				// WORKAROUND These versions have the same music data as the
				// DOS versions, which target MT-32, despite Windows using GM
				// as its MIDI format. To fix this, the MT-32 instruments are
				// remapped to corresponding GM instruments.
				_driverMsMusic->setInstrumentRemapping(MidiDriver::_mt32ToGm);
			}

			// WORKAROUND The Simon 1 MIDI data does not always set a value for
			// program before it starts playing notes on a MIDI channel. This
			// can cause instruments from a previous track to be used
			// unintentionally.
			// To correct this, default instruments are set when a new track is
			// started.
			_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM);

			break;
		case MT_MT32:
		case MT_GM:
			_driverMsMusic = new MidiDriver_MT32GM(_dataType);

			// WORKAROUND See above.
			int16 defaultInstruments[16];
			Common::fill(defaultInstruments, defaultInstruments + ARRAYSIZE(defaultInstruments), -1);
			Common::copy(MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS, MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS + ARRAYSIZE(MidiDriver_MT32GM::MT32_DEFAULT_INSTRUMENTS), defaultInstruments + 1);
			_driverMsMusic->setControllerDefaults(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM, defaultInstruments);

			if (_vm->getPlatform() == Common::kPlatformDOS && !(_vm->getFeatures() & GF_TALKIE) &&
					ConfMan.getBool("multi_midi")) {
				// The DOS floppy version can use AdLib MIDI SFX with MT-32
				// music.
				_driverMsSfx = createMidiDriverSimon1AdLib("MT_FM.IBK", oplType);
			}

			break;
		default:
			_driverMsMusic = new MidiDriver_NULL_Multisource();
			break;
		}
		_driver = _driverMsMusic;

		// WORKAROUND The Simon 1 MIDI data does not always set a value for
		// volume or panning before it starts playing notes on a MIDI channel.
		// This can cause settings for these parameters from a previous track
		// to be used unintentionally. To correct this, default values for
		// these parameters are set when a new track is started.
		_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_VOLUME);
		_driverMsMusic->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PANNING);

		// The Windows version uses music tempos which are noticably faster
		// than those used by the DOS versions. The MIDI parsers can be
		// configured to use one of both tempos. The DOS tempos will be used
		// for the DOS versions or if the user has selected the "Use DOS music
		// tempos" option. Otherwise the Windows tempos will be used.
		bool useDosTempos = ConfMan.hasKey("dos_music_tempos") ? ConfMan.getBool("dos_music_tempos") : _vm->getPlatform() == Common::kPlatformDOS;

		// Create the MIDI parser(s) for the format used by the platform.
		if (_vm->getPlatform() == Common::kPlatformWindows) {
			// The Windows version uses a slightly different SMF variant.
			_parserMusic = new MidiParser_SimonWin(0, useDosTempos);
		} else {
			// DOS floppy & CD and Acorn CD use GMF (also an SMF variant).
			_parserMusic = new MidiParser_GMF(0, useDosTempos);

			if (_driverMsSfx) {
				// DOS floppy needs a second GMF parser for AdLib SFX.
				_parserSfx = new MidiParser_GMF(1, true);
				_parserMusic->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
				_parserSfx->property(MidiParser::mpDisableAllNotesOffMidiEvents, true);
				_parserSfx->property(MidiParser::mpDisableAutoStartPlayback, true);
			}
		}
	} else if (_vm->getGameType() == GType_SIMON2) {
		// Simon 2

		// The DOS version uses MIDPAK, which is similar to Miles AIL v2.
		// The Windows version uses the standard Windows drivers.
		switch (_deviceType) {
		case MT_ADLIB:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				// DOS
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
				// DOS
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

		// Create the MIDI parser(s) for the format used by the platform.
		if (_vm->getPlatform() == Common::kPlatformDOS) {
			// DOS uses Miles XMIDI.
			_parserMusic = MidiParser::createParser_XMIDI(MidiParser::defaultXMidiCallback, 0, 0);
		} else {
			// Windows version uses an SMF variant (same as Simon 1 Windows).
			_parserMusic = new MidiParser_SimonWin(0);
		}
	}

	// Set common properties for the drivers and music parser.
	if (_driverMsMusic)
		_driverMsMusic->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	if (_driverMsSfx && _driverMsSfx != _driverMsMusic)
		_driverMsSfx->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);

	_parserMusic->property(MidiParser::mpDisableAutoStartPlayback, true);

	// Open the MIDI driver(s).
	int returnCode = _driver->open();
	if (returnCode != 0)
		error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);
	if (_driverMsSfx && _driverMsSfx != _driverMsMusic) {
		returnCode = _driverMsSfx->open();
		if (returnCode != 0)
			error("MidiPlayer::open - Failed to open MIDI SFX driver - error code %d.", returnCode);
	}

	syncSoundSettings();

	// Connect the driver(s) and the parser(s).
	_parserMusic->setMidiDriver(_driver);
	_parserMusic->setTimerRate(_driver->getBaseTempo());
	if (_parserSfx) {
		_parserSfx->setMidiDriver(_driverMsSfx);
	} else if (_parserSfxAccolade) {
		_parserSfxAccolade->setMidiDriver(_driverMsSfx);
	}

	if ((_parserSfx || _parserSfxAccolade) && _driverMsSfx == _driver) {
		// Use MidiPlayer::onTimer to trigger both parsers from the
		// single driver (it can only have one timer callback).
		_driver->setTimerCallback(this, &onTimer);
		if (_parserSfx) {
			_parserSfx->setTimerRate(_driver->getBaseTempo());
		} else if (_parserSfxAccolade) {
			_parserSfxAccolade->setTimerRate(_driver->getBaseTempo());
		}
	} else {
		// Connect each parser to its own driver.
		_driver->setTimerCallback(_parserMusic, &_parserMusic->timerCallback);
		if (_parserSfx) {
			_driverMsSfx->setTimerCallback(_parserSfx, &_parserSfx->timerCallback);
			_parserSfx->setTimerRate(_driverMsSfx->getBaseTempo());
		} else if (_parserSfxAccolade) {
			_driverMsSfx->setTimerCallback(_parserSfxAccolade, &_parserSfxAccolade->timerCallback);
			_parserSfxAccolade->setTimerRate(_driverMsSfx->getBaseTempo());
		}
	}

	return 0;
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

	if (p->_parserSfxAccolade)
		p->_parserSfxAccolade->onTimer();
}

bool MidiPlayer::usesMT32Data() const {
	return _dataType == MT_MT32;
}

bool MidiPlayer::hasMidiSfx() const {
	return _parserSfx != nullptr || _parserSfxAccolade != nullptr;
}

bool MidiPlayer::isPlaying(bool checkQueued) {
	Common::StackLock lock(_mutex);

	return _parserMusic->isPlaying() && (!checkQueued || _queuedTrack != 255);
}

void MidiPlayer::stop(bool sfx) {
	Common::StackLock lock(_mutex);

	if (!sfx) {
		// Clear the queued track to prevent it from starting when the current
		// track is stopped.
		_queuedTrack = 255;

		if (_parserMusic) {
			_parserMusic->stopPlaying();
			if (_driverMsMusic)
				_driverMsMusic->deinitSource(0);
		}
	} else {
		if (_parserSfx) {
			_parserSfx->stopPlaying();
			if (_driverMsSfx)
				_driverMsSfx->deinitSource(1);
		}
		if (_parserSfxAccolade) {
			_parserSfxAccolade->stopAll();
		}
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
	if (_parserSfxAccolade)
		_parserSfxAccolade->pauseAll(_paused);
}

void MidiPlayer::fadeOut() {
	Common::StackLock lock(_mutex);

	if (!_parserMusic->isPlaying())
		return;

	// 1 second fade-out to silence.
	_driverMsMusic->startFade(0, 1000, 0);
}

void MidiPlayer::syncSoundSettings() {
	if (_driverMsMusic)
		_driverMsMusic->syncSoundSettings();
	if (_driverMsSfx)
		_driverMsSfx->syncSoundSettings();

	if (_pc98) {
		// Sync code for non-multisource drivers.
		bool mute = false;
		if (ConfMan.hasKey("mute"))
			mute = ConfMan.getBool("mute");

		// Sync the engine with the config manager
		int soundVolumeMusic = ConfMan.getInt("music_volume");
		int soundVolumeSFX = ConfMan.getInt("sfx_volume");

		_driver->property(0x10, mute ? 0 : soundVolumeMusic);
		_driver->property(0x20, mute ? 0 : soundVolumeSFX);
	}
}

void MidiPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	if (_parserMusic)
		_parserMusic->property(MidiParser::mpAutoLoop, loop);
}

void MidiPlayer::setSimon2Remapping(bool remap) {
	if (_driverMsMusic)
		_driverMsMusic->setInstrumentRemapping(remap ? SIMON2_TRACK10_GM_MT32_INSTRUMENT_REMAPPING : nullptr);
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

void MidiPlayer::load(Common::SeekableReadStream *in, int32 size, bool sfx) {
	Common::StackLock lock(_mutex);

	if (sfx && _parserSfxAccolade) {
		_parserSfxAccolade->load(in, size);
		return;
	}

	MidiParser *parser = sfx ? _parserSfx : _parserMusic;
	if (!parser)
		return;

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = parser->determineDataSize(in);
		if (size < 0) {
			warning("MidiPlayer::load - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos);
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

	if (sfx && _parserSfxAccolade) {
		_parserSfxAccolade->play(track);
		return;
	}
	
	MidiParser *parser = sfx ? _parserSfx : _parserMusic;
	if (!parser)
		return;

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
