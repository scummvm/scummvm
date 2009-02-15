/***************************************************************************
 dc_save.c Copyright (C) 2003 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

   Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include <beos/fnmatch.h>
#include <stdio.h>
#include <zlib/zlib.h>
#include <sci_memory.h>
#include <dc/vmu_pkg.h>

#define DC_TEMP_FILE "/ram/freesci.tmp"

/* The file header that is put before each file during the concatenation */
struct file_hdr_t {
	char filename[32];
	int filesize;
};

int dc_delete_temp_file() {
	if (fs_unlink(DC_TEMP_FILE)) {
		sciprintf("%s, L%d: fs_unlink(\"" DC_TEMP_FILE "\") failed!\n", __FILE__, __LINE__);
		return -1;
	}
	return 0;
}

static int dc_cat_write(gzFile outf, char *infname)
/* Adds a fileheader and a file's data to a compressed file at the current
** file position.
** Parameters: (gzFile) outf: The compressed file to write to.
**             (char *) infname: The path and name of the file to add.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t inf;
	char *buf, *name;
	struct file_hdr_t file_hdr;
	int rd;

	/* Determine file name */
	name = strrchr(infname, '/');
	if (name) name++;
	else name = infname;
	
	if (strlen(name) > 31) {
		sciprintf("%s, L%d: Filename `%s' too long!\n", __FILE__, __LINE__, name);
		return -1;
	}

	if (!(inf = fs_open(infname, O_RDONLY))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY) failed!\n", __FILE__, __LINE__, infname);
		return -1;
	}

	strcpy(file_hdr.filename, name);
	file_hdr.filesize = fs_total(inf);

	if (gzwrite(outf, &file_hdr, sizeof(struct file_hdr_t)) < sizeof(struct file_hdr_t)) {
		sciprintf("%s, L%d: gzwrite() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}

	buf = sci_malloc(1024);

	while ((rd = fs_read(inf, buf, 1024)) > 0) {
		if (!(gzwrite(outf, buf, rd))) {
			sciprintf("%s, L%d: gzwrite() failed!\n", __FILE__, __LINE__);
			fs_close(inf);
			sci_free(buf);
			return -1;
		}
	}

	if (rd == -1) {
		sciprintf("%s, L%d: fs_read() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		sci_free(buf);
		return -1;
	}

	fs_close(inf);
	sci_free(buf);

	return 0;
}

static int dc_cat_read(gzFile inf, char *outfdir)
/* Extracts a file from a concatenated compressed file. The current file
** position must point to a file header.
** Parameters: (gzFile) inf: The compressed file to read from.
**             (char *) outfdir: The directory path where the file should be
**               written.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t outf;
	char *buf, *outfname;
	struct file_hdr_t file_hdr;
	int rd;

	if (gzread(inf, &file_hdr, sizeof(struct file_hdr_t)) < sizeof(struct file_hdr_t)) {
		sciprintf("%s, L%d: gzread() failed!\n", __FILE__, __LINE__);
		return -1;
	}

	/* Construct output filename */
	outfname = sci_malloc(strlen(file_hdr.filename)+strlen(outfdir)+2);
	strcpy(outfname, outfdir);
	strcat(outfname, "/");
	strcat(outfname, file_hdr.filename);
	
	if (!(outf = fs_open(outfname, O_WRONLY | O_TRUNC))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_WRONLY | O_TRUNC) failed!\n", __FILE__, __LINE__, outfname);
		sci_free(outfname);
		return -1;
	}

	sci_free(outfname);
	buf = sci_malloc(1024);

	while (file_hdr.filesize > 0) {
		rd = file_hdr.filesize;
		if (rd > 1024) rd = 1024;
		if (gzread(inf, buf, rd) < rd) {
			sciprintf("%s, L%d: gzread() failed!\n", __FILE__, __LINE__);
			fs_close(outf);
			sci_free(buf);
			return -1;
		}
		if (!(fs_write(outf, buf, rd))) {
			sciprintf("%s, L%d: fs_write() failed!\n", __FILE__, __LINE__);
			fs_close(outf);
			sci_free(buf);
			return -1;
		}
		file_hdr.filesize -= rd;
	}

	fs_close(outf);
	sci_free(buf);

	return 0;
}

static int dc_is_save_file(char *fn)
/* Determines whether a filename is one of FreeSCI's filenames that are used
** for save ganes.
** Parameters: (char *) fn: The filename to consider.
** Returns   : 1 when the filename is used for save games, 0 otherwise.
*/
{
	return !fnmatch("state", fn, 0) || !fnmatch("heap", fn, 0) ||
		!fnmatch("hunk*", fn, 0) || !fnmatch("song.*", fn, 0) ||
		!fnmatch("sound", fn, 0) || !fnmatch("*.id", fn, 0);
}

static int dc_package(char *infname, char *outdir, char *outname, char* desc)
/* Packages a file with a VMU header and stores it.
** Parameters: (char *) infname: The filename of the file to package.
**             (char *) outdir: The directory path to write the packaged file.
**             (char *) outname: The filename the packaged file should get.
**             (char *) desc: The description of the file to put in the
**                      header.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t inf, outf;
	uint8 *data, *pkg_out;
	vmu_pkg_t pkg;
	int pkg_size;
	sci_dir_t dirent;
	char *olddir, *outfname;
	
	if (!(inf = fs_open(infname, O_RDONLY))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY) failed!\n", __FILE__, __LINE__, infname);
		return -1;
	}
	if (!(data = fs_mmap(inf))) {
		sciprintf("%s, L%d: fs_mmap() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	strcpy(pkg.desc_short, "FreeSCI");
	strncpy(pkg.desc_long, desc, 31);
	pkg.desc_long[31] = 0;
	strcpy(pkg.app_id, "FreeSCI");
	pkg.icon_cnt = 0;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data = data;
	pkg.data_len = fs_total(inf);
	if (vmu_pkg_build(&pkg, &pkg_out, &pkg_size) < 0) {
		sciprintf("%s, L%d: vmu_pkg_build() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}

	outfname = sci_malloc(strlen(outdir) + strlen(outname) + 2);
	strcpy(outfname, outdir);
	strcat(outfname, "/");
	strcat(outfname, outname);

	if (!(outf = fs_open(outfname, O_WRONLY | O_TRUNC))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_WRONLY | O_TRUNC) failed!\n", __FILE__, __LINE__, outfname);
		fs_close(inf);
		free(outfname);
		return -1;
	}
	free(outfname);
	if (fs_write(outf, pkg_out, pkg_size) < pkg_size) {
		sciprintf("%s, L%d: fs_write() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		fs_close(outf);
		return -1;
	}
	fs_close(inf);
	fs_close(outf);

	/* Check whether the save is actually written */

	olddir = strdup(fs_getwd());
	fs_chdir(outdir);
	sci_init_dir(&dirent);
	if (!sci_find_first(&dirent, outname)) {
		sciprintf("%s, L%d: Write to VMU failed!\n", __FILE__, __LINE__);
		sci_finish_find(&dirent);
		fs_chdir(olddir);
		sci_free(olddir);
		return -1;
	}
	sci_finish_find(&dirent);
	fs_chdir(olddir);
	sci_free(olddir);
	return 0;
}

static int dc_depackage(char *infname, char *outfname)
/* Depackages a file with a VMU header and stores it.
** Parameters: (char *) infname: The full path and filename of the file to
**                      depackage.
**             (char *) outfname: The full path and filename of where to
**                      write the depackaged file.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t inf, outf;
	uint8 *data;
	vmu_pkg_t pkg;
	if (!(inf = fs_open(infname, O_RDONLY))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY) failed!\n", __FILE__, __LINE__, infname);
		return -1;
	}
	if (!(data = fs_mmap(inf))) {
		sciprintf("%s, L%d: fs_mmap() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	if (vmu_pkg_parse(data, &pkg) == -1) {
		sciprintf("%s, L%d: vmu_pkg_parse() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	if (!(outf = fs_open(outfname, O_WRONLY | O_TRUNC))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_WRONLY | O_TRUNC) failed!\n", __FILE__, __LINE__, outfname);
		fs_close(inf);
		return -1;
	}
	if (fs_write(outf, pkg.data, pkg.data_len) < pkg.data_len) {
		sciprintf("%s, L%d: fs_write() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		fs_close(outf);
		return -1;
	}
	fs_close(inf);
	fs_close(outf);
	return 0;
}

static int store_files(char *src_dir, char *tar_dir, char *tar_name, char *desc, int mode)
/* Concatenates files from a directory and creates a package that can be put
** on a VMU.
** Parameters: (char *) src_dir: The full path of the directory that contains
**                      the files that should be stored.
**             (char *) tar_dir: The full path of the directory where the
**                      resulting file should be placed.
**             (char *) tar_name: The file name that the resulting file should
**                      get.
**             (char *) desc: The description to use for the VMU header.
**             (int) mode: 0: all files will be stored; 1: only FreeSCI save
**                   files will be stored.
** Returns   : 0 on success, -1 on error.
*/
{
	gzFile outf;
	file_t d;
	dirent_t *entry;
	
	if (!(d = fs_open(src_dir, O_RDONLY | O_DIR))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY | O_DIR) failed!\n", __FILE__, __LINE__, src_dir);
		return -1;
	}
	
	if (!(outf = gzopen(DC_TEMP_FILE, "w"))) {
		sciprintf("%s, L%d: gzopen(\"" DC_TEMP_FILE "\", \"w\") failed!\n", __FILE__, __LINE__);
		return -1;
	}

	while ((entry = fs_readdir(d))) {
		if ((mode == 1) && !dc_is_save_file(entry->name)) continue;
		char *fn = sci_malloc(strlen(src_dir)+strlen(entry->name)+2);
		strcpy(fn, src_dir);
		strcat(fn, "/");
		strcat(fn, entry->name);
		if (dc_cat_write(outf, fn)) {
			sciprintf("%s, L%d: dc_cat_write() failed!\n", __FILE__, __LINE__);
			sci_free(fn);
			fs_close(d);
			gzclose(outf);
			dc_delete_temp_file();
			return -1;
		}
		sci_free(fn);
	}

	fs_close(d);

	if (gzclose(outf) < 0) {
		sciprintf("%s, L%d: gzclose() failed!\n", __FILE__, __LINE__);
		return -1;
	}

	if (dc_package(DC_TEMP_FILE, tar_dir, tar_name, desc)) {
		sciprintf("%s, L%d: dc_package(\"" DC_TEMP_FILE "\", \"%s\", \"%s\", \"%s\") failed!\n", __FILE__, __LINE__, tar_dir, tar_name, desc);
		return -1;
	}

	return dc_delete_temp_file();
}

static int retrieve_files(char *fname, char *dir)
/* Extracts all files from a packaged concatenated file.
** Parameters: (char *) fname: The full path and filename of the concatenated
**                      packaged file.
**             (char *) dir: The full path of the directory where the
**                      resulting files should be placed.
** Returns   : 0 on success, -1 on error.
*/
{
	gzFile inf;
	if (dc_depackage(fname, DC_TEMP_FILE)) {
		sciprintf("%s, L%d: dc_depackage(\"%s\", \"" DC_TEMP_FILE "\") failed!\n", __FILE__, __LINE__, fname);
		return -1;
	}
	if (!(inf = gzopen(DC_TEMP_FILE, "r"))) {
		sciprintf("%s, L%d: gzopen(\"" DC_TEMP_FILE "\", \"r\") failed!\n", __FILE__, __LINE__);
		return -1;
	}
	while(!gzeof(inf)) dc_cat_read(inf, dir);
	if (gzclose(inf) < 0) {
		sciprintf("%s, L%d: gzclose() failed!\n", __FILE__, __LINE__);
		return -1;
	}
	return dc_delete_temp_file();
}

void dc_delete_save_files(char *dir) {
	file_t d;
	dirent_t *entry;
	
	if (!(d = fs_open(dir, O_RDONLY | O_DIR))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY | O_DIR) failed!\n", __FILE__, __LINE__, dir);
		return;
	}
	
	while ((entry = fs_readdir(d))) {
		char *fn = sci_malloc(strlen(dir)+strlen(entry->name)+2);
		strcpy(fn, dir);
		strcat(fn, "/");
		strcat(fn, entry->name);
		if ((dc_is_save_file(entry->name)) && (fs_unlink(fn) < 0))
			sciprintf("%s, L%d: fs_unlink(\"%s\") failed!\n", __FILE__, __LINE__, fn);
		sci_free(fn);
	}
	
	fs_close(d);
}

char *dc_get_cat_name(char *game_name, int nr) {
	char suffices[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char *save_name = sci_malloc(strlen(game_name) + 3);
	strcpy(save_name, game_name);
	save_name[strlen(game_name)] = '.';
  	save_name[strlen(game_name) + 1] = suffices[nr];
	save_name[strlen(game_name) + 2] = 0;
	return save_name;
}

char *dc_get_first_vmu() {
	sci_dir_t dirent;
	char *olddir = strdup(fs_getwd());
	char *vmu;
	char *retval = NULL;
	fs_chdir("/vmu");
	sci_init_dir(&dirent);
	vmu = sci_find_first(&dirent, "*");
	if (vmu) {
		retval = sci_malloc(5 + strlen(vmu) + 1);
		strcpy(retval, "/vmu/");
		strcat(retval, vmu);
	}
	sci_finish_find(&dirent);
	fs_chdir(olddir);
	sci_free(olddir);
	return retval;
}

int dc_retrieve_savegame(char *game_name, int nr) {
	int retval = 0;
	char *name, *fname, *vmu;

	dc_delete_save_files("/ram");

	if (!(vmu = dc_get_first_vmu())) {
		sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);
		return -1;
	}
	name = dc_get_cat_name(game_name, nr);
	fname = sci_malloc(strlen(vmu) + strlen(name) + 2);

	strcpy(fname, vmu);
	strcat(fname, "/");
	strcat(fname, name);
	sci_free(name);
	sci_free(vmu);
	if (retrieve_files(fname, "/ram") < 0) {
		sciprintf("%s, L%d: retrieve_files(\"%s\", \"/ram\") failed!\n", __FILE__, __LINE__, fname);
		retval = -1;
	}
	sci_free(fname);
	return retval;
}

int dc_store_savegame(char *game_name, char *desc, int nr) {
	char *vmu, *name;
	int retval = 0;
	
	if (!(vmu = dc_get_first_vmu())) {
		sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);
		return -1;
	}

	name = dc_get_cat_name(game_name, nr);

	if (store_files("/ram", vmu, name, desc, 1) < 0) {
		sciprintf("%s, L%d: store_files(\"/ram\", \"%s\", \"%s\", \"%s\") failed!\n", __FILE__, __LINE__, vmu, name, desc);
		retval = -1;
	}

	dc_delete_save_files("/ram");

	sci_free(name);
	sci_free(vmu);
	
	return retval;
}

int dc_retrieve_mirrored(char *game_name) {
	int retval = 0;
	char *vmu, *fname;

	if (!(vmu = dc_get_first_vmu())) {
		sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);
		return -1;
	}

	fname = sci_malloc(strlen(vmu) + strlen(game_name) + 2);

	strcpy(fname, vmu);
	strcat(fname, "/");
	strcat(fname, game_name);
	sci_free(vmu);
	if (retrieve_files(fname, "/ram") < 0) {
		sciprintf("%s, L%d: retrieve_files(\"%s\", \"/ram\") failed!\n", __FILE__, __LINE__, fname);
		retval = -1;
	}
	sci_free(fname);
	return retval;
}

int dc_store_mirrored(char *game_name) {
	int retval = 0;
	char *vmu;
	sci_dir_t dirent;
	char *olddir = strdup(fs_getwd());

	fs_chdir("/ram");

	sci_init_dir(&dirent);
	if (!sci_find_first(&dirent, "*")) {
		sciprintf("%s, L%d: No mirrored files found!\n", __FILE__, __LINE__);
		sci_finish_find(&dirent);
		fs_chdir(olddir);
		sci_free(olddir);
		return 0;
	}

	sci_finish_find(&dirent);
	fs_chdir(olddir);
	sci_free(olddir);

	if (!(vmu = dc_get_first_vmu())) {
		sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);
		return -1;
	}

	if (store_files("/ram", vmu, game_name, "Configuration", 0) < 0) {
		sciprintf("%s, L%d: store_files(\"/ram\", \"%s\", \"%s\", \"Configuration\") failed!\n", __FILE__, __LINE__, vmu, game_name);
		retval = -1;
	}

	sci_free(vmu);

	return retval;
}
