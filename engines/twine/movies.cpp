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

#include "twine/movies.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/language.h"
#include "common/str.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "image/gif.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/grid.h"
#include "twine/shared.h"
#include "twine/twine.h"
#include "video/smk_decoder.h"

namespace TwinE {

/** FLA Frame Opcode types */
enum FlaFrameOpcode {
	kLoadPalette = 1,
	kInfo = 2,
	kPlaySample = 3,
	kSampleBalance = 4,
	kStopSample = 5,
	kDeltaFrame = 6,
	kBlackFrame = 7,
	kBrownFrame = 8,
	kCopy = 9,
	kFliCopy = 16
};

/** FLA movie sample structure */
struct FLASampleStruct {
	/** Number os samples */
	int16 sampleNum = 0;
	/** Sample frequency */
	int16 freq = 0;
	/** Numbers of time to repeat */
	int16 repeat = 0;
	uint8 balance = 0;
	uint8 volumeLeft = 0;
	uint8 volumeRight = 0;
};

/** FLA movie extension */
#define FLA_EXT ".fla"

void Movies::drawKeyFrame(Common::MemoryReadStream &stream, int32 width, int32 height) {
	uint8 *destPtr = (uint8 *)_flaBuffer;
	uint8 *startOfLine = destPtr;

	for (int32 y = 0; y < height; ++y) {
		const int8 lineEntryCount = stream.readByte();

		for (int8 a = 0; a < lineEntryCount; a++) {
			const int8 rleFlag = stream.readByte();

			if (rleFlag < 0) {
				const int8 rleCnt = ABS(rleFlag);
				for (int8 b = 0; b < rleCnt; ++b) {
					*destPtr++ = stream.readByte();
				}
			} else {
				const char colorFill = stream.readByte();
				Common::fill(&destPtr[0], &destPtr[rleFlag], colorFill);
				destPtr += rleFlag;
			}
		}

		startOfLine = destPtr = startOfLine + width;
	}
}

void Movies::drawDeltaFrame(Common::MemoryReadStream &stream, int32 width) {
	const uint16 skip = stream.readUint16LE() * width;
	const int32 height = stream.readSint16LE();

	uint8 *destPtr = (uint8 *)_flaBuffer + skip;
	uint8 *startOfLine = destPtr;
	for (int32 y = 0; y < height; ++y) {
		const int8 lineEntryCount = stream.readByte();

		for (int8 a = 0; a < lineEntryCount; ++a) {
			destPtr += stream.readByte();
			const int8 rleFlag = stream.readByte();

			if (rleFlag > 0) {
				for (int8 b = 0; b < rleFlag; ++b) {
					*destPtr++ = stream.readByte();
				}
			} else {
				const char colorFill = stream.readByte();
				const int8 rleCnt = ABS(rleFlag);
				Common::fill(&destPtr[0], &destPtr[rleCnt], colorFill);
				destPtr += rleCnt;
			}
		}

		startOfLine = destPtr = startOfLine + width;
	}
}

void Movies::scaleFla2x() {
	uint8 *source = (uint8 *)_flaBuffer;
	uint8 *dest = (uint8 *)_engine->_imageBuffer.getPixels();

	if (_engine->_cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->_imageBuffer.w * 40], 0x00);
		dest += _engine->_imageBuffer.w * 40;
	}

	for (int32 i = 0; i < FLASCREEN_HEIGHT; i++) {
		for (int32 j = 0; j < FLASCREEN_WIDTH; j++) {
			*dest++ = *source;
			*dest++ = *source++;
		}
		if (_engine->_cfgfile.Movie == CONF_MOVIE_FLAWIDE) { // include wide bars
			memcpy(dest, dest - _engine->_imageBuffer.w, FLASCREEN_WIDTH * 2);
			dest += FLASCREEN_WIDTH * 2;
		} else { // stretch the movie like original game.
			if (i % 2) {
				memcpy(dest, dest - _engine->_imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
			if (i % 10) {
				memcpy(dest, dest - _engine->_imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
		}
	}

	if (_engine->_cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->_imageBuffer.w * 40], 0x00);
	}
}

void Movies::drawNextFrameFla() {
	FLASampleStruct sample;

	_frameData.nbFrames = _file.readSint16LE();
	_frameData.offsetNextFrame = _file.readSint32LE();
	if (_frameData.offsetNextFrame > _engine->_imageBuffer.w * _engine->_imageBuffer.h) {
		warning("Skipping video frame - it would exceed the screen buffer: %i", _frameData.offsetNextFrame);
		return;
	}

	uint8 *outBuf = (uint8 *)_engine->_imageBuffer.getPixels();
	_file.read(outBuf, _frameData.offsetNextFrame);

	if ((int32)_frameData.nbFrames <= 0) {
		return;
	}

	Common::MemoryReadStream stream(outBuf, _frameData.offsetNextFrame);
	for (int32 frame = 0; frame < _frameData.nbFrames; ++frame) {
		const uint16 opcode = stream.readUint16LE();
		const uint16 opcodeBlockSize = stream.readUint16LE();
		const int64 pos = stream.pos();

		switch (opcode) {
		case kLoadPalette: {
			int16 numOfColor = stream.readSint16LE();
			int16 startColor = stream.readSint16LE();
			if (_paletteOrg.size() < (uint)(numOfColor + startColor)) {
				Graphics::Palette palette(numOfColor + startColor);
				palette.set(_paletteOrg, 0, _paletteOrg.size());
				_paletteOrg = palette;
			}
			for (int16 i = 0; i < numOfColor; ++i) {
				const byte r = stream.readByte();
				const byte g = stream.readByte();
				const byte b = stream.readByte();
				_paletteOrg.set(i + startColor, r, g, b);
			}
			break;
		}
		case kInfo: {
			int16 innerOpcpde = stream.readSint16LE();
			switch (innerOpcpde) {
			case 1: // fla flute
				_engine->_music->playMidiFile(26);
				break;
			case 2:
				// FLA movies don't use cross fade
				// fade out tricky
				if (_fadeOut != 1) {
					_engine->_screens->fadeToBlack(_paletteOrg);
					_fadeOut = 1;
				}
				break;
			case 3:
				_flagFirst = true;
				break;
			case 4:
				// TODO: fade out for 1 second before we stop it
				_engine->_music->stopMusicMidi();
				break;
			}
			break;
		}
		case kPlaySample: {
			sample.sampleNum = stream.readSint16LE();
			sample.freq = stream.readSint16LE();
			sample.repeat = stream.readSint16LE();
			sample.balance = stream.readByte();
			sample.volumeLeft = stream.readByte();
			sample.volumeRight = stream.readByte();
			_engine->_sound->playFlaSample(sample.sampleNum, sample.repeat, sample.balance, sample.volumeLeft, sample.volumeRight);
			break;
		}
		case kStopSample: {
			const int16 sampleNum = stream.readSint16LE();
			if (sampleNum == -1) {
				_engine->_sound->stopSamples();
			} else {
				_engine->_sound->stopSample(sampleNum);
			}
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(stream, FLASCREEN_WIDTH);
			if (_fadeOut == 1) {
				++_fadeOutFrames;
			}
			break;
		}
		case kBrownFrame: {
			drawKeyFrame(stream, FLASCREEN_WIDTH, _flaHeaderData.ysize);
			break;
		}
		case kBlackFrame: {
			const Common::Rect rect(0, 0, FLASCREEN_WIDTH - 1, FLASCREEN_HEIGHT - 1);
			_engine->_interface->box(rect, 0);
			break;
		}
		case kCopy:
		case kFliCopy: {
			const Common::Rect rect(0, 0, 80, 200);
			byte *ptr = (byte *)_engine->_frontVideoBuffer.getPixels();
			for (int y = rect.top; y < rect.bottom; ++y) {
				for (int x = rect.left; x < rect.right; ++x) {
					*ptr++ = stream.readByte();
				}
				ptr = ptr + rect.width();
			}
			_engine->_frontVideoBuffer.addDirtyRect(rect);
			break;
		}
		case kSampleBalance: {
			/* int16 num = */ stream.readSint16LE();
			/* uint8 offset = */ stream.readByte();
			stream.skip(1); // padding
			/* int16 balance = */ stream.readSint16LE();
			/* uint8 volumeLeft = */ stream.readByte();
			/* uint8 volumeRight = */ stream.readByte();
			// TODO: change balance
			break;
		}
		default: {
			break;
		}
		}

		stream.seek(pos + opcodeBlockSize);
	}
}

Movies::Movies(TwinEEngine *engine) : _engine(engine) {}

void Movies::prepareGIF(int index) {
	Image::GIFDecoder decoder;
	Common::SeekableReadStream *stream = HQR::makeReadStream(Resources::HQR_FLAGIF_FILE, index);
	if (stream == nullptr) {
		warning("Failed to load gif hqr entry with id %i from %s", index, Resources::HQR_FLAGIF_FILE);
		return;
	}
	if (!decoder.loadStream(*stream)) {
		delete stream;
		warning("Failed to load gif with id %i from %s", index, Resources::HQR_FLAGIF_FILE);
		return;
	}
	const Graphics::Surface *surface = decoder.getSurface();
	_engine->setPalette(0, decoder.getPaletteColorCount(), decoder.getPalette());
	Graphics::ManagedSurface& target = _engine->_frontVideoBuffer;
	const Common::Rect surfaceBounds(0, 0, surface->w, surface->h);
	target.transBlitFrom(*surface, surfaceBounds, target.getBounds(), 0, false, 0xff, nullptr, true);
	debugC(1, TwinE::kDebugMovies, "Show gif with id %i from %s", index, Resources::HQR_FLAGIF_FILE);
	delete stream;
	_engine->delaySkip(5000);
	_engine->setPalette(_engine->_screens->_ptrPal);
}

void Movies::playGIFMovie(const char *flaName) {
	if (!Common::File::exists(Resources::HQR_FLAGIF_FILE)) {
		warning("%s file doesn't exist", Resources::HQR_FLAGIF_FILE);
		return;
	}

	Common::String name(flaName);
	name.toLowercase();

	debugC(1, TwinE::kDebugMovies, "Play gif %s", name.c_str());
	// TODO: use the HQR 23th entry (movies informations)
	// TODO: there are gifs [1-18]
	if (name == FLA_INTROD) {
		prepareGIF(3);
		prepareGIF(4);
		prepareGIF(5);
	} else if (name == "bateau" || name == "bateau2") {
		prepareGIF(7);
	} else if (name == "navette") {
		prepareGIF(15);
	} else if (name == "templebu") {
		prepareGIF(12);
	} else if (name == "flute2") {
		prepareGIF(8); // TODO: same as glass2?
	} else if (name == "glass2") {
		prepareGIF(8); // TODO: same as flute2?
	} else if (name == "surf") {
		prepareGIF(9);
	} else if (name == "verser" || name == "verser2") {
		prepareGIF(10);
	} else if (name == "neige2") {
		prepareGIF(11);
	} else if (name == "capture") {
		prepareGIF(14); // TODO: same as sendel?
	} else if (name == "sendel") {
		prepareGIF(14); // TODO: same as capture?
	} else if (name == "sendel2") {
		prepareGIF(17);
	} else if (name == FLA_DRAGON3) {
		prepareGIF(1);
		prepareGIF(2);
	} else if (name == "baffe" || name.matchString("baffe#")) {
		prepareGIF(6);
	} else {
		warning("unknown gif image: %s", name.c_str());
	}
}

bool Movies::playMovie(const char *name) { // PlayAnimFla
	if (_engine->isLBA2()) {
		const int index = _engine->_resources->findSmkMovieIndex(name);
		return playSmkMovie(name, index);
	}

	Common::String fileNamePath = name;
	const size_t n = fileNamePath.findLastOf(".");
	if (n != Common::String::npos) {
		fileNamePath.erase(n);
	}

	if (_engine->_cfgfile.Movie == CONF_MOVIE_FLAGIF) {
		playGIFMovie(fileNamePath.c_str());
		return true;
	}

	_fadeOut = -1;
	_fadeOutFrames = 0;

	_file.close();
	if (!_file.open(Common::Path(fileNamePath + FLA_EXT))) {
		warning("Failed to open fla movie '%s'", fileNamePath.c_str());
		playGIFMovie(fileNamePath.c_str());
		_engine->_screens->setBlackPal();
		_engine->_screens->clearScreen();
		return true;
	}

	const uint32 version = _file.readUint32BE();
	_file.skip(2); // version field is 5 bytes - and one padding byte
	_flaHeaderData.numOfFrames = _file.readUint32LE();
	_flaHeaderData.speed = _file.readByte();
	_file.skip(1); // padding byte
	_flaHeaderData.xsize = _file.readUint16LE();
	_flaHeaderData.ysize = _file.readUint16LE();

	_samplesInFla = (int16)_file.readUint16LE();
	/*const uint16 offsetFrame1 =*/ _file.readUint16LE();

	// sample number int16
	// loop int16
	_file.skip(4 * _samplesInFla);

	bool finished = false;
	if (version == MKTAG('V', '1', '.', '3')) {
		int32 currentFrame = 0;

		debugC(1, TwinE::kDebugMovies, "Play fla: %s", name);

		ScopedKeyMap scopedKeyMap(_engine, cutsceneKeyMapId);

		_flagFirst = true;
		do {
			FrameMarker frame(_engine, _flaHeaderData.speed);
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			if (currentFrame == _flaHeaderData.numOfFrames) {
				finished = true;
				break;
			}
			drawNextFrameFla();
			scaleFla2x();
			_engine->_frontVideoBuffer.blitFrom(_engine->_imageBuffer, _engine->_imageBuffer.getBounds(), _engine->_frontVideoBuffer.getBounds());

			// Only blit to screen if isn't a fade
			if (_fadeOut == -1) {
				if (currentFrame == 0) {
					// fade in the first frame
					_engine->_screens->fadeToPal(_paletteOrg);
				} else {
					_engine->setPalette(_paletteOrg);
				}
			}

			// TRICKY: fade in tricky
			if (_fadeOutFrames >= 2) {
				_engine->_screens->fadeToPal(_paletteOrg);
				_fadeOut = -1;
				_fadeOutFrames = 0;
			}

			currentFrame++;
		} while (!_engine->_input->toggleAbortAction());
	} else {
		warning("Unsupported fla version: %u, %s", version, fileNamePath.c_str());
	}

	_engine->_screens->fadeToBlack(_paletteOrg);

	_engine->_sound->stopSamples();
	_engine->_screens->setBlackPal();
	_engine->_screens->clearScreen();
	return finished;
}

class TwineSmackerDecoder : public Video::SmackerDecoder {
public:
	void enableLanguage(int track, int volume) {
		AudioTrack* audio = getAudioTrack(track);
		if (audio == nullptr) {
			return;
		}
		audio->setMute(false);
		audio->setVolume(CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume));
	}
};

bool Movies::playSmkMovie(const char *name, int index) {
	assert(_engine->isLBA2());
	TwineSmackerDecoder decoder;
	Common::SeekableReadStream *stream = HQR::makeReadStream(TwineResource(Resources::HQR_VIDEO_FILE, index));
	if (stream == nullptr) {
		warning("Failed to find smacker video %i", index);
		return false;
	}
	if (!decoder.loadStream(stream)) {
		warning("Failed to load smacker video %i", index);
		return false;
	}
	const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	decoder.setVolume(CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume));
	decoder.start();

	decoder.setAudioTrack(0); // music
	if (ConfMan.getInt("audio_language") > 0) {
		int additionalAudioTrack = -1;
		if (!scumm_strnicmp(name, "INTRO", 5)) {
			switch (_engine->getGameLang()) {
			default:
			case Common::Language::EN_ANY:
			case Common::Language::EN_GRB:
			case Common::Language::EN_USA:
				additionalAudioTrack = 3;
				break;
			case Common::Language::DE_DEU:
				additionalAudioTrack = 2;
				break;
			case Common::Language::FR_FRA:
				additionalAudioTrack = 1;
				break;
			}
		}
		const int speechVolume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
		debugC(1, TwinE::kDebugMovies, "Play additional speech track: %i (of %i tracks)", additionalAudioTrack, decoder.getAudioTrackCount());
		decoder.enableLanguage(additionalAudioTrack, speechVolume);
	} else {
		debugC(1, TwinE::kDebugMovies, "Disabled smacker speech");
	}

	for (;;) {
		if (decoder.endOfVideo()) {
			break;
		}
		FrameMarker frame(_engine);
		_engine->_input->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frameSurf = decoder.decodeNextFrame();
			if (!frameSurf) {
				continue;
			}
			if (decoder.hasDirtyPalette()) {
				_engine->setPalette(0, 256, decoder.getPalette());
			}

			Graphics::ManagedSurface& target = _engine->_frontVideoBuffer;
			const Common::Rect frameBounds(0, 0, frameSurf->w, frameSurf->h);
			target.transBlitFrom(*frameSurf, frameBounds, target.getBounds(), 0, false, 0xff, nullptr, true);
		}
	}

	decoder.close();
	return true;
}

} // namespace TwinE
