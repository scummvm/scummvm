/* Header is not present in Windows CE SDK */

#include <sys/types.h>

struct stat {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	_off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
};


#define _S_IFDIR        0040000         /* directory */
#define S_IFDIR  _S_IFDIR

int stat(const char *, struct stat *);
