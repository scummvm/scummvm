/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "stdafx.h"

#include "sound/vorbis.h"
#include "common/file.h"
#include "common/util.h"

#ifdef USE_VORBIS
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
#endif

#ifdef USE_VORBIS

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

int VorbisTrackInfo::play(SoundMixer *mixer, PlayingSoundHandle *handle, int startFrame, int duration) {
#ifdef VORBIS_TREMOR
	ov_time_seek(&_ov_file, (ogg_int64_t)(startFrame / 75.0 * 1000));
#else
	ov_time_seek(&_ov_file, startFrame / 75.0);
#endif
	return mixer->playVorbis(handle, &_ov_file,
							duration * ov_info(&_ov_file, -1)->rate / 75, true);
}

VorbisTrackInfo::~VorbisTrackInfo() {
	if (! _error_flag) {
		ov_clear(&_ov_file);
		delete _file;
	}
}

void playSfxSound_Vorbis(SoundMixer *mixer, File *file, uint32 size, PlayingSoundHandle *handle) {
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
	} else
		mixer->playVorbis(handle, ov_file, 0, false);
}

#endif
