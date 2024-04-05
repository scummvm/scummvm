/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include <windows.h>

extern "C" {
#define API
#include "api.h"
};

#include "infozip_api.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

namespace infozip_api
{
	int WINAPI dummy_password(LPSTR p, int n, LPCSTR m, LPCSTR name);
	int WINAPI dummy_print(char far *buf, unsigned long size);
	int WINAPI dummy_comment(char *szBuf);
};

/* --------------------------- DEFINITION SECTION --------------------------- */

namespace infozip_api
{

bool init()
{
	ZIPUSERFUNCTIONS zip_functions;
	memset(&zip_functions,0,sizeof(ZIPUSERFUNCTIONS));
	zip_functions.comment = dummy_comment;
	zip_functions.password = dummy_password;
	zip_functions.print = dummy_print;
	ZpInit(&zip_functions);

	set_options(0);
	return true;
}

bool set_options(int compression)
{
	ZPOPT zip_options;

	zip_options.Date = NULL;             /* Date to include after */
	zip_options.szRootDir = NULL;        /* Directory to use as base for zipping */
	zip_options.szTempDir = NULL;        /* Temporary directory used during zipping */
	zip_options.fTemp = FALSE;             /* Use temporary directory '-b' during zipping */
	zip_options.fSuffix = FALSE;           /* include suffixes (not implemented) */
	zip_options.fEncrypt = FALSE;          /* encrypt files */
	zip_options.fSystem = TRUE;           /* include system and hidden files */
	zip_options.fVolume = FALSE;           /* Include volume label */
	zip_options.fExtra = FALSE;            /* Exclude extra attributes */
	zip_options.fNoDirEntries = FALSE;     /* Do not add directory entries */
	zip_options.fExcludeDate = FALSE;      /* Exclude files newer than specified date */
	zip_options.fIncludeDate = FALSE;      /* Include only files newer than specified date */
	zip_options.fVerbose = FALSE;          /* Mention oddities in zip file structure */
	zip_options.fQuiet = TRUE;            /* Quiet operation */
	zip_options.fCRLF_LF = FALSE;          /* Translate CR/LF to LF */
	zip_options.fLF_CRLF = FALSE;          /* Translate LF to CR/LF */
	zip_options.fJunkDir = FALSE;          /* Junk directory names */
	zip_options.fGrow = TRUE;             /* Allow appending to a zip file */
	zip_options.fForce = FALSE;            /* Make entries using DOS names (k for Katz) */
	zip_options.fMove = FALSE;             /* Delete files added or updated in zip file */
	zip_options.fDeleteEntries = FALSE;    /* Delete files from zip file */
	zip_options.fUpdate = FALSE;           /* Update zip file--overwrite only if newer */
	zip_options.fFreshen = FALSE;          /* Freshen zip file--overwrite only */
	zip_options.fJunkSFX = TRUE;          /* Junk SFX prefix */
	zip_options.fLatestTime = FALSE;       /* Set zip file time to time of latest file in it */
	zip_options.fComment = FALSE;          /* Put comment in zip file */
	zip_options.fOffsets = FALSE;          /* Update archive offsets for SFX files */
	zip_options.fPrivilege = FALSE;        /* Use privileges (WIN32 only) */
	zip_options.fEncryption = FALSE;       /* TRUE if encryption supported, else FALSE.
				   this is a read only flag */
	zip_options.fRecurse = 0;          /* Recurse into subdirectories. 1 => -r, 2 => -R */
	zip_options.fRepair = 0;           /* Repair archive. 1 => -F, 2 => -FF */

	if(compression < 0) compression = 0;
	if(compression > 9) compression = 9;

	zip_options.fLevel = '0' + compression;            /* Compression level (0 - 9) */

	ZpSetOptions(&zip_options);

	return true;
}

bool add_to_zip(const char* zip_file_name, const char* file_name, int compression)
{
	static char* file_list[1];

	ZCL zip_comline;
	zip_comline.argc = 1;
	zip_comline.lpszZipFN = const_cast<char*>(zip_file_name);
	zip_comline.FNV = file_list;
	zip_comline.FNV[0] = const_cast<char*>(file_name);

	set_options(compression);

	return !ZpArchive(zip_comline);
}

int WINAPI dummy_password(LPSTR p, int n, LPCSTR m, LPCSTR name)
{
	return 1;
}

int WINAPI dummy_print(char far *buf, unsigned long size)
{
//	printf("%s", buf);
	return (unsigned int)size;
}

int WINAPI dummy_comment(char *szBuf)
{
	szBuf[0] = '\0';
	return TRUE;
}

}; // namespace infozip_api


