//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Functions for reading version information from PE (Windows EXE) files
//
//=============================================================================

#if defined(ANDROID) || defined(PSP)

#include "pe.h"

#include <string.h>
#include <stdlib.h>
#include "util/stdio_compat.h"


// Simplified structs for PE files

typedef struct {
  char padding[60];
  unsigned int e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct {
  char padding[2];
  unsigned short NumberOfSections;
  char padding1[16];
} IMAGE_FILE_HEADER;

typedef struct {
  unsigned int Signature;
  IMAGE_FILE_HEADER FileHeader;
  char padding[224];
} IMAGE_NT_HEADERS;

typedef struct {
  char Name[8];
  char padding[4];
  unsigned int VirtualAddress;
  char padding1[4];
  unsigned int PointerToRawData;
  char padding2[16];
} IMAGE_SECTION_HEADER;


// These structs are original

typedef struct {
  unsigned int Characteristics;
  unsigned int TimeDateStamp;
  unsigned short MajorVersion;
  unsigned short MinorVersion;
  unsigned short NumberOfNamedEntries;
  unsigned short NumberOfIdEntries;
} IMAGE_RESOURCE_DIRECTORY;

typedef struct {
  union {
    struct {
      unsigned int NameOffset:31;
      unsigned int NameIsString:1;
    };
    unsigned int Name;
    unsigned short Id;
  };
  union {
    unsigned int OffsetToData;
    struct {
       unsigned int OffsetToDirectory:31;
       unsigned int DataIsDirectory:1;
    };
  };
} IMAGE_RESOURCE_DIRECTORY_ENTRY;

typedef struct {
  unsigned int OffsetToData;
  unsigned int Size;
  unsigned int CodePage;
  unsigned int Reserved;
} IMAGE_RESOURCE_DATA_ENTRY;



// Version information

typedef struct {
  unsigned int dwSignature;
  unsigned int dwStrucVersion;
  unsigned int dwFileVersionMS;
  unsigned int dwFileVersionLS;
  unsigned int dwProductVersionMS;
  unsigned int dwProductVersionLS;
  unsigned int dwFileFlagsMask;
  unsigned int dwFileFlags;
  unsigned int dwFileOS;
  unsigned int dwFileType;
  unsigned int dwFileSubtype;
  unsigned int dwFileDateMS;
  unsigned int dwFileDateLS;
} VS_FIXEDFILEINFO; 

typedef struct {
  unsigned short wLength;
  unsigned short wValueLength;
  unsigned short wType;
} STRINGFILEINFO_HEADER;



IMAGE_DOS_HEADER dos_header;
IMAGE_NT_HEADERS nt_headers;
IMAGE_SECTION_HEADER section_header;
IMAGE_RESOURCE_DIRECTORY resource_directory;
IMAGE_RESOURCE_DIRECTORY_ENTRY resource_directory_entry;
IMAGE_RESOURCE_DATA_ENTRY resource_data_entry;

unsigned int resource_virtual_address;
unsigned int resource_start;




void fillBufferFromWidechar(unsigned short* inputBuffer, char* outputText)
{
  unsigned short* input = inputBuffer;
  char* output = outputText;
  
  while (*input)
    *output++ = *input++;

  *output = '\0';
}



int getVersionString(char* version_data, unsigned int size, char* buffer, char* name)
{
  char* current = version_data;
  char* last = version_data + size;

  char temp[200];
  
  // Skip header
  current += 0x28;
  
  // Skip VS_FIXEDFILEINFO
  current += sizeof(VS_FIXEDFILEINFO);
  
  // Now comes either "StringFileInfo" or "VarFileInfo"
  STRINGFILEINFO_HEADER* stringfileinfo_header = (STRINGFILEINFO_HEADER*)current;
  current += sizeof(STRINGFILEINFO_HEADER);
  fillBufferFromWidechar((unsigned short*)current, temp);
  if (strcmp(temp, "VarFileInfo") == 0)
  {
    current += (stringfileinfo_header->wLength - sizeof(STRINGFILEINFO_HEADER));

    // Skip "StringFileInfo" header too
    stringfileinfo_header = (STRINGFILEINFO_HEADER*)current;
    current += 0x3C;
  }
  else
    current += (0x3C - sizeof(STRINGFILEINFO_HEADER));
  
  while (current < last)
  {
    STRINGFILEINFO_HEADER* header = (STRINGFILEINFO_HEADER*)current;
    current += sizeof(STRINGFILEINFO_HEADER);
  
    // Read name
    fillBufferFromWidechar((unsigned short*)current, temp);
  
    if (strcmp(temp, name) == 0)
    {
      current += (2 + 2 * strlen(temp));
  
      // Next value is 32 bit aligned
      current = (char*)((unsigned long)(current + 3) & (~3));
    
      // Read value
      fillBufferFromWidechar((unsigned short*)current, buffer);

      return 1;
    }
    else
      current += (header->wLength - sizeof(STRINGFILEINFO_HEADER));
  
    // Next value is 32 bit aligned
    current = (char*)((unsigned long)(current + 3) & (~3));
  }

  return 0;
}



int seekToResource(FILE* pe, int id)
{
  int i;
  
  // Read in resource directory
  fread(&resource_directory, sizeof(IMAGE_RESOURCE_DIRECTORY), 1, pe);

  // Loop through root node entries till we find the entry with the given id
  for (i = 0; i < resource_directory.NumberOfIdEntries + resource_directory.NumberOfNamedEntries; i++)
  {
    // Read in resource node
    fread(&resource_directory_entry, sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY), 1, pe);

    if ((!resource_directory_entry.NameIsString) && (resource_directory_entry.Id == id))
    {
      // Seek to end of subdirectory
      ags_fseek(pe, resource_start + resource_directory_entry.OffsetToDirectory, SEEK_SET);
      return 1;
    }
  }
  
  return 0;
}



int getVersionInformation(char* filename, version_info_t* version_info)
{
  FILE* pe = fopen(filename, "rb");
  
  if (!pe)
    return 0;

  // Read in the DOS header, get the offset to the PE header and seek
  fread(&dos_header, sizeof(IMAGE_DOS_HEADER), 1, pe);
  ags_fseek(pe, dos_header.e_lfanew, SEEK_SET);

  // Read in the PE header
  fread(&nt_headers, sizeof(IMAGE_NT_HEADERS), 1, pe);

  // Loop through sections till we find the resource section
  int i;
  for (i = 0; i < nt_headers.FileHeader.NumberOfSections; i++)
  {
    fread(&section_header, sizeof(IMAGE_SECTION_HEADER), 1, pe);

    if (strcmp(".rsrc", (char*)section_header.Name) == 0)
      break;
  }
  
  if (i == nt_headers.FileHeader.NumberOfSections)
    goto error_exit;

  // Save virtual address of the resource section
  resource_virtual_address = section_header.VirtualAddress;

  // Seek to the resource section
  ags_fseek(pe, section_header.PointerToRawData, SEEK_SET);

  // Save file offset to the resource section
  resource_start = section_header.PointerToRawData;

  // Search for the version resource in the resource tree
  if (!seekToResource(pe, 16))
    goto error_exit;

  // Enter the first subdirectory in the version resource
  if (!seekToResource(pe, 1))
    goto error_exit;

  // Hopefully found the resource
  fread(&resource_directory, sizeof(IMAGE_RESOURCE_DATA_ENTRY), 1, pe);

  // Read in resource node
  fread(&resource_directory_entry, sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY), 1, pe);

  // Seek to the resource data
  ags_fseek(pe, resource_start + resource_directory_entry.OffsetToData, SEEK_SET);

  // Read in version info
  fread(&resource_data_entry, sizeof(IMAGE_RESOURCE_DATA_ENTRY), 1, pe);

  // Finally we got a virtual address of the resource, now seek to it
  ags_fseek(pe, resource_start + resource_data_entry.OffsetToData - resource_virtual_address, SEEK_SET);
  
  // Read version resource
  char* version_data = (char*)malloc(resource_data_entry.Size);
  fread(version_data, resource_data_entry.Size, 1, pe);
  
  memset(version_info, 0, sizeof(version_info_t));  
  getVersionString(version_data, resource_data_entry.Size, version_info->version, "FileVersion");
  getVersionString(version_data, resource_data_entry.Size, version_info->description, "FileDescription");
  getVersionString(version_data, resource_data_entry.Size, version_info->internal_name, "InternalName");

  free(version_data);
  fclose(pe);

  return 1;
  
error_exit:
  fclose(pe);
  return 0;
}

#endif