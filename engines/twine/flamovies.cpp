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
	kLoadPalette = 0,
	kFade = 1,
	kPlaySample = 2,
	kStopSample = 4,
	kDeltaFrame = 5,
	kKeyFrame = 7
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
	uint8 *destPtr = (uint8 *)flaBuffer;
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

	uint8 *destPtr = (uint8 *)flaBuffer + skip;
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
	uint8 *source = (uint8 *)flaBuffer;
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

	file.read(&frameData.videoSize, 1);
	file.read(&frameData.dummy, 1);
	file.read(&frameData.frameVar0, 4);
	if (frameData.frameVar0 > _engine->imageBuffer.w * _engine->imageBuffer.h) {
		warning("Skipping video frame - it would exceed the screen buffer: %i", frameData.frameVar0);
		return;
	}

	uint8 *outBuf = (uint8 *)_engine->imageBuffer.getPixels();
	file.read(outBuf, frameData.frameVar0);

	if ((int32)frameData.videoSize <= 0) {
		return;
	}

	Common::MemoryReadStream stream(outBuf, frameData.frameVar0);
	for (int32 frame = 0; frame < frameData.videoSize; ++frame) {
		const uint8 opcode = stream.readByte();
		stream.skip(1);
		const uint32 opcodeBlockSize = stream.readUint16LE();
		const int32 pos = stream.pos();

		switch (opcode - 1) {
		case kLoadPalette: {
			int16 numOfColor = stream.readSint16LE();
			int16 startColor = stream.readSint16LE();
			uint8 *dest = _engine->_screens->palette + (startColor * 3);
			stream.read(dest, numOfColor * 3);
			break;
		}
		case kFade: {
			// FLA movies don't use cross fade
			// fade out tricky
			if (_fadeOut != 1) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
				_fadeOut = 1;
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
				++fadeOutFrames;
			}
			break;
		}
		case kKeyFrame: {
			drawKeyFrame(stream, FLASCREEN_WIDTH, FLASCREEN_HEIGHT);
			break;
		}
		default: {
			break;
		}
		}

		stream.seek(pos + opcodeBlockSize);
	}
}

FlaMovies::FlaMovies(TwinEEngine *engine) : _engine(engine) {}

void FlaMovies::prepareGIF(int index) {
#if 0
	Image::GIFDecoder decoder;
	Common::SeekableReadStream *stream = HQR::makeReadStream("FLA_GIF.HQR", index);
	if (stream == nullptr) {
		warning("Failed to load gif hqr entry with id %i from FLA_GIF.HQR", index);
		return;
	}
	if (!decoder.loadStream(*stream)) {
		delete stream;
		warning("Failed to load gif with id %i from FLA_GIF.HQR", index);
		return;
	}
	const Graphics::Surface *surface = decoder.getSurface();
	const bool state = Graphics::crossBlit((uint8*)_engine->imageBuffer.getPixels(), (const uint8*)surface->getPixels(), _engine->imageBuffer.pitch, surface->pitch, surface->w, surface->h, _engine->imageBuffer.format, surface->format);
	if (!state) {
		error("Failed to blit");
	}
	_engine->frontVideoBuffer.transBlitFrom(_engine->imageBuffer, _engine->imageBuffer.getBounds(), _engine->frontVideoBuffer.getBounds());
	debug(2, "Show gif with id %i from FLA_GIF.HQR", index);
	_engine->flip();
	delete stream;
	g_system->delayMillis(5000);
#endif
}

void FlaMovies::playGIFMovie(const char *flaName) {
	if (!Common::File::exists("FLA_GIF.HQR")) {
		warning("FLA_GIF file doesn't exist!");
		return;
	}

	debug("Play gif %s", flaName);
	// TODO: use the HQR 23th entry (movies informations)
	if (!strcmp(flaName, FLA_INTROD)) {
		prepareGIF(3);
		prepareGIF(4);
		prepareGIF(5);
	} else if (!strcmp(flaName, "BAFFE") || !strcmp(flaName, "BAFFE2") || !strcmp(flaName, "BAFFE3") || !strcmp(flaName, "BAFFE4")) {
		prepareGIF(6);
	} else if (!strcmp(flaName, "bateau") || !strcmp(flaName, "bateau2")) {
		prepareGIF(7);
	} else if (!strcmp(flaName, "navette")) {
		prepareGIF(15);
	} else if (!strcmp(flaName, "templebu")) {
		prepareGIF(12);
	} else if (!strcmp(flaName, "flute2")) {
		prepareGIF(8); // TODO: same as glass2?
	} else if (!strcmp(flaName, "glass2")) {
		prepareGIF(8); // TODO: same as flute2?
	} else if (!strcmp(flaName, "surf")) {
		prepareGIF(9);
	} else if (!strcmp(flaName, "verser") || !strcmp(flaName, "verser2")) {
		prepareGIF(10);
	} else if (!strcmp(flaName, "neige2")) {
		prepareGIF(11);
	} else if (!strcmp(flaName, "capture")) {
		prepareGIF(14); // TODO: same as sendel?
	} else if (!strcmp(flaName, "sendel")) {
		prepareGIF(14); // TODO: same as capture?
	} else if (!strcmp(flaName, "sendel2")) {
		prepareGIF(17);
	} else if (!strcmp(flaName, FLA_DRAGON3)) {
		prepareGIF(1);
		prepareGIF(2);
	}
}

void FlaMovies::playFlaMovie(const char *flaName) {
	_engine->_sound->stopSamples();

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAGIF) {
		playGIFMovie(flaName);
		return;
	}

	_engine->_music->stopMusic();

	Common::String fileNamePath = Common::String::format("%s", flaName);
	const size_t n = fileNamePath.findLastOf(".");
	if (n != Common::String::npos) {
		fileNamePath.erase(n);
	}
	fileNamePath += FLA_EXT;

	_fadeOut = -1;
	fadeOutFrames = 0;

	file.close();
	if (!file.open(fileNamePath)) {
		warning("Failed to open fla movie '%s'", fileNamePath.c_str());
		return;
	}

	file.read(&flaHeaderData.version, 6);
	flaHeaderData.numOfFrames = file.readUint32LE();
	flaHeaderData.speed = file.readByte();
	flaHeaderData.var1 = file.readByte();
	flaHeaderData.xsize = file.readUint16LE();
	flaHeaderData.ysize = file.readUint16LE();

	samplesInFla = file.readUint16LE();
	file.skip(2);

	file.skip(4 * samplesInFla);

	if (!strcmp((const char *)flaHeaderData.version, "V1.3")) {
		int32 currentFrame = 0;

		debug("Play fla: %s", flaName);

		ScopedKeyMap scopedKeyMap(_engine, cutsceneKeyMapId);

		do {
			ScopedFPS scopedFps(flaHeaderData.speed);
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			if (currentFrame == flaHeaderData.numOfFrames) {
				break;
			}
			processFrame();
			scaleFla2x();
			_engine->frontVideoBuffer.transBlitFrom(_engine->imageBuffer, _engine->imageBuffer.getBounds(), _engine->frontVideoBuffer.getBounds());

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
			if (fadeOutFrames >= 2) {
				_engine->flip();
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				_engine->_screens->fadeToPal(_engine->_screens->paletteRGBACustom);
				_fadeOut = -1;
				fadeOutFrames = 0;
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
