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

#include "made/pmvplayer.h"
#include "made/made.h"
#include "made/screen.h"
#include "made/graphics.h"

#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/events.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

#include "graphics/surface.h"

namespace Made {

#ifdef USE_TTS

// English seems to be the only language that doesn't have voice clips for these lines
static const char *introOpeningLines[] = {
	"You are standing by a white house",
	"Behind House\nYou are standing behind the white house. In one corner is a small window which is slightly ajar.",
	"Go southwest then go northwest",
	"West of House\nYou are standing in a field west of a white house with a boarded front door. There is a small mailbox here.",
	"Open mailbox"
};

static const char *openingCreditsEnglish[] = {
	"Design: Doug Barnett",
	"Art Direction: Joe Asperin",
	"Technical Direction: William Volk",
	"Screenplay: Michele Em",
	"Music: Nathan Wang and Teri Mason",
	"Producer: Eddie Dombrower"
};

static const char *openingCreditsGerman[] = {
	"Entwurf: Doug Barnett",
	"K\201nstlerischer Leitung: Joe Asperin",
	"Technische Leitung: William Volk",
	"Drehbuch: Michele Em",
	"Musik: Nathan Wang und Teri Mason",
	"Produzent: Eddie Dombrower"
};

static const char *openingCreditsItalian[] = {
	"Disegno: Doug Barnett",
	"Direzione Artistica: Joe Asperin",
	"Direzione Tecnica: William Volk",
	"Sceneggiatura: Michele Em",
	"Musica: Nathan Wang e Teri Mason",
	"Produttore: Eddie Dombrower"
};

static const char *openingCreditsFrench[] = {
	"Conception: Doug Barnett",
	"Direction Artistique: Joe Asperin",
	"Direction Technique: William Volk",
	"Sc\202nario: Michele Em",
	"Musique: Nathan Wang et Teri Mason",
	"Producteur: Eddie Dombrower"
};

static const char *openingCreditsJapanese[] = {
	"\x83\x66\x83\x55\x83\x43\x83\x93\x81\x45\x83\x5f\x83\x4f\x81\x45\x83\x6f\x81\x5b\x83\x6c\x83\x62\x83\x67",	// デザイン・ダグ・バーネット
	"\x83\x41\x81\x5b\x83\x67\x83\x66\x83\x42\x83\x8c\x83\x4e\x83\x56\x83\x87\x83\x93:"
	"\x83\x57\x83\x87\x81\x5b\x81\x45\x83\x41\x83\x58\x83\x79\x83\x8a\x83\x93",	// アートディレクション：ジョー・アスペリン
	"\x83\x65\x83\x4e\x83\x6a\x83\x4a\x83\x8b\x83\x66\x83\x42\x83\x8c\x83\x4e\x83\x56\x83\x87\x83\x93:"
	"\x83\x45\x83\x42\x83\x8a\x83\x41\x83\x80\x81\x45\x83\x94\x83\x48\x83\x8b\x83\x4e",	// テクニカルディレクション：ウィリアム・ヴォルク
	"\x8b\x72\x96\x7b:\x83\x7e\x83\x56\x83\x46\x83\x8b\x81\x45\x83\x47\x83\x80",	// 脚本：ミシェル・エム
	"\x89\xb9\x8a\x79:\x83\x6c\x83\x43\x83\x54\x83\x93\x81\x45\x83\x8f\x83\x93\x83\x67\x83\x65\x83\x8a\x81\x5b"
	"\x81\x45\x83\x81\x83\x43\x83\x5c\x83\x93",	// 音楽：ネイサン・ワンとテリー・メイソン
	"\x83\x76\x83\x8d\x83\x66\x83\x85\x81\x5b\x83\x54\x81\x5b:\x83\x47\x83\x66\x83\x42\x81\x45\x83\x68\x83\x93"
	"\x83\x75\x83\x8d\x83\x8f\x81\x5b"	// プロデューサー: エディ・ドンブロワー
};

enum IntroTextFrame {
	kStandingByHouse = 20,
	kBehindHouse = 53,
	kGoSouthwest = 170,
	kWestOfHouse = 312,
	kOpenMailbox = 430,
	kDesign = 716,
	kArtDirection = 773,
	kTechnicalDirection = 833,
	kScreenplay = 892,
	kMusic = 948,
	kProducer = 1004
};

#endif

PmvPlayer::PmvPlayer(MadeEngine *vm, Audio::Mixer *mixer) : _fd(nullptr), _vm(vm), _mixer(mixer) {
	_audioStream = nullptr;
	_surface = nullptr;
	_aborted = false;
}

PmvPlayer::~PmvPlayer() {
}

bool PmvPlayer::play(const char *filename) {
	_aborted = false;
	_surface = nullptr;

	_fd = new Common::File();
	if (!_fd->open(filename)) {
		delete _fd;
		return false;
	}

	uint32 chunkType, chunkSize, prevChunkSize = 0;

	readChunk(chunkType, chunkSize);	// "MOVE"
	if (chunkType != MKTAG('M','O','V','E')) {
		warning("Unexpected PMV video header, expected 'MOVE'");
		delete _fd;
		return false;
	}

	readChunk(chunkType, chunkSize);	// "MHED"
	if (chunkType != MKTAG('M','H','E','D')) {
		warning("Unexpected PMV video header, expected 'MHED'");
		delete _fd;
		return false;
	}

	uint frameDelay = _fd->readUint16LE();
	_fd->skip(4);	// always 0?
	uint frameCount = _fd->readUint16LE();
	_fd->skip(4);	// always 0?

	uint soundFreq = _fd->readUint16LE();
	// Note: There seem to be weird sound frequencies in PMV videos.
	// Not sure why, but leaving those original frequencies intact
	// results to sound being choppy. Therefore, we set them to more
	// "common" values here (11025 instead of 11127 and 22050 instead
	// of 22254)
	if (soundFreq == 11127)
		soundFreq = 11025;

	if (soundFreq == 22254)
		soundFreq = 22050;

	for (int i = 0; i < 22; i++) {
		int unk = _fd->readUint16LE();
		debug(2, "%i ", unk);
	}

	_mixer->stopAll();

	// Read palette
	_fd->read(_paletteRGB, 768);
	_vm->_screen->setRGBPalette(_paletteRGB);

	uint32 frameNumber = 0;
	uint16 chunkCount = 0;
	uint32 soundSize = 0;
	uint32 soundChunkOfs = 0, palChunkOfs = 0;
	uint32 palSize = 0;
	byte *frameData = nullptr, *audioData, *soundData, *palData, *imageData;
	bool firstTime = true;

	uint32 skipFrames = 0;

	uint32 bytesRead;
	uint16 width, height, cmdOffs, pixelOffs, maskOffs, lineSize;

	// TODO: Sound can still be a little choppy. A bug in the decoder or -
	// perhaps more likely - do we have to implement double buffering to
	// get it to work well?
	_audioStream = Audio::makeQueuingAudioStream(soundFreq, false);

	SoundDecoderData *soundDecoderData = new SoundDecoderData();

	// First cutscene after the opening credits finish
	if (strcmp(filename, "FWIZ01X1.PMV") == 0) {
		_vm->_openingCreditsOpen = false;
	}

	while (!_vm->shouldQuit() && !_aborted && !_fd->eos() && frameNumber < frameCount) {

		int32 frameTime = _vm->getTotalPlayTime();

		readChunk(chunkType, chunkSize);
		if (chunkType != MKTAG('M','F','R','M')) {
			warning("Unknown chunk type");
		}

		// Only reallocate the frame data buffer if its size has changed
		if (prevChunkSize != chunkSize || !frameData) {
			delete[] frameData;
			frameData = new byte[chunkSize];
		}

		prevChunkSize = chunkSize;

		bytesRead = _fd->read(frameData, chunkSize);

		if (bytesRead < chunkSize || _fd->eos())
			break;

		soundChunkOfs = READ_LE_UINT32(frameData + 8);
		palChunkOfs = READ_LE_UINT32(frameData + 16);

		// Handle audio
		if (soundChunkOfs) {
			audioData = frameData + soundChunkOfs - 8;
			chunkSize = READ_LE_UINT16(audioData + 4);
			chunkCount = READ_LE_UINT16(audioData + 6);

			debug(1, "chunkCount = %d; chunkSize = %d; total = %d\n", chunkCount, chunkSize, chunkCount * chunkSize);

			soundSize = chunkCount * chunkSize;
			soundData = (byte *)malloc(soundSize);
			decompressSound(audioData + 8, soundData, chunkSize, chunkCount, nullptr, soundDecoderData);
			_audioStream->queueBuffer(soundData, soundSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
		}

		// Handle palette
		if (palChunkOfs) {
			palData = frameData + palChunkOfs - 8;
			palSize = READ_LE_UINT32(palData + 4);
			decompressPalette(palData + 8, _paletteRGB, palSize);
			_vm->_screen->setRGBPalette(_paletteRGB);
		}

		// Handle video
		imageData = frameData + READ_LE_UINT32(frameData + 12) - 8;

		// frameNum @0
		width = READ_LE_UINT16(imageData + 8);
		height = READ_LE_UINT16(imageData + 10);
		cmdOffs = READ_LE_UINT16(imageData + 12);
		pixelOffs = READ_LE_UINT16(imageData + 16);
		maskOffs = READ_LE_UINT16(imageData + 20);
		lineSize = READ_LE_UINT16(imageData + 24);

		debug(2, "width = %d; height = %d; cmdOffs = %04X; pixelOffs = %04X; maskOffs = %04X; lineSize = %d\n",
			width, height, cmdOffs, pixelOffs, maskOffs, lineSize);

		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		}

		decompressMovieImage(imageData, *_surface, cmdOffs, pixelOffs, maskOffs, lineSize);

		if (firstTime) {
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioStreamHandle, _audioStream);
			skipFrames = 0;
			firstTime = false;
		}

		handleEvents();
		updateScreen();

		if (skipFrames == 0) {
			uint32 soundElapsedTime = _vm->_mixer->getElapsedTime(_audioStreamHandle).msecs();
			int32 waitTime = (frameNumber * frameDelay) -
				soundElapsedTime - (_vm->getTotalPlayTime() - frameTime);

			if (waitTime < 0) {
				skipFrames = -waitTime / frameDelay;
				warning("Video A/V sync broken, skipping %d frame(s)", skipFrames + 1);
			} else if (waitTime > 0)
				g_system->delayMillis(waitTime);

		} else
			skipFrames--;

#ifdef USE_TTS
		if (strcmp(filename, "fintro00.pmv") == 0 || strcmp(filename, "fintro01.pmv") == 0) {
			const char **texts;

			switch (_vm->getLanguage()) {
			case Common::EN_ANY:
				if (frameNumber < kDesign) {
					texts = introOpeningLines;
				} else {
					texts = openingCreditsEnglish;
				}
				break;
			case Common::DE_DEU:
				texts = openingCreditsGerman;
				break;
			case Common::IT_ITA:
				texts = openingCreditsItalian;
				break;
			case Common::FR_FRA:
				texts = openingCreditsFrench;
				break;
			case Common::JA_JPN:
				texts = openingCreditsJapanese;
				break;
			case Common::KO_KOR:
				texts = openingCreditsEnglish;
				break;
			default:
				texts = openingCreditsEnglish;
			}

			int index = -1;

			switch (frameNumber) {
			case kStandingByHouse:
			case kDesign:
				index = 0;
				break;
			case kBehindHouse:
			case kArtDirection:
				index = 1;
				break;
			case kGoSouthwest:
			case kTechnicalDirection:
				index = 2;
				break;
			case kWestOfHouse:
			case kScreenplay:
				index = 3;
				break;
			case kOpenMailbox:
			case kMusic:
				index = 4;
				break;
			case kProducer:
				index = 5;
			}

			if (index != -1 && (_vm->getLanguage() == Common::EN_ANY || frameNumber >= kDesign)) {
				_vm->sayText(texts[index], Common::TextToSpeechManager::QUEUE);
			}
		}
#endif

		frameNumber++;

	}

	delete soundDecoderData;
	delete[] frameData;

	_audioStream->finish();
	_mixer->stopHandle(_audioStreamHandle);

	//delete _audioStream;
	delete _fd;

	if(_surface)
		_surface->free();

	delete _surface;

	return !_aborted;

}

void PmvPlayer::readChunk(uint32 &chunkType, uint32 &chunkSize) {
	chunkType = _fd->readUint32BE();
	chunkSize = _fd->readUint32LE();

	debug(2, "ofs = %08X; chunkType = %c%c%c%c; chunkSize = %d\n",
		(int)_fd->pos(),
		(chunkType >> 24) & 0xFF, (chunkType >> 16) & 0xFF, (chunkType >> 8) & 0xFF, chunkType & 0xFF,
		chunkSize);

}

void PmvPlayer::handleEvents() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_aborted = true;
				_vm->stopTextToSpeech();
			}
			break;
		default:
			break;
		}
	}
}

void PmvPlayer::updateScreen() {
	_vm->_system->copyRectToScreen(_surface->getPixels(), _surface->pitch,
									(320 - _surface->w) / 2, (200 - _surface->h) / 2, _surface->w, _surface->h);
	_vm->_system->updateScreen();
}

void PmvPlayer::decompressPalette(byte *palData, byte *outPal, uint32 palDataSize) {
	byte *palDataEnd = palData + palDataSize;
	while (palData < palDataEnd) {
		byte count = *palData++;
		byte entry = *palData++;
		if (count == 255 && entry == 255)
			break;
		memcpy(&outPal[entry * 3], palData, (count + 1) * 3);
		palData += (count + 1) * 3;
	}
}

}
