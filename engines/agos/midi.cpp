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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "agos/agos.h"
#include "agos/midi.h"

#include "agos/drivers/accolade/mididriver.h"
// Miles Audio for Simon 2
#include "audio/miles.h"

#include "gui/message.h"

namespace AGOS {


// MidiParser_S1D is not considered part of the standard
// MidiParser suite, but we still try to mask its details
// and just provide a factory function.
extern MidiParser *MidiParser_createS1D();

MidiPlayer::MidiPlayer() {
	// Since initialize() is called every time the music changes,
	// this is where we'll initialize stuff that must persist
	// between songs.
	_driver = 0;
	_map_mt32_to_gm = false;

	_adlibPatches = NULL;

	_enable_sfx = true;
	_current = 0;

	_musicVolume = 255;
	_sfxVolume = 255;

	resetVolumeTable();
	_paused = false;

	_currentTrack = 255;
	_loopTrackDefault = false;
	_queuedTrack = 255;
	_loopQueuedTrack = 0;

	_musicMode = kMusicModeDisabled;
}

MidiPlayer::~MidiPlayer() {
	stop();

	Common::StackLock lock(_mutex);
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
	}
	_driver = NULL;
	clearConstructs();
	unloadAdlibPatches();
}

int MidiPlayer::open(int gameType, bool isDemo) {
	// Don't call open() twice!
	assert(!_driver);

	bool accoladeUseMusicDrvFile = false;
	MusicType accoladeMusicType = MT_INVALID;
	MusicType milesAudioMusicType = MT_INVALID;

	switch (gameType) {
	case GType_ELVIRA1:
		_musicMode = kMusicModeAccolade;
		break;
	case GType_ELVIRA2:
	case GType_WW:
		// Attention: Elvira 2 shipped with INSTR.DAT and MUSIC.DRV
		// MUSIC.DRV is the correct one. INSTR.DAT seems to be a left-over
		_musicMode = kMusicModeAccolade;
		accoladeUseMusicDrvFile = true;
		break;
	case GType_SIMON1:
		if (isDemo) {
			_musicMode = kMusicModeAccolade;
			accoladeUseMusicDrvFile = true;
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

	switch (_musicMode) {
	case kMusicModeAccolade:
		dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
		accoladeMusicType = MidiDriver::getMusicType(dev);

		switch (accoladeMusicType) {
		case MT_ADLIB:
		case MT_MT32:
			break;
		case MT_GM:
			if (!ConfMan.getBool("native_mt32")) {
				// Not a real MT32 / no MUNT
				::GUI::MessageDialog dialog(("You appear to be using a General MIDI device,\n"
											"but your game only supports Roland MT32 MIDI.\n"
											"We try to map the Roland MT32 instruments to\n"
											"General MIDI ones. It is still possible that\n"
											"some tracks sound incorrect."));
				dialog.runModal();
			}
			// Switch to MT32 driver in any case
			accoladeMusicType = MT_MT32;
			break;
		default:
			_musicMode = kMusicModeDisabled;
			break;
		}
		break;

	case kMusicModeMilesAudio:
		dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
		milesAudioMusicType = MidiDriver::getMusicType(dev);

		switch (milesAudioMusicType) {
		case MT_ADLIB:
		case MT_MT32:
			break;
		case MT_GM:
			if (!ConfMan.getBool("native_mt32")) {
				// Not a real MT32 / no MUNT
				::GUI::MessageDialog dialog(("You appear to be using a General MIDI device,\n"
											"but your game only supports Roland MT32 MIDI.\n"
											"Music got disabled because of this.\n"
											"You may choose AdLib instead.\n"));
				dialog.runModal();
			}
			// Switch to MT32 driver in any case
			milesAudioMusicType = MT_MT32;
			break;
		default:
			_musicMode = kMusicModeDisabled;
			break;
		}
		break;

	default:
		break;
	}

	switch (_musicMode) {
	case kMusicModeAccolade: {
		// Setup midi driver
		switch (accoladeMusicType) {
		case MT_ADLIB:
			_driver = MidiDriver_Accolade_AdLib_create();
			break;
		case MT_MT32:
			_driver = MidiDriver_Accolade_MT32_create();
			break;
		default:
			assert(0);
			break;
		}
		if (!_driver)
			return 255;

		byte  *instrumentData = NULL;
		uint16 instrumentDataSize = 0;

		if (!accoladeUseMusicDrvFile) {
			// Elvira 1 / Elvira 2: read INSTR.DAT
			Common::File *instrDatStream = new Common::File();

			if (!instrDatStream->open("INSTR.DAT")) {
				error("INSTR.DAT: unable to open file");
			}

			uint32 streamSize = instrDatStream->size();
			uint32 streamLeft = streamSize;
			uint16 skipChunks = 0; // 1 for MT32, 0 for AdLib
			uint16 chunkSize  = 0;

			switch (accoladeMusicType) {
			case MT_ADLIB:
				skipChunks = 0;
				break;
			case MT_MT32:
				skipChunks = 1; // Skip one entry for MT32
				break;
			default:
				assert(0);
				break;
			}

			do {
				if (streamLeft < 2)
					error("INSTR.DAT: unexpected EOF");

				chunkSize = instrDatStream->readUint16LE();
				streamLeft -= 2;

				if (streamLeft < chunkSize)
					error("INSTR.DAT: unexpected EOF");

				if (skipChunks) {
					// Skip the chunk
					instrDatStream->skip(chunkSize);
					streamLeft -= chunkSize;

					skipChunks--;
				}
			} while (skipChunks);

			// Seek over the ASCII string until there is a NUL terminator
			byte curByte = 0;

			do {
				if (chunkSize == 0)
					error("INSTR.DAT: no actual instrument data found");

				curByte = instrDatStream->readByte();
				chunkSize--;
			} while (curByte);

			instrumentDataSize = chunkSize;

			// Read the requested instrument data entry
			instrumentData = new byte[instrumentDataSize];
			instrDatStream->read(instrumentData, instrumentDataSize);

			instrDatStream->close();
			delete instrDatStream;

		} else {
			// Waxworks / Simon 1 demo: Read MUSIC.DRV
			Common::File *musicDrvStream = new Common::File();

			if (!musicDrvStream->open("MUSIC.DRV")) {
				error("MUSIC.DRV: unable to open file");
			}

			uint32 streamSize = musicDrvStream->size();
			uint32 streamLeft = streamSize;
			uint16 getChunk   = 0; // 4 for MT32, 2 for AdLib

			switch (accoladeMusicType) {
			case MT_ADLIB:
				getChunk = 2;
				break;
			case MT_MT32:
				getChunk = 4;
				break;
			default:
				assert(0);
				break;
			}

			if (streamLeft < 2)
				error("MUSIC.DRV: unexpected EOF");

			uint16 chunkCount = musicDrvStream->readUint16LE();
			streamLeft -= 2;

			if (getChunk >= chunkCount)
				error("MUSIC.DRV: required chunk not available");

			uint16 headerOffset = 2 + (28 * getChunk);
			streamLeft -= (28 * getChunk);

			if (streamLeft < 28)
				error("MUSIC.DRV: unexpected EOF");

			// Seek to required chunk
			musicDrvStream->seek(headerOffset);
			musicDrvStream->skip(20); // skip over name
			streamLeft -= 20;

			uint16 musicDrvSignature = musicDrvStream->readUint16LE();
			uint16 musicDrvType = musicDrvStream->readUint16LE();
			uint16 chunkOffset = musicDrvStream->readUint16LE();
			uint16 chunkSize   = musicDrvStream->readUint16LE();

			// Security checks
			if (musicDrvSignature != 0xFEDC)
				error("MUSIC.DRV: chunk signature mismatch");
			if (musicDrvType != 1)
				error("MUSIC.DRV: not a music driver");
			if (chunkOffset >= streamSize)
				error("MUSIC.DRV: driver chunk points outside of file");

			streamLeft = streamSize - chunkOffset;
			if (streamLeft < chunkSize)
				error("MUSIC.DRV: driver chunk is larger than file");

			instrumentDataSize = chunkSize;

			// Read the requested instrument data entry
			instrumentData = new byte[instrumentDataSize];

			musicDrvStream->seek(chunkOffset);
			musicDrvStream->read(instrumentData, instrumentDataSize);

			musicDrvStream->close();
			delete musicDrvStream;
		}

		// Pass the instrument data to the driver
		bool instrumentSuccess = false;

		switch (accoladeMusicType) {
		case MT_ADLIB:
			instrumentSuccess = MidiDriver_Accolade_AdLib_setupInstruments(_driver, instrumentData, instrumentDataSize, accoladeUseMusicDrvFile);
			break;
		case MT_MT32:
			instrumentSuccess = MidiDriver_Accolade_MT32_setupInstruments(_driver, instrumentData, instrumentDataSize, accoladeUseMusicDrvFile);
			break;
		default:
			assert(0);
			break;
		}
		delete[] instrumentData;

		if (!instrumentSuccess) {
			// driver did not like the contents
			if (!accoladeUseMusicDrvFile)
				error("INSTR.DAT: contents not acceptable");
			else
				error("MUSIC.DRV: contents not acceptable");
		}

		ret = _driver->open();
		if (ret == 0) {
			// Reset is done inside our MIDI driver
			_driver->setTimerCallback(this, &onTimer);
		}

		//setTimerRate(_driver->getBaseTempo());
		return 0;
	}
	
	case kMusicModeMilesAudio: {
		switch (milesAudioMusicType) {
		case MT_ADLIB: {
			_driver = Audio::MidiDriver_Miles_AdLib_create("MIDPAK.AD", "MIDPAK.AD");
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
				milesAudioMusicType = MT_MT32;
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

	/* Disabled due to not sounding right, and low volume level
	if (gameType == GType_SIMON1 && MidiDriver::getMusicType(dev) == MT_ADLIB) {
			loadAdlibPatches();
	}
	*/

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
		// Send directly to Accolade/Miles Audio driver
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
	} else if ((b & 0xF0) == 0xC0) {
		if (_map_mt32_to_gm && !_adlibPatches) {
			b = (b & 0xFFFF00FF) | (MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8);
		}
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

		if ((b & 0xF0) == 0xC0 && _adlibPatches) {
			// NOTE: In the percussion channel, this function is a
			//       no-op. Any percussion instruments you hear may
			//       be the stock ones from adlib.cpp.
			_driver->sysEx_customInstrument(_current->channel[channel]->getNumber(), 'ADL ', _adlibPatches + 30 * ((b >> 8) & 0xFF));
		} else {
			_current->channel[channel]->send(b);
		}

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
	} else if (_current->loopTrack) {
		_current->parser->jumpToTick(0);
	} else if (_queuedTrack != 255) {
		_currentTrack = 255;
		byte destination = _queuedTrack;
		_queuedTrack = 255;
		_current->loopTrack = _loopQueuedTrack;
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
	p->_current = 0;
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
			_current = 0;
			_music.parser = 0;
		}

		MidiParser *parser = MidiParser::createParser_SMF();
		parser->property (MidiParser::mpMalformedPitchBends, 1);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		if (!parser->loadMusic(_music.songs[track], _music.song_sizes[track])) {
			warning("Error reading track %d", track);
			delete parser;
			parser = 0;
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
		_current = 0;
	}
}

void MidiPlayer::stop() {
	Common::StackLock lock(_mutex);

	if (_music.parser) {
		_current = &_music;
		_music.parser->jumpToTick(0);
	}
	_current = 0;
	_currentTrack = 255;
}

void MidiPlayer::pause(bool b) {
	if (_paused == b || !_driver)
		return;
	_paused = b;

	Common::StackLock lock(_mutex);
	for (int i = 0; i < 16; ++i) {
		if (_music.channel[i])
			_music.channel[i]->volume(_paused ? 0 : (_music.volume[i] * _musicVolume / 255));
		if (_sfx.channel[i])
			_sfx.channel[i]->volume(_paused ? 0 : (_sfx.volume[i] * _sfxVolume / 255));
	}
}

void MidiPlayer::setVolume(int musicVol, int sfxVol) {
	if (musicVol < 0)
		musicVol = 0;
	else if (musicVol > 255)
		musicVol = 255;
	if (sfxVol < 0)
		sfxVol = 0;
	else if (sfxVol > 255)
		sfxVol = 255;

	if (_musicVolume == musicVol && _sfxVolume == sfxVol)
		return;

	_musicVolume = musicVol;
	_sfxVolume = sfxVol;

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

void MidiPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	_loopTrackDefault = loop;
}

void MidiPlayer::queueTrack(int track, bool loop) {
	_mutex.lock();
	if (_currentTrack == 255) {
		_mutex.unlock();
		setLoop(loop);
		startTrack(track);
	} else {
		_queuedTrack = track;
		_loopQueuedTrack = loop;
		_mutex.unlock();
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
	info.data = 0;

	delete info.parser;
	info.parser = 0;

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

void MidiPlayer::loadAdlibPatches() {
	Common::File ibk;

	if (!ibk.open("mt_fm.ibk"))
		return;

	if (ibk.readUint32BE() == 0x49424b1a) {
		_adlibPatches = new byte[128 * 30];
		byte *ptr = _adlibPatches;

		memset(_adlibPatches, 0, 128 * 30);

		for (int i = 0; i < 128; i++) {
			byte instr[16];

			ibk.read(instr, 16);

			ptr[0] = instr[0];   // Modulator Sound Characteristics
			ptr[1] = instr[2];   // Modulator Scaling/Output Level
			ptr[2] = ~instr[4];  // Modulator Attack/Decay
			ptr[3] = ~instr[6];  // Modulator Sustain/Release
			ptr[4] = instr[8];   // Modulator Wave Select
			ptr[5] = instr[1];   // Carrier Sound Characteristics
			ptr[6] = instr[3];   // Carrier Scaling/Output Level
			ptr[7] = ~instr[5];  // Carrier Attack/Delay
			ptr[8] = ~instr[7];  // Carrier Sustain/Release
			ptr[9] = instr[9];   // Carrier Wave Select
			ptr[10] = instr[10]; // Feedback/Connection

			// The remaining six bytes are reserved for future use

			ptr += 30;
		}
	}
}

void MidiPlayer::unloadAdlibPatches() {
	delete[] _adlibPatches;
	_adlibPatches = NULL;
}

static const int simon1_gmf_size[] = {
	8900, 12166, 2848, 3442, 4034, 4508, 7064, 9730, 6014, 4742, 3138,
	6570, 5384, 8909, 6457, 16321, 2742, 8968, 4804, 8442, 7717,
	9444, 5800, 1381, 5660, 6684, 2456, 4744, 2455, 1177, 1232,
	17256, 5103, 8794, 4884, 16
};

void MidiPlayer::loadSMF(Common::File *in, int song, bool sfx) {
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
		// 1 BYTE : Loop control. 0 = no loop, 1 = loop

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
		p->loopTrack = (p->data[6] != 0);
	} else {
		p->loopTrack = _loopTrackDefault;
	}

	MidiParser *parser = MidiParser::createParser_SMF();
	parser->property(MidiParser::mpMalformedPitchBends, 1);
	parser->setMidiDriver(this);
	parser->setTimerRate(timerRate);
	if (!parser->loadMusic(p->data, size)) {
		warning("Error reading track");
		delete parser;
		parser = 0;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
}

void MidiPlayer::loadMultipleSMF(Common::File *in, bool sfx) {
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

	p->loopTrack = _loopTrackDefault;

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
}

void MidiPlayer::loadXMIDI(Common::File *in, bool sfx) {
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
		p->loopTrack = _loopTrackDefault;
	} else {
		error("Expected 'FORM' tag but found '%c%c%c%c' instead", buf[0], buf[1], buf[2], buf[3]);
	}

	// In the DOS version of Simon the Sorcerer 2, the music contains lots
	// of XMIDI callback controller events. As far as we know, they aren't
	// actually used, so we disable the callback handler explicitly.

	MidiParser *parser = MidiParser::createParser_XMIDI(NULL);
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

void MidiPlayer::loadS1D(Common::File *in, bool sfx) {
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
	p->loopTrack = _loopTrackDefault;
	p->parser = parser; // That plugs the power cord into the wall
}

} // End of namespace AGOS
