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

// MIDI and digital music class

#include "saga/saga.h"

#include "saga/resource.h"
#include "saga/music.h"

#include "audio/adlib_ms.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiparser_qt.h"
#include "audio/miles.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"
#include "audio/mods/mod_xm_s3m.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/translation.h"
#include "gui/message.h"

namespace Saga {

const uint8 Music::MT32_GOODBYE_MSG[] = { 0x47, 0x6F, 0x6F, 0x64, 0x62, 0x79, 0x65, 0x21, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

Music::Music(SagaEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer), _parser(nullptr), _driver(nullptr), _driverPC98(nullptr), _musicContext(nullptr) {
	_currentVolume = 0;
	_currentMusicBuffer = nullptr;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		_musicType = _driverType = MT_AMIGA;
	} else if (_vm->getPlatform() == Common::kPlatformPC98) {
		_musicType = _driverType = MT_PC98;

		_driverPC98 = new TownsPC98_AudioDriver(mixer, PC98AudioPluginDriver::kType86);
		_driverPC98->init();
	} else {
		_musicType = (_vm->getGameId() == GID_ITE && _vm->getPlatform() == Common::kPlatformDOS ? MT_MT32 : MT_GM);

		MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | (_musicType == MT_MT32 ? MDT_PREFER_MT32 : MDT_PREFER_GM));
		_driverType = MidiDriver::getMusicType(dev);
		if (_driverType == MT_GM && ConfMan.getBool("native_mt32"))
			_driverType = MT_MT32;

		switch (_driverType) {
		case MT_ADLIB:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				const char *opl2InstDefFilename;
				const char *opl3InstDefFilename;
				if (_vm->getGameId() == GID_ITE) {
					opl2InstDefFilename = "INSTR.AD";
					opl3InstDefFilename = "INSTR.OPL";
				} else {
					// IHNM
					opl2InstDefFilename = "SAMPLE.AD";
					opl3InstDefFilename = "SAMPLE.OPL";
				}
				if (Common::File::exists(opl2InstDefFilename) && Common::File::exists(opl3InstDefFilename)) {
					_driver = (MidiDriver_Multisource *)Audio::MidiDriver_Miles_AdLib_create(opl2InstDefFilename, opl3InstDefFilename);
					_driver->property(MidiDriver::PROP_MILES_VERSION, _vm->getGameId() == GID_ITE ?
						Audio::MILES_VERSION_2 : Audio::MILES_VERSION_3);
				} else {
					// WORKAROUND The GOG version of IHNM is missing the AdLib
					// instrument definition files. In this case we fall back
					// to the regular AdLib driver, which has a built-in set of
					// instrument definitions.
					// We cannot distinguish between this GOG version and the
					// case where the user has a physical version of the game,
					// but has forgotten to copy the instrument definition
					// files. So we show a warning that these files are missing.
					GUI::MessageDialog dialog(
						Common::U32String::format(
							_("Could not find AdLib instrument definition files\n"
							  "%s and %s. Without these files,\n"
							  "the music will not sound the same as the original game."),
							opl2InstDefFilename, opl3InstDefFilename),
						_("OK"));
					dialog.runModal();

					OPL::Config::OplType oplType =
						MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::kOpl3) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;

					_driver = new MidiDriver_ADLIB_Multisource(oplType);
				}
			} else {
				OPL::Config::OplType oplType =
					MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::kOpl3) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;

				_driver = new MidiDriver_ADLIB_Multisource(oplType);
			}
			break;
		case MT_MT32:
		case MT_GM:
			if (_vm->getPlatform() == Common::kPlatformDOS) {
				_driver = Audio::MidiDriver_Miles_MIDI_create(_musicType, "");
				_driver->property(MidiDriver::PROP_MILES_VERSION, _vm->getGameId() == GID_ITE ?
					Audio::MILES_VERSION_2 : Audio::MILES_VERSION_3);
			} else {
				_driver = new MidiDriver_MT32GM(_musicType);
			}
			break;
		default:
			_driver = new MidiDriver_NULL_Multisource();
			break;
		}

		if (_driver) {
			_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
			if (_driver->open() != 0)
				error("Failed to open MIDI driver.");

			_driver->setTimerCallback(this, &timerCallback);
			_driver->setSourceNeutralVolume(255);
		}
	}

	_digitalMusicContext = _vm->_resource->getContext(GAME_DIGITALMUSICFILE);
	if (_driverType != MT_ADLIB)
		_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_GM);

	if (!_musicContext)
		_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_FM);

	if (!_musicContext) {
		if (_vm->getGameId() == GID_ITE) {
			_musicContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
		} else if (_vm->getGameId() == GID_IHNM) {
			// TODO If program flow gets here, this getContext call previously
			// returned null...
			_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_FM);
		}
	}

	_trackNumber = 0;
	_userVolume = 0;
	_userMute = false;
	_targetVolume = 0;
	_currentVolumePercent = 100;

	_digitalMusic = false;
}

Music::~Music() {
	_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
	_mixer->stopHandle(_musicHandle);
	if (_parser) {
		_parser->stopPlaying();
		delete _parser;
	}
	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
		delete _driver;
	}
	if (_driverPC98) {
		_driverPC98->reset();
		delete _driverPC98;
	}
}

void Music::close() {
	if (_parser)
		_parser->stopPlaying();

	if (_vm->getGameId() == GID_ITE && _vm->getPlatform() == Common::kPlatformDOS && _driver) {
		MidiDriver_MT32GM *mt32Driver = dynamic_cast<MidiDriver_MT32GM *>(_driver);
		if (mt32Driver)
			mt32Driver->sysExMT32(MT32_GOODBYE_MSG, MidiDriver_MT32GM::MT32_DISPLAY_NUM_CHARS,
				MidiDriver_MT32GM::MT32_DISPLAY_MEMORY_ADDRESS, false, false);
	}
}

void Music::musicVolumeGaugeCallback(void *refCon) {
	((Music *)refCon)->musicVolumeGauge();
}

void Music::musicVolumeGauge() {
	// CHECKME: This is potentially called from a different thread because it is
	// called from a timer callback. However, it does not seem to take any
	// precautions to avoid race conditions.
	int volume;

	_currentVolumePercent += 10;

	if (_currentVolume - _targetVolume > 0) { // Volume decrease
		volume = _targetVolume + (_currentVolume - _targetVolume) * (100 - _currentVolumePercent) / 100;
	} else {
		volume = _currentVolume + (_targetVolume - _currentVolume) * _currentVolumePercent / 100;
	}

	if (volume < 0)
		volume = 1;

	int scaledVolume;
	if (_userMute) {
		scaledVolume = 0;
	} else {
		scaledVolume = (volume * _userVolume) >> 8;
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, scaledVolume);
	if (_driverPC98)
		_driverPC98->setMusicVolume(scaledVolume);

	if (_currentVolumePercent == 100) {
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = _targetVolume;
	}
}

void Music::setVolume(int volume, int time) {
	_targetVolume = volume;

	if (volume == -1) // Set Full volume
		volume = 255;

	if (time == 1) {
		if (_driver) {
			if (_driver->isFading(0))
				_driver->abortFade(0, MidiDriver_Multisource::FADE_ABORT_TYPE_CURRENT_VOLUME);
			_driver->setSourceVolume(0, volume);
		}

		_currentVolumePercent = 100;
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);

		int scaledVolume;
		if (_userMute) {
			scaledVolume = 0;
		} else {
			scaledVolume = (volume * _userVolume) >> 8;
		}

		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, scaledVolume);
		if (_driverPC98)
			_driverPC98->setMusicVolume(scaledVolume);

		_currentVolume = volume;
		return;
	}

	if (_driver)
		_driver->startFade(0, time * 3, volume);

	_currentVolumePercent = 0;
	_vm->getTimerManager()->installTimerProc(&musicVolumeGaugeCallback, time * 300L, this, "sagaMusicVolume");
}

void Music::resetVolume() {
	// Abort a fade / gauge if active and set volume to max.
	setVolume(255);
}

bool Music::isFading() {
	bool isFading = false;
	if (_driver)
		isFading = _driver->isFading(0);
	isFading = isFading || (_currentVolumePercent < 100);

	return isFading;
}

bool Music::isPlaying() {

	return _mixer->isSoundHandleActive(_musicHandle) || (_parser ? _parser->isPlaying() : false) || (_driverPC98 ? _driverPC98->musicPlaying() : false);
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	debug(2, "Music::play %d, %d", resourceId, flags);

	if (isPlaying() && _trackNumber == resourceId)
		return;

	if (_vm->getFeatures() & GF_ITE_DOS_DEMO) {
		warning("TODO: Music::play %d, %d for ITE DOS demo", resourceId, flags);
		return;
	}

	_trackNumber = resourceId;
	_mixer->stopHandle(_musicHandle);
	if (_parser) {
		_parser->unloadMusic();
		delete _parser;
		_parser = nullptr;
	}
	if (_driverPC98)
		_driverPC98->reset();

	resetVolume();

	bool digital = playDigital(resourceId, flags);

	if (!digital) {
		// Load MIDI/XMI resource data
		if (_vm->getGameId() == GID_ITE && _vm->getPlatform() == Common::Platform::kPlatformAmiga) {
			playProtracker(resourceId, flags);
		} else if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
			// Load the external music file for Mac IHNM
			playQuickTime(resourceId, flags);
		} else {
			playMidi(resourceId, flags);
		}
	}
}

bool Music::playDigital(uint32 resourceId, MusicFlags flags) {
	Audio::SeekableAudioStream *audioStream = nullptr;
	uint32 loopStart = 0;
	int realTrackNumber = 0;

	if (_vm->getGameId() == GID_ITE) {
		if (resourceId != 13 && resourceId != 19)
			flags = MUSIC_LOOP;
		realTrackNumber = resourceId - 8;
	} else if (_vm->getGameId() == GID_IHNM) {
		realTrackNumber = resourceId + 1;
	}

	// Try to open standalone digital track
	char trackName[2][16];
	Common::sprintf_s(trackName[0], "track%d", realTrackNumber);
	Common::sprintf_s(trackName[1], "track%02d", realTrackNumber);
	Audio::SeekableAudioStream *stream = nullptr;
	for (int i = 0; i < 2; ++i) {
		stream = Audio::SeekableAudioStream::openStreamFile(trackName[i]);
		if (stream) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
							   Audio::makeLoopingAudioStream(stream, (flags == MUSIC_LOOP) ? 0 : 1));
			_digitalMusic = true;
			return true;
		}
	}

	if (_vm->getGameId() == GID_ITE) {
		if (resourceId >= 9 && resourceId <= 34) {
			if (_digitalMusicContext != nullptr) {
				loopStart = 0;
				// Fix ITE sunstatm/sunspot score
				if (resourceId == MUSIC_SUNSPOT)
					loopStart = 18727;

				// Digital music
				ResourceData *resData = _digitalMusicContext->getResourceData(resourceId - 9);
				Common::SeekableReadStream *musicFile = _digitalMusicContext->getFile(resData);
				int offs = (_digitalMusicContext->isCompressed()) ? 9 : 0;

				Common::SeekableSubReadStream *musicStream = new Common::SeekableSubReadStream(musicFile,
																							   (uint32)resData->offset + offs, (uint32)resData->offset + resData->size - offs);

				if (!_digitalMusicContext->isCompressed()) {
					byte musicFlags = Audio::FLAG_STEREO |
									  Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

					if (_vm->isBigEndian() || (_vm->getFeatures() & GF_SOME_MAC_RESOURCES))
						musicFlags &= ~Audio::FLAG_LITTLE_ENDIAN;

					// The newer ITE Mac demo version contains a music file, but it has mono music.
					// This is the only music file that is about 7MB, whereas all the other ones
					// are much larger. Thus, we use this simple heuristic to determine if we got
					// mono music in the ITE demos or not.
					if (!strcmp(_digitalMusicContext->fileName(), "musicd.rsc") &&
						_digitalMusicContext->fileSize() < 8000000)
						musicFlags &= ~Audio::FLAG_STEREO;

					audioStream = Audio::makeRawStream(musicStream, 11025, musicFlags, DisposeAfterUse::YES);
				} else {
					// Read compressed header to determine compression type
					musicFile->seek((uint32)resData->offset, SEEK_SET);
					byte identifier = musicFile->readByte();

					if (identifier == 0) {		// MP3
#ifdef USE_MAD
						audioStream = Audio::makeMP3Stream(musicStream, DisposeAfterUse::YES);
#endif
					} else if (identifier == 1) {	// OGG
#ifdef USE_VORBIS
						audioStream = Audio::makeVorbisStream(musicStream, DisposeAfterUse::YES);
#endif
					} else if (identifier == 2) {	// FLAC
#ifdef USE_FLAC
						audioStream = Audio::makeFLACStream(musicStream, DisposeAfterUse::YES);
#endif
					}
				}

				if (!audioStream)
					delete musicStream;
			}
		}
	}

	if (audioStream) {
		debug(2, "Playing digitized music");
		if (loopStart) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
							   new Audio::SubLoopingAudioStream(audioStream,
																(flags == MUSIC_LOOP ? 0 : 1),
																Audio::Timestamp(0, loopStart, audioStream->getRate()),
																audioStream->getLength()));
		} else {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
							   Audio::makeLoopingAudioStream(audioStream, (flags == MUSIC_LOOP ? 0 : 1)));
		}
		_digitalMusic = true;
		return true;
	}

	return false;
}

void Music::playQuickTime(uint32 resourceId, MusicFlags flags) {
	// IHNM Mac uses QuickTime MIDI
	_parser = MidiParser::createParser_QT();

	_parser->setMidiDriver(_driver);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	// Handle music looping
	_parser->property(MidiParser::mpAutoLoop, flags & MUSIC_LOOP);

	Common::Path musicName(Common::String::format("Music/Music%02x", resourceId));
	if (!((MidiParser_QT *)_parser)->loadFromContainerFile(musicName))
		error("Music::playQuickTime(): Failed to load file '%s'", musicName.toString().c_str());
	_parser->setTrack(0);
}

void Music::playProtracker(uint32 resourceId, MusicFlags flags) {
	ByteArray ba;

	_vm->_resource->loadResource(_musicContext, resourceId, ba);

	Common::MemoryReadStream ms(ba.getBuffer(), ba.size());

	/* No reference to the 'stream' object is kept, so you can safely delete it after
	   invoking this factory. */
	Audio::RewindableAudioStream *amigaModStream = Audio::makeModXmS3mStream(&ms, DisposeAfterUse::NO);

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
			   Audio::makeLoopingAudioStream(amigaModStream, (flags == MUSIC_LOOP ? 0 : 1)));
}

void Music::playMidi(uint32 resourceId, MusicFlags flags) {
	if (_currentMusicBuffer == &_musicBuffer[1]) {
		_currentMusicBuffer = &_musicBuffer[0];
	} else {
		_currentMusicBuffer = &_musicBuffer[1];
	}

	_vm->_resource->loadResource(_musicContext, resourceId, *_currentMusicBuffer);

	if (_driverPC98) {
		_driverPC98->loadMusicData(_currentMusicBuffer->data() + 4);
	} else {
		if (_currentMusicBuffer->size() < 4) {
			error("Music::playMidi() wrong music resource size");
		}

		// Check if the game is using XMIDI or SMF music
		if (!memcmp(_currentMusicBuffer->getBuffer(), "FORM", 4)) {
			_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
		} else {
			_parser = MidiParser::createParser_SMF(0);
		}

		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		if (_vm->getGameId() == GID_IHNM) {
			// IHNM XMIDI uses sustain and does not reset pitch bend at the
			// start of a new track.
			_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
			_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
		}

		// Handle music looping
		_parser->property(MidiParser::mpAutoLoop, flags & MUSIC_LOOP);
		if (!_parser->loadMusic(_currentMusicBuffer->getBuffer(), _currentMusicBuffer->size()))
			error("Music::play() wrong music resource");
	}
}

void Music::pause() {
	if (_parser) {
		_parser->pausePlaying();
	} else if (_driverPC98) {
		_driverPC98->pause();
	}
}

void Music::resume() {
	if (_parser) {
		_parser->resumePlaying();
	} else if (_driverPC98) {
		_driverPC98->cont();
	}
}

void Music::stop() {
	if (_parser)
		_parser->stopPlaying();
	else if (_driverPC98)
		_driverPC98->reset();
}

void Music::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();

	_userVolume = ConfMan.getInt("music_volume");
	_userMute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	setVolume(_currentVolume);
}

void Music::onTimer() {
	if (_parser)
		_parser->onTimer();
}

void Music::timerCallback(void *data) {
	((Music *)data)->onTimer();
}

} // End of namespace Saga
