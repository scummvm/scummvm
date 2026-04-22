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

#include "common/system.h"
#include "common/file.h"
#include "common/savefile.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/fileio.h"

namespace MADS {
namespace MADSV2 {

#define SLOW_BUFFER_SIZE        256

bool fileio_suppress_unbuffering = false;

void fileio_purge_trailing_spaces(char *myline) {
	char *search;
	int again;

	search = &myline[strlen(myline) - 1];

	do {
		again = true;
		search = &myline[strlen(myline) - 1];
		if ((*search == 0x20) || (*search == 0x09)) {
			*search = 0;
		} else {
			again = false;
		}

		search--;
		if (search < myline) again = false;
	} while (again);
}

char *fileio_ffgets(char *mystring, int num, Common::SeekableReadStream *stream) {
	char *temp;
	char *from;
	char *unto;
	char workchar;
	char ffbuf[256];
	int worklen;
	int tabstop;

	if (stream->eos()) {
		temp = nullptr;
	} else {
		Common::String line = stream->readLine();
		Common::strcpy_s(ffbuf, line.c_str());
		temp = ffbuf;
	}

	if (temp != NULL) {
		worklen = 0;
		from = ffbuf;
		unto = mystring;
		workchar = true;

		while (workchar != 0) {
			if (*from != 0x09) {
				if (worklen <= num) {
					workchar = *(unto++) = *(from++);
					worklen++;
				} else {
					workchar = 0;
					mystring = NULL;
				}
			} else {
				from++;
				tabstop = (((worklen >> 3) + 1) << 3);
				if (tabstop < num) {
					while (worklen < tabstop) {
						*(unto++) = 0x20;
						worklen++;
					}
				} else {
					workchar = 0;
					mystring = NULL;
				}
			}
		}
		return (mystring);
	} else {
		return (NULL);
	}
}

int fileio_ffputs(const char *mystring, Common::WriteStream *stream) {
	char ffbuf[256];

	Common::strcpy_s(ffbuf, mystring);
	stream->writeString(mystring);
	return 0;
}

char *fileio_fix_lf_input(char *mystring) {
	char *temp;

	temp = strrchr(mystring, 0x0a);
	if (temp != NULL) {
		*temp = '\0';
	}

	return temp;
}

void fileio_fix_lf_output(char *mystring) {
	char *temp;

	temp = mystring + strlen(mystring);
	*(temp++) = 0x0a;
	*temp = '\0';
}

int fileio_copy(const char *source, const char *dest) {
	Common::File src;
	Common::OutSaveFile *dst;

	if (src.open(source)) {
		dst = g_system->getSavefileManager()->openForSaving(dest, false);
		if (dst != nullptr) {
			dst->writeStream(&src);
			return 0;
		}
	}

	return -1;
}

long fileio_setpos(Common::Stream *handle, long pos) {
	auto *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	auto *ws = dynamic_cast<Common::SeekableWriteStream *>(handle);

	if (rs)
		return rs->seek(pos) ? pos : -1;
	if (ws)
		return ws->seek(pos) ? pos : -1;

	return -1;
}

long fileio_fread_f(void *target, long record_size, long record_count, Common::SeekableReadStream *handle) {
	if (!record_size)
		return 0;

	const long total_size = record_size * record_count;
	const long total_read = handle->read(target, total_size);

	return total_read / record_size;
}

long fileio_fwrite_f(const void *source, long record_size, long record_count, Common::WriteStream *handle) {
	if (!record_size)
		return 0;

	const long total_size = record_size * record_count;
	const long total_written = handle->write(source, total_size);

	return total_written / record_size;
}

long fileio_file_to_file(Common::SeekableReadStream *source, Common::WriteStream *dest, long amount) {
	return dest->writeStream(source, amount);
}

long fileio_get_file_size(const char *filename) {
	Common::File f;
	if (Common::File::exists(filename) && f.open(filename))
		return f.size();

	Common::InSaveFile *sf = g_system->getSavefileManager()->openForLoading(filename);
	long result = sf ? sf->size() : -1;
	delete sf;

	return result;
}

bool fileio_exist(const char *inp) {
	return Common::File::exists(inp);
}

char *fileio_get_filetype(char *outp, char *inp) {
	Common::File fcb;
	char test[20];
	int  a;

	Common::strcpy_s(outp, 65536, "");
	if (fileio_exist(inp) && fcb.open(inp) && fcb.read(test, 8) == 8) {
		if ((test[0] == 'D') && (test[1] == '4') && (test[2] == 'A'))
			Common::strcpy_s(outp, 4, "D4A");
		else {
			a = 0;
			while ((a < 11) && (test[a] != 26) && (test[a] != 0)) {
				outp[a] = test[a];
				outp[a + 1] = 0;
				a++;
			}
		}

		fcb.close();
		return outp;
	}

	return nullptr;
}

unsigned long fileio_get_file_time(const char *filename) {
	error("TODO: fileio_get_file_time");
}

char *fileio_read_header(char *target, Common::SeekableReadStream *handle) {
	char *mark = target;
	int next = 1;
	int count = 0;

	while (next && (count < 79)) {
		next = handle->readByte();
		*mark = (byte)next;
		mark++;
		count++;
	}
	(void)handle->readByte();  // Throw away EOF mark

	return target;
}


void fileio_write_header(char *text, Common::WriteStream *handle) {
	int next = 1;

	while (next) {
		next = *text;
		text++;
		handle->writeByte(next);
	}
	handle->writeByte(26);  // Write fake EOF mark
}

char *fileio_get_line(char *target, Common::SeekableReadStream *handle) {
	bool going = true;
	byte item;

	while (going) {
		if (handle->eos()) {
			*target = 0;
			target = NULL;
			going = false;
		} else {
			item = handle->readByte();

			if (item == 0x1a) {
				*target = 0;
				target = NULL;
				going = false;
			} else if (item == 0x0a) {
				*target = 0;
				going = false;
			} else {
				*(target++) = item;
			}
		}
	}

	return target;
}

int fileio_put_line(const char *source, Common::WriteStream *handle) {
	handle->writeString(source);
	handle->writeByte('\n');

	return 0;
}

long fileio_get_disk_free(char drive) {
	return 999999;
}

void fileio_add_ext(char *name, const char *ext) {
	char *mark;

	mark = strrchr(name, '.');
	if (mark == NULL) {
		Common::strcat_s(name, 65536, ".");
		Common::strcat_s(name, 65536, ext);
	}

	mads_strupr(name);
}

void fileio_new_ext(char *target, const char *name, const char *ext) {
	char *mark;

	if (name != target) {
		Common::strcpy_s(target, 65536, name);
	}

	mark = strrchr(target, '.');
	if (mark != NULL) *mark = 0;
	Common::strcat_s(target, 65536, ".");
	Common::strcat_s(target, 65536, ext);

	mads_strupr(target);
}

int fileio_logpath(const char *path) {
	// No implementation in ScummVM
	return 0;
}

char *fileio_parse_filename(char *target, char *filepath) {
	char temp_buf[80];
	char *mark;

	Common::strcpy_s(temp_buf, filepath);

	mark = strrchr(temp_buf, '\\');
	if (mark != NULL) {
		mark++;
	} else {
		mark = temp_buf;
	}

	Common::strcpy_s(target, 65536, mark);
	mads_strupr(target);

	return target;
}

char *fileio_parse_path(char *target, const char *filepath) {
	char temp_buf[80];
	char *mark;

	Common::strcpy_s(temp_buf, filepath);

	mark = strrchr(temp_buf, '\\');
	if (mark != NULL) {
		if (*(mark + 1)) {
			*mark = 0;
		}
		Common::strcpy_s(target, 65536, temp_buf);
		*mark = '\\';
	} else {
		Common::strcpy_s(target, 65536, temp_buf);
	}

	mads_strupr(target);

	return target;
}

char *fileio_swap_path(char *target, const char *base, const char *file) {
	char temp_buf[80];
	char *mark;

	Common::strcpy_s(temp_buf, base);

	mark = strrchr(temp_buf, '\\');
	if (mark == NULL) {
		*target = 0;
	} else {
		*mark = 0;
		Common::strcpy_s(target, 65536, temp_buf);
		Common::strcat_s(target, 65536, "\\");
		Common::strcat_s(target, 65536, file);
		mads_strupr(target);
	}

	return target;
}


char *fileio_join_path(char *target, const char *path, const char *file) {
	char temp_buf[80];
	char *mark;

	Common::strcpy_s(temp_buf, path);

	mark = temp_buf;
	while (*mark) mark++;
	mark--;
	if (*mark != '\\')
		Common::strcat_s(temp_buf, "\\");

	Common::strcpy_s(target, 65536, temp_buf);
	Common::strcat_s(target, 65536, file);
	mads_strupr(target);

	return target;
}


void fileio_get_volume_label(char *volume_label, char drive_letter) {
	error("TODO: fileio_get_volume_label");
}

int fileio_set_file_time(char *filename, long new_time) {
	error("TODO: fileio_set_file_time");
}

int fileio_get_file_attributes(const char *filename, word *attributes) {
	error("TODO: fileio_get_file_attributes");
}

int fileio_set_file_attributes(const char *filename, word attributes) {
	error("TODO: fileio_set_file_attributes");
}

int fileio_read_till_null(char *target, Common::SeekableReadStream *handle) {
	return handle->read(target, handle->size());
}

char *fileio_prepend(char *target, const char *source, const char *prepend) {
	char temp_buf[256];

	Common::strcpy_s(temp_buf, prepend);
	Common::strcat_s(temp_buf, source);
	Common::strcpy_s(target, 65536, temp_buf);

	return target;
}

char *fileio_chop_ext(char *target, const char *source) {
	char *mark;

	if (source != target) {
		Common::strcpy_s(target, 65536, source);
	}

	mark = strchr(target, '.');
	if (mark != NULL) *mark = 0;

	return target;
}

void fileio_purge_all_spaces(char *text) {
	char work[256];
	char *mark;

	fileio_purge_trailing_spaces(text);

	mark = text;
	while (*mark && ((*mark == ' ') || (*mark == 0x09))) mark++;

	Common::strcpy_s(work, mark);
	Common::strcpy_s(text, 65536, work);
}

} // namespace MADSV2
} // namespace MADS
