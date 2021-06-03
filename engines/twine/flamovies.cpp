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

#include "twine/flamovies.h"
#include "common/file.h"
#include "common/system.h"
#include "image/gif.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/grid.h"
#include "twine/twine.h"

namespace TwinE {

/** FLA Frame Opcode types */
enum FlaFrameOpcode {
	kLoadPalette = 1,
	kFade = 2,
	kPlaySample = 3,
	kFlaUnknown4 = 4,
	kStopSample = 5,
	kDeltaFrame = 6,
	kFlaUnknown7 = 7,
	kKeyFrame = 8,
	kFlaUnknown9 = 9,
	kFlaUnknown16SameAs9 = 16
};

/** FLA movie sample structure */
struct FLASampleStruct {
	/** Number os samples */
	int16 sampleNum = 0;
	/** Sample frequency */
	int16 freq = 0;
	/** Numbers of time to repeat */
	int16 repeat = 0;
	/** Dummy variable */
	int8 dummy = 0;
	/** Unknown x */
	uint8 x = 0;
	/** Unknown y */
	uint8 y = 0;
};

/** FLA movie extension */
#define FLA_EXT ".fla"

void FlaMovies::drawKeyFrame(Common::MemoryReadStream &stream, int32 width, int32 height) {
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

void FlaMovies::drawDeltaFrame(Common::MemoryReadStream &stream, int32 width) {
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

void FlaMovies::scaleFla2x() {
	uint8 *source = (uint8 *)_flaBuffer;
	uint8 *dest = (uint8 *)_engine->imageBuffer.getPixels();

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->imageBuffer.w * 40], 0x00);
		dest += _engine->imageBuffer.w * 40;
	}

	for (int32 i = 0; i < FLASCREEN_HEIGHT; i++) {
		for (int32 j = 0; j < FLASCREEN_WIDTH; j++) {
			*dest++ = *source;
			*dest++ = *source++;
		}
		if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) { // include wide bars
			memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
			dest += FLASCREEN_WIDTH * 2;
		} else { // stretch the movie like original game.
			if (i % 2) {
				memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
			if (i % 10) {
				memcpy(dest, dest - _engine->imageBuffer.w, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
		}
	}

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		Common::fill(&dest[0], &dest[_engine->imageBuffer.w * 40], 0x00);
	}
}

void FlaMovies::processFrame() {
	FLASampleStruct sample;

	_file.read(&_frameData.videoSize, 2);
	_file.read(&_frameData.frameVar0, 4);
	if (_frameData.frameVar0 > _engine->imageBuffer.w * _engine->imageBuffer.h) {
		warning("Skipping video frame - it would exceed the screen buffer: %i", _frameData.frameVar0);
		return;
	}

	uint8 *outBuf = (uint8 *)_engine->imageBuffer.getPixels();
	_file.read(outBuf, _frameData.frameVar0);

	if ((int32)_frameData.videoSize <= 0) {
		return;
	}

	Common::MemoryReadStream stream(outBuf, _frameData.frameVar0);
	for (int32 frame = 0; frame < _frameData.videoSize; ++frame) {
		const uint16 opcode = stream.readUint16LE();
		const uint16 opcodeBlockSize = stream.readUint16LE();
		const int32 pos = stream.pos();

		switch (opcode) {
		case kLoadPalette: {
			int16 numOfColor = stream.readSint16LE();
			int16 startColor = stream.readSint16LE();
			uint8 *dest = _engine->_screens->palette + (startColor * 3);
			stream.read(dest, numOfColor * 3);
			break;
		}
		case kFade: {
			int16 innerOpcpde = stream.readSint16LE();
			switch (innerOpcpde) {
			case 1:
				_engine->_music->playMidiMusic(26);
				break;
			case 2:
				// FLA movies don't use cross fade
				// fade out tricky
				if (_fadeOut != 1) {
					_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
					_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
					_fadeOut = 1;
				}
				break;
			case 3:
				_flaPaletteVar = true;
				break;
			case 4:
				_engine->_music->stopMidiMusic();
				break;
			}
			break;
		}
		case kPlaySample: {
			sample.sampleNum = stream.readSint16LE();
			sample.freq = stream.readSint16LE();
			sample.repeat = stream.readSint16LE();
			sample.dummy = stream.readSByte();
			sample.x = stream.readByte();
			sample.y = stream.readByte();
			_engine->_sound->playFlaSample(sample.sampleNum, sample.repeat, sample.x, sample.y);
			break;
		}
		case kStopSample: {
			_engine->_sound->stopSample(sample.sampleNum);
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(stream, FLASCREEN_WIDTH);
			if (_fadeOut == 1) {
				++_fadeOutFrames;
			}
			break;
		}
		case kKeyFrame: {
			drawKeyFrame(stream, FLASCREEN_WIDTH, FLASCREEN_HEIGHT);
			break;
		}
		case kFlaUnknown7: {
			byte *ptr = (byte *)_engine->frontVideoBuffer.getPixels();
			for (int y = 0; y < 200; ++y) {
				for (int x = 0; x < 80; ++x) {
					*ptr++ = 0;
				}
				ptr = ptr + 80;
			}
			break;
		}
		case kFlaUnknown9:
		case kFlaUnknown16SameAs9: {
			byte *ptr = (byte *)_engine->frontVideoBuffer.getPixels();
			for (int y = 0; y < 200; ++y) {
				for (int x = 0; x < 80; ++x) {
					*ptr++ = stream.readByte();
				}
				ptr = ptr + 80;
			}
			break;
		}
		case kFlaUnknown4:
		default: {
			break;
		}
		}

		stream.seek(pos + opcodeBlockSize);
	}
}

FlaMovies::FlaMovies(TwinEEngine *engine) : _engine(engine) {}

void FlaMovies::prepareGIF(int index) {
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
	g_system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
	g_system->updateScreen();
	debug(2, "Show gif with id %i from %s", index, Resources::HQR_FLAGIF_FILE);
	delete stream;
	_engine->delaySkip(5000);
	_engine->setPalette(_engine->_screens->paletteRGBA);
}

void FlaMovies::playGIFMovie(const char *flaName) {
	if (!Common::File::exists(Resources::HQR_FLAGIF_FILE)) {
		warning("%s file doesn't exist", Resources::HQR_FLAGIF_FILE);
		return;
	}

	Common::String name(flaName);
	name.toLowercase();

	debug(1, "Play gif %s", name.c_str());
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

void FlaMovies::playFlaMovie(const char *flaName) {
	_engine->_sound->stopSamples();

	Common::String fileNamePath = Common::String::format("%s", flaName);
	const size_t n = fileNamePath.findLastOf(".");
	if (n != Common::String::npos) {
		fileNamePath.erase(n);
	}

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAGIF) {
		playGIFMovie(fileNamePath.c_str());
		return;
	}

	_engine->_music->stopMusic();

	_fadeOut = -1;
	_fadeOutFrames = 0;

	_file.close();
	if (!_file.open(fileNamePath + FLA_EXT)) {
		warning("Failed to open fla movie '%s'", fileNamePath.c_str());
		playGIFMovie(fileNamePath.c_str());
		return;
	}

	_file.read(&_flaHeaderData.version, 6);
	_flaHeaderData.numOfFrames = _file.readUint32LE();
	_flaHeaderData.speed = _file.readByte();
	_flaHeaderData.var1 = _file.readByte();
	debug(2, "Unknown byte in fla file: %i", _flaHeaderData.var1);
	_flaHeaderData.xsize = _file.readUint16LE();
	_flaHeaderData.ysize = _file.readUint16LE();

	_samplesInFla = _file.readUint16LE();
	const uint16 unk2 = _file.readUint16LE();
	debug(2, "Unknown uint16 in fla file: %i", unk2);

	_file.skip(4 * _samplesInFla);

	if (!strcmp((const char *)_flaHeaderData.version, "V1.3")) {
		int32 currentFrame = 0;

		debug("Play fla: %s", flaName);

		ScopedKeyMap scopedKeyMap(_engine, cutsceneKeyMapId);

		_flaPaletteVar = true;
		do {
			FrameMarker frame(_engine, _flaHeaderData.speed);
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			if (currentFrame == _flaHeaderData.numOfFrames) {
				break;
			}
			processFrame();
			scaleFla2x();
			_engine->frontVideoBuffer.blitFrom(_engine->imageBuffer, _engine->imageBuffer.getBounds(), _engine->frontVideoBuffer.getBounds());

			// Only blit to screen if isn't a fade
			if (_fadeOut == -1) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				if (currentFrame == 0) {
					// fade in the first frame
					_engine->_screens->fadeIn(_engine->_screens->paletteRGBACustom);
				} else {
					_engine->setPalette(_engine->_screens->paletteRGBACustom);
				}
			}

			// TRICKY: fade in tricky
			if (_fadeOutFrames >= 2) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				_engine->_screens->fadeToPal(_engine->_screens->paletteRGBACustom);
				_fadeOut = -1;
				_fadeOutFrames = 0;
			}

			currentFrame++;
		} while (!_engine->_input->toggleAbortAction());
	}

	if (_engine->cfgfile.CrossFade) {
		_engine->crossFade(_engine->frontVideoBuffer, _engine->_screens->paletteRGBACustom);
	} else {
		_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
	}

	_engine->_sound->stopSamples();
}

} // namespace TwinE
