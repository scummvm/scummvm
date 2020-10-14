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
#include "twine/keyboard.h"
#include "twine/music.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

/** Config movie types */
#define CONF_MOVIE_NONE 0
#define CONF_MOVIE_FLA 1
#define CONF_MOVIE_FLAWIDE 2
#define CONF_MOVIE_FLAPCX 3

/** FLA movie extension */
#define FLA_EXT ".fla"

void FlaMovies::drawKeyFrame(uint8 *ptr, int32 width, int32 height) {
	uint8 *destPtr = (uint8 *)flaBuffer;
	uint8 *startOfLine = destPtr;

	do {
		int8 flag1 = *(ptr++);

		for (int8 a = 0; a < flag1; a++) {
			int8 flag2 = *(ptr++);

			if (flag2 < 0) {
				flag2 = -flag2;
				for (int8 b = 0; b < flag2; b++) {
					*(destPtr++) = *(ptr++);
				}
			} else {
				char colorFill = *(ptr++);

				for (int8 b = 0; b < flag2; b++) {
					*(destPtr++) = colorFill;
				}
			}
		}

		startOfLine = destPtr = startOfLine + width;
	} while (--height);
}

void FlaMovies::drawDeltaFrame(uint8 *ptr, int32 width) {
	int32 a, b;
	uint16 skip;
	uint8 *destPtr;
	uint8 *startOfLine;
	int32 height;

	int8 flag1;
	int8 flag2;

	skip = *((uint16 *)ptr);
	ptr += 2;
	skip *= width;
	startOfLine = destPtr = (uint8 *)flaBuffer + skip;
	height = *((int16 *)ptr);
	ptr += 2;

	do {
		flag1 = *(ptr++);

		for (a = 0; a < flag1; a++) {
			destPtr += (unsigned char)*(ptr++);
			flag2 = *(ptr++);

			if (flag2 > 0) {
				for (b = 0; b < flag2; b++) {
					*(destPtr++) = *(ptr++);
				}
			} else {
				char colorFill;
				flag2 = -flag2;

				colorFill = *(ptr++);

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
	uint8 *dest = (uint8 *)_engine->workVideoBuffer;

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
	uint8 *ptr;

	file.read(&frameData.videoSize, 1);
	file.read(&frameData.dummy, 1);
	file.read(&frameData.frameVar0, 4);

	file.read(workVideoBufferCopy, frameData.frameVar0);

	if ((int32)frameData.videoSize <= 0)
		return;

	ptr = workVideoBufferCopy;

	do {
		opcode = *((uint8 *)ptr);
		ptr += 2;
		opcodeBlockSize = *((uint16 *)ptr);
		ptr += 2;

		switch (opcode - 1) {
		case kLoadPalette: {
			int16 numOfColor = *((int16 *)ptr);
			int16 startColor = *((int16 *)(ptr + 2));
			memcpy((_engine->_screens->palette + (startColor * 3)), (ptr + 4), numOfColor * 3);
			break;
		}
		case kFade: {
			// FLA movies don't use cross fade
			// fade out tricky
			if (_fadeOut != 1) {
				_engine->_screens->copyPal(_engine->_screens->palette, _engine->_screens->paletteRGBCustom);
				_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBCustom);
				_fadeOut = 1;
			}
			break;
		}
		case kPlaySample: {
			memcpy(&sample, ptr, sizeof(FLASampleStruct));
			_engine->_sound->playFlaSample(sample.sampleNum, sample.freq, sample.repeat, sample.x, sample.y);
			break;
		}
		case kStopSample: {
			_engine->_sound->stopSample(sample.sampleNum);
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(ptr, FLASCREEN_WIDTH);
			if (_fadeOut == 1)
				fadeOutFrames++;
			break;
		}
		case kKeyFrame: {
			drawKeyFrame(ptr, FLASCREEN_WIDTH, FLASCREEN_HEIGHT);
			break;
		}
		default: {
			return;
		}
		}

		aux++;
		ptr += opcodeBlockSize;

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

	Common::String fileNamePath = Common::String::format(FLA_DIR "%s", flaName);
	const size_t n = fileNamePath.findLastOf(".");
	if (n != Common::String::npos) {
		fileNamePath.erase(n);
	}
	fileNamePath += FLA_EXT;

	_fadeOut = -1;
	fadeOutFrames = 0;

	if (!file.open(fileNamePath)) {
		warning("Failed to open fla movie '%s'", fileNamePath.c_str());
		return;
	}

	workVideoBufferCopy = _engine->workVideoBuffer;

	file.read(&flaHeaderData.version, 6);
	flaHeaderData.numOfFrames = file.readUint32LE();
	flaHeaderData.speed = file.readByte();
	flaHeaderData.var1 = file.readByte();
	flaHeaderData.xsize = file.readUint16LE();
	flaHeaderData.ysize = file.readUint16LE();

	samplesInFla = file.readUint16LE();
	file.skip(2);

	for (int32 i = 0; i < samplesInFla; i++) {
		flaSampleTable[i] = file.readSint16LE();
		file.skip(2);
	}

	if (!strcmp((const char *)flaHeaderData.version, "V1.3")) {
		int32 currentFrame = 0;

		do {
			if (currentFrame == flaHeaderData.numOfFrames) {
				break;
			}
			processFrame();
			scaleFla2x();
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

			// Only blit to screen if isn't a fade
			if (_fadeOut == -1) {
				_engine->_screens->copyPal(_engine->_screens->palette, _engine->_screens->paletteRGBCustom);
				if (!currentFrame) // fade in the first frame
					_engine->_screens->fadeIn(_engine->_screens->paletteRGBCustom);
				else
					_engine->setPalette(_engine->_screens->paletteRGBCustom);
			}

			// TRICKY: fade in tricky
			if (fadeOutFrames >= 2) {
				_engine->flip();
				_engine->_screens->copyPal(_engine->_screens->palette, _engine->_screens->paletteRGBCustom);
				_engine->_screens->fadeToPal(_engine->_screens->paletteRGBCustom);
				_fadeOut = -1;
				fadeOutFrames = 0;
			}

			currentFrame++;

			_engine->_system->delayMillis(1000 / flaHeaderData.speed + 1);

			_engine->readKeys();

			if (_engine->shouldQuit()) {
				break;
			}

			if (_engine->_keyboard.skipIntro) {
				break;
			}
		} while (true);
	}

	if (_engine->cfgfile.CrossFade) {
		_engine->crossFade(_engine->frontVideoBuffer, _engine->_screens->paletteRGBCustom);
	} else {
		_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBCustom);
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
