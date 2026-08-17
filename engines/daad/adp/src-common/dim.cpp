#ifdef HAS_VIRTUALFILESYSTEM

#include <dim.h>
#include <dim_fat.h>
#include <dim_cpc.h>
#include <dim_adf.h>
#include <os_mem.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static DIM_Error _DIM_Error = DIMError_None;

DIM_Error DIM_GetError ()
{
	return _DIM_Error;
}

const char* DIM_GetErrorString ()
{
	switch (_DIM_Error)
	{
		case DIMError_None:                 return "No error";
		case DIMError_CreatingFile:         return "Error creating file";
		case DIMError_DiskFull:             return "Disk full";
		case DIMError_FileNotFound:         return "File not found";
		case DIMError_InvalidFile:          return "Invalid or corrupted file";
		case DIMError_InvalidDisk:          return "Unknown disk image format";
		case DIMError_OutOfMemory:          return "Out of memory";
		case DIMError_ReadError:            return "Read error";
		case DIMError_WriteError:           return "Write error";
		case DIMError_FileExists:           return "File exists";
		case DIMError_DirectoryFull:        return "Directory full";
		case DIMError_DirectoryNotEmpty:    return "Directory not empty";
		case DIMError_DirectoryNotFound:    return "Directory not found";
		case DIMError_NotADirectory:        return "Not a directory";
		case DIMError_CommandNotSupported:  return "Command not supported";
        case DIMError_FormatNotSupported:   return "Disk format not supported";
	}
	return "Unknown error";
}

void DIM_SetError (DIM_Error error)
{
	_DIM_Error = error;
}

// -------------------------------------------------------------------------

void DIM_CopyPatternTo8D3 (const char* pattern, char* fileName, char* extension)
{
	if (pattern == NULL)
		pattern = "*";

	const char* ptr = pattern;
	const char* dot = strrchr(pattern, '.');
	char* out = fileName;
	memcpy(fileName, "        ", 8);
	while (*ptr && ptr != dot && *ptr != '\\' && out < fileName + 8)
		*out++ = (char)toupper(*ptr++);
	if (ptr == dot)
	{
		memcpy(extension, "   ", 3);
		ptr++;
		out = extension;
		while (*ptr && *ptr != '\\' && out < extension + 3)
			*out++ = (char)toupper(*ptr++);
	}
	else
	{
		memcpy(extension, "*  ", 3);
	}
}

bool DIM_MatchWildcards (const char *fileName, int fileNameSize, const char *pattern, int patternSize)
{
	const char* fileEnd = fileName + fileNameSize;
	const char* patternEnd = pattern + patternSize;

	while (fileName < fileEnd && pattern < patternEnd)
	{
		if (*pattern == '?')
		{
			pattern++;
			fileName++;
			continue;
		}
		if (*pattern == '*')
		{
			while (*pattern == '*')
				pattern++;
			if (pattern == patternEnd || *pattern == ' ')
				return true;
			while (fileName < fileEnd)
			{
				if (DIM_MatchWildcards(fileName, (int)(fileEnd - fileName), pattern, (int)(patternEnd - pattern)))
					return true;
				fileName++;
			}
			return false;
		}

		// TODO: Make this optional
		char c = *fileName & 0x7F;

		if (c == *pattern || toupper(c) == toupper(*pattern))
		{
			if (c == ' ')
				return true;
			pattern++;
			fileName++;
			continue;
		}
		return false;
	}
	if (fileName == fileEnd && pattern != patternEnd && *pattern != ' ' && *pattern != '*')
		return false;
	if (pattern == patternEnd && fileName != fileEnd && *fileName != ' ')
		return false;
	return true;
}

const char* DIM_CheckPatternFolder (DIM_Disk* disk, const char* pattern)
{
	char buffer[128];

	if (pattern == 0)
		return 0;
	for (;;)
	{
		const char* dirsep = strchr(pattern, '/');
		const char* bslash = strchr(pattern, '\\');
		if (bslash && (dirsep == 0 || dirsep > bslash))
			dirsep = bslash;
		if (dirsep == 0)
			return pattern;
		if (dirsep-pattern > 127)
		{
			DIM_SetError(DIMError_CommandNotSupported);
			return 0;
		}

		memcpy(buffer, pattern, dirsep - pattern);
		buffer[dirsep-pattern] = 0;
		if (DIM_ChangeDirectory(disk, buffer) == 0)
			return 0;
		pattern = dirsep+1;
	}
}

// -------------------------------------------------------------------------

DIM_Disk* DIM_CreateDisk(const char* fileName, uint32_t size, const char* preset)
{
    const char* extension = strrchr(fileName, '.');
    if (extension == NULL)
    {
        DIM_SetError(DIMError_FormatNotSupported);
        return NULL;
    }

	DIM_DiskType type;
	if (StrIComp(extension, ".img") == 0 || StrIComp(extension, ".st") == 0)
	{
		type = DIM_FAT;
	}
	else if (StrIComp(extension, ".dsk") == 0)
    {
        type = DIM_CPC;
    }
    else if (StrIComp(extension, ".adf") == 0)
    {
        type = DIM_ADF;
    }
	else
	{
		DIM_SetError(DIMError_FormatNotSupported);
		return NULL;
	}

    switch (type)
    {
        case DIM_FAT:
        {
        	FAT_Disk* fatDisk = FAT_CreateDisk(fileName, size);
        	if (fatDisk == NULL)
        		return 0;

        	DIM_Disk* disk = Allocate<DIM_Disk>("DIM Disk");
        	if (disk == NULL)
        	{
        		DIM_SetError(DIMError_OutOfMemory);
        		return 0;
        	}
        	disk->fat = fatDisk;
        	disk->type = DIM_FAT;
        	disk->capabilities = DIMCaps_Directories;
        	return disk;
        }
        case DIM_CPC:
        {
	        	CPC_Disk* cpcDisk = CPC_CreateDisk(fileName, preset);
	        	if (cpcDisk == NULL)
	        		return 0;

	        	DIM_Disk* disk = Allocate<DIM_Disk>("DIM Disk");
	        	if (disk == NULL)
	        	{
	        		DIM_SetError(DIMError_OutOfMemory);
	        		return 0;
	        	}
	        	disk->cpc = cpcDisk;
	        	disk->type = DIM_CPC;
	        	disk->capabilities = 0;
	        	return disk;
        }
        case DIM_ADF:
        {
            ADF_Disk* adfDisk = ADF_CreateDisk(fileName, size);
            if (adfDisk == NULL)
                return 0;
            DIM_Disk* disk = Allocate<DIM_Disk>("DIM Disk");
            if (disk == NULL)
            {
                DIM_SetError(DIMError_OutOfMemory);
                return 0;
            }
            disk->adf = adfDisk;
            disk->type = DIM_ADF;
            disk->capabilities = DIMCaps_Directories;
            return disk;
        }
        default:
            DIM_SetError(DIMError_FormatNotSupported);
            return NULL;
    }
}

DIM_Disk* DIM_OpenDisk (const char* fileName)
{
	DIM_Disk* disk = Allocate<DIM_Disk>("DIM Disk");
	if (disk == NULL)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return 0;
	}
	
	disk->fat = FAT_OpenDisk(fileName);
	if (disk->fat != NULL)
	{
		disk->type = DIM_FAT;
		disk->capabilities = DIMCaps_Directories;
		return disk;
	}

	disk->cpc = CPC_OpenDisk(fileName);
	if (disk->cpc != NULL)
	{
		disk->type = DIM_CPC;
		disk->capabilities = 0;
		return disk;
	}

	disk->adf = ADF_OpenDisk(fileName);
	if (disk->adf != NULL)
	{
		disk->type = DIM_ADF;
		disk->capabilities = DIMCaps_Directories;
		return disk;
	}

	File* file = File_Open(fileName);
	if (file)
	{
		File_Close(file);
		DIM_SetError(DIMError_InvalidDisk);
	}
	else
	{
		DIM_SetError(DIMError_FileNotFound);
	}
	Free(disk);
	return 0;
}

static void FillFindResults (DIM_Disk* disk, FindFileResults* results)
{
	switch (disk->type)
	{
		case DIM_FAT:
		{
			FAT_FindResults* fr = (FAT_FindResults*) &results->internalData;
			FAT_GetFileName(&fr->entry, results->fileName, FILE_MAX_PATH);
			results->fileSize = fr->entry.fileSize;
			results->attributes = 0;
			if (fr->entry.attributes & FAT_DIRECTORY)
				results->attributes |= FileAttribute_Directory;
			if (fr->entry.attributes & FAT_READONLY)
				results->attributes |= FileAttribute_ReadOnly;
			if (fr->entry.attributes & FAT_HIDDEN)
				results->attributes |= FileAttribute_Hidden;
			if (fr->entry.attributes & FAT_SYSTEM)
				results->attributes |= FileAttribute_System;
			if (fr->entry.attributes & FAT_ARCHIVE)
				results->attributes |= FileAttribute_Archive;
			results->description[0] = 0;

			if (fr->entry.modifyDate == 0)
			{
				results->modifyTime = 0;
			}
			else
			{
				struct tm t = {0};
				t.tm_sec  = (fr->entry.modifyTime & 0x1F) * 2;
				t.tm_min  = (fr->entry.modifyTime >> 5) & 0x3F;
				t.tm_hour = (fr->entry.modifyTime >> 11) & 0x1F;
				t.tm_mday = (fr->entry.modifyDate & 0x1F);
				t.tm_mon  = ((fr->entry.modifyDate >> 5) & 0x0F) - 1;
				t.tm_year = ((fr->entry.modifyDate >> 9) & 0x7F) + 80;
				t.tm_isdst = -1;
				results->modifyTime = mktime(&t);
			}
			break;
		}
		case DIM_CPC:
		{
			CPC_FindResults* fr = (CPC_FindResults*) &results->internalData;
			strncpy(results->fileName, fr->fileName, 12);
			strncpy(results->description, fr->description, 32);
			results->fileSize = fr->fileSize;
			results->fileName[12] = 0;
			results->attributes = 0;
			results->modifyTime = 0;
			break;
		}
		case DIM_ADF:
		{
			ADF_FindResults* fr = (ADF_FindResults*) &results->internalData;
			strncpy(results->fileName, fr->fileName, 30);
			results->description[0] = 0;
			results->fileSize = fr->fileSize;
			results->attributes = fr->directory ? FileAttribute_Directory : 0;
			results->modifyTime = (time_t)(fr->days * 86400LL + fr->minutes * 60LL + fr->ticks / 50 + 252460800);
			break;
		}
	}
}

bool DIM_FindFile (DIM_Disk* disk, FindFileResults* results, const char* name)
{
	name = DIM_CheckPatternFolder(disk, name);
	if (name == 0)
		return 0;

	bool found = false;
	switch (disk->type)
	{
		case DIM_FAT:
			found = FAT_FindFile(disk->fat, (FAT_FindResults*)&results->internalData, name);
			break;
		case DIM_CPC:
			found = CPC_FindFile(disk->cpc, (CPC_FindResults*)&results->internalData, name);
			break;
		case DIM_ADF:
			found = ADF_FindFile(disk->adf, (ADF_FindResults*)&results->internalData, name);
			break;
	}
	if (found)
	{
		FillFindResults(disk, results);
		return true;
	}

	DIM_SetError(DIMError_CommandNotSupported);
	return false;
}

bool DIM_FindFirstFile (DIM_Disk* disk, FindFileResults* results, const char* pattern)
{
	if (pattern != 0)
	{
		pattern = DIM_CheckPatternFolder(disk, pattern);
		if (pattern == 0)
			return 0;
	}

	bool found = false;
	switch (disk->type)
	{
		case DIM_FAT:
			found = FAT_FindFirstFile(disk->fat, (FAT_FindResults*) &results->internalData, pattern);
			break;
		case DIM_CPC:
			found = CPC_FindFirstFile(disk->cpc, (CPC_FindResults*) &results->internalData, pattern);
			break;
		case DIM_ADF:
			found = ADF_FindFirstFile(disk->adf, (ADF_FindResults*) &results->internalData, pattern);
			break;
	}
	if (found)
	{
		FillFindResults(disk, results);
		return true;
	}
	DIM_SetError(DIMError_CommandNotSupported);
	return false;
}

bool DIM_FindNextFile (DIM_Disk* disk, FindFileResults* results)
{
	bool found = false;
	switch (disk->type)
	{
		case DIM_FAT:
			found = FAT_FindNextFile(disk->fat, (FAT_FindResults*) &results->internalData);
			break;
		case DIM_CPC:
			found = CPC_FindNextFile(disk->cpc, (CPC_FindResults*) &results->internalData);
			break;
		case DIM_ADF:
			found = ADF_FindNextFile(disk->adf, (ADF_FindResults*) &results->internalData);
			break;
	}
	if (found)
	{
		FillFindResults(disk, results);
		return true;
	}
	DIM_SetError(DIMError_CommandNotSupported);
	return false;
}

uint32_t DIM_ReadFile (DIM_Disk* disk, const char* path, uint8_t* buffer, uint32_t bufferSize)
{
	path = DIM_CheckPatternFolder(disk, path);
	if (path == 0)
		return 0;

	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_ReadFile(disk->fat, path, buffer, bufferSize);
		case DIM_CPC:
			return CPC_ReadFile(disk->cpc, path, buffer, bufferSize);
		case DIM_ADF:
			return ADF_ReadFile(disk->adf, path, buffer, bufferSize);
	}
	DIM_SetError(DIMError_CommandNotSupported);
	return 0;
}

uint32_t DIM_WriteFile (DIM_Disk* disk, const char* path, const uint8_t* buffer, uint32_t bufferSize)
{
	path = DIM_CheckPatternFolder(disk, path);
	if (path == 0)
		return 0;

	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_WriteFile(disk->fat, path, buffer, bufferSize);
		case DIM_CPC:
			return CPC_WriteFile(disk->cpc, path, buffer, bufferSize);
        case DIM_ADF:
            return ADF_WriteFile(disk->adf, path, buffer, bufferSize);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return 0;
	}
}

bool DIM_RemoveFile (DIM_Disk* disk, const char* name)
{
	name = DIM_CheckPatternFolder(disk, name);
	if (name == 0)
		return 0;

	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_RemoveFile(disk->fat, name);
		case DIM_CPC:
			return CPC_DeleteFile(disk->cpc, name);
        case DIM_ADF:
            return ADF_RemoveFile(disk->adf, name);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

bool DIM_ChangeDirectory (DIM_Disk* disk, const char* name)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_ChangeDirectory(disk->fat, name);
		case DIM_ADF:
			return ADF_ChangeDirectory(disk->adf, name);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

bool DIM_MakeDirectory (DIM_Disk* disk, const char* name)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_MakeDirectory(disk->fat, name);
        case DIM_ADF:
            return ADF_MakeDirectory(disk->adf, name);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

bool DIM_RemoveDirectory (DIM_Disk* disk, const char* name)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_RemoveDirectory(disk->fat, name);
        case DIM_ADF:
            return ADF_RemoveDirectory(disk->adf, name);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

uint32_t DIM_GetCWD (DIM_Disk* disk, char* buffer, uint32_t bufferSize)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_GetCWD(disk->fat, buffer, bufferSize);
		case DIM_ADF:
			return ADF_GetCWD(disk->adf, buffer, bufferSize);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return 0;
	}
}

uint64_t DIM_GetFreeSpace (DIM_Disk* disk)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_GetFreeSpace(disk->fat);
		case DIM_CPC:
			return CPC_GetFreeSpace(disk->cpc);
		case DIM_ADF:
			return ADF_GetFreeSpace(disk->adf);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return 0;
	}
}

uint32_t DIM_GetVolumeLabel(DIM_Disk* disk, char* buffer, uint32_t bufferSize)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_GetVolumeLabel(disk->fat, buffer, bufferSize);
		case DIM_ADF:
			return ADF_GetVolumeLabel(disk->adf, buffer, bufferSize);
		default:
			return 0;
	}
}

bool DIM_SetVolumeLabel(DIM_Disk* disk, const char* label)
{
	switch (disk->type)
	{
		case DIM_FAT:
			return FAT_SetVolumeLabel(disk->fat, label);
		case DIM_ADF:
			return ADF_SetVolumeLabel(disk->adf, label);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

bool DIM_ReadBootBlock(DIM_Disk* disk, void* buffer, uint32_t size)
{
	switch (disk->type)
	{
		case DIM_ADF:
			return ADF_ReadBootBlock(disk->adf, buffer, size);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

bool DIM_WriteBootBlock(DIM_Disk* disk, const void* buffer, uint32_t size)
{
	switch (disk->type)
	{
		case DIM_ADF:
			return ADF_WriteBootBlock(disk->adf, buffer, size);
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return false;
	}
}

void DIM_DumpInfo (DIM_Disk* disk)
{
	switch (disk->type)
	{
		case DIM_FAT:
			FAT_DumpInfo(disk->fat);
			return;
		case DIM_CPC:
			CPC_DumpInfo(disk->cpc);
			return;
		case DIM_ADF:
			ADF_DumpInfo(disk->adf);
			return;
		default:
			DIM_SetError(DIMError_CommandNotSupported);
			return;
	}
}

void DIM_CloseDisk(DIM_Disk* disk)
{
	switch (disk->type)
	{
		case DIM_FAT: 
			FAT_CloseDisk(disk->fat);
			break;
		case DIM_CPC: 
			CPC_CloseDisk(disk->cpc);
			break;
		case DIM_ADF: 
			ADF_CloseDisk(disk->adf);
			break;
		default:
			Free(disk);
			break;
	}
}

#endif
