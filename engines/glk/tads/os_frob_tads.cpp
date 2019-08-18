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



/* ------------------------------------------------------------------------ */
/*
*   Convert an OS filename path to URL-style format.  This isn't a true URL
*   conversion; rather, it simply expresses a filename in Unix-style
*   notation, as a series of path elements separated by '/' characters.
*   Unlike true URLs, we don't use % encoding or a scheme prefix (file://,
*   etc).
*
*   The result path never ends in a trailing '/', unless the entire result
*   path is "/".  This is for consistency; even if the source path ends with
*   a local path separator, the result doesn't.
*
*   If the local file system syntax uses '/' characters as ordinary filename
*   characters, these must be replaced with some other suitable character in
*   the result, since otherwise they'd be taken as path separators when the
*   URL is parsed.  If possible, the substitution should be reversible with
*   respect to os_cvt_dir_url(), so that the same URL read back in on this
*   same platform will produce the same original filename.  One particular
*   suggestion is that if the local system uses '/' to delimit what would be
*   a filename extension on other platforms, replace '/' with '.', since
*   this will provide reversibility as well as a good mapping if the URL is
*   read back in on another platform.
*
*   The local equivalents of "." and "..", if they exist, are converted to
*   "." and ".." in the URL notation.
*
*   Examples:
*
*.   Windows: images\rooms\startroom.jpg -> images/rooms/startroom.jpg
*.   Windows: ..\startroom.jpg -> ../startroom.jpg
*.   Mac:     :images:rooms:startroom.jpg -> images/rooms/startroom.jpg
*.   Mac:     ::startroom.jpg -> ../startroom.jpg
*.   VMS:     [.images.rooms]startroom.jpg -> images/rooms/startroom.jpg
*.   VMS:     [-.images]startroom.jpg -> ../images/startroom.jpg
*.   Unix:    images/rooms/startroom.jpg -> images/rooms/startroom.jpg
*.   Unix:    ../images/startroom.jpg -> ../images/startroom.jpg
*
*   If the local name is an absolute path in the local file system (e.g.,
*   Unix /file, Windows C:\file), translate as follows.  If the local
*   operating system uses a volume or device designator (Windows C:, VMS
*   SYS$DISK:, etc), make the first element of the path the exact local
*   syntax for the device designator: /C:/ on Windows, /SYS$DISK:/ on VMS,
*   etc.  Include the local syntax for the device prefix.  For a system like
*   Unix with a unified file system root ("/"), simply start with the root
*   directory.  Examples:
*
*.    Windows:  C:\games\deep.gam         -> /C:/games/deep.gam
*.    Windows:  C:games\deep.gam          -> /C:./games/deep.gam
*.    Windows:  \\SERVER\DISK\games\deep.gam -> /\\SERVER/DISK/games/deep.gam
*.    Mac OS 9: Hard Disk:games:deep.gam  -> /Hard Disk:/games/deep.gam
*.    VMS:      SYS$DISK:[games]deep.gam  -> /SYS$DISK:/games/deep.gam
*.    Unix:     /games/deep.gam           -> /games/deep.gam
*
*   Rationale: it's effectively impossible to create a truly portable
*   representation of an absolute path.  Operating systems are too different
*   in the way they represent root paths, and even if that were solvable, a
*   root path is essentially unusable across machines anyway because it
*   creates a dependency on the contents of a particular machine's disk.  So
*   if we're called upon to translate an absolute path, we can forget about
*   trying to be truly portable and instead focus on round-trip fidelity -
*   i.e., making sure that applying os_cvt_url_dir() to our result recovers
*   the exact original path, assuming it's done on the same operating
*   system.  The approach outlined above should achieve round-trip fidelity
*   when a local path is converted to a URL and back on the same machine,
*   since the local URL-to-path converter should recognize its own special
*   type of local absolute path prefix.  It also produces reasonable results
*   on other platforms - see the os_cvt_url_dir() comments below for
*   examples of the decoding results for absolute paths moved to new
*   platforms.  The result when a device-rooted absolute path is encoded on
*   one machine and then decoded on another will generally be a local path
*   with a root on the default device/volume and an outermost directory with
*   a name based on the original machine's device/volume name.  This
*   obviously won't reproduce the exact original path, but since that's
*   impossible anyway, this is probably as good an approximation as we can
*   create.
*
*   Character sets: the input could be in local or UTF-8 character sets.
*   The implementation shouldn't care, though - just treat bytes in the
*   range 0-127 as plain ASCII, and everything else as opaque.  I.e., do not
*   quote or otherwise modify characters outside the 0-127 range.
*/
void os_cvt_dir_url(char *result_buf, size_t result_buf_size,
	const char *src_path);

/*
*   Convert a URL-style path into a filename path expressed in the local
*   file system's syntax.  Fills in result_buf with a file path, constructed
*   using the local file system syntax, that corresponds to the path in
*   src_url expressed in URL-style syntax.  Examples:
*
*   images/rooms/startroom.jpg ->
*.   Windows   -> images\rooms\startroom.jpg
*.   Mac OS 9  -> :images:rooms:startroom.jpg
*.   VMS       -> [.images.rooms]startroom.jpg
*
*   The source format isn't a true URL; it's simply a series of path
*   elements separated by '/' characters.  Unlike true URLs, our input
*   format doesn't use % encoding and doesn't have a scheme (file://, etc).
*   (Any % in the source is treated as an ordinary character and left as-is,
*   even if it looks like a %XX sequence.  Anything that looks like a scheme
*   prefix is left as-is, with any // treated as path separators.
*
*   images/file%20name.jpg ->
*.   Windows   -> images\file%20name.jpg
*
*   file://images/file.jpg ->
*.   Windows   -> file_\\images\file.jpg
*
*   Any characters in the path that are invalid in the local file system
*   naming rules are converted to "_", unless "_" is itself invalid, in
*   which case they're converted to "X".  One exception is that if '/' is a
*   valid local filename character (rather than a path separator as it is on
*   Unix and Windows), it can be used as the replacement for the character
*   that os_cvt_dir_url uses as its replacement for '/', so that this
*   substitution is reversible when a URL is generated and then read back in
*   on this same platform.
*
*   images/file:name.jpg ->
*.   Windows   -> images\file_name.jpg
*.   Mac OS 9  -> :images:file_name.jpg
*.   Unix      -> images/file:name.jpg
*
*   The path elements "." and ".." are specifically defined as having their
*   Unix meanings: "." is an alias for the preceding path element, or the
*   working directory if it's the first element, and ".." is an alias for
*   the parent of the preceding element.  When these appear as path
*   elements, this routine translates them to the appropriate local
*   conventions.  "." may be translated simply by removing it from the path,
*   since it reiterates the previous path element.  ".." may be translated
*   by removing the previous element - HOWEVER, if ".." appears as the first
*   element, it has to be retained and translated to the equivalent local
*   notation, since it will have to be applied later, when the result_buf
*   path is actually used to open a file, at which point it will combined
*   with the working directory or another base path.
*
*.  /images/../file.jpg -> [Windows] file.jpg
*.  ../images/file.jpg ->
*.   Windows  -> ..\images\file.jpg
*.   Mac OS 9 -> ::images:file.jpg
*.   VMS      -> [-.images]file.jpg
*
*   If the URL path is absolute (starts with a '/'), the routine inspects
*   the path to see if it was created by the same OS, according to the local
*   rules for converting absolute paths in os_cvt_dir_url() (see).  If so,
*   we reverse the encoding done there.  If it doesn't appear that the name
*   was created by the same operating system - that is, if reversing the
*   encoding doesn't produce a valid local filename - then we create a local
*   absolute path as follows.  If the local system uses device/volume
*   designators, we start with the current working device/volume or some
*   other suitable default volume.  We then add the first element of the
*   path, if any, as the root directory name, applying the usual "_" or "X"
*   substitution for any characters that aren't allowed in local names.  The
*   rest of the path is handled in the usual fashion.
*
*.  /images/file.jpg ->
*.    Windows -> \images\file.jpg
*.    Unix    -> /images/file.jpg
*
*.  /c:/images/file.jpg ->
*.    Windows -> c:\images\file.jpg
*.    Unix    -> /c:/images/file.jpg
*.    VMS     -> SYS$DISK:[c__.images]file.jpg
*
*.  /Hard Disk:/images/file.jpg ->
*.    Windows -> \Hard Disk_\images\file.jpg
*.    Unix    -> SYS$DISK:[Hard_Disk_.images]file.jpg
*
*   Note how the device/volume prefix becomes the top-level directory when
*   moving a path across machines.  It's simply not possible to reconstruct
*   the exact original path in such cases, since device/volume syntax rules
*   have little in common across systems.  But this seems like a good
*   approximation in that (a) it produces a valid local path, and (b) it
*   gives the user a reasonable basis for creating a set of folders to mimic
*   the original source system, if they want to use that approach to port
*   the data rather than just changing the paths internally in the source
*   material.
*
*   Character sets: use the same rules as for os_cvt_dir_url().
*/
void os_cvt_url_dir(char *result_buf, size_t result_buf_size,
	const char *src_url);


} // End of namespace TADS
} // End of namespace Glk
