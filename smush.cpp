// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include <cstring>
#include "smush.h"
#include "timer.h"
#include "mixer/mixer.h"
#include "driver_gl.h"

#include "resource.h"

#include <zlib.h>

Smush *smush;

void hexdump(const byte * data, int len, int bytesPerLine) {
        assert(1 <= bytesPerLine && bytesPerLine <= 32);
        int i;
        byte c;
        int offset = 0;
        while (len >= bytesPerLine) {
                printf("%06x: ", offset);
                for (i = 0; i < bytesPerLine; i++) {
                        printf("%02x ", data[i]);
                        if (i % 4 == 3)
                                printf(" ");
                }
                printf(" |");
                for (i = 0; i < bytesPerLine; i++) {
                        c = data[i];
                        if (c < 32 || c >= 127)
                                c = '.';
                        printf("%c", c);
                }
                printf("|\n");
                data += bytesPerLine;
                len -= bytesPerLine;
                offset += bytesPerLine;
        }

        if (len <= 0)
                return;

        printf("%06x: ", offset);
        for (i = 0; i < bytesPerLine; i++) {
                if (i < len)
                        printf("%02x ", data[i]);
                else
                        printf("   ");
                if (i % 4 == 3)
                        printf(" ");
        }
        printf(" |");
        for (i = 0; i < len; i++) {
                c = data[i];
                if (c < 32 || c >= 127)
                        c = '.';
                printf("%c", c);
        }
        for (; i < bytesPerLine; i++)
                printf(" ");
        printf("|\n");
}


void Smush::timerCallback(void *refCon) {
	smush->handleFrame();
}

Smush::Smush() {
	smush = this;
	_nbframes = 0;
	_dst = NULL;
	_width = 0;
	_height = 0;
	_speed = 0;
	_channels = -1;
	_freq = 22050;
}

Smush::~Smush() {
	deinit();
}

void Smush::init() {
	_frame = 0;
	_alreadyInit = false;
	_videoFinished = false;
	g_timer->installTimerProc(&timerCallback, _speed, NULL);
}

void Smush::deinit() {
	g_timer->removeTimerProc(&timerCallback);
}

void Smush::handleBlocky16(byte *src) {
	if (!_alreadyInit) {
		_blocky16.init(_width, _height);
		_alreadyInit = true;
	}
	_blocky16.decode(_dst, src);
}

void decompressVima(const char *src, int16 *dest, int destLen);
void vimaInit();

extern SoundMixer *g_mixer;

void Smush::handleWave(const byte *src, uint32 size) {
	int16 *dst = new int16[size * _channels];
	decompressVima((char *)src, dst, size * _channels * 2);

	for (uint32 j = 0; j < size * _channels; j++)
		dst[j] = SWAP_BYTES_16(dst[j]);
 
	int flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE;
//	int flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_LITTLE_ENDIAN;
	if (_channels == 2)
		flags |= SoundMixer::FLAG_STEREO;

	if (!_soundHandle.isActive())
		g_mixer->newStream(&_soundHandle, _freq, flags, 500000);
	g_mixer->appendStream(_soundHandle, (byte *)dst, size * _channels * 2);
}

void Smush::handleFrame() {
	uint32 tag;
	int32 size;
	int pos = 0;

	tag = _file.readUint32BE();
	if (tag == MKID_BE('ANNO')) {
		size = _file.readUint32BE();
		_file.seek(size, SEEK_CUR);
		tag = _file.readUint32BE();
	}
	assert(tag == MKID_BE('FRME'));
	size = _file.readUint32BE();
	byte *frame = (byte*)malloc(size);
	_file.read(frame, size);
	do {
		if (READ_BE_UINT32(frame + pos) == MKID_BE('Bl16')) {
			handleBlocky16(frame + pos + 8);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (READ_BE_UINT32(frame + pos) == MKID_BE('Wave')) {
			handleWave(frame + pos + 8 + 4, READ_BE_UINT32(frame + pos + 8));
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else {
			error("unknown tag");
		}
	} while (pos < size);
	free(frame);
	_frame++;
	if (_frame == _nbframes) {
		_videoFinished = true;
	}

	g_driver->drawSMUSHframe(_width, _height, _dst);
}

void Smush::handleFramesHeader() {
	uint32 tag;
	int32 size;
	int pos = 0;

	tag = _file.readUint32BE();
	assert(tag == MKID_BE('FLHD'));
	size = _file.readUint32BE();
	byte *f_header = (byte*)malloc(size);
	_file.read(f_header, size);
	do {
		if (READ_BE_UINT32(f_header + pos) == MKID_BE('Bl16')) {
			pos += READ_BE_UINT32(f_header + pos + 4) + 8;
		} else if (READ_BE_UINT32(f_header + pos) == MKID_BE('Wave')) {
			_freq = READ_LE_UINT32(f_header + pos + 8);
			_channels = READ_LE_UINT32(f_header + pos + 12);
			vimaInit();
			pos += 20;
		} else {
			error("unknown tag");
		}
	} while (pos < size);
	free(f_header);
}

void Smush::setupAnim(const char *file, const char *directory) {
	_file.open(file, directory);
	uint32 tag;
	int32 size;
	
	tag = _file.readUint32BE();
	assert(tag == MKID_BE('SANM'));
	size = _file.readUint32BE();
	tag = _file.readUint32BE();
	assert(tag == MKID_BE('SHDR'));
	size = _file.readUint32BE();
	byte *s_header = (byte *)malloc(size);
	_file.read(s_header, size);
	_nbframes = READ_LE_UINT32(s_header + 2);
	_width = READ_LE_UINT16(s_header + 8);
	_height = READ_LE_UINT16(s_header + 10);
	if ((_width != 640) || (_height != 480))
		error("resolution of smush frame other than 640x480 not supported");
	_speed = READ_LE_UINT32(s_header + 14);
	free(s_header);
}

void Smush::play(const char *filename, const char *directory) {
	char tmpOut[256];
	FILE *tmp = ResourceLoader::instance()->openNewStream(filename);
	FILE *outFile = NULL;
	sprintf(tmpOut, "smush.temp");
	
	if (tmp != NULL) {
		z_stream z;
		char inBuf[1024], outBuf[1024], flags;
		int status = 0;

		warning("Decompressing SMUSH cutscene %s - This may take a minute", filename);
		fread(inBuf, 4, sizeof(byte), tmp);		//	Header, Method, Flags
		flags = inBuf[3];
		fread(inBuf, 6, sizeof(byte), tmp);		// 	XFlags

		if (((flags & 0x04) != 0) || ((flags & 0x10) != 0))	// Misc
			error("Unsupported header flag");

		if ((flags & 0x08) != 0) {				// Name
			do {
				fread(inBuf, 1, sizeof(byte), tmp);
				printf("%c", inBuf[0]);
			} while(inBuf[0] != 0);

		}

		if ((flags & 0x02) != 0)				// CRC
			fread(inBuf, 2, sizeof(byte), tmp);

		z.zalloc = NULL;
		z.zfree = NULL;
		z.opaque = Z_NULL;

		if (inflateInit2(&z, -15) != Z_OK)
			error("Smush::play() - inflateInit error");

		z.next_in = (Bytef *)inBuf;
		z.avail_in = (uInt)fread(inBuf, 1, sizeof(inBuf), tmp);
		z.next_out = (Bytef *)outBuf;
		z.avail_out = sizeof(outBuf);

		for (;;) {
			if (z.avail_in == 0) {
				z.next_in = (Bytef *)inBuf;
				z.avail_in = (uInt)fread(inBuf, 1, sizeof(inBuf), tmp);
			}

			status = inflate(&z, Z_NO_FLUSH);
			if (status == Z_STREAM_END) {
				if (sizeof(outBuf) - z.avail_out) {
					if (outFile == NULL)
						outFile = fopen(tmpOut, "wb");
					fwrite(outBuf, 1, sizeof(outBuf) - z.avail_out, outFile);
				}
				break;
			}

			if (status != Z_OK) {
				warning("Smush::play() - Error inflating stream (%d) [-3 means bad data]", status);
				return;
			}

			if (z.avail_out == 0) {
				if (outFile == NULL)
					outFile = fopen(tmpOut, "wb");

				fwrite(outBuf, 1, sizeof(outBuf), outFile);
				z.next_out = (Bytef *)outBuf;
				z.avail_out = sizeof(outBuf);
			}
		}

		inflateEnd(&z);
		fclose(outFile);
		warning("Smush::play() Open okay for %s!\n", filename);
	}

	// Verify the specified file exists
	File f;
	f.open(tmpOut, NULL);
	if (!f.isOpen()) {
		warning("Smush::play() File not found %s", filename);
		return;
	}
	f.close();

	// Load the video
	setupAnim(tmpOut, directory);
	handleFramesHeader();

	SDL_Surface* image;
	image = SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000);

	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = image->w;
	src.h = image->h;

	_dst = (byte *)image->pixels;

	init();

	while (!_videoFinished) {
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Skip cutscene?
			if (event.type == SDL_KEYDOWN)
				if (event.key.keysym.sym == SDLK_ESCAPE)
					_videoFinished = true;
		}
		SDL_Delay(10);
	};
	deinit();
}

FILE *File::fopenNoCase(const char *filename, const char *directory, const char *mode) {
	FILE *file;
	char buf[512];
	char *ptr;

	assert(directory);
	strcpy(buf, directory);

#ifdef WIN32
	// Fix for Win98 issue related with game directory pointing to root drive ex. "c:\"
	if ((buf[0] != 0) && (buf[1] == ':') && (buf[2] == '\\') && (buf[3] == 0)) {
		buf[2] = 0;
	}
#endif

	// Record the length of the dir name (so we can cut of anything trailing it
	// later, when we try with different file names).
	const int dirLen = (int)strlen(buf);

	if (dirLen > 0) {
		strcat(buf, "/");	// prevent double /
	}
	strcat(buf, filename);

	file = fopen(buf, mode);
	if (file)
		return file;

	const char *dirs[] = {
		"",
		"video/",
		"VIDEO/",
	};

	for (int dirIdx = 0; dirIdx < ARRAYSIZE(dirs); dirIdx++) {
		buf[dirLen] = 0;
		strcat(buf, dirs[dirIdx]);
		int len = (int)strlen(buf);
		strcat(buf, filename);

		ptr = buf + len;
		do
			*ptr = toupper(*ptr);
		while (*ptr++);
		file = fopen(buf, mode);
		if (file)
			return file;

		ptr = buf + len;
		do
			*ptr = tolower(*ptr);
		while (*ptr++);
		file = fopen(buf, mode);
		if (file)
			return file;
	}

	return NULL;
}

File::File() {
	_handle = NULL;
	_ioFailed = false;
	_encbyte = 0;
	_name = 0;
}

File::~File() {
	close();
	delete [] _name;
}

bool File::open(const char *filename, const char *directory, int mode, byte encbyte) {
	if (_handle) {
		warning("File %s already opened", filename);
		return false;
	}

	if (filename == NULL || *filename == 0)
		return false;
	
	// If no directory was specified, use the default directory (if any).
	if (directory == NULL)
		directory = "";

	clearIOFailed();

	if (mode == kFileReadMode) {
		_handle = fopenNoCase(filename, directory, "rb");
		if (_handle == NULL) {
			warning("File %s not found", filename);
			return false;
		}
	}	else {
		warning("Only read/write mode supported!");
		return false;
	}

	_encbyte = encbyte;

	int len = (int)strlen(filename);
	if (_name != 0)
		delete [] _name;
	_name = new char[len+1];
	memcpy(_name, filename, len+1);

	return true;
}

void File::close() {
	if (_handle)
		fclose(_handle);
	_handle = NULL;
}

bool File::isOpen() {
	return _handle != NULL;
}

bool File::ioFailed() {
	return _ioFailed != 0;
}

void File::clearIOFailed() {
	_ioFailed = false;
}

bool File::eof() {
	if (_handle == NULL) {
		error("File is not open!");
		return false;
	}

	return feof(_handle) != 0;
}

uint32 File::pos() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	return ftell(_handle);
}

uint32 File::size() {
	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	uint32 oldPos = ftell(_handle);
	fseek(_handle, 0, SEEK_END);
	uint32 length = ftell(_handle);
	fseek(_handle, oldPos, SEEK_SET);

	return length;
}

void File::seek(int32 offs, int whence) {
	if (_handle == NULL) {
		error("File is not open!");
		return;
	}

	if (fseek(_handle, offs, whence) != 0)
		clearerr(_handle);
}

uint32 File::read(void *ptr, uint32 len) {
	byte *ptr2 = (byte *)ptr;
	uint32 real_len;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	real_len = (uint32)fread(ptr2, 1, len, _handle);
	if (real_len < len) {
		clearerr(_handle);
		_ioFailed = true;
	}

	if (_encbyte != 0) {
		uint32 t_size = real_len;
		do {
			*ptr2++ ^= _encbyte;
		} while (--t_size);
	}

	return real_len;
}

byte File::readByte() {
	byte b;

	if (_handle == NULL) {
		error("File is not open!");
		return 0;
	}

	if (fread(&b, 1, 1, _handle) != 1) {
		clearerr(_handle);
		_ioFailed = true;
	}
	return b ^ _encbyte;
}

uint16 File::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 File::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 File::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 File::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}
