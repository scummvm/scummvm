/* Header is not present in Windows CE SDK */
/* It would not be a bad idea to take this thing from gcc distro and port
   it properly. For now only required part is ported. */

struct dirent
{
	long		d_ino;		/* Always zero. */
	unsigned short	d_reclen;	/* Always zero. */
	unsigned short	d_namlen;	/* Length of name in d_name. */
	char*		d_name; 	/* File name. */
	/* NOTE: The name in the dirent structure points to the name in the
	 *		 finddata_t structure in the DIR. */
};

/*
 * This is an internal data structure. Good programmers will not use it
 * except as an argument to one of the functions below.
 */
typedef struct
{
	/* disk transfer area for this dir */
/*	struct _finddata_t	dd_dta; */

	/* dirent struct to return from dir (NOTE: this makes this thread
	 * safe as long as only one thread uses a particular DIR struct at
	 * a time) */
	struct dirent		dd_dir;

	/* _findnext handle */
	long			dd_handle;

	/*
		 * Status of search:
	 *	 0 = not started yet (next entry to read is first entry)
	 *	-1 = off the end
	 *	 positive = 0 based index of next entry
	 */
	short			dd_stat;

	/* given path for dir with search pattern (struct is extended) */
	char			dd_name[1];
} DIR;


DIR*		opendir (const char*);
struct dirent*	readdir (DIR*);
int 	closedir (DIR*);
/*
void		rewinddir (DIR*);
long		telldir (DIR*);
void		seekdir (DIR*, long);
*/
