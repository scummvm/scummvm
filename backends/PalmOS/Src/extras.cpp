//##############################################################################
#include "extras.h"
#include "StarterRsc.h"
#include "mathlib.h"
#include "globals.h"
//#include "palm.h"
//##############################################################################
//UInt16	gVolRefNum = 0;
//UInt16	HRrefNum,SndRefNum;

//FileRef	logfile;
//Boolean _flipping = false;
//Boolean _vibrator = false;
//extern Boolean gFlipping;
//extern FileRef gLogFile;
//##############################################################################
//##############################################################################
void *memchr(const void *s, int c, UInt32 n)
{
	UInt32 chr;
	for(chr = 0; chr < n;chr++,((UInt8 *)s)++)
		if ( *((UInt8 *)s) == c)
			return (void *)s;
	
	return NULL;
} 
/*

UInt32 time(UInt32 *cur)
{
	UInt32 secs = TimGetTicks();
	
	if (cur != NULL)
		*cur = secs;

	return secs;
}
*/

///////////////////////////////////////////////////////////////////////////////


const Char *SCUMMVM_SAVEPATH = "/PALM/Programs/ScummVM/Saved/";



///////////////////////////////////////////////////////////////////////////////
Err SonySoundLib(UInt16 *refNumP)
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = errNone;

	if ((error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		/* Not CLIE: maybe not available */
	} else {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrFm) {
			/* Sound-Lib available */
			if ((error = SysLibFind(sonySysLibNameSound, refNumP))) {
				if (error == sysErrLibNotFound) {
				/* couldn't find lib */
					error = SysLibLoad( 'libr', sonySysFileCSoundLib, refNumP );
				}
			}

			if ( error ) {
				/* Now we can use Sound-Lib */
				FrmCustomAlert(FrmWarnAlert,"Sound Lib not found.",0,0);
			}
		}
	}
	
	if (!error)
		FrmCustomAlert(FrmWarnAlert,"Sound Lib ok",0,0);
	
	return error;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



//UInt8	_indicatorColorOn = 255;
//UInt8	_indicatorColorOff = 0;





/*
int getc(FILE *stream)
{
	UInt8 c;
	DrawStatus(true);
	Err error = VFSFileRead(*stream, 1, &c, NULL);
	DrawStatus(false);

	if (error == errNone || error == vfsErrFileEOF)
		return c;

#ifdef DEBUG
		switch (error)
		{
			case expErrNotOpen:
				FrmCustomAlert(FrmWarn,"expErrNotOpen",0,0);
				break;
			case vfsErrFileBadRef:
				FrmCustomAlert(FrmWarn,"vfsErrFileBadRef",0,0);
				break;
			case vfsErrFileEOF:
				FrmCustomAlert(FrmWarn,"vfsErrFileEOF",0,0);
				break;
			case vfsErrFilePermissionDenied:
				FrmCustomAlert(FrmWarn,"vfsErrFilePermissionDenied",0,0);
				break;
			case vfsErrIsADirectory:
				FrmCustomAlert(FrmWarn,"vfsErrIsADirectory",0,0);
				break;
			case vfsErrNoFileSystem:
				FrmCustomAlert(FrmWarn,"vfsErrNoFileSystem",0,0);
				break;
		}
#endif
	return 0;
}

*/
/*
static Char *enumSaveLoad = NULL;


void doEnumSaveLoad(const Char* path, const Char* game)
{
	if (enumSaveLoad)
		return;
	
	FileRef fileRef;
	Err e = VFSFileOpen(gVars->volRefNum, path, vfsModeRead, &fileRef);
	
	if (e != errNone)
		return;

	UInt32 dirEntryIterator = vfsIteratorStart;
	Char file[32];
	FileInfoType info = {0, file, 32};
	UInt16 lenght;
	UInt32 oldSize = 0;
	UInt32 count = 0;

	while (dirEntryIterator != vfsIteratorStop)
	{
		e = VFSDirEntryEnumerate (fileRef, &dirEntryIterator, &info);
		if (e != expErrEnumerationEmpty)
		{	
			if (StrNCaselessCompare(game, info.nameP, StrLen(game)) == 0)
			{
				lenght = StrLen(info.nameP);
				if (!enumSaveLoad)
					enumSaveLoad = (Char *)MemPtrNew(lenght+1);
				else
				{	
					oldSize = MemPtrSize(enumSaveLoad);
					enumSaveLoad = (Char *)realloc(enumSaveLoad, oldSize + StrLen(info.nameP)+1);
				}
				TxtGlueLowerStr(info.nameP,StrLen(info.nameP));
				StrCopy(enumSaveLoad+oldSize, info.nameP);
				count++;
			}
		}
	}
	
	if (count == 0) {	// if no files, create an empty list item to prevent ALL FILES EXIST
		enumSaveLoad = (Char *)MemPtrNew(1);
		enumSaveLoad[0] = 0;
	}
//	if (count > 20)
//		endEnumSaveLoad();	// too manu files
}

void endEnumSaveLoad()
{
	if (!enumSaveLoad)
		return;

	MemPtrFree(enumSaveLoad);
	enumSaveLoad = NULL;
}

Boolean fexists(const Char* filename)
{
	if (!enumSaveLoad)
		return true;

	Char *found;
	UInt32 search = 0;
	UInt32 size = MemPtrSize(enumSaveLoad);
	UInt32 length = StrLen(filename);

//	Char find[256];
//	StrCopy(find,filename);
	TxtGlueLowerStr((Char *)filename,length);

	while (search < size)
	{
		found = StrStr(filename, enumSaveLoad+search);
		if ( found && found == (filename+length-StrLen(enumSaveLoad+search)) )
			return true;

		search += StrLen(enumSaveLoad+search)+1;
	}
	
	return false;
}*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
