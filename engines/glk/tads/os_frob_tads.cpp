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

#include "glk/tads/os_frob_tads.h"
#include "common/file.h"
#include "common/memstream.h"

namespace Glk {
namespace TADS {

static osfildef *openForReading(const char *fname) {
	Common::File f;
	if (f.open(fname))
		return f.readStream(f.size());

	Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(fname);
	return save;
}

static osfildef *openForWriting(const char *fname) {
	return g_system->getSavefileManager()->openForSaving(fname, false);
}

int osfacc(const char *fname) {
	return Common::File::exists(fname) ? 1 : 0;
}

osfildef *osfoprt(const char *fname, os_filetype_t typ) {
	return openForReading(fname);
}

osfildef *osfoprtv(const char *fname, os_filetype_t typ) {
	return openForReading(fname);
}

osfildef *osfopwt(const char *fname, os_filetype_t typ) {
	return openForWriting(fname);
}

osfildef *osfoprwt(const char *fname, os_filetype_t typ) {
	warning("ScummVM files can't be opened for both reading and writing simultaneously");
	return openForWriting(fname);
}

osfildef *osfoprwtt(const char *fname, os_filetype_t typ) {
	warning("ScummVM files can't be opened for both reading and writing simultaneously");
	return openForWriting(fname);
}

osfildef *osfopwb(const char *fname, os_filetype_t typ) {
	return openForWriting(fname);
}

osfildef *osfoprs(const char *fname, os_filetype_t typ) {
	return openForReading(fname);
}

osfildef *osfoprb(const char *fname, os_filetype_t typ) {
	return openForReading(fname);
}

osfildef *osfoprbv(const char *fname, os_filetype_t typ) {
	return openForReading(fname);
}

osfildef *osfoprwb(const char *fname, os_filetype_t typ) {
	warning("ScummVM files can't be opened for both reading and writing simultaneously");
	return openForWriting(fname);
}

osfildef *osfoprwtb(const char *fname, os_filetype_t typ) {
	warning("ScummVM files can't be opened for both reading and writing simultaneously");
	return openForWriting(fname);
}

osfildef *osfdup(osfildef *orig, const char *mode) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(orig);
	int32 currPos = rs->pos();

	rs->seek(0);
	osfildef *result = rs->readStream(rs->size());
	rs->seek(currPos);

	return result;
}

void os_settype(const char *f, os_filetype_t typ) {
	// No implementation
}

char *osfgets(char *buf, size_t count, osfildef *fp) {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(fp);
	char *ptr = buf;
	char c;
	while (!rs->eos() && --count > 0) {
		c = rs->readByte();
		if (c == '\n' || c == '\0')
			break;
		*ptr++ = c;
	}

	*ptr++ = '\0';
	return buf;
}

int osfputs(const char *str, osfildef *fp) {
	return dynamic_cast<Common::WriteStream *>(fp)->write(str, strlen(str)) == strlen(str) ? 0 : -1;
}

void os_fprintz(osfildef *fp, const char *str) {
	dynamic_cast<Common::WriteStream *>(fp)->write(str, strlen(str));
}

void os_fprint(osfildef *fp, const char *str, size_t len) {
	Common::String s(str, str + MIN(len, strlen(str)));
	dynamic_cast<Common::WriteStream *>(fp)->write(s.c_str(), s.size());
}

int osfwb(osfildef *fp, const void *buf, size_t bufl) {
	return dynamic_cast<Common::WriteStream *>(fp)->write(buf, bufl) == bufl ? 0 : 1;
}

int osfflush(osfildef *fp) {
	return dynamic_cast<Common::WriteStream *>(fp)->flush() ? 0 : 1;
}

int osfgetc(osfildef *fp) {
	return dynamic_cast<Common::ReadStream *>(fp)->readByte();
}

int osfrb(osfildef *fp, void *buf, size_t bufl) {
	return dynamic_cast<Common::ReadStream *>(fp)->read(buf, bufl) == bufl ? 0 : 1;
}

size_t osfrbc(osfildef *fp, void *buf, size_t bufl) {
	return dynamic_cast<Common::ReadStream *>(fp)->read(buf, bufl);
}

long osfpos(osfildef *fp) {
	return dynamic_cast<Common::SeekableReadStream *>(fp)->pos();
}

int osfseek(osfildef *fp, long pos, int mode) {
	return dynamic_cast<Common::SeekableReadStream *>(fp)->seek(pos, mode);
}

void osfcls(osfildef *fp) {
	delete fp;
}

int osfdel(const char *fname) {
	return g_system->getSavefileManager()->removeSavefile(fname) ? 0 : 1;
}

int os_rename_file(const char *oldname, const char *newname) {
	return g_system->getSavefileManager()->renameSavefile(oldname, newname);
}


bool os_locate(const char *fname, int flen, const char *arg0, char *buf, size_t bufsiz) {
	Common::String name = !flen ? Common::String(fname) : Common::String(fname, fname + flen);

	if (!Common::File::exists(fname))
		return false;

	strncpy(buf, name.c_str(), bufsiz - 1);
	buf[bufsiz - 1] = '\0';
	return true;
}

osfildef *os_create_tempfile(const char *fname, char *buf) {
	strcpy(buf, "tmpfile");
	return new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
}

int osfdel_temp(const char *fname) {
	// Temporary files in ScummVM are just memory streams, so there isn't a file to delete
	return 0;
}

void os_get_tmp_path(char *buf) {
	strcpy(buf, "");
}

int os_gen_temp_filename(char *buf, size_t buflen) {
	error("TODO: If results from this are being passed to file open methods, will need to do further work");
}

/* ------------------------------------------------------------------------ */

void os_set_pwd(const char *dir) {
	// No implementation
}

void os_set_pwd_file(const char *filename) {
	// No implementation
}

bool os_mkdir(const char *dir, int create_parents) {
	// Unsupported
	return false;
}

bool os_rmdir(const char *dir) {
	// Unsupported
	return false;
}

/* ------------------------------------------------------------------------ */

void os_defext(char *fname, const char *ext) {
	if (!strchr(fname, '.'))
		strcat(fname, ext);
}

void os_addext(char *fname, const char *ext) {
	strcat(fname, ext);
}

void os_remext(char *fname) {
	char *p = strchr(fname, '.');
	if (p)
		*p = '\0';
}

bool os_file_names_equal(const char *a, const char *b) {
	return !strcmp(a, b);
}

const char *os_get_root_name(const char *buf) {
	return buf;
}

bool os_is_file_absolute(const char *fname) {
	return false;
}

void os_get_path_name(char *pathbuf, size_t pathbuflen, const char *fname) {
	strcpy(pathbuf, "");
}

void os_build_full_path(char *fullpathbuf, size_t fullpathbuflen,
	const char *path, const char *filename) {
	strcpy(fullpathbuf, filename);
}

void os_combine_paths(char *fullpathbuf, size_t pathbuflen,
	const char *path, const char *filename) {
	strcpy(fullpathbuf, filename);
}

bool os_get_abs_filename(char *result_buf, size_t result_buf_size,
	const char *filename) {
	strcpy(result_buf, filename);
	return true;
}

bool os_get_rel_path(char *result_buf, size_t result_buf_size,
	const char *basepath, const char *filename) {
	strcpy(result_buf, filename);
	return true;
}

bool os_is_file_in_dir(const char *filename, const char *path,
	bool include_subdirs, bool match_self) {
	assert(!include_subdirs && !match_self);

	return Common::File::exists(filename);
}

} // End of namespace TADS
} // End of namespace Glk
