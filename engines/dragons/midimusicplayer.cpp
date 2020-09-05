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
 */
#include "common/debug.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "audio/midiparser.h"
#include "audio/soundfont/rawfile.h"
#include "audio/soundfont/vab/vab.h"
#include "audio/soundfont/vgmcoll.h"
#include "midimusicplayer.h"


namespace Dragons {

MidiMusicPlayer::MidiMusicPlayer(BigfileArchive *bigFileArchive, VabSound *musicVab): _musicVab(musicVab), _midiDataSize(0) {
	_midiData = nullptr;
	MidiPlayer::createDriver(MDT_PREFER_FLUID | MDT_MIDI);

	if (_driver->acceptsSoundFontData()) {
		_driver->setEngineSoundFont(loadSoundFont(bigFileArchive));
	} else {
		//If the selected driver doesn't support loading soundfont we should assume we got a fluid Synth V1 and reload
		delete _driver;
		MidiPlayer::createDriver();
	}

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

MidiMusicPlayer::~MidiMusicPlayer() {
	if (isPlaying()) {
		stop();
	}
}

void MidiMusicPlayer::playSong(Common::SeekableReadStream *seqData) {
	Common::StackLock lock(_mutex);

	if (isPlaying()) {
		stop();
	}

	if (seqData->readUint32LE() != MKTAG('S', 'E', 'Q', 'p'))
		error("Failed to find SEQp tag");

	// Make sure we don't have a SEP file (with multiple SEQ's inside)
	if (seqData->readUint32BE() != 1)
		error("Can only play SEQ files, not SEP");

	uint16 ppqn = seqData->readUint16BE();
	uint32 tempo = seqData->readUint16BE() << 8;
	tempo |= seqData->readByte();
	/* uint16 beat = */ seqData->readUint16BE();

	// SEQ is directly based on SMF and we'll use that to our advantage here
	// and convert to SMF and then use the SMF MidiParser.

	// Calculate the SMF size we'll need
	uint32 dataSize = seqData->size() - 15;
	uint32 actualSize = dataSize + 7 + 22;

	// Resize the buffer if necessary
	byte *midiData = resizeMidiBuffer(actualSize);

	// Now construct the header
	WRITE_BE_UINT32(midiData, MKTAG('M', 'T', 'h', 'd'));
	WRITE_BE_UINT32(midiData + 4, 6); // header size
	WRITE_BE_UINT16(midiData + 8, 0); // type 0
	WRITE_BE_UINT16(midiData + 10, 1); // one track
	WRITE_BE_UINT16(midiData + 12, ppqn);
	WRITE_BE_UINT32(midiData + 14, MKTAG('M', 'T', 'r', 'k'));
	WRITE_BE_UINT32(midiData + 18, dataSize + 7); // SEQ data size + tempo change event size

	// Add in a fake tempo change event
	WRITE_BE_UINT32(midiData + 22, 0x00FF5103); // no delta, meta event, tempo change, param size = 3
	WRITE_BE_UINT16(midiData + 26, tempo >> 8);
	midiData[28] = tempo & 0xFF;

	// Now copy in the rest of the events
	seqData->read(midiData + 29, dataSize);

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(midiData, actualSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

		_parser = parser;

		_isLooping = true;
		_isPlaying = true;
	} else {
		delete parser;
	}
}

byte *MidiMusicPlayer::resizeMidiBuffer(uint32 desiredSize) {
	if (_midiData == nullptr) {
		_midiData = (byte *)malloc(desiredSize);
		_midiDataSize = desiredSize;
	} else {
		if (desiredSize > _midiDataSize) {
			_midiData = (byte *)realloc(_midiData, desiredSize);
			_midiDataSize = desiredSize;
		}
	}
	return _midiData;
}

void MidiMusicPlayer::setVolume(int volume) {
//	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume); TODO do we need this?
	MidiPlayer::setVolume(volume);
}

void MidiMusicPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

Common::SeekableReadStream *MidiMusicPlayer::loadSoundFont(BigfileArchive *bigFileArchive) {
	uint32 headSize, bodySize;
	byte *headData = bigFileArchive->load("musx.vh", headSize);
	byte *bodyData = bigFileArchive->load("musx.vb", bodySize);

	byte *vabData = (byte *)malloc(headSize + bodySize);

	memcpy(vabData, headData, headSize);
	memcpy(vabData + headSize, bodyData, bodySize);

	free(headData);
	free(bodyData);

	MemFile *memFile = new MemFile(vabData, headSize + bodySize);
	debug("Loading soundfont2 from musx vab file.");
	Vab *vab = new Vab(memFile, 0);
	vab->LoadVGMFile();
	VGMColl vabCollection;
	SF2File *file = vabCollection.CreateSF2File(vab);
	const byte *bytes = (const byte *)file->SaveToMem();
	uint32 size = file->GetSize();

	delete file;
	delete vab;
	delete memFile;

	return new Common::MemoryReadStream(bytes, size, DisposeAfterUse::YES);
}

} // End of namespace Dragons
