/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "sound/vorbis.h"

#ifdef USE_VORBIS

#include "common/file.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"

#include <vorbis/vorbisfile.h>


AudioStream *makeVorbisStream(OggVorbis_File *file, int duration);


#pragma mark -
#pragma mark --- Ogg Vorbis Audio CD emulation ---
#pragma mark -

class VorbisTrackInfo : public DigitalTrackInfo {
private:
	File *_file;
	OggVorbis_File _ov_file;
	bool _error_flag;

public:
	VorbisTrackInfo(File *file);
	~VorbisTrackInfo();
	bool error() { return _error_flag; }
	void play(SoundMixer *mixer, PlayingSoundHandle *handle, int startFrame, int duration);
};


// These are wrapper functions to allow using a File object to
// provide data to the OggVorbis_File object.

struct file_info {
	File *file;
	int start, curr_pos;
	size_t len;
};

static size_t read_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	file_info *f = (file_info *) datasource;
	int result;

	nmemb *= size;
	if (f->curr_pos > (int) f->len)
		nmemb = 0;
	else if (nmemb > f->len - f->curr_pos)
		nmemb = f->len - f->curr_pos;
	result = f->file->read(ptr, nmemb);
	if (result == -1) {
		f->curr_pos = f->file->pos() - f->start;
		return (size_t) -1;
	} else {
		f->curr_pos += result;
		return result / size;
	}
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence) {
	file_info *f = (file_info *) datasource;

	if (whence == SEEK_SET)
		offset += f->start;
	else if (whence == SEEK_END) {
		offset += f->start + f->len;
		whence = SEEK_SET;
	}

	f->file->seek(offset, whence);
	f->curr_pos = f->file->pos() - f->start;
	return f->curr_pos;
}

static int close_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	f->file->close();
	delete f;
	return 0;
}

static long tell_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	return f->curr_pos;
}

static ov_callbacks g_File_wrap = {
	read_wrap, seek_wrap, close_wrap, tell_wrap
};


VorbisTrackInfo::VorbisTrackInfo(File *file) {
	file_info *f = new file_info;

	f->file = file;
	f->start = 0;
	f->len = file->size();
	f->curr_pos = file->pos();

	if (ov_open_callbacks((void *) f, &_ov_file, NULL, 0, g_File_wrap) < 0) {
		warning("Invalid file format");
		_error_flag = true;
		delete f;
		delete file;
	} else {
		_error_flag = false;
		_file = file;
	}
}

#ifdef CHUNKSIZE
#define VORBIS_TREMOR
#endif

void VorbisTrackInfo::play(SoundMixer *mixer, PlayingSoundHandle *handle, int startFrame, int duration) {
#ifdef VORBIS_TREMOR
	ov_time_seek(&_ov_file, (ogg_int64_t)(startFrame / 75.0 * 1000));
#else
	ov_time_seek(&_ov_file, startFrame / 75.0);
#endif

	AudioStream *input = makeVorbisStream(&_ov_file, duration * ov_info(&_ov_file, -1)->rate / 75);
	mixer->playInputStream(handle, input, true);
}

VorbisTrackInfo::~VorbisTrackInfo() {
	if (! _error_flag) {
		ov_clear(&_ov_file);
		delete _file;
	}
}

DigitalTrackInfo *makeVorbisTrackInfo(File *file) {
	return new VorbisTrackInfo(file);
}

#pragma mark -
#pragma mark --- Ogg Vorbis stream ---
#pragma mark -


class VorbisInputStream : public AudioStream {
	OggVorbis_File *_ov_file;
	int _end_pos;
	int _numChannels;
	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;
	
	void refill();
	inline bool eosIntern() const;
public:
	VorbisInputStream(OggVorbis_File *file, int duration);
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read();
	bool endOfData() const		{ return eosIntern(); }
	bool isStereo() const		{ return _numChannels >= 2; }
	
	int getRate() const			{ return ov_info(_ov_file, -1)->rate; }
};


#ifdef CHUNKSIZE
#define VORBIS_TREMOR
#endif


VorbisInputStream::VorbisInputStream(OggVorbis_File *file, int duration) 
	: _ov_file(file), _bufferEnd(_buffer + ARRAYSIZE(_buffer)) {

	// Check the header, determine if this is a stereo stream
	_numChannels = ov_info(_ov_file, -1)->channels;

	// Determine the end position
	if (duration)
		_end_pos = ov_pcm_tell(_ov_file) + duration;
	else
		_end_pos = ov_pcm_total(_ov_file, -1);

	// Read in initial data
	refill();
}

inline int16 VorbisInputStream::read() {
	assert(!eosIntern());

	int16 sample = *_pos++;
	if (_pos >= _bufferEnd) {
		refill();
	}
	return sample;
}

inline bool VorbisInputStream::eosIntern() const {
	return _pos >= _bufferEnd;
}

int VorbisInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void VorbisInputStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0 && _end_pos > ov_pcm_tell(_ov_file)) {
		long result = ov_read(_ov_file, read_pos, len_left,
#ifndef VORBIS_TREMOR
#ifdef SCUMM_BIG_ENDIAN
						1,
#else
						0,
#endif
						2,	// 16 bit
						1,	// signed
#endif
						NULL);
		if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result <= 0) {
			if (result < 0)
				debug(1, "Decode error %d in Vorbis file", result);
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			memset(read_pos, 0, len_left);
			break;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
	_bufferEnd = (int16 *)read_pos;
}

AudioStream *makeVorbisStream(OggVorbis_File *file, int duration) {
	return new VorbisInputStream(file, duration);
}

AudioStream *makeVorbisStream(File *file, uint32 size) {
	OggVorbis_File *ov_file = new OggVorbis_File;
	file_info *f = new file_info;

	f->file = file;
	f->start = file->pos();
	f->len = size;
	f->curr_pos = 0;

	if (ov_open_callbacks((void *) f, ov_file, NULL, 0, g_File_wrap) < 0) {
		warning("Invalid file format");
		delete ov_file;
		delete f;
		return 0;
	} else
		return new VorbisInputStream(ov_file, 0);
}

#endif
