
#ifndef __GPSTDIO_H__
#define __GPSTDIO_H__

#include "gpdef.h"
/*****************************************************************/
/*GP32 FAT                                                        */
/*****************************************************************/
typedef enum {
	SM_OK,					/* 0 */
	ERR_FLASH_STATUS,
	ERR_CARD_NOT_DETECTED,
	ERR_CARD_CHANGED,
	ERR_NOT_ERASED,
	ERR_NOT_FORMATTED,
	ERR_INVALID_MBR,
	ERR_INVALID_PBR,
	ERR_INVALID_FAT,
	ERR_INVALID_NAME,
	ERR_INVALID_HANDLE,		/* 10 */
	ERR_INVALID_PARAM,
	ERR_NO_EMPTY_BLOCK,
	ERR_INVALID_BLOCK,
	ERR_ECC,
	ERR_ECC_CORRECTABLE,
	ERR_FILE_OPENED,
	ERR_FILE_EXIST,
	ERR_DIR_NOT_EMPTY,
	ERR_EOF,
	ERR_FILE_NOT_OPENED,	/* 20 */
	ERR_FILE_NOT_EXIST,
	ERR_NO_LONG_NAME,
	ERR_NOT_FOUND,
	ERR_INCORRECT_FAT,
	ERR_ROOT_DIR,
	ERR_LOCKED,
	ERR_NOT_PERMITTED,
	ERR_OUT_OF_MEMORY,
	ERR_OUT_OF_ROOT_ENTRY,
	ERR_NO_MORE_ENTRY,		/* 30 */
	ERR_SM_TIMEOUT,
	ERR_SYSTEM_PARAMETER,
	ERR_INTERNAL,
	ERR_FILE_NAME_LEN_TOO_LONG,
	ERR_NO_EMPTY_CLUSTER,
	ERR_INVALID_R_PATH,
	ERR_ID_NOT_FOUND,
	ERR_ID_INVALID,
	ERR_NOT_INITIALIZED
} ERR_CODE;		/* 0 when success. otherwise error code */

typedef long F_HANDLE;	/* MSB is 1, when error. otherwise cluster
						 * number
						 *	(bits 30 - 24: device number (0 - 128))
						 *	(bits 23 - 17: file opened id) =>
						 *		used multi-access mode
						 */
						 
/* fcreate mode */
#define NOT_IF_EXIST		0
#define ALWAYS_CREATE		1

/* open mode (can be ORed) */
#define OPEN_R				1
#define OPEN_W				2

/* ddel mode */
#define NOT_IF_NOT_EMPTY	0
#define ALWAYS_DELETE		1

/* seek mode */
#define FROM_CURRENT		0
#define FROM_BEGIN			1
#define FROM_END			2

/* format mode */
#define FORMAT_NORMAL		0
#define FORMAT_RESCUE		1


typedef struct {
     uword year;
     ubyte month;
     ubyte day;
     ubyte hour;
     ubyte min;
     ubyte sec;
     ubyte msec;
}sTIME;

/**********
 * attr
 *	7-6 Reserved. Must be 0's.
 *	5: 1 = ARCHIVE file was modified
 *	4: 1 = DIRECTORY, 0 = file
 *	3: 1 = VOLUME label
 *	2: 1 = SYSTEM file or directory
 *	1: 1 = HIDDEN file or directory
 *	0: 1 = READONLY file
 **********/
typedef struct {
	udword attr;
	udword cluster;
	udword size;
	sTIME time;
} GPFILEATTR;

typedef struct {           
	char name[16];
} GPDIRENTRY;


ERR_CODE GpFatInit (void);
ERR_CODE GpRelativePathSet (const char * p_path);
void GpRelativePathGet (char *p_path);

ERR_CODE GpFileCreate (const char * p_file_name, ulong fcreate_mode, F_HANDLE *p_handle);
ERR_CODE GpFileOpen (const char * p_file_name, ulong fopen_mode, F_HANDLE *p_handle);
ERR_CODE GpFileRead (F_HANDLE h_file, void * p_buf, ulong buf_size, ulong * p_read_count);
ERR_CODE GpFileWrite (F_HANDLE h_file, const void * p_buf, ulong count);
ERR_CODE GpFileSeek (F_HANDLE h_file, ulong seek_mode, ulong offset, long * p_old_offset);
ERR_CODE GpFileClose (F_HANDLE h_file);
ERR_CODE GpFileRemove (const char * p_file_name);
ERR_CODE GpFileGetSize (const char * p_file_name, ulong * p_size);
ERR_CODE GpFileExtend (F_HANDLE h_file, ulong size);
ERR_CODE GpFileMove (const char * old_path, const char * new_path);
ERR_CODE GpFileRename (const char * old_path, const char * new_path);

ERR_CODE GpDirCreate (const char * p_dir_name, ulong dcreate_mode);
ERR_CODE GpDirRemove (const char * p_dir_name, ulong ddel_mode);
ERR_CODE GpDirEnumNum (const char * p_dir_name, ulong * p_num);
ERR_CODE GpDirEnumList (const char * p_dir_name, ulong entry_start, ulong entry_count, GPDIRENTRY * p_list, ulong * p_read_count);

ERR_CODE GpFileAttr (const char * p_name, GPFILEATTR * p_attr);

ERR_CODE GpVolumeInfo (const char * p_vol_name, ulong * total_size, ulong * used_size, ulong * free_size);
ERR_CODE GpFormat (const char * p_vol_name, ulong format_id, ulong * p_bad_block);

ERR_CODE GpNoFATUpdate (const char * p_vol_name);
ERR_CODE GpFATUpdate (const char * p_vol_name);

ERR_CODE GpFatInitialized (void);
ERR_CODE GpSMCDetected (void);

#endif
