/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/mods/protracker.h"

using Common::File;

namespace AGOS {

void AGOSEngine::playSpeech(uint speech_id, uint vgaSpriteId) {
	if (getGameType() == GType_SIMON1) {
		if (speech_id == 9999) {
			if (_subtitles)
				return;
			if (!getBitFlag(14) && !getBitFlag(28)) {
				setBitFlag(14, true);
				_variableArray[100] = 15;
				animate(4, 1, 130, 0, 0, 0);
				waitForSync(130);
			}
			_skipVgaWait = true;
		} else {
			if (_subtitles && _scriptVar2) {
				animate(4, 2, 204, 0, 0, 0);
				waitForSync(204);
				stopAnimate(204);
			}
			stopAnimate(vgaSpriteId + 201);
			loadVoice(speech_id);
			animate(4, 2, vgaSpriteId + 201, 0, 0, 0);
		}
	} else {
		if (speech_id == 0xFFFF) {
			if (_subtitles)
				return;
			if (!getBitFlag(14) && !getBitFlag(28)) {
				setBitFlag(14, true);
				_variableArray[100] = 5;
				animate(4, 1, 30, 0, 0, 0);
				waitForSync(130);
			}
			_skipVgaWait = true;
		} else {
			if (getGameType() == GType_SIMON2 && _subtitles && _language != Common::HB_ISR) {
				loadVoice(speech_id);
				return;
			}

			if (_subtitles && _scriptVar2) {
				animate(4, 2, 5, 0, 0, 0);
				waitForSync(205);
				stopAnimateSimon2(2,5);
			}

			stopAnimateSimon2(2, vgaSpriteId + 2);
			loadVoice(speech_id);
			animate(4, 2, vgaSpriteId + 2, 0, 0, 0);
		}
	}
}

void AGOSEngine::skipSpeech() {
	_sound->stopVoice();
	if (!getBitFlag(28)) {
		setBitFlag(14, true);
		if (getGameType() == GType_FF) {
			_variableArray[103] = 5;
			animate(4, 2, 13, 0, 0, 0);
			waitForSync(213);
			stopAnimateSimon2(2, 1);
		} else if (getGameType() == GType_SIMON2) {
			_variableArray[100] = 5;
			animate(4, 1, 30, 0, 0, 0);
			waitForSync(130);
			stopAnimateSimon2(2, 1);
		} else {
			_variableArray[100] = 15;
			animate(4, 1, 130, 0, 0, 0);
			waitForSync(130);
			stopAnimate(1);
		}
	}
}

void AGOSEngine::loadMusic(uint music) {
	char buf[4];

	if (getPlatform() == Common::kPlatformAtariST) {
		// TODO: Add support for music format used by Elvira 2
	} else if (getPlatform() == Common::kPlatformAmiga) {
		_mixer->stopHandle(_modHandle);

		char filename[15];
		File f;

		if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO)
			sprintf(filename, "elvira2");
		else
			sprintf(filename, "%dtune", music);

		f.open(filename);
		if (f.isOpen() == false) {
			error("loadMusic: Can't load module from '%s'", filename);
		}

		Audio::AudioStream *audioStream;
		if (!(getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) &&
			getFeatures() & GF_CRUNCHED) {

			uint srcSize = f.size();
			byte *srcBuf = (byte *)malloc(srcSize);
			if (f.read(srcBuf, srcSize) != srcSize)
				error("loadMusic: Read failed");

			uint dstSize = READ_BE_UINT32(srcBuf + srcSize - 4);
			byte *dstBuf = (byte *)malloc(dstSize);
			decrunchFile(srcBuf, dstBuf, srcSize);
			free(srcBuf);

			Common::MemoryReadStream stream(dstBuf, dstSize);
			audioStream = Audio::makeProtrackerStream(&stream, _mixer->getOutputRate());
		} else {
			audioStream = Audio::makeProtrackerStream(&f, _mixer->getOutputRate());
		}

		_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_modHandle, audioStream);
	} else if (getGameType() == GType_SIMON2) {
		midi.stop();
		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
		_gameFile->read(buf, 4);
		if (!memcmp(buf, "FORM", 4)) {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
			midi.loadXMIDI (_gameFile);
		} else {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
			midi.loadMultipleSMF (_gameFile);
		}

		_lastMusicPlayed = music;
		_nextMusicToPlay = -1;
	} else if (getGameType() == GType_SIMON1) {
		midi.stop();
		midi.setLoop (true); // Must do this BEFORE loading music. (GMF may have its own override.)

		if (getFeatures() & GF_TALKIE) {
			// FIXME: The very last music resource, a cymbal crash for when the
			// two demons crash into each other, should NOT be looped like the
			// other music tracks. In simon1dos/talkie the GMF resource includes
			// a loop override that acomplishes this, but there seems to be nothing
			// for this in the SMF resources.
			if (music == 35)
				midi.setLoop (false);

			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
			_gameFile->read(buf, 4);
			if (!memcmp(buf, "GMF\x1", 4)) {
				_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
				midi.loadSMF (_gameFile, music);
			} else {
				_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
				midi.loadMultipleSMF (_gameFile);
			}

		} else {
			char filename[15];
			File f;
			sprintf(filename, "MOD%d.MUS", music);
			f.open(filename);
			if (f.isOpen() == false)
				error("loadMusic: Can't load music from '%s'", filename);

			if (getFeatures() & GF_DEMO)
				midi.loadS1D (&f);
			else
				midi.loadSMF (&f, music);
		}

		midi.startTrack (0);
	} else {
		midi.stop();
		midi.setLoop (true); // Must do this BEFORE loading music.

		char filename[15];
		File f;
		sprintf(filename, "MOD%d.MUS", music);
		f.open(filename);
		if (f.isOpen() == false)
			error("loadMusic: Can't load music from '%s'", filename);

		midi.loadS1D (&f);
		midi.startTrack (0);
	}
}

void AGOSEngine::playSting(uint a) {
	if (!midi._enable_sfx)
		return;

	char filename[15];

	File mus_file;
	uint16 mus_offset;

	sprintf(filename, "STINGS%i.MUS", _soundFileId);
	mus_file.open(filename);
	if (!mus_file.isOpen())
		error("playSting: Can't load sound effect from '%s'", filename);

	mus_file.seek(a * 2, SEEK_SET);
	mus_offset = mus_file.readUint16LE();
	if (mus_file.ioFailed())
		error("playSting: Can't read sting %d offset", a);

	mus_file.seek(mus_offset, SEEK_SET);
	midi.loadSMF(&mus_file, a, true);
	midi.startTrack(0);
}

static const byte elvira1_soundTable[100] = {
	0, 2, 0, 1, 0, 0, 0, 0, 0, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 6, 4, 0, 0, 9, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 5, 0, 6, 6, 0, 0,
	0, 5, 0, 0, 6, 0, 0, 0, 0, 8,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool AGOSEngine::loadVGASoundFile(uint id, uint type) {
	File in;
	char filename[15];
	byte *dst;
	uint32 srcSize, dstSize;

	if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
		if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
			if (id == 20)
				sprintf(filename, "D%d.out", type);
			else if (id == 26)
				sprintf(filename, "J%d.out", type);
			else if (id == 27)
				sprintf(filename, "K%d.out", type);
			else if (id == 33)
				sprintf(filename, "Q%d.out", type);
			else if (id == 34)
				sprintf(filename, "R%d.out", type);
			else
				sprintf(filename, "%.1d%d.out", id, type);
		} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
			sprintf(filename, "%.2d%d.out", id, type);
		} else {
			sprintf(filename, "%.3d%d.out", id, type);
		}
	} else {
		if (getGameType() == GType_ELVIRA1) {
			if (elvira1_soundTable[id] == 0)
				return false;

			sprintf(filename, "%.2d.SND", elvira1_soundTable[id]);
		} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
			sprintf(filename, "%.2d%d.VGA", id, type);
		} else {
			sprintf(filename, "%.3d%d.VGA", id, type);
		}
	}

	in.open(filename);
	if (in.isOpen() == false || in.size() == 0) {
		return false;
	}

	dstSize = srcSize = in.size();
	if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
		byte *srcBuffer = (byte *)malloc(srcSize);
		if (in.read(srcBuffer, srcSize) != srcSize)
			error("loadVGASoundFile: Read failed");

		dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
		dst = allocBlock (dstSize);
		decrunchFile(srcBuffer, dst, srcSize);
		free(srcBuffer);
	} else {
		dst = allocBlock(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadVGASoundFile: Read failed");
	}
	in.close();

	return true;
}

static const char *dimpSoundList[32] = {
	"Beep",
	"Birth",
	"Boiling",
	"Burp",
	"Cough",
	"Die1",
	"Die2",
	"Fart",
	"Inject",
	"Killchik",
	"Puke",
	"Lights",
	"Shock",
	"Snore",
	"Snotty",
	"Whip",
	"Whistle",
	"Work1",
	"Work2",
	"Yawn",
	"And0w",
	"And0x",
	"And0y",
	"And0z",
	"And10",
	"And11",
	"And12",
	"And13",
	"And14",
	"And15",
	"And16",
	"And17",
};


void AGOSEngine::loadSound(uint sound, int pan, int vol, uint type) {
	byte *dst;

	if (getGameId() == GID_DIMP) {
		File in;
		char filename[15];

		assert(sound >= 1 && sound <= 32);
		sprintf(filename, "%s.wav", dimpSoundList[sound - 1]);

		in.open(filename);
		if (in.isOpen() == false)
			error("loadSound: Can't load %s", filename);

		uint32 dstSize = in.size();
		dst = (byte *)malloc(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadSound: Read failed");
		in.close();
	} else if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"sfxindex.dat", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"effects.wav", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sfx%d.wav", file);
		else
			sprintf(filename, "effects.wav");

		dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
	} else {
		if (!_curSfxFile)
			error("loadSound: Can't load sound data file '%d3.VGA'", _zoneNumber);

		dst = _curSfxFile + READ_LE_UINT32(_curSfxFile + sound * 4);
	}

	if (type == 3)
		_sound->playSfx5Data(dst, sound, pan, vol);
	else if (type == 2)
		_sound->playAmbientData(dst, sound, pan, vol);
	else
		_sound->playSfxData(dst, sound, pan, vol);
}

void AGOSEngine::loadVoice(uint speechId) {
	if (getGameType() == GType_PP && speechId == 99) {
		_sound->stopVoice();
		return;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"spindex.dat", speechId, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"speech.wav", speechId, file, offset, srcSize, dstSize);
		}

		// Voice segment doesn't exist
		if (offset == 0xFFFFFFFF && srcSize == 0xFFFFFFFF && dstSize == 0xFFFFFFFF) {
			debug(0, "loadVoice: speechId %d removed", speechId);
			return;
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sp%d.wav", file);
		else
			sprintf(filename, "speech.wav");

		byte *dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		_sound->playVoiceData(dst, speechId);
	} else {
		_sound->playVoice(speechId);
	}
}

} // End of namespace AGOS
