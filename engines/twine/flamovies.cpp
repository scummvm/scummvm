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
#include "twine/grid.h"
#include "twine/input.h"
#include "twine/music.h"
#include "twine/screens.h"
#include "twine/sound.h"
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

	do {
		int8 flag1 = stream.readByte();

		for (int8 a = 0; a < flag1; a++) {
			int8 flag2 = stream.readByte();

			if (flag2 < 0) {
				flag2 = -flag2;
				for (int8 b = 0; b < flag2; b++) {
					*(destPtr++) = stream.readByte();
				}
			} else {
				char colorFill = stream.readByte();

				for (int8 b = 0; b < flag2; b++) {
					*(destPtr++) = colorFill;
				}
			}
		}

		startOfLine = destPtr = startOfLine + width;
	} while (--height);
}

void FlaMovies::drawDeltaFrame(Common::MemoryReadStream &stream, int32 width) {
	int32 a, b;
	uint16 skip;
	uint8 *destPtr;
	uint8 *startOfLine;
	int32 height;

	int8 flag1;
	int8 flag2;

	skip = stream.readUint16LE();
	skip *= width;
	startOfLine = destPtr = (uint8 *)flaBuffer + skip;
	height = stream.readSint16LE();

	do {
		flag1 = stream.readByte();

		for (a = 0; a < flag1; a++) {
			destPtr += stream.readByte();
			flag2 = stream.readByte();

			if (flag2 > 0) {
				for (b = 0; b < flag2; b++) {
					*(destPtr++) = stream.readByte();
				}
			} else {
				char colorFill;
				flag2 = -flag2;

				colorFill = stream.readByte();

				for (b = 0; b < flag2; b++) {
					*(destPtr++) = colorFill;
				}
			}
		}

		startOfLine = destPtr = startOfLine + width;
	} while (--height);
}

void FlaMovies::scaleFla2x() {
	int32 i, j;
	uint8 *source = (uint8 *)flaBuffer;
	uint8 *dest = (uint8 *)_engine->workVideoBuffer.getPixels();

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		for (i = 0; i < SCREEN_WIDTH / SCALE * 40; i++) {
			*(dest++) = 0x00;
		}
	}

	for (i = 0; i < FLASCREEN_HEIGHT; i++) {
		for (j = 0; j < FLASCREEN_WIDTH; j++) {
			*(dest++) = *(source);
			*(dest++) = *(source++);
		}
		if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) { // include wide bars
			memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH * 2);
			dest += FLASCREEN_WIDTH * 2;
		} else { // stretch the movie like original game.
			if (i % (2)) {
				memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
			if (i % 10) {
				memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH * 2);
				dest += FLASCREEN_WIDTH * 2;
			}
		}
	}

	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		for (i = 0; i < SCREEN_WIDTH / SCALE * 40; i++) {
			*(dest++) = 0x00;
		}
	}
}

void FlaMovies::processFrame() {
	FLASampleStruct sample;
	uint32 opcodeBlockSize;
	uint8 opcode;
	int32 aux = 0;

	file.read(&frameData.videoSize, 1);
	file.read(&frameData.dummy, 1);
	file.read(&frameData.frameVar0, 4);
	if (frameData.frameVar0 > _engine->workVideoBuffer.w * _engine->workVideoBuffer.h) {
		warning("Skipping video frame - it would exceed the screen buffer: %i", frameData.frameVar0);
		return;
	}

	uint8 *outBuf = (uint8*)_engine->workVideoBuffer.getPixels();
	file.read(outBuf, frameData.frameVar0);

	if ((int32)frameData.videoSize <= 0) {
		return;
	}

	Common::MemoryReadStream stream(outBuf, frameData.frameVar0);
	do {
		opcode = stream.readByte();
		stream.skip(1);
		opcodeBlockSize = stream.readUint16LE();
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
			_engine->_sound->playFlaSample(sample.sampleNum, sample.freq, sample.repeat, sample.x, sample.y);
			break;
		}
		case kStopSample: {
			_engine->_sound->stopSample(sample.sampleNum);
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(stream, FLASCREEN_WIDTH);
			if (_fadeOut == 1)
				fadeOutFrames++;
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

		aux++;
		stream.seek(pos + opcodeBlockSize);

	} while (aux < (int32)frameData.videoSize);
	//free(workVideoBufferCopy);
}

/** Play FLA PCX Screens
	@param flaName FLA movie name */
static void fla_pcxList(const char *flaName) {
	// TODO if is using FLA PCX than show the images instead
}

FlaMovies::FlaMovies(TwinEEngine *engine) : _engine(engine) {}

void FlaMovies::playFlaMovie(const char *flaName) {
	_engine->_sound->stopSamples();

	// Play FLA PCX instead of movies
	if (_engine->cfgfile.Movie == CONF_MOVIE_FLAPCX) {
		fla_pcxList(flaName);
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
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			if (currentFrame == flaHeaderData.numOfFrames) {
				break;
			}
			processFrame();
			scaleFla2x();
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

			// Only blit to screen if isn't a fade
			if (_fadeOut == -1) {
				_engine->_screens->convertPalToRGBA(_engine->_screens->palette, _engine->_screens->paletteRGBACustom);
				if (!currentFrame) // fade in the first frame
					_engine->_screens->fadeIn(_engine->_screens->paletteRGBACustom);
				else
					_engine->setPalette(_engine->_screens->paletteRGBACustom);
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

			_engine->_system->delayMillis(1000 / flaHeaderData.speed + 1);
		} while (!_engine->_input->toggleAbortAction());
	}

	if (_engine->cfgfile.CrossFade) {
		_engine->crossFade(_engine->frontVideoBuffer, _engine->_screens->paletteRGBACustom);
	} else {
		_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
	}

	_engine->_sound->stopSamples();
}

/*
void fla_pcxList(char *flaName)
{
	// check if FLAPCX file exist
//	if(!checkIfFileExist("FLA_PCX.HQR") || !checkIfFileExist("FLA_GIF.HQR")){
//		printf("FLA_PCX file doesn't exist!");
		//return;
	//}

	// TODO: done this with the HQR 23th entry (movies informations)
	if(!strcmp(flaName,"INTROD"))
	{
		prepareFlaPCX(1);
		WaitTime(5000);
		prepareFlaPCX(2);
		WaitTime(5000);
		prepareFlaPCX(3);
		WaitTime(5000);
		prepareFlaPCX(4);
		WaitTime(5000);
		prepareFlaPCX(5);
		WaitTime(5000);

	}
	else if(!strcmp(flaName,"BAFFE") || !strcmp(flaName,"BAFFE2") || !strcmp(flaName,"BAFFE3") || !strcmp(flaName,"BAFFE4"))
	{
		prepareFlaPCX(6);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"bateau") || !strcmp(flaName,"bateau2"))
	{
		prepareFlaPCX(7);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"flute2"))
	{
		prepareFlaPCX(8);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"navette"))
	{
		prepareFlaPCX(15);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"templebu"))
	{
		prepareFlaPCX(12);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"glass2"))
	{
		prepareFlaPCX(8);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"surf"))
	{
		prepareFlaPCX(9);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"verser") || !strcmp(flaName,"verser2"))
	{
		prepareFlaPCX(10);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"capture"))
	{
		prepareFlaPCX(14);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"neige2"))
	{
		prepareFlaPCX(11);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"sendel"))
	{
		prepareFlaPCX(14);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"sendel2"))
	{
		prepareFlaPCX(17);
		WaitTime(5000);
	}
}

void prepareFlaPCX(int index)
{
	int i;
	SDL_Surface *image;

	// TODO: Done this without SDL_Image Library
	if(checkIfFileExist("FLA_PCX.HQR"))
		image = IMG_LoadPCX_RW(SDL_RWFromMem(HQR_Get(HQR_FlaPCX,index), Size_HQR("FLA_PCX.HQR", index))); // rwop
	else if(checkIfFileExist("FLA_GIF.HQR"))
		image = IMG_LoadGIF_RW(SDL_RWFromMem(HQR_Get(HQR_FlaGIF,index), Size_HQR("fla_gif.hqr", index))); // rwop

	if(!image) {
		printf("Can't load FLA PCX: %s\n", IMG_GetError());
	}

	osystem_FlaPCXCrossFade(image);
}*/

} // namespace TwinE
