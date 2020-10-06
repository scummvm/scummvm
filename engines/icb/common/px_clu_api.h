/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_CLU_API_H
#define ICB_CLU_API_H

namespace ICB {

#define MAX_DESCRIPTION_SIZE 60
#define MAX_FILENAME_SIZE 128
#define NON_KOSHA_FILE "ZZT_666_BUGGERED_666_ZZT"
#define NON_KOSHA_HASH 0xFFFFFFFF

#define CLUSTER_API_SCHEMA 2
#define CLUSTER_API_ID "CLU"

#define NULL_HASH 0x0

#define FOLDER_FILE_ID "ZZT"

// moved actual function to clu_api.cpp to save some memory maybe (testing)
uint32 EngineHashString(const char *fn);
uint32 EngineHashFile(const char *fn, char *output);
uint32 EngineHashToFile(uint32 hash, char *output); // Take a hash convert that hash value to 7 character filename

// To let the engine use the old version of HashString & HashFile
#define HashString EngineHashString
#define HashFile EngineHashFile

// File Layout is:
// HEADER_OPEN
// HEADER_NORMAL[n]
// filename1
// filename2
// ...
// filenamen
// file1
// file2
// ...
// filen
//
// The filenames are padded to multiple of 4 bytes
// The files are padded to multiple of 8 bytes

typedef struct HEADER_OPEN {
	uint32 header_size;
	uint32 noFiles;
	uint32 cluster_hash;
	int cdpos;
	char description[MAX_DESCRIPTION_SIZE];
} HEADER_OPEN;

typedef struct HEADER_NORMAL {
	uint32 fnOffset; // WAS: char* fn;
	uint32 size;
	uint32 offset;
	uint32 hash;
} HEADER_NORMAL;

typedef struct Cluster_API {
	char ID[4];
	uint32 schema;
	HEADER_OPEN ho;
	HEADER_NORMAL hn[1]; // hn[n]
	                     // string data
	                     // file data
} Cluster_API;

#define FILE_NAME(a) (sizeof(uint) + sizeof(uint) + sizeof(HEADER_OPEN) + (sizeof(HEADER_NORMAL) * (a)))
#define FILE_SIZE(a) (FILE_NAME(a) + sizeof(uint32))
#define FILE_OFFSET(a) (FILE_SIZE(a) + sizeof(uint32))
#define FILE_HASH(a) (FILE_OFFSET(a) + sizeof(uint32))

} // End of namespace ICB

#endif // #ifndef CLU_API_H
